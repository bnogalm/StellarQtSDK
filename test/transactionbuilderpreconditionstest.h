#ifndef TRANSACTIONBUILDERPRECONDITIONSTEST_H
#define TRANSACTIONBUILDERPRECONDITIONSTEST_H

#include <QObject>
#include <QtTest>
#include "testcollector.h"

#include "../src/transactionbuilder.h"
#include "../src/transaction.h"
#include "../src/account.h"
#include "../src/createaccountoperation.h"
#include "../src/keypair.h"
#include "../src/network.h"
#include "../src/timebounds.h"
#include "../src/ledgerbounds.h"
#include "../src/signerkey.h"

/** Tests for the CAP-21 setters wired into TransactionBuilder (0.7.0 Iter 3). */
class TransactionBuilderPreconditionsTest: public QObject
{
    Q_OBJECT
private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    static KeyPair* sourceKeyPair()
    {
        return KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));
    }

    static TransactionBuilder makeBuilder(Account*& acc)
    {
        acc = new Account(sourceKeyPair(), 2908908335136768L);
        TransactionBuilder b(acc, Network::testnetNetwork());
        b.addOperation(new CreateAccountOperation(
            KeyPair::fromAccountId("GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR"),
            "1000"));
        b.setBaseFee(100);
        return b;
    }

    /** Round-trip a transaction with PRECOND_V2 through V1 envelope serialization. */
    void testBuilderEmitsV2WhenSettersUsed()
    {
        Account* acc = nullptr;
        TransactionBuilder b = makeBuilder(acc);
        b.addTimeBounds(new TimeBounds(100, 200));
        b.setLedgerBounds(new LedgerBounds(10, 50));
        b.setMinSeqNumber(42);
        b.setMinSeqAge(60);
        b.setMinSeqLedgerGap(3);
        b.addExtraSigner(SignerKey::ed25519PublicKey(
            "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"));

        QScopedPointer<Transaction> tx(b.build());
        QVERIFY(tx->getPreconditions().hasV2());

        stellar::Transaction xdr = tx->toV1Xdr();
        QCOMPARE(static_cast<int>(xdr.cond.type),
                 static_cast<int>(stellar::PreconditionType::PRECOND_V2));
        QCOMPARE(xdr.cond.v2.minSeqAge, quint64(60));
        QCOMPARE(xdr.cond.v2.minSeqLedgerGap, quint32(3));
        QCOMPARE(xdr.cond.v2.extraSigners.value.size(), 1);

        delete acc;
    }

    /** A plain timeBounds-only transaction still emits PRECOND_TIME (wire-compat). */
    void testBuilderEmitsTimeForOnlyTimeBounds()
    {
        Account* acc = nullptr;
        TransactionBuilder b = makeBuilder(acc);
        b.addTimeBounds(new TimeBounds(100, 200));

        QScopedPointer<Transaction> tx(b.build());
        QVERIFY(!tx->getPreconditions().hasV2());

        stellar::Transaction xdr = tx->toV1Xdr();
        QCOMPARE(static_cast<int>(xdr.cond.type),
                 static_cast<int>(stellar::PreconditionType::PRECOND_TIME));
        QCOMPARE(xdr.cond.timeBounds.minTime, quint64(100));
        QCOMPARE(xdr.cond.timeBounds.maxTime, quint64(200));

        delete acc;
    }

    /** Transaction round-trip through V1 envelope preserves the V2 fields. */
    void testEnvelopeRoundTripPreservesV2()
    {
        Account* acc = nullptr;
        TransactionBuilder b = makeBuilder(acc);
        b.addTimeBounds(new TimeBounds(100, 200));
        b.setMinSeqAge(120);
        b.addExtraSigner(SignerKey::sha256Hash(QByteArray(32, '\x77')));

        QScopedPointer<Transaction> tx(b.build());
        stellar::TransactionV1Envelope env;
        env.tx = tx->toV1Xdr();

        QByteArray bytes;
        { QDataStream s(&bytes, QIODevice::WriteOnly); s << env; }

        stellar::TransactionV1Envelope parsed;
        { QDataStream s(&bytes, QIODevice::ReadOnly); s >> parsed; }

        QScopedPointer<Transaction> tx2(
            Transaction::fromV1EnvelopeXdr(parsed, Network::testnetNetwork()));
        QVERIFY(tx2->getPreconditions().hasV2());
        QCOMPARE(tx2->getPreconditions().getMinSeqAge(), quint64(120));
        QCOMPARE(tx2->getPreconditions().getExtraSigners().size(), 1);
        QCOMPARE(static_cast<int>(tx2->getPreconditions().getExtraSigners().at(0).getType()),
                 static_cast<int>(stellar::SignerKeyType::SIGNER_KEY_TYPE_HASH_X));

        delete acc;
    }

    // ─── CAP-40 signPayloadDecorated ─────────────────────────────────────

    void testSignPayloadDecoratedXorsHint()
    {
        QScopedPointer<KeyPair> kp(sourceKeyPair());
        QByteArray payload(8, '\0');
        // Last 4 bytes = 0x01 0x02 0x03 0x04
        payload[4] = 0x01; payload[5] = 0x02; payload[6] = 0x03; payload[7] = 0x04;

        stellar::SignatureHint base = kp->getSignatureHint();
        stellar::DecoratedSignature sig = kp->signPayloadDecorated(payload);

        QCOMPARE(quint8(sig.hint.signatureHint[0]), quint8(base.signatureHint[0] ^ 0x01));
        QCOMPARE(quint8(sig.hint.signatureHint[1]), quint8(base.signatureHint[1] ^ 0x02));
        QCOMPARE(quint8(sig.hint.signatureHint[2]), quint8(base.signatureHint[2] ^ 0x03));
        QCOMPARE(quint8(sig.hint.signatureHint[3]), quint8(base.signatureHint[3] ^ 0x04));
    }

    /** Payload shorter than 4 bytes is right-padded with zeros for the XOR. */
    void testSignPayloadDecoratedShortPayloadZeroPads()
    {
        QScopedPointer<KeyPair> kp(sourceKeyPair());
        QByteArray payload;
        payload.append(static_cast<char>(0xAA));
        payload.append(static_cast<char>(0xBB));
        // Only 2 bytes — last-2 bytes XOR; positions 2 and 3 XOR with 0.

        stellar::SignatureHint base = kp->getSignatureHint();
        stellar::DecoratedSignature sig = kp->signPayloadDecorated(payload);

        QCOMPARE(quint8(sig.hint.signatureHint[0]), quint8(base.signatureHint[0] ^ 0xAA));
        QCOMPARE(quint8(sig.hint.signatureHint[1]), quint8(base.signatureHint[1] ^ 0xBB));
        QCOMPARE(quint8(sig.hint.signatureHint[2]), quint8(base.signatureHint[2]));
        QCOMPARE(quint8(sig.hint.signatureHint[3]), quint8(base.signatureHint[3]));
    }

    /** signPayloadDecorated produces a signature equal to signDecorated for the same payload. */
    void testSignPayloadDecoratedSignatureUnaffected()
    {
        QScopedPointer<KeyPair> kp(sourceKeyPair());
        QByteArray payload(20, '\xCC');

        stellar::DecoratedSignature plain = kp->signDecorated(payload);
        stellar::DecoratedSignature withXor = kp->signPayloadDecorated(payload);

        // signature is ed25519(payload) — identical between the two methods.
        QCOMPARE(plain.signature.value, withXor.signature.value);
    }
};

ADD_TEST(TransactionBuilderPreconditionsTest)
#endif // TRANSACTIONBUILDERPRECONDITIONSTEST_H
