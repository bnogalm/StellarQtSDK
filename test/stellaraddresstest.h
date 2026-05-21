#ifndef STELLARADDRESSTEST_H
#define STELLARADDRESSTEST_H

#include <QObject>
#include <QtTest>
#include "testcollector.h"

#include "../src/stellaraddress.h"
#include "../src/strkey.h"

/** Tests for the polymorphic StellarAddress (SEP-23 wrapper over all 5 strkey forms). */
class StellarAddressTest: public QObject
{
    Q_OBJECT
private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    void testAccountRoundTrip()
    {
        const QString g = "GA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVSGZ";
        StellarAddress a = StellarAddress::fromString(g);
        QCOMPARE(static_cast<int>(a.getType()),
                 static_cast<int>(StellarAddress::Type::ACCOUNT));
        QCOMPARE(a.getBytes().size(), 32);
        QCOMPARE(a.toString(), g);
    }

    void testMuxedAccountRoundTrip()
    {
        const QString m = "MA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVAAAAAAAAAAAAAJLK";
        StellarAddress a = StellarAddress::fromString(m);
        QCOMPARE(static_cast<int>(a.getType()),
                 static_cast<int>(StellarAddress::Type::MUXED_ACCOUNT));
        QCOMPARE(a.getBytes().size(), 40); // 32 ed25519 + 8 muxed id (BE)
        QCOMPARE(a.toString(), m);
    }

    void testContractRoundTrip()
    {
        QByteArray contractId(32, '\0');
        for (int i = 0; i < 32; ++i) contractId[i] = static_cast<char>(i * 11 + 5);
        QString c = StrKey::encodeContract(contractId);

        StellarAddress a = StellarAddress::fromString(c);
        QCOMPARE(static_cast<int>(a.getType()),
                 static_cast<int>(StellarAddress::Type::CONTRACT));
        QCOMPARE(a.getBytes(), contractId);
        QCOMPARE(a.toString(), c);
    }

    void testClaimableBalanceRoundTrip()
    {
        QByteArray body;
        body.append(static_cast<char>(0)); // V0 discriminant
        for (int i = 0; i < 32; ++i) body.append(static_cast<char>(0xC0 ^ i));
        QString b = StrKey::encodeClaimableBalance(body);

        StellarAddress a = StellarAddress::fromString(b);
        QCOMPARE(static_cast<int>(a.getType()),
                 static_cast<int>(StellarAddress::Type::CLAIMABLE_BALANCE));
        QCOMPARE(a.getBytes().size(), 33);
        QCOMPARE(a.getBytes(), body);
        QCOMPARE(a.toString(), b);
    }

    void testLiquidityPoolRoundTrip()
    {
        QByteArray poolId(32, '\xAB');
        QString l = StrKey::encodeLiquidityPool(poolId);

        StellarAddress a = StellarAddress::fromString(l);
        QCOMPARE(static_cast<int>(a.getType()),
                 static_cast<int>(StellarAddress::Type::LIQUIDITY_POOL));
        QCOMPARE(a.getBytes(), poolId);
        QCOMPARE(a.toString(), l);
    }

    void testRejectsUnsupportedPrefix()
    {
        // S (seed) is intentionally not exposed by StellarAddress.
        bool threw = false;
        try {
            StellarAddress::fromString("SDJHRQF4GCMIIKAAAQ6IHY42X73FQFLHUULAPSKKD4DFDM7UXWWCRHBE");
        } catch (const std::exception&) {
            threw = true;
        }
        QVERIFY(threw);
    }

    void testEqualityByTypeAndBytes()
    {
        const QString g = "GA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVSGZ";
        StellarAddress a = StellarAddress::fromString(g);
        StellarAddress b = StellarAddress::fromString(g);
        QVERIFY(a == b);

        StellarAddress c(StellarAddress::Type::CONTRACT, a.getBytes()); // same bytes, different type
        QVERIFY(a != c);
    }
};

ADD_TEST(StellarAddressTest)
#endif // STELLARADDRESSTEST_H
