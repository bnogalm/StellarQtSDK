#ifndef SIGNERKEYTEST_H
#define SIGNERKEYTEST_H

#include <QObject>
#include <QtTest>
#include "testcollector.h"

#include "../src/signerkey.h"
#include "../src/strkey.h"
#include "../src/keypair.h"

/** Tests for CAP-40 signed payload signer + StrKey P + unified SignerKey. */
class SignerKeyTest: public QObject
{
    Q_OBJECT
private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    // ─── StrKey P-strkey ─────────────────────────────────────────────────

    void testEncodeDecodePStrKey()
    {
        const QString accountId = "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG";
        QByteArray ed25519 = StrKey::decodeStellarAccountId(accountId);
        QCOMPARE(ed25519.size(), 32);

        QByteArray payload;
        for (int i = 0; i < 32; ++i) payload.append(static_cast<char>(i + 1));

        QString p = StrKey::encodeSignedPayload(ed25519, payload);
        QCOMPARE(p.at(0), QChar('P'));

        auto pair = StrKey::decodeSignedPayload(p);
        QCOMPARE(pair.first, ed25519);
        QCOMPARE(pair.second, payload);
    }

    void testPStrKeyHandlesShortAndMaxPayload()
    {
        QByteArray ed25519 = StrKey::decodeStellarAccountId(
            "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");

        // 1-byte payload (minimum) — pads to 4 bytes.
        QByteArray p1(1, '\xAB');
        QString s1 = StrKey::encodeSignedPayload(ed25519, p1);
        QCOMPARE(StrKey::decodeSignedPayload(s1).second, p1);

        // 64-byte payload (maximum) — no padding needed.
        QByteArray p64(64, '\xCD');
        QString s64 = StrKey::encodeSignedPayload(ed25519, p64);
        QCOMPARE(StrKey::decodeSignedPayload(s64).second, p64);
    }

    void testPStrKeyRejectsOverflowPayload()
    {
        QByteArray ed25519 = StrKey::decodeStellarAccountId(
            "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        bool threw = false;
        try {
            StrKey::encodeSignedPayload(ed25519, QByteArray(65, '\0'));
        } catch (const std::exception&) {
            threw = true;
        }
        QVERIFY(threw);
    }

    void testDecodeVersionByteRecognisesP()
    {
        QByteArray ed25519 = StrKey::decodeStellarAccountId(
            "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        QString p = StrKey::encodeSignedPayload(ed25519, QByteArray(4, '\xAA'));
        QCOMPARE(static_cast<int>(StrKey::decodeVersionByte(p)),
                 static_cast<int>(StrKey::VersionByte::SIGNED_PAYLOAD));
    }

    // ─── SignerKey unified class ─────────────────────────────────────────

    void testSignerKeyEd25519RoundTrip()
    {
        const QString accountId = "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG";
        SignerKey k = SignerKey::ed25519PublicKey(accountId);
        QCOMPARE(static_cast<int>(k.getType()),
                 static_cast<int>(stellar::SignerKeyType::SIGNER_KEY_TYPE_ED25519));
        QCOMPARE(k.toString(), accountId);

        SignerKey roundtrip = SignerKey::fromString(k.toString());
        QVERIFY(k == roundtrip);
    }

    void testSignerKeyPreAuthTxRoundTrip()
    {
        QByteArray hash(32, '\xAA');
        SignerKey k = SignerKey::preAuthTx(hash);
        QCOMPARE(static_cast<int>(k.getType()),
                 static_cast<int>(stellar::SignerKeyType::SIGNER_KEY_TYPE_PRE_AUTH_TX));
        QCOMPARE(k.toString().at(0), QChar('T'));

        SignerKey rt = SignerKey::fromString(k.toString());
        QVERIFY(k == rt);
        QCOMPARE(rt.getPreAuthTx(), hash);
    }

    void testSignerKeyHashXRoundTrip()
    {
        QByteArray hash(32, '\xBB');
        SignerKey k = SignerKey::sha256Hash(hash);
        QCOMPARE(static_cast<int>(k.getType()),
                 static_cast<int>(stellar::SignerKeyType::SIGNER_KEY_TYPE_HASH_X));
        QCOMPARE(k.toString().at(0), QChar('X'));

        SignerKey rt = SignerKey::fromString(k.toString());
        QVERIFY(k == rt);
        QCOMPARE(rt.getHashX(), hash);
    }

    void testSignerKeyEd25519SignedPayloadRoundTrip()
    {
        const QString accountId = "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG";
        QByteArray payload;
        for (int i = 0; i < 7; ++i) payload.append(static_cast<char>(i * 17 + 3));

        SignerKey k = SignerKey::ed25519SignedPayload(accountId, payload);
        QCOMPARE(static_cast<int>(k.getType()),
                 static_cast<int>(stellar::SignerKeyType::SIGNER_KEY_TYPE_ED25519_SIGNED_PAYLOAD));
        QCOMPARE(k.toString().at(0), QChar('P'));
        QCOMPARE(k.getSignedPayload(), payload);

        SignerKey rt = SignerKey::fromString(k.toString());
        QVERIFY(k == rt);
        QCOMPARE(rt.getEd25519(),
                 StrKey::decodeStellarAccountId(accountId));
        QCOMPARE(rt.getSignedPayload(), payload);
    }

    void testSignerKeyXdrRoundTripForSignedPayload()
    {
        SignerKey k = SignerKey::ed25519SignedPayload(
            "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG",
            QByteArray(13, '\x42'));

        stellar::SignerKey xdr = k.toXdr();
        QByteArray bytes;
        { QDataStream s(&bytes, QIODevice::WriteOnly); s << xdr; }

        stellar::SignerKey parsed;
        { QDataStream s(&bytes, QIODevice::ReadOnly); s >> parsed; }

        QVERIFY(parsed == xdr);
        SignerKey wrap = SignerKey::fromXdr(parsed);
        QCOMPARE(wrap.getSignedPayload(), k.getSignedPayload());
        QCOMPARE(wrap.toString(), k.toString());
    }

    void testSignerKeyRejectsEmptyPayload()
    {
        bool threw = false;
        try {
            SignerKey::ed25519SignedPayload(
                "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG",
                QByteArray());
        } catch (const std::exception&) {
            threw = true;
        }
        QVERIFY(threw);
    }
};

ADD_TEST(SignerKeyTest)
#endif // SIGNERKEYTEST_H
