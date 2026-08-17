#ifndef SEP11TXREPTEST_H
#define SEP11TXREPTEST_H

#include <QObject>
#include <QtTest>
#include "testcollector.h"

#include "../src/txrep.h"
#include "../src/abstracttransaction.h"
#include "../src/transaction.h"
#include "../src/transactionbuilder.h"
#include "../src/account.h"
#include "../src/managedataoperation.h"
#include "../src/paymentoperation.h"
#include "../src/createaccountoperation.h"
#include "../src/accountmergeoperation.h"
#include "../src/bumpsequenceoperation.h"
#include "../src/setoptionsoperation.h"
#include "../src/signerkey.h"
#include "../src/changetrustoperation.h"
// AllowTrustOperation is deprecated (superseded by SetTrustLineFlags); in the
// legacy-global-namespace test build its QSTELLAR_ALIAS expands to a using-
// declaration that names the deprecated class, so the include itself warns.
#if defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable: 4996)
#elif defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#include "../src/allowtrustoperation.h"
#if defined(_MSC_VER)
#  pragma warning(pop)
#elif defined(__GNUC__)
#  pragma GCC diagnostic pop
#endif
#include "../src/managesellofferoperation.h"
#include "../src/managebuyofferoperation.h"
#include "../src/createpassivesellofferoperation.h"
#include "../src/pathpaymentstrictreceiveoperation.h"
#include "../src/pathpaymentstrictsendoperation.h"
#include "../src/beginsponsoringfuturereservesoperation.h"
#include "../src/endsponsoringfuturereservesoperation.h"
#include "../src/clawbackoperation.h"
#include "../src/clawbackclaimablebalanceoperation.h"
#include "../src/settrustlineflagsoperation.h"
#include "../src/claimclaimablebalanceoperation.h"
#include "../src/liquiditypooldepositoperation.h"
#include "../src/liquiditypoolwithdrawoperation.h"
#include "../src/createclaimablebalanceoperation.h"
#include "../src/claimant.h"
#include "../src/predicate.h"
#include "../src/revokeaccountsponsorshipoperation.h"
#include "../src/revoketrustlinesponsorshipoperation.h"
#include "../src/revokeoffersponsorshipoperation.h"
#include "../src/revokedatasponsorshipoperation.h"
#include "../src/revokeclaimablebalancesponsorshipoperation.h"
#include "../src/revokeliquiditypoolsponsorshipoperation.h"
#include "../src/revokesignersponsorshipoperation.h"
#include "../src/price.h"
#include "../src/assettypenative.h"
#include "../src/asset.h"
#include "../src/keypair.h"
#include "../src/network.h"
#include "../src/timebounds.h"
#include "../src/memo.h"
#include "../src/transactionpreconditions.h"
#include "../src/ledgerbounds.h"
#include "../src/signerkey.h"

/** Minimal SEP-11 Txrep round-trip tests (SEP-10-style subset). */
class Sep11TxrepTest: public QObject
{
    Q_OBJECT
private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    void testEmitContainsExpectedLines()
    {
        KeyPair* source = KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 41);
        QScopedPointer<Transaction> tx(
            TransactionBuilder(acc, Network::testnetNetwork())
                .addOperation(new ManageDataOperation("auth", "deadbeef"))
                .addTimeBounds(new TimeBounds(100, 200))
                .setBaseFee(100)
                .build());

        QString txrep = Txrep::toTxrep(tx.data());
        QVERIFY(txrep.contains("type: ENVELOPE_TYPE_TX"));
        QVERIFY(txrep.contains("tx.sourceAccount: GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"));
        QVERIFY(txrep.contains("tx.fee: 100"));
        QVERIFY(txrep.contains("tx.seqNum: 42"));
        QVERIFY(txrep.contains("tx.cond.type: PRECOND_TIME"));
        QVERIFY(txrep.contains("tx.cond.timeBounds.minTime: 100"));
        QVERIFY(txrep.contains("tx.cond.timeBounds.maxTime: 200"));
        QVERIFY(txrep.contains("tx.memo.type: MEMO_NONE"));
        QVERIFY(txrep.contains("tx.operations.len: 1"));
        QVERIFY(txrep.contains("tx.operations[0].body.type: MANAGE_DATA"));
        QVERIFY(txrep.contains("tx.operations[0].body.manageDataOp.dataName: \"auth\""));

