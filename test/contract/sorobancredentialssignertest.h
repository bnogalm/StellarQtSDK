#ifndef SOROBANCREDENTIALSSIGNERTEST_H
#define SOROBANCREDENTIALSSIGNERTEST_H

#include <QObject>
#include <QtTest>
#include "../testcollector.h"

#include "../../src/contract/sorobancredentialssigner.h"
#include "../../src/keypair.h"
#include "../../src/network.h"
#include "../../src/scval/scv.h"
#include "../../src/stellaraddress.h"
#include "../../src/util.h"

class SorobanCredentialsSignerTest: public QObject
{
    Q_OBJECT

    static KeyPair* signer() {
        return KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));
    }

    static stellar::SorobanAuthorizationEntry makeAddressEntry()
    {
        stellar::SorobanAuthorizationEntry entry;
        entry.credentials.type = stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS;
        // Address = the signer (would normally be a contract or muxed; for test
        // any well-formed SCAddress is fine).
        QByteArray pub = signer()->getPublicKey();
        entry.credentials.address.address.type = stellar::SCAddressType::SC_ADDRESS_TYPE_ACCOUNT;
        std::memcpy(entry.credentials.address.address.accountId, pub.constData(), 32);
        entry.credentials.address.nonce = qint64(0x1122334455667788LL);
        entry.credentials.address.signatureExpirationLedger = 0;
        // Default invocation: a CONTRACT_FN with empty args is fine for fixture purposes.
        entry.rootInvocation.function.type =
            stellar::SorobanAuthorizedFunctionType::SOROBAN_AUTHORIZED_FUNCTION_TYPE_CONTRACT_FN;
        return entry;
    }

private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    void testSourceAccountEntryReturnedUnchanged()
    {
        stellar::SorobanAuthorizationEntry entry;
        entry.credentials.type = stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_SOURCE_ACCOUNT;
        auto out = SorobanCredentialsSigner::authorizeEntry(
            entry, signer(), 1234, Network::testnetNetwork()->getNetworkPassphrase());
        QCOMPARE(out.credentials.type,
                 stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_SOURCE_ACCOUNT);
    }

    void testAddressEntryGetsStampedAndSigned()
    {
        auto entry = makeAddressEntry();
        const quint32 validUntil = 99999;
        auto signed_ = SorobanCredentialsSigner::authorizeEntry(
            entry, signer(), validUntil, Network::testnetNetwork()->getNetworkPassphrase());

        QCOMPARE(signed_.credentials.address.signatureExpirationLedger, validUntil);
        // Signature is now an SCV_VEC of one SCV_MAP{public_key, signature}.
        QCOMPARE(signed_.credentials.address.signature.type, stellar::SCValType::SCV_VEC);
        auto vec = Scv::fromVec(signed_.credentials.address.signature);
        QCOMPARE(vec.size(), 1);
        auto entries = Scv::fromMap(vec.first());
        QCOMPARE(entries.size(), 2);

        // Find public_key and signature entries and verify their lengths.
        QByteArray pub, sig;
        for (const auto& e : entries) {
            const QString k = QString::fromUtf8(e.key.symbol);
            if (k == QLatin1String("public_key")) pub = Scv::fromBytes(e.val);
            else if (k == QLatin1String("signature")) sig = Scv::fromBytes(e.val);
        }
        QCOMPARE(pub.size(), 32);
        QCOMPARE(sig.size(), 64);
        QCOMPARE(pub, signer()->getPublicKey());

        // The signature must verify against the same payload the signer used.
        const QByteArray payload = SorobanCredentialsSigner::hashedSignaturePayload(
            signed_, validUntil, Network::testnetNetwork()->getNetworkPassphrase());
        QVERIFY(signer()->verify(payload, sig));
    }

    void testPayloadDependsOnNetworkAndExpiration()
    {
        auto entry = makeAddressEntry();
        const QString testnet = Network::testnetNetwork()->getNetworkPassphrase();
        const QString mainnet = Network::publicNetwork()->getNetworkPassphrase();

        QByteArray p1 = SorobanCredentialsSigner::hashedSignaturePayload(entry, 100, testnet);
        QByteArray p2 = SorobanCredentialsSigner::hashedSignaturePayload(entry, 100, mainnet);
        QByteArray p3 = SorobanCredentialsSigner::hashedSignaturePayload(entry, 200, testnet);
        QCOMPARE(p1.size(), 32);
        QVERIFY(p1 != p2);   // network sensitivity
        QVERIFY(p1 != p3);   // expiration sensitivity
    }

    void testBuildSignatureSCValShape()
    {
        QByteArray pub(32, char(0xAA));
        QByteArray sig(64, char(0xBB));
        auto v = SorobanCredentialsSigner::buildSignatureSCVal(pub, sig);
        QCOMPARE(v.type, stellar::SCValType::SCV_VEC);
        auto vec = Scv::fromVec(v);
        QCOMPARE(vec.size(), 1);
        auto map = Scv::fromMap(vec.first());
        QCOMPARE(map.size(), 2);
        // Each value is SCV_BYTES.
        for (const auto& e : map) {
            QCOMPARE(e.val.type, stellar::SCValType::SCV_BYTES);
        }
    }

    void testRejectsBadPublicKeyLength()
    {
        bool threw = false;
        try {
            SorobanCredentialsSigner::buildSignatureSCVal(QByteArray(31, '\0'), QByteArray(64, '\0'));
        } catch (const std::exception&) { threw = true; }
        QVERIFY(threw);
    }

    void testRejectsBadSignatureLength()
    {
        bool threw = false;
        try {
            SorobanCredentialsSigner::buildSignatureSCVal(QByteArray(32, '\0'), QByteArray(63, '\0'));
        } catch (const std::exception&) { threw = true; }
        QVERIFY(threw);
    }
};

ADD_TEST(SorobanCredentialsSignerTest)
#endif // SOROBANCREDENTIALSSIGNERTEST_H
