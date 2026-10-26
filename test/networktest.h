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
    /**
     * The network id is SHA-256 of the passphrase's UTF-8, as core, js and py
     * hash it. It was hashed from Latin-1: a custom network whose passphrase
     * has a character outside ASCII signed under another id. The expected id
     * is js-stellar-sdk 17.1.0's hash().
     */
    void testNetworkIdIsTheUtf8Hash()
    {
        Network custom(QString::fromUtf8("Red de prueba \xc3\xb1" " ; 2026"));
        QCOMPARE(QString::fromLatin1(custom.getNetworkId().toHex()),
                 QStringLiteral("f6acc00604201e0f2307704af32c864471177c51a915326673dedd545ad195fb"));
        Network testnet(QStringLiteral("Test SDF Network ; September 2015"));
        QCOMPARE(testnet.getNetworkId(), Network::testnetNetwork()->getNetworkId());
    }

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

    /**
     * Futurenet's, a standalone core's and the local sandbox's passphrases,
     * as js Networks (FUTURENET, STANDALONE, SANDBOX) and py Network
     * (*_NETWORK_PASSPHRASE) give them in js-stellar-sdk 17.1.0 and
     * py-stellar-sdk 14.1.0. The QML layer, TxRep and SEP-10 kept their own
     * copies; the futurenet id is py's Network(...).network_id().
     */
    void testTheOtherPassphrasesJsAndPyName()
    {
        QCOMPARE(Network::FUTURENET_NETWORK_PASSPHRASE(),
                 QStringLiteral("Test SDF Future Network ; October 2022"));
        QCOMPARE(Network::STANDALONE_NETWORK_PASSPHRASE(),
                 QStringLiteral("Standalone Network ; February 2017"));
        QCOMPARE(Network::SANDBOX_NETWORK_PASSPHRASE(),
                 QStringLiteral("Local Sandbox Stellar Network ; September 2022"));
        QCOMPARE(QString::fromLatin1(Network(Network::FUTURENET_NETWORK_PASSPHRASE()).getNetworkId().toHex()),
                 QStringLiteral("a3a1c6a78286713e29be0e9785670fa838d13917cd8eaeb4a3579ff1debc7fd5"));
        Network futurenet(Network::FUTURENET_NETWORK_PASSPHRASE());
        QVERIFY(!futurenet.isPublicNetwork());
        QVERIFY(!futurenet.isTestNetwork());
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
