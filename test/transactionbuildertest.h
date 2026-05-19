#ifndef TRANSACTIONBUILDERTEST_H
#define TRANSACTIONBUILDERTEST_H
#include <QObject>
#include <QtTest>
#include "testcollector.h"

#include "../src/transactionbuilder.h"
#include "../src/feebumptransactionbuilder.h"
#include "../src/transaction.h"
#include "../src/feebumptransaction.h"
#include "../src/account.h"
#include "../src/network.h"
#include "../src/keypair.h"
#include "../src/bumpsequenceoperation.h"
#include "../src/createaccountoperation.h"
#include "../src/paymentoperation.h"
#include "../src/assettypenative.h"
#include "../src/memo.h"

/**
 * Verifies that the free `TransactionBuilder` class (introduced in 0.4.0)
 * is a drop-in replacement for the nested `Transaction::Builder`, and that
 * the nested alias still compiles for existing downstream code.
 *
 * Partial port of `TransactionBuilderTest.java` from java-stellar-sdk.
 */
class TransactionBuilderTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    /** Free TransactionBuilder can be used directly. */
    void testFreeBuilderDirect()
    {
        KeyPair* source = KeyPair::fromSecretSeed(QString("SCH27VUZZ6UAKB67BDNF6FA42YMBMQCBKXWGMFD5TZ6S5ZZCZFLRXKHS"));
        Account* account = new Account(source, 2908908335136768LL);

        Transaction* tx = TransactionBuilder(AccountConverter().enableMuxed(), account, Network::testnetNetwork())
            .addOperation(BumpSequenceOperation::create(qint64(2)))
            .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
            .setBaseFee(TransactionBuilder::BASE_FEE)
            .build();

        QVERIFY(tx != nullptr);
        QCOMPARE(tx->getSequenceNumber(), qint64(2908908335136769LL));
        QCOMPARE(tx->getOperations().size(), 1);
        QCOMPARE(tx->getFee(), qint64(100));
        QCOMPARE(tx->getSourceAccount(), source->getAccountId());

        delete tx;
        delete account; // also frees the KeyPair `source` (Account owns it)
    }

    /**
     * Java-aligned constructor: TransactionBuilder(account, Network*) — no
     * AccountConverter (muxed always on).
     */
    void testFreeBuilderJavaStyle()
    {
        KeyPair* source = KeyPair::random();
        Account* account = new Account(source, 1LL);

        Transaction* tx = TransactionBuilder(account, Network::testnetNetwork())
            .addOperation(BumpSequenceOperation::create(qint64(2)))
            .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
            .setBaseFee(TransactionBuilder::BASE_FEE)
            .build();

        QVERIFY(tx != nullptr);
        QCOMPARE(tx->getOperations().size(), 1);
        delete tx;
        delete account;
    }

    /** Java-aligned constructor rejects null Network. */
    void testFreeBuilderJavaStyleNullNetwork()
    {
        KeyPair* source = KeyPair::random();
        Account* account = new Account(source, 1LL);
        bool threw = false;
        try {
            TransactionBuilder b(account, nullptr);
        } catch (const std::exception&) {
            threw = true;
        }
        QVERIFY2(threw, "TransactionBuilder(account, nullptr) must throw");
        delete account;
    }

    /** Transaction::Builder is still a valid name (deprecated alias). */
    void testNestedAliasStillWorks()
    {
        KeyPair* source = KeyPair::random();
        Account* account = new Account(source, 1LL);

        // Uses the nested name (Transaction::Builder) which is now a
        // [[deprecated]] type alias for ::TransactionBuilder. Silence the
        // warning locally — the point is that existing code still compiles.
        #ifdef _MSC_VER
        #pragma warning(push)
        #pragma warning(disable: 4996)
        #endif
        Transaction* tx = Transaction::Builder(AccountConverter().enableMuxed(), account, Network::testnetNetwork())
            .addOperation(BumpSequenceOperation::create(qint64(2)))
            .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
            .setBaseFee(Transaction::Builder::BASE_FEE)
            .build();
        #ifdef _MSC_VER
        #pragma warning(pop)
        #endif

        QVERIFY(tx != nullptr);
        QCOMPARE(tx->getOperations().size(), 1);
        delete tx;
        delete account;
    }

    /** Memo is attached correctly via the free TransactionBuilder. */
    void testFreeBuilderWithMemo()
    {
        KeyPair* source = KeyPair::random();
        Account* account = new Account(source, 1LL);

        Transaction* tx = TransactionBuilder(AccountConverter().enableMuxed(), account, Network::testnetNetwork())
            .addOperation(BumpSequenceOperation::create(qint64(2)))
            .addMemo(Memo::text("hello"))
            .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
            .setBaseFee(TransactionBuilder::BASE_FEE)
            .build();

        QVERIFY(tx->getMemo() != nullptr);
        QCOMPARE(dynamic_cast<MemoText*>(tx->getMemo())->getText(), QStringLiteral("hello"));
        delete tx;
        delete account;
    }

    /** No operations → build() must throw. */
    void testFreeBuilderNoOperations()
    {
        KeyPair* source = KeyPair::random();
        Account* account = new Account(source, 1LL);

        bool threw = false;
        try {
            Transaction* tx = TransactionBuilder(AccountConverter().enableMuxed(), account, Network::testnetNetwork())
                .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                .setBaseFee(TransactionBuilder::BASE_FEE)
                .build();
            delete tx;
        } catch (const std::exception&) {
            threw = true;
        }
        QVERIFY2(threw, "build() should throw without operations");

        delete account;
    }

    /** Free FeeBumpTransactionBuilder works. */
    void testFreeFeeBumpBuilderDirect()
    {
        KeyPair* source = KeyPair::random();
        Account* account = new Account(source, 1LL);

        Transaction* inner = TransactionBuilder(AccountConverter().enableMuxed(), account, Network::testnetNetwork())
            .addOperation(BumpSequenceOperation::create(qint64(2)))
            .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
            .setBaseFee(TransactionBuilder::BASE_FEE)
            .build();
        inner->sign(source);

        KeyPair* feeAccount = KeyPair::random();
        FeeBumpTransaction* bump = FeeBumpTransactionBuilder(AccountConverter().enableMuxed(), inner)
            .setFeeAccount(feeAccount->getAccountId())
            .setBaseFee(AbstractTransaction::MIN_BASE_FEE * 2)
            .build();

        QVERIFY(bump != nullptr);
        QCOMPARE(bump->getFeeAccount(), feeAccount->getAccountId());
        QCOMPARE(bump->getFee(), qint64(AbstractTransaction::MIN_BASE_FEE * 2 * 2)); // (numOps+1)*baseFee = 2 * 200

        delete bump; // also frees inner via destructor (§1.1 fix)
        delete feeAccount;
        delete account;
    }

    /** FeeBumpTransaction::Builder is still a valid alias. */
    void testFeeBumpNestedAliasStillWorks()
    {
        KeyPair* source = KeyPair::random();
        Account* account = new Account(source, 1LL);

        Transaction* inner = TransactionBuilder(AccountConverter().enableMuxed(), account, Network::testnetNetwork())
            .addOperation(BumpSequenceOperation::create(qint64(2)))
            .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
            .setBaseFee(TransactionBuilder::BASE_FEE)
            .build();
        inner->sign(source);

        KeyPair* feeAccount = KeyPair::random();
        #ifdef _MSC_VER
        #pragma warning(push)
        #pragma warning(disable: 4996)
        #endif
        FeeBumpTransaction* bump = FeeBumpTransaction::Builder(AccountConverter().enableMuxed(), inner)
            .setFeeAccount(feeAccount->getAccountId())
            .setBaseFee(AbstractTransaction::MIN_BASE_FEE * 2)
            .build();
        #ifdef _MSC_VER
        #pragma warning(pop)
        #endif

        QVERIFY(bump != nullptr);
        delete bump;
        delete feeAccount;
        delete account;
    }
};

ADD_TEST(TransactionBuilderTest)
#endif // TRANSACTIONBUILDERTEST_H
