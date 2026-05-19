#ifndef NETWORKTEST_H
#define NETWORKTEST_H

#include <QObject>

#include <QtTest>
#include "testcollector.h"
#include "../src/network.h"
#include "../src/util.h"

// Tests below intentionally exercise deprecated APIs (they verify the
// legacy alias/singleton remains callable). Silence C4996 locally.
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4996)
#endif
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

class NetworkTest: public QObject
{
    Q_OBJECT


private slots:

    void initTestCase() {}
    void cleanupTestCase() {}

    // ============ New API (0.4.0) ============

    /** Public constructor with explicit passphrase. */
    void testPublicConstructorWithPassphrase()
    {
        Network n(QStringLiteral("Custom Net ; 2026"));
        QCOMPARE(n.getNetworkPassphrase(), QStringLiteral("Custom Net ; 2026"));
        QVERIFY(!n.isPublicNetwork());
        QVERIFY(!n.isTestNetwork());
    }

    /** Public constants for the two canonical passphrases. */
    void testPassphraseConstants()
    {
        QCOMPARE(Network::PUBLIC_NETWORK_PASSPHRASE(),
                 QStringLiteral("Public Global Stellar Network ; September 2015"));
        QCOMPARE(Network::TESTNET_NETWORK_PASSPHRASE(),
                 QStringLiteral("Test SDF Network ; September 2015"));
    }

    /** Factories `publicNetwork()` / `testnetNetwork()` don't touch the singleton. */
    void testFactoriesNonGlobal()
    {
        Network* pub = Network::publicNetwork();
        QVERIFY(pub != nullptr);
        QVERIFY(pub->isPublicNetwork());
        QVERIFY(!pub->isTestNetwork());

        Network* test = Network::testnetNetwork();
        QVERIFY(test != nullptr);
        QVERIFY(test->isTestNetwork());
        QVERIFY(!test->isPublicNetwork());

        // Factories return the same instance on repeated calls.
        QVERIFY(Network::publicNetwork() == pub);
        QVERIFY(Network::testnetNetwork() == test);
    }

    /** getNetworkId() uses this instance's passphrase (FIX §6.1). */
    void testGetNetworkIdIsPerInstance()
    {
        Network test(QStringLiteral("Test SDF Network ; September 2015"));
        Network custom(QStringLiteral("Some Custom Net"));

        QByteArray testId = test.getNetworkId();
        QByteArray customId = custom.getNetworkId();

        QVERIFY2(testId != customId,
                 "two Networks with different passphrases must yield different ids");
        QCOMPARE(testId, Util::hash(QByteArray("Test SDF Network ; September 2015")));
        QCOMPARE(customId, Util::hash(QByteArray("Some Custom Net")));
    }

    // ============ Legacy API (deprecated — must remain callable) ============

    void testNoDefaultNetwork() {
        Network::use(nullptr);
        try {
            QVERIFY(Network::current() == nullptr);
        } catch (const std::runtime_error& e) {
            QCOMPARE(e.what(), "Not selected network");
        }
    }

    void testSwitchToTestNetwork() {
        Network::useTestNetwork();
        QCOMPARE(Network::current()->getNetworkPassphrase(),
                 QStringLiteral("Test SDF Network ; September 2015"));
    }

    void testSwitchToPublicNetwork() {
        Network::usePublicNetwork();
        QCOMPARE(Network::current()->getNetworkPassphrase(),
                 QStringLiteral("Public Global Stellar Network ; September 2015"));
    }
};

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif
#ifdef _MSC_VER
#pragma warning(pop)
#endif

ADD_TEST(NetworkTest)
#endif // NETWORKTEST_H
