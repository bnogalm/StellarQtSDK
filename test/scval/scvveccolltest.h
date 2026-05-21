#ifndef SCVVECCOLLTEST_H
#define SCVVECCOLLTEST_H

#include <QObject>
#include <QtTest>
#include "../testcollector.h"

#include "../../src/scval/scv.h"

/**
 * SCV_VEC / SCV_MAP helpers and the canonical SCVal comparator.
 */
class ScvVecMapTest: public QObject
{
    Q_OBJECT
private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    static stellar::SCVal roundtripXdr(const stellar::SCVal& src)
    {
        QByteArray bytes;
        { QDataStream s(&bytes, QIODevice::WriteOnly); s << src; }
        stellar::SCVal back;
        { QDataStream s(&bytes, QIODevice::ReadOnly); s >> back; }
        return back;
    }

    // ─── Vec ─────────────────────────────────────────────────────────

    void testVecEmpty()
    {
        stellar::SCVal v = Scv::toVec({});
        QCOMPARE(static_cast<int>(v.type),
                 static_cast<int>(stellar::SCValType::SCV_VEC));
        QCOMPARE(Scv::fromVec(roundtripXdr(v)).size(), 0);
    }

    void testVecHeterogeneous()
    {
        QList<stellar::SCVal> elems;
        elems.append(Scv::toUint32(1));
        elems.append(Scv::toBoolean(true));
        elems.append(Scv::toSymbol("hello"));
        elems.append(Scv::toBytes(QByteArray("\x01\x02\x03", 3)));

        stellar::SCVal v = roundtripXdr(Scv::toVec(elems));
        QList<stellar::SCVal> back = Scv::fromVec(v);
        QCOMPARE(back.size(), 4);
        QCOMPARE(Scv::fromUint32(back.at(0)), quint32(1));
        QCOMPARE(Scv::fromBoolean(back.at(1)), true);
        QCOMPARE(Scv::fromSymbol(back.at(2)), QString("hello"));
        QCOMPARE(Scv::fromBytes(back.at(3)), QByteArray("\x01\x02\x03", 3));
    }

    void testVecNested()
    {
        // [ [1, 2], [3] ]
        QList<stellar::SCVal> inner1 = { Scv::toInt32(1), Scv::toInt32(2) };
        QList<stellar::SCVal> inner2 = { Scv::toInt32(3) };
        stellar::SCVal nested = Scv::toVec({ Scv::toVec(inner1), Scv::toVec(inner2) });

        stellar::SCVal back = roundtripXdr(nested);
        QList<stellar::SCVal> outer = Scv::fromVec(back);
        QCOMPARE(outer.size(), 2);
        QList<stellar::SCVal> i1 = Scv::fromVec(outer.at(0));
        QList<stellar::SCVal> i2 = Scv::fromVec(outer.at(1));
        QCOMPARE(i1.size(), 2);
        QCOMPARE(i2.size(), 1);
        QCOMPARE(Scv::fromInt32(i1.at(0)), 1);
        QCOMPARE(Scv::fromInt32(i1.at(1)), 2);
        QCOMPARE(Scv::fromInt32(i2.at(0)), 3);
    }

    // ─── Map ─────────────────────────────────────────────────────────

    void testMapEmpty()
    {
        stellar::SCVal v = Scv::toMap(QList<stellar::SCMapEntry>{});
        QCOMPARE(static_cast<int>(v.type),
                 static_cast<int>(stellar::SCValType::SCV_MAP));
        QCOMPARE(Scv::fromMap(roundtripXdr(v)).size(), 0);
    }

    void testMapParallelLists()
    {
        QList<stellar::SCVal> keys   = { Scv::toSymbol("name"),  Scv::toSymbol("age") };
        QList<stellar::SCVal> values = { Scv::toString(QStringLiteral("alice")), Scv::toUint32(30) };
        stellar::SCVal v = roundtripXdr(Scv::toMap(keys, values));
        QList<stellar::SCMapEntry> entries = Scv::fromMap(v);
        QCOMPARE(entries.size(), 2);
    }

