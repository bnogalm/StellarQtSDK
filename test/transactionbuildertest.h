#ifndef TRANSACTIONBUILDERTEST_H
#define TRANSACTIONBUILDERTEST_H
#include "../src/transactionpreconditions.h"
#include <QObject>
#include <QtTest>
#include <QScopedPointer>
#include <limits>
#include <type_traits>
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
#include "../src/timebounds.h"
#include "../src/txrep.h"

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

    /** 10 XLM to GDW6..., fee 100: the probe transaction js and py built too. */
    static Transaction* paymentOfTen(TransactionBuilder& builder)
    {
        builder.addOperation(PaymentOperation::create(
            QStringLiteral("GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR"),
            new AssetTypeNative(), QStringLiteral("10")));
        return builder.setBaseFee(100).build();
    }

private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    /**
     * An Account made from an M... address sent the transaction from its G
     * account: the builder took the source from the key pair, and the muxed
     * id was dropped without a word (js and py sign the muxed source). The
     * envelope and hash are js-stellar-sdk 17.1.0's for the same MuxedAccount.
     */
    void testMuxedAccountSourceIsSentAsMuxed()
    {
        const QString muxed = QStringLiteral("MCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6AAAAEPXD6YEZPS2W");
        Account* account = new Account(muxed, 100);
        QCOMPARE(account->getAccountId(), muxed);
        QScopedPointer<Transaction> tx(TransactionBuilder(account, Network::testnetNetwork())
            .addOperation(PaymentOperation::create(QStringLiteral("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"),
                                                   new AssetTypeNative(), QStringLiteral("10")))
            .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
            .setBaseFee(100)
            .build());
        QCOMPARE(tx->getSourceAccount(), muxed);
        QCOMPARE(tx->toEnvelopeXdrBase64(), QStringLiteral(
            "AAAAAgAAAQAAAAEfcfsEy4AhFYiyQHpQEpE1YX6+rUa8RXJOM7GklxG7rZZHm1CPAAAAZAAAAAAAAABlAAAAAQAAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAQAAAAAAAAABAAAAAD1O39Zz2qUNUKLDH6BnGEEFOs8+CC/aVz5CUmCj3euQAAAAAAAAAAAF9eEAAAAAAAAAAAA="));
        QCOMPARE(tx->hashHex(), QStringLiteral("b0399e01d1d74346e33bbaf1b35b7b72c4f1c319069a5d43c7940f93530ccb1d"));
        QCOMPARE(account->getSequenceNumber(), qint64(101));
        delete account;
    }

    /**
     * setBaseFee refused anything under 100, so the inner transaction of a
     * fee bump (fee 0: the fee source pays; 411 of 4,454 real fee bumps) could
     * not be built. js and py build it; 0 to 4294967295 is taken now, and a
     * builder given no fee still throws.
     */
    void testBaseFeeUnderTheMinimumIsForAFeeBumpsInnerTransaction()
    {
        auto build = [](qint64 fee, bool setFee) {
            Account* account = new Account(KeyPair::fromAccountId(QStringLiteral("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG")), 100);
            TransactionBuilder builder(account, Network::testnetNetwork());
            builder.addOperation(BumpSequenceOperation::create(qint64(5)))
                   .setTimeout(TransactionBuilder::TIMEOUT_INFINITE);
            if (setFee)
                builder.setBaseFee(fee);
            QScopedPointer<Transaction> tx(builder.build());
            const qint64 out = tx->getFee();
            delete account;
            return out;
        };
        QCOMPARE(build(0, true), qint64(0));
        QCOMPARE(build(1, true), qint64(1));
        QCOMPARE(build(99, true), qint64(99));
        QVERIFY_EXCEPTION_THROWN(build(-1, true), std::runtime_error);
#ifndef STELLAR_QT_AUTOSET_BASE_FEE
        QVERIFY_EXCEPTION_THROWN(build(0, false), std::runtime_error);
#endif
    }

    /**
     * setMinSeqAge(0) and setMinSeqLedgerGap(0) left a PRECOND_TIME: js and
     * py sign a PRECOND_V2 as soon as either is set, 0 included (another
     * hash). The hashes are js-stellar-sdk 17.1.0's.
     */
    void testExplicitZeroMinSeqPreconditionsAreV2()
    {
        auto hash = [](int which) {
            Account* account = new Account(KeyPair::fromAccountId(QStringLiteral("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG")), 100);
            TransactionBuilder builder(account, Network::testnetNetwork());
            builder.addOperation(BumpSequenceOperation::create(qint64(5)))
                   .setTimeout(TransactionBuilder::TIMEOUT_INFINITE).setBaseFee(100);
            if (which == 1)
                builder.setMinSeqAge(0);
            if (which == 2)
                builder.setMinSeqLedgerGap(0);
            QScopedPointer<Transaction> tx(builder.build());
            const QString out = tx->hashHex();
            delete account;
            return out;
        };
        QCOMPARE(hash(0), QStringLiteral("52372326207c81fea7e9c63322cfc5d82401c72f88ae32e626ffa30a8a7371cf"));
        QCOMPARE(hash(1), QStringLiteral("d4e5c0e178e595c61e2258d8bc99f233fa3efb0379fab0800801d81b8e78fc2e"));
        QCOMPARE(hash(2), QStringLiteral("d4e5c0e178e595c61e2258d8bc99f233fa3efb0379fab0800801d81b8e78fc2e"));
    }

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

    /**
     * The envelope's fee field is a uint32. A total above 4294967295 used to be
     * assigned into it unchecked: the transaction was signed with a truncated
     * fee while getFee() reported the full one.
     */
    void testTotalFeeMustFitTheEnvelope()
    {
        KeyPair* source = KeyPair::fromSecretSeed(QString("SCH27VUZZ6UAKB67BDNF6FA42YMBMQCBKXWGMFD5TZ6S5ZZCZFLRXKHS"));
        Account* account = new Account(source, 100);

        bool threw = false;
        try {
            TransactionBuilder builder(account, Network::testnetNetwork());
            builder.addOperation(BumpSequenceOperation::create(qint64(2)))
                   .addOperation(BumpSequenceOperation::create(qint64(3)))
                   .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                   .setBaseFee(4294967295u);
            QScopedPointer<Transaction> tx(builder.build());
        } catch (const std::exception&) {
            threw = true;
        }
        QVERIFY(threw);

        // The largest fee that fits is still allowed, and is what gets signed.
        QScopedPointer<Transaction> tx(
            TransactionBuilder(account, Network::testnetNetwork())
                .addOperation(BumpSequenceOperation::create(qint64(2)))
                .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                .setBaseFee(4294967295u)
                .build());
        QCOMPARE(tx->toV1Xdr().fee, quint32(4294967295u));

        delete account;
    }

    /**
     * setBaseFee took a quint32, so a caller holding a signed value — as
     * ContractClient::invoke (qint64) and AssembledTransaction::restoreFootprint
     * (int) do — passed -5 in as 4294967291, and it was accepted.
     */
    void testBaseFeeOutOfRangeIsRefused()
    {
        KeyPair* source = KeyPair::random();
        Account* account = new Account(source, 1LL);
        TransactionBuilder builder(account, Network::testnetNetwork());

        // 0 to 99 are fees too: a fee bump's inner transaction (see
        // testBaseFeeUnderTheMinimumIsForAFeeBumpsInnerTransaction).
        for (qint64 bad : {qint64(-5), qint64(-100000), qint64(-1),
                           qint64(4294967296LL), qint64(4294967396LL)}) {
            bool threw = false;
            try { builder.setBaseFee(bad); }
            catch (const std::exception&) { threw = true; }
            QVERIFY2(threw, qPrintable(QString::number(bad)));
        }
        builder.setBaseFee(4294967295LL);   // the largest that fits
        delete account;
    }

    /**
     * An envelope holds at most 100 operations and the XDR array dropped the
     * rest without a word: 101 payments were signed, and accepted, as 100.
     */
    void testMoreThanOneHundredOperationsIsRefused()
    {
        KeyPair* source = KeyPair::random();
        Account* account = new Account(source, 1LL);

        bool threw = false;
        try {
            TransactionBuilder builder(account, Network::testnetNetwork());
            for (int i = 0; i < 101; ++i)
                builder.addOperation(BumpSequenceOperation::create(qint64(i + 2)));
            builder.setTimeout(TransactionBuilder::TIMEOUT_INFINITE).setBaseFee(100);
            QScopedPointer<Transaction> tx(builder.build());
        } catch (const std::exception&) {
            threw = true;
        }
        QVERIFY(threw);
        QCOMPARE(account->getSequenceNumber(), qint64(1));   // not consumed

        TransactionBuilder builder(account, Network::testnetNetwork());
        for (int i = 0; i < 100; ++i)
            builder.addOperation(BumpSequenceOperation::create(qint64(i + 2)));
        builder.setTimeout(TransactionBuilder::TIMEOUT_INFINITE).setBaseFee(100);
        QScopedPointer<Transaction> tx(builder.build());
        QCOMPARE(tx->toV1Xdr().operations.value.size(), 100);
        delete account;
    }

    /** The fee-bump envelope's signature array dropped the 21st signature
     *  and beyond without a word; Transaction's already refused. */
    void testAFeeBumpWithTooManySignaturesIsRefused()
    {
        KeyPair* source = KeyPair::random();
        Account* account = new Account(source, 1LL);
        Transaction* inner = TransactionBuilder(account, Network::testnetNetwork())
            .addOperation(BumpSequenceOperation::create(qint64(2)))
            .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
            .setBaseFee(TransactionBuilder::BASE_FEE)
            .build();
        inner->sign(source);

        KeyPair* feeAccount = KeyPair::random();
        QScopedPointer<FeeBumpTransaction> bump(
            FeeBumpTransactionBuilder(AccountConverter().enableMuxed(), inner)
                .setFeeAccount(feeAccount->getAccountId())
                .setBaseFee(AbstractTransaction::MIN_BASE_FEE * 2)
                .build());
        for (int i = 0; i < 20; ++i) {
            QScopedPointer<KeyPair> k(KeyPair::random());
            bump->sign(k.data());
        }
        QVERIFY(!bump->toEnvelopeXdrBase64().isEmpty());   // 20 fit

        QScopedPointer<KeyPair> oneTooMany(KeyPair::random());
        bump->sign(oneTooMany.data());
        bool threw = false;
        try { bump->toEnvelopeXdrBase64(); }
        catch (const std::exception&) { threw = true; }
        QVERIFY(threw);

        delete feeAccount;
        delete account;
    }

    /**
     * The inner fee is compared exactly, as core does it (baseFee * ops >=
     * inner inclusion fee). A truncating division accepted 100 against a
     * 301-stroop fee over three operations, and core refused the bump.
     */
    void testFeeBumpComparesTheInnerFeeExactly()
    {
        // A 301-stroop fee over three operations: not a multiple, which is the
        // case the truncating division got wrong. A builder cannot produce
        // it, so it comes from a TxRep document, whose fee is now exact.
        KeyPair* source = KeyPair::random();
        Account* account = new Account(source, 1LL);
        TransactionBuilder builder(account, Network::testnetNetwork());
        for (int i = 0; i < 3; ++i)
            builder.addOperation(BumpSequenceOperation::create(qint64(i + 2)));
        QScopedPointer<Transaction> template_(
            builder.setTimeout(TransactionBuilder::TIMEOUT_INFINITE).setBaseFee(100).build());
        QString doc = Txrep::toTxrep(template_.data());
        QVERIFY(doc.contains(QStringLiteral("tx.fee: 300\n")));
        doc.replace(QStringLiteral("tx.fee: 300\n"), QStringLiteral("tx.fee: 301\n"));
        const auto makeInner = [&]() {
            Transaction* inner = Txrep::fromTxrep(doc, Network::testnetNetwork());
            inner->sign(source);
            return inner;
        };

        bool threw = false;
        try {
            // The builder owns the inner transaction and frees it with itself.
            FeeBumpTransactionBuilder(AccountConverter().enableMuxed(), makeInner())
                .setBaseFee(100);
        } catch (const std::exception&) {
            threw = true;
        }
        QVERIFY(threw);

        KeyPair* feeAccount = KeyPair::random();
        QScopedPointer<FeeBumpTransaction> bump(
            FeeBumpTransactionBuilder(AccountConverter().enableMuxed(), makeInner())
                .setFeeAccount(feeAccount->getAccountId())
                .setBaseFee(101)
                .build());
        QCOMPARE(bump->getFee(), qint64(101 * 4));
        delete feeAccount;
        delete account;
    }

    /**
     * With muxing off (AccountConverter(false)) the operations went out as
     * G... while the transaction source and a fee bump's fee source were
     * still written M..., and read back as G...: they go through the
     * converter now, as in java. Built from an M... address with muxing off,
     * each is the transaction built from its G... account. js and py have no
     * such switch: they write the M... form, as muxing on does here.
     */
    void testMuxingOffWritesTheSourcesAsG()
    {
        const QString muxedSource = QStringLiteral("MA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVAAAAAAAAAAAAAJLK");
        const QString source = QStringLiteral("GA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVSGZ");
        const QString muxedFee = QStringLiteral("MDQNY3PBOJOKYZSRMK2S7LHHGWZIUISD4QORETLMXEWXBI7KFZZMKAAAAAAAAAAAA5S6G");
        const QString fee = QStringLiteral("GDQNY3PBOJOKYZSRMK2S7LHHGWZIUISD4QORETLMXEWXBI7KFZZMKTL3");
        const auto build = [](const QString& from, bool muxing) {
            Account account(from, 41LL);
            TransactionBuilder builder(AccountConverter(muxing), &account, Network::testnetNetwork());
            builder.setTimeout(TransactionBuilder::TIMEOUT_INFINITE);
            return paymentOfTen(builder);
        };
        const int ed25519 = static_cast<int>(stellar::CryptoKeyType::KEY_TYPE_ED25519);
        const int muxed = static_cast<int>(stellar::CryptoKeyType::KEY_TYPE_MUXED_ED25519);

        QScopedPointer<Transaction> off(build(muxedSource, false));
        QScopedPointer<Transaction> plain(build(source, true));
        QCOMPARE(static_cast<int>(off->toEnvelopeXdr().v1.tx.sourceAccount.type), ed25519);
        QCOMPARE(off->toEnvelopeXdrBase64(), plain->toEnvelopeXdrBase64());
        QCOMPARE(off->hashHex(), plain->hashHex());

        QScopedPointer<FeeBumpTransaction> bumpOff(
            FeeBumpTransactionBuilder(AccountConverter(false), off.take())
                .setFeeAccount(muxedFee).setBaseFee(200).build());
        QScopedPointer<FeeBumpTransaction> bumpPlain(
            FeeBumpTransactionBuilder(AccountConverter(), plain.take())
                .setFeeAccount(fee).setBaseFee(200).build());
        QCOMPARE(static_cast<int>(bumpOff->toXdr().feeSource.type), ed25519);
        QCOMPARE(bumpOff->toEnvelopeXdrBase64(), bumpPlain->toEnvelopeXdrBase64());
        QCOMPARE(bumpOff->hashHex(), bumpPlain->hashHex());

        // Muxing on keeps both M..., as js and py write them.
        QScopedPointer<Transaction> on(build(muxedSource, true));
        QCOMPARE(static_cast<int>(on->toEnvelopeXdr().v1.tx.sourceAccount.type), muxed);
        QScopedPointer<FeeBumpTransaction> bumpOn(
            FeeBumpTransactionBuilder(AccountConverter(), on.take())
                .setFeeAccount(muxedFee).setBaseFee(200).build());
        QCOMPARE(static_cast<int>(bumpOn->toXdr().feeSource.type), muxed);
    }

    /** Equal time bounds are a valid window; only min > max is not. */
    void testTimeBoundsAcceptAnEqualWindow()
    {
        TimeBounds equal(100, 100);
        QCOMPARE(equal.getMaxTime(), qint64(100));
        bool threw = false;
        try { TimeBounds backwards(200, 100); Q_UNUSED(backwards) }
        catch (const std::exception&) { threw = true; }
        QVERIFY(threw);
        TimeBounds unbounded(200, 0);   // maxTime 0 means no upper bound
        QCOMPARE(unbounded.getMinTime(), qint64(200));
    }

    /**
     * A negative time, sequence age or ledger gap from the caller was signed
     * as a huge uint64: TimeBounds(-1, 0) as minTime 2^64 - 1 (never valid),
     * setMinSeqAge(-1) as 2^64 - 1 seconds. js and py refuse the time bounds
     * ("min_time cannot be negative"), js the age and the gap; so does this.
     * Decoded TimePoints of 2^63 and up are still read (fromTimePoints).
     */
    void testNegativeTimesAndSequenceConditionsAreRefused()
    {
        QVERIFY_EXCEPTION_THROWN(TimeBounds(-1, 0), std::runtime_error);
        QVERIFY_EXCEPTION_THROWN(TimeBounds(0, -5), std::runtime_error);
        QVERIFY_EXCEPTION_THROWN(TimeBounds(-10, -5), std::runtime_error);
        QScopedPointer<TimeBounds> wide(TimeBounds::fromTimePoints(quint64(1) << 63,
                                                                   std::numeric_limits<quint64>::max()));
        QCOMPARE(wide->getMinTimePoint(), quint64(1) << 63);
        QVERIFY_EXCEPTION_THROWN(delete TimeBounds::fromTimePoints(200, 100), std::runtime_error);

        Account account(KeyPair::random(), 1LL);
        TransactionBuilder builder(&account, Network::testnetNetwork());
        QVERIFY_EXCEPTION_THROWN(builder.setMinSeqAge(-1), std::runtime_error);
        QVERIFY_EXCEPTION_THROWN(builder.setMinSeqLedgerGap(-1), std::runtime_error);
        QVERIFY_EXCEPTION_THROWN(builder.setMinSeqLedgerGap(Q_INT64_C(4294967296)), std::runtime_error);
        builder.setMinSeqAge(Q_INT64_C(9223372036854775807)).setMinSeqLedgerGap(Q_INT64_C(4294967295));
        builder.setTimeout(TransactionBuilder::TIMEOUT_INFINITE);
        QScopedPointer<Transaction> tx(paymentOfTen(builder));
        QCOMPARE(tx->getPreconditions().getMinSeqAge(), quint64(9223372036854775807ULL));
        QCOMPARE(tx->getPreconditions().getMinSeqLedgerGap(), quint32(4294967295U));

        // A transaction that carries such a TimePoint reads back and re-signs
        // as itself, through XDR and through TxRep.
        Account other(KeyPair::random(), 1LL);
        TransactionBuilder wideBuilder(&other, Network::testnetNetwork());
        wideBuilder.addTimeBounds(TimeBounds::fromTimePoints(quint64(1) << 63, 0));
        QScopedPointer<Transaction> wideTx(paymentOfTen(wideBuilder));
        QScopedPointer<AbstractTransaction> decoded(
            AbstractTransaction::fromEnvelopeXdr(wideTx->toEnvelopeXdrBase64(), Network::testnetNetwork()));
        QCOMPARE(decoded->hashHex(), wideTx->hashHex());
        QScopedPointer<Transaction> fromTxrep(Txrep::fromTxrep(Txrep::toTxrep(wideTx.data()), Network::testnetNetwork()));
        QCOMPARE(fromTxrep->hashHex(), wideTx->hashHex());
    }

    /** now + timeout overflowed qint64 into a maxTime in the distant past. */
    void testATimeoutThatOverflowsIsRefused()
    {
        KeyPair* source = KeyPair::random();
        Account* account = new Account(source, 1LL);
        TransactionBuilder builder(account, Network::testnetNetwork());
        bool threw = false;
        try { builder.setTimeout(std::numeric_limits<qint64>::max()); }
        catch (const std::exception&) { threw = true; }
        QVERIFY(threw);
        delete account;
    }

    // ─── A timeout set is a timeout kept ─────────────────────────────

    /** setTimeout(180) and then preconditions without time bounds used to
     *  sign a transaction that never expires. As in java, the preconditions
     *  replace the time bounds, and build() asks for a timeout again. */
    void testPreconditionsWithoutTimeBoundsNeedANewTimeout()
    {
        Account account(KeyPair::random(), 1LL);
        TransactionBuilder builder(&account, Network::testnetNetwork());
        builder.addOperation(BumpSequenceOperation::create(qint64(2)));
        builder.setBaseFee(100).setTimeout(180);
        TransactionPreconditions pre;
        pre.setMinSeqAge(1);
        builder.addPreconditions(pre);
        bool threw = false;
        try { QScopedPointer<Transaction> tx(builder.build()); }
        catch (const std::exception&) { threw = true; }
        QVERIFY(threw);

        builder.setTimeout(180);
        QScopedPointer<Transaction> tx(builder.build());
        QVERIFY(tx->getTimeBounds() && tx->getTimeBounds()->getMaxTime() > 0);
        QCOMPARE(tx->getPreconditions().getMinSeqAge(), quint64(1));
    }

    /** A second build() on the same builder was signed with no time bounds:
     *  the first transaction took them, the "timeout set" flag stayed. */
    void testEachBuildNeedsItsOwnTimeout()
    {
        Account account(KeyPair::random(), 1LL);
        TransactionBuilder builder(&account, Network::testnetNetwork());
        builder.addOperation(BumpSequenceOperation::create(qint64(2))).setBaseFee(100).setTimeout(180);
        QScopedPointer<Transaction> first(builder.build());
        QVERIFY(first->getTimeBounds() && first->getTimeBounds()->getMaxTime() > 0);

        builder.addOperation(BumpSequenceOperation::create(qint64(3)));
        bool threw = false;
        try { QScopedPointer<Transaction> second(builder.build()); }
        catch (const std::exception&) { threw = true; }
        QVERIFY(threw);
    }

    /** The copy keeps the Soroban data (it built with the base fee alone), and
     *  the builder it came from keeps no timeout it no longer has bounds for. */
    void testACopiedBuilderKeepsItsSorobanData()
    {
        Account account(KeyPair::random(), 1LL);
        TransactionBuilder original(&account, Network::testnetNetwork());
        original.addOperation(BumpSequenceOperation::create(qint64(2))).setBaseFee(100).setTimeout(180);
        stellar::SorobanTransactionData data;
        data.resourceFee = 5000;
        original.setSorobanData(data);

        TransactionBuilder copy(original);
        QScopedPointer<Transaction> tx(copy.build());
        QCOMPARE(tx->getFee(), qint64(100 + 5000));

        original.addOperation(BumpSequenceOperation::create(qint64(3)));
        bool threw = false;
        try { QScopedPointer<Transaction> t(original.build()); }
        catch (const std::exception&) { threw = true; }
        QVERIFY(threw);
    }

    /** `x = y` (or `x = std::move(y)`) compiled to a shallow copy: both
     *  builders owned the same operations, and the second to go freed them
     *  again, or signed whatever had been allocated where they were. */
    void testABuilderCannotBeAssigned()
    {
        QVERIFY(!std::is_copy_assignable<TransactionBuilder>::value);
        QVERIFY(!std::is_move_assignable<TransactionBuilder>::value);

        // Handing a builder on by construction still moves what it owns.
        Account account(KeyPair::random(), 1LL);
        TransactionBuilder original(&account, Network::testnetNetwork());
        original.addOperation(BumpSequenceOperation::create(qint64(2)));
        TransactionBuilder taken(original);
        QCOMPARE(taken.getOperationsCount(), 1);
        QCOMPARE(original.getOperationsCount(), 0);
    }

    /** The implicit copy shared the inner transaction it owns:
     *  `auto b = FeeBumpTransactionBuilder(...).setBaseFee(200);` left b with
     *  an inner the temporary had deleted, and b.build() signed freed memory. */
    void testAFeeBumpBuilderCannotBeCopied()
    {
        QVERIFY(!std::is_copy_constructible<FeeBumpTransactionBuilder>::value);
        QVERIFY(!std::is_copy_assignable<FeeBumpTransactionBuilder>::value);
    }

    // ─── An infinite timeout is written as js and java write it ─────

    /** setTimeout(TIMEOUT_INFINITE) built PRECOND_NONE, where js-stellar-sdk
     *  17.1.0 and java write time bounds {0, 0}: the same transaction had
     *  hash c1a6cb3c... here and 312611e7... there. */
    void testAnInfiniteTimeoutIsZeroTimeBounds()
    {
        const QString source = QStringLiteral("GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H");
        {
            Account account(source, 1234567889LL);
            TransactionBuilder builder(&account, Network::testnetNetwork());
            builder.setTimeout(TransactionBuilder::TIMEOUT_INFINITE);
            QScopedPointer<Transaction> tx(paymentOfTen(builder));
            QCOMPARE(static_cast<int>(tx->toV1Xdr().cond.type),
                     static_cast<int>(stellar::PreconditionType::PRECOND_TIME));
            QCOMPARE(tx->hashHex(),
                     QStringLiteral("312611e786b70b32367f1f2c2ed245d19fe674ba9bf4c24936ec759339774d89"));
            QCOMPARE(tx->toEnvelopeXdrBase64(), QStringLiteral(
                "AAAAAgAAAABi/B0L0JGythwN1lY0aypo19NHxvLCyO5tBEcCVvwF9wAAAGQAAAAASZYC0gAAAAEAAAAAAAAAAAAAAAAAAAAA"
                "AAAAAAAAAAEAAAAAAAAAAQAAAADt4FJhvNwvlQqjuhc7bjLVyRf5e4K2QOzI0c6nWfVvEAAAAAAAAAAABfXhAAAAAAAAAAAA"));
        }
        {
            // With a V2 condition: the {0, 0} goes in its timeBounds, as in js.
            Account account(source, 1234567889LL);
            TransactionBuilder builder(&account, Network::testnetNetwork());
            builder.setMinSeqAge(7).setTimeout(TransactionBuilder::TIMEOUT_INFINITE);
            QScopedPointer<Transaction> tx(paymentOfTen(builder));
            QCOMPARE(tx->hashHex(),
                     QStringLiteral("855725184900d7bdda7e9bc61386160ac6bc63ca2dfe250bf3b4c68dc41968a9"));
        }
        {
            // Over a minTime it is refused, as py refuses it ("TimeBounds has
            // been already set"): js writes {0, 0}, dropping the 5, and this
            // kept {5, 0}. Bounds of {5, 0} on their own are still that.
            Account account(source, 1234567889LL);
            TransactionBuilder builder(&account, Network::testnetNetwork());
            builder.addTimeBounds(new TimeBounds(5, 0));
            QVERIFY_EXCEPTION_THROWN(builder.setTimeout(TransactionBuilder::TIMEOUT_INFINITE),
                                     std::runtime_error);
            QScopedPointer<Transaction> tx(paymentOfTen(builder));
            QCOMPARE(tx->getTimeBounds()->getMinTime(), qint64(5));
            QCOMPARE(tx->getTimeBounds()->getMaxTime(), qint64(0));
            QCOMPARE(tx->hashHex(),
                     QStringLiteral("2241da08dfb7cf8ad47e3bb6020dfd91c64e1cdc16e626143ee1e69a3c43f5e2"));
        }
        {
            // {0, 0} already there: TIMEOUT_INFINITE again changes nothing.
            Account account(source, 1234567889LL);
            TransactionBuilder builder(&account, Network::testnetNetwork());
            builder.setTimeout(TransactionBuilder::TIMEOUT_INFINITE).setTimeout(TransactionBuilder::TIMEOUT_INFINITE);
            QScopedPointer<Transaction> tx(paymentOfTen(builder));
            QCOMPARE(tx->hashHex(),
                     QStringLiteral("312611e786b70b32367f1f2c2ed245d19fe674ba9bf4c24936ec759339774d89"));
        }
    }

    /** An account type of the caller's own whose sequence wraps past
     *  INT64_MAX, as Account's did: build() signed sequence INT64_MIN. */
    void testNoTransactionIsBuiltPastTheLastSequenceNumber()
    {
        // Wraps in unsigned arithmetic, so the test itself has no overflow.
        class WrappingAccount : public TransactionBuilderAccount
        {
        public:
            QScopedPointer<KeyPair> keyPair{KeyPair::random()};
            qint64 sequence = std::numeric_limits<qint64>::max();
            KeyPair* getKeypair() override { return keyPair.data(); }
            qint64 getSequenceNumber() override { return sequence; }
            qint64 getIncrementedSequenceNumber() override
            {
                return static_cast<qint64>(static_cast<quint64>(sequence) + 1u);
            }
            void incrementSequenceNumber() override { sequence = getIncrementedSequenceNumber(); }
        } account;

        TransactionBuilder builder(&account, Network::testnetNetwork());
        builder.addOperation(BumpSequenceOperation::create(qint64(2))).setTimeout(180).setBaseFee(100);
        bool threw = false;
        try { delete builder.build(); }
        catch (const std::exception&) { threw = true; }
        QVERIFY(threw);
        QCOMPARE(account.sequence, std::numeric_limits<qint64>::max());

        // Refused before anything was handed over: one below, the same
        // builder signs INT64_MAX itself.
        account.sequence = std::numeric_limits<qint64>::max() - 1;
        QScopedPointer<Transaction> tx(builder.build());
        QCOMPARE(tx->getSequenceNumber(), std::numeric_limits<qint64>::max());
        QVERIFY(tx->getTimeBounds() && tx->getTimeBounds()->getMaxTime() > 0);
    }

    /** setDefaultOperationFee took a quint32: -5 arrived as 4294967291. */
    void testDefaultOperationFeeIsRangeChecked()
    {
        for (qint64 bad : {qint64(-5), qint64(0), qint64(99), qint64(4294967296LL)}) {
            bool threw = false;
            try { TransactionBuilder::setDefaultOperationFee(bad); }
            catch (const std::exception&) { threw = true; }
            QVERIFY2(threw, qPrintable(QString::number(bad)));
        }
        TransactionBuilder::setDefaultOperationFee(TransactionBuilder::BASE_FEE);
    }

    /** No operation takes a negative amount; "-5" was signed as -50000000. */
    void testNegativeAmountsAreRefused()
    {
        bool threw = false;
        try {
            delete PaymentOperation::create(
                QStringLiteral("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"),
                new AssetTypeNative(), QStringLiteral("-5"));
        } catch (const std::exception&) { threw = true; }
        QVERIFY(threw);
        QScopedPointer<PaymentOperation> ok(PaymentOperation::create(
            QStringLiteral("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"),
            new AssetTypeNative(), QStringLiteral("0.0000001")));
        QVERIFY(ok);
    }

    // ─── Fee bumps built by hand ─────────────────────────────────────

    /** The public constructor signed any fee; the builder already refused. */
    void testAFeeBumpFeeCannotBeNegative()
    {
        Account account(KeyPair::random(), 1LL);
        Transaction* inner = TransactionBuilder(&account, Network::testnetNetwork())
            .addOperation(BumpSequenceOperation::create(qint64(2)))
            .setTimeout(TransactionBuilder::TIMEOUT_INFINITE).setBaseFee(100).build();
        bool threw = false;
        try {
            FeeBumpTransaction fb(AccountConverter().enableMuxed(),
                                  account.getKeypair()->getAccountId(), -5, inner);
        } catch (const std::exception&) { threw = true; }
        QVERIFY(threw);
        delete inner;   // not taken: the constructor threw
    }

    /** A v0 inner was read through the v1 arm of its union: an access
     *  violation when signed. It is written as the v1 it is equivalent to. */
    void testAFeeBumpOverAV0InnerIsWrittenAsV1()
    {
        KeyPair* signer = KeyPair::random();
        Account account(KeyPair::fromSecretSeed(signer->getSecretSeed()), 1LL);
        QScopedPointer<Transaction> built(TransactionBuilder(&account, Network::testnetNetwork())
            .addOperation(BumpSequenceOperation::create(qint64(2)))
            .setTimeout(TransactionBuilder::TIMEOUT_INFINITE).setBaseFee(100).build());
        built->sign(signer);

        stellar::TransactionV0Envelope v0;
        v0.tx = built->toV0Xdr();
        for (const stellar::DecoratedSignature& s : built->getSignatures())
            v0.signatures.append(s);
        QByteArray bytes;
        {
            QDataStream out(&bytes, QIODevice::WriteOnly);
            out << stellar::TransactionEnvelope(v0);
        }
        Transaction* inner = dynamic_cast<Transaction*>(AbstractTransaction::fromEnvelopeXdr(
            QString::fromLatin1(bytes.toBase64()), Network::testnetNetwork()));
        QVERIFY(inner);
        QCOMPARE(inner->toEnvelopeXdr().type, stellar::EnvelopeType::ENVELOPE_TYPE_TX_V0);

        FeeBumpTransaction fb(AccountConverter().enableMuxed(),
                              KeyPair::random()->getAccountId(), 400, inner);
        fb.sign(signer);                                  // used to crash here
        const stellar::TransactionEnvelope env = fb.toEnvelopeXdr();
        QCOMPARE(env.type, stellar::EnvelopeType::ENVELOPE_TYPE_TX_FEE_BUMP);
        QCOMPARE(env.feeBump.tx.type, stellar::EnvelopeType::ENVELOPE_TYPE_TX);
        QCOMPARE(env.feeBump.tx.v1.signatures.value.size(), 1);   // the inner signature kept
        QCOMPARE(inner->hashHex(), built->hashHex());              // the same transaction
        delete signer;
    }

    // ─── Envelopes are read exactly or not at all ───────────────────

    static QString offerEnvelope()
    {
        Account account(KeyPair::random(), 41LL);
        QScopedPointer<Transaction> tx(TransactionBuilder(&account, Network::testnetNetwork())
            .addOperation(BumpSequenceOperation::create(qint64(987654321)))
            .addMemo(Memo::text(QStringLiteral("deposit 7")))
            .setTimeout(TransactionBuilder::TIMEOUT_INFINITE).setBaseFee(100).build());
        return tx->toEnvelopeXdrBase64();
    }

    /** A truncated envelope decoded with its missing fields as zeros and was
     *  signed as that; trailing bytes were ignored. */
    void testATruncatedOrPaddedEnvelopeIsRefused()
    {
        const QByteArray raw = QByteArray::fromBase64(offerEnvelope().toLatin1());
        auto refused = [](const QByteArray& bytes) {
            try {
                delete AbstractTransaction::fromEnvelopeXdr(QString::fromLatin1(bytes.toBase64()),
                                                            Network::testnetNetwork());
                return false;
            } catch (const std::exception&) { return true; }
        };
        QVERIFY(refused(raw.left(raw.size() - 4)));
        QVERIFY(refused(raw.left(raw.size() - 12)));
        QVERIFY(refused(raw + QByteArray(4, '\0')));
        QVERIFY(!refused(raw));

        // Base64 strictly, but whitespace in a pasted envelope is fine.
        QString wrapped = offerEnvelope();
        wrapped.insert(20, QStringLiteral("\n  "));
        QScopedPointer<AbstractTransaction> ok(
            AbstractTransaction::fromEnvelopeXdr(wrapped, Network::testnetNetwork()));
        QVERIFY(ok);
        bool threw = false;
        try { delete AbstractTransaction::fromEnvelopeXdr(QStringLiteral("AAAA!AAA"), Network::testnetNetwork()); }
        catch (const std::exception&) { threw = true; }
        QVERIFY(threw);
    }

    /** An envelope the decoder refuses part-way (inverted time bounds, no
     *  operations) throws, and frees what it had decoded. */
    void testARefusedEnvelopeDecodeThrowsCleanly()
    {
        auto encoded = [](stellar::TransactionEnvelope& env) {
            QByteArray bytes;
            QDataStream stream(&bytes, QIODevice::WriteOnly);
            stream << env;
            return QString::fromLatin1(bytes.toBase64());
        };
        auto refused = [](const QString& base64) {
            try {
                delete AbstractTransaction::fromEnvelopeXdr(base64, Network::testnetNetwork());
                return false;
            } catch (const std::exception&) { return true; }
        };
        Account account(KeyPair::random(), 41LL);
        QScopedPointer<Transaction> tx(TransactionBuilder(&account, Network::testnetNetwork())
            .addOperation(BumpSequenceOperation::create(qint64(987654321)))
            .addOperation(BumpSequenceOperation::create(qint64(987654322)))
            .addMemo(Memo::text(QStringLiteral("deposit 7")))
            .setTimeout(TransactionBuilder::TIMEOUT_INFINITE).setBaseFee(100).build());

        stellar::TransactionEnvelope inverted = tx->toEnvelopeXdr();
        inverted.v1.tx.cond.type = stellar::PreconditionType::PRECOND_TIME;
        inverted.v1.tx.cond.timeBounds.minTime = 20;
        inverted.v1.tx.cond.timeBounds.maxTime = 10;
        QVERIFY(refused(encoded(inverted)));

        stellar::TransactionEnvelope empty = tx->toEnvelopeXdr();
        empty.v1.tx.operations.value.clear();
        QVERIFY(refused(encoded(empty)));

        stellar::TransactionEnvelope same = tx->toEnvelopeXdr();
        QVERIFY(!refused(encoded(same)));
    }

    /** js-stellar-sdk 17.1.0, revokeTrustlineSponsorship of a liquidity-pool
     *  share: its key holds a TrustLineAsset POOL_SHARE. It is read and
     *  written back byte for byte, like its credit-asset twin. (It used to be
     *  refused: the key held a plain Asset, which cannot hold a pool share.) */
    void testAPoolShareTrustlineRevokeIsReadNotMisread()
    {
        const QString poolShare = QStringLiteral(
            "AAAAAgAAAADqSmxj4pxSCr71UHsTLsX5lUd2rr6+e5JCHuppFEbSLAAAAGQAAAAAAAAAKgAAAAEAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAEAAAAAAAAAEgAAAAAAAAABAAAAAOpKbGPinFIKvvVQexMuxfmVR3auvr57kkIe6mkURtIsAAAAA917GrgxwnMx"
            "Ddvsb5eHCqg8L714ziKt7Tfsv08zgPrHAAAAAAAAAAA=");
        const QString credit = QStringLiteral(
            "AAAAAgAAAADqSmxj4pxSCr71UHsTLsX5lUd2rr6+e5JCHuppFEbSLAAAAGQAAAAAAAAAKgAAAAEAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAEAAAAAAAAAEgAAAAAAAAABAAAAAOpKbGPinFIKvvVQexMuxfmVR3auvr57kkIe6mkURtIsAAAAAVVTRAAAAAAA"
            "6kpsY+KcUgq+9VB7Ey7F+ZVHdq6+vnuSQh7qaRRG0iwAAAAAAAAAAA==");
        QScopedPointer<AbstractTransaction> pool(
            AbstractTransaction::fromEnvelopeXdr(poolShare, Network::testnetNetwork()));
        QCOMPARE(pool->toEnvelopeXdrBase64(), poolShare);
        QScopedPointer<AbstractTransaction> ok(
            AbstractTransaction::fromEnvelopeXdr(credit, Network::testnetNetwork()));
        QCOMPARE(ok->toEnvelopeXdrBase64(), credit);
    }

    /** A memo text is bytes. One that is not UTF-8 was re-encoded with
     *  U+FFFD in it, and the signature covered a different transaction. */
    void testAMemoThatIsNotUtf8KeepsItsBytes()
    {
        Account account(KeyPair::random(), 41LL);
        QScopedPointer<Transaction> tx(TransactionBuilder(&account, Network::testnetNetwork())
            .addOperation(BumpSequenceOperation::create(qint64(42)))
            .addMemo(MemoText::fromBytes(QByteArray("12345\xff", 6)))
            .setTimeout(TransactionBuilder::TIMEOUT_INFINITE).setBaseFee(100).build());
        const QString envelope = tx->toEnvelopeXdrBase64();
        QScopedPointer<AbstractTransaction> read(
            AbstractTransaction::fromEnvelopeXdr(envelope, Network::testnetNetwork()));
        QCOMPARE(read->toEnvelopeXdrBase64(), envelope);
        QCOMPARE(read->hashHex(), tx->hashHex());
        auto* memo = dynamic_cast<MemoText*>(dynamic_cast<Transaction*>(read.data())->getMemo());
        QVERIFY(memo);
        QCOMPARE(memo->getBytes(), QByteArray("12345\xff", 6));

        // Text UTF-8 cannot hold as given is refused, not changed.
        bool threw = false;
        try { delete Memo::text(QStringLiteral("12345") + QChar(0xD800)); }
        catch (const std::exception&) { threw = true; }
        QVERIFY(threw);
    }

    /** A build() that threw after taking the time bounds left the flag set:
     *  the retry was signed with no time bounds. */
    void testAFailedBuildKeepsItsTimeout()
    {
        Account account(KeyPair::random(), 100LL);
        TransactionBuilder builder(&account, Network::testnetNetwork());
        builder.setBaseFee(100).setTimeout(180);
        bool threw = false;
        try { QScopedPointer<Transaction> none(builder.build()); }       // no operations
        catch (const std::exception&) { threw = true; }
        QVERIFY(threw);
        builder.addOperation(BumpSequenceOperation::create(qint64(200)));
        QScopedPointer<Transaction> tx(builder.build());
        QVERIFY(tx->getTimeBounds() && tx->getTimeBounds()->getMaxTime() > 0);
    }

    /** A PRECOND_V2 whose V2 fields are all zero is its own envelope (and
     *  hash): it was written back as PRECOND_TIME, so a valid envelope from
     *  js or py (setMinAccountSequenceAge(0)) was refused on reading. */
    void testAnAllZeroV2StaysV2()
    {
        Account account(KeyPair::random(), 100LL);
        TransactionPreconditions pre;
        pre.setTimeBounds(new TimeBounds(0, 1900000000));
        pre.setV2(true);
        QScopedPointer<Transaction> tx(TransactionBuilder(&account, Network::testnetNetwork())
            .addOperation(BumpSequenceOperation::create(qint64(200)))
            .addPreconditions(pre).setBaseFee(100).build());
        QVERIFY(tx->toV1Xdr().cond.type == stellar::PreconditionType::PRECOND_V2);
        const QString envelope = tx->toEnvelopeXdrBase64();
        QScopedPointer<AbstractTransaction> read(
            AbstractTransaction::fromEnvelopeXdr(envelope, Network::testnetNetwork()));
        QCOMPARE(read->toEnvelopeXdrBase64(), envelope);
        QCOMPARE(read->hashHex(), tx->hashHex());

        QScopedPointer<Transaction> back(Txrep::fromTxrep(Txrep::toTxrep(tx.data()),
                                                          Network::testnetNetwork()));
        QCOMPARE(back->hashHex(), tx->hashHex());
    }
};

ADD_TEST(TransactionBuilderTest)
#endif // TRANSACTIONBUILDERTEST_H