        delete acc;
    }

    void testManageDataRoundTrip()
    {
        KeyPair* source = KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 0);
        QScopedPointer<Transaction> original(
            TransactionBuilder(acc, Network::testnetNetwork())
                .addOperation(new ManageDataOperation("example.com auth", "value"))
                .addTimeBounds(new TimeBounds(1000, 2000))
                .setBaseFee(100)
                .build());

        QString txrep = Txrep::toTxrep(original.data());
        QScopedPointer<Transaction> parsed(Txrep::fromTxrep(txrep, Network::testnetNetwork()));

        QCOMPARE(parsed->getSourceAccount(), original->getSourceAccount());
        QCOMPARE(parsed->getFee(),           original->getFee());
        QCOMPARE(parsed->getSequenceNumber(),original->getSequenceNumber());
        QVERIFY(parsed->getTimeBounds() != nullptr);
        QCOMPARE(parsed->getTimeBounds()->getMinTime(), qint64(1000));
        QCOMPARE(parsed->getTimeBounds()->getMaxTime(), qint64(2000));
        QCOMPARE(parsed->getOperations().size(), 1);
        auto* md = dynamic_cast<ManageDataOperation*>(parsed->getOperations().at(0));
        QVERIFY(md != nullptr);
        QCOMPARE(md->getName(), QString("example.com auth"));
        QCOMPARE(md->getValue(), QByteArray("value"));

        delete acc;
    }

    void testManageDataDeletePresentFalse()
    {
        KeyPair* source = KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 0);
        QScopedPointer<Transaction> tx(
            TransactionBuilder(acc, Network::testnetNetwork())
                .addOperation(new ManageDataOperation("delete me")) // no value → delete
                .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build());

        QString txrep = Txrep::toTxrep(tx.data());
        QVERIFY(txrep.contains("tx.cond.type: PRECOND_NONE"));
        QVERIFY(txrep.contains("tx.operations[0].body.manageDataOp.dataValue._present: false"));

        QScopedPointer<Transaction> parsed(Txrep::fromTxrep(txrep, Network::testnetNetwork()));
        auto* md = dynamic_cast<ManageDataOperation*>(parsed->getOperations().at(0));
        QVERIFY(md != nullptr);
        QCOMPARE(md->getName(), QString("delete me"));
        QVERIFY(md->getValue().isEmpty());

        delete acc;
    }

    void testPaymentRoundTrip()
    {
        KeyPair* source = KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 99);
        QScopedPointer<Transaction> original(
            TransactionBuilder(acc, Network::testnetNetwork())
                .addOperation(PaymentOperation::create(
                    QString("GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7"),
                    new AssetTypeNative(),
                    "10.5"))
                .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build());

        QString txrep = Txrep::toTxrep(original.data());
        QVERIFY(txrep.contains("PAYMENT"));
        QVERIFY(txrep.contains("tx.operations[0].body.paymentOp.asset: native"));
        QVERIFY(txrep.contains("tx.operations[0].body.paymentOp.amount: 105000000"));

        QScopedPointer<Transaction> parsed(Txrep::fromTxrep(txrep, Network::testnetNetwork()));
        auto* pay = dynamic_cast<PaymentOperation*>(parsed->getOperations().at(0));
        QVERIFY(pay != nullptr);
        QCOMPARE(pay->getDestination(),
                 QString("GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7"));
        QCOMPARE(pay->getAmount(), QString("10.5"));
        QVERIFY(dynamic_cast<AssetTypeNative*>(pay->getAsset()) != nullptr);

        delete acc;
    }

    // INTEROP: parse the EXACT canonical txrep that py-stellar-sdk / stc /
    // Laboratory emit — with human-readable `(...)` comments, `native` asset
    // and PRECOND_TIME. Verbatim from py-stellar-sdk's to_txrep().
    void testParsesCanonicalTxrepFromOtherTools()
    {
        QString txrep =
            "type: ENVELOPE_TYPE_TX\n"
            "tx.sourceAccount: GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG\n"
            "tx.fee: 100\n"
            "tx.seqNum: 42\n"
            "tx.cond.type: PRECOND_TIME\n"
            "tx.cond.timeBounds.minTime: 0 (1970-01-01 00:00:00.000000+00:00 (UTC))\n"
            "tx.cond.timeBounds.maxTime: 0 (1970-01-01 00:00:00.000000+00:00 (UTC))\n"
            "tx.memo.type: MEMO_NONE\n"
            "tx.operations.len: 1\n"
            "tx.operations[0].sourceAccount._present: false\n"
            "tx.operations[0].body.type: PAYMENT\n"
            "tx.operations[0].body.paymentOp.destination: GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7\n"
            "tx.operations[0].body.paymentOp.asset: native\n"
            "tx.operations[0].body.paymentOp.amount: 105000000 (10.5)\n"
            "tx.ext.v: 0\n"
            "signatures.len: 0\n";

        QScopedPointer<Transaction> parsed(Txrep::fromTxrep(txrep, Network::testnetNetwork()));
        QCOMPARE(parsed->getSourceAccount(), QString("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"));
        QCOMPARE(parsed->getSequenceNumber(), qint64(42));
        QVERIFY(parsed->getTimeBounds() != nullptr);
        QCOMPARE(parsed->getTimeBounds()->getMinTime(), qint64(0));
        QCOMPARE(parsed->getOperations().size(), 1);
        auto* pay = dynamic_cast<PaymentOperation*>(parsed->getOperations().at(0));
        QVERIFY(pay != nullptr);
        QCOMPARE(pay->getDestination(), QString("GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7"));
        QVERIFY(dynamic_cast<AssetTypeNative*>(pay->getAsset()) != nullptr);  // "native" parsed
        QCOMPARE(pay->getAmount(), QString("10.5"));   // "105000000 (10.5)" → comment stripped
    }

    // ── Memo round-trips (all SEP-11 types) ──────────────────────────────

    void testMemoTextRoundTrip()
    {
        KeyPair* source = KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 0);
        QScopedPointer<Transaction> original(
            TransactionBuilder(acc, Network::testnetNetwork())
                .addOperation(new ManageDataOperation("k", "v"))
                .addMemo(Memo::text("hello world"))
                .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build());

        QString txrep = Txrep::toTxrep(original.data());
        QVERIFY(txrep.contains("tx.memo.type: MEMO_TEXT"));
        QVERIFY(txrep.contains("tx.memo.text: \"hello world\""));

        QScopedPointer<Transaction> parsed(Txrep::fromTxrep(txrep, Network::testnetNetwork()));
        auto* m = dynamic_cast<MemoText*>(parsed->getMemo());
        QVERIFY(m != nullptr);
        QCOMPARE(m->getText(), QString("hello world"));

        delete acc;
    }

    void testMemoIdRoundTrip()
    {
        KeyPair* source = KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 0);
        QScopedPointer<Transaction> original(
            TransactionBuilder(acc, Network::testnetNetwork())
                .addOperation(new ManageDataOperation("k", "v"))
                .addMemo(Memo::id(Q_UINT64_C(18446744073709551615)))  // 2^64-1, must not overflow
                .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build());

        QString txrep = Txrep::toTxrep(original.data());
        QVERIFY(txrep.contains("tx.memo.type: MEMO_ID"));
        QVERIFY(txrep.contains("tx.memo.id: 18446744073709551615"));

        QScopedPointer<Transaction> parsed(Txrep::fromTxrep(txrep, Network::testnetNetwork()));
        auto* m = dynamic_cast<MemoId*>(parsed->getMemo());
        QVERIFY(m != nullptr);
        QCOMPARE(m->getId(), Q_UINT64_C(18446744073709551615));

        delete acc;
    }

    void testMemoHashRoundTrip()
    {
        KeyPair* source = KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 0);
        QScopedPointer<Transaction> original(
            TransactionBuilder(acc, Network::testnetNetwork())
                .addOperation(new ManageDataOperation("k", "v"))
                .addMemo(Memo::hash(QString("deadbeef")))
                .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build());

        QString txrep = Txrep::toTxrep(original.data());
        QVERIFY(txrep.contains("tx.memo.type: MEMO_HASH"));

        QScopedPointer<Transaction> parsed(Txrep::fromTxrep(txrep, Network::testnetNetwork()));
        auto* m = dynamic_cast<MemoHash*>(parsed->getMemo());
        QVERIFY(m != nullptr);
        QCOMPARE(m->getHexValue(),
                 dynamic_cast<MemoHash*>(original->getMemo())->getHexValue());

        delete acc;
    }

    void testMemoReturnRoundTrip()
    {
        KeyPair* source = KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 0);
        QScopedPointer<Transaction> original(
            TransactionBuilder(acc, Network::testnetNetwork())
                .addOperation(new ManageDataOperation("k", "v"))
                .addMemo(Memo::returnHash(QString("0123456789abcdef")))
                .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build());

        QString txrep = Txrep::toTxrep(original.data());
        QVERIFY(txrep.contains("tx.memo.type: MEMO_RETURN"));

        QScopedPointer<Transaction> parsed(Txrep::fromTxrep(txrep, Network::testnetNetwork()));
        auto* m = dynamic_cast<MemoReturnHash*>(parsed->getMemo());
        QVERIFY(m != nullptr);
        QCOMPARE(m->getHexValue(),
                 dynamic_cast<MemoReturnHash*>(original->getMemo())->getHexValue());

        delete acc;
    }

    void testCreateAccountRoundTrip()
    {
        KeyPair* source = KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 0);
        QScopedPointer<Transaction> original(
            TransactionBuilder(acc, Network::testnetNetwork())
                .addOperation(CreateAccountOperation::create(
                    KeyPair::fromAccountId("GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7"),
                    "25.5"))
                .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build());

        QString txrep = Txrep::toTxrep(original.data());
        QVERIFY(txrep.contains("tx.operations[0].body.type: CREATE_ACCOUNT"));
        QVERIFY(txrep.contains("tx.operations[0].body.createAccountOp.startingBalance: 255000000"));

        QScopedPointer<Transaction> parsed(Txrep::fromTxrep(txrep, Network::testnetNetwork()));
        auto* ca = dynamic_cast<CreateAccountOperation*>(parsed->getOperations().at(0));
        QVERIFY(ca != nullptr);
        QCOMPARE(ca->getDestination()->getAccountId(),
                 QString("GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7"));
        QCOMPARE(ca->getStartingBalance(), QString("25.5"));

        delete acc;
    }

    void testAccountMergeRoundTrip()
    {
        KeyPair* source = KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 0);
        QScopedPointer<Transaction> original(
            TransactionBuilder(acc, Network::testnetNetwork())
                .addOperation(AccountMergeOperation::create(
                    QString("GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7")))
                .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build());

        QString txrep = Txrep::toTxrep(original.data());
        QVERIFY(txrep.contains("tx.operations[0].body.type: ACCOUNT_MERGE"));
        QVERIFY(txrep.contains("tx.operations[0].body.destination: "
                               "GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7"));

        QScopedPointer<Transaction> parsed(Txrep::fromTxrep(txrep, Network::testnetNetwork()));
        auto* am = dynamic_cast<AccountMergeOperation*>(parsed->getOperations().at(0));
        QVERIFY(am != nullptr);
        QCOMPARE(am->getDestination(),
                 QString("GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7"));

        delete acc;
    }

    // ── RevokeSponsorship: all 7 ledger-key / signer variants ────────────

    void testRevokeAccountSponsorshipRoundTrip()
    {
        const QString a = "GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO";
        Account* acc = new Account(KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"), 0);
        QScopedPointer<Transaction> original(TransactionBuilder(acc, Network::testnetNetwork())
            .addOperation(RevokeAccountSponsorshipOperation::create(a))
            .setTimeout(TransactionBuilder::TIMEOUT_INFINITE).setBaseFee(100).build());
        QString txrep = Txrep::toTxrep(original.data());
        QVERIFY(txrep.contains("tx.operations[0].body.type: REVOKE_SPONSORSHIP"));
        QVERIFY(txrep.contains("revokeSponsorshipOp.ledgerKey.type: ACCOUNT"));
        QVERIFY(txrep.contains("revokeSponsorshipOp.ledgerKey.account: " + a));
        QScopedPointer<Transaction> parsed(Txrep::fromTxrep(txrep, Network::testnetNetwork()));
        auto* r = dynamic_cast<RevokeAccountSponsorshipOperation*>(parsed->getOperations().at(0));
        QVERIFY(r != nullptr); QCOMPARE(r->getAccountId(), a);
        delete acc;
    }

    void testRevokeTrustlineSponsorshipRoundTrip()
    {
        const QString a = "GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO";
        const QString issuer = "GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7";
        Account* acc = new Account(KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"), 0);
        QScopedPointer<Transaction> original(TransactionBuilder(acc, Network::testnetNetwork())
            .addOperation(RevokeTrustlineSponsorshipOperation::create(a, Asset::createNonNativeAsset("USD", issuer)))
            .setTimeout(TransactionBuilder::TIMEOUT_INFINITE).setBaseFee(100).build());
        QString txrep = Txrep::toTxrep(original.data());
        QVERIFY(txrep.contains("revokeSponsorshipOp.ledgerKey.type: TRUSTLINE"));
        QVERIFY(txrep.contains("revokeSponsorshipOp.ledgerKey.trustLine.asset: USD:" + issuer));
        QScopedPointer<Transaction> parsed(Txrep::fromTxrep(txrep, Network::testnetNetwork()));
        auto* r = dynamic_cast<RevokeTrustlineSponsorshipOperation*>(parsed->getOperations().at(0));
        QVERIFY(r != nullptr);
        QCOMPARE(r->getAccountId(), a);
        QVERIFY(r->getAsset()->equals(Asset::createNonNativeAsset("USD", KeyPair::fromAccountId(issuer))));
        delete acc;
    }

    void testRevokeOfferSponsorshipRoundTrip()
    {
        const QString seller = "GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO";
        Account* acc = new Account(KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"), 0);
        QScopedPointer<Transaction> original(TransactionBuilder(acc, Network::testnetNetwork())
            .addOperation(RevokeOfferSponsorshipOperation::create(seller, Q_INT64_C(12345)))
            .setTimeout(TransactionBuilder::TIMEOUT_INFINITE).setBaseFee(100).build());
        QString txrep = Txrep::toTxrep(original.data());
        QVERIFY(txrep.contains("revokeSponsorshipOp.ledgerKey.type: OFFER"));
        QVERIFY(txrep.contains("revokeSponsorshipOp.ledgerKey.offer.offerID: 12345"));
        QScopedPointer<Transaction> parsed(Txrep::fromTxrep(txrep, Network::testnetNetwork()));
        auto* r = dynamic_cast<RevokeOfferSponsorshipOperation*>(parsed->getOperations().at(0));
        QVERIFY(r != nullptr);
        QCOMPARE(r->getSeller(), seller);
        QCOMPARE(r->getOfferId(), Q_INT64_C(12345));
        delete acc;
    }

    void testRevokeDataSponsorshipRoundTrip()
    {
        const QString a = "GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO";
        Account* acc = new Account(KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"), 0);
        QScopedPointer<Transaction> original(TransactionBuilder(acc, Network::testnetNetwork())
            .addOperation(RevokeDataSponsorshipOperation::create(a, "config.memo"))
            .setTimeout(TransactionBuilder::TIMEOUT_INFINITE).setBaseFee(100).build());
        QString txrep = Txrep::toTxrep(original.data());
        QVERIFY(txrep.contains("revokeSponsorshipOp.ledgerKey.type: DATA"));
        QVERIFY(txrep.contains("revokeSponsorshipOp.ledgerKey.data.dataName: config.memo"));
        QScopedPointer<Transaction> parsed(Txrep::fromTxrep(txrep, Network::testnetNetwork()));
        auto* r = dynamic_cast<RevokeDataSponsorshipOperation*>(parsed->getOperations().at(0));
        QVERIFY(r != nullptr);
        QCOMPARE(r->getAccountId(), a);
        QCOMPARE(r->getDataName(), QString("config.memo"));
        delete acc;
    }

    void testRevokeClaimableBalanceSponsorshipRoundTrip()
    {
        const QString balanceId =
            "00000000""abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789";
        Account* acc = new Account(KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"), 0);
        QScopedPointer<Transaction> original(TransactionBuilder(acc, Network::testnetNetwork())
            .addOperation(RevokeClaimableBalanceSponsorshipOperation::create(balanceId))
            .setTimeout(TransactionBuilder::TIMEOUT_INFINITE).setBaseFee(100).build());
        QString txrep = Txrep::toTxrep(original.data());
        QVERIFY(txrep.contains("revokeSponsorshipOp.ledgerKey.type: CLAIMABLE_BALANCE"));
        QScopedPointer<Transaction> parsed(Txrep::fromTxrep(txrep, Network::testnetNetwork()));
        auto* r = dynamic_cast<RevokeClaimableBalanceSponsorshipOperation*>(parsed->getOperations().at(0));
        QVERIFY(r != nullptr);
        QCOMPARE(r->getBalanceId(), balanceId);
        delete acc;
    }

    void testRevokeLiquidityPoolSponsorshipRoundTrip()
    {
        const QString poolId = "dd7b1ab831c273310ddbec6f97870aa83c2fbcf57320c0bd000242331b46f1d2";
        Account* acc = new Account(KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"), 0);
        QScopedPointer<Transaction> original(TransactionBuilder(acc, Network::testnetNetwork())
            .addOperation(RevokeLiquidityPoolSponsorshipOperation::create(poolId))
            .setTimeout(TransactionBuilder::TIMEOUT_INFINITE).setBaseFee(100).build());
        QString txrep = Txrep::toTxrep(original.data());
        QVERIFY(txrep.contains("revokeSponsorshipOp.ledgerKey.type: LIQUIDITY_POOL"));
        QScopedPointer<Transaction> parsed(Txrep::fromTxrep(txrep, Network::testnetNetwork()));
        auto* r = dynamic_cast<RevokeLiquidityPoolSponsorshipOperation*>(parsed->getOperations().at(0));
        QVERIFY(r != nullptr);
        QCOMPARE(r->getLiquidityPoolId(), poolId);
        delete acc;
    }

    void testRevokeSignerSponsorshipRoundTrip()
    {
        const QString a      = "GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO";
        const QString signer = "GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7";
        Account* acc = new Account(KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"), 0);
        QScopedPointer<Transaction> original(TransactionBuilder(acc, Network::testnetNetwork())
            .addOperation(RevokeSignerSponsorshipOperation::create(
                a, SignerKey::ed25519PublicKey(signer).toXdr()))
            .setTimeout(TransactionBuilder::TIMEOUT_INFINITE).setBaseFee(100).build());
        QString txrep = Txrep::toTxrep(original.data());
        QVERIFY(txrep.contains("revokeSponsorshipOp.type: REVOKE_SPONSORSHIP_SIGNER"));
        QVERIFY(txrep.contains("revokeSponsorshipOp.signer.signerKey: " + signer));
        QScopedPointer<Transaction> parsed(Txrep::fromTxrep(txrep, Network::testnetNetwork()));
        auto* r = dynamic_cast<RevokeSignerSponsorshipOperation*>(parsed->getOperations().at(0));
        QVERIFY(r != nullptr);
        QCOMPARE(r->getAccountId(), a);
        QCOMPARE(SignerKey::fromXdr(r->getSigner()).toString(), signer);
        delete acc;
    }

    // CreateClaimableBalance with a NESTED predicate tree covering every
    // predicate type: claimant 0 = And[ AbsBefore, Or[ RelBefore, Not(Uncond) ] ],
    // claimant 1 = Unconditional.
    /** Every txrep test builds its operations with the value constructors, so
     *  none of them ever exercises the XDR constructor that
     *  `Transaction::fromEnvelopeXdr` uses. `CreateClaimableBalanceOperation`'s
     *  XDR constructor left its `m_asset` cache pointer uninitialized, so
     *  emitting txrep for a transaction decoded from an envelope dereferenced
     *  garbage (and the destructor deleted it). Go through the envelope. */
    void testToTxrepFromEnvelopeDecodedClaimableBalance()
    {
        KeyPair* source = KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 0);
        QList<Claimant> claimants;
        claimants.append(Claimant("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO",
                                  new Predicate::Unconditional()));

        QScopedPointer<Transaction> built(
            TransactionBuilder(acc, Network::testnetNetwork())
                .addOperation(CreateClaimableBalanceOperation::create(
                    "10", new AssetTypeNative(), claimants))
                .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build());

        // Round-trip through the envelope so the operation is rebuilt by the
        // XDR constructor, then emit.
        QScopedPointer<AbstractTransaction> decoded(
            AbstractTransaction::fromEnvelopeXdr(built->toEnvelopeXdrBase64(),
                                                 Network::testnetNetwork()));
        auto* tx = dynamic_cast<Transaction*>(decoded.data());
        QVERIFY(tx != nullptr);

        QString txrep = Txrep::toTxrep(tx);
        QVERIFY(txrep.contains("tx.operations[0].body.type: CREATE_CLAIMABLE_BALANCE"));
        QVERIFY(txrep.contains("tx.operations[0].body.createClaimableBalanceOp.asset: native"));
        QVERIFY(txrep.contains("tx.operations[0].body.createClaimableBalanceOp.amount: 100000000"));

        delete acc;
    }

    void testCreateClaimableBalanceRoundTrip()
    {
        const QString dest0 = "GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO";
        const QString dest1 = "GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7";
        KeyPair* source = KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 0);

        QList<Predicate*> orInner;
        orInner.append(new Predicate::RelBefore(100));
        orInner.append(new Predicate::Not(new Predicate::Unconditional()));
        QList<Predicate*> andInner;
        andInner.append(new Predicate::AbsBefore(1600000000));
        andInner.append(new Predicate::Or(orInner));

        QList<Claimant> claimants;
        claimants.append(Claimant(dest0, new Predicate::And(andInner)));
        claimants.append(Claimant(dest1, new Predicate::Unconditional()));

        QScopedPointer<Transaction> original(
            TransactionBuilder(acc, Network::testnetNetwork())
                .addOperation(CreateClaimableBalanceOperation::create(
                    "10", new AssetTypeNative(), claimants))
                .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build());

        QString txrep = Txrep::toTxrep(original.data());
        QVERIFY(txrep.contains("tx.operations[0].body.type: CREATE_CLAIMABLE_BALANCE"));
        QVERIFY(txrep.contains("tx.operations[0].body.createClaimableBalanceOp.amount: 100000000"));
        QVERIFY(txrep.contains("tx.operations[0].body.createClaimableBalanceOp.claimants.len: 2"));
        QVERIFY(txrep.contains("tx.operations[0].body.createClaimableBalanceOp.claimants[0].v0.predicate.type: CLAIM_PREDICATE_AND"));
        QVERIFY(txrep.contains("tx.operations[0].body.createClaimableBalanceOp.claimants[0].v0.predicate.andPredicates[0].type: CLAIM_PREDICATE_BEFORE_ABSOLUTE_TIME"));
        QVERIFY(txrep.contains("tx.operations[0].body.createClaimableBalanceOp.claimants[0].v0.predicate.andPredicates[0].absBefore: 1600000000"));
        QVERIFY(txrep.contains("tx.operations[0].body.createClaimableBalanceOp.claimants[1].v0.predicate.type: CLAIM_PREDICATE_UNCONDITIONAL"));

        QScopedPointer<Transaction> parsed(Txrep::fromTxrep(txrep, Network::testnetNetwork()));
        auto* cc = dynamic_cast<CreateClaimableBalanceOperation*>(parsed->getOperations().at(0));
        QVERIFY(cc != nullptr);
        QCOMPARE(cc->getAmount(), QString("10"));
        QVERIFY(dynamic_cast<AssetTypeNative*>(cc->getAsset()) != nullptr);

        QList<Claimant> cl = cc->getClaimants();
        QCOMPARE(cl.size(), 2);
        QCOMPARE(cl.at(0).getDestination(), dest0);
        QCOMPARE(cl.at(1).getDestination(), dest1);

        // claimant 0: And[ AbsBefore(1600000000), Or[ RelBefore(100), Not(Uncond) ] ]
        auto* andp = dynamic_cast<const Predicate::And*>(&cl.at(0).getPredicate());
        QVERIFY(andp != nullptr);
        QList<Predicate*> a = andp->getInner();
        QCOMPARE(a.size(), 2);
        auto* ab = dynamic_cast<Predicate::AbsBefore*>(a.at(0));
        QVERIFY(ab != nullptr);
        QCOMPARE(ab->getTimestampSeconds(), Q_INT64_C(1600000000));
        auto* orp = dynamic_cast<Predicate::Or*>(a.at(1));
        QVERIFY(orp != nullptr);
        QList<Predicate*> o = orp->getInner();
        QCOMPARE(o.size(), 2);
        auto* rb = dynamic_cast<Predicate::RelBefore*>(o.at(0));
        QVERIFY(rb != nullptr);
        QCOMPARE(rb->getSecondsSinceClose(), Q_INT64_C(100));
        QVERIFY(dynamic_cast<Predicate::Not*>(o.at(1)) != nullptr);

        // claimant 1: Unconditional
        QVERIFY(dynamic_cast<const Predicate::Unconditional*>(&cl.at(1).getPredicate()) != nullptr);

        delete acc;
    }

    void testClaimClaimableBalanceRoundTrip()
    {
        const QString balanceId =
            "00000000""fedcba9876543210fedcba9876543210fedcba9876543210fedcba9876543210";
        KeyPair* source = KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 0);
        QScopedPointer<Transaction> original(
            TransactionBuilder(acc, Network::testnetNetwork())
                .addOperation(ClaimClaimableBalanceOperation::create(balanceId))
                .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build());

        QString txrep = Txrep::toTxrep(original.data());
        QVERIFY(txrep.contains("tx.operations[0].body.type: CLAIM_CLAIMABLE_BALANCE"));
        QVERIFY(txrep.contains("tx.operations[0].body.claimClaimableBalanceOp.balanceID: " + balanceId));

        QScopedPointer<Transaction> parsed(Txrep::fromTxrep(txrep, Network::testnetNetwork()));
        auto* cl = dynamic_cast<ClaimClaimableBalanceOperation*>(parsed->getOperations().at(0));
        QVERIFY(cl != nullptr);
        QCOMPARE(cl->getBalanceId(), balanceId);

        delete acc;
    }

    void testLiquidityPoolDepositRoundTrip()
    {
        const QByteArray poolId = QByteArray::fromHex(
            "dd7b1ab831c273310ddbec6f97870aa83c2fbcf57320c0bd000242331b46f1d2");
        KeyPair* source = KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 0);
        QScopedPointer<Transaction> original(
            TransactionBuilder(acc, Network::testnetNetwork())
                .addOperation(LiquidityPoolDepositOperation::create(
                    poolId, "100", "200", Price(1, 2), Price(2, 1)))
                .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build());

        QString txrep = Txrep::toTxrep(original.data());
        QVERIFY(txrep.contains("tx.operations[0].body.type: LIQUIDITY_POOL_DEPOSIT"));
        QVERIFY(txrep.contains("tx.operations[0].body.liquidityPoolDepositOp.maxAmountA: 1000000000"));
        QVERIFY(txrep.contains("tx.operations[0].body.liquidityPoolDepositOp.maxAmountB: 2000000000"));
        QVERIFY(txrep.contains("tx.operations[0].body.liquidityPoolDepositOp.minPrice.n: 1"));
        QVERIFY(txrep.contains("tx.operations[0].body.liquidityPoolDepositOp.minPrice.d: 2"));
        QVERIFY(txrep.contains("tx.operations[0].body.liquidityPoolDepositOp.maxPrice.n: 2"));

        QScopedPointer<Transaction> parsed(Txrep::fromTxrep(txrep, Network::testnetNetwork()));
        auto* lpd = dynamic_cast<LiquidityPoolDepositOperation*>(parsed->getOperations().at(0));
        QVERIFY(lpd != nullptr);
        QCOMPARE(lpd->getLiquidityPoolID(), poolId);
        QCOMPARE(lpd->getMaxAmountA(), QString("100"));
        QCOMPARE(lpd->getMaxAmountB(), QString("200"));
        QCOMPARE(lpd->getMinPrice().getNumerator(), 1);
        QCOMPARE(lpd->getMinPrice().getDenominator(), 2);
        QCOMPARE(lpd->getMaxPrice().getNumerator(), 2);
        QCOMPARE(lpd->getMaxPrice().getDenominator(), 1);

        delete acc;
    }

    void testLiquidityPoolWithdrawRoundTrip()
    {
        const QByteArray poolId = QByteArray::fromHex(
            "dd7b1ab831c273310ddbec6f97870aa83c2fbcf57320c0bd000242331b46f1d2");
        KeyPair* source = KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 0);
        QScopedPointer<Transaction> original(
            TransactionBuilder(acc, Network::testnetNetwork())
                .addOperation(LiquidityPoolWithdrawOperation::create(
                    poolId, "50", "10", "20"))
                .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build());

        QString txrep = Txrep::toTxrep(original.data());
        QVERIFY(txrep.contains("tx.operations[0].body.type: LIQUIDITY_POOL_WITHDRAW"));
        QVERIFY(txrep.contains("tx.operations[0].body.liquidityPoolWithdrawOp.amount: 500000000"));
        QVERIFY(txrep.contains("tx.operations[0].body.liquidityPoolWithdrawOp.minAmountA: 100000000"));

        QScopedPointer<Transaction> parsed(Txrep::fromTxrep(txrep, Network::testnetNetwork()));
        auto* lpw = dynamic_cast<LiquidityPoolWithdrawOperation*>(parsed->getOperations().at(0));
        QVERIFY(lpw != nullptr);
        QCOMPARE(lpw->getLiquidityPoolID(), poolId);
        QCOMPARE(lpw->getAmount(), QString("50"));
        QCOMPARE(lpw->getMinAmountA(), QString("10"));
        QCOMPARE(lpw->getMinAmountB(), QString("20"));

        delete acc;
    }

    void testClawbackRoundTrip()
    {
        const QString issuer = "GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7";
        const QString from   = "GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO";
        KeyPair* source = KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 0);
        QScopedPointer<Transaction> original(
            TransactionBuilder(acc, Network::testnetNetwork())
                .addOperation(ClawbackOperation::create(
                    from, Asset::createNonNativeAsset("USD", issuer), "25"))
                .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build());

        QString txrep = Txrep::toTxrep(original.data());
        QVERIFY(txrep.contains("tx.operations[0].body.type: CLAWBACK"));
        QVERIFY(txrep.contains("tx.operations[0].body.clawbackOp.from: " + from));
        QVERIFY(txrep.contains("tx.operations[0].body.clawbackOp.amount: 250000000"));

        QScopedPointer<Transaction> parsed(Txrep::fromTxrep(txrep, Network::testnetNetwork()));
        auto* cb = dynamic_cast<ClawbackOperation*>(parsed->getOperations().at(0));
        QVERIFY(cb != nullptr);
        QCOMPARE(cb->getFrom(), from);
        QCOMPARE(cb->getAmount(), QString("25"));
        QVERIFY(cb->getAsset()->equals(Asset::createNonNativeAsset("USD", KeyPair::fromAccountId(issuer))));

        delete acc;
    }

    void testClawbackClaimableBalanceRoundTrip()
    {
        const QString balanceId =
            "00000000""0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
        KeyPair* source = KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 0);
        QScopedPointer<Transaction> original(
            TransactionBuilder(acc, Network::testnetNetwork())
                .addOperation(ClawbackClaimableBalanceOperation::create(balanceId))
                .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build());

        QString txrep = Txrep::toTxrep(original.data());
        QVERIFY(txrep.contains("tx.operations[0].body.type: CLAWBACK_CLAIMABLE_BALANCE"));
        QVERIFY(txrep.contains("tx.operations[0].body.clawbackClaimableBalanceOp.balanceID: " + balanceId));

        QScopedPointer<Transaction> parsed(Txrep::fromTxrep(txrep, Network::testnetNetwork()));
        auto* ccb = dynamic_cast<ClawbackClaimableBalanceOperation*>(parsed->getOperations().at(0));
        QVERIFY(ccb != nullptr);
        QCOMPARE(ccb->getBalanceId(), balanceId);

        delete acc;
    }

    void testSetTrustLineFlagsRoundTrip()
    {
        const QString issuer  = "GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7";
        const QString trustor = "GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO";
        KeyPair* source = KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 0);
        QScopedPointer<Transaction> original(
            TransactionBuilder(acc, Network::testnetNetwork())
                .addOperation(SetTrustlineFlagsOperation::create(
                    trustor, Asset::createNonNativeAsset("USD", issuer),
                    QFlags<stellar::TrustLineFlags>(QFlag(1)),   // clear: AUTHORIZED
                    QFlags<stellar::TrustLineFlags>(QFlag(4))))  // set: CLAWBACK_ENABLED
                .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build());

        QString txrep = Txrep::toTxrep(original.data());
        QVERIFY(txrep.contains("tx.operations[0].body.type: SET_TRUST_LINE_FLAGS"));
        QVERIFY(txrep.contains("tx.operations[0].body.setTrustLineFlagsOp.trustor: " + trustor));
        QVERIFY(txrep.contains("tx.operations[0].body.setTrustLineFlagsOp.clearFlags: 1"));
        QVERIFY(txrep.contains("tx.operations[0].body.setTrustLineFlagsOp.setFlags: 4"));

        QScopedPointer<Transaction> parsed(Txrep::fromTxrep(txrep, Network::testnetNetwork()));
        auto* stf = dynamic_cast<SetTrustlineFlagsOperation*>(parsed->getOperations().at(0));
        QVERIFY(stf != nullptr);
        QCOMPARE(stf->getTrustor(), trustor);
        QCOMPARE(static_cast<quint32>(stf->getClearFlags()), quint32(1));
        QCOMPARE(static_cast<quint32>(stf->getSetFlags()), quint32(4));
        QVERIFY(stf->getAsset()->equals(Asset::createNonNativeAsset("USD", KeyPair::fromAccountId(issuer))));

        delete acc;
    }

    void testBeginSponsoringFutureReservesRoundTrip()
    {
        const QString sponsored = "GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7";
        KeyPair* source = KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 0);
        QScopedPointer<Transaction> original(
            TransactionBuilder(acc, Network::testnetNetwork())
                .addOperation(BeginSponsoringFutureReservesOperation::create(sponsored))
                .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build());

        QString txrep = Txrep::toTxrep(original.data());
        QVERIFY(txrep.contains("tx.operations[0].body.type: BEGIN_SPONSORING_FUTURE_RESERVES"));
        QVERIFY(txrep.contains("tx.operations[0].body.beginSponsoringFutureReservesOp.sponsoredID: " + sponsored));

        QScopedPointer<Transaction> parsed(Txrep::fromTxrep(txrep, Network::testnetNetwork()));
        auto* bsp = dynamic_cast<BeginSponsoringFutureReservesOperation*>(parsed->getOperations().at(0));
        QVERIFY(bsp != nullptr);
        QCOMPARE(bsp->getSponsoredID(), sponsored);

        delete acc;
    }

    void testEndSponsoringFutureReservesRoundTrip()
    {
        KeyPair* source = KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 0);
        QScopedPointer<Transaction> original(
            TransactionBuilder(acc, Network::testnetNetwork())
                .addOperation(EndSponsoringFutureReservesOperation::create())
                .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build());

        QString txrep = Txrep::toTxrep(original.data());
        QVERIFY(txrep.contains("tx.operations[0].body.type: END_SPONSORING_FUTURE_RESERVES"));

        QScopedPointer<Transaction> parsed(Txrep::fromTxrep(txrep, Network::testnetNetwork()));
        QVERIFY(dynamic_cast<EndSponsoringFutureReservesOperation*>(parsed->getOperations().at(0)) != nullptr);

        delete acc;
    }

    void testPathPaymentStrictReceiveRoundTrip()
    {
        const QString issuer = "GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7";
        KeyPair* source = KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 0);

        QList<Asset*> path;
        path.append(Asset::createNonNativeAsset("BTC", issuer));

        QScopedPointer<Transaction> original(
            TransactionBuilder(acc, Network::testnetNetwork())
                .addOperation(new PathPaymentStrictReceiveOperation(
                    new AssetTypeNative(), "100", issuer,
                    Asset::createNonNativeAsset("USD", issuer), "50", path))
                .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build());

        QString txrep = Txrep::toTxrep(original.data());
        QVERIFY(txrep.contains("tx.operations[0].body.type: PATH_PAYMENT_STRICT_RECEIVE"));
        QVERIFY(txrep.contains("tx.operations[0].body.pathPaymentStrictReceiveOp.sendMax: 1000000000"));
        QVERIFY(txrep.contains("tx.operations[0].body.pathPaymentStrictReceiveOp.destAmount: 500000000"));
        QVERIFY(txrep.contains("tx.operations[0].body.pathPaymentStrictReceiveOp.path.len: 1"));
        QVERIFY(txrep.contains("tx.operations[0].body.pathPaymentStrictReceiveOp.path[0]: BTC:" + issuer));

        QScopedPointer<Transaction> parsed(Txrep::fromTxrep(txrep, Network::testnetNetwork()));
        auto* pp = dynamic_cast<PathPaymentStrictReceiveOperation*>(parsed->getOperations().at(0));
        QVERIFY(pp != nullptr);
        QVERIFY(dynamic_cast<AssetTypeNative*>(pp->getSendAsset()) != nullptr);
        QCOMPARE(pp->getSendMax(), QString("100"));
        QCOMPARE(pp->getDestination(), issuer);
        QVERIFY(pp->getDestAsset()->equals(Asset::createNonNativeAsset("USD", KeyPair::fromAccountId(issuer))));
        QCOMPARE(pp->getDestAmount(), QString("50"));
        QCOMPARE(pp->getPath().size(), 1);
        QVERIFY(pp->getPath().at(0)->equals(Asset::createNonNativeAsset("BTC", KeyPair::fromAccountId(issuer))));

        delete acc;
    }

    void testPathPaymentStrictSendRoundTrip()
    {
        const QString issuer = "GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7";
        KeyPair* source = KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 0);

        QList<Asset*> path;
        path.append(Asset::createNonNativeAsset("BTC", issuer));

        QScopedPointer<Transaction> original(
            TransactionBuilder(acc, Network::testnetNetwork())
                .addOperation(new PathPaymentStrictSendOperation(
                    new AssetTypeNative(), "100", issuer,
                    Asset::createNonNativeAsset("USD", issuer), "50", path))
                .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build());

        QString txrep = Txrep::toTxrep(original.data());
        QVERIFY(txrep.contains("tx.operations[0].body.type: PATH_PAYMENT_STRICT_SEND"));
        QVERIFY(txrep.contains("tx.operations[0].body.pathPaymentStrictSendOp.sendAmount: 1000000000"));
        QVERIFY(txrep.contains("tx.operations[0].body.pathPaymentStrictSendOp.destMin: 500000000"));
        QVERIFY(txrep.contains("tx.operations[0].body.pathPaymentStrictSendOp.path.len: 1"));

        QScopedPointer<Transaction> parsed(Txrep::fromTxrep(txrep, Network::testnetNetwork()));
        auto* pp = dynamic_cast<PathPaymentStrictSendOperation*>(parsed->getOperations().at(0));
        QVERIFY(pp != nullptr);
        QCOMPARE(pp->getSendAmount(), QString("100"));
        QCOMPARE(pp->getDestMin(), QString("50"));
        QCOMPARE(pp->getDestination(), issuer);
        QCOMPARE(pp->getPath().size(), 1);
        QVERIFY(pp->getPath().at(0)->equals(Asset::createNonNativeAsset("BTC", KeyPair::fromAccountId(issuer))));

        delete acc;
    }

    void testManageSellOfferRoundTrip()
    {
        KeyPair* source = KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 0);
        QScopedPointer<Transaction> original(
            TransactionBuilder(acc, Network::testnetNetwork())
                .addOperation(ManageSellOfferOperation::create(
                    new AssetTypeNative(),
                    Asset::createNonNativeAsset("USD",
                        QString("GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7")),
                    "100", "0.5"))
                .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build());

        QString txrep = Txrep::toTxrep(original.data());
        QVERIFY(txrep.contains("tx.operations[0].body.type: MANAGE_SELL_OFFER"));
        QVERIFY(txrep.contains("tx.operations[0].body.manageSellOfferOp.amount: 1000000000"));
        QVERIFY(txrep.contains("tx.operations[0].body.manageSellOfferOp.price.n: 1"));
        QVERIFY(txrep.contains("tx.operations[0].body.manageSellOfferOp.price.d: 2"));
        QVERIFY(txrep.contains("tx.operations[0].body.manageSellOfferOp.offerID: 0"));

        QScopedPointer<Transaction> parsed(Txrep::fromTxrep(txrep, Network::testnetNetwork()));
        auto* mso = dynamic_cast<ManageSellOfferOperation*>(parsed->getOperations().at(0));
        QVERIFY(mso != nullptr);
        QVERIFY(dynamic_cast<AssetTypeNative*>(mso->getSelling()) != nullptr);
        QVERIFY(mso->getBuying()->equals(Asset::createNonNativeAsset("USD",
            KeyPair::fromAccountId(QString("GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7")))));
        QCOMPARE(mso->getAmount(), QString("100"));
        QCOMPARE(mso->getPrice(), QString("0.5"));
        QCOMPARE(mso->getOfferId(), Q_INT64_C(0));

        delete acc;
    }

    void testManageBuyOfferRoundTrip()
    {
        KeyPair* source = KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 0);
        QScopedPointer<Transaction> original(
            TransactionBuilder(acc, Network::testnetNetwork())
                .addOperation(ManageBuyOfferOperation::create(
                    new AssetTypeNative(),
                    Asset::createNonNativeAsset("USD",
                        QString("GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7")),
                    "100", "0.5"))
                .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build());

        QString txrep = Txrep::toTxrep(original.data());
        QVERIFY(txrep.contains("tx.operations[0].body.type: MANAGE_BUY_OFFER"));
        QVERIFY(txrep.contains("tx.operations[0].body.manageBuyOfferOp.buyAmount: 1000000000"));

        QScopedPointer<Transaction> parsed(Txrep::fromTxrep(txrep, Network::testnetNetwork()));
        auto* mbo = dynamic_cast<ManageBuyOfferOperation*>(parsed->getOperations().at(0));
        QVERIFY(mbo != nullptr);
        QCOMPARE(mbo->getAmount(), QString("100"));
        QCOMPARE(mbo->getPrice(), QString("0.5"));
        QCOMPARE(mbo->getOfferId(), Q_INT64_C(0));

        delete acc;
    }

    void testCreatePassiveSellOfferRoundTrip()
    {
        KeyPair* source = KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 0);
        QScopedPointer<Transaction> original(
            TransactionBuilder(acc, Network::testnetNetwork())
                .addOperation(CreatePassiveSellOfferOperation::create(
                    new AssetTypeNative(),
                    Asset::createNonNativeAsset("USD",
                        QString("GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7")),
                    "100", "0.5"))
                .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build());

        QString txrep = Txrep::toTxrep(original.data());
        QVERIFY(txrep.contains("tx.operations[0].body.type: CREATE_PASSIVE_SELL_OFFER"));
        QVERIFY(txrep.contains("tx.operations[0].body.createPassiveSellOfferOp.amount: 1000000000"));
        QVERIFY(txrep.contains("tx.operations[0].body.createPassiveSellOfferOp.price.d: 2"));

        QScopedPointer<Transaction> parsed(Txrep::fromTxrep(txrep, Network::testnetNetwork()));
        auto* cpo = dynamic_cast<CreatePassiveSellOfferOperation*>(parsed->getOperations().at(0));
        QVERIFY(cpo != nullptr);
        QCOMPARE(cpo->getAmount(), QString("100"));
        QCOMPARE(cpo->getPrice(), QString("0.5"));

        delete acc;
    }

    void testChangeTrustRoundTrip()
    {
        KeyPair* source = KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 0);
        QScopedPointer<Transaction> original(
            TransactionBuilder(acc, Network::testnetNetwork())
                .addOperation(ChangeTrustOperation::create(
                    Asset::createNonNativeAsset("USD",
                        QString("GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7")),
                    "1000"))
                .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build());

        QString txrep = Txrep::toTxrep(original.data());
        QVERIFY(txrep.contains("tx.operations[0].body.type: CHANGE_TRUST"));
        QVERIFY(txrep.contains("tx.operations[0].body.changeTrustOp.line.type: ASSET_TYPE_CREDIT_ALPHANUM4"));
        QVERIFY(txrep.contains("tx.operations[0].body.changeTrustOp.line: "
                               "USD:GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7"));

        QScopedPointer<Transaction> parsed(Txrep::fromTxrep(txrep, Network::testnetNetwork()));
        auto* ct = dynamic_cast<ChangeTrustOperation*>(parsed->getOperations().at(0));
        QVERIFY(ct != nullptr);
        QVERIFY(ct->getAsset()->equals(Asset::createNonNativeAsset("USD",
            KeyPair::fromAccountId(QString("GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7")))));
        QCOMPARE(ct->getLimit(), QString("1000"));

        delete acc;
    }

