#ifndef AUTHTEST_H
#define AUTHTEST_H

#include <QObject>
#include <QtTest>
#include "testcollector.h"

#include "../src/auth.h"
#include "../src/keypair.h"

/** Tests for SEP-53 (Auth::signMessage / Auth::verifyMessage). */
class AuthTest: public QObject
{
    Q_OBJECT
private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    void testSignAndVerifyRoundTrip()
    {
        QScopedPointer<KeyPair> kp(KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK")));
        QByteArray msg = "hello stellar";

        QByteArray sig = Auth::signMessage(msg, kp.data());
        QCOMPARE(sig.size(), 64);
        QVERIFY(Auth::verifyMessage(kp->getAccountId(), msg, sig));
    }

    void testVerifyFailsOnTamperedMessage()
    {
        QScopedPointer<KeyPair> kp(KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK")));
        QByteArray msg = "hello stellar";
        QByteArray sig = Auth::signMessage(msg, kp.data());

        QByteArray tampered = "hello stellar.";
        QVERIFY(!Auth::verifyMessage(kp->getAccountId(), tampered, sig));
    }

    void testVerifyFailsOnDifferentSigner()
    {
        QScopedPointer<KeyPair> signer(KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK")));
        QScopedPointer<KeyPair> other(KeyPair::fromSecretSeed(QString("SDHZGHURAYXKU2KMVHPOXI6JG2Q4BSQUQCEOY72O3QQTCLR2T455PMII")));

        QByteArray msg = "test";
        QByteArray sig = Auth::signMessage(msg, signer.data());

        // Same message, signature was made with `signer`, verifying against
        // `other`'s account id must fail.
        QVERIFY(!Auth::verifyMessage(other->getAccountId(), msg, sig));
    }

    /** SEP-53 domain separation: the same bytes signed as a raw message vs.
     *  via Auth::signMessage produce DIFFERENT signatures, because Auth
     *  hashes with the "Stellar Signed Message:\n" prefix first. */
    void testDomainSeparationDiffersFromRawSign()
    {
        QScopedPointer<KeyPair> kp(KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK")));
        QByteArray msg = "test";

        QByteArray rawSig = kp->sign(msg);                 // no domain prefix
        QByteArray sep53Sig = Auth::signMessage(msg, kp.data()); // hashed with prefix

        QVERIFY(rawSig != sep53Sig);
    }

    void testEmptyMessageRoundTrip()
    {
        QScopedPointer<KeyPair> kp(KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK")));
        QByteArray sig = Auth::signMessage(QByteArray(), kp.data());
        QVERIFY(Auth::verifyMessage(kp->getAccountId(), QByteArray(), sig));
    }

    /** messageHash is deterministic. */
    void testMessageHashDeterministic()
    {
        QByteArray h1 = Auth::messageHash("abc");
        QByteArray h2 = Auth::messageHash("abc");
        QCOMPARE(h1, h2);
        QCOMPARE(h1.size(), 32); // SHA-256
    }
};

ADD_TEST(AuthTest)
#endif // AUTHTEST_H
