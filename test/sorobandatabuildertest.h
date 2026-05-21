#ifndef SOROBANDATABUILDERTEST_H
#define SOROBANDATABUILDERTEST_H

#include <QObject>
#include <QtTest>
#include "testcollector.h"

#include "../src/sorobandatabuilder.h"
#include "../src/scval/scv.h"

/** SorobanDataBuilder fluent composition + XDR round-trip (CAP-46). */
class SorobanDataBuilderTest: public QObject
{
    Q_OBJECT
private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    static stellar::SorobanLedgerKey makeContractDataKey(const QString& accountStrkey,
                                                         const QString& storageKeySymbol)
    {
        stellar::SorobanLedgerKey k;
        k.type = stellar::LedgerEntryType::CONTRACT_DATA;
        // Contract address from a G-strkey (using ACCOUNT variant of SCAddress).
        stellar::SCVal addrVal = Scv::toAddress(accountStrkey);
        k.contractData.contract = addrVal.address;
        k.contractData.key = Scv::toSymbol(storageKeySymbol);
        k.contractData.durability = stellar::ContractDataDurability::PERSISTENT;
        return k;
    }

    static stellar::SorobanLedgerKey makeContractCodeKey(quint8 fill)
    {
        stellar::SorobanLedgerKey k;
        k.type = stellar::LedgerEntryType::CONTRACT_CODE;
        for (int i = 0; i < 32; ++i) k.contractCode.hash[i] = fill;
        return k;
    }

    static stellar::SorobanLedgerKey makeTtlKey(quint8 fill)
    {
        stellar::SorobanLedgerKey k;
        k.type = stellar::LedgerEntryType::TTL;
        for (int i = 0; i < 32; ++i) k.ttl.keyHash[i] = fill;
        return k;
    }

    void testEmptyBuilderHasZeroFootprint()
    {
        stellar::SorobanTransactionData data = SorobanDataBuilder().build();
        QCOMPARE(data.resources.footprint.readOnly.size(), 0);
        QCOMPARE(data.resources.footprint.readWrite.size(), 0);
        QCOMPARE(data.resources.instructions, quint32(0));
        QCOMPARE(data.resourceFee, qint64(0));
    }

    void testFluentChainStoresAllFields()
    {
        QList<stellar::SorobanLedgerKey> ro = {
            makeContractDataKey("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG", "data"),
            makeTtlKey(0xAA),
        };
        QList<stellar::SorobanLedgerKey> rw = {
            makeContractCodeKey(0xCC),
        };

        SorobanDataBuilder b;
        stellar::SorobanTransactionData data = b
            .setReadOnly(ro)
            .setReadWrite(rw)
            .setResources(1'000'000u, 4096u, 2048u)
            .setResourceFee(qint64(123456))
            .build();

        QCOMPARE(data.resources.footprint.readOnly.size(),  2);
        QCOMPARE(data.resources.footprint.readWrite.size(), 1);
        QCOMPARE(data.resources.instructions,  quint32(1'000'000));
        QCOMPARE(data.resources.diskReadBytes, quint32(4096));
        QCOMPARE(data.resources.writeBytes,    quint32(2048));
        QCOMPARE(data.resourceFee,             qint64(123456));
    }

    void testXdrRoundTrip()
    {
        QList<stellar::SorobanLedgerKey> ro = {
            makeContractCodeKey(0x11),
            makeTtlKey(0x22),
        };
        QList<stellar::SorobanLedgerKey> rw = {
            makeContractDataKey("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG", "balance"),
        };

        SorobanDataBuilder b;
        QByteArray bytes = b
            .setReadOnly(ro)
            .setReadWrite(rw)
            .setResources(500'000u, 1024u, 512u)
            .setResourceFee(qint64(99))
            .toXdr();

        stellar::SorobanTransactionData back;
        { QDataStream s(&bytes, QIODevice::ReadOnly); s >> back; }

        QCOMPARE(back.resources.footprint.readOnly.size(),  2);
        QCOMPARE(back.resources.footprint.readWrite.size(), 1);
        QCOMPARE(static_cast<int>(back.resources.footprint.readOnly.at(0).type),
                 static_cast<int>(stellar::LedgerEntryType::CONTRACT_CODE));
        QCOMPARE(static_cast<int>(back.resources.footprint.readOnly.at(1).type),
                 static_cast<int>(stellar::LedgerEntryType::TTL));
        QCOMPARE(static_cast<int>(back.resources.footprint.readWrite.at(0).type),
                 static_cast<int>(stellar::LedgerEntryType::CONTRACT_DATA));
        QCOMPARE(back.resources.instructions,  quint32(500'000));
        QCOMPARE(back.resources.diskReadBytes, quint32(1024));
        QCOMPARE(back.resources.writeBytes,    quint32(512));
        QCOMPARE(back.resourceFee,             qint64(99));
    }

    void testBuilderFromExisting()
    {
        SorobanDataBuilder original;
        stellar::SorobanTransactionData base = original
            .setResources(42u, 0u, 0u)
            .setResourceFee(qint64(7))
            .build();

        // Amend via the existing-data ctor.
        SorobanDataBuilder amended(base);
        stellar::SorobanTransactionData out = amended
            .setResources(100u, 200u, 300u)
            .build();

        QCOMPARE(out.resources.instructions,  quint32(100));
        QCOMPARE(out.resources.diskReadBytes, quint32(200));
        QCOMPARE(out.resources.writeBytes,    quint32(300));
        QCOMPARE(out.resourceFee,             qint64(7));  // preserved from base
    }

    void testContractDataKeyContainsScVal()
    {
        stellar::SorobanLedgerKey k = makeContractDataKey(
            "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG", "totalSupply");
        QCOMPARE(static_cast<int>(k.contractData.durability),
                 static_cast<int>(stellar::ContractDataDurability::PERSISTENT));
        QCOMPARE(static_cast<int>(k.contractData.key.type),
                 static_cast<int>(stellar::SCValType::SCV_SYMBOL));
        QCOMPARE(QString::fromUtf8(k.contractData.key.symbol), QString("totalSupply"));
    }
};

ADD_TEST(SorobanDataBuilderTest)
#endif // SOROBANDATABUILDERTEST_H