// AllowTrust is deprecated (superseded by SetTrustLineFlags) but txrep still
// round-trips it; suppress the deprecation warning for this one test.
#if defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable: 4996)
#elif defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
    void testAllowTrustRoundTrip()
    {
        KeyPair* source = KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 0);
        QScopedPointer<Transaction> original(
            TransactionBuilder(acc, Network::testnetNetwork())
                .addOperation(AllowTrustOperation::create(
                    KeyPair::fromAccountId("GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7"),
                    "USD", /*authorize*/ true, /*authMaintain*/ false))
                .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build());

        QString txrep = Txrep::toTxrep(original.data());
        QVERIFY(txrep.contains("tx.operations[0].body.type: ALLOW_TRUST"));
        QVERIFY(txrep.contains("tx.operations[0].body.allowTrustOp.trustor: "
                               "GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7"));
        QVERIFY(txrep.contains("tx.operations[0].body.allowTrustOp.asset: USD"));
        QVERIFY(txrep.contains("tx.operations[0].body.allowTrustOp.authorize: true"));

        QScopedPointer<Transaction> parsed(Txrep::fromTxrep(txrep, Network::testnetNetwork()));
        auto* at = dynamic_cast<AllowTrustOperation*>(parsed->getOperations().at(0));
        QVERIFY(at != nullptr);
        QCOMPARE(at->getTrustor()->getAccountId(),
                 QString("GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7"));
        QCOMPARE(at->getAssetCode(), QString("USD"));
        QVERIFY(at->getAuthorize());
        QVERIFY(!at->getAuthorizeToMaintainLiabilities());

        delete acc;
    }

    /** A de-authorising AllowTrust (both flags false) is the one combination
     *  no test built before. `stellar::AllowTrustOp` has no default member
     *  initializer, so if the constructor's if/else-if chain has no final
     *  else, `authorize` keeps whatever was on the heap — and any garbage with
     *  bit 0 set reads back as AUTHORIZED, silently turning a de-authorize
     *  into an authorize. */
    void testAllowTrustDeauthorizeRoundTrip()
    {
        KeyPair* source = KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 0);
        QScopedPointer<Transaction> original(
            TransactionBuilder(acc, Network::testnetNetwork())
                .addOperation(AllowTrustOperation::create(
                    KeyPair::fromAccountId("GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7"),
                    "USD", /*authorize*/ false, /*authMaintain*/ false))
                .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build());

        QString txrep = Txrep::toTxrep(original.data());
        QVERIFY(txrep.contains("tx.operations[0].body.allowTrustOp.authorize: false"));

        QScopedPointer<Transaction> parsed(Txrep::fromTxrep(txrep, Network::testnetNetwork()));
        auto* at = dynamic_cast<AllowTrustOperation*>(parsed->getOperations().at(0));
        QVERIFY(at != nullptr);
        QVERIFY(!at->getAuthorize());
        QVERIFY(!at->getAuthorizeToMaintainLiabilities());

        delete acc;
    }