    /** Keys are canonicalized into XDR-byte ascending order. */
    void testMapCanonicalOrdering()
    {
        QList<stellar::SCVal> keys   = { Scv::toUint32(3), Scv::toUint32(1), Scv::toUint32(2) };
        QList<stellar::SCVal> values = { Scv::toSymbol("c"), Scv::toSymbol("a"), Scv::toSymbol("b") };
        QList<stellar::SCMapEntry> entries = Scv::fromMap(Scv::toMap(keys, values));
        QCOMPARE(entries.size(), 3);
        QCOMPARE(Scv::fromUint32(entries.at(0).key), quint32(1));
        QCOMPARE(Scv::fromUint32(entries.at(1).key), quint32(2));
        QCOMPARE(Scv::fromUint32(entries.at(2).key), quint32(3));
        QCOMPARE(Scv::fromSymbol(entries.at(0).val), QString("a"));
        QCOMPARE(Scv::fromSymbol(entries.at(1).val), QString("b"));
        QCOMPARE(Scv::fromSymbol(entries.at(2).val), QString("c"));
    }

    /** Two semantically-equal maps (same keys/values, different input order)
     *  serialize to byte-identical XDR. */
    void testMapByteIdenticalForReorderedInput()
    {
        QList<stellar::SCVal> k1 = { Scv::toUint32(1), Scv::toUint32(2), Scv::toUint32(3) };
        QList<stellar::SCVal> v1 = { Scv::toInt32(10), Scv::toInt32(20), Scv::toInt32(30) };
        QList<stellar::SCVal> k2 = { Scv::toUint32(3), Scv::toUint32(1), Scv::toUint32(2) };
        QList<stellar::SCVal> v2 = { Scv::toInt32(30), Scv::toInt32(10), Scv::toInt32(20) };

        QByteArray a, b;
        { QDataStream s(&a, QIODevice::WriteOnly); s << Scv::toMap(k1, v1); }
        { QDataStream s(&b, QIODevice::WriteOnly); s << Scv::toMap(k2, v2); }
        QCOMPARE(a, b);
    }

    void testMapRejectsDuplicateKeys()
    {
        QList<stellar::SCVal> keys = { Scv::toUint32(1), Scv::toUint32(1) };
        QList<stellar::SCVal> values = { Scv::toInt32(1), Scv::toInt32(2) };
        bool threw = false;
        try { Scv::toMap(keys, values); }
        catch (const std::exception&) { threw = true; }
        QVERIFY(threw);
    }

    void testMapKeysValuesLengthMismatch()
    {
        bool threw = false;
        try {
            Scv::toMap({ Scv::toUint32(1) }, { Scv::toInt32(1), Scv::toInt32(2) });
        } catch (const std::exception&) {
            threw = true;
        }
        QVERIFY(threw);
    }

    // ─── Comparator ──────────────────────────────────────────────────

    void testCompareSameValueIsZero()
    {
        QCOMPARE(Scv::compare(Scv::toUint32(42), Scv::toUint32(42)), 0);
        QCOMPARE(Scv::compare(Scv::toSymbol("abc"), Scv::toSymbol("abc")), 0);
    }

    void testCompareByTypeDiscriminator()
    {
        // SCV_BOOL (0) precedes SCV_VOID (1) precedes SCV_U32 (3) ...
        QVERIFY(Scv::compare(Scv::toBoolean(true), Scv::toVoid()) < 0);
        QVERIFY(Scv::compare(Scv::toVoid(), Scv::toUint32(0)) < 0);
    }

    void testCompareByContent()
    {
        QVERIFY(Scv::compare(Scv::toUint32(1), Scv::toUint32(2)) < 0);
        QVERIFY(Scv::compare(Scv::toUint32(2), Scv::toUint32(1)) > 0);
    }
};

ADD_TEST(ScvVecMapTest)
#endif // SCVVECCOLLTEST_H
