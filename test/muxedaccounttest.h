#ifndef MUXEDACCOUNTTEST_H
#define MUXEDACCOUNTTEST_H

#include <QObject>
#include <QtTest>
#include "testcollector.h"

#include "../src/muxedaccount.h"
#include "../src/strkey.h"

/** Tests for the first-class MuxedAccount value type (SEP-23). */
class MuxedAccountTest: public QObject
{
    Q_OBJECT
private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    void testFromAccountIdNotMuxed()
    {
        const QString g = "GA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVSGZ";
        MuxedAccount m = MuxedAccount::fromAccountId(g);
        QVERIFY(!m.isMuxed());
        QCOMPARE(m.getAccountId(), g);
        QCOMPARE(m.getMuxedId(), quint64(0));
        QCOMPARE(m.toAddress(), g); // G-strkey out
    }

    void testFromAccountIdAndMuxedId()
    {
        const QString g = "GA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVSGZ";
        const quint64 id = 0x8000000000000000ULL;

        MuxedAccount m = MuxedAccount::fromAccountIdAndMuxedId(g, id);
        QVERIFY(m.isMuxed());
        QCOMPARE(m.getAccountId(), g);
        QCOMPARE(m.getMuxedId(), id);
        QCOMPARE(m.toAddress(),
                 QString("MA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVAAAAAAAAAAAAAJLK"));
    }

    void testFromAddressRoundTrip()
    {
        const QString muxed = "MA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVAAAAAAAAAAAAAJLK";
        MuxedAccount m = MuxedAccount::fromAddress(muxed);
        QVERIFY(m.isMuxed());
        QCOMPARE(m.toAddress(), muxed);
        QCOMPARE(m.getAccountId(),
                 QString("GA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVSGZ"));
        QCOMPARE(m.getMuxedId(), quint64(0x8000000000000000ULL));

        const QString plainG = "GA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVSGZ";
        MuxedAccount p = MuxedAccount::fromAddress(plainG);
        QVERIFY(!p.isMuxed());
        QCOMPARE(p.toAddress(), plainG);
    }

    void testXdrRoundTrip()
    {
        const QString muxed = "MA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVAAAAAAAAAAAAAJLK";
        MuxedAccount m = MuxedAccount::fromAddress(muxed);
        stellar::MuxedAccount xdr = m.toXdr();
        MuxedAccount back(xdr);
        QVERIFY(m == back);
        QCOMPARE(back.toAddress(), muxed);
    }
};

ADD_TEST(MuxedAccountTest)
#endif // MUXEDACCOUNTTEST_H
