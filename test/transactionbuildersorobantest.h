#ifndef TRANSACTIONBUILDERSOROBANTEST_H
#define TRANSACTIONBUILDERSOROBANTEST_H

#include <QObject>
#include <QtTest>
#include "testcollector.h"

#include "../src/transactionbuilder.h"
#include "../src/transaction.h"
#include "../src/account.h"
#include "../src/keypair.h"
#include "../src/network.h"
#include "../src/sorobandatabuilder.h"
#include "../src/invokehostfunctionoperation.h"
#include "../src/restorefootprintoperation.h"
#include "../src/extendfootprintttloperation.h"
#include "../src/scval/scv.h"
#include "../src/stellaraddress.h"
#include "../src/strkey.h"

/**
 * TransactionBuilder + Soroban data integration (CAP-46).
 * Verifies fee summation, ext.v=1 wire encoding, envelope round-trip.
 */
class TransactionBuilderSorobanTest: public QObject
{
    Q_OBJECT
private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    static KeyPair* sourceKeyPair()
    {
        return KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));
    }

    static stellar::SorobanTransactionData makeSorobanData(qint64 resourceFee)
    {
        return SorobanDataBuilder()
            .setResources(1'000'000u, 4096u, 2048u)
            .setResourceFee(resourceFee)
            .build();
    }

    static Operation* makeRestoreOp()
    {
        return RestoreFootprintOperation::create();
    }

    void testBuilderSumsResourceFee()
    {
        Account* acc = new Account(sourceKeyPair(), 41);
        TransactionBuilder b(acc, Network::testnetNetwork());
        b.addOperation(makeRestoreOp());
        b.setBaseFee(100);
        b.setTimeout(TransactionBuilder::TIMEOUT_INFINITE);
        b.setSorobanData(makeSorobanData(qint64(50'000)));

        QScopedPointer<Transaction> tx(b.build());

        // baseFee(100) * numOps(1) + resourceFee(50'000) = 50'100
        QCOMPARE(tx->getFee(), qint64(50'100));

        delete acc;
    }

    void testTransactionRetainsSorobanData()
    {
        Account* acc = new Account(sourceKeyPair(), 0);
        TransactionBuilder b(acc, Network::testnetNetwork());
        b.addOperation(makeRestoreOp());
        b.setBaseFee(100);
        b.setTimeout(TransactionBuilder::TIMEOUT_INFINITE);
        b.setSorobanData(makeSorobanData(qint64(7)));

        QScopedPointer<Transaction> tx(b.build());
        QVERIFY(!tx->getSorobanData().isNull());
        QCOMPARE(tx->getSorobanData()->resourceFee, qint64(7));

        delete acc;
    }

    void testToV1XdrEmitsExtV1WhenSorobanDataSet()
    {
        Account* acc = new Account(sourceKeyPair(), 0);
        TransactionBuilder b(acc, Network::testnetNetwork());
        b.addOperation(makeRestoreOp());
        b.setBaseFee(100);
        b.setTimeout(TransactionBuilder::TIMEOUT_INFINITE);
        b.setSorobanData(makeSorobanData(qint64(13)));

        QScopedPointer<Transaction> tx(b.build());
        stellar::Transaction xdr = tx->toV1Xdr();
        QCOMPARE(xdr.ext.v, qint32(1));
        QCOMPARE(xdr.ext.sorobanData.resourceFee, qint64(13));

        delete acc;
    }

    void testToV1XdrEmitsExtV0WhenNoSorobanData()
    {
        // Non-Soroban transaction must still encode ext.v=0 (wire-compat with
        // the legacy "Reserved" field).
        Account* acc = new Account(sourceKeyPair(), 0);
        TransactionBuilder b(acc, Network::testnetNetwork());
        b.addOperation(makeRestoreOp());
        b.setBaseFee(100);
        b.setTimeout(TransactionBuilder::TIMEOUT_INFINITE);

        QScopedPointer<Transaction> tx(b.build());
        stellar::Transaction xdr = tx->toV1Xdr();
        QCOMPARE(xdr.ext.v, qint32(0));

        delete acc;
    }

    void testEnvelopeRoundTripPreservesSorobanData()
    {
        Account* acc = new Account(sourceKeyPair(), 0);
        TransactionBuilder b(acc, Network::testnetNetwork());
        b.addOperation(makeRestoreOp());
        b.setBaseFee(100);
        b.setTimeout(TransactionBuilder::TIMEOUT_INFINITE);
        b.setSorobanData(makeSorobanData(qint64(99)));

        QScopedPointer<Transaction> tx(b.build());
        stellar::TransactionV1Envelope env;
        env.tx = tx->toV1Xdr();

        QByteArray bytes;
        { QDataStream s(&bytes, QIODevice::WriteOnly); s << env; }
        stellar::TransactionV1Envelope parsed;
        { QDataStream s(&bytes, QIODevice::ReadOnly); s >> parsed; }

        QScopedPointer<Transaction> back(
            Transaction::fromV1EnvelopeXdr(parsed, Network::testnetNetwork()));
        QVERIFY(!back->getSorobanData().isNull());
        QCOMPARE(back->getSorobanData()->resourceFee, qint64(99));
        QCOMPARE(back->getFee(), qint64(199));   // baseFee*1 + 99

        delete acc;
    }

    void testRejectsNegativeResourceFee()
    {
        Account* acc = new Account(sourceKeyPair(), 0);
        TransactionBuilder b(acc, Network::testnetNetwork());
        b.addOperation(makeRestoreOp());
        b.setBaseFee(100);
        b.setTimeout(TransactionBuilder::TIMEOUT_INFINITE);

        // Forge invalid SorobanData with negative resourceFee.
        stellar::SorobanTransactionData bad;
        bad.resourceFee = -1;
        b.setSorobanData(bad);

        bool threw = false;
        try {
            QScopedPointer<Transaction> tx(b.build());
        } catch (const std::exception&) {
            threw = true;
        }
        QVERIFY(threw);

        delete acc;
    }

    /**
     * Wire-compat: a plain (non-Soroban) transaction's serialized bytes are
     * unchanged by the addition of TransactionExt — `ext.v=0` is still
     * 4 zero bytes.
     */
    void testWireCompatForNonSorobanTransaction()
    {
        Account* acc = new Account(sourceKeyPair(), 0);
        TransactionBuilder b(acc, Network::testnetNetwork());
        b.addOperation(makeRestoreOp());
        b.setBaseFee(100);
        b.setTimeout(TransactionBuilder::TIMEOUT_INFINITE);
        QScopedPointer<Transaction> tx(b.build());

        stellar::Transaction xdr = tx->toV1Xdr();
        QByteArray bytes;
        { QDataStream s(&bytes, QIODevice::WriteOnly); s << xdr.ext; }
        // ext.v=0 + nothing = 4 zero bytes
        QCOMPARE(bytes.size(), 4);
        QCOMPARE(bytes, QByteArray("\x00\x00\x00\x00", 4));

        delete acc;
    }
};

ADD_TEST(TransactionBuilderSorobanTest)
#endif // TRANSACTIONBUILDERSOROBANTEST_H