#if defined(_MSC_VER)
#  pragma warning(pop)
#elif defined(__GNUC__)
#  pragma GCC diagnostic pop
#endif

    void testBumpSequenceRoundTrip()
    {
        KeyPair* source = KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 0);
        QScopedPointer<Transaction> original(
            TransactionBuilder(acc, Network::testnetNetwork())
                .addOperation(BumpSequenceOperation::create(Q_INT64_C(9223372036854775807)))
                .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build());

        QString txrep = Txrep::toTxrep(original.data());
        QVERIFY(txrep.contains("tx.operations[0].body.type: BUMP_SEQUENCE"));
        QVERIFY(txrep.contains("tx.operations[0].body.bumpSequenceOp.bumpTo: 9223372036854775807"));

        QScopedPointer<Transaction> parsed(Txrep::fromTxrep(txrep, Network::testnetNetwork()));
        auto* bs = dynamic_cast<BumpSequenceOperation*>(parsed->getOperations().at(0));
        QVERIFY(bs != nullptr);
        QCOMPARE(bs->getBumpTo(), Q_INT64_C(9223372036854775807));

        delete acc;
    }

    void testSetOptionsRoundTrip()
    {
        KeyPair* source = KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 0);

        SetOptionsOperation* setOpts =
            SetOptionsOperation::create()
                ->setInflationDestination(KeyPair::fromAccountId(
                    "GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7"))
                ->setClearFlags(1)
                ->setSetFlags(2)
                ->setMasterKeyWeight(3)
                ->setLowThreshold(4)
                ->setMediumThreshold(5)
                ->setHighThreshold(6)
                ->setHomeDomain("example.com")
                ->setSigner(SignerKey::ed25519PublicKey(QString(
                    "GCFKT6BN2FEASCEVDNHEC4LLFT2KLUUPEMKM4OJPEJ65H2AEZ7IH4RV6")).toXdr(), 7);

        QScopedPointer<Transaction> original(
            TransactionBuilder(acc, Network::testnetNetwork())
                .addOperation(setOpts)
                .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build());

        QString txrep = Txrep::toTxrep(original.data());
        QVERIFY(txrep.contains("tx.operations[0].body.type: SET_OPTIONS"));
        QVERIFY(txrep.contains("tx.operations[0].body.setOptionsOp.clearFlags: 1"));
        QVERIFY(txrep.contains("tx.operations[0].body.setOptionsOp.highThreshold: 6"));
        QVERIFY(txrep.contains("tx.operations[0].body.setOptionsOp.homeDomain: \"example.com\""));
        QVERIFY(txrep.contains("tx.operations[0].body.setOptionsOp.signer.weight: 7"));

        QScopedPointer<Transaction> parsed(Txrep::fromTxrep(txrep, Network::testnetNetwork()));
        auto* so = dynamic_cast<SetOptionsOperation*>(parsed->getOperations().at(0));
        QVERIFY(so != nullptr);
        QVERIFY(so->getInflationDestination() != nullptr);
        QCOMPARE(so->getInflationDestination()->getAccountId(),
                 QString("GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7"));
        QCOMPARE(so->getClearFlags().value,     quint32(1));
        QCOMPARE(so->getSetFlags().value,       quint32(2));
        QCOMPARE(so->getMasterKeyWeight().value, quint32(3));
        QCOMPARE(so->getLowThreshold().value,   quint32(4));
        QCOMPARE(so->getMediumThreshold().value, quint32(5));
        QCOMPARE(so->getHighThreshold().value,  quint32(6));
        QCOMPARE(so->getHomeDomain(), QString("example.com"));
        QVERIFY(so->getSigner() != nullptr);
        QCOMPARE(SignerKey::fromXdr(*so->getSigner()).toString(),
                 QString("GCFKT6BN2FEASCEVDNHEC4LLFT2KLUUPEMKM4OJPEJ65H2AEZ7IH4RV6"));
        QCOMPARE(so->getSignerWeight().value, quint32(7));

        delete acc;
    }

    // PRECOND_V2 round-trip: every V2 field populated (timeBounds, ledgerBounds,
    // minSeqNum, minSeqAge, minSeqLedgerGap, one extra signer). Previously the
    // emitter threw on any V2 precondition.
    void testPreconditionsV2RoundTrip()
    {
        KeyPair* source = KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");

        TransactionPreconditions pre;
        pre.setTimeBounds(new TimeBounds(100, 200));
        pre.setLedgerBounds(new LedgerBounds(5, 10));
        pre.setMinSeqNumber(new qint64(123456789));
        pre.setMinSeqAge(60);
        pre.setMinSeqLedgerGap(7);
        pre.addExtraSigner(SignerKey::ed25519PublicKey(
            QString("GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7")));

        Account* acc = new Account(source, 0);
        QScopedPointer<Transaction> original(
            TransactionBuilder(acc, Network::testnetNetwork())
                .addOperation(new ManageDataOperation("k", "v"))
                .addPreconditions(pre)
                .setBaseFee(100)
                .build());

        QString txrep = Txrep::toTxrep(original.data());
        QVERIFY(txrep.contains("tx.cond.type: PRECOND_V2"));
        QVERIFY(txrep.contains("tx.cond.minSeqAge: 60"));
        QVERIFY(txrep.contains("tx.cond.minSeqLedgerGap: 7"));
        QVERIFY(txrep.contains("tx.cond.extraSigners.len: 1"));
        QVERIFY(txrep.contains("tx.cond.extraSigners[0].type: SIGNER_KEY_TYPE_ED25519"));
        QVERIFY(txrep.contains("tx.cond.extraSigners[0].ed25519: "
                               "GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7"));

        QScopedPointer<Transaction> parsed(Txrep::fromTxrep(txrep, Network::testnetNetwork()));
        const TransactionPreconditions& p = parsed->getPreconditions();
        QVERIFY(p.hasV2());
        QVERIFY(p.getTimeBounds() != nullptr);
        QCOMPARE(p.getTimeBounds()->getMinTime(), qint64(100));
        QCOMPARE(p.getTimeBounds()->getMaxTime(), qint64(200));
        QVERIFY(p.getLedgerBounds() != nullptr);
        QCOMPARE(p.getLedgerBounds()->getMinLedger(), quint32(5));
        QCOMPARE(p.getLedgerBounds()->getMaxLedger(), quint32(10));
        QVERIFY(p.getMinSeqNumber() != nullptr);
        QCOMPARE(*p.getMinSeqNumber(), qint64(123456789));
        QCOMPARE(p.getMinSeqAge(), quint64(60));
        QCOMPARE(p.getMinSeqLedgerGap(), quint32(7));
        QCOMPARE(p.getExtraSigners().size(), 1);
        QCOMPARE(p.getExtraSigners().at(0).toString(),
                 QString("GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7"));

        delete acc;
    }
};

ADD_TEST(Sep11TxrepTest)
#endif // SEP11TXREPTEST_H
