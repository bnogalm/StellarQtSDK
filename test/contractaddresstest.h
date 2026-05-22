#ifndef CONTRACTADDRESSTEST_H
#define CONTRACTADDRESSTEST_H

#include <QObject>
#include <QtTest>
#include "testcollector.h"

#include "../src/util.h"
#include "../src/asset.h"
#include "../src/assettypenative.h"
#include "../src/stellaraddress.h"
#include "../src/scval/scv.h"
#include "../src/strkey.h"
#include "../src/network.h"

/**
 * CAP-46-7 contract address derivation:
 *   - Util::getContractAddress(networkPassphrase, fromAddress, salt)
 *   - Util::getContractAddressFromAsset(networkPassphrase, asset)
 *   - Asset::toSAC / Asset::toSACAddress
 *
 * Cross-checks against the well-known native-SAC fixture for testnet /
 * mainnet, plus property-based tests (deterministic, salt-sensitive,
 * network-sensitive).
 */
class ContractAddressTest: public QObject
{
    Q_OBJECT
private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    static QString testnetPassphrase()
    {
        return Network::TESTNET_NETWORK_PASSPHRASE();
    }
    static QString publicPassphrase()
    {
        return Network::PUBLIC_NETWORK_PASSPHRASE();
    }

    // ─── Util::getContractAddress (FROM_ADDRESS) ─────────────────────

    void testGetContractAddressReturns32Bytes()
    {
        stellar::SCVal addrVal = Scv::toAddress(QString(
            "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"));
        QByteArray salt(32, '\x42');
        QByteArray id = Util::getContractAddress(testnetPassphrase(), addrVal.address, salt);
        QCOMPARE(id.size(), 32);
    }

    void testGetContractAddressDeterministic()
    {
        stellar::SCVal addrVal = Scv::toAddress(QString(
            "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"));
        QByteArray salt(32, '\x42');
        QByteArray a = Util::getContractAddress(testnetPassphrase(), addrVal.address, salt);
        QByteArray b = Util::getContractAddress(testnetPassphrase(), addrVal.address, salt);
        QCOMPARE(a, b);
    }

    void testGetContractAddressSaltSensitive()
    {
        stellar::SCVal addrVal = Scv::toAddress(QString(
            "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"));
        QByteArray a = Util::getContractAddress(testnetPassphrase(), addrVal.address, QByteArray(32, '\x00'));
        QByteArray b = Util::getContractAddress(testnetPassphrase(), addrVal.address, QByteArray(32, '\x01'));
        QVERIFY(a != b);
    }

    void testGetContractAddressNetworkSensitive()
    {
        stellar::SCVal addrVal = Scv::toAddress(QString(
            "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"));
        QByteArray salt(32, '\x42');
        QByteArray a = Util::getContractAddress(testnetPassphrase(), addrVal.address, salt);
        QByteArray b = Util::getContractAddress(publicPassphrase(), addrVal.address, salt);
        QVERIFY(a != b);
    }

    void testRejectsBadSaltLength()
    {
        stellar::SCVal addrVal = Scv::toAddress(QString(
            "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"));
        bool threw = false;
        try {
            Util::getContractAddress(testnetPassphrase(), addrVal.address, QByteArray(31, '\0'));
        } catch (const std::exception&) {
            threw = true;
        }
        QVERIFY(threw);
    }

    // ─── Util::getContractAddressFromAsset (FROM_ASSET) ──────────────

    void testNativeAssetSACTestnetMatchesCanonical()
    {
        AssetTypeNative native;
        QString sacStrkey = StrKey::encodeContract(native.toSAC(testnetPassphrase()));
        // Canonical testnet native SAC contract id.
        QCOMPARE(sacStrkey,
                 QString("CDLZFC3SYJYDZT7K67VZ75HPJVIEUVNIXF47ZG2FB2RMQQVU2HHGCYSC"));
    }

    void testNativeAssetSACPublicMatchesCanonical()
    {
        AssetTypeNative native;
        QString sacStrkey = StrKey::encodeContract(native.toSAC(publicPassphrase()));
        // Canonical mainnet native SAC contract id.
        QCOMPARE(sacStrkey,
                 QString("CAS3J7GYLGXMF6TDJBBYYSE3HQ6BBSMLNUQ34T6TZMYMW2EVH34XOWMA"));
    }

    void testToSACAddressReturnsContractType()
    {
        AssetTypeNative native;
        StellarAddress addr = native.toSACAddress(testnetPassphrase());
        QCOMPARE(static_cast<int>(addr.getType()),
                 static_cast<int>(StellarAddress::Type::CONTRACT));
        QCOMPARE(addr.getBytes().size(), 32);
    }

    void testSACDiffersBetweenAssets()
    {
        AssetTypeNative native;
        QScopedPointer<Asset> usd(Asset::create("USD",
            "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"));

        QByteArray a = native.toSAC(testnetPassphrase());
        QByteArray b = usd->toSAC(testnetPassphrase());
        QVERIFY(a != b);
    }
};

ADD_TEST(ContractAddressTest)
#endif // CONTRACTADDRESSTEST_H
