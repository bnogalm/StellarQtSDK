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

    /** hashedSignaturePayload now requires the entry to already carry the
     *  expiration it is being hashed for (Core rebuilds the preimage from that
     *  field). authorizeEntry stamps it internally; direct callers must too. */
    static stellar::SorobanAuthorizationEntry stamped(
        stellar::SorobanAuthorizationEntry e, quint32 ledger)
    {
        e.credentials.address.signatureExpirationLedger = ledger;
        return e;
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

        QByteArray p1 = SorobanCredentialsSigner::hashedSignaturePayload(stamped(entry, 100), 100, testnet);
        QByteArray p2 = SorobanCredentialsSigner::hashedSignaturePayload(stamped(entry, 100), 100, mainnet);
        QByteArray p3 = SorobanCredentialsSigner::hashedSignaturePayload(stamped(entry, 200), 200, testnet);
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

    // ─── CAP-71 / Protocol 27: ADDRESS_V2 ────────────────────────────

    /** V2's whole point is that the signer's address is bound into the
     *  preimage, so the same entry must hash differently under V2 — otherwise
     *  we would be emitting a V1 signature under a V2 discriminant. Checked
     *  against a preimage assembled by hand from the canonical layout
     *  (type, networkID, nonce, expirationLedger, address, invocation) rather
     *  than against our own helper. */
    void testAddressV2SignaturePayloadBindsTheAddress()
    {
        stellar::SorobanAuthorizationEntry v1 = makeAddressEntry();
        stellar::SorobanAuthorizationEntry v2 = makeAddressEntry();
        v2.credentials.type = stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS_V2;

        const QString net = Network::testnetNetwork()->getNetworkPassphrase();
        v1 = stamped(v1, 4321);
        v2 = stamped(v2, 4321);
        const QByteArray h1 = SorobanCredentialsSigner::hashedSignaturePayload(v1, 4321, net);
        const QByteArray h2 = SorobanCredentialsSigner::hashedSignaturePayload(v2, 4321, net);
        QCOMPARE(h1.size(), 32);
        QCOMPARE(h2.size(), 32);
        QVERIFY(h1 != h2);

        QByteArray expected;
        {
            const QByteArray networkId = Util::hash(net.toUtf8());
            QDataStream s(&expected, QIODevice::WriteOnly);
            s << stellar::EnvelopeType::ENVELOPE_TYPE_SOROBAN_AUTHORIZATION_WITH_ADDRESS;
            s.writeRawData(networkId.constData(), 32);
            s << v2.credentials.address.nonce;
            s << quint32(4321);
            s << v2.credentials.address.address;
            s << v2.rootInvocation;
        }
        QCOMPARE(h2, Util::hash(expected));
    }

    void testAuthorizeEntrySignsAddressV2()
    {
        stellar::SorobanAuthorizationEntry entry = makeAddressEntry();
        entry.credentials.type = stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS_V2;
        QScopedPointer<KeyPair> kp(signer());

        stellar::SorobanAuthorizationEntry signed_ = SorobanCredentialsSigner::authorizeEntry(
            entry, kp.data(), 4321, Network::testnetNetwork()->getNetworkPassphrase());

        QCOMPARE(static_cast<int>(signed_.credentials.type),
                 static_cast<int>(stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS_V2));
        QCOMPARE(signed_.credentials.address.signatureExpirationLedger, quint32(4321));
        QCOMPARE(signed_.credentials.address.signature.type, stellar::SCValType::SCV_VEC);

        // Verify the bytes really were signed over the V2 preimage. Without
        // this the test would still pass if authorizeEntry regressed to hashing
        // a V1-shaped payload for a V2 entry.
        auto vec = Scv::fromVec(signed_.credentials.address.signature);
        QCOMPARE(vec.size(), 1);
        auto map = Scv::fromMap(vec.first());
        QCOMPARE(map.size(), 2);
        // Look the entry up by key: an SCMap is sorted by encoded key, and the
        // length prefix sorts first, so "signature" (9 bytes) comes before
        // "public_key" (10) — indexing positionally would pick the wrong one.
        QByteArray sig;
        for (const auto& e : map) {
            if (QString::fromUtf8(e.key.symbol) == QLatin1String("signature"))
                sig = Scv::fromBytes(e.val);
        }
        QCOMPARE(sig.size(), 64);
        const QByteArray v2Payload = SorobanCredentialsSigner::hashedSignaturePayload(
            signed_, 4321, Network::testnetNetwork()->getNetworkPassphrase());
        QVERIFY(kp->verify(v2Payload, sig));
    }

    /** Signing an expiration the entry does not carry produces a signature Core
     *  rejects, with nothing local to catch it. The builder must refuse. */
    void testPayloadRejectsExpirationMismatch()
    {
        stellar::SorobanAuthorizationEntry e = makeAddressEntry();   // expiration 0
        bool threw = false;
        try {
            SorobanCredentialsSigner::hashedSignaturePayload(
                e, 4321, Network::testnetNetwork()->getNetworkPassphrase());
        } catch (const std::exception&) { threw = true; }
        QVERIFY(threw);
    }

    /** A delegate chain cannot be signed by one keypair; returning the entry
     *  unsigned would look like success and only fail later at simulate. */
    void testAuthorizeEntryRejectsDelegates()
    {
        stellar::SorobanAuthorizationEntry entry = makeAddressEntry();
        entry.credentials.type =
            stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS_WITH_DELEGATES;
        QScopedPointer<KeyPair> kp(signer());
        bool threw = false;
        try {
            SorobanCredentialsSigner::authorizeEntry(
                entry, kp.data(), 4321, Network::testnetNetwork()->getNetworkPassphrase());
        } catch (const std::exception&) { threw = true; }
        QVERIFY(threw);
    }
};

ADD_TEST(SorobanCredentialsSignerTest)
#endif // SOROBANCREDENTIALSSIGNERTEST_H
