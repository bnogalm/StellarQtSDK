#ifndef MESSAGEAUTHTEST_H
#define MESSAGEAUTHTEST_H

#include <QObject>
#include <QtTest>
#include "testcollector.h"

#include "../src/messageauth.h"
#include "../src/keypair.h"

/** Tests for SEP-53 (MessageAuth::signMessage / MessageAuth::verifyMessage). */
class MessageAuthTest: public QObject
{
    Q_OBJECT
private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    void testSignAndVerifyRoundTrip()
    {
        QScopedPointer<KeyPair> kp(KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK")));
        QByteArray msg = "hello stellar";

        QByteArray sig = MessageAuth::signMessage(msg, kp.data());
        QCOMPARE(sig.size(), 64);
        QVERIFY(MessageAuth::verifyMessage(kp->getAccountId(), msg, sig));
    }

    void testVerifyFailsOnTamperedMessage()
    {
        QScopedPointer<KeyPair> kp(KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK")));
        QByteArray msg = "hello stellar";
        QByteArray sig = MessageAuth::signMessage(msg, kp.data());

        QByteArray tampered = "hello stellar.";
        QVERIFY(!MessageAuth::verifyMessage(kp->getAccountId(), tampered, sig));
    }

    void testVerifyFailsOnDifferentSigner()
    {
        QScopedPointer<KeyPair> signer(KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK")));
        QScopedPointer<KeyPair> other(KeyPair::fromSecretSeed(QString("SDHZGHURAYXKU2KMVHPOXI6JG2Q4BSQUQCEOY72O3QQTCLR2T455PMII")));

        QByteArray msg = "test";
        QByteArray sig = MessageAuth::signMessage(msg, signer.data());

        // Same message, signature was made with `signer`, verifying against
        // `other`'s account id must fail.
        QVERIFY(!MessageAuth::verifyMessage(other->getAccountId(), msg, sig));
    }

    /** SEP-53 domain separation: the same bytes signed as a raw message vs.
     *  via MessageAuth::signMessage produce DIFFERENT signatures, because
     *  MessageAuth hashes with the "Stellar Signed Message:\n" prefix first. */
    void testDomainSeparationDiffersFromRawSign()
    {
        QScopedPointer<KeyPair> kp(KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK")));
        QByteArray msg = "test";

        QByteArray rawSig = kp->sign(msg);                                 // no domain prefix
        QByteArray sep53Sig = MessageAuth::signMessage(msg, kp.data());    // hashed with prefix

        QVERIFY(rawSig != sep53Sig);
    }

    void testEmptyMessageRoundTrip()
    {
        QScopedPointer<KeyPair> kp(KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK")));
        QByteArray sig = MessageAuth::signMessage(QByteArray(), kp.data());
        QVERIFY(MessageAuth::verifyMessage(kp->getAccountId(), QByteArray(), sig));
    }

    /** messageHash is deterministic. */
    void testMessageHashDeterministic()
    {
        QByteArray h1 = MessageAuth::messageHash("abc");
        QByteArray h2 = MessageAuth::messageHash("abc");
        QCOMPARE(h1, h2);
        QCOMPARE(h1.size(), 32); // SHA-256
    }
};

ADD_TEST(MessageAuthTest)
#endif // MESSAGEAUTHTEST_H
