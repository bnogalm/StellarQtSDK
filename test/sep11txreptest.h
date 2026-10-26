#ifndef SEP11TXREPTEST_H
#define SEP11TXREPTEST_H

#include <QObject>
#include <QtTest>
#include <QRegularExpression>
#include "testcollector.h"

#include "../src/txrep.h"
#include "../src/abstracttransaction.h"
#include "../src/transaction.h"
#include "../src/feebumptransaction.h"
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
#include "../src/changetrustasset.h"
#include "../src/inflationoperation.h"
#include "../src/restorefootprintoperation.h"
#include "../src/muxedaccount.h"
#include <vector>
#if defined(_MSC_VER) && defined(_DEBUG)
#  include <crtdbg.h>     // testARefusedDocumentLeaksNothing counts the debug heap
#endif

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
        // TIMEOUT_INFINITE is time bounds {0, 0}, as js and java write it (it
        // was PRECOND_NONE until 1.0.4).
        QVERIFY(txrep.contains("tx.cond.type: PRECOND_TIME\n"
                               "tx.cond.timeBounds.minTime: 0 (1970-01-01 00:00:00.000000+00:00 (UTC))\n"
                               "tx.cond.timeBounds.maxTime: 0 (1970-01-01 00:00:00.000000+00:00 (UTC))\n"));
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
                .addMemo(Memo::hash(QString("deadbeef00000000000000000000000000000000000000000000000000000000")))
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
                .addMemo(Memo::returnHash(QString("0123456789abcdef000000000000000000000000000000000000000000000000")))
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
        // Bare, as py-stellar-sdk writes and reads it (1.0.4). Quoted, py
        // revoked the name with its quotes in it: another data entry.
        QVERIFY(txrep.contains("revokeSponsorshipOp.ledgerKey.data.dataName: config.memo\n"));
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
        // py-stellar-sdk's form (1.0.4); it read nothing from the single
        // `signer.signerKey: G...` line this SDK wrote before.
        QVERIFY(txrep.contains("revokeSponsorshipOp.signer.signerKey.type: SIGNER_KEY_TYPE_ED25519\n"));
        QVERIFY(txrep.contains("revokeSponsorshipOp.signer.signerKey.ed25519: " + signer + "\n"));
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

    /**
     * A path payment that has been through XDR must still emit as TxRep.
     *
     * It did not. Operation::fromXdr returns the DEPRECATED PathPaymentOperation
     * for PATH_PAYMENT_STRICT_RECEIVE (operation.cpp), while the emitter
     * dispatched on PathPaymentStrictReceiveOperation — and the two are
     * siblings under Operation, unrelated to each other, so the dynamic_cast
     * missed. The result was that the SDK could write an envelope it could not
     * then describe: "Txrep emitter: unsupported operation type", for an
     * operation it fully supports.
     *
     * The existing round-trip test above could not catch it, because it emits
     * from the operation it just constructed and never goes out to XDR and
     * back. This one does exactly that, which is the path every real caller
     * takes: someone hands you a base64 envelope and you want to read it.
     */
    void testPathPaymentSurvivesXdrThenTxrep()
    {
        const QString issuer = "GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7";
        KeyPair* source = KeyPair::fromAccountId(
            "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
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

        const QString envelope = original->toEnvelopeXdrBase64();

        // Out to XDR and back, which is where the class changes underneath us.
        QScopedPointer<AbstractTransaction> decoded(
            AbstractTransaction::fromEnvelopeXdr(envelope, Network::testnetNetwork()));
        auto* plain = dynamic_cast<Transaction*>(decoded.data());
        QVERIFY(plain != nullptr);

        const QString txrep = Txrep::toTxrep(plain);
        QVERIFY(txrep.contains("tx.operations[0].body.type: PATH_PAYMENT_STRICT_RECEIVE"));
        QVERIFY(txrep.contains(
            "tx.operations[0].body.pathPaymentStrictReceiveOp.sendMax: 1000000000"));
        QVERIFY(txrep.contains(
            "tx.operations[0].body.pathPaymentStrictReceiveOp.destAmount: 500000000"));
        QVERIFY(txrep.contains(
            "tx.operations[0].body.pathPaymentStrictReceiveOp.path[0]: BTC:" + issuer));

        // And the text describes the same transaction the bytes did.
        QScopedPointer<Transaction> reparsed(
            Txrep::fromTxrep(txrep, Network::testnetNetwork()));
        QCOMPARE(reparsed->hashHex(), plain->hashHex());
    }

    /**
     * tx.fee is the transaction's fee, exactly. The parser used to hand the
     * builder quint32(fee / opCount) and let it multiply back, so 1000 over
     * three operations came back as 999 — a different fee and a different
     * hash from the document reviewed — and a fee a uint32 cannot hold
     * wrapped. And every integer field fell back to 0 on text that is not a
     * number, so "abc" was rebuilt as 0.
     */
    void testFeeIsExactAndIntegersAreChecked()
    {
        KeyPair* source = KeyPair::fromAccountId(
            "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 0);
        QScopedPointer<Transaction> tx(
            TransactionBuilder(acc, Network::testnetNetwork())
                .addOperation(BumpSequenceOperation::create(qint64(2)))
                .addOperation(BumpSequenceOperation::create(qint64(3)))
                .addOperation(BumpSequenceOperation::create(qint64(4)))
                .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build());
        delete acc;
        const QString txrep = Txrep::toTxrep(tx.data());
        QVERIFY(txrep.contains("tx.fee: 300\n"));

        QString edited = txrep;
        edited.replace("tx.fee: 300\n", "tx.fee: 1000\n");
        QScopedPointer<Transaction> back(Txrep::fromTxrep(edited, Network::testnetNetwork()));
        QCOMPARE(back->getFee(), qint64(1000));
        QCOMPARE(back->toV1Xdr().fee, quint32(1000));
        QVERIFY(Txrep::toTxrep(back.data()).contains("tx.fee: 1000\n"));

        for (const char* bad : {"tx.fee: abc\n", "tx.fee: -1\n", "tx.fee: 4294967296\n",
                                "tx.fee: 12.5\n"}) {
            QString broken = txrep;
            broken.replace("tx.fee: 300\n", bad);
            bool threw = false;
            try { QScopedPointer<Transaction> t(Txrep::fromTxrep(broken, Network::testnetNetwork())); }
            catch (const std::exception&) { threw = true; }
            QVERIFY2(threw, bad);
        }

        QString badOp = txrep;
        QVERIFY(badOp.contains("bumpSequenceOp.bumpTo: 2\n"));
        badOp.replace("bumpSequenceOp.bumpTo: 2\n", "bumpSequenceOp.bumpTo: abc\n");
        bool threw = false;
        try { QScopedPointer<Transaction> t(Txrep::fromTxrep(badOp, Network::testnetNetwork())); }
        catch (const std::exception&) { threw = true; }
        QVERIFY(threw);
    }

    /**
     * The emitter writes py's glosses: each amount followed by its units
     * ("125000000 (12.5)", "1E-7" for one stroop as Python's Decimal has it)
     * and each time bound by its date in UTC. The document is the one
     * py-stellar-sdk 14.1.0's to_txrep writes for this envelope, line for
     * line (py leaves out the final newline), and it reads back.
     */
    void testTheEmitterWritesPysGlosses()
    {
        const QString envelope = QStringLiteral(
            "AAAAAgAAAAADoQe/884Qvh1w3RjnS8CZZ+TWMJulDV8d3IZkElUxuAAAASwAAAAAAAAAAgAAAAEAAAAAZVPxAAAAAABrSdIAAAAAAAAAAAMAAAAAAAAAAQAAAAADoQe/884Qvh1w3RjnS8CZZ+TWMJulDV8d3IZkElUxuAAAAAAAAAAAB3NZQAAAAAAAAAADAAAAAAAAAAFVU0QAAAAAAAOhB7/zzhC+HXDdGOdLwJln5NYwm6UNXx3chmQSVTG4AAAAAAC+vCAAAAABAAAAAgAAAAAAAAAAAAAAAAAAAAEAAAAAA6EHv/POEL4dcN0Y50vAmWfk1jCbpQ1fHdyGZBJVMbgAAAAAAAAAAAAAAAEAAAAAAAAAAA==");
        QScopedPointer<AbstractTransaction> tx(AbstractTransaction::fromEnvelopeXdr(envelope, Network::testnetNetwork()));
        const QString expected = QStringLiteral(
            "type: ENVELOPE_TYPE_TX\n"
            "tx.sourceAccount: GAB2CB576PHBBPQ5ODORRZ2LYCMWPZGWGCN2KDK7DXOIMZASKUY3QZ6Q\n"
            "tx.fee: 300\n"
            "tx.seqNum: 2\n"
            "tx.cond.type: PRECOND_TIME\n"
            "tx.cond.timeBounds.minTime: 1700000000 (2023-11-14 22:13:20.000000+00:00 (UTC))\n"
            "tx.cond.timeBounds.maxTime: 1800000000 (2027-01-15 08:00:00.000000+00:00 (UTC))\n"
            "tx.memo.type: MEMO_NONE\n"
            "tx.operations.len: 3\n"
            "tx.operations[0].sourceAccount._present: false\n"
            "tx.operations[0].body.type: PAYMENT\n"
            "tx.operations[0].body.paymentOp.destination: GAB2CB576PHBBPQ5ODORRZ2LYCMWPZGWGCN2KDK7DXOIMZASKUY3QZ6Q\n"
            "tx.operations[0].body.paymentOp.asset: native\n"
            "tx.operations[0].body.paymentOp.amount: 125000000 (12.5)\n"
            "tx.operations[1].sourceAccount._present: false\n"
            "tx.operations[1].body.type: MANAGE_SELL_OFFER\n"
            "tx.operations[1].body.manageSellOfferOp.selling: native\n"
            "tx.operations[1].body.manageSellOfferOp.buying: USD:GAB2CB576PHBBPQ5ODORRZ2LYCMWPZGWGCN2KDK7DXOIMZASKUY3QZ6Q\n"
            "tx.operations[1].body.manageSellOfferOp.amount: 12500000 (1.25)\n"
            "tx.operations[1].body.manageSellOfferOp.price.n: 1\n"
            "tx.operations[1].body.manageSellOfferOp.price.d: 2\n"
            "tx.operations[1].body.manageSellOfferOp.offerID: 0\n"
            "tx.operations[2].sourceAccount._present: false\n"
            "tx.operations[2].body.type: PAYMENT\n"
            "tx.operations[2].body.paymentOp.destination: GAB2CB576PHBBPQ5ODORRZ2LYCMWPZGWGCN2KDK7DXOIMZASKUY3QZ6Q\n"
            "tx.operations[2].body.paymentOp.asset: native\n"
            "tx.operations[2].body.paymentOp.amount: 1 (1E-7)\n"
            "tx.ext.v: 0\n"
            "signatures.len: 0\n");
        QCOMPARE(Txrep::toTxrep(tx.data()), expected);
        QScopedPointer<AbstractTransaction> back(Txrep::fromTxrepEnvelope(expected, Network::testnetNetwork()));
        QCOMPARE(back->toEnvelopeXdrBase64(), envelope);
    }

    /**
     * Claim predicate times carry py's gloss too: absBefore its date in UTC,
     * and relBefore (seconds after the ledger closes) the date that many
     * seconds after 1970, as py writes it. The document is the one
     * py-stellar-sdk 14.1.0's to_txrep writes for this envelope, line for
     * line (py leaves out the last newline); it reads back, and a gloss
     * stating another time is refused.
     */
    void testTheEmitterGlossesPredicateTimesAsPy()
    {
        const QString envelope = QStringLiteral(
            "AAAAAgAAAAADoQe/884Qvh1w3RjnS8CZZ+TWMJulDV8d3IZkElUxuAAAAGQAAAAAAAAAAgAAAAEAAAAAZVPxAAAAAABrSdIAAAAAAAAAAAEAAAAAAAAADgAAAAAAAAAAAJiWgAAAAAMAAAAAAAAAAIAhFYiyQHpQEpE1YX6+rUa8RXJOM7GklxG7rZZHm1CPAAAABAAAAABrSdIAAAAAAAAAAACAIRWIskB6UBKRNWF+vq1GvEVyTjOxpJcRu62WR5tQjwAAAAUAAAAAAAAOEAAAAAAAAAAAgCEViLJAelASkTVhfr6tRrxFck4zsaSXEbutlkebUI8AAAACAAAAAgAAAAQAAAAAAAAAAAAAAAUAAAAAAAFRgAAAAAAAAAAA");
        QScopedPointer<AbstractTransaction> tx(AbstractTransaction::fromEnvelopeXdr(envelope, Network::testnetNetwork()));
        const QString expected = QStringLiteral(
            "type: ENVELOPE_TYPE_TX\n"
            "tx.sourceAccount: GAB2CB576PHBBPQ5ODORRZ2LYCMWPZGWGCN2KDK7DXOIMZASKUY3QZ6Q\n"
            "tx.fee: 100\n"
            "tx.seqNum: 2\n"
            "tx.cond.type: PRECOND_TIME\n"
            "tx.cond.timeBounds.minTime: 1700000000 (2023-11-14 22:13:20.000000+00:00 (UTC))\n"
            "tx.cond.timeBounds.maxTime: 1800000000 (2027-01-15 08:00:00.000000+00:00 (UTC))\n"
            "tx.memo.type: MEMO_NONE\n"
            "tx.operations.len: 1\n"
            "tx.operations[0].sourceAccount._present: false\n"
            "tx.operations[0].body.type: CREATE_CLAIMABLE_BALANCE\n"
            "tx.operations[0].body.createClaimableBalanceOp.asset: native\n"
            "tx.operations[0].body.createClaimableBalanceOp.amount: 10000000 (1)\n"
            "tx.operations[0].body.createClaimableBalanceOp.claimants.len: 3\n"
            "tx.operations[0].body.createClaimableBalanceOp.claimants[0].type: CLAIMANT_TYPE_V0\n"
            "tx.operations[0].body.createClaimableBalanceOp.claimants[0].v0.destination: GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG\n"
            "tx.operations[0].body.createClaimableBalanceOp.claimants[0].v0.predicate.type: CLAIM_PREDICATE_BEFORE_ABSOLUTE_TIME\n"
            "tx.operations[0].body.createClaimableBalanceOp.claimants[0].v0.predicate.absBefore: 1800000000 (2027-01-15 08:00:00.000000+00:00 (UTC))\n"
            "tx.operations[0].body.createClaimableBalanceOp.claimants[1].type: CLAIMANT_TYPE_V0\n"
            "tx.operations[0].body.createClaimableBalanceOp.claimants[1].v0.destination: GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG\n"
            "tx.operations[0].body.createClaimableBalanceOp.claimants[1].v0.predicate.type: CLAIM_PREDICATE_BEFORE_RELATIVE_TIME\n"
            "tx.operations[0].body.createClaimableBalanceOp.claimants[1].v0.predicate.relBefore: 3600 (1970-01-01 01:00:00.000000+00:00 (UTC))\n"
            "tx.operations[0].body.createClaimableBalanceOp.claimants[2].type: CLAIMANT_TYPE_V0\n"
            "tx.operations[0].body.createClaimableBalanceOp.claimants[2].v0.destination: GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG\n"
            "tx.operations[0].body.createClaimableBalanceOp.claimants[2].v0.predicate.type: CLAIM_PREDICATE_OR\n"
            "tx.operations[0].body.createClaimableBalanceOp.claimants[2].v0.predicate.orPredicates.len: 2\n"
            "tx.operations[0].body.createClaimableBalanceOp.claimants[2].v0.predicate.orPredicates[0].type: CLAIM_PREDICATE_BEFORE_ABSOLUTE_TIME\n"
            "tx.operations[0].body.createClaimableBalanceOp.claimants[2].v0.predicate.orPredicates[0].absBefore: 0 (1970-01-01 00:00:00.000000+00:00 (UTC))\n"
            "tx.operations[0].body.createClaimableBalanceOp.claimants[2].v0.predicate.orPredicates[1].type: CLAIM_PREDICATE_BEFORE_RELATIVE_TIME\n"
            "tx.operations[0].body.createClaimableBalanceOp.claimants[2].v0.predicate.orPredicates[1].relBefore: 86400 (1970-01-02 00:00:00.000000+00:00 (UTC))\n"
            "tx.ext.v: 0\n"
            "signatures.len: 0\n");
        QCOMPARE(Txrep::toTxrep(tx.data()), expected);
        QScopedPointer<AbstractTransaction> back(Txrep::fromTxrepEnvelope(expected, Network::testnetNetwork()));
        QCOMPARE(back->toEnvelopeXdrBase64(), envelope);

        // A day off, on either kind of predicate time.
        const QStringList pairs = QStringList()
            << QStringLiteral("absBefore: 1800000000 (2027-01-15") << QStringLiteral("absBefore: 1800000000 (2027-01-16")
            << QStringLiteral("relBefore: 86400 (1970-01-02") << QStringLiteral("relBefore: 86400 (1970-01-03");
        for (int i = 0; i < pairs.size(); i += 2) {
            QString wrong = expected;
            wrong.replace(pairs.at(i), pairs.at(i + 1));
            QVERIFY(wrong != expected);
            bool threw = false;
            try { QScopedPointer<AbstractTransaction> t(Txrep::fromTxrepEnvelope(wrong, Network::testnetNetwork())); }
            catch (const std::exception&) { threw = true; }
            QVERIFY2(threw, qPrintable(pairs.at(i + 1)));
        }
    }

    /**
     * A predicate time past year 9999, which py cannot write, and one below
     * 0, which core refuses, are the number alone; the last second of 9999
     * has its date. All read back.
     */
    void testPredicateTimesWithNoDateAreWrittenBare()
    {
        const QString destination = QStringLiteral("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO");
        KeyPair* source = KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 0);
        QList<Claimant> claimants;
        claimants.append(Claimant(destination, new Predicate::AbsBefore(Q_INT64_C(253402300799))));
        claimants.append(Claimant(destination, new Predicate::AbsBefore(Q_INT64_C(253402300800))));
        claimants.append(Claimant(destination, new Predicate::RelBefore(Q_INT64_C(-1))));
        QScopedPointer<Transaction> tx(TransactionBuilder(acc, Network::testnetNetwork())
            .addOperation(CreateClaimableBalanceOperation::create(QStringLiteral("10"), new AssetTypeNative(), claimants))
            .setTimeout(TransactionBuilder::TIMEOUT_INFINITE).setBaseFee(100).build());
        delete acc;
        const QString doc = Txrep::toTxrep(tx.data());
        const QString p = QStringLiteral("tx.operations[0].body.createClaimableBalanceOp.claimants");
        QVERIFY2(doc.contains(p + QStringLiteral("[0].v0.predicate.absBefore: 253402300799 (9999-12-31 23:59:59.000000+00:00 (UTC))\n")),
                 qPrintable(doc));
        QVERIFY2(doc.contains(p + QStringLiteral("[1].v0.predicate.absBefore: 253402300800\n")), qPrintable(doc));
        QVERIFY2(doc.contains(p + QStringLiteral("[2].v0.predicate.relBefore: -1\n")), qPrintable(doc));
        QScopedPointer<Transaction> back(Txrep::fromTxrep(doc, Network::testnetNetwork()));
        QCOMPARE(back->hashHex(), tx->hashHex());
    }

    /**
     * Time bounds are uint64 in the XDR, and read and written as such. The
     * emitter used to print a decoded UINT64_MAX as "-1", another tool's
     * "18446744073709551615" was refused, and a "-1" typed by hand was
     * accepted and signed as a time 584 billion years out.
     */
    void testTimeBoundsAreUnsignedBothWays()
    {
        KeyPair* source = KeyPair::fromAccountId(
            "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 0);
        QScopedPointer<Transaction> tx(
            TransactionBuilder(acc, Network::testnetNetwork())
                .addOperation(BumpSequenceOperation::create(qint64(2)))
                .setTimeout(300)
                .setBaseFee(100)
                .build());
        delete acc;
        const QString txrep = Txrep::toTxrep(tx.data());
        // The number and its gloss (the date), both replaced.
        const QRegularExpression maxTime(QStringLiteral("tx\\.cond\\.timeBounds\\.maxTime: \\d+[^\\n]*"));
        QVERIFY(txrep.contains(maxTime));

        QString unbounded = txrep;
        unbounded.replace(maxTime, QStringLiteral("tx.cond.timeBounds.maxTime: 18446744073709551615"));
        QScopedPointer<Transaction> parsed(Txrep::fromTxrep(unbounded, Network::testnetNetwork()));
        const QString again = Txrep::toTxrep(parsed.data());
        QVERIFY(again.contains(QStringLiteral("tx.cond.timeBounds.maxTime: 18446744073709551615\n")));
        QScopedPointer<Transaction> reparsed(Txrep::fromTxrep(again, Network::testnetNetwork()));
        QCOMPARE(reparsed->hashHex(), parsed->hashHex());

        QString negative = txrep;
        negative.replace(QRegularExpression(QStringLiteral("tx\\.cond\\.timeBounds\\.minTime: \\d+[^\\n]*")),
                         QStringLiteral("tx.cond.timeBounds.minTime: -1"));
        bool threw = false;
        try { QScopedPointer<Transaction> t(Txrep::fromTxrep(negative, Network::testnetNetwork())); }
        catch (const std::exception&) { threw = true; }
        QVERIFY(threw);
    }

    /**
     * A document listing more entries than its .len says was signed with the
     * fewer: here a path payment whose one hop is listed but whose .len says
     * 0 — signed with no path, looking complete to anybody reading it.
     */
    void testEntriesPastTheirLengthAreRefused()
    {
        const QString issuer = "GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7";
        KeyPair* source = KeyPair::fromAccountId(
            "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 0);
        QList<Asset*> path;
        path.append(Asset::createNonNativeAsset("BTC", issuer));
        QScopedPointer<Transaction> tx(
            TransactionBuilder(acc, Network::testnetNetwork())
                .addOperation(new PathPaymentStrictReceiveOperation(
                    new AssetTypeNative(), "100", issuer,
                    Asset::createNonNativeAsset("USD", issuer), "50", path))
                .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build());
        delete acc;
        const QString txrep = Txrep::toTxrep(tx.data());
        QVERIFY(txrep.contains("pathPaymentStrictReceiveOp.path.len: 1\n"));

        QString shorter = txrep;
        shorter.replace("pathPaymentStrictReceiveOp.path.len: 1\n",
                        "pathPaymentStrictReceiveOp.path.len: 0\n");
        bool threw = false;
        try { QScopedPointer<Transaction> t(Txrep::fromTxrep(shorter, Network::testnetNetwork())); }
        catch (const std::exception&) { threw = true; }
        QVERIFY(threw);

        // Past a gap too: one hop said, hops [0] and [2] listed.
        QString gap = txrep;
        gap.append("tx.operations[0].body.pathPaymentStrictReceiveOp.path[2]: XLM\n");
        threw = false;
        try { QScopedPointer<Transaction> t(Txrep::fromTxrep(gap, Network::testnetNetwork())); }
        catch (const std::exception&) { threw = true; }
        QVERIFY(threw);

        QString fewerOps = txrep;
        fewerOps.replace("tx.operations.len: 1\n", "tx.operations.len: 0\n");
        threw = false;
        try { QScopedPointer<Transaction> t(Txrep::fromTxrep(fewerOps, Network::testnetNetwork())); }
        catch (const std::exception&) { threw = true; }
        QVERIFY(threw);
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

        // Anything but the five known spellings used to fall through to
        // "false" — a revocation, signed for a document that said otherwise.
        for (const char* bad : {"TRUE", "3", "abc", "yes"}) {
            QString broken = txrep;
            broken.replace("allowTrustOp.authorize: true", QStringLiteral("allowTrustOp.authorize: ")
                                                               + QString::fromLatin1(bad));
            bool threw = false;
            try { QScopedPointer<Transaction> t(Txrep::fromTxrep(broken, Network::testnetNetwork())); }
            catch (const std::exception&) { threw = true; }
            QVERIFY2(threw, bad);
        }

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

    // ─── Every line is read, and none is guessed ────────────────────

    static QString documentWithMemoTimeAndSource()
    {
        KeyPair* source = KeyPair::fromAccountId(
            "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 0);
        PaymentOperation* pay = PaymentOperation::create(
            QStringLiteral("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"),
            new AssetTypeNative(), QStringLiteral("1000"));
        pay->setSourceAccount(QStringLiteral("GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7"));
        QScopedPointer<Transaction> tx(
            TransactionBuilder(acc, Network::testnetNetwork())
                .addOperation(pay)
                .addMemo(Memo::text(QStringLiteral("12345")))
                .addTimeBounds(new TimeBounds(0, 1800000000))
                .setBaseFee(100)
                .build());
        delete acc;
        return Txrep::toTxrep(tx.data());
    }

    static bool txrepRefused(const QString& document)
    {
        try {
            QScopedPointer<Transaction> t(Txrep::fromTxrep(document, Network::testnetNetwork()));
            return false;
        } catch (const std::exception&) {
            return true;
        }
    }

    static QString withoutLineStarting(const QString& document, const QString& start)
    {
        QStringList out;
        for (const QString& line : document.split('\n'))
            if (!line.startsWith(start))
                out.append(line);
        return out.join('\n');
    }

    /**
     * A missing type or _present line used to mean "none", and "TRUE" or "1"
     * meant false: the memo, the time bounds or an operation's source account
     * were dropped from what was signed while the document still showed them.
     * A line nothing reads, or a key given twice, was ignored or won.
     */
    void testEveryLineIsReadAndNoneIsGuessed()
    {
        const QString doc = documentWithMemoTimeAndSource();
        QVERIFY(!txrepRefused(doc));

        QVERIFY(txrepRefused(withoutLineStarting(doc, QStringLiteral("tx.memo.type:"))));
        QVERIFY(txrepRefused(withoutLineStarting(doc, QStringLiteral("tx.cond.type:"))));
        QVERIFY(txrepRefused(withoutLineStarting(doc, QStringLiteral("tx.operations[0].sourceAccount._present:"))));
        QString upper = doc;
        upper.replace(QStringLiteral("sourceAccount._present: true"), QStringLiteral("sourceAccount._present: TRUE"));
        QVERIFY(upper != doc);
        QVERIFY(txrepRefused(upper));

        QString none = doc;
        none.replace(QStringLiteral("tx.memo.type: MEMO_TEXT"), QStringLiteral("tx.memo.type: MEMO_NONE"));
        QVERIFY(txrepRefused(none));                         // the text line is still there

        QVERIFY(txrepRefused(doc + QStringLiteral("tx.fee: 4000000000\n")));     // given twice
        QVERIFY(txrepRefused(doc + QStringLiteral("tx.timeBounds.maxTime: 1\n")));
        QVERIFY(txrepRefused(doc + QStringLiteral("not a key value line\n")));
    }

    /** A memo hash is exactly 32 bytes: a short one was padded with zeros. */
    void testAMemoHashIsExactly32Bytes()
    {
        QString doc = documentWithMemoTimeAndSource();
        doc.replace(QStringLiteral("tx.memo.type: MEMO_TEXT\ntx.memo.text: \"12345\""),
                    QStringLiteral("tx.memo.type: MEMO_HASH\ntx.memo.hash: ") + QString(62, QLatin1Char('a')));
        QVERIFY(doc.contains(QStringLiteral("MEMO_HASH")));
        QVERIFY(txrepRefused(doc));
        doc.replace(QString(62, QLatin1Char('a')), QString(64, QLatin1Char('a')));
        QVERIFY(!txrepRefused(doc));
    }

    /** A memo text that is not UTF-8 is shown as hex (as py-stellar-sdk does)
     *  and read back as the same bytes; a newline stays on its line. */
    void testMemoTextBytesRoundTrip()
    {
        KeyPair* source = KeyPair::fromAccountId(
            "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        for (const QByteArray& bytes : {QByteArray("12345\xff", 6), QByteArray("a\nb\"c\\d")}) {
            Account* acc = new Account(KeyPair::fromAccountId(source->getAccountId()), 0);
            QScopedPointer<Transaction> tx(
                TransactionBuilder(acc, Network::testnetNetwork())
                    .addOperation(BumpSequenceOperation::create(qint64(2)))
                    .addMemo(MemoText::fromBytes(bytes))
                    .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                    .setBaseFee(100)
                    .build());
            delete acc;
            const QString doc = Txrep::toTxrep(tx.data());
            QVERIFY(!doc.contains(QStringLiteral("a\nb")));
            QScopedPointer<Transaction> back(Txrep::fromTxrep(doc, Network::testnetNetwork()));
            QCOMPARE(back->hashHex(), tx->hashHex());
            QCOMPARE(dynamic_cast<MemoText*>(back->getMemo())->getBytes(), bytes);
        }
        delete source;
        QVERIFY(Txrep::toTxrep(QScopedPointer<Transaction>(
            TransactionBuilder(new Account(KeyPair::fromAccountId(
                "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"), 0),
                Network::testnetNetwork())
                .addOperation(BumpSequenceOperation::create(qint64(2)))
                .addMemo(MemoText::fromBytes(QByteArray("12345\xff", 6)))
                .setTimeout(TransactionBuilder::TIMEOUT_INFINITE).setBaseFee(100).build()).data())
                    .contains(QStringLiteral("tx.memo.text: 3132333435ff\n")));
    }

    /** What py-stellar-sdk writes and what SEP-11 allows, read; what would
     *  show one thing and sign another, refused. */
    void testInteroperableButNotAmbiguous()
    {
        const QString doc = documentWithMemoTimeAndSource();

        // SEP-11 ':' comment lines, as '#' ones.
        QVERIFY(!txrepRefused(QStringLiteral(": a comment line\n") + doc));
        // An empty memo text is a memo, "".
        QString empty = doc;
        empty.replace(QStringLiteral("tx.memo.text: \"12345\""), QStringLiteral("tx.memo.text: \"\""));
        QVERIFY(!txrepRefused(empty));
        // Bare: only for bytes that are not UTF-8. "12345678" is text.
        QString bare = doc;
        bare.replace(QStringLiteral("tx.memo.text: \"12345\""), QStringLiteral("tx.memo.text: 12345678"));
        QVERIFY(txrepRefused(bare));
        // A leading zero means octal to a C reader, decimal to this one.
        QString octal = doc;
        octal.replace(QStringLiteral("tx.fee: 100\n"), QStringLiteral("tx.fee: 0144\n"));
        QVERIFY(octal != doc);
        QVERIFY(txrepRefused(octal));
        // Invisible or line-reordering characters, raw, are refused.
        QString bidi = doc;
        bidi.replace(QStringLiteral("\"12345\""), QStringLiteral("\"") + QChar(0x202e) + QStringLiteral("54321\""));
        QVERIFY(txrepRefused(bidi));
        // ... and this emitter writes them escaped, so its output reads back.
        KeyPair* source = KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 0);
        QScopedPointer<Transaction> tx(TransactionBuilder(acc, Network::testnetNetwork())
            .addOperation(BumpSequenceOperation::create(qint64(2)))
            .addMemo(Memo::text(QChar(0x202e) + QStringLiteral("54321")))
            .setTimeout(TransactionBuilder::TIMEOUT_INFINITE).setBaseFee(100).build());
        delete acc;
        const QString emitted = Txrep::toTxrep(tx.data());
        QVERIFY(emitted.contains(QStringLiteral("\\u202e54321")));
        QScopedPointer<Transaction> back(Txrep::fromTxrep(emitted, Network::testnetNetwork()));
        QCOMPARE(back->hashHex(), tx->hashHex());
    }

    /**
     * A lone CR (not the first half of a CRLF) was taken anywhere, comment
     * lines included. Editors and terminals show it as a line break or go back
     * to the start of the line, so "# seen\r<destination line>" showed a
     * destination the parser never read (the real one, further down, was
     * signed). The invisible-character table also missed format characters
     * that draw nothing (soft hyphen, Arabic letter mark, Mongolian vowel
     * separator, fillers, language tags), and did not see past the BMP.
     */
    void testNoLoneCrAndNoInvisibleCharacter()
    {
        const QString doc = documentWithMemoTimeAndSource();
        QVERIFY(!txrepRefused(doc));
        // CRLF line ends are still a document.
        QString crlf = doc;
        crlf.replace(QStringLiteral("\n"), QStringLiteral("\r\n"));
        QScopedPointer<Transaction> lf(Txrep::fromTxrep(doc, Network::testnetNetwork()));
        QScopedPointer<Transaction> fromCrlf(Txrep::fromTxrep(crlf, Network::testnetNetwork()));
        QCOMPARE(fromCrlf->hashHex(), lf->hashHex());
        // A CR inside a comment line, or a ':' comment line, is refused.
        QVERIFY(txrepRefused(QStringLiteral("# seen\rtx.fee: 999\n") + doc));
        QVERIFY(txrepRefused(QStringLiteral(": seen\rtx.fee: 999\n") + doc));
        QVERIFY(txrepRefused(QStringLiteral("# seen\r\r\n") + doc));
        // ... and raw inside a string.
        QString memoCr = doc;
        memoCr.replace(QStringLiteral("\"12345\""), QStringLiteral("\"123\r45\""));
        QVERIFY(memoCr != doc);
        QVERIFY(txrepRefused(memoCr));
        // Escaped, a CR is text, and reads.
        QString memoEscapedCr = doc;
        memoEscapedCr.replace(QStringLiteral("\"12345\""), QStringLiteral("\"123\\r45\""));
        QVERIFY(!txrepRefused(memoEscapedCr));

        const QList<uint> hidden = {0xad, 0x34f, 0x61c, 0x115f, 0x17b4, 0x180e, 0x3164, 0xffa0,
                                    0xfff9, 0x1bca0, 0x1d173, 0xe0001, 0xe0031, 0xe007f};
        const QString source = QStringLiteral("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        for (const uint c : hidden) {
            const QString ch = QString::fromUcs4(&c, 1);
            // Raw in a string, or in a comment line: refused.
            QString raw = doc;
            raw.replace(QStringLiteral("\"12345\""), QStringLiteral("\"123") + ch + QStringLiteral("45\""));
            QVERIFY2(txrepRefused(raw), qPrintable(QString::number(c, 16)));
            QVERIFY2(txrepRefused(QStringLiteral("# 1") + ch + QStringLiteral("2\n") + doc),
                     qPrintable(QString::number(c, 16)));
            // Written escaped (a surrogate pair as two escapes), it reads back.
            Account* acc = new Account(KeyPair::fromAccountId(source), 0);
            QScopedPointer<Transaction> tx(TransactionBuilder(acc, Network::testnetNetwork())
                .addOperation(BumpSequenceOperation::create(qint64(2)))
                .addMemo(Memo::text(QStringLiteral("123") + ch + QStringLiteral("45")))
                .setTimeout(TransactionBuilder::TIMEOUT_INFINITE).setBaseFee(100).build());
            delete acc;
            const QString emitted = Txrep::toTxrep(tx.data());
            QString escape;
            for (const QChar unit : ch)
                escape += QString::asprintf("\\u%04x", unit.unicode());
            QVERIFY2(emitted.contains(QStringLiteral("\"123") + escape + QStringLiteral("45\"")),
                     qPrintable(QString::number(c, 16)));
            QScopedPointer<Transaction> back(Txrep::fromTxrep(emitted, Network::testnetNetwork()));
            QCOMPARE(back->hashHex(), tx->hashHex());
        }

        // A variation selector only picks an emoji's look: real memos carry
        // one, raw, as py writes them; it is read and written as it is.
        const QString heart = QString::fromUtf8("\xe2\x9d\xa4\xef\xb8\x8f");
        QString emoji = doc;
        emoji.replace(QStringLiteral("\"12345\""), QStringLiteral("\"") + heart + QStringLiteral("\""));
        QVERIFY(!txrepRefused(emoji));
        Account* acc = new Account(KeyPair::fromAccountId(source), 0);
        QScopedPointer<Transaction> tx(TransactionBuilder(acc, Network::testnetNetwork())
            .addOperation(BumpSequenceOperation::create(qint64(2)))
            .addMemo(Memo::text(heart))
            .setTimeout(TransactionBuilder::TIMEOUT_INFINITE).setBaseFee(100).build());
        delete acc;
        QVERIFY(Txrep::toTxrep(tx.data()).contains(QStringLiteral("\"") + heart + QStringLiteral("\"")));
    }

    /**
     * The UTF-8 check was a round trip through QString::fromUtf8, which drops
     * a leading byte-order mark: a memo "\ufeffhi" was written as bare hex
     * (py writes the escaped text), and a home domain starting with one could
     * not be written at all. Both are written as py writes them now, and read
     * back as the same envelope.
     */
    void testALeadingByteOrderMarkIsText()
    {
        const QByteArray bom("\xEF\xBB\xBF");
        auto build = [&](Operation* op, Memo* memo) {
            Account* acc = new Account(KeyPair::fromAccountId(QStringLiteral("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG")), 0);
            TransactionBuilder builder(acc, Network::testnetNetwork());
            builder.addOperation(op).setTimeout(TransactionBuilder::TIMEOUT_INFINITE).setBaseFee(100);
            if (memo)
                builder.addMemo(memo);
            Transaction* tx = builder.build();
            delete acc;
            return tx;
        };
        QScopedPointer<Transaction> memoTx(build(BumpSequenceOperation::create(qint64(2)),
                                                 MemoText::fromBytes(bom + QByteArray("hi"))));
        const QString memoDoc = Txrep::toTxrep(memoTx.data());
        QVERIFY2(memoDoc.contains(QStringLiteral("tx.memo.text: \"\\ufeffhi\"")), qPrintable(memoDoc));
        QScopedPointer<Transaction> memoBack(Txrep::fromTxrep(memoDoc, Network::testnetNetwork()));
        QCOMPARE(memoBack->hashHex(), memoTx->hashHex());
        // Two of them: only the first was kept, and the document refused.
        QScopedPointer<Transaction> twoTx(build(BumpSequenceOperation::create(qint64(2)),
                                                MemoText::fromBytes(bom + bom + QByteArray("hi"))));
        const QString twoDoc = Txrep::toTxrep(twoTx.data());
        QVERIFY2(twoDoc.contains(QStringLiteral("tx.memo.text: \"\\ufeff\\ufeffhi\"")), qPrintable(twoDoc));
        QScopedPointer<Transaction> twoBack(Txrep::fromTxrep(twoDoc, Network::testnetNetwork()));
        QCOMPARE(twoBack->hashHex(), twoTx->hashHex());

        SetOptionsOperation* options = SetOptionsOperation::create();
        options->setHomeDomain(QString(QChar(0xfeff)) + QStringLiteral("bom.org"));
        QScopedPointer<Transaction> domainTx(build(options, nullptr));
        const QString domainDoc = Txrep::toTxrep(domainTx.data());
        QVERIFY2(domainDoc.contains(QStringLiteral("setOptionsOp.homeDomain: \"\\ufeffbom.org\"")), qPrintable(domainDoc));
        QScopedPointer<Transaction> domainBack(Txrep::fromTxrep(domainDoc, Network::testnetNetwork()));
        QCOMPARE(domainBack->hashHex(), domainTx->hashHex());

        // Not UTF-8 is still hex: an overlong form, a lone continuation byte.
        QScopedPointer<Transaction> hexTx(build(BumpSequenceOperation::create(qint64(2)),
                                                MemoText::fromBytes(QByteArray("\xC0\xAF", 2))));
        QVERIFY(Txrep::toTxrep(hexTx.data()).contains(QStringLiteral("tx.memo.text: c0af")));
    }

    /** py writes a NOT predicate with no _present line; it was refused. */
    void testANotPredicateWithoutItsPresentLineReads()
    {
        KeyPair* source = KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 0);
        QList<Claimant> claimants;
        claimants.append(Claimant(QStringLiteral("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"),
                                  new Predicate::Not(new Predicate::RelBefore(86400))));
        QScopedPointer<Transaction> tx(TransactionBuilder(acc, Network::testnetNetwork())
            .addOperation(new CreateClaimableBalanceOperation(QStringLiteral("10"), new AssetTypeNative(), claimants))
            .setTimeout(TransactionBuilder::TIMEOUT_INFINITE).setBaseFee(100).build());
        delete acc;
        const QString doc = Txrep::toTxrep(tx.data());
        QVERIFY(doc.contains(QStringLiteral("notPredicate._present: true")));
        const QString pyStyle = withoutLineStarting(doc,
            QStringLiteral("tx.operations[0].body.createClaimableBalanceOp.claimants[0].v0.predicate.notPredicate._present"));
        QScopedPointer<Transaction> back(Txrep::fromTxrep(pyStyle, Network::testnetNetwork()));
        QCOMPARE(back->hashHex(), tx->hashHex());
    }

    /** A price is above zero; an extra signer is of the type its line names. */
    void testValuesCoreRefusesAreRefused()
    {
        KeyPair* source = KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 0);
        QScopedPointer<Transaction> tx(TransactionBuilder(acc, Network::testnetNetwork())
            .addOperation(new ManageSellOfferOperation(new AssetTypeNative(),
                Asset::create(QStringLiteral("USD"), QStringLiteral("GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7")),
                QStringLiteral("10"), Price(1, 2), 0))
            .setTimeout(TransactionBuilder::TIMEOUT_INFINITE).setBaseFee(100).build());
        delete acc;
        QString doc = Txrep::toTxrep(tx.data());
        QVERIFY(!txrepRefused(doc));
        QString zero = doc;
        zero.replace(QStringLiteral("price.n: 1\n"), QStringLiteral("price.n: 0\n"));
        QVERIFY(zero != doc);
        QVERIFY(txrepRefused(zero));
        QString code = doc;
        code.replace(QStringLiteral("USD:"), QStringLiteral("U D:"));
        QVERIFY(txrepRefused(code));
    }

    static QString paymentDocument(const QString& condition)
    {
        return QStringLiteral(
            "type: ENVELOPE_TYPE_TX\n"
            "tx.sourceAccount: GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H\n"
            "tx.fee: 100\n"
            "tx.seqNum: 1234567890\n")
            + condition + QStringLiteral(
            "tx.memo.type: MEMO_NONE\n"
            "tx.operations.len: 1\n"
            "tx.operations[0].sourceAccount._present: false\n"
            "tx.operations[0].body.type: PAYMENT\n"
            "tx.operations[0].body.paymentOp.destination: GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR\n"
            "tx.operations[0].body.paymentOp.asset: native\n"
            "tx.operations[0].body.paymentOp.amount: 100000000\n"
            "tx.ext.v: 0\n"
            "signatures.len: 0\n");
    }

    /** setTimeout(TIMEOUT_INFINITE) now writes time bounds {0, 0}. A document
     *  with none (PRECOND_NONE, or V2 without timeBounds) must still be rebuilt
     *  with none: through setTimeout it would get {0, 0} and another hash than
     *  the one reviewed. The hashes are js-stellar-sdk 17.1.0's for these. */
    void testADocumentWithoutTimeBoundsKeepsItsHash()
    {
        const QString none = paymentDocument(QStringLiteral("tx.cond.type: PRECOND_NONE\n"));
        QScopedPointer<Transaction> tx(Txrep::fromTxrep(none, Network::testnetNetwork()));
        QVERIFY(tx->getTimeBounds() == nullptr);
        QCOMPARE(tx->hashHex(),
                 QStringLiteral("c1a6cb3c5f1b1038b93e50c015e9007575b2e713bf6b8c191f315f794d3dc105"));
        QVERIFY(Txrep::toTxrep(tx.data()).contains(QStringLiteral("tx.cond.type: PRECOND_NONE\n")));

        const QString v2 = paymentDocument(QStringLiteral(
            "tx.cond.type: PRECOND_V2\n"
            "tx.cond.timeBounds._present: false\n"
            "tx.cond.ledgerBounds._present: false\n"
            "tx.cond.minSeqNum._present: false\n"
            "tx.cond.minSeqAge: 7\n"
            "tx.cond.minSeqLedgerGap: 0\n"
            "tx.cond.extraSigners.len: 0\n"));
        QScopedPointer<Transaction> tx2(Txrep::fromTxrep(v2, Network::testnetNetwork()));
        QVERIFY(tx2->getTimeBounds() == nullptr);
        QCOMPARE(tx2->hashHex(),
                 QStringLiteral("2463fc2baf41990e14ff04ace6063fb121d8fce18bc59b832e14b9a60fe0822b"));

        // Time bounds {0, 0} are read as written, too.
        QScopedPointer<Transaction> tx3(Txrep::fromTxrep(paymentDocument(QStringLiteral(
            "tx.cond.type: PRECOND_TIME\n"
            "tx.cond.timeBounds.minTime: 0\n"
            "tx.cond.timeBounds.maxTime: 0\n")), Network::testnetNetwork()));
        QCOMPARE(tx3->hashHex(),
                 QStringLiteral("312611e786b70b32367f1f2c2ed245d19fe674ba9bf4c24936ec759339774d89"));
    }

    /** "-0" was read as 0 in tx.fee and every other unsigned field (the
     *  uint64 ones refused it). SEP-11 never writes it: refused everywhere. */
    void testMinusZeroIsRefused()
    {
        const QString v2 = paymentDocument(QStringLiteral(
            "tx.cond.type: PRECOND_V2\n"
            "tx.cond.timeBounds._present: false\n"
            "tx.cond.ledgerBounds._present: false\n"
            "tx.cond.minSeqNum._present: false\n"
            "tx.cond.minSeqAge: 7\n"
            "tx.cond.minSeqLedgerGap: 0\n"
            "tx.cond.extraSigners.len: 0\n"));
        QVERIFY(!txrepRefused(v2));

        const QStringList lines = {QStringLiteral("tx.fee: 100\n"),
                                   QStringLiteral("tx.cond.minSeqLedgerGap: 0\n"),
                                   QStringLiteral("tx.ext.v: 0\n"),
                                   QStringLiteral("tx.seqNum: 1234567890\n")};
        for (const QString& line : lines) {
            const QString key = line.left(line.indexOf(QLatin1Char(':')) + 1);
            for (const QString& bad : {QStringLiteral(" -0\n"), QStringLiteral("  -0 \n")}) {
                QString edited = v2;
                edited.replace(line, key + bad);
                QVERIFY(edited != v2);
                QVERIFY2(txrepRefused(edited), qPrintable(key + bad));
            }
        }
        // A negative value still reads where the field is signed.
        QString negative = v2;
        negative.replace(QStringLiteral("tx.seqNum: 1234567890\n"), QStringLiteral("tx.seqNum: -5\n"));
        QScopedPointer<Transaction> tx(Txrep::fromTxrep(negative, Network::testnetNetwork()));
        QCOMPARE(tx->getSequenceNumber(), qint64(-5));
    }

    // ─── 1.0.4: muxed accounts, pool shares, py's forms, leaks, py corpus ───

    /**
     * Every operation and union arm TxRep covers, as py-stellar-sdk 14.1.0
     * writes it (pyCorpus() below). Read here, each document is the envelope
     * py built, byte for byte. Written here from that envelope, the document
     * gives py what py's own gives it, so py signs the same transaction.
     * Before 1.0.4 the muxed document (its M... source), INFLATION and the
     * pool-share ChangeTrust were refused, and py read this SDK's
     * revoke-signer lines as nothing and its quoted revoke-data name with the
     * quotes in it.
     */
    void testPyStellarSdkCorpusBothWays()
    {
        QVERIFY(pyCorpus().size() >= 20);
        for (const PyCorpusCase& c : pyCorpus()) {
            const QString envelope = QString::fromLatin1(c.envelope);
            QScopedPointer<Transaction> read(Txrep::fromTxrep(QString::fromLatin1(c.txrep),
                                                              Network::testnetNetwork()));
            QVERIFY2(read->toEnvelopeXdrBase64() == envelope, c.name);
            QVERIFY2(read->hashHex() == QLatin1String(c.hash), c.name);

            QScopedPointer<AbstractTransaction> decoded(
                AbstractTransaction::fromEnvelopeXdr(envelope, Network::testnetNetwork()));
            Transaction* tx = dynamic_cast<Transaction*>(decoded.data());
            QVERIFY2(tx != nullptr, c.name);
            const QString ours = Txrep::toTxrep(tx);
            QMap<QString, QString> mine = pyReads(ours);
            QMap<QString, QString> theirs = pyReads(QString::fromLatin1(c.txrep));
            // Written here (SEP-11's pointer marker); py neither writes nor reads it.
            for (const QString& key : mine.keys()) {
                if (key.endsWith(QLatin1String(".notPredicate._present")))
                    mine.remove(key);
            }
            if (qstrcmp(c.name, "memo_text_unicode") == 0) {
                // py writes the letters outside ASCII as JSON escapes, and reads
                // them back as other letters ("cafu00e9"): its own document does
                // not give it the memo; this one, in UTF-8, does.
                QCOMPARE(mine.take(QStringLiteral("tx.memo.text")),
                         QString::fromUtf8("\"caf\xc3\xa9 \xe6\x97\xa5\xe6\x9c\xac\""));
                theirs.remove(QStringLiteral("tx.memo.text"));
            }
            QStringList keys = mine.keys() + theirs.keys();
            keys.removeDuplicates();
            for (const QString& key : keys) {
                QVERIFY2(mine.value(key, QStringLiteral("<none>")) == theirs.value(key, QStringLiteral("<none>")),
                         qPrintable(QLatin1String(c.name) + QStringLiteral(": ") + key + QStringLiteral(": ")
                                    + mine.value(key, QStringLiteral("<none>")) + QStringLiteral(" / ")
                                    + theirs.value(key, QStringLiteral("<none>"))));
            }
            QScopedPointer<Transaction> back(Txrep::fromTxrep(ours, Network::testnetNetwork()));
            QVERIFY2(back->toEnvelopeXdrBase64() == envelope, c.name);
        }
    }

    /**
     * A muxed (M...) transaction source was refused: it was read with
     * KeyPair::fromAccountId, which takes G... only, although this emitter
     * wrote it. Every MuxedAccount field reads an M... address now, and one
     * is written with py-stellar-sdk's comment naming its account and id. A
     * comment of that form naming another account or id, or on a G...
     * address, is refused, as is an M... address where the XDR holds an
     * AccountID.
     */
    void testMuxedAccountsAsPyWritesThem()
    {
        const QString doc = pyDocument("muxed");
        QScopedPointer<Transaction> tx(Txrep::fromTxrep(doc, Network::testnetNetwork()));
        QCOMPARE(tx->getSourceAccount(),
                 QStringLiteral("MCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYABAAAAAAAAAAFPM6"));
        auto* pay = dynamic_cast<PaymentOperation*>(tx->getOperations().at(0));
        QVERIFY(pay != nullptr);
        QCOMPARE(pay->getDestination(),
                 QStringLiteral("MCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZIAAAAAAAAAAAAHZ7W"));
        QCOMPARE(tx->getOperations().at(1)->getSourceAccount(),
                 QStringLiteral("MDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435D7777777777777RCC"));
        QCOMPARE(tx->toEnvelopeXdrBase64(), pyEnvelope("muxed"));
        QVERIFY(Txrep::toTxrep(tx.data()).contains(QStringLiteral(
            "tx.sourceAccount: MCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYABAAAAAAAAAAFPM6 "
            "(accountID: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR, "
            "accountMuxedID: 9007199254740993)\n")));

        // Without the comment the address says the same, and reads the same.
        QString bare = doc;
        bare.replace(QStringLiteral(" (accountID: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR, "
                                    "accountMuxedID: 9007199254740993)"), QString());
        QVERIFY(bare != doc);
        QScopedPointer<Transaction> fromBare(Txrep::fromTxrep(bare, Network::testnetNetwork()));
        QCOMPARE(fromBare->hashHex(), tx->hashHex());

        QString otherId = doc;
        otherId.replace(QStringLiteral("accountMuxedID: 9007199254740993)"),
                        QStringLiteral("accountMuxedID: 9007199254740994)"));
        QVERIFY(otherId != doc);
        QVERIFY(txrepRefused(otherId));
        QString otherAccount = doc;
        otherAccount.replace(
            QStringLiteral("(accountID: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U, accountMuxedID: 1)"),
            QStringLiteral("(accountID: GDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435DIAG, accountMuxedID: 1)"));
        QVERIFY(otherAccount != doc);
        QVERIFY(txrepRefused(otherAccount));
        QString onAG = doc;
        onAG.replace(QStringLiteral("paymentOp.destination: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"),
                     QStringLiteral("paymentOp.destination: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U "
                                    "(accountID: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U, "
                                    "accountMuxedID: 1)\n"));
        QVERIFY(onAG != doc);
        QVERIFY(txrepRefused(onAG));
        QString broken = doc;
        broken.replace(QStringLiteral("tx.sourceAccount: MCFIRY65"), QStringLiteral("tx.sourceAccount: MCFIRY66"));
        QVERIFY(broken != doc);
        QVERIFY(txrepRefused(broken));

        // A new account, a claimant, a sponsored account: AccountIDs, never muxed.
        const QString operations = pyDocument("operations");
        for (const QString& line : {
                 QStringLiteral("createAccountOp.destination: "),
                 QStringLiteral("claimants[0].v0.destination: "),
                 QStringLiteral("beginSponsoringFutureReservesOp.sponsoredID: ")}) {
            QString muxed = operations;
            const int at = muxed.indexOf(line) + line.size();
            QVERIFY(at > line.size());
            muxed.replace(at, 56, QStringLiteral("MCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZIAAAAAAAAAAAAHZ7W"));
            QVERIFY2(txrepRefused(muxed), qPrintable(line));
        }
    }

    /**
     * The signer of a REVOKE_SPONSORSHIP was written as one
     * `signer.signerKey: G...` line, which py-stellar-sdk does not read (it
     * wants `.type` and a field per type). It is written in py's form now,
     * for every key type, and py's form is read. The single line is still
     * read, as this SDK wrote it before: a strkey states its own key type and
     * carries a checksum, so it cannot be taken for another key. But not
     * beside py's lines, and never under a type it is not.
     */
    void testRevokeSignerSponsorshipAsPyWritesIt()
    {
        const QString account = QStringLiteral("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO");
        const QString g = QStringLiteral("GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7");
        const QString path = QStringLiteral("tx.operations[0].body.revokeSponsorshipOp.signer.signerKey");
        const QList<QPair<SignerKey, QString>> keys = {
            {SignerKey::ed25519PublicKey(g), QStringLiteral("SIGNER_KEY_TYPE_ED25519:ed25519")},
            {SignerKey::preAuthTx(QByteArray(32, '\x01')), QStringLiteral("SIGNER_KEY_TYPE_PRE_AUTH_TX:preAuthTx")},
            {SignerKey::sha256Hash(QByteArray(32, '\x02')), QStringLiteral("SIGNER_KEY_TYPE_HASH_X:hashX")},
            {SignerKey::ed25519SignedPayload(g, QByteArray("hello")),
             QStringLiteral("SIGNER_KEY_TYPE_ED25519_SIGNED_PAYLOAD:ed25519SignedPayload")},
        };
        for (const auto& entry : keys) {
            const SignerKey& key = entry.first;
            const QString type = entry.second.section(QLatin1Char(':'), 0, 0);
            const QString field = entry.second.section(QLatin1Char(':'), 1, 1);
            QScopedPointer<Transaction> tx(transactionOf(
                RevokeSignerSponsorshipOperation::create(account, key.toXdr())));
            const QString doc = Txrep::toTxrep(tx.data());
            QVERIFY2(doc.contains(path + QStringLiteral(".type: ") + type + QLatin1Char('\n')
                                  + path + QLatin1Char('.') + field + QStringLiteral(": ") + key.toString()
                                  + QLatin1Char('\n')), qPrintable(type));
            QScopedPointer<Transaction> back(Txrep::fromTxrep(doc, Network::testnetNetwork()));
            QCOMPARE(back->hashHex(), tx->hashHex());

            const QString flatLine = path + QStringLiteral(": ") + key.toString() + QLatin1Char('\n');
            const QString flat = withoutLineStarting(withoutLineStarting(doc, path + QStringLiteral(".type:")),
                                                     path + QLatin1Char('.') + field + QLatin1Char(':'))
                                 + flatLine;
            QScopedPointer<Transaction> fromFlat(Txrep::fromTxrep(flat, Network::testnetNetwork()));
            QCOMPARE(fromFlat->hashHex(), tx->hashHex());
            QVERIFY(txrepRefused(doc + flatLine));                  // both forms
        }
        // A key under a type it is not, or a field the type does not name.
        QScopedPointer<Transaction> tx(transactionOf(
            RevokeSignerSponsorshipOperation::create(account, SignerKey::ed25519PublicKey(g).toXdr())));
        const QString doc = Txrep::toTxrep(tx.data());
        QString wrongType = doc;
        wrongType.replace(QStringLiteral(".signerKey.type: SIGNER_KEY_TYPE_ED25519\n"),
                          QStringLiteral(".signerKey.type: SIGNER_KEY_TYPE_PRE_AUTH_TX\n"));
        QVERIFY(wrongType != doc);
        QVERIFY(txrepRefused(wrongType));
        wrongType.replace(QStringLiteral(".signerKey.ed25519: "), QStringLiteral(".signerKey.preAuthTx: "));
        QVERIFY(txrepRefused(wrongType));
    }

    /**
     * A revoked trustline of pool shares has no asset: toTxrep threw "asset
     * cannot be null" for it. It is written as SEP-11 writes that
     * TrustLineAsset, the pool id in lowercase hex and ":lp" (py-stellar-sdk
     * has no form for it: it refuses to write one, and refuses this one), and
     * read back as the same pool.
     */
    void testRevokePoolShareTrustlineSponsorship()
    {
        const QString hexId = QStringLiteral("2c3dc9d8d5f72c61f4468a88a65722b3edf113edbcae1f6f45010b42f52d1f11");
        const QByteArray poolId = QByteArray::fromHex(hexId.toLatin1());
        QScopedPointer<Transaction> tx(transactionOf(RevokeTrustlineSponsorshipOperation::create(
            QStringLiteral("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"), poolId)));
        const QString doc = Txrep::toTxrep(tx.data());
        const QString line = QStringLiteral("revokeSponsorshipOp.ledgerKey.trustLine.asset: ");
        QVERIFY(doc.contains(line + hexId + QStringLiteral(":lp\n")));
        QScopedPointer<Transaction> back(Txrep::fromTxrep(doc, Network::testnetNetwork()));
        QCOMPARE(back->hashHex(), tx->hashHex());
        auto* revoke = dynamic_cast<RevokeTrustlineSponsorshipOperation*>(back->getOperations().at(0));
        QVERIFY(revoke != nullptr);
        QVERIFY(revoke->isLiquidityPoolShare());
        QCOMPARE(revoke->getLiquidityPoolID(), poolId);
        QVERIFY(revoke->getAsset() == nullptr);

        // Not 32 bytes of hex, or not ":lp": no pool, and no asset either.
        for (const QString& bad : QStringList{hexId.left(62) + QStringLiteral(":lp"),
                                              QString(64, QLatin1Char('z')) + QStringLiteral(":lp"),
                                              QStringLiteral("USD:lp"), hexId + QStringLiteral(":LP"), hexId}) {
            QString edited = doc;
            edited.replace(line + hexId + QStringLiteral(":lp"), line + bad);
            QVERIFY(edited != doc);
            QVERIFY2(txrepRefused(edited), qPrintable(bad));
        }
    }

    /**
     * A ChangeTrust for pool shares threw "liquidity-pool ChangeTrust not yet
     * supported", and py-stellar-sdk's lines for one were refused. Both ways
     * now, in py's form. A fee other than 30, or the two assets out of their
     * order, names no pool: refused.
     */
    void testLiquidityPoolShareChangeTrust()
    {
        const QString issuer = QStringLiteral("GDFJHLAXAUMHA4OWPOB4P7YO72AQR2HMIUYFOXLXE2DZGM633K7HZDQP");
        const QString usd = QStringLiteral("USD:") + issuer;
        QScopedPointer<ChangeTrustAsset> pool(new LiquidityPoolShareChangeTrustAsset(
            new AssetTypeNative(), Asset::create(QStringLiteral("USD"), issuer), 30));
        QScopedPointer<Transaction> tx(transactionOf(ChangeTrustOperation::create(pool.data(), QStringLiteral("50"))));
        const QString doc = Txrep::toTxrep(tx.data());
        const QString p = QStringLiteral("tx.operations[0].body.changeTrustOp.");
        QVERIFY(doc.contains(p + QStringLiteral("line.type: ASSET_TYPE_POOL_SHARE\n")
                             + p + QStringLiteral("line.liquidityPool.constantProduct.assetA: native\n")
                             + p + QStringLiteral("line.liquidityPool.constantProduct.assetB: ") + usd + QLatin1Char('\n')
                             + p + QStringLiteral("line.liquidityPool.constantProduct.fee: 30\n")
                             + p + QStringLiteral("limit: 500000000 (50)\n")));
        QScopedPointer<Transaction> back(Txrep::fromTxrep(doc, Network::testnetNetwork()));
        QCOMPARE(back->hashHex(), tx->hashHex());
        auto* changeTrust = dynamic_cast<ChangeTrustOperation*>(back->getOperations().at(0));
        QVERIFY(changeTrust != nullptr);
        QVERIFY(changeTrust->getAsset() == nullptr);
        QVERIFY(changeTrust->getLine()->equals(pool.data()));

        QString fee = doc;
        fee.replace(QStringLiteral("constantProduct.fee: 30"), QStringLiteral("constantProduct.fee: 31"));
        QVERIFY(fee != doc);
        QVERIFY(txrepRefused(fee));
        QString swapped = doc;
        swapped.replace(QStringLiteral("assetA: native"), QStringLiteral("assetA: ") + usd);
        swapped.replace(QStringLiteral("assetB: ") + usd, QStringLiteral("assetB: native"));
        QVERIFY(swapped != doc);
        QVERIFY(txrepRefused(swapped));
        QVERIFY(txrepRefused(doc + p + QStringLiteral("line: native\n")));    // the other arm's line
        QVERIFY(txrepRefused(withoutLineStarting(doc, p + QStringLiteral("line.liquidityPool.constantProduct.fee:"))));
    }

    /**
     * py-stellar-sdk writes a revoke-data name bare and takes quotes around
     * it as part of the name: the quoted name this SDK wrote was, to py,
     * another data entry. It is written bare, and reads back here and in py.
     * A name no bare form says (a space, a leading quote, a character that
     * must be escaped) was written, and read, quoted, which py signed as
     * the name with its quotes: it has no form both read alike, and is
     * refused.
     */
    void testRevokeDataNameAsPyReadsIt()
    {
        const QString key = QStringLiteral("tx.operations[0].body.revokeSponsorshipOp.ledgerKey.data.dataName");
        const QStringList names = {QStringLiteral("config.memo"), QString::fromUtf8("caf\xc3\xa9"),
                                   QStringLiteral("k(1)")};
        for (const QString& name : names) {
            QScopedPointer<Transaction> tx(transactionOf(RevokeDataSponsorshipOperation::create(
                QStringLiteral("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"), name)));
            const QString doc = Txrep::toTxrep(tx.data());
            QVERIFY2(doc.contains(key + QStringLiteral(": ") + name + QLatin1Char('\n')), qPrintable(name));
            QCOMPARE(pyReads(doc).value(key), name);                    // py reads the name itself
            QScopedPointer<Transaction> back(Txrep::fromTxrep(doc, Network::testnetNetwork()));
            QCOMPARE(back->hashHex(), tx->hashHex());
        }
        for (const QString& name : {QStringLiteral("two words"), QStringLiteral("\"quoted"),
                                    QStringLiteral("tab\there")}) {
            QScopedPointer<Transaction> tx(transactionOf(RevokeDataSponsorshipOperation::create(
                QStringLiteral("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"), name)));
            QVERIFY_EXCEPTION_THROWN(Txrep::toTxrep(tx.data()), std::runtime_error);
        }
    }

    /** INFLATION (it has no fields) was refused both ways; py-stellar-sdk writes it. */
    void testInflationRoundTrip()
    {
        QScopedPointer<Transaction> tx(transactionOf(InflationOperation::create()));
        const QString doc = Txrep::toTxrep(tx.data());
        QVERIFY(doc.contains(QStringLiteral("tx.operations[0].body.type: INFLATION\n")));
        QScopedPointer<Transaction> back(Txrep::fromTxrep(doc, Network::testnetNetwork()));
        QCOMPARE(back->hashHex(), tx->hashHex());
        QVERIFY(dynamic_cast<InflationOperation*>(back->getOperations().at(0)) != nullptr);
    }

    /**
     * What this SDK does not hold as TxRep is refused, never read as
     * something else: the Soroban operations, which no SDK writes in TxRep,
     * and, from fromTxrep, which returns a Transaction, a fee bump (its inner
     * transaction is not the fee bump; fromTxrepEnvelope reads it). The V0
     * document, refused too until 1.0.4, is the V0 transaction it is.
     */
    void testWhatTxrepDoesNotHoldIsRefused()
    {
        QVERIFY(txrepRefused(QString::fromLatin1(pyFeeBumpDocument())));
        QScopedPointer<AbstractTransaction> feeBump(Txrep::fromTxrepEnvelope(
            QString::fromLatin1(pyFeeBumpDocument()), Network::testnetNetwork()));
        QVERIFY(dynamic_cast<FeeBumpTransaction*>(feeBump.data()) != nullptr);
        for (const char* type : {"INVOKE_HOST_FUNCTION", "EXTEND_FOOTPRINT_TTL", "RESTORE_FOOTPRINT"}) {
            QString doc = withoutLineStarting(paymentDocument(QStringLiteral("tx.cond.type: PRECOND_NONE\n")),
                                              QStringLiteral("tx.operations[0].body.paymentOp."));
            doc.replace(QStringLiteral("body.type: PAYMENT"), QStringLiteral("body.type: ") + QLatin1String(type));
            QVERIFY2(txrepRefused(doc), type);
        }
        QScopedPointer<Transaction> soroban(transactionOf(RestoreFootprintOperation::create()));
        QVERIFY_EXCEPTION_THROWN(Txrep::toTxrep(soroban.data()), std::runtime_error);
    }

    /**
     * A refused document frees everything read for it. SET_OPTIONS held its
     * operation in a bare pointer, so a weight above 255 (which the setter
     * refuses) leaked it; so did the assets, keys and predicates read in the
     * other branches before a later field was refused, and every operation
     * whose source-account line was then refused. Counted on the MSVC debug
     * heap: each document is refused once to warm up, then 20 times more,
     * and the heap must hold no more blocks than before.
     */
    void testARefusedDocumentLeaksNothing()
    {
#if defined(_MSC_VER) && defined(_DEBUG)
        const QString operations = pyDocument("operations");
        const QList<QPair<QString, QString>> edits = {
            {QStringLiteral("setOptionsOp.masterWeight: 255"), QStringLiteral("setOptionsOp.masterWeight: 256")},
            {QStringLiteral("setOptionsOp.inflationDest: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U"),
             QStringLiteral("setOptionsOp.inflationDest: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55V")},
            {QStringLiteral("tx.operations[2].body.paymentOp.amount: 1 (0.0000001)"),
             QStringLiteral("tx.operations[2].body.paymentOp.amount: 1x")},
            {QStringLiteral("manageSellOfferOp.price.n: 7"), QStringLiteral("manageSellOfferOp.price.n: 0")},
            {QStringLiteral("[5].body.pathPaymentStrictReceiveOp.destination: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U"),
             QStringLiteral("[5].body.pathPaymentStrictReceiveOp.destination: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55V")},
            {QStringLiteral("andPredicates[1].type: CLAIM_PREDICATE_BEFORE_RELATIVE_TIME"),
             QStringLiteral("andPredicates[1].type: CLAIM_PREDICATE_SOMETIME")},
            {QStringLiteral("createClaimableBalanceOp.amount: 10000000 (1)"),
             QStringLiteral("createClaimableBalanceOp.amount: 1x")},
            {QStringLiteral("tx.operations[2].sourceAccount._present: true"),
             QStringLiteral("tx.operations[2].sourceAccount._present: yes")},
            {QStringLiteral("allowTrustOp.asset: USD"), QStringLiteral("allowTrustOp.asset: U$D")},
            {QStringLiteral("constantProduct.fee: 30"), QStringLiteral("constantProduct.fee: 31")},
            {QStringLiteral("[13].body.changeTrustOp.limit: 10000000000 (1000)"),
             QStringLiteral("[13].body.changeTrustOp.limit: x")},
            {QStringLiteral("setTrustLineFlagsOp.clearFlags: 1"), QStringLiteral("setTrustLineFlagsOp.clearFlags: -1")},
            {QStringLiteral("clawbackOp.amount: 20000000 (2)"), QStringLiteral("clawbackOp.amount: 2x")},
        };
        QStringList refused;
        for (const auto& edit : edits) {
            QString doc = operations;
            doc.replace(edit.first, edit.second);
            QVERIFY2(doc != operations, qPrintable(edit.first));
            QVERIFY2(txrepRefused(doc), qPrintable(edit.second));
            refused.append(doc);
        }
        _CrtMemState before, after, difference;
        _CrtMemCheckpoint(&before);
        for (int round = 0; round < 20; ++round) {
            for (const QString& doc : refused)
                txrepRefused(doc);
        }
        _CrtMemCheckpoint(&after);
        _CrtMemDifference(&difference, &before, &after);
        // A net count, in a size_t: "below zero" when something older was freed.
        const qint64 left = static_cast<qint64>(difference.lCounts[_NORMAL_BLOCK]);
        QVERIFY2(left < 20, qPrintable(QStringLiteral("%1 blocks left on the heap").arg(left)));
#else
        QSKIP("counts blocks on the MSVC debug heap");
#endif
    }

    /**
     * The document without the glosses the emitter writes after amounts and
     * time bounds (py's "105000000 (10.5)"), for a test that writes its own
     * comment there.
     */
    static QString withoutGlosses(const QString& doc)
    {
        static const QRegularExpression gloss(QStringLiteral(
            "^([^:\\n]*\\.(?:amount|startingBalance|limit|buyAmount|sendMax|destAmount|sendAmount|destMin|"
            "maxAmountA|maxAmountB|minAmountA|minAmountB|minTime|maxTime): -?\\d+) \\(.*\\)$"),
            QRegularExpression::MultilineOption);
        QString out = doc;
        out.replace(gloss, QStringLiteral("\\1"));
        return out;
    }

    /** A comment that states a value must state the one that is signed: a
     *  reviewer reads "105000000 (10.5)" and trusts the 10.5, while the
     *  number was signed whatever the comment said (a comment was taken off
     *  and ignored). Where py-stellar-sdk states a value in a comment, the
     *  comment must be that value, as a number or a date that can be checked,
     *  and nothing else: words state values too ("(ninety-nine)"), a unit can
     *  name another asset, and a check on a leading number alone read past
     *  "(XLM 99.5)". On an account, only py's comment after an M... address. */
    void testACommentThatStatesAValueMustMatchIt()
    {
        const QString id = QStringLiteral("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account account(KeyPair::fromAccountId(id), 41);
        QScopedPointer<Transaction> tx(TransactionBuilder(&account, Network::testnetNetwork())
            .addOperation(new PaymentOperation(id, new AssetTypeNative(), QStringLiteral("10.5")))
            .addTimeBounds(new TimeBounds(1700000000, 1800000000))
            .setBaseFee(100)
            .build());
        const QString doc = withoutGlosses(Txrep::toTxrep(tx.data()));
        const QString amount = QStringLiteral("tx.operations[0].body.paymentOp.amount: 105000000\n");
        const QString minTime = QStringLiteral("tx.cond.timeBounds.minTime: 1700000000\n");
        const QString fee = QStringLiteral("tx.fee: 100\n");
        QVERIFY(doc.contains(amount));
        QVERIFY(doc.contains(minTime));
        QVERIFY(doc.contains(fee));
        const auto reads = [&](const QString& line, const QString& comment) {
            QString changed = doc;
            changed.replace(line, line.left(line.size() - 1) + QLatin1Char(' ') + comment + QLatin1Char('\n'));
            try {
                QScopedPointer<Transaction> t(Txrep::fromTxrep(changed, Network::testnetNetwork()));
                return t->hash() == tx->hash();
            } catch (const std::exception&) {
                return false;
            }
        };
        // What py-stellar-sdk writes, and the same values in other spellings, read.
        QVERIFY(reads(amount, QStringLiteral("(10.5)")));
        QVERIFY(reads(amount, QStringLiteral("(10.5000000)")));
        QVERIFY(reads(amount, QStringLiteral("(1.05e1)")));
        QVERIFY(reads(amount, QStringLiteral("(1.05E+1)")));
        QVERIFY(reads(minTime, QStringLiteral("(2023-11-14 22:13:20.000000+00:00 (UTC))")));
        QVERIFY(reads(minTime, QStringLiteral("(2023-11-14T22:13:20Z)")));
        QVERIFY(reads(minTime, QStringLiteral("(2023-11-14T23:13:20+01:00)")));
        QVERIFY(reads(fee, QStringLiteral("(0.00001)")));
        // Stroops, only with the exponent 7 that marks them (SEP-11's
        // "(40.0004e7)", stc's "(10.5e7)"). A plain number that is the amount
        // only in stroops was read as stroops, and a py-trained reader takes
        // "(105000000)" for 105000000 units: 10^7 times the amount signed.
        // These two expectations were `reads` until 1.0.4.
        QVERIFY(!reads(amount, QStringLiteral("(105000000)")));
        QVERIFY(!reads(fee, QStringLiteral("(100)")));
        QVERIFY(reads(amount, QStringLiteral("(10.5e7)")));
        QVERIFY(reads(amount, QStringLiteral("(10.5E+7)")));
        QVERIFY(reads(fee, QStringLiteral("(0.00001e7)")));
        QVERIFY(!reads(amount, QStringLiteral("(1.05e8)")));        // the stroops, without the mark
        // A comment that says another value is refused.
        QVERIFY(!reads(amount, QStringLiteral("(99.5)")));
        QVERIFY(!reads(amount, QStringLiteral("(1.05)")));
        QVERIFY(!reads(minTime, QStringLiteral("(2024-11-14 22:13:20.000000+00:00 (UTC))")));
        QVERIFY(!reads(minTime, QStringLiteral("(2023-11-14 22:13:20.500000+00:00 (UTC))")));
        QVERIFY(!reads(fee, QStringLiteral("(0.0001)")));
        // So is one no check can compare with the value.
        QVERIFY(!reads(amount, QStringLiteral("(XLM 99.5)")));
        QVERIFY(!reads(amount, QStringLiteral("(~99.5)")));
        QVERIFY(!reads(amount, QStringLiteral("(99,5)")));
        QVERIFY(!reads(amount, QStringLiteral("(10.5) 99.5")));
        QVERIFY(!reads(amount, QStringLiteral("(1.05e1 USD)")));
        QVERIFY(!reads(amount, QStringLiteral("(ninety-nine)")));
        QVERIFY(!reads(amount, QString::fromUtf8("(\xd9\xa9\xd9\xa9)")));        // Arabic-Indic 99
        QVERIFY(!reads(minTime, QStringLiteral("(2023-11-14 22:13:20)")));       // no zone
        QVERIFY(!reads(minTime, QStringLiteral("(2023-11-14T22:13:20+01:00 (UTC))")));
        QVERIFY(!reads(minTime, QStringLiteral("(Tue Nov 14 22:13:20 PST 2023)")));
        QVERIFY(!reads(minTime, QStringLiteral("(in 2025)")));
        QVERIFY(!reads(fee, QStringLiteral("(1 op)")));

        // A G... account takes no comment: none says who it is that can be checked.
        const QString destination = QStringLiteral("tx.operations[0].body.paymentOp.destination: ") + id + QLatin1Char('\n');
        QVERIFY(doc.contains(destination));
        QVERIFY(!reads(destination, QStringLiteral("(GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO)")));
        QVERIFY(!reads(destination, QStringLiteral("(the exchange)")));
        QVERIFY(!reads(destination, QStringLiteral("(accountMuxedID: 7)")));
    }

    /** py-stellar-sdk writes a revoke-data name bare and reads quotes as part
     *  of it, so its document for the name "x" (quotes included) was read
     *  here as x: the revoke of another entry. And the quoted document this
     *  SDK wrote for a name with a space was, to py, that name with quotes,
     *  and a bare `a b` was a b here and a to py. No document says such a
     *  name to both alike, so none is written, and none is read. */
    void testAQuotedDataNamePyWouldWriteBareIsRefused()
    {
        const QString id = QStringLiteral("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        const auto transaction = [&](const QString& name) {
            Account account(KeyPair::fromAccountId(id), 41);
            return TransactionBuilder(&account, Network::testnetNetwork())
                .addOperation(new RevokeDataSponsorshipOperation(id, name))
                .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build();
        };
        QScopedPointer<Transaction> x(transaction(QStringLiteral("x")));
        const QString bare = Txrep::toTxrep(x.data());
        QVERIFY(bare.contains(QStringLiteral("dataName: x\n")));
        QScopedPointer<Transaction> back(Txrep::fromTxrep(bare, Network::testnetNetwork()));
        QCOMPARE(back->hashHex(), x->hashHex());
        for (const QString& value : {QStringLiteral("\"x\""), QStringLiteral("\"k (1)\""),
                                     QStringLiteral("a b"), QStringLiteral("x (a comment)")}) {
            QString edited = bare;
            edited.replace(QStringLiteral("dataName: x\n"), QStringLiteral("dataName: ") + value + QLatin1Char('\n'));
            QVERIFY2(txrepRefused(edited), qPrintable(value));
        }
        QScopedPointer<Transaction> spaced(transaction(QStringLiteral("k (1)")));
        QVERIFY_EXCEPTION_THROWN(Txrep::toTxrep(spaced.data()), std::runtime_error);
    }

    /**
     * Fee-bump and V0 documents, as py-stellar-sdk 14.1.0 writes them
     * (pyEnvelopeCorpus() below), were refused. Read with fromTxrepEnvelope,
     * each is now the envelope py built, byte for byte, both signature lists
     * included. Written here from that envelope, the document gives py what
     * py's own gives it (so py builds the same envelope from it), and reads
     * back as the envelope. fromTxrep, which returns a Transaction, reads a
     * V0 document as the V0 transaction, to be signed, and still refuses a
     * fee bump: its inner transaction is not the fee bump.
     */
    void testPyFeeBumpAndV0CorpusBothWays()
    {
        QVERIFY(pyEnvelopeCorpus().size() >= 9);
        for (const PyCorpusCase& c : pyEnvelopeCorpus()) {
            const QString document = QString::fromLatin1(c.txrep);
            const QString envelope = QString::fromLatin1(c.envelope);
            QScopedPointer<AbstractTransaction> read(Txrep::fromTxrepEnvelope(document, Network::testnetNetwork()));
            QVERIFY2(read->toEnvelopeXdrBase64() == envelope, c.name);
            QVERIFY2(read->hashHex() == QLatin1String(c.hash), c.name);

            QScopedPointer<AbstractTransaction> decoded(
                AbstractTransaction::fromEnvelopeXdr(envelope, Network::testnetNetwork()));
            const QString ours = Txrep::toTxrep(decoded.data());
            const QMap<QString, QString> mine = pyReads(ours);
            const QMap<QString, QString> theirs = pyReads(document);
            QStringList keys = mine.keys() + theirs.keys();
            keys.removeDuplicates();
            for (const QString& key : keys) {
                QVERIFY2(mine.value(key, QStringLiteral("<none>")) == theirs.value(key, QStringLiteral("<none>")),
                         qPrintable(QLatin1String(c.name) + QStringLiteral(": ") + key + QStringLiteral(": ")
                                    + mine.value(key, QStringLiteral("<none>")) + QStringLiteral(" / ")
                                    + theirs.value(key, QStringLiteral("<none>"))));
            }
            QScopedPointer<AbstractTransaction> back(Txrep::fromTxrepEnvelope(ours, Network::testnetNetwork()));
            QVERIFY2(back->toEnvelopeXdrBase64() == envelope, c.name);

            if (document.startsWith(QLatin1String("type: ENVELOPE_TYPE_TX_FEE_BUMP\n"))) {
                QVERIFY2(dynamic_cast<FeeBumpTransaction*>(read.data()) != nullptr, c.name);
                QVERIFY2(txrepRefused(document), c.name);
            } else {
                QScopedPointer<Transaction> plain(Txrep::fromTxrep(document, Network::testnetNetwork()));
                QVERIFY2(plain->hashHex() == QLatin1String(c.hash), c.name);
                QVERIFY2(plain->getSignatures().isEmpty(), c.name);
                QVERIFY2(plain->toEnvelopeXdr().type == stellar::EnvelopeType::ENVELOPE_TYPE_TX_V0, c.name);
            }
        }
    }

    /**
     * Every document of pyCorpus() and pyEnvelopeCorpus() is, line for line,
     * what this SDK writes for its envelope: the same keys in the same order,
     * the same values and the same comments (amounts in units, dates in UTC,
     * an M... address's account and id), not only what py reads from them.
     * The expected lines are py-stellar-sdk 14.1.0's to_txrep of each
     * envelope decoded from its XDR (TransactionEnvelope.from_xdr /
     * FeeBumpTransactionEnvelope.from_xdr, testnet): each corpus document,
     * but for one stroop, which py glosses "0.0000001" when it is given that
     * string and "1E-7" from the XDR, as written here. Three differences are
     * this SDK's, and are checked as such: the last line ends in a newline
     * (py joins its lines); a NOT predicate has SEP-11's
     * `notPredicate._present: true` line, which py neither writes nor reads;
     * and a memo text outside ASCII is written in UTF-8, where py writes
     * JSON escapes that it reads back as other letters. py's document, so
     * written, reads back here as the envelope.
     */
    void testOurDocumentIsPysLineForLine()
    {
        std::vector<PyCorpusCase> all = pyCorpus();
        all.insert(all.end(), pyEnvelopeCorpus().begin(), pyEnvelopeCorpus().end());
        QVERIFY(all.size() >= 32);
        const QString notType = QStringLiteral(".type: CLAIM_PREDICATE_NOT");
        for (const PyCorpusCase& c : all) {
            QString py = QString::fromLatin1(c.txrep);
            py.replace(QStringLiteral(" (0.0000001)"), QStringLiteral(" (1E-7)"));
            const QString envelope = QString::fromLatin1(c.envelope);
            QScopedPointer<AbstractTransaction> decoded(
                AbstractTransaction::fromEnvelopeXdr(envelope, Network::testnetNetwork()));
            const QString ours = Txrep::toTxrep(decoded.data());
            QVERIFY2(ours.endsWith(QLatin1Char('\n')) && !py.endsWith(QLatin1Char('\n')), c.name);

            QStringList mine = ours.left(ours.size() - 1).split(QLatin1Char('\n'));
            QStringList theirs = py.split(QLatin1Char('\n'));
            int markers = 0;
            for (int i = mine.size() - 1; i > 0; --i) {
                if (!mine.at(i).endsWith(QLatin1String(".notPredicate._present: true")))
                    continue;
                const QString& before = mine.at(i - 1);
                QVERIFY2(before.endsWith(notType), c.name);
                QCOMPARE(mine.at(i), before.left(before.size() - notType.size())
                                         + QStringLiteral(".notPredicate._present: true"));
                mine.removeAt(i);
                ++markers;
            }
            QVERIFY2(markers == theirs.filter(notType).size(), c.name);

            if (qstrcmp(c.name, "memo_text_unicode") == 0) {
                const int at = theirs.indexOf(QStringLiteral("tx.memo.text: \"caf\\u00e9 \\u65e5\\u672c\""));
                QVERIFY(at > 0);
                QCOMPARE(mine.value(at),
                         QString::fromUtf8("tx.memo.text: \"caf\xc3\xa9 \xe6\x97\xa5\xe6\x9c\xac\""));
                mine[at] = theirs.at(at);
            }

            for (int i = 0; i < qMax(mine.size(), theirs.size()); ++i) {
                QVERIFY2(mine.value(i) == theirs.value(i),
                         qPrintable(QLatin1String(c.name) + QStringLiteral(" line ") + QString::number(i + 1)
                                    + QStringLiteral(": ") + mine.value(i) + QStringLiteral(" / ")
                                    + theirs.value(i)));
            }

            QScopedPointer<AbstractTransaction> read(Txrep::fromTxrepEnvelope(py, Network::testnetNetwork()));
            QVERIFY2(read->toEnvelopeXdrBase64() == envelope, c.name);
            QVERIFY2(read->hashHex() == QLatin1String(c.hash), c.name);
        }
    }

    /**
     * The Soroban operations have no document to agree with: py-stellar-sdk
     * 14.1.0's to_txrep raises ValueError ("This operation has not been
     * implemented yet") for each, and js-stellar-sdk 17.1.0 has no TxRep.
     * These envelopes were built with py 14.1.0 (TransactionBuilder, testnet,
     * pyCorpus()'s source): an INVOKE_HOST_FUNCTION calling hello(world, 7)
     * with its Soroban data, the same without it, and an
     * EXTEND_FOOTPRINT_TTL. Each is decoded here (the hash is py's) and
     * refused by toTxrep, as py refuses it, instead of written as another
     * transaction.
     */
    void testPysSorobanEnvelopesHaveNoDocument()
    {
        const char* const cases[][2] = {
            { "AAAAAgAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXAAAE+wAAAEfcfsEywAAAAEAAAAAAAAAAAAAAABq3j0B"
              "AAAAAAAAAAEAAAAAAAAAGAAAAAAAAAABNj6qOGeEH7rQ9O2Ix3nk/mblaiRw3JjA7JwHPQXHsQMAAAAFaGVsbG8AAAAAAAAC"
              "AAAADwAAAAV3b3JsZAAAAAAAAAMAAAAHAAAAAAAAAAEAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAATiAAAAAA=",
              "86f9e3021a08d5978a21ad154fa895a2139d1f9c1a9f58a7e0793812d94332a6" },
            { "AAAAAgAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXAAAAGQAAAEfcfsEywAAAAEAAAAAAAAAAAAAAABq3j0B"
              "AAAAAAAAAAEAAAAAAAAAGAAAAAAAAAABNj6qOGeEH7rQ9O2Ix3nk/mblaiRw3JjA7JwHPQXHsQMAAAAFaGVsbG8AAAAAAAAC"
              "AAAADwAAAAV3b3JsZAAAAAAAAAMAAAAHAAAAAAAAAAAAAAAA",
              "d3de3443ef5eb198765deef3d99a5907baee35b619564e2fa33226152a71c093" },
            { "AAAAAgAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXAAAE+wAAAEfcfsEywAAAAEAAAAAAAAAAAAAAABq3j0B"
              "AAAAAAAAAAEAAAAAAAAAGQAAAAAAAAPoAAAAAQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABOIAAAAAA==",
              "43d35c2700662b12a2ca4eda67bf620ee4388a0445415adc7f072f536cf09159" },
        };
        for (const auto& c : cases) {
            QScopedPointer<AbstractTransaction> decoded(
                AbstractTransaction::fromEnvelopeXdr(QString::fromLatin1(c[0]), Network::testnetNetwork()));
            QCOMPARE(decoded->hashHex(), QString::fromLatin1(c[1]));
            QVERIFY_EXCEPTION_THROWN(Txrep::toTxrep(decoded.data()), std::runtime_error);
        }
    }

    /**
     * A fee bump the SDK builds is written as py-stellar-sdk writes one: the
     * fee source (an M... one with py's comment), the fee, the transaction it
     * wraps and both signature lists. toTxrep had no overload for one, and
     * a fee-bump document was refused. What would sign something else than
     * the document shows is refused: an inner type that is not
     * ENVELOPE_TYPE_TX, an ext that is not 0, a negative fee, a comment
     * naming another id, a signature past its count, fee-bump lines in a
     * plain document.
     */
    void testFeeBumpDocumentsAreWrittenAndRead()
    {
        QScopedPointer<KeyPair> source(KeyPair::fromSecretSeed(QByteArray(32, '\x01')));
        QScopedPointer<KeyPair> payer(KeyPair::fromSecretSeed(QByteArray(32, '\x02')));
        const QString payerMuxed = QStringLiteral("MCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZIAAAAAAAAAAAAHZ7W");
        Account account(source->getAccountId(), 41);
        Transaction* inner = TransactionBuilder(&account, Network::testnetNetwork())
            .addOperation(PaymentOperation::create(payer->getAccountId(), new AssetTypeNative(), QStringLiteral("10.5")))
            .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
            .setBaseFee(100)
            .build();
        inner->sign(source.data());
        FeeBumpTransactionBuilder builder(AccountConverter().enableMuxed(), inner);
        builder.setBaseFee(200).setFeeAccount(payerMuxed);
        QScopedPointer<FeeBumpTransaction> feeBump(builder.build());
        feeBump->sign(payer.data());

        const QString doc = Txrep::toTxrep(feeBump.data());
        QVERIFY(doc.startsWith(QStringLiteral(
            "type: ENVELOPE_TYPE_TX_FEE_BUMP\n"
            "feeBump.tx.feeSource: MCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZIAAAAAAAAAAAAHZ7W "
            "(accountID: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U, accountMuxedID: 1)\n"
            "feeBump.tx.fee: 400\n"
            "feeBump.tx.innerTx.type: ENVELOPE_TYPE_TX\n"
            "feeBump.tx.innerTx.tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n")));
        QVERIFY(doc.contains(QStringLiteral("feeBump.tx.innerTx.tx.ext.v: 0\n"
                                            "feeBump.tx.innerTx.signatures.len: 1\n"
                                            "feeBump.tx.innerTx.signatures[0].hint: b40f6f5c\n")));
        QVERIFY(doc.contains(QStringLiteral("feeBump.tx.ext.v: 0\n"
                                            "feeBump.signatures.len: 1\n"
                                            "feeBump.signatures[0].hint: 8fc9b394\n")));
        QCOMPARE(Txrep::toTxrep(static_cast<AbstractTransaction*>(feeBump.data())), doc);

        QScopedPointer<AbstractTransaction> read(Txrep::fromTxrepEnvelope(doc, Network::testnetNetwork()));
        auto* back = dynamic_cast<FeeBumpTransaction*>(read.data());
        QVERIFY(back != nullptr);
        QCOMPARE(back->toEnvelopeXdrBase64(), feeBump->toEnvelopeXdrBase64());
        QCOMPARE(back->hashHex(), feeBump->hashHex());
        QCOMPARE(back->getFeeAccount(), payerMuxed);
        QCOMPARE(back->getFee(), qint64(400));
        QCOMPARE(back->getInnerTransaction()->getSignatures().size(), 1);
        QVERIFY(txrepRefused(doc));                         // fromTxrep returns no fee bump

        const auto refused = [](const QString& document) {
            try {
                QScopedPointer<AbstractTransaction> t(Txrep::fromTxrepEnvelope(document, Network::testnetNetwork()));
                return false;
            } catch (const std::exception&) {
                return true;
            }
        };
        QVERIFY(!refused(doc));
        const QList<QPair<QString, QString>> edits = {
            {QStringLiteral("feeBump.tx.innerTx.type: ENVELOPE_TYPE_TX\n"),
             QStringLiteral("feeBump.tx.innerTx.type: ENVELOPE_TYPE_TX_V0\n")},
            {QStringLiteral("feeBump.tx.ext.v: 0\n"), QStringLiteral("feeBump.tx.ext.v: 1\n")},
            {QStringLiteral("feeBump.tx.fee: 400\n"), QStringLiteral("feeBump.tx.fee: -400\n")},
            {QStringLiteral("accountMuxedID: 1)"), QStringLiteral("accountMuxedID: 2)")},
            {QStringLiteral("feeBump.tx.fee: 400\n"), QStringLiteral("feeBump.tx.fee: 400 (0.0004)\n")},
            {QStringLiteral("feeBump.signatures.len: 1\n"), QStringLiteral("feeBump.signatures.len: 0\n")},
            {QStringLiteral("feeBump.tx.innerTx.type: ENVELOPE_TYPE_TX\n"), QString()},
        };
        for (const auto& edit : edits) {
            QString edited = doc;
            edited.replace(edit.first, edit.second);
            QVERIFY2(edited != doc, qPrintable(edit.first));
            QVERIFY2(refused(edited), qPrintable(edit.second));
        }
        QString plain = pyDocument("cond_none");
        QVERIFY(!refused(plain));
        QVERIFY(refused(plain + QStringLiteral("\nfeeBump.tx.fee: 400")));
    }

    /**
     * A transaction's document is its envelope. toTxrep wrote
     * `signatures.len: 0` for a signed transaction, and ENVELOPE_TYPE_TX for
     * a V0 one (whose envelope py would then rebuild as V1). Both are
     * written now; fromTxrepEnvelope carries the signatures, and fromTxrep
     * still hands the transaction back unsigned. A comment naming another
     * signer than a hint's was read past. A V0 document holds a G... source
     * and time bounds at most: an M... source or V2 preconditions, which a
     * V0 envelope has no room for, are refused.
     */
    void testSignaturesAndTheV0TypeAreWritten()
    {
        QScopedPointer<KeyPair> signer(KeyPair::fromSecretSeed(QByteArray(32, '\x01')));
        Account account(signer->getAccountId(), 41);
        QScopedPointer<Transaction> tx(TransactionBuilder(&account, Network::testnetNetwork())
            .addOperation(BumpSequenceOperation::create(qint64(7)))
            .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
            .setBaseFee(100)
            .build());
        tx->sign(signer.data());
        const QString doc = Txrep::toTxrep(tx.data());
        QVERIFY(doc.contains(QStringLiteral("tx.ext.v: 0\nsignatures.len: 1\nsignatures[0].hint: b40f6f5c\n"
                                            "signatures[0].signature: ")));
        QScopedPointer<AbstractTransaction> envelope(Txrep::fromTxrepEnvelope(doc, Network::testnetNetwork()));
        QCOMPARE(envelope->toEnvelopeXdrBase64(), tx->toEnvelopeXdrBase64());
        QScopedPointer<Transaction> unsigned_(Txrep::fromTxrep(doc, Network::testnetNetwork()));
        QVERIFY(unsigned_->getSignatures().isEmpty());
        QCOMPARE(unsigned_->hashHex(), tx->hashHex());
        // SEP-11's example names the signer after its hint: it must be one
        // the hint is of.
        QString named = doc;
        named.replace(QStringLiteral("signatures[0].hint: b40f6f5c\n"), QStringLiteral(
            "signatures[0].hint: b40f6f5c (GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR)\n"));
        QVERIFY(named != doc);
        QVERIFY(!txrepRefused(named));
        named.replace(QStringLiteral("(GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR)"),
                      QStringLiteral("(GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U)"));
        QVERIFY(txrepRefused(named));

        // A V0 envelope stays V0.
        QScopedPointer<AbstractTransaction> v0(AbstractTransaction::fromEnvelopeXdr(
            pyEnvelope("v0_time_signed"), Network::testnetNetwork()));
        auto* v0tx = dynamic_cast<Transaction*>(v0.data());
        QVERIFY(v0tx != nullptr);
        const QString v0doc = Txrep::toTxrep(v0tx);
        QVERIFY(v0doc.startsWith(QStringLiteral("type: ENVELOPE_TYPE_TX_V0\n")));
        QVERIFY(v0doc.contains(QStringLiteral("\nsignatures.len: 2\n")));
        QScopedPointer<Transaction> v0back(Txrep::fromTxrep(v0doc, Network::testnetNetwork()));
        QVERIFY(v0back->toEnvelopeXdr().type == stellar::EnvelopeType::ENVELOPE_TYPE_TX_V0);
        QCOMPARE(v0back->hashHex(), v0tx->hashHex());
        QScopedPointer<Transaction> pyV0(Txrep::fromTxrep(QString::fromLatin1(pyV0Document()), Network::testnetNetwork()));
        QVERIFY(pyV0->toEnvelopeXdr().type == stellar::EnvelopeType::ENVELOPE_TYPE_TX_V0);

        QString muxed = v0doc;
        muxed.replace(QStringLiteral("tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"),
                      QStringLiteral("tx.sourceAccount: MCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYABAAAAAAAAAAFPM6\n"));
        QVERIFY(muxed != v0doc);
        QVERIFY(txrepRefused(muxed));
        QString v2 = QString::fromLatin1(pyV0Document());
        v2.replace(QStringLiteral("tx.cond.type: PRECOND_NONE\n"), QStringLiteral(
            "tx.cond.type: PRECOND_V2\n"
            "tx.cond.timeBounds._present: false\n"
            "tx.cond.ledgerBounds._present: false\n"
            "tx.cond.minSeqNum._present: false\n"
            "tx.cond.minSeqAge: 7\n"
            "tx.cond.minSeqLedgerGap: 0\n"
            "tx.cond.extraSigners.len: 0\n"));
        QVERIFY(v2 != QString::fromLatin1(pyV0Document()));
        QVERIFY(txrepRefused(v2));
    }

    /**
     * The document is written from the envelope the transaction sends, not
     * from what its objects hold. Built with muxing off, a payment to an
     * M... address goes out to its G... account, and was written as the
     * M... address: read back, another transaction. Soroban data, which
     * TxRep has no form for, was dropped under `tx.ext.v: 0`. And a document
     * that does not read back as the envelope and the hash is not written.
     */
    void testTheDocumentIsTheEnvelopeSent()
    {
        const QString muxed = QStringLiteral("MCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZIAAAAAAAAAAAAHZ7W");
        Account account(QStringLiteral("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"), 41);
        QScopedPointer<Transaction> tx(
            TransactionBuilder(AccountConverter().disableMuxed(), &account, Network::testnetNetwork())
                .addOperation(PaymentOperation::create(muxed, new AssetTypeNative(), QStringLiteral("1")))
                .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build());
        const QString doc = Txrep::toTxrep(tx.data());
        QVERIFY(doc.contains(QStringLiteral(
            "paymentOp.destination: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n")));
        QScopedPointer<Transaction> back(Txrep::fromTxrep(doc, Network::testnetNetwork()));
        QCOMPARE(back->hashHex(), tx->hashHex());

        Account other(QStringLiteral("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"), 41);
        QScopedPointer<Transaction> soroban(TransactionBuilder(&other, Network::testnetNetwork())
            .addOperation(BumpSequenceOperation::create(qint64(7)))
            .setSorobanData(stellar::SorobanTransactionData())
            .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
            .setBaseFee(100)
            .build());
        QVERIFY_EXCEPTION_THROWN(Txrep::toTxrep(soroban.data()), std::runtime_error);

        // A V0 transaction given V2 preconditions hashes them, and its V0
        // envelope has no room for them: no document says both. It was
        // written as an ENVELOPE_TYPE_TX with them, which is neither.
        Account third(QStringLiteral("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"), 41);
        QScopedPointer<Transaction> v0(TransactionBuilder(&third, Network::testnetNetwork())
            .addOperation(BumpSequenceOperation::create(qint64(7)))
            .setMinSeqAge(7)
            .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
            .setBaseFee(100)
            .build());
        QVERIFY(v0->getPreconditions().hasV2());
        v0->setEnvelopeType(stellar::EnvelopeType::ENVELOPE_TYPE_TX_V0);
        QVERIFY_EXCEPTION_THROWN(Txrep::toTxrep(v0.data()), std::runtime_error);
    }

    // ─── 1.0.4: stc's documents, and a comment on every field ───────────

    /**
     * Documents written by stc (xdrpp/stc v0.2.0, the Go implementation
     * SEP-11 lists; stcCorpus() below), each read to the envelope stc signed
     * and to its hash. They were refused: stc writes every int64 with a
     * comment in stroops (`limit: 9223372036854775807
     * (922,337,203,685.4775807e7)` on a trustline at the default limit,
     * `offerID: 12345 (0.0012345e7)`, `absBefore: 1800000000 (180e7)`), a
     * time in Go's UnixDate (`(Tue Nov 14 22:13:20 UTC 2023)`, or with an
     * offset, `+04`), a signature hint with the key that made the signature
     * or its note that it could not tell, the native asset as "TestXLM" on
     * the test network, a ChangeTrust asset as the union's arm, a claimable
     * balance id as its union, V2 preconditions under `tx.cond.v2.`, a V0
     * transaction's `tx.timeBounds`, and an empty data value or signature as
     * "0 bytes". Written back here, each is the same envelope.
     */
    void testStcDocumentsReadAsStcWroteThem()
    {
        QVERIFY(stcCorpus().size() >= 20);
        for (const StcCorpusCase& c : stcCorpus()) {
            Network* network = c.publicNetwork ? Network::publicNetwork() : Network::testnetNetwork();
            const QString document = QString::fromLatin1(c.txrep);
            QScopedPointer<AbstractTransaction> read(Txrep::fromTxrepEnvelope(document, network));
            QVERIFY2(read->toEnvelopeXdrBase64() == QLatin1String(c.envelope), c.name);
            QVERIFY2(read->hashHex() == QLatin1String(c.hash), c.name);
            if (!document.startsWith(QLatin1String("type: ENVELOPE_TYPE_TX_FEE_BUMP\n"))) {
                QScopedPointer<Transaction> tx(Txrep::fromTxrep(document, network));
                QVERIFY2(tx->hashHex() == QLatin1String(c.hash), c.name);
            }
            QScopedPointer<AbstractTransaction> back(
                Txrep::fromTxrepEnvelope(Txrep::toTxrep(read.data()), network));
            QVERIFY2(back->toEnvelopeXdrBase64() == QLatin1String(c.envelope), c.name);
        }

        // One form or the other, and each exactly: a document with both
        // leaves one unread; "USD" under alphaNum12 is another asset than
        // USD, and a code with a zero inside it is none.
        const QString trustlines = stcDocument("trustlines");
        QVERIFY(!txrepRefused(trustlines));
        QVERIFY(!trustlines.endsWith(QLatin1Char('\n')));
        QVERIFY(txrepRefused(trustlines + QStringLiteral(
            "\ntx.operations[0].body.changeTrustOp.line: USD:GDFJHLAXAUMHA4OWPOB4P7YO72AQR2HMIUYFOXLXE2DZGM633K7HZDQP")));
        QString twelve = trustlines;
        twelve.replace(QStringLiteral("line.type: ASSET_TYPE_CREDIT_ALPHANUM4\n"
                                      "tx.operations[0].body.changeTrustOp.line.alphaNum4.assetCode: 55534400\n"
                                      "tx.operations[0].body.changeTrustOp.line.alphaNum4."),
                       QStringLiteral("line.type: ASSET_TYPE_CREDIT_ALPHANUM12\n"
                                      "tx.operations[0].body.changeTrustOp.line.alphaNum12.assetCode: 555344000000000000000000\n"
                                      "tx.operations[0].body.changeTrustOp.line.alphaNum12."));
        QVERIFY(twelve != trustlines);
        QVERIFY(txrepRefused(twelve));
        QString inside = trustlines;
        inside.replace(QStringLiteral("assetCode: 55534400\n"), QStringLiteral("assetCode: 55004400\n"));
        QVERIFY(inside != trustlines);
        QVERIFY(txrepRefused(inside));
        QString v2 = stcDocument("v2_cond");
        QVERIFY(!txrepRefused(v2));
        QVERIFY(txrepRefused(v2 + QStringLiteral("\ntx.cond.minSeqAge: 10")));

        // "0 bytes", in those words, is no bytes where the length is free; a
        // hint has four, and a bare "0" is no hex.
        const QString empty = stcDocument("manage_data_empty_value");
        QVERIFY(!txrepRefused(empty));
        const auto changed = [&](const QString& from, const QString& to) {
            QString edited = empty;
            edited.replace(from, to);
            return edited != empty && txrepRefused(edited);
        };
        QVERIFY(changed(QStringLiteral("dataValue: 0 bytes\n"), QStringLiteral("dataValue: 0\n")));
        QVERIFY(changed(QStringLiteral("dataValue: 0 bytes\n"), QStringLiteral("dataValue: 0 byte\n")));
        QVERIFY(changed(QStringLiteral("dataValue: 0 bytes\n"), QStringLiteral("dataValue: 0 bytes (0)\n")));
        QVERIFY(changed(QStringLiteral("hint: b40f6f5c (bad signature/unknown key/test is wrong network)"),
                        QStringLiteral("hint: 0 bytes")));
    }

    /**
     * What stc writes that no reader can check is refused. A time at the
     * local zone's abbreviation when it names several offsets: stc on a
     * machine in Kolkata writes IST, which is India's, Ireland's and
     * Israel's (CST is Chicago's, Shanghai's and Havana's); the same instant
     * with its offset, as stc writes it where the zone has no name, reads
     * (stcCorpus()), and so, since 1.0.4, does one at an abbreviation that
     * names one offset, as the CET stc writes in Madrid
     * (testADateAtAnAbbreviationThatNamesOneOffsetIsRead). The writer's own
     * labels from its stc.conf,
     * on an account (`(the exchange)`) and on a signer (`(G... signer 1)`,
     * `(G... signer for account G...)` with -l): whoever wrote the document
     * would tell the reviewer who is paid and who signed. The list of names
     * stc's editor writes after a type on a "?": it states every value the
     * field is not as much as the one it is, and any capitals would pass for
     * it (it was taken off and ignored, as every comment on a field
     * py-stellar-sdk does not annotate). A revoke-data name in quotes, which
     * py-stellar-sdk takes for part of the name. And "TestXLM" read for
     * another network than the test one, where it would show test coins
     * while lumens are spent. The strings are stc's own, from the runs of
     * stcCorpus().
     */
    void testWhatStcWritesThatCannotBeCheckedIsRefused()
    {
        const QString doc = stcDocument("payments_big");
        QVERIFY(!txrepRefused(doc));
        const QString signer = QStringLiteral("(GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR");
        // CET names one offset, +01:00, and reads there. This edit was in
        // the list below, refused, until 1.0.4.
        QString cet = doc;
        cet.replace(QStringLiteral("(Tue Nov 14 22:13:20 UTC 2023)"), QStringLiteral("(Tue Nov 14 23:13:20 CET 2023)"));
        cet.replace(QStringLiteral("(Fri Jan 15 08:00:00 UTC 2027)"), QStringLiteral("(Fri Jan 15 09:00:00 CET 2027)"));
        QVERIFY(cet != doc);
        QVERIFY(!txrepRefused(cet));
        const QList<QList<QPair<QString, QString>>> edits = {
            {{QStringLiteral("(Tue Nov 14 22:13:20 UTC 2023)"), QStringLiteral("(Wed Nov 15 03:43:20 IST 2023)")},
             {QStringLiteral("(Fri Jan 15 08:00:00 UTC 2027)"), QStringLiteral("(Fri Jan 15 13:30:00 IST 2027)")}},
            {{QStringLiteral("GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"),
              QStringLiteral("GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U (the exchange)\n")}},
            {{signer + QLatin1Char(')'), signer + QStringLiteral(" signer 1)")}},
            {{signer + QLatin1Char(')'),
              signer + QStringLiteral(" signer for account GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR)")}},
            // Go's UnixDate with the weekday of another day: two statements.
            {{QStringLiteral("(Tue Nov 14 22:13:20 UTC 2023)"), QStringLiteral("(Wed Nov 14 22:13:20 UTC 2023)")}},
            {{QStringLiteral("tx.memo.type: MEMO_NONE\n"),
              QStringLiteral("tx.memo.type: MEMO_NONE (MEMO_TEXT, MEMO_ID, MEMO_HASH, MEMO_RETURN, MEMO_NONE)\n")}},
            {{QStringLiteral("tx.operations[0].body.type: PAYMENT\n"),
              QStringLiteral("tx.operations[0].body.type: PAYMENT (CREATE_PASSIVE_SELL_OFFER, SET_OPTIONS, "
                             "ACCOUNT_MERGE, INFLATION, PATH_PAYMENT_STRICT_SEND, BEGIN_SPONSORING_FUTURE_RESERVES, "
                             "END_SPONSORING_FUTURE_RESERVES, REVOKE_SPONSORSHIP, PATH_PAYMENT_STRICT_RECEIVE, "
                             "CHANGE_TRUST, CREATE_CLAIMABLE_BALANCE, CLAWBACK, CLAWBACK_CLAIMABLE_BALANCE, "
                             "LIQUIDITY_POOL_WITHDRAW, MANAGE_SELL_OFFER, ALLOW_TRUST, MANAGE_DATA, BUMP_SEQUENCE, "
                             "MANAGE_BUY_OFFER, CLAIM_CLAIMABLE_BALANCE, SET_TRUST_LINE_FLAGS, LIQUIDITY_POOL_DEPOSIT, "
                             "CREATE_ACCOUNT, PAYMENT)\n")}},
        };
        for (const auto& edit : edits) {
            QString edited = doc;
            for (const auto& change : edit)
                edited.replace(change.first, change.second);
            QVERIFY2(edited != doc, qPrintable(edit.first().second));
            QVERIFY2(txrepRefused(edited), qPrintable(edit.first().second));
        }

        const QString revokeData = QStringLiteral(
            "type: ENVELOPE_TYPE_TX\n"
            "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
            "tx.fee: 100\n"
            "tx.seqNum: 1234567890123\n"
            "tx.cond.type: PRECOND_NONE\n"
            "tx.memo.type: MEMO_NONE\n"
            "tx.operations.len: 1\n"
            "tx.operations[0].sourceAccount._present: false\n"
            "tx.operations[0].body.type: REVOKE_SPONSORSHIP\n"
            "tx.operations[0].body.revokeSponsorshipOp.type: REVOKE_SPONSORSHIP_LEDGER_ENTRY\n"
            "tx.operations[0].body.revokeSponsorshipOp.ledgerKey.type: DATA\n"
            "tx.operations[0].body.revokeSponsorshipOp.ledgerKey.data.accountID: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
            "tx.operations[0].body.revokeSponsorshipOp.ledgerKey.data.dataName: \"k\"\n"
            "tx.ext.v: 0\n"
            "signatures.len: 1\n"
            "signatures[0].hint: b40f6f5c (GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR)\n"
            "signatures[0].signature: 7cc7eb16cc354bce9bc4b75382a6c064c345e4c866c6013412ce9a9b7ec9d929"
            "a66af4903a9bdd2a065332caa10debc290a4899e2fc2d744097faa9a65847705");
        QVERIFY(txrepRefused(revokeData));
        QString bareName = revokeData;
        bareName.replace(QStringLiteral("dataName: \"k\"\n"), QStringLiteral("dataName: k\n"));
        QVERIFY(bareName != revokeData);
        QVERIFY(!txrepRefused(bareName));                   // the same entry, the same signature

        // The native asset's name for the network read for, and only there.
        const auto readsFor = [](const QString& document, Network* network) {
            try {
                QScopedPointer<AbstractTransaction> t(Txrep::fromTxrepEnvelope(document, network));
                return true;
            } catch (const std::exception&) {
                return false;
            }
        };
        const QString onPublic = stcDocument("payments_big_public");
        QVERIFY(onPublic.contains(QStringLiteral("asset: XLM\n")));
        QVERIFY(readsFor(onPublic, Network::publicNetwork()));
        QString testCoins = onPublic;
        testCoins.replace(QStringLiteral("asset: XLM\n"), QStringLiteral("asset: TestXLM\n"));
        QVERIFY(!readsFor(testCoins, Network::publicNetwork()));
        QString standalone = doc;
        standalone.replace(QStringLiteral("asset: TestXLM\n"), QStringLiteral("asset: StandaloneXLM\n"));
        QVERIFY(standalone != doc);
        QVERIFY(!readsFor(standalone, Network::testnetNetwork()));
    }

    /**
     * The exponent 7 is SEP-11's and stc's mark of stroops, and only that:
     * "(1e7)" after 10^14 stroops is 10^7 units to a py-trained reader and
     * 1 to an stc-trained one, so it is read as stroops, and refused there.
     * Digits grouped by commas are read only in stc's form, with the
     * exponent: "(1,000)" is a thousand to one reader and one to another.
     */
    void testAnAmountInStroopsIsMarkedByTheExponentSeven()
    {
        QScopedPointer<Transaction> tx(transactionOf(
            PaymentOperation::create(QStringLiteral("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"),
                                     new AssetTypeNative(), QStringLiteral("10000000"))));
        const QString doc = withoutGlosses(Txrep::toTxrep(tx.data()));
        const QString line = QStringLiteral("paymentOp.amount: 100000000000000\n");
        QVERIFY(doc.contains(line));
        const auto reads = [&](const QString& comment) {
            QString changed = doc;
            changed.replace(line, line.left(line.size() - 1) + QLatin1Char(' ') + comment + QLatin1Char('\n'));
            return !txrepRefused(changed);
        };
        QVERIFY(reads(QStringLiteral("(10000000)")));                // units, as py writes it
        QVERIFY(reads(QStringLiteral("(10,000,000e7)")));            // stroops, as stc writes it
        QVERIFY(!reads(QStringLiteral("(1e7)")));
        QVERIFY(!reads(QStringLiteral("(1E+7)")));
        QVERIFY(!reads(QStringLiteral("(10,000,000)")));
        QVERIFY(!reads(QStringLiteral("(10,000,00e7)")));
        QVERIFY(!reads(QStringLiteral("(10.000.000e7)")));
    }

    /**
     * A comment on a field py-stellar-sdk never annotates was taken off and
     * ignored: `tx.seqNum: 77 (78)` was signed as 77, `tx.memo.type:
     * MEMO_TEXT (MEMO_ID)` as a text memo, a memo `"12345" (54321)` as
     * "12345". On every field a comment must now state that field's value in
     * a form that is checked, or the document is refused. An integer takes
     * the number itself (stc writes "(0.0012345e7)" after offer 12345). Words
     * are refused everywhere, as on an amount: they state values too, a label
     * names someone nothing in the document confirms, and a type's name, said
     * again or in stc's list of the names it could take, is words as well.
     */
    void testACommentOnAnyFieldMustStateItsValue()
    {
        const QString doc = withoutGlosses(documentWithMemoTimeAndSource());
        QScopedPointer<Transaction> tx(Txrep::fromTxrep(doc, Network::testnetNetwork()));
        const auto reads = [&](const QString& start, const QString& comment) {
            QStringList lines = doc.split(QLatin1Char('\n'));
            int found = 0;
            for (QString& line : lines) {
                if (line.startsWith(start)) {
                    line += QLatin1Char(' ') + comment;
                    ++found;
                }
            }
            if (found != 1)
                return false;
            try {
                QScopedPointer<Transaction> t(Txrep::fromTxrep(lines.join(QLatin1Char('\n')),
                                                                Network::testnetNetwork()));
                return t->hash() == tx->hash();
            } catch (const std::exception&) {
                return false;
            }
        };
        QVERIFY(doc.contains(QStringLiteral("tx.seqNum: 1\n")));
        QVERIFY(reads(QStringLiteral("tx.seqNum:"), QStringLiteral("(1)")));
        QVERIFY(reads(QStringLiteral("tx.seqNum:"), QStringLiteral("(0.0000001e7)")));
        QVERIFY(!reads(QStringLiteral("tx.seqNum:"), QStringLiteral("(2)")));
        QVERIFY(!reads(QStringLiteral("tx.seqNum:"), QStringLiteral("(one)")));
        QVERIFY(reads(QStringLiteral("tx.operations.len:"), QStringLiteral("(1)")));
        QVERIFY(!reads(QStringLiteral("tx.operations.len:"), QStringLiteral("(2)")));
        QVERIFY(reads(QStringLiteral("tx.ext.v:"), QStringLiteral("(0)")));
        QVERIFY(!reads(QStringLiteral("tx.ext.v:"), QStringLiteral("(1)")));
        QVERIFY(reads(QStringLiteral("tx.cond.timeBounds.maxTime:"), QStringLiteral("(180e7)")));
        QVERIFY(reads(QStringLiteral("tx.cond.timeBounds.maxTime:"), QStringLiteral("(Fri Jan 15 08:00:00 UTC 2027)")));
        QVERIFY(reads(QStringLiteral("tx.cond.timeBounds.maxTime:"), QStringLiteral("(Fri Jan 15 12:00:00 +04 2027)")));
        QVERIFY(!reads(QStringLiteral("tx.cond.timeBounds.maxTime:"), QStringLiteral("(1800000001)")));
        QVERIFY(!reads(QStringLiteral("tx.cond.timeBounds.maxTime:"), QStringLiteral("(Fri Jan 15 08:00:00 2027)")));
        // A union's type takes none: not stc's list of the names it could
        // take, which states every other one as much, nor the name again.
        QVERIFY(!reads(QStringLiteral("tx.memo.type:"),
                       QStringLiteral("(MEMO_RETURN, MEMO_NONE, MEMO_TEXT, MEMO_ID, MEMO_HASH)")));
        QVERIFY(!reads(QStringLiteral("tx.operations[0].body.type:"), QStringLiteral("(PAYMENT)")));
        QVERIFY(!reads(QStringLiteral("tx.memo.type:"), QStringLiteral("(MEMO_ID)")));
        QVERIFY(!reads(QStringLiteral("tx.memo.type:"), QStringLiteral("(MEMO_NONE, MEMO_ID)")));
        QVERIFY(!reads(QStringLiteral("tx.memo.type:"), QStringLiteral("(a text memo)")));
        QVERIFY(!reads(QStringLiteral("type:"), QStringLiteral("(ENVELOPE_TYPE_TX_V0)")));
        QVERIFY(!reads(QStringLiteral("tx.operations[0].body.type:"), QStringLiteral("(a payment to the exchange)")));
        // Every other value takes none.
        QVERIFY(!reads(QStringLiteral("tx.memo.text:"), QStringLiteral("(54321)")));
        QVERIFY(!reads(QStringLiteral("tx.memo.text:"), QStringLiteral("(the deposit reference)")));
        QVERIFY(!reads(QStringLiteral("tx.operations[0].body.paymentOp.asset:"), QStringLiteral("(native)")));
        QVERIFY(!reads(QStringLiteral("tx.operations[0].body.paymentOp.asset:"), QStringLiteral("(USD)")));
        QVERIFY(!reads(QStringLiteral("tx.operations[0].sourceAccount._present:"), QStringLiteral("(true)")));
        QVERIFY(!reads(QStringLiteral("tx.operations[0].sourceAccount:"), QStringLiteral("(the exchange)")));
        QVERIFY(!reads(QStringLiteral("tx.cond.type:"), QStringLiteral("(no time bounds)")));
        QString hash = pyDocument("memo_hash");
        QVERIFY(!txrepRefused(hash));
        hash.replace(QStringLiteral("\ntx.memo.hash: "), QStringLiteral("\ntx.memo.hash: 0x"));
        QVERIFY(txrepRefused(hash));
        QString hashNoted = pyDocument("memo_hash");
        const int end = hashNoted.indexOf(QLatin1Char('\n'), hashNoted.indexOf(QStringLiteral("tx.memo.hash: ")));
        hashNoted.insert(end, QStringLiteral(" (the invoice)"));
        QVERIFY(txrepRefused(hashNoted));
    }

    /**
     * A comment on a signature hint says who signed, and a reviewer takes it
     * so. It was read when it named a G... key ending in the hint; four bytes
     * are no proof (a key ending in any four is found in moments), so it
     * could name a signer who had not signed. It must now name a key the
     * signature is valid for, as stc names one: an ed25519 key over the hash,
     * a hash-x signer by its preimage, a signed-payload signer over its
     * payload (stcCorpus()'s signers_x_p) — or be stc's note that it could
     * not tell, and nothing more.
     */
    void testASignatureNoteMustNameTheKeyThatSigned()
    {
        const QString doc = stcDocument("signers_x_p");
        QVERIFY(!txrepRefused(doc));
        const auto changedSignature = [&](int index) {
            const QString key = QStringLiteral("signatures[%1].signature: ").arg(index);
            QString changed = doc;
            const int at = changed.indexOf(key) + key.size();
            changed.replace(at, 1, changed.at(at) == QLatin1Char('0') ? QStringLiteral("1") : QStringLiteral("0"));
            return changed;
        };
        for (int i = 0; i < 3; ++i) {
            const QString changed = changedSignature(i);
            QVERIFY(changed != doc);
            QVERIFY2(txrepRefused(changed), qPrintable(QString::number(i)));
            // Without the note the signature is only carried, as written.
            const int hint = changed.indexOf(QStringLiteral("signatures[%1].hint: ").arg(i));
            const int open = changed.indexOf(QStringLiteral(" ("), hint);
            QString bare = changed;
            bare.remove(open, changed.indexOf(QLatin1Char('\n'), hint) - open);
            QVERIFY2(!txrepRefused(bare), qPrintable(QString::number(i)));
        }
        const QString named = QStringLiteral("(GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR)");
        QVERIFY(doc.contains(named));
        const auto withNote = [&](const QString& note) {
            QString changed = doc;
            changed.replace(named, note);
            return changed;
        };
        QVERIFY(!txrepRefused(withNote(QStringLiteral("(bad signature/unknown key/test is wrong network)"))));
        QVERIFY(txrepRefused(withNote(QStringLiteral("(bad signature/unknown key/test is wrong network, signed by the CFO)"))));
        QVERIFY(txrepRefused(withNote(QStringLiteral("(bad signature/unknown key/the CFO signed; test is wrong network)"))));
        QVERIFY(txrepRefused(withNote(QStringLiteral("(TAAACAQDAQCQMBYIBEFAWDANBYHRAEISCMKBKFQXDAMRUGY4DUPB6ULG)"))));
        QVERIFY(txrepRefused(withNote(QStringLiteral("(GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U)"))));
    }

    /**
     * Go writes a year with as many digits as it has, and stc dates every
     * TimePoint that is a positive int64: public-network transaction
     * 0f27112a... (stcCorpus()'s pubnet_58712) has maxTime 1790626169993,
     * milliseconds given as seconds (a common client bug), which stc dates
     * `(Sat Sep  7 22:19:53 UTC 58712)`. A year of more than four digits was
     * refused, and with it stc's document of that transaction. The date must
     * still be the value's, to the second and the weekday, over all the
     * int64 range Go dates (the dates below are Go 1.26.3's, microseconds
     * and nanoseconds given as seconds among them, the last past the range
     * of an int year), and a year Go does not write is refused.
     */
    void testADateWithAYearOfMoreThanFourDigitsIsRead()
    {
        const QString doc = stcDocument("pubnet_58712");
        const QString date = QStringLiteral("(Sat Sep  7 22:19:53 UTC 58712)");
        QVERIFY(doc.contains(QStringLiteral("maxTime: 1790626169993 ") + date + QLatin1Char('\n')));
        const auto reads = [&](const QString& from, const QString& to) {
            QString changed = doc;
            changed.replace(from, to);
            try {
                QScopedPointer<AbstractTransaction> t(Txrep::fromTxrepEnvelope(changed, Network::publicNetwork()));
                return t->hashHex() == QLatin1String("0f27112a246d29f40ee17e8283f2574b2ccd12937f19e1c54262b45882b621d8");
            } catch (const std::exception&) {
                return false;
            }
        };
        QVERIFY(reads(date, date));                                  // was refused
        QVERIFY(!reads(date, QStringLiteral("(Sat Sep  7 22:19:53 UTC 58713)")));
        QVERIFY(!reads(date, QStringLiteral("(Sat Sep  7 22:19:53 UTC 5871)")));
        QVERIFY(!reads(date, QStringLiteral("(Sat Sep  7 22:19:54 UTC 58712)")));
        QVERIFY(!reads(date, QStringLiteral("(Sat Sep  8 22:19:53 UTC 58712)")));
        QVERIFY(!reads(date, QStringLiteral("(Sun Sep  7 22:19:53 UTC 58712)")));
        QVERIFY(!reads(date, QStringLiteral("(Sat Sep  7 22:19:53 58712)")));
        QVERIFY(!reads(date, QStringLiteral("(Sat Sep  7 22:19:53 UTC 058712)")));   // Go pads to four digits only
        QVERIFY(!reads(date, QStringLiteral("(Sat Sep  7 22:19:53 UTC 58712000000000000000000)")));

        struct GoDate
        {
            qint64 maxTime;
            const char* date;
        };
        const GoDate goDates[] = {
            { Q_INT64_C(1790626169993000), "Mon Jun 19 11:23:20 UTC 56744654" },
            { Q_INT64_C(1790626169993000000), "Fri May 12 12:53:20 UTC 56742686434" },
            { Q_INT64_C(1790626169993000000), "Fri May 12 14:53:20 CEST 56742686434" },     // in Madrid
            { Q_INT64_C(67767976233532799), "Tue Dec 31 23:59:59 UTC 2147483647" },
            { Q_INT64_C(67767976233532800), "Wed Jan  1 00:00:00 UTC 2147483648" },
            { Q_INT64_C(67767976233532799), "Wed Jan  1 00:59:59 CET 2147483648" },
            { Q_INT64_C(9223372036854775807), "Sun Dec  4 15:30:07 UTC 292277026596" },
        };
        for (const GoDate& d : goDates) {
            Account account(KeyPair::fromAccountId(QStringLiteral("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG")), 41);
            QScopedPointer<Transaction> tx(TransactionBuilder(&account, Network::testnetNetwork())
                .addOperation(PaymentOperation::create(QStringLiteral("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"),
                                                       new AssetTypeNative(), QStringLiteral("1")))
                .addTimeBounds(new TimeBounds(0, d.maxTime))
                .setBaseFee(100)
                .build());
            const QString bare = Txrep::toTxrep(tx.data());
            const QString line = QStringLiteral("tx.cond.timeBounds.maxTime: %1\n").arg(d.maxTime);
            QVERIFY2(bare.contains(line), d.date);
            const auto readsWith = [&](const QString& said) {
                QString changed = bare;
                changed.replace(line, line.left(line.size() - 1) + QStringLiteral(" (") + said + QStringLiteral(")\n"));
                try {
                    QScopedPointer<Transaction> t(Txrep::fromTxrep(changed, Network::testnetNetwork()));
                    return t->hash() == tx->hash();
                } catch (const std::exception&) {
                    return false;
                }
            };
            const QString said = QLatin1String(d.date);
            QVERIFY2(readsWith(said), d.date);
            // A year on, and a second on: a long year is compared whole.
            const int space = said.lastIndexOf(QLatin1Char(' '));
            const qint64 year = said.mid(space + 1).toLongLong();
            QVERIFY2(!readsWith(said.left(space + 1) + QString::number(year + 1)), d.date);
            QVERIFY2(!readsWith(said.left(space + 1) + QString::number(year - 1)), d.date);
            QString second = said;
            second.replace(QStringLiteral(":20 "), QStringLiteral(":21 "));
            second.replace(QStringLiteral(":59 "), QStringLiteral(":58 "));
            second.replace(QStringLiteral(":00 "), QStringLiteral(":01 "));
            second.replace(QStringLiteral(":07 "), QStringLiteral(":08 "));
            QVERIFY2(second != said && !readsWith(second), d.date);
        }
    }

    /**
     * Digits grouped by commas are read only as stc writes them (ScaleFmt:
     * the first group from 1 to 999, groups of three, a fraction with no
     * trailing zero, a lowercase e7). They were read in any grouping: a
     * leading "0," is how a locale with a decimal comma writes a fraction,
     * so "(0,100e7)" after 100 units, which was read, is 0.1e7 there, a
     * thousandth of what is signed; "(00,001,000e7)" and "(0,000,000,100e7)"
     * were read too.
     */
    void testAGroupedNumberIsReadOnlyAsStcWritesIt()
    {
        const auto reads = [](const QString& units, const QString& comment) {
            QScopedPointer<Transaction> tx(transactionOf(
                PaymentOperation::create(QStringLiteral("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"),
                                         new AssetTypeNative(), units)));
            const QString doc = withoutGlosses(Txrep::toTxrep(tx.data()));
            const int at = doc.indexOf(QStringLiteral("paymentOp.amount: "));
            const int end = doc.indexOf(QLatin1Char('\n'), at);
            if (at < 0 || end < 0)
                return false;
            QString changed = doc;
            changed.insert(end, QLatin1Char(' ') + comment);
            try {
                QScopedPointer<Transaction> t(Txrep::fromTxrep(changed, Network::testnetNetwork()));
                return t->hash() == tx->hash();
            } catch (const std::exception&) {
                return false;
            }
        };
        // As stc writes them.
        QVERIFY(reads(QStringLiteral("100"), QStringLiteral("(100e7)")));
        QVERIFY(reads(QStringLiteral("1000"), QStringLiteral("(1,000e7)")));
        QVERIFY(reads(QStringLiteral("1000.5"), QStringLiteral("(1,000.5e7)")));
        QVERIFY(reads(QStringLiteral("12345.6789012"), QStringLiteral("(12,345.6789012e7)")));
        QVERIFY(reads(QStringLiteral("1000000"), QStringLiteral("(1,000,000e7)")));
        // A leading zero group, which a decimal-comma reader takes for a
        // fraction; each was read.
        QVERIFY(!reads(QStringLiteral("100"), QStringLiteral("(0,100e7)")));
        QVERIFY(!reads(QStringLiteral("1000"), QStringLiteral("(00,001,000e7)")));
        QVERIFY(!reads(QStringLiteral("100"), QStringLiteral("(0,000,000,100e7)")));
        QVERIFY(!reads(QStringLiteral("1000"), QStringLiteral("(01,000e7)")));
        QVERIFY(!reads(QStringLiteral("1000"), QStringLiteral("(0,001,000e7)")));
        // Nor any other spelling stc never writes; each was read.
        QVERIFY(!reads(QStringLiteral("1000"), QStringLiteral("(1,000E7)")));
        QVERIFY(!reads(QStringLiteral("1000"), QStringLiteral("(+1,000e7)")));
        QVERIFY(!reads(QStringLiteral("1000"), QStringLiteral("(1,000e+7)")));
        QVERIFY(!reads(QStringLiteral("1000.5"), QStringLiteral("(1,000.50e7)")));
        QVERIFY(!reads(QStringLiteral("1000"), QStringLiteral("(1,000.0e7)")));
        // Ungrouped, as before.
        QVERIFY(reads(QStringLiteral("100"), QStringLiteral("(100)")));
        QVERIFY(!reads(QStringLiteral("100"), QStringLiteral("(0,1e7)")));
    }

    /**
     * stc dates a time at the local zone's abbreviation: on Windows in
     * Madrid, where this user runs it, CET and CEST (Go takes them from the
     * zone Windows is set to; on Linux, from the tz database). They were
     * refused with every abbreviation, since one can name several offsets.
     * An abbreviation that names one, in every tz release and wherever else
     * it is read, now reads at that offset (txrep.cpp's table: CET +01:00,
     * CEST +02:00), in stc's own documents of this machine (stcCorpus()'s
     * pubnet_summer_madrid, a summer date, and pubnet_58712_madrid, whose
     * September of 58712 Go on Windows writes as CET, past the century of
     * rules it takes from Windows; pubnet_58712_madrid_tzdata writes CEST,
     * as the tz database does); one that names several (IST, BST, PST,
     * PDT...) is refused as before: stc's own dates of the same
     * transactions in Los Angeles, London and Kolkata.
     */
    void testADateAtAnAbbreviationThatNamesOneOffsetIsRead()
    {
        const auto readsAs = [](const QString& document, const char* hash) {
            try {
                QScopedPointer<AbstractTransaction> t(Txrep::fromTxrepEnvelope(document, Network::publicNetwork()));
                return t->hashHex() == QLatin1String(hash);
            } catch (const std::exception&) {
                return false;
            }
        };
        const char* summerHash = "700851e730fdb2e01d4a1a763e135c69fb105c91f0a9855afbf1be467330eb34";
        const char* hash58712 = "0f27112a246d29f40ee17e8283f2574b2ccd12937f19e1c54262b45882b621d8";
        const QString summer = stcDocument("pubnet_summer_madrid");
        const QString summerDate = QStringLiteral("(Sun Jul 19 15:55:19 CEST 2026)");
        QVERIFY(summer.contains(QStringLiteral("maxTime: 1784469319 ") + summerDate + QLatin1Char('\n')));
        QVERIFY(readsAs(summer, summerHash));                       // was refused
        const QString windows = stcDocument("pubnet_58712_madrid");
        QVERIFY(windows.contains(QStringLiteral("minTime: 1000 (Thu Jan  1 01:16:40 CET 1970)\n")));
        QVERIFY(windows.contains(QStringLiteral("maxTime: 1790626169993 (Sat Sep  7 23:19:53 CET 58712)\n")));
        QVERIFY(readsAs(windows, hash58712));                       // was refused
        const QString tzdata = stcDocument("pubnet_58712_madrid_tzdata");
        QVERIFY(tzdata.contains(QStringLiteral("maxTime: 1790626169993 (Sun Sep  8 00:19:53 CEST 58712)\n")));
        QVERIFY(readsAs(tzdata, hash58712));                        // was refused

        const auto summerWith = [&](const QString& date) {
            QString changed = summer;
            changed.replace(summerDate, date);
            return changed;
        };
        // Each at its one offset: the same instant at CET reads, an hour off does not.
        QVERIFY(readsAs(summerWith(QStringLiteral("(Sun Jul 19 14:55:19 CET 2026)")), summerHash));
        QVERIFY(readsAs(summerWith(QStringLiteral("(Sun Jul 19 13:55:19 UTC 2026)")), summerHash));
        QVERIFY(!readsAs(summerWith(QStringLiteral("(Sun Jul 19 15:55:19 CET 2026)")), summerHash));
        QVERIFY(!readsAs(summerWith(QStringLiteral("(Sun Jul 19 14:55:19 CEST 2026)")), summerHash));
        QVERIFY(!readsAs(summerWith(QStringLiteral("(Sun Jul 19 15:55:19 cest 2026)")), summerHash));
        // stc's dates of the same instants where the abbreviation names
        // several offsets (Go 1.26.3, the zone's tz database rules): refused,
        // and the documents read without them.
        const QStringList several = {
            QStringLiteral("(Sun Jul 19 06:55:19 PDT 2026)"),       // Los Angeles; PDT is Manila's too
            QStringLiteral("(Sun Jul 19 14:55:19 BST 2026)"),       // London; BST was Adak's
            QStringLiteral("(Sun Jul 19 19:25:19 IST 2026)"),       // Kolkata; IST is Dublin's and Jerusalem's
        };
        for (const QString& date : several)
            QVERIFY2(!readsAs(summerWith(date), summerHash), qPrintable(date));
        // The refusal says which abbreviations read (it said none did).
        try {
            QScopedPointer<AbstractTransaction> t(Txrep::fromTxrepEnvelope(summerWith(several.last()),
                                                                           Network::publicNetwork()));
            QFAIL("an IST date was read");
        } catch (const std::exception& e) {
            QVERIFY2(QString::fromLatin1(e.what()).contains(QStringLiteral("abbreviation that names one offset")),
                     e.what());
        }
        QString undated = summer;
        undated.replace(QLatin1Char(' ') + summerDate, QString());
        QVERIFY(undated != summer && readsAs(undated, summerHash));
        const QString minTime = QStringLiteral("(Thu Jan  1 01:16:40 CET 1970)");
        const QStringList severalAt1000 = {
            QStringLiteral("(Wed Dec 31 16:16:40 PST 1969)"),       // Los Angeles
            QStringLiteral("(Thu Jan  1 01:16:40 BST 1970)"),       // London, British Standard Time
            QStringLiteral("(Thu Jan  1 05:46:40 IST 1970)"),       // Kolkata
        };
        for (const QString& date : severalAt1000) {
            QString changed = windows;
            changed.replace(minTime, date);
            QVERIFY2(changed != windows && !readsAs(changed, hash58712), qPrintable(date));
        }
        // An abbreviation in no zone of the tz database, as Go makes one of
        // a Windows zone's capitals when it knows none, names no offset.
        QVERIFY(!readsAs(summerWith(QStringLiteral("(Sun Jul 19 15:55:19 RDT 2026)")), summerHash));
    }

    /**
     * An abbreviation is read only if no one reads it at another offset. The
     * first table of 1.0.4 held every abbreviation that names one offset in
     * tzdata 2026d, and so read "(Sun Jul 19 23:55:19 GST 2026)" at +10:00
     * (Guam's GST, before 2000), where a reader in Dubai, whose time every
     * tz release to 2016j, Go, Java and CLDR call GST, takes it for +04:00,
     * six hours off. The table now holds only what every tz release since
     * 1993 and every other source checked (txrep.cpp) read at one offset:
     * GST, AST (Arabia's +03:00 to 2016j), EST (Sydney's +10:00 to 2014) and
     * BDT (Dhaka's +06:00 to 2016j) are refused at either offset, on stc's
     * own document of public transaction 700851e7... with its date changed,
     * and read with the offset in digits. CET and CEST, which stc writes on
     * Windows in Madrid, read as before (stcCorpus()'s Madrid documents),
     * and so do JST and EAT at their offsets.
     */
    void testADateAtAnAbbreviationReadAtAnotherOffsetIsRefused()
    {
        const auto readsAs = [](const QString& document, const char* hash) {
            try {
                QScopedPointer<AbstractTransaction> t(Txrep::fromTxrepEnvelope(document, Network::publicNetwork()));
                return t->hashHex() == QLatin1String(hash);
            } catch (const std::exception&) {
                return false;
            }
        };
        const char* summerHash = "700851e730fdb2e01d4a1a763e135c69fb105c91f0a9855afbf1be467330eb34";
        const char* hash58712 = "0f27112a246d29f40ee17e8283f2574b2ccd12937f19e1c54262b45882b621d8";
        // stc's own, in Madrid: CEST in summer, CET in winter and past the
        // century of rules Go takes from Windows, CEST from the tz database.
        const QString summer = stcDocument("pubnet_summer_madrid");
        const QString summerDate = QStringLiteral("(Sun Jul 19 15:55:19 CEST 2026)");
        QVERIFY(summer.contains(QStringLiteral("maxTime: 1784469319 ") + summerDate + QLatin1Char('\n')));
        QVERIFY(readsAs(summer, summerHash));
        const QString windows = stcDocument("pubnet_58712_madrid");
        QVERIFY(windows.contains(QStringLiteral("(Thu Jan  1 01:16:40 CET 1970)")));
        QVERIFY(windows.contains(QStringLiteral("(Sat Sep  7 23:19:53 CET 58712)")));
        QVERIFY(readsAs(windows, hash58712));
        const QString tzdata = stcDocument("pubnet_58712_madrid_tzdata");
        QVERIFY(tzdata.contains(QStringLiteral("(Sun Sep  8 00:19:53 CEST 58712)")));
        QVERIFY(readsAs(tzdata, hash58712));

        const auto summerWith = [&](const QString& date) {
            QString changed = summer;
            changed.replace(summerDate, date);
            return changed;
        };
        // 2026-07-19 13:55:19 UTC, at the offset the first table read each
        // at, and at the one it is read at elsewhere: refused at both.
        const QStringList refused = {
            QStringLiteral("(Sun Jul 19 23:55:19 GST 2026)"),   // +10:00, the first table's (Guam to 2000)
            QStringLiteral("(Sun Jul 19 17:55:19 GST 2026)"),   // +04:00, Dubai
            QStringLiteral("(Sun Jul 19 09:55:19 AST 2026)"),   // -04:00, Halifax
            QStringLiteral("(Sun Jul 19 16:55:19 AST 2026)"),   // +03:00, Riyadh
            QStringLiteral("(Sun Jul 19 08:55:19 EST 2026)"),   // -05:00, New York
            QStringLiteral("(Sun Jul 19 23:55:19 EST 2026)"),   // +10:00, Sydney to 2014
            QStringLiteral("(Sun Jul 19 03:55:19 BDT 2026)"),   // -10:00, Nome's Bering Daylight to 1983
            QStringLiteral("(Sun Jul 19 19:55:19 BDT 2026)"),   // +06:00, Dhaka
        };
        for (const QString& date : refused)
            QVERIFY2(!readsAs(summerWith(date), summerHash), qPrintable(date));
        // The same instants with their offsets in digits, as stc writes a
        // zone with no name, read.
        QVERIFY(readsAs(summerWith(QStringLiteral("(Sun Jul 19 17:55:19 +04 2026)")), summerHash));
        QVERIFY(readsAs(summerWith(QStringLiteral("(Sun Jul 19 23:55:19 +10 2026)")), summerHash));
        QVERIFY(readsAs(summerWith(QStringLiteral("(Sun Jul 19 19:55:19 +06 2026)")), summerHash));
        // An abbreviation that stays reads at its offset, and only there.
        QVERIFY(readsAs(summerWith(QStringLiteral("(Sun Jul 19 22:55:19 JST 2026)")), summerHash));
        QVERIFY(!readsAs(summerWith(QStringLiteral("(Sun Jul 19 23:55:19 JST 2026)")), summerHash));
        QVERIFY(readsAs(summerWith(QStringLiteral("(Sun Jul 19 16:55:19 EAT 2026)")), summerHash));
        QVERIFY(!readsAs(summerWith(QStringLiteral("(Sun Jul 19 15:55:19 EAT 2026)")), summerHash));
        QVERIFY(readsAs(summerWith(QStringLiteral("(Sun Jul 19 14:55:19 CET 2026)")), summerHash));
        QVERIFY(!readsAs(summerWith(QStringLiteral("(Sun Jul 19 15:55:19 CET 2026)")), summerHash));
    }

    /**
     * No abbreviation dropped from the first table of 1.0.4 comes back, at
     * any offset: each of the 39 is refused at the offset that table read it
     * at (txrep.cpp says where it is read at another) and at every quarter
     * hour from -12:00 to +14:00, and so is each of the 8 that name several
     * offsets in tzdata 2026d alone. Each of the 16 kept reads at its
     * offset, and an hour either side is refused; UTC and GMT, which that
     * table held too, are read before it, at +00:00 only. The dates are
     * written as Go's UnixDate writes them, checked on stc's own CEST date.
     */
    void testNoAbbreviationReadAtSeveralOffsetsComesBack()
    {
        const qint64 instant = Q_INT64_C(1784469319);   // pubnet_summer_madrid's maxTime
        const auto unixDate = [](qint64 t, int offset, const char* zone) {
            static const char* const weekdays[] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
            static const char* const months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                                  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
            qint64 days = (t + offset) / 86400;
            qint64 second = (t + offset) % 86400;
            if (second < 0) {
                second += 86400;
                --days;
            }
            const QDate date = QDate(1970, 1, 1).addDays(days);
            return QStringLiteral("%1 %2 %3 %4:%5:%6 %7 %8")
                .arg(QLatin1String(weekdays[date.dayOfWeek() - 1]), QLatin1String(months[date.month() - 1]))
                .arg(date.day(), 2, 10, QLatin1Char(' '))
                .arg(int(second / 3600), 2, 10, QLatin1Char('0'))
                .arg(int(second / 60 % 60), 2, 10, QLatin1Char('0'))
                .arg(int(second % 60), 2, 10, QLatin1Char('0'))
                .arg(QLatin1String(zone))
                .arg(date.year());
        };
        QCOMPARE(unixDate(instant, 7200, "CEST"), QStringLiteral("Sun Jul 19 15:55:19 CEST 2026"));
        QCOMPARE(unixDate(Q_INT64_C(1000), 3600, "CET"), QStringLiteral("Thu Jan  1 01:16:40 CET 1970"));

        Account account(KeyPair::fromAccountId(QStringLiteral("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG")), 41);
        QScopedPointer<Transaction> tx(TransactionBuilder(&account, Network::testnetNetwork())
            .addOperation(PaymentOperation::create(QStringLiteral("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"),
                                                   new AssetTypeNative(), QStringLiteral("1")))
            .addTimeBounds(new TimeBounds(0, instant))
            .setBaseFee(100)
            .build());
        const QString bare = withoutGlosses(Txrep::toTxrep(tx.data()));
        const QString line = QStringLiteral("tx.cond.timeBounds.maxTime: %1\n").arg(instant);
        QVERIFY(bare.contains(line));
        const auto readsWith = [&](const QString& said) {
            QString changed = bare;
            changed.replace(line, line.left(line.size() - 1) + QStringLiteral(" (") + said + QStringLiteral(")\n"));
            try {
                QScopedPointer<Transaction> t(Txrep::fromTxrep(changed, Network::testnetNetwork()));
                return t->hash() == tx->hash();
            } catch (const std::exception&) {
                return false;
            }
        };
        QVERIFY(readsWith(unixDate(instant, 0, "UTC")));
        QVERIFY(readsWith(unixDate(instant, 7200, "CEST")));
        // UTC and GMT at +00:00 only, though old Go on Windows wrote UTC at
        // -09:00, -08:00 and +13:00 (Windows' "UTC-09", "UTC-08", "UTC+13").
        QVERIFY(readsWith(unixDate(instant, 0, "GMT")));
        for (const int offset : { -9 * 3600, -8 * 3600, 13 * 3600 }) {
            QVERIFY(!readsWith(unixDate(instant, offset, "UTC")));
            QVERIFY(!readsWith(unixDate(instant, offset, "GMT")));
        }

        struct Abbreviation
        {
            const char* name;
            int offset;                             // seconds east of UTC
        };
        const int hour = 3600;
        const Abbreviation kept[] = {
            { "AHST", -10 * hour }, { "AHDT", -9 * hour }, { "AKDT", -8 * hour }, { "ADDT", -2 * hour },
            { "NDDT", -hour - 1800 }, { "CET", hour }, { "CEST", 2 * hour }, { "EAT", 3 * hour },
            { "WIB", 7 * hour }, { "WITA", 8 * hour }, { "AWDT", 9 * hour }, { "JST", 9 * hour },
            { "ChST", 10 * hour }, { "ACDT", 10 * hour + 1800 }, { "AEDT", 11 * hour }, { "NZDT", 13 * hour },
        };
        QCOMPARE(int(sizeof(kept) / sizeof(kept[0])), 16);
        for (const Abbreviation& a : kept) {
            QVERIFY2(readsWith(unixDate(instant, a.offset, a.name)), a.name);
            QVERIFY2(!readsWith(unixDate(instant, a.offset + hour, a.name)), a.name);
            QVERIFY2(!readsWith(unixDate(instant, a.offset - hour, a.name)), a.name);
        }
        // Each at the offset the first table of 1.0.4 read it at.
        const Abbreviation dropped[] = {
            { "ACST", 9 * hour + 1800 }, { "ADT", -3 * hour }, { "AEST", 10 * hour }, { "AKST", -9 * hour },
            { "AST", -4 * hour }, { "AWST", 8 * hour }, { "BDT", -10 * hour }, { "CAST", 3 * hour },
            { "CAT", 2 * hour }, { "EDT", -4 * hour }, { "EEST", 3 * hour }, { "EET", 2 * hour },
            { "EST", -5 * hour }, { "GDT", 11 * hour }, { "GST", 10 * hour }, { "HDT", -9 * hour },
            { "HST", -10 * hour }, { "IDT", 3 * hour }, { "KDT", 10 * hour }, { "MDT", -6 * hour },
            { "MMT", -(44 * 60 + 30) }, { "MSD", 4 * hour }, { "MST", -7 * hour }, { "NDT", -2 * hour - 1800 },
            { "NST", -3 * hour - 1800 }, { "NZST", 12 * hour }, { "PKT", 5 * hour }, { "SAST", 2 * hour },
            { "SST", -11 * hour }, { "WAST", 2 * hour }, { "WAT", hour }, { "WET", 0 },
            { "WIT", 9 * hour }, { "YDT", -8 * hour }, { "YST", -9 * hour },
            // Read until the table was checked against CLDR from 1970 and
            // against the "Standard" an S is read as.
            { "HKT", 8 * hour }, { "HKST", 9 * hour }, { "PKST", 6 * hour },
            // Read until that "Standard" was held against it too: "Western
            // European Standard Time" is CLDR's +00:00, and Java writes WEST
            // at +02:00 for Lisbon's summers of 1992-1996.
            { "WEST", hour },
        };
        QCOMPARE(int(sizeof(dropped) / sizeof(dropped[0])), 39);
        for (const Abbreviation& a : dropped) {
            QVERIFY2(!readsWith(unixDate(instant, a.offset, a.name)), a.name);
            for (int offset = -12 * hour; offset <= 14 * hour; offset += 900) {
                const QString date = unixDate(instant, offset, a.name);
                QVERIFY2(!readsWith(date), qPrintable(date));
            }
        }
        for (const char* name : { "BST", "CDT", "CST", "IST", "KST", "MSK", "PDT", "PST" }) {
            for (int offset = -12 * hour; offset <= 14 * hour; offset += 900) {
                const QString date = unixDate(instant, offset, name);
                QVERIFY2(!readsWith(date), qPrintable(date));
            }
        }
    }

private:
    static Transaction* transactionOf(Operation* operation)
    {
        Account account(QStringLiteral("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"), 0);
        return TransactionBuilder(&account, Network::testnetNetwork())
            .addOperation(operation)
            .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
            .setBaseFee(100)
            .build();
    }

    /**
     * What py-stellar-sdk 14.1.0's from_txrep takes from a document
     * (_get_raw_data_map): per line the key, and the value up to its first
     * space, or a quoted value up to its closing quote with `\n` read as a
     * newline and `\x` as x. What py builds depends on nothing else, so a
     * document that gives the same map is read by py as the same transaction.
     */
    static QMap<QString, QString> pyReads(const QString& document)
    {
        QMap<QString, QString> map;
        for (const QString& line : document.trimmed().split(QLatin1Char('\n'))) {
            if (line.startsWith(QLatin1Char(':')) || line.trimmed().isEmpty())
                continue;
            const int colon = line.indexOf(QLatin1Char(':'));
            if (colon < 0)
                continue;
            QString value = line.mid(colon + 1).trimmed();
            if (value.startsWith(QLatin1Char('"'))) {
                QString read(QLatin1Char('"'));
                bool escaped = false;
                for (int i = 1; i < value.size(); ++i) {
                    const QChar c = value.at(i);
                    if (escaped) {
                        read += c == QLatin1Char('n') ? QChar(QLatin1Char('\n')) : c;
                        escaped = false;
                    } else if (c == QLatin1Char('\\')) {
                        escaped = true;
                    } else {
                        read += c;
                        if (c == QLatin1Char('"'))
                            break;
                    }
                }
                value = read;
            } else {
                value = value.section(QLatin1Char(' '), 0, 0);
            }
            map.insert(line.left(colon).trimmed(), value);
        }
        return map;
    }

    // ─── The py-stellar-sdk 14.1.0 corpus ───────────────────────────────

    /** A document written by py-stellar-sdk's to_txrep, and its envelope. */
    struct PyCorpusCase
    {
        const char* name;
        const char* txrep;
        const char* envelope;
        const char* hash;
    };

    static const PyCorpusCase* pyCase(const char* name)
    {
        for (const std::vector<PyCorpusCase>* corpus : {&pyCorpus(), &pyEnvelopeCorpus()}) {
            for (const PyCorpusCase& c : *corpus) {
                if (qstrcmp(c.name, name) == 0)
                    return &c;
            }
        }
        return nullptr;
    }

    static QString pyDocument(const char* name)
    {
        const PyCorpusCase* c = pyCase(name);
        return c ? QString::fromLatin1(c->txrep) : QString();
    }

    static QString pyEnvelope(const char* name)
    {
        const PyCorpusCase* c = pyCase(name);
        return c ? QString::fromLatin1(c->envelope) : QString();
    }

    /**
     * Written with py-stellar-sdk 14.1.0 (to_txrep of TransactionEnvelope,
     * testnet): keys from the raw seeds 0x01..0x05 repeated (source
     * GCFIRY..., A = GCATS..., B = GDWUSK..., issuers GDFJHL... for USD and
     * GBXHUH... for EUR and LONGASSET012), M... addresses of those with ids
     * 9007199254740993, 1 and 2^64 - 1. "operations" holds every classic
     * operation and union arm TxRep covers; "muxed" every MuxedAccount
     * field; then every memo, every precondition shape, and the extremes of
     * sequence number and fee. py reads each back to its own envelope except
     * memo_text_unicode (it writes \u escapes it does not read),
     * claimable_unconditional (it does not read CLAIM_PREDICATE_UNCONDITIONAL)
     * and manage_data_empty_value (it fails on its own empty value): refusals
     * or py's own misreading, the same for this SDK's document.
     */
    static const std::vector<PyCorpusCase>& pyCorpus()
    {
        static const std::vector<PyCorpusCase> cases = {
            { "operations",
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 4600\n"
                "tx.seqNum: 1234567890123\n"
                "tx.cond.type: PRECOND_TIME\n"
                "tx.cond.timeBounds.minTime: 1700000000 (2023-11-14 22:13:20.000000+00:00 (UTC))\n"
                "tx.cond.timeBounds.maxTime: 1800000000 (2027-01-15 08:00:00.000000+00:00 (UTC))\n"
                "tx.memo.type: MEMO_NONE\n"
                "tx.operations.len: 46\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: CREATE_ACCOUNT\n"
                "tx.operations[0].body.createAccountOp.destination: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[0].body.createAccountOp.startingBalance: 125000000 (12.5)\n"
                "tx.operations[1].sourceAccount._present: false\n"
                "tx.operations[1].body.type: PAYMENT\n"
                "tx.operations[1].body.paymentOp.destination: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[1].body.paymentOp.asset: native\n"
                "tx.operations[1].body.paymentOp.amount: 105000000 (10.5)\n"
                "tx.operations[2].sourceAccount._present: true\n"
                "tx.operations[2].sourceAccount: GDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435DIAG\n"
                "tx.operations[2].body.type: PAYMENT\n"
                "tx.operations[2].body.paymentOp.destination: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[2].body.paymentOp.asset: USD:GDFJHLAXAUMHA4OWPOB4P7YO72AQR2HMIUYFOXLXE2DZGM633K7HZDQP\n"
                "tx.operations[2].body.paymentOp.amount: 1 (0.0000001)\n"
                "tx.operations[3].sourceAccount._present: false\n"
                "tx.operations[3].body.type: PAYMENT\n"
                "tx.operations[3].body.paymentOp.destination: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[3].body.paymentOp.asset: LONGASSET012:GBXHUHG5FGYLPD6RHL2MKWMP572O6KUXCZXDZJXS4T57ZTMAKBN7DWXN\n"
                "tx.operations[3].body.paymentOp.amount: 9223372036854775807 (922337203685.4775807)\n"
                "tx.operations[4].sourceAccount._present: false\n"
                "tx.operations[4].body.type: PATH_PAYMENT_STRICT_RECEIVE\n"
                "tx.operations[4].body.pathPaymentStrictReceiveOp.sendAsset: native\n"
                "tx.operations[4].body.pathPaymentStrictReceiveOp.sendMax: 50000000 (5)\n"
                "tx.operations[4].body.pathPaymentStrictReceiveOp.destination: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[4].body.pathPaymentStrictReceiveOp.destAsset: USD:GDFJHLAXAUMHA4OWPOB4P7YO72AQR2HMIUYFOXLXE2DZGM633K7HZDQP\n"
                "tx.operations[4].body.pathPaymentStrictReceiveOp.destAmount: 40000000 (4)\n"
                "tx.operations[4].body.pathPaymentStrictReceiveOp.path.len: 0\n"
                "tx.operations[5].sourceAccount._present: false\n"
                "tx.operations[5].body.type: PATH_PAYMENT_STRICT_RECEIVE\n"
                "tx.operations[5].body.pathPaymentStrictReceiveOp.sendAsset: USD:GDFJHLAXAUMHA4OWPOB4P7YO72AQR2HMIUYFOXLXE2DZGM633K7HZDQP\n"
                "tx.operations[5].body.pathPaymentStrictReceiveOp.sendMax: 50000000 (5)\n"
                "tx.operations[5].body.pathPaymentStrictReceiveOp.destination: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[5].body.pathPaymentStrictReceiveOp.destAsset: LONGASSET012:GBXHUHG5FGYLPD6RHL2MKWMP572O6KUXCZXDZJXS4T57ZTMAKBN7DWXN\n"
                "tx.operations[5].body.pathPaymentStrictReceiveOp.destAmount: 40000000 (4)\n"
                "tx.operations[5].body.pathPaymentStrictReceiveOp.path.len: 2\n"
                "tx.operations[5].body.pathPaymentStrictReceiveOp.path[0]: native\n"
                "tx.operations[5].body.pathPaymentStrictReceiveOp.path[1]: EUR:GBXHUHG5FGYLPD6RHL2MKWMP572O6KUXCZXDZJXS4T57ZTMAKBN7DWXN\n"
                "tx.operations[6].sourceAccount._present: false\n"
                "tx.operations[6].body.type: MANAGE_SELL_OFFER\n"
                "tx.operations[6].body.manageSellOfferOp.selling: native\n"
                "tx.operations[6].body.manageSellOfferOp.buying: USD:GDFJHLAXAUMHA4OWPOB4P7YO72AQR2HMIUYFOXLXE2DZGM633K7HZDQP\n"
                "tx.operations[6].body.manageSellOfferOp.amount: 1000000000 (100)\n"
                "tx.operations[6].body.manageSellOfferOp.price.n: 7\n"
                "tx.operations[6].body.manageSellOfferOp.price.d: 9\n"
                "tx.operations[6].body.manageSellOfferOp.offerID: 0\n"
                "tx.operations[7].sourceAccount._present: false\n"
                "tx.operations[7].body.type: MANAGE_SELL_OFFER\n"
                "tx.operations[7].body.manageSellOfferOp.selling: USD:GDFJHLAXAUMHA4OWPOB4P7YO72AQR2HMIUYFOXLXE2DZGM633K7HZDQP\n"
                "tx.operations[7].body.manageSellOfferOp.buying: LONGASSET012:GBXHUHG5FGYLPD6RHL2MKWMP572O6KUXCZXDZJXS4T57ZTMAKBN7DWXN\n"
                "tx.operations[7].body.manageSellOfferOp.amount: 0 (0)\n"
                "tx.operations[7].body.manageSellOfferOp.price.n: 1\n"
                "tx.operations[7].body.manageSellOfferOp.price.d: 2147483647\n"
                "tx.operations[7].body.manageSellOfferOp.offerID: 12345\n"
                "tx.operations[8].sourceAccount._present: false\n"
                "tx.operations[8].body.type: CREATE_PASSIVE_SELL_OFFER\n"
                "tx.operations[8].body.createPassiveSellOfferOp.selling: USD:GDFJHLAXAUMHA4OWPOB4P7YO72AQR2HMIUYFOXLXE2DZGM633K7HZDQP\n"
                "tx.operations[8].body.createPassiveSellOfferOp.buying: native\n"
                "tx.operations[8].body.createPassiveSellOfferOp.amount: 30000000 (3)\n"
                "tx.operations[8].body.createPassiveSellOfferOp.price.n: 2147483647\n"
                "tx.operations[8].body.createPassiveSellOfferOp.price.d: 1\n"
                "tx.operations[9].sourceAccount._present: false\n"
                "tx.operations[9].body.type: SET_OPTIONS\n"
                "tx.operations[9].body.setOptionsOp.inflationDest._present: false\n"
                "tx.operations[9].body.setOptionsOp.clearFlags._present: false\n"
                "tx.operations[9].body.setOptionsOp.setFlags._present: false\n"
                "tx.operations[9].body.setOptionsOp.masterWeight._present: false\n"
                "tx.operations[9].body.setOptionsOp.lowThreshold._present: false\n"
                "tx.operations[9].body.setOptionsOp.medThreshold._present: false\n"
                "tx.operations[9].body.setOptionsOp.highThreshold._present: false\n"
                "tx.operations[9].body.setOptionsOp.homeDomain._present: false\n"
                "tx.operations[9].body.setOptionsOp.signer._present: false\n"
                "tx.operations[10].sourceAccount._present: false\n"
                "tx.operations[10].body.type: SET_OPTIONS\n"
                "tx.operations[10].body.setOptionsOp.inflationDest._present: true\n"
                "tx.operations[10].body.setOptionsOp.inflationDest: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[10].body.setOptionsOp.clearFlags._present: true\n"
                "tx.operations[10].body.setOptionsOp.clearFlags: 2\n"
                "tx.operations[10].body.setOptionsOp.setFlags._present: true\n"
                "tx.operations[10].body.setOptionsOp.setFlags: 9\n"
                "tx.operations[10].body.setOptionsOp.masterWeight._present: true\n"
                "tx.operations[10].body.setOptionsOp.masterWeight: 255\n"
                "tx.operations[10].body.setOptionsOp.lowThreshold._present: true\n"
                "tx.operations[10].body.setOptionsOp.lowThreshold: 0\n"
                "tx.operations[10].body.setOptionsOp.medThreshold._present: true\n"
                "tx.operations[10].body.setOptionsOp.medThreshold: 2\n"
                "tx.operations[10].body.setOptionsOp.highThreshold._present: true\n"
                "tx.operations[10].body.setOptionsOp.highThreshold: 3\n"
                "tx.operations[10].body.setOptionsOp.homeDomain._present: true\n"
                "tx.operations[10].body.setOptionsOp.homeDomain: \"example.com\"\n"
                "tx.operations[10].body.setOptionsOp.signer._present: true\n"
                "tx.operations[10].body.setOptionsOp.signer.key: GDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435DIAG\n"
                "tx.operations[10].body.setOptionsOp.signer.weight: 1\n"
                "tx.operations[11].sourceAccount._present: false\n"
                "tx.operations[11].body.type: SET_OPTIONS\n"
                "tx.operations[11].body.setOptionsOp.inflationDest._present: false\n"
                "tx.operations[11].body.setOptionsOp.clearFlags._present: false\n"
                "tx.operations[11].body.setOptionsOp.setFlags._present: false\n"
                "tx.operations[11].body.setOptionsOp.masterWeight._present: false\n"
                "tx.operations[11].body.setOptionsOp.lowThreshold._present: false\n"
                "tx.operations[11].body.setOptionsOp.medThreshold._present: false\n"
                "tx.operations[11].body.setOptionsOp.highThreshold._present: false\n"
                "tx.operations[11].body.setOptionsOp.homeDomain._present: false\n"
                "tx.operations[11].body.setOptionsOp.signer._present: true\n"
                "tx.operations[11].body.setOptionsOp.signer.key: TAAACAQDAQCQMBYIBEFAWDANBYHRAEISCMKBKFQXDAMRUGY4DUPB6ULG\n"
                "tx.operations[11].body.setOptionsOp.signer.weight: 0\n"
                "tx.operations[12].sourceAccount._present: false\n"
                "tx.operations[12].body.type: SET_OPTIONS\n"
                "tx.operations[12].body.setOptionsOp.inflationDest._present: false\n"
                "tx.operations[12].body.setOptionsOp.clearFlags._present: false\n"
                "tx.operations[12].body.setOptionsOp.setFlags._present: false\n"
                "tx.operations[12].body.setOptionsOp.masterWeight._present: false\n"
                "tx.operations[12].body.setOptionsOp.lowThreshold._present: false\n"
                "tx.operations[12].body.setOptionsOp.medThreshold._present: false\n"
                "tx.operations[12].body.setOptionsOp.highThreshold._present: false\n"
                "tx.operations[12].body.setOptionsOp.homeDomain._present: false\n"
                "tx.operations[12].body.setOptionsOp.signer._present: true\n"
                "tx.operations[12].body.setOptionsOp.signer.key: XAQCCIRDEQSSMJZIFEVCWLBNFYXTAMJSGM2DKNRXHA4TUOZ4HU7D6IMI\n"
                "tx.operations[12].body.setOptionsOp.signer.weight: 10\n"
                "tx.operations[13].sourceAccount._present: false\n"
                "tx.operations[13].body.type: CHANGE_TRUST\n"
                "tx.operations[13].body.changeTrustOp.line.type: ASSET_TYPE_CREDIT_ALPHANUM4\n"
                "tx.operations[13].body.changeTrustOp.line: USD:GDFJHLAXAUMHA4OWPOB4P7YO72AQR2HMIUYFOXLXE2DZGM633K7HZDQP\n"
                "tx.operations[13].body.changeTrustOp.limit: 10000000000 (1000)\n"
                "tx.operations[14].sourceAccount._present: false\n"
                "tx.operations[14].body.type: CHANGE_TRUST\n"
                "tx.operations[14].body.changeTrustOp.line.type: ASSET_TYPE_CREDIT_ALPHANUM12\n"
                "tx.operations[14].body.changeTrustOp.line: LONGASSET012:GBXHUHG5FGYLPD6RHL2MKWMP572O6KUXCZXDZJXS4T57ZTMAKBN7DWXN\n"
                "tx.operations[14].body.changeTrustOp.limit: 9223372036854775807 (922337203685.4775807)\n"
                "tx.operations[15].sourceAccount._present: false\n"
                "tx.operations[15].body.type: CHANGE_TRUST\n"
                "tx.operations[15].body.changeTrustOp.line.type: ASSET_TYPE_POOL_SHARE\n"
                "tx.operations[15].body.changeTrustOp.line.liquidityPool.constantProduct.assetA: native\n"
                "tx.operations[15].body.changeTrustOp.line.liquidityPool.constantProduct.assetB: USD:GDFJHLAXAUMHA4OWPOB4P7YO72AQR2HMIUYFOXLXE2DZGM633K7HZDQP\n"
                "tx.operations[15].body.changeTrustOp.line.liquidityPool.constantProduct.fee: 30\n"
                "tx.operations[15].body.changeTrustOp.limit: 500000000 (50)\n"
                "tx.operations[16].sourceAccount._present: false\n"
                "tx.operations[16].body.type: ALLOW_TRUST\n"
                "tx.operations[16].body.allowTrustOp.trustor: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[16].body.allowTrustOp.asset: USD\n"
                "tx.operations[16].body.allowTrustOp.authorize: true\n"
                "tx.operations[17].sourceAccount._present: false\n"
                "tx.operations[17].body.type: ALLOW_TRUST\n"
                "tx.operations[17].body.allowTrustOp.trustor: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[17].body.allowTrustOp.asset: LONGASSET012\n"
                "tx.operations[17].body.allowTrustOp.authorize: false\n"
                "tx.operations[18].sourceAccount._present: false\n"
                "tx.operations[18].body.type: ACCOUNT_MERGE\n"
                "tx.operations[18].body.destination: GDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435DIAG\n"
                "tx.operations[19].sourceAccount._present: false\n"
                "tx.operations[19].body.type: INFLATION\n"
                "tx.operations[20].sourceAccount._present: false\n"
                "tx.operations[20].body.type: MANAGE_DATA\n"
                "tx.operations[20].body.manageDataOp.dataName: \"name\"\n"
                "tx.operations[20].body.manageDataOp.dataValue._present: true\n"
                "tx.operations[20].body.manageDataOp.dataValue: 76616c7565\n"
                "tx.operations[21].sourceAccount._present: false\n"
                "tx.operations[21].body.type: MANAGE_DATA\n"
                "tx.operations[21].body.manageDataOp.dataName: \"delete me\"\n"
                "tx.operations[21].body.manageDataOp.dataValue._present: false\n"
                "tx.operations[22].sourceAccount._present: false\n"
                "tx.operations[22].body.type: MANAGE_DATA\n"
                "tx.operations[22].body.manageDataOp.dataName: \"quote\\\" back\\\\slash\"\n"
                "tx.operations[22].body.manageDataOp.dataValue._present: true\n"
                "tx.operations[22].body.manageDataOp.dataValue: 00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000\n"
                "tx.operations[23].sourceAccount._present: false\n"
                "tx.operations[23].body.type: BUMP_SEQUENCE\n"
                "tx.operations[23].body.bumpSequenceOp.bumpTo: 9223372036854775807\n"
                "tx.operations[24].sourceAccount._present: false\n"
                "tx.operations[24].body.type: MANAGE_BUY_OFFER\n"
                "tx.operations[24].body.manageBuyOfferOp.selling: native\n"
                "tx.operations[24].body.manageBuyOfferOp.buying: USD:GDFJHLAXAUMHA4OWPOB4P7YO72AQR2HMIUYFOXLXE2DZGM633K7HZDQP\n"
                "tx.operations[24].body.manageBuyOfferOp.buyAmount: 70000000 (7)\n"
                "tx.operations[24].body.manageBuyOfferOp.price.n: 1\n"
                "tx.operations[24].body.manageBuyOfferOp.price.d: 3\n"
                "tx.operations[24].body.manageBuyOfferOp.offerID: 99\n"
                "tx.operations[25].sourceAccount._present: false\n"
                "tx.operations[25].body.type: PATH_PAYMENT_STRICT_SEND\n"
                "tx.operations[25].body.pathPaymentStrictSendOp.sendAsset: native\n"
                "tx.operations[25].body.pathPaymentStrictSendOp.sendAmount: 50000000 (5)\n"
                "tx.operations[25].body.pathPaymentStrictSendOp.destination: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[25].body.pathPaymentStrictSendOp.destAsset: USD:GDFJHLAXAUMHA4OWPOB4P7YO72AQR2HMIUYFOXLXE2DZGM633K7HZDQP\n"
                "tx.operations[25].body.pathPaymentStrictSendOp.destMin: 40000000 (4)\n"
                "tx.operations[25].body.pathPaymentStrictSendOp.path.len: 1\n"
                "tx.operations[25].body.pathPaymentStrictSendOp.path[0]: EUR:GBXHUHG5FGYLPD6RHL2MKWMP572O6KUXCZXDZJXS4T57ZTMAKBN7DWXN\n"
                "tx.operations[26].sourceAccount._present: false\n"
                "tx.operations[26].body.type: CREATE_CLAIMABLE_BALANCE\n"
                "tx.operations[26].body.createClaimableBalanceOp.asset: USD:GDFJHLAXAUMHA4OWPOB4P7YO72AQR2HMIUYFOXLXE2DZGM633K7HZDQP\n"
                "tx.operations[26].body.createClaimableBalanceOp.amount: 10000000 (1)\n"
                "tx.operations[26].body.createClaimableBalanceOp.claimants.len: 2\n"
                "tx.operations[26].body.createClaimableBalanceOp.claimants[0].type: CLAIMANT_TYPE_V0\n"
                "tx.operations[26].body.createClaimableBalanceOp.claimants[0].v0.destination: GDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435DIAG\n"
                "tx.operations[26].body.createClaimableBalanceOp.claimants[0].v0.predicate.type: CLAIM_PREDICATE_AND\n"
                "tx.operations[26].body.createClaimableBalanceOp.claimants[0].v0.predicate.andPredicates.len: 2\n"
                "tx.operations[26].body.createClaimableBalanceOp.claimants[0].v0.predicate.andPredicates[0].type: CLAIM_PREDICATE_BEFORE_ABSOLUTE_TIME\n"
                "tx.operations[26].body.createClaimableBalanceOp.claimants[0].v0.predicate.andPredicates[0].absBefore: 1700000000 (2023-11-14 22:13:20.000000+00:00 (UTC))\n"
                "tx.operations[26].body.createClaimableBalanceOp.claimants[0].v0.predicate.andPredicates[1].type: CLAIM_PREDICATE_BEFORE_RELATIVE_TIME\n"
                "tx.operations[26].body.createClaimableBalanceOp.claimants[0].v0.predicate.andPredicates[1].relBefore: 3600 (1970-01-01 01:00:00.000000+00:00 (UTC))\n"
                "tx.operations[26].body.createClaimableBalanceOp.claimants[1].type: CLAIMANT_TYPE_V0\n"
                "tx.operations[26].body.createClaimableBalanceOp.claimants[1].v0.destination: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.operations[26].body.createClaimableBalanceOp.claimants[1].v0.predicate.type: CLAIM_PREDICATE_OR\n"
                "tx.operations[26].body.createClaimableBalanceOp.claimants[1].v0.predicate.orPredicates.len: 2\n"
                "tx.operations[26].body.createClaimableBalanceOp.claimants[1].v0.predicate.orPredicates[0].type: CLAIM_PREDICATE_NOT\n"
                "tx.operations[26].body.createClaimableBalanceOp.claimants[1].v0.predicate.orPredicates[0].notPredicate.type: CLAIM_PREDICATE_BEFORE_RELATIVE_TIME\n"
                "tx.operations[26].body.createClaimableBalanceOp.claimants[1].v0.predicate.orPredicates[0].notPredicate.relBefore: 60 (1970-01-01 00:01:00.000000+00:00 (UTC))\n"
                "tx.operations[26].body.createClaimableBalanceOp.claimants[1].v0.predicate.orPredicates[1].type: CLAIM_PREDICATE_BEFORE_ABSOLUTE_TIME\n"
                "tx.operations[26].body.createClaimableBalanceOp.claimants[1].v0.predicate.orPredicates[1].absBefore: 0 (1970-01-01 00:00:00.000000+00:00 (UTC))\n"
                "tx.operations[27].sourceAccount._present: false\n"
                "tx.operations[27].body.type: CLAIM_CLAIMABLE_BALANCE\n"
                "tx.operations[27].body.claimClaimableBalanceOp.balanceID: 00000000da0d57da7d4850e7fc10d2a9d0ebc731f7afb40574c03395b17d49149b91f5be\n"
                "tx.operations[28].sourceAccount._present: false\n"
                "tx.operations[28].body.type: BEGIN_SPONSORING_FUTURE_RESERVES\n"
                "tx.operations[28].body.beginSponsoringFutureReservesOp.sponsoredID: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[29].sourceAccount._present: true\n"
                "tx.operations[29].sourceAccount: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[29].body.type: END_SPONSORING_FUTURE_RESERVES\n"
                "tx.operations[30].sourceAccount._present: false\n"
                "tx.operations[30].body.type: REVOKE_SPONSORSHIP\n"
                "tx.operations[30].body.revokeSponsorshipOp.type: REVOKE_SPONSORSHIP_LEDGER_ENTRY\n"
                "tx.operations[30].body.revokeSponsorshipOp.ledgerKey.type: ACCOUNT\n"
                "tx.operations[30].body.revokeSponsorshipOp.ledgerKey.account: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[31].sourceAccount._present: false\n"
                "tx.operations[31].body.type: REVOKE_SPONSORSHIP\n"
                "tx.operations[31].body.revokeSponsorshipOp.type: REVOKE_SPONSORSHIP_LEDGER_ENTRY\n"
                "tx.operations[31].body.revokeSponsorshipOp.ledgerKey.type: TRUSTLINE\n"
                "tx.operations[31].body.revokeSponsorshipOp.ledgerKey.trustLine.accountID: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[31].body.revokeSponsorshipOp.ledgerKey.trustLine.asset: USD:GDFJHLAXAUMHA4OWPOB4P7YO72AQR2HMIUYFOXLXE2DZGM633K7HZDQP\n"
                "tx.operations[32].sourceAccount._present: false\n"
                "tx.operations[32].body.type: REVOKE_SPONSORSHIP\n"
                "tx.operations[32].body.revokeSponsorshipOp.type: REVOKE_SPONSORSHIP_LEDGER_ENTRY\n"
                "tx.operations[32].body.revokeSponsorshipOp.ledgerKey.type: TRUSTLINE\n"
                "tx.operations[32].body.revokeSponsorshipOp.ledgerKey.trustLine.accountID: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[32].body.revokeSponsorshipOp.ledgerKey.trustLine.asset: LONGASSET012:GBXHUHG5FGYLPD6RHL2MKWMP572O6KUXCZXDZJXS4T57ZTMAKBN7DWXN\n"
                "tx.operations[33].sourceAccount._present: false\n"
                "tx.operations[33].body.type: REVOKE_SPONSORSHIP\n"
                "tx.operations[33].body.revokeSponsorshipOp.type: REVOKE_SPONSORSHIP_LEDGER_ENTRY\n"
                "tx.operations[33].body.revokeSponsorshipOp.ledgerKey.type: OFFER\n"
                "tx.operations[33].body.revokeSponsorshipOp.ledgerKey.offer.sellerID: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[33].body.revokeSponsorshipOp.ledgerKey.offer.offerID: 42\n"
                "tx.operations[34].sourceAccount._present: false\n"
                "tx.operations[34].body.type: REVOKE_SPONSORSHIP\n"
                "tx.operations[34].body.revokeSponsorshipOp.type: REVOKE_SPONSORSHIP_LEDGER_ENTRY\n"
                "tx.operations[34].body.revokeSponsorshipOp.ledgerKey.type: DATA\n"
                "tx.operations[34].body.revokeSponsorshipOp.ledgerKey.data.accountID: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[34].body.revokeSponsorshipOp.ledgerKey.data.dataName: data-name\n"
                "tx.operations[35].sourceAccount._present: false\n"
                "tx.operations[35].body.type: REVOKE_SPONSORSHIP\n"
                "tx.operations[35].body.revokeSponsorshipOp.type: REVOKE_SPONSORSHIP_LEDGER_ENTRY\n"
                "tx.operations[35].body.revokeSponsorshipOp.ledgerKey.type: CLAIMABLE_BALANCE\n"
                "tx.operations[35].body.revokeSponsorshipOp.ledgerKey.claimableBalance.balanceID: 00000000da0d57da7d4850e7fc10d2a9d0ebc731f7afb40574c03395b17d49149b91f5be\n"
                "tx.operations[36].sourceAccount._present: false\n"
                "tx.operations[36].body.type: REVOKE_SPONSORSHIP\n"
                "tx.operations[36].body.revokeSponsorshipOp.type: REVOKE_SPONSORSHIP_LEDGER_ENTRY\n"
                "tx.operations[36].body.revokeSponsorshipOp.ledgerKey.type: LIQUIDITY_POOL\n"
                "tx.operations[36].body.revokeSponsorshipOp.ledgerKey.liquidityPool.liquidityPoolID: 2c3dc9d8d5f72c61f4468a88a65722b3edf113edbcae1f6f45010b42f52d1f11\n"
                "tx.operations[37].sourceAccount._present: false\n"
                "tx.operations[37].body.type: REVOKE_SPONSORSHIP\n"
                "tx.operations[37].body.revokeSponsorshipOp.type: REVOKE_SPONSORSHIP_SIGNER\n"
                "tx.operations[37].body.revokeSponsorshipOp.signer.accountID: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[37].body.revokeSponsorshipOp.signer.signerKey.type: SIGNER_KEY_TYPE_ED25519\n"
                "tx.operations[37].body.revokeSponsorshipOp.signer.signerKey.ed25519: GDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435DIAG\n"
                "tx.operations[38].sourceAccount._present: false\n"
                "tx.operations[38].body.type: REVOKE_SPONSORSHIP\n"
                "tx.operations[38].body.revokeSponsorshipOp.type: REVOKE_SPONSORSHIP_SIGNER\n"
                "tx.operations[38].body.revokeSponsorshipOp.signer.accountID: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[38].body.revokeSponsorshipOp.signer.signerKey.type: SIGNER_KEY_TYPE_PRE_AUTH_TX\n"
                "tx.operations[38].body.revokeSponsorshipOp.signer.signerKey.preAuthTx: TAAACAQDAQCQMBYIBEFAWDANBYHRAEISCMKBKFQXDAMRUGY4DUPB6ULG\n"
                "tx.operations[39].sourceAccount._present: false\n"
                "tx.operations[39].body.type: REVOKE_SPONSORSHIP\n"
                "tx.operations[39].body.revokeSponsorshipOp.type: REVOKE_SPONSORSHIP_SIGNER\n"
                "tx.operations[39].body.revokeSponsorshipOp.signer.accountID: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[39].body.revokeSponsorshipOp.signer.signerKey.type: SIGNER_KEY_TYPE_HASH_X\n"
                "tx.operations[39].body.revokeSponsorshipOp.signer.signerKey.hashX: XAQCCIRDEQSSMJZIFEVCWLBNFYXTAMJSGM2DKNRXHA4TUOZ4HU7D6IMI\n"
                "tx.operations[40].sourceAccount._present: false\n"
                "tx.operations[40].body.type: CLAWBACK\n"
                "tx.operations[40].body.clawbackOp.asset: USD:GDFJHLAXAUMHA4OWPOB4P7YO72AQR2HMIUYFOXLXE2DZGM633K7HZDQP\n"
                "tx.operations[40].body.clawbackOp.from: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[40].body.clawbackOp.amount: 20000000 (2)\n"
                "tx.operations[41].sourceAccount._present: false\n"
                "tx.operations[41].body.type: CLAWBACK_CLAIMABLE_BALANCE\n"
                "tx.operations[41].body.clawbackClaimableBalanceOp.balanceID: 00000000da0d57da7d4850e7fc10d2a9d0ebc731f7afb40574c03395b17d49149b91f5be\n"
                "tx.operations[42].sourceAccount._present: false\n"
                "tx.operations[42].body.type: SET_TRUST_LINE_FLAGS\n"
                "tx.operations[42].body.setTrustLineFlagsOp.trustor: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[42].body.setTrustLineFlagsOp.asset: USD:GDFJHLAXAUMHA4OWPOB4P7YO72AQR2HMIUYFOXLXE2DZGM633K7HZDQP\n"
                "tx.operations[42].body.setTrustLineFlagsOp.clearFlags: 1\n"
                "tx.operations[42].body.setTrustLineFlagsOp.setFlags: 6\n"
                "tx.operations[43].sourceAccount._present: false\n"
                "tx.operations[43].body.type: SET_TRUST_LINE_FLAGS\n"
                "tx.operations[43].body.setTrustLineFlagsOp.trustor: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[43].body.setTrustLineFlagsOp.asset: LONGASSET012:GBXHUHG5FGYLPD6RHL2MKWMP572O6KUXCZXDZJXS4T57ZTMAKBN7DWXN\n"
                "tx.operations[43].body.setTrustLineFlagsOp.clearFlags: 0\n"
                "tx.operations[43].body.setTrustLineFlagsOp.setFlags: 0\n"
                "tx.operations[44].sourceAccount._present: false\n"
                "tx.operations[44].body.type: LIQUIDITY_POOL_DEPOSIT\n"
                "tx.operations[44].body.liquidityPoolDepositOp.liquidityPoolID: 2c3dc9d8d5f72c61f4468a88a65722b3edf113edbcae1f6f45010b42f52d1f11\n"
                "tx.operations[44].body.liquidityPoolDepositOp.maxAmountA: 100000000 (10)\n"
                "tx.operations[44].body.liquidityPoolDepositOp.maxAmountB: 200000000 (20)\n"
                "tx.operations[44].body.liquidityPoolDepositOp.minPrice.n: 1\n"
                "tx.operations[44].body.liquidityPoolDepositOp.minPrice.d: 2\n"
                "tx.operations[44].body.liquidityPoolDepositOp.maxPrice.n: 2\n"
                "tx.operations[44].body.liquidityPoolDepositOp.maxPrice.d: 1\n"
                "tx.operations[45].sourceAccount._present: false\n"
                "tx.operations[45].body.type: LIQUIDITY_POOL_WITHDRAW\n"
                "tx.operations[45].body.liquidityPoolWithdrawOp.liquidityPoolID: 2c3dc9d8d5f72c61f4468a88a65722b3edf113edbcae1f6f45010b42f52d1f11\n"
                "tx.operations[45].body.liquidityPoolWithdrawOp.amount: 50000000 (5)\n"
                "tx.operations[45].body.liquidityPoolWithdrawOp.minAmountA: 10000000 (1)\n"
                "tx.operations[45].body.liquidityPoolWithdrawOp.minAmountB: 20000000 (2)\n"
                "tx.ext.v: 0\n"
                "signatures.len: 0",
                "AAAAAgAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXAAAEfgAAAEfcfsEywAAAAEAAAAAZVPxAAAAAABrSdIA"
                "AAAAAAAAAC4AAAAAAAAAAAAAAACBOXcOqH0XX1ajVGbDTH7My42KkbTuN6Jd9g9bj8mzlAAAAAAHc1lAAAAAAAAAAAEAAAAA"
                "gTl3Dqh9F19Wo1Rmw0x+zMuNipG07jeiXfYPW4/Js5QAAAAAAAAAAAZCLEAAAAABAAAAAO1JKMYo0cLG6ukDOJBZlWEpWSc6"
                "XGP5NjbBRhSshzfRAAAAAQAAAACBOXcOqH0XX1ajVGbDTH7My42KkbTuN6Jd9g9bj8mzlAAAAAFVU0QAAAAAAMqTrBcFGHBx"
                "1nuDx/8O/oEI6OxFMFdddyaHkzPb2r58AAAAAAAAAAEAAAAAAAAAAQAAAACBOXcOqH0XX1ajVGbDTH7My42KkbTuN6Jd9g9b"
                "j8mzlAAAAAJMT05HQVNTRVQwMTIAAAAAbnoc3Smwt4/ROvTFWY/v9O8qlxZuPKby5Pv8zYBQW/F//////////wAAAAAAAAAC"
                "AAAAAAAAAAAC+vCAAAAAAIE5dw6ofRdfVqNUZsNMfszLjYqRtO43ol32D1uPybOUAAAAAVVTRAAAAAAAypOsFwUYcHHWe4PH"
                "/w7+gQjo7EUwV113JoeTM9vavnwAAAAAAmJaAAAAAAAAAAAAAAAAAgAAAAFVU0QAAAAAAMqTrBcFGHBx1nuDx/8O/oEI6OxF"
                "MFdddyaHkzPb2r58AAAAAAL68IAAAAAAgTl3Dqh9F19Wo1Rmw0x+zMuNipG07jeiXfYPW4/Js5QAAAACTE9OR0FTU0VUMDEy"
                "AAAAAG56HN0psLeP0Tr0xVmP7/TvKpcWbjym8uT7/M2AUFvxAAAAAAJiWgAAAAACAAAAAAAAAAFFVVIAAAAAAG56HN0psLeP"
                "0Tr0xVmP7/TvKpcWbjym8uT7/M2AUFvxAAAAAAAAAAMAAAAAAAAAAVVTRAAAAAAAypOsFwUYcHHWe4PH/w7+gQjo7EUwV113"
                "JoeTM9vavnwAAAAAO5rKAAAAAAcAAAAJAAAAAAAAAAAAAAAAAAAAAwAAAAFVU0QAAAAAAMqTrBcFGHBx1nuDx/8O/oEI6OxF"
                "MFdddyaHkzPb2r58AAAAAkxPTkdBU1NFVDAxMgAAAABuehzdKbC3j9E69MVZj+/07yqXFm48pvLk+/zNgFBb8QAAAAAAAAAA"
                "AAAAAX////8AAAAAAAAwOQAAAAAAAAAEAAAAAVVTRAAAAAAAypOsFwUYcHHWe4PH/w7+gQjo7EUwV113JoeTM9vavnwAAAAA"
                "AAAAAAHJw4B/////AAAAAQAAAAAAAAAFAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAUAAAAB"
                "AAAAAIE5dw6ofRdfVqNUZsNMfszLjYqRtO43ol32D1uPybOUAAAAAQAAAAIAAAABAAAACQAAAAEAAAD/AAAAAQAAAAAAAAAB"
                "AAAAAgAAAAEAAAADAAAAAQAAAAtleGFtcGxlLmNvbQAAAAABAAAAAO1JKMYo0cLG6ukDOJBZlWEpWSc6XGP5NjbBRhSshzfR"
                "AAAAAQAAAAAAAAAFAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABAAAAAQABAgMEBQYHCAkKCwwNDg8QERIT"
                "FBUWFxgZGhscHR4fAAAAAAAAAAAAAAAFAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABAAAAAiAhIiMkJSYn"
                "KCkqKywtLi8wMTIzNDU2Nzg5Ojs8PT4/AAAACgAAAAAAAAAGAAAAAVVTRAAAAAAAypOsFwUYcHHWe4PH/w7+gQjo7EUwV113"
                "JoeTM9vavnwAAAACVAvkAAAAAAAAAAAGAAAAAkxPTkdBU1NFVDAxMgAAAABuehzdKbC3j9E69MVZj+/07yqXFm48pvLk+/zN"
                "gFBb8X//////////AAAAAAAAAAYAAAADAAAAAAAAAAAAAAABVVNEAAAAAADKk6wXBRhwcdZ7g8f/Dv6BCOjsRTBXXXcmh5Mz"
                "29q+fAAAAB4AAAAAHc1lAAAAAAAAAAAHAAAAAIE5dw6ofRdfVqNUZsNMfszLjYqRtO43ol32D1uPybOUAAAAAVVTRAAAAAAB"
                "AAAAAAAAAAcAAAAAgTl3Dqh9F19Wo1Rmw0x+zMuNipG07jeiXfYPW4/Js5QAAAACTE9OR0FTU0VUMDEyAAAAAAAAAAAAAAAI"
                "AAAAAO1JKMYo0cLG6ukDOJBZlWEpWSc6XGP5NjbBRhSshzfRAAAAAAAAAAkAAAAAAAAACgAAAARuYW1lAAAAAQAAAAV2YWx1"
                "ZQAAAAAAAAAAAAAKAAAACWRlbGV0ZSBtZQAAAAAAAAAAAAAAAAAACgAAABFxdW90ZSIgYmFja1xzbGFzaAAAAAAAAAEAAABA"
                "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAL"
                "f/////////8AAAAAAAAADAAAAAAAAAABVVNEAAAAAADKk6wXBRhwcdZ7g8f/Dv6BCOjsRTBXXXcmh5Mz29q+fAAAAAAELB2A"
                "AAAAAQAAAAMAAAAAAAAAYwAAAAAAAAANAAAAAAAAAAAC+vCAAAAAAIE5dw6ofRdfVqNUZsNMfszLjYqRtO43ol32D1uPybOU"
                "AAAAAVVTRAAAAAAAypOsFwUYcHHWe4PH/w7+gQjo7EUwV113JoeTM9vavnwAAAAAAmJaAAAAAAEAAAABRVVSAAAAAABuehzd"
                "KbC3j9E69MVZj+/07yqXFm48pvLk+/zNgFBb8QAAAAAAAAAOAAAAAVVTRAAAAAAAypOsFwUYcHHWe4PH/w7+gQjo7EUwV113"
                "JoeTM9vavnwAAAAAAJiWgAAAAAIAAAAAAAAAAO1JKMYo0cLG6ukDOJBZlWEpWSc6XGP5NjbBRhSshzfRAAAAAQAAAAIAAAAE"
                "AAAAAGVT8QAAAAAFAAAAAAAADhAAAAAAAAAAAIqI4910CfGV/VLbLTy6XXLKZwm/HZQSG/N0iAG0D29cAAAAAgAAAAIAAAAD"
                "AAAAAQAAAAUAAAAAAAAAPAAAAAQAAAAAAAAAAAAAAAAAAAAPAAAAANoNV9p9SFDn/BDSqdDrxzH3r7QFdMAzlbF9SRSbkfW+"
                "AAAAAAAAABAAAAAAgTl3Dqh9F19Wo1Rmw0x+zMuNipG07jeiXfYPW4/Js5QAAAABAAAAAIE5dw6ofRdfVqNUZsNMfszLjYqR"
                "tO43ol32D1uPybOUAAAAEQAAAAAAAAASAAAAAAAAAAAAAAAAgTl3Dqh9F19Wo1Rmw0x+zMuNipG07jeiXfYPW4/Js5QAAAAA"
                "AAAAEgAAAAAAAAABAAAAAIE5dw6ofRdfVqNUZsNMfszLjYqRtO43ol32D1uPybOUAAAAAVVTRAAAAAAAypOsFwUYcHHWe4PH"
                "/w7+gQjo7EUwV113JoeTM9vavnwAAAAAAAAAEgAAAAAAAAABAAAAAIE5dw6ofRdfVqNUZsNMfszLjYqRtO43ol32D1uPybOU"
                "AAAAAkxPTkdBU1NFVDAxMgAAAABuehzdKbC3j9E69MVZj+/07yqXFm48pvLk+/zNgFBb8QAAAAAAAAASAAAAAAAAAAIAAAAA"
                "gTl3Dqh9F19Wo1Rmw0x+zMuNipG07jeiXfYPW4/Js5QAAAAAAAAAKgAAAAAAAAASAAAAAAAAAAMAAAAAgTl3Dqh9F19Wo1Rm"
                "w0x+zMuNipG07jeiXfYPW4/Js5QAAAAJZGF0YS1uYW1lAAAAAAAAAAAAABIAAAAAAAAABAAAAADaDVfafUhQ5/wQ0qnQ68cx"
                "96+0BXTAM5WxfUkUm5H1vgAAAAAAAAASAAAAAAAAAAUsPcnY1fcsYfRGioimVyKz7fET7byuH29FAQtC9S0fEQAAAAAAAAAS"
                "AAAAAQAAAACBOXcOqH0XX1ajVGbDTH7My42KkbTuN6Jd9g9bj8mzlAAAAADtSSjGKNHCxurpAziQWZVhKVknOlxj+TY2wUYU"
                "rIc30QAAAAAAAAASAAAAAQAAAACBOXcOqH0XX1ajVGbDTH7My42KkbTuN6Jd9g9bj8mzlAAAAAEAAQIDBAUGBwgJCgsMDQ4P"
                "EBESExQVFhcYGRobHB0eHwAAAAAAAAASAAAAAQAAAACBOXcOqH0XX1ajVGbDTH7My42KkbTuN6Jd9g9bj8mzlAAAAAIgISIj"
                "JCUmJygpKissLS4vMDEyMzQ1Njc4OTo7PD0+PwAAAAAAAAATAAAAAVVTRAAAAAAAypOsFwUYcHHWe4PH/w7+gQjo7EUwV113"
                "JoeTM9vavnwAAAAAgTl3Dqh9F19Wo1Rmw0x+zMuNipG07jeiXfYPW4/Js5QAAAAAATEtAAAAAAAAAAAUAAAAANoNV9p9SFDn"
                "/BDSqdDrxzH3r7QFdMAzlbF9SRSbkfW+AAAAAAAAABUAAAAAgTl3Dqh9F19Wo1Rmw0x+zMuNipG07jeiXfYPW4/Js5QAAAAB"
                "VVNEAAAAAADKk6wXBRhwcdZ7g8f/Dv6BCOjsRTBXXXcmh5Mz29q+fAAAAAEAAAAGAAAAAAAAABUAAAAAgTl3Dqh9F19Wo1Rm"
                "w0x+zMuNipG07jeiXfYPW4/Js5QAAAACTE9OR0FTU0VUMDEyAAAAAG56HN0psLeP0Tr0xVmP7/TvKpcWbjym8uT7/M2AUFvx"
                "AAAAAAAAAAAAAAAAAAAAFiw9ydjV9yxh9EaKiKZXIrPt8RPtvK4fb0UBC0L1LR8RAAAAAAX14QAAAAAAC+vCAAAAAAEAAAAC"
                "AAAAAgAAAAEAAAAAAAAAFyw9ydjV9yxh9EaKiKZXIrPt8RPtvK4fb0UBC0L1LR8RAAAAAAL68IAAAAAAAJiWgAAAAAABMS0A"
                "AAAAAAAAAAA=",
                "1a79e540bb53817f5afda9f5666bf80260c3a2c302262c47efc572607735b88d" },
            { "muxed",
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: MCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYABAAAAAAAAAAFPM6 (accountID: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR, accountMuxedID: 9007199254740993)\n"
                "tx.fee: 600\n"
                "tx.seqNum: 1234567890123\n"
                "tx.cond.type: PRECOND_NONE\n"
                "tx.memo.type: MEMO_NONE\n"
                "tx.operations.len: 6\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: PAYMENT\n"
                "tx.operations[0].body.paymentOp.destination: MCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZIAAAAAAAAAAAAHZ7W (accountID: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U, accountMuxedID: 1)\n"
                "tx.operations[0].body.paymentOp.asset: native\n"
                "tx.operations[0].body.paymentOp.amount: 10000000 (1)\n"
                "tx.operations[1].sourceAccount._present: true\n"
                "tx.operations[1].sourceAccount: MDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435D7777777777777RCC (accountID: GDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435DIAG, accountMuxedID: 18446744073709551615)\n"
                "tx.operations[1].body.type: PAYMENT\n"
                "tx.operations[1].body.paymentOp.destination: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[1].body.paymentOp.asset: native\n"
                "tx.operations[1].body.paymentOp.amount: 10000000 (1)\n"
                "tx.operations[2].sourceAccount._present: false\n"
                "tx.operations[2].body.type: PATH_PAYMENT_STRICT_RECEIVE\n"
                "tx.operations[2].body.pathPaymentStrictReceiveOp.sendAsset: native\n"
                "tx.operations[2].body.pathPaymentStrictReceiveOp.sendMax: 50000000 (5)\n"
                "tx.operations[2].body.pathPaymentStrictReceiveOp.destination: MDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435D7777777777777RCC (accountID: GDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435DIAG, accountMuxedID: 18446744073709551615)\n"
                "tx.operations[2].body.pathPaymentStrictReceiveOp.destAsset: USD:GDFJHLAXAUMHA4OWPOB4P7YO72AQR2HMIUYFOXLXE2DZGM633K7HZDQP\n"
                "tx.operations[2].body.pathPaymentStrictReceiveOp.destAmount: 40000000 (4)\n"
                "tx.operations[2].body.pathPaymentStrictReceiveOp.path.len: 0\n"
                "tx.operations[3].sourceAccount._present: true\n"
                "tx.operations[3].sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.operations[3].body.type: PATH_PAYMENT_STRICT_SEND\n"
                "tx.operations[3].body.pathPaymentStrictSendOp.sendAsset: native\n"
                "tx.operations[3].body.pathPaymentStrictSendOp.sendAmount: 50000000 (5)\n"
                "tx.operations[3].body.pathPaymentStrictSendOp.destination: MCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZIAAAAAAAAAAAAHZ7W (accountID: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U, accountMuxedID: 1)\n"
                "tx.operations[3].body.pathPaymentStrictSendOp.destAsset: USD:GDFJHLAXAUMHA4OWPOB4P7YO72AQR2HMIUYFOXLXE2DZGM633K7HZDQP\n"
                "tx.operations[3].body.pathPaymentStrictSendOp.destMin: 40000000 (4)\n"
                "tx.operations[3].body.pathPaymentStrictSendOp.path.len: 0\n"
                "tx.operations[4].sourceAccount._present: false\n"
                "tx.operations[4].body.type: ACCOUNT_MERGE\n"
                "tx.operations[4].body.destination: MDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435D7777777777777RCC (accountID: GDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435DIAG, accountMuxedID: 18446744073709551615)\n"
                "tx.operations[5].sourceAccount._present: true\n"
                "tx.operations[5].sourceAccount: MCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYABAAAAAAAAAAFPM6 (accountID: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR, accountMuxedID: 9007199254740993)\n"
                "tx.operations[5].body.type: CLAWBACK\n"
                "tx.operations[5].body.clawbackOp.asset: USD:GDFJHLAXAUMHA4OWPOB4P7YO72AQR2HMIUYFOXLXE2DZGM633K7HZDQP\n"
                "tx.operations[5].body.clawbackOp.from: MCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZIAAAAAAAAAAAAHZ7W (accountID: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U, accountMuxedID: 1)\n"
                "tx.operations[5].body.clawbackOp.amount: 20000000 (2)\n"
                "tx.ext.v: 0\n"
                "signatures.len: 0",
                "AAAAAgAAAQAAIAAAAAAAAYqI4910CfGV/VLbLTy6XXLKZwm/HZQSG/N0iAG0D29cAAACWAAAAR9x+wTLAAAAAAAAAAAAAAAG"
                "AAAAAAAAAAEAAAEAAAAAAAAAAAGBOXcOqH0XX1ajVGbDTH7My42KkbTuN6Jd9g9bj8mzlAAAAAAAAAAAAJiWgAAAAAEAAAEA"
                "///////////tSSjGKNHCxurpAziQWZVhKVknOlxj+TY2wUYUrIc30QAAAAEAAAAAgTl3Dqh9F19Wo1Rmw0x+zMuNipG07jei"
                "XfYPW4/Js5QAAAAAAAAAAACYloAAAAAAAAAAAgAAAAAAAAAAAvrwgAAAAQD//////////+1JKMYo0cLG6ukDOJBZlWEpWSc6"
                "XGP5NjbBRhSshzfRAAAAAVVTRAAAAAAAypOsFwUYcHHWe4PH/w7+gQjo7EUwV113JoeTM9vavnwAAAAAAmJaAAAAAAAAAAAB"
                "AAAAAIqI4910CfGV/VLbLTy6XXLKZwm/HZQSG/N0iAG0D29cAAAADQAAAAAAAAAAAvrwgAAAAQAAAAAAAAAAAYE5dw6ofRdf"
                "VqNUZsNMfszLjYqRtO43ol32D1uPybOUAAAAAVVTRAAAAAAAypOsFwUYcHHWe4PH/w7+gQjo7EUwV113JoeTM9vavnwAAAAA"
                "AmJaAAAAAAAAAAAAAAAACAAAAQD//////////+1JKMYo0cLG6ukDOJBZlWEpWSc6XGP5NjbBRhSshzfRAAAAAQAAAQAAIAAA"
                "AAAAAYqI4910CfGV/VLbLTy6XXLKZwm/HZQSG/N0iAG0D29cAAAAEwAAAAFVU0QAAAAAAMqTrBcFGHBx1nuDx/8O/oEI6OxF"
                "MFdddyaHkzPb2r58AAABAAAAAAAAAAABgTl3Dqh9F19Wo1Rmw0x+zMuNipG07jeiXfYPW4/Js5QAAAAAATEtAAAAAAAAAAAA",
                "11f90d6c536c5422e7eb77983722e0252b3d383a79c874297207eb6c479edbf7" },
            { "claimable_unconditional",
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 100\n"
                "tx.seqNum: 1234567890123\n"
                "tx.cond.type: PRECOND_NONE\n"
                "tx.memo.type: MEMO_NONE\n"
                "tx.operations.len: 1\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: CREATE_CLAIMABLE_BALANCE\n"
                "tx.operations[0].body.createClaimableBalanceOp.asset: native\n"
                "tx.operations[0].body.createClaimableBalanceOp.amount: 10000000 (1)\n"
                "tx.operations[0].body.createClaimableBalanceOp.claimants.len: 2\n"
                "tx.operations[0].body.createClaimableBalanceOp.claimants[0].type: CLAIMANT_TYPE_V0\n"
                "tx.operations[0].body.createClaimableBalanceOp.claimants[0].v0.destination: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[0].body.createClaimableBalanceOp.claimants[0].v0.predicate.type: CLAIM_PREDICATE_UNCONDITIONAL\n"
                "tx.operations[0].body.createClaimableBalanceOp.claimants[1].type: CLAIMANT_TYPE_V0\n"
                "tx.operations[0].body.createClaimableBalanceOp.claimants[1].v0.destination: GDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435DIAG\n"
                "tx.operations[0].body.createClaimableBalanceOp.claimants[1].v0.predicate.type: CLAIM_PREDICATE_NOT\n"
                "tx.operations[0].body.createClaimableBalanceOp.claimants[1].v0.predicate.notPredicate.type: CLAIM_PREDICATE_UNCONDITIONAL\n"
                "tx.ext.v: 0\n"
                "signatures.len: 0",
                "AAAAAgAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXAAAAGQAAAEfcfsEywAAAAAAAAAAAAAAAQAAAAAAAAAO"
                "AAAAAAAAAAAAmJaAAAAAAgAAAAAAAAAAgTl3Dqh9F19Wo1Rmw0x+zMuNipG07jeiXfYPW4/Js5QAAAAAAAAAAAAAAADtSSjG"
                "KNHCxurpAziQWZVhKVknOlxj+TY2wUYUrIc30QAAAAMAAAABAAAAAAAAAAAAAAAA",
                "77488f448b0f7e4bb8d534dbf28dd730a8a631100d6bfb4f79a275a93b2f6157" },
            { "manage_data_empty_value",
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 100\n"
                "tx.seqNum: 1234567890123\n"
                "tx.cond.type: PRECOND_NONE\n"
                "tx.memo.type: MEMO_NONE\n"
                "tx.operations.len: 1\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: MANAGE_DATA\n"
                "tx.operations[0].body.manageDataOp.dataName: \"k\"\n"
                "tx.operations[0].body.manageDataOp.dataValue._present: true\n"
                "tx.operations[0].body.manageDataOp.dataValue: \n"
                "tx.ext.v: 0\n"
                "signatures.len: 0",
                "AAAAAgAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXAAAAGQAAAEfcfsEywAAAAAAAAAAAAAAAQAAAAAAAAAK"
                "AAAAAWsAAAAAAAABAAAAAAAAAAAAAAAA",
                "08968439eaedf3156644673afc343acb39568a51cd0946b6d9c74ed25ef61d42" },
            { "memo_text",
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 100\n"
                "tx.seqNum: 1234567890123\n"
                "tx.cond.type: PRECOND_NONE\n"
                "tx.memo.type: MEMO_TEXT\n"
                "tx.memo.text: \"hello world\"\n"
                "tx.operations.len: 1\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: BUMP_SEQUENCE\n"
                "tx.operations[0].body.bumpSequenceOp.bumpTo: 1\n"
                "tx.ext.v: 0\n"
                "signatures.len: 0",
                "AAAAAgAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXAAAAGQAAAEfcfsEywAAAAAAAAABAAAAC2hlbGxvIHdv"
                "cmxkAAAAAAEAAAAAAAAACwAAAAAAAAABAAAAAAAAAAA=",
                "eae42b5860bbd1a2d5b54434f0ec7b14b4afbf6c5595c59b4618d3ce63c6999e" },
            { "memo_text_escapes",
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 100\n"
                "tx.seqNum: 1234567890123\n"
                "tx.cond.type: PRECOND_NONE\n"
                "tx.memo.type: MEMO_TEXT\n"
                "tx.memo.text: \"say \\\"hi\\\" \\\\ bye\\nnext\"\n"
                "tx.operations.len: 1\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: BUMP_SEQUENCE\n"
                "tx.operations[0].body.bumpSequenceOp.bumpTo: 1\n"
                "tx.ext.v: 0\n"
                "signatures.len: 0",
                "AAAAAgAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXAAAAGQAAAEfcfsEywAAAAAAAAABAAAAE3NheSAiaGki"
                "IFwgYnllCm5leHQAAAAAAQAAAAAAAAALAAAAAAAAAAEAAAAAAAAAAA==",
                "844f600784b4f9e9b4f5c37d38cd8e861434a5ae5b62468e4335ad2b817d5094" },
            { "memo_text_unicode",
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 100\n"
                "tx.seqNum: 1234567890123\n"
                "tx.cond.type: PRECOND_NONE\n"
                "tx.memo.type: MEMO_TEXT\n"
                "tx.memo.text: \"caf\\u00e9 \\u65e5\\u672c\"\n"
                "tx.operations.len: 1\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: BUMP_SEQUENCE\n"
                "tx.operations[0].body.bumpSequenceOp.bumpTo: 1\n"
                "tx.ext.v: 0\n"
                "signatures.len: 0",
                "AAAAAgAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXAAAAGQAAAEfcfsEywAAAAAAAAABAAAADGNhZsOpIOaX"
                "peacrAAAAAEAAAAAAAAACwAAAAAAAAABAAAAAAAAAAA=",
                "ff2c3ddafcaeea21aafe30e43b9ce0c86a54aa2408362231fb34d6304eb69d23" },
            { "memo_text_bytes",
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 100\n"
                "tx.seqNum: 1234567890123\n"
                "tx.cond.type: PRECOND_NONE\n"
                "tx.memo.type: MEMO_TEXT\n"
                "tx.memo.text: fffe0041\n"
                "tx.operations.len: 1\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: BUMP_SEQUENCE\n"
                "tx.operations[0].body.bumpSequenceOp.bumpTo: 1\n"
                "tx.ext.v: 0\n"
                "signatures.len: 0",
                "AAAAAgAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXAAAAGQAAAEfcfsEywAAAAAAAAABAAAABP/+AEEAAAAB"
                "AAAAAAAAAAsAAAAAAAAAAQAAAAAAAAAA",
                "ebac94522652ffd0a5585cacfb3c963735dc4418ad097ffbe8d07504b261649a" },
            { "memo_text_empty",
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 100\n"
                "tx.seqNum: 1234567890123\n"
                "tx.cond.type: PRECOND_NONE\n"
                "tx.memo.type: MEMO_TEXT\n"
                "tx.memo.text: \"\"\n"
                "tx.operations.len: 1\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: BUMP_SEQUENCE\n"
                "tx.operations[0].body.bumpSequenceOp.bumpTo: 1\n"
                "tx.ext.v: 0\n"
                "signatures.len: 0",
                "AAAAAgAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXAAAAGQAAAEfcfsEywAAAAAAAAABAAAAAAAAAAEAAAAA"
                "AAAACwAAAAAAAAABAAAAAAAAAAA=",
                "721dc75232238e6cd8f708e82d2750de3033495bee0be440584410ef35f3eb02" },
            { "memo_id",
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 100\n"
                "tx.seqNum: 1234567890123\n"
                "tx.cond.type: PRECOND_NONE\n"
                "tx.memo.type: MEMO_ID\n"
                "tx.memo.id: 18446744073709551615\n"
                "tx.operations.len: 1\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: BUMP_SEQUENCE\n"
                "tx.operations[0].body.bumpSequenceOp.bumpTo: 1\n"
                "tx.ext.v: 0\n"
                "signatures.len: 0",
                "AAAAAgAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXAAAAGQAAAEfcfsEywAAAAAAAAAC//////////8AAAAB"
                "AAAAAAAAAAsAAAAAAAAAAQAAAAAAAAAA",
                "027c4d4c6c52012c85ed7796e782af4e2292889eb201c8c990f2379971f2b2a6" },
            { "memo_hash",
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 100\n"
                "tx.seqNum: 1234567890123\n"
                "tx.cond.type: PRECOND_NONE\n"
                "tx.memo.type: MEMO_HASH\n"
                "tx.memo.hash: 000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f\n"
                "tx.operations.len: 1\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: BUMP_SEQUENCE\n"
                "tx.operations[0].body.bumpSequenceOp.bumpTo: 1\n"
                "tx.ext.v: 0\n"
                "signatures.len: 0",
                "AAAAAgAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXAAAAGQAAAEfcfsEywAAAAAAAAADAAECAwQFBgcICQoL"
                "DA0ODxAREhMUFRYXGBkaGxwdHh8AAAABAAAAAAAAAAsAAAAAAAAAAQAAAAAAAAAA",
                "3bf19800d46b2508cf9b8d4e80dd4e147df431e56e8c6bed855f367b8a17c6b2" },
            { "memo_return",
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 100\n"
                "tx.seqNum: 1234567890123\n"
                "tx.cond.type: PRECOND_NONE\n"
                "tx.memo.type: MEMO_RETURN\n"
                "tx.memo.retHash: 6465666768696a6b6c6d6e6f707172737475767778797a7b7c7d7e7f80818283\n"
                "tx.operations.len: 1\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: BUMP_SEQUENCE\n"
                "tx.operations[0].body.bumpSequenceOp.bumpTo: 1\n"
                "tx.ext.v: 0\n"
                "signatures.len: 0",
                "AAAAAgAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXAAAAGQAAAEfcfsEywAAAAAAAAAEZGVmZ2hpamtsbW5v"
                "cHFyc3R1dnd4eXp7fH1+f4CBgoMAAAABAAAAAAAAAAsAAAAAAAAAAQAAAAAAAAAA",
                "8c1711c7746fb7a1144a53127785d5ef9510971d4077820e65823622031b6168" },
            { "cond_none",
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 100\n"
                "tx.seqNum: 1234567890123\n"
                "tx.cond.type: PRECOND_NONE\n"
                "tx.memo.type: MEMO_NONE\n"
                "tx.operations.len: 1\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: BUMP_SEQUENCE\n"
                "tx.operations[0].body.bumpSequenceOp.bumpTo: 1\n"
                "tx.ext.v: 0\n"
                "signatures.len: 0",
                "AAAAAgAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXAAAAGQAAAEfcfsEywAAAAAAAAAAAAAAAQAAAAAAAAAL"
                "AAAAAAAAAAEAAAAAAAAAAA==",
                "c14b9f0c3f9a9f8b02019ac474a94e60d5c36bbfe9db5af822d9e559f7cedfed" },
            { "cond_time_zero",
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 100\n"
                "tx.seqNum: 1234567890123\n"
                "tx.cond.type: PRECOND_TIME\n"
                "tx.cond.timeBounds.minTime: 0 (1970-01-01 00:00:00.000000+00:00 (UTC))\n"
                "tx.cond.timeBounds.maxTime: 0 (1970-01-01 00:00:00.000000+00:00 (UTC))\n"
                "tx.memo.type: MEMO_NONE\n"
                "tx.operations.len: 1\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: BUMP_SEQUENCE\n"
                "tx.operations[0].body.bumpSequenceOp.bumpTo: 1\n"
                "tx.ext.v: 0\n"
                "signatures.len: 0",
                "AAAAAgAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXAAAAGQAAAEfcfsEywAAAAEAAAAAAAAAAAAAAAAAAAAA"
                "AAAAAAAAAAEAAAAAAAAACwAAAAAAAAABAAAAAAAAAAA=",
                "3c7173516de363774692272db8c03bcfe21d1b44adcfe91e3be9dd9e133b3591" },
            { "cond_time_max",
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 100\n"
                "tx.seqNum: 1234567890123\n"
                "tx.cond.type: PRECOND_TIME\n"
                "tx.cond.timeBounds.minTime: 32503679999 (2999-12-31 23:59:59.000000+00:00 (UTC))\n"
                "tx.cond.timeBounds.maxTime: 32503680000 (3000-01-01 00:00:00.000000+00:00 (UTC))\n"
                "tx.memo.type: MEMO_NONE\n"
                "tx.operations.len: 1\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: BUMP_SEQUENCE\n"
                "tx.operations[0].body.bumpSequenceOp.bumpTo: 1\n"
                "tx.ext.v: 0\n"
                "signatures.len: 0",
                "AAAAAgAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXAAAAGQAAAEfcfsEywAAAAEAAAAHkV7L/wAAAAeRXswA"
                "AAAAAAAAAAEAAAAAAAAACwAAAAAAAAABAAAAAAAAAAA=",
                "12c44cd43d41b17f5cebee66e3201b641dbb68ed7cc4295177d46d5b1d5e433a" },
            { "cond_v2_ledger",
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 100\n"
                "tx.seqNum: 1234567890123\n"
                "tx.cond.type: PRECOND_V2\n"
                "tx.cond.timeBounds._present: false\n"
                "tx.cond.ledgerBounds._present: true\n"
                "tx.cond.ledgerBounds.minLedger: 10\n"
                "tx.cond.ledgerBounds.maxLedger: 0\n"
                "tx.cond.minSeqNum._present: false\n"
                "tx.cond.minSeqAge: 0\n"
                "tx.cond.minSeqLedgerGap: 0\n"
                "tx.cond.extraSigners.len: 0\n"
                "tx.memo.type: MEMO_NONE\n"
                "tx.operations.len: 1\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: BUMP_SEQUENCE\n"
                "tx.operations[0].body.bumpSequenceOp.bumpTo: 1\n"
                "tx.ext.v: 0\n"
                "signatures.len: 0",
                "AAAAAgAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXAAAAGQAAAEfcfsEywAAAAIAAAAAAAAAAQAAAAoAAAAA"
                "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAQAAAAAAAAALAAAAAAAAAAEAAAAAAAAAAA==",
                "f7ea0318e540a2955e1efbc9e12dd4bbc1f4cbcb4552d9982abd889d6cab01ae" },
            { "cond_v2_minseq",
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 100\n"
                "tx.seqNum: 1234567890123\n"
                "tx.cond.type: PRECOND_V2\n"
                "tx.cond.timeBounds._present: false\n"
                "tx.cond.ledgerBounds._present: false\n"
                "tx.cond.minSeqNum._present: true\n"
                "tx.cond.minSeqNum: -5\n"
                "tx.cond.minSeqAge: 0\n"
                "tx.cond.minSeqLedgerGap: 0\n"
                "tx.cond.extraSigners.len: 0\n"
                "tx.memo.type: MEMO_NONE\n"
                "tx.operations.len: 1\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: BUMP_SEQUENCE\n"
                "tx.operations[0].body.bumpSequenceOp.bumpTo: 1\n"
                "tx.ext.v: 0\n"
                "signatures.len: 0",
                "AAAAAgAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXAAAAGQAAAEfcfsEywAAAAIAAAAAAAAAAAAAAAH/////"
                "////+wAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAQAAAAAAAAALAAAAAAAAAAEAAAAAAAAAAA==",
                "e6b260c86a09475d28ad8d5a8be9f8be41c387b9cc16ef35546d16666bd61547" },
            { "cond_v2_age_gap",
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 100\n"
                "tx.seqNum: 1234567890123\n"
                "tx.cond.type: PRECOND_V2\n"
                "tx.cond.timeBounds._present: false\n"
                "tx.cond.ledgerBounds._present: false\n"
                "tx.cond.minSeqNum._present: false\n"
                "tx.cond.minSeqAge: 18446744073709551615\n"
                "tx.cond.minSeqLedgerGap: 4294967295\n"
                "tx.cond.extraSigners.len: 0\n"
                "tx.memo.type: MEMO_NONE\n"
                "tx.operations.len: 1\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: BUMP_SEQUENCE\n"
                "tx.operations[0].body.bumpSequenceOp.bumpTo: 1\n"
                "tx.ext.v: 0\n"
                "signatures.len: 0",
                "AAAAAgAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXAAAAGQAAAEfcfsEywAAAAIAAAAAAAAAAAAAAAD/////"
                "//////////8AAAAAAAAAAAAAAAEAAAAAAAAACwAAAAAAAAABAAAAAAAAAAA=",
                "a77b2c962192f9a6f2982f65c41dfbb53a110cec9e666a5d9eb059b700cebbf8" },
            { "cond_v2_zero",
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 100\n"
                "tx.seqNum: 1234567890123\n"
                "tx.cond.type: PRECOND_V2\n"
                "tx.cond.timeBounds._present: false\n"
                "tx.cond.ledgerBounds._present: false\n"
                "tx.cond.minSeqNum._present: false\n"
                "tx.cond.minSeqAge: 0\n"
                "tx.cond.minSeqLedgerGap: 0\n"
                "tx.cond.extraSigners.len: 0\n"
                "tx.memo.type: MEMO_NONE\n"
                "tx.operations.len: 1\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: BUMP_SEQUENCE\n"
                "tx.operations[0].body.bumpSequenceOp.bumpTo: 1\n"
                "tx.ext.v: 0\n"
                "signatures.len: 0",
                "AAAAAgAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXAAAAGQAAAEfcfsEywAAAAIAAAAAAAAAAAAAAAAAAAAA"
                "AAAAAAAAAAAAAAAAAAAAAAAAAAEAAAAAAAAACwAAAAAAAAABAAAAAAAAAAA=",
                "bc2bba126c1891f2fe43fe145ffaf3e7a9e82c32f9f6c70e446886bb098f6628" },
            { "cond_v2_signers",
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 100\n"
                "tx.seqNum: 1234567890123\n"
                "tx.cond.type: PRECOND_V2\n"
                "tx.cond.timeBounds._present: true\n"
                "tx.cond.timeBounds.minTime: 1 (1970-01-01 00:00:01.000000+00:00 (UTC))\n"
                "tx.cond.timeBounds.maxTime: 2 (1970-01-01 00:00:02.000000+00:00 (UTC))\n"
                "tx.cond.ledgerBounds._present: false\n"
                "tx.cond.minSeqNum._present: false\n"
                "tx.cond.minSeqAge: 0\n"
                "tx.cond.minSeqLedgerGap: 0\n"
                "tx.cond.extraSigners.len: 2\n"
                "tx.cond.extraSigners[0].type: SIGNER_KEY_TYPE_ED25519\n"
                "tx.cond.extraSigners[0].ed25519: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.cond.extraSigners[1].type: SIGNER_KEY_TYPE_ED25519_SIGNED_PAYLOAD\n"
                "tx.cond.extraSigners[1].ed25519SignedPayload: PADQOBYHA4DQOBYHA4DQOBYHA4DQOBYHA4DQOBYHA4DQOBYHA4DQOAAAAACWQZLMNRXQAAAAQOXQ\n"
                "tx.memo.type: MEMO_NONE\n"
                "tx.operations.len: 1\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: BUMP_SEQUENCE\n"
                "tx.operations[0].body.bumpSequenceOp.bumpTo: 1\n"
                "tx.ext.v: 0\n"
                "signatures.len: 0",
                "AAAAAgAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXAAAAGQAAAEfcfsEywAAAAIAAAABAAAAAAAAAAEAAAAA"
                "AAAAAgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAgAAAACBOXcOqH0XX1ajVGbDTH7My42KkbTuN6Jd9g9bj8mzlAAAAAMHBwcH"
                "BwcHBwcHBwcHBwcHBwcHBwcHBwcHBwcHBwcHBwAAAAVoZWxsbwAAAAAAAAAAAAABAAAAAAAAAAsAAAAAAAAAAQAAAAAAAAAA",
                "8c8b50304b849377737cd4d919baf69ff437e2c2356b962b5d8be7b568cd17b5" },
            { "cond_v2_hashes",
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 100\n"
                "tx.seqNum: 1234567890123\n"
                "tx.cond.type: PRECOND_V2\n"
                "tx.cond.timeBounds._present: false\n"
                "tx.cond.ledgerBounds._present: false\n"
                "tx.cond.minSeqNum._present: false\n"
                "tx.cond.minSeqAge: 0\n"
                "tx.cond.minSeqLedgerGap: 0\n"
                "tx.cond.extraSigners.len: 2\n"
                "tx.cond.extraSigners[0].type: SIGNER_KEY_TYPE_PRE_AUTH_TX\n"
                "tx.cond.extraSigners[0].preAuthTx: TAAACAQDAQCQMBYIBEFAWDANBYHRAEISCMKBKFQXDAMRUGY4DUPB6ULG\n"
                "tx.cond.extraSigners[1].type: SIGNER_KEY_TYPE_HASH_X\n"
                "tx.cond.extraSigners[1].hashX: XAQCCIRDEQSSMJZIFEVCWLBNFYXTAMJSGM2DKNRXHA4TUOZ4HU7D6IMI\n"
                "tx.memo.type: MEMO_NONE\n"
                "tx.operations.len: 1\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: BUMP_SEQUENCE\n"
                "tx.operations[0].body.bumpSequenceOp.bumpTo: 1\n"
                "tx.ext.v: 0\n"
                "signatures.len: 0",
                "AAAAAgAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXAAAAGQAAAEfcfsEywAAAAIAAAAAAAAAAAAAAAAAAAAA"
                "AAAAAAAAAAAAAAACAAAAAQABAgMEBQYHCAkKCwwNDg8QERITFBUWFxgZGhscHR4fAAAAAiAhIiMkJSYnKCkqKywtLi8wMTIz"
                "NDU2Nzg5Ojs8PT4/AAAAAAAAAAEAAAAAAAAACwAAAAAAAAABAAAAAAAAAAA=",
                "2b0654ca8f6baa6bddd0caaf38ffd2bdc925812e20331f36b8be7a68d85df159" },
            { "cond_v2_all",
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 100\n"
                "tx.seqNum: 1234567890123\n"
                "tx.cond.type: PRECOND_V2\n"
                "tx.cond.timeBounds._present: true\n"
                "tx.cond.timeBounds.minTime: 5 (1970-01-01 00:00:05.000000+00:00 (UTC))\n"
                "tx.cond.timeBounds.maxTime: 6 (1970-01-01 00:00:06.000000+00:00 (UTC))\n"
                "tx.cond.ledgerBounds._present: true\n"
                "tx.cond.ledgerBounds.minLedger: 7\n"
                "tx.cond.ledgerBounds.maxLedger: 8\n"
                "tx.cond.minSeqNum._present: true\n"
                "tx.cond.minSeqNum: 9\n"
                "tx.cond.minSeqAge: 10\n"
                "tx.cond.minSeqLedgerGap: 11\n"
                "tx.cond.extraSigners.len: 1\n"
                "tx.cond.extraSigners[0].type: SIGNER_KEY_TYPE_ED25519\n"
                "tx.cond.extraSigners[0].ed25519: GDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435DIAG\n"
                "tx.memo.type: MEMO_NONE\n"
                "tx.operations.len: 1\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: BUMP_SEQUENCE\n"
                "tx.operations[0].body.bumpSequenceOp.bumpTo: 1\n"
                "tx.ext.v: 0\n"
                "signatures.len: 0",
                "AAAAAgAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXAAAAGQAAAEfcfsEywAAAAIAAAABAAAAAAAAAAUAAAAA"
                "AAAABgAAAAEAAAAHAAAACAAAAAEAAAAAAAAACQAAAAAAAAAKAAAACwAAAAEAAAAA7UkoxijRwsbq6QM4kFmVYSlZJzpcY/k2"
                "NsFGFKyHN9EAAAAAAAAAAQAAAAAAAAALAAAAAAAAAAEAAAAAAAAAAA==",
                "720619dc081e1569df184978024102d1134912f2a7efdbe6f974591866ec1f82" },
            { "seq_fee_extremes",
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 4294967295\n"
                "tx.seqNum: -9223372036854775807\n"
                "tx.cond.type: PRECOND_NONE\n"
                "tx.memo.type: MEMO_NONE\n"
                "tx.operations.len: 3\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: BUMP_SEQUENCE\n"
                "tx.operations[0].body.bumpSequenceOp.bumpTo: 1\n"
                "tx.operations[1].sourceAccount._present: false\n"
                "tx.operations[1].body.type: BUMP_SEQUENCE\n"
                "tx.operations[1].body.bumpSequenceOp.bumpTo: 2\n"
                "tx.operations[2].sourceAccount._present: false\n"
                "tx.operations[2].body.type: BUMP_SEQUENCE\n"
                "tx.operations[2].body.bumpSequenceOp.bumpTo: 3\n"
                "tx.ext.v: 0\n"
                "signatures.len: 0",
                "AAAAAgAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXP////+AAAAAAAAAAQAAAAAAAAAAAAAAAwAAAAAAAAAL"
                "AAAAAAAAAAEAAAAAAAAACwAAAAAAAAACAAAAAAAAAAsAAAAAAAAAAwAAAAAAAAAA",
                "6c7380111f2dc36c1faaa305f1dc974b7956aa84cc4074275ab3f08e185b05a7" },
        };
        return cases;
    }

    static const char* pyFeeBumpDocument()
    {
        return
                "type: ENVELOPE_TYPE_TX_FEE_BUMP\n"
                "feeBump.tx.feeSource: MCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZIAAAAAAAAAAAAHZ7W (accountID: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U, accountMuxedID: 1)\n"
                "feeBump.tx.fee: 200\n"
                "feeBump.tx.innerTx.type: ENVELOPE_TYPE_TX\n"
                "feeBump.tx.innerTx.tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "feeBump.tx.innerTx.tx.fee: 100\n"
                "feeBump.tx.innerTx.tx.seqNum: 1\n"
                "feeBump.tx.innerTx.tx.cond.type: PRECOND_NONE\n"
                "feeBump.tx.innerTx.tx.memo.type: MEMO_NONE\n"
                "feeBump.tx.innerTx.tx.operations.len: 1\n"
                "feeBump.tx.innerTx.tx.operations[0].sourceAccount._present: false\n"
                "feeBump.tx.innerTx.tx.operations[0].body.type: BUMP_SEQUENCE\n"
                "feeBump.tx.innerTx.tx.operations[0].body.bumpSequenceOp.bumpTo: 1\n"
                "feeBump.tx.innerTx.tx.ext.v: 0\n"
                "feeBump.tx.innerTx.signatures.len: 0\n"
                "feeBump.tx.ext.v: 0\n"
                "feeBump.signatures.len: 0";
    }

    static const char* pyV0Document()
    {
        return
                "type: ENVELOPE_TYPE_TX_V0\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 100\n"
                "tx.seqNum: 1\n"
                "tx.cond.type: PRECOND_NONE\n"
                "tx.memo.type: MEMO_NONE\n"
                "tx.operations.len: 1\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: BUMP_SEQUENCE\n"
                "tx.operations[0].body.bumpSequenceOp.bumpTo: 1\n"
                "tx.ext.v: 0\n"
                "signatures.len: 0";
    }

    /**
     * Written with py-stellar-sdk 14.1.0 (to_txrep of a TransactionEnvelope
     * or FeeBumpTransactionEnvelope, testnet), with the keys and accounts of
     * pyCorpus(): V0 transactions (unsigned; with time bounds, muxed accounts
     * in its operations and two signatures; with the extremes of fee and
     * sequence and a hash-x signature) and fee bumps (plain; with an M...
     * fee source and inner source and both lists signed; around a V2 inner
     * transaction; with the extremes of both fees and 20 signatures in each
     * list; around a V0 inner transaction, which py wraps as the V1 envelope
     * it is; and one py decoded from its XDR, whose amounts py comments in
     * their Decimal form, "1E-7"). py reads each back to its own envelope.
     */
    static const std::vector<PyCorpusCase>& pyEnvelopeCorpus()
    {
        static const std::vector<PyCorpusCase> cases = {
            { "v0_plain",
                "type: ENVELOPE_TYPE_TX_V0\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 100\n"
                "tx.seqNum: 1234567890123\n"
                "tx.cond.type: PRECOND_NONE\n"
                "tx.memo.type: MEMO_NONE\n"
                "tx.operations.len: 1\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: BUMP_SEQUENCE\n"
                "tx.operations[0].body.bumpSequenceOp.bumpTo: 1\n"
                "tx.ext.v: 0\n"
                "signatures.len: 0",
                "AAAAAIqI4910CfGV/VLbLTy6XXLKZwm/HZQSG/N0iAG0D29cAAAAZAAAAR9x+wTLAAAAAAAAAAAAAAABAAAAAAAAAAsAAAAA"
                "AAAAAQAAAAAAAAAA",
                "c14b9f0c3f9a9f8b02019ac474a94e60d5c36bbfe9db5af822d9e559f7cedfed" },
            { "v0_time_signed",
                "type: ENVELOPE_TYPE_TX_V0\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 300\n"
                "tx.seqNum: 1234567890124\n"
                "tx.cond.type: PRECOND_TIME\n"
                "tx.cond.timeBounds.minTime: 1700000000 (2023-11-14 22:13:20.000000+00:00 (UTC))\n"
                "tx.cond.timeBounds.maxTime: 1800000000 (2027-01-15 08:00:00.000000+00:00 (UTC))\n"
                "tx.memo.type: MEMO_HASH\n"
                "tx.memo.hash: 000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f\n"
                "tx.operations.len: 3\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: PAYMENT\n"
                "tx.operations[0].body.paymentOp.destination: MCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZIAAAAAAAAAAAAHZ7W (accountID: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U, accountMuxedID: 1)\n"
                "tx.operations[0].body.paymentOp.asset: native\n"
                "tx.operations[0].body.paymentOp.amount: 105000000 (10.5)\n"
                "tx.operations[1].sourceAccount._present: true\n"
                "tx.operations[1].sourceAccount: MDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435D7777777777777RCC (accountID: GDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435DIAG, accountMuxedID: 18446744073709551615)\n"
                "tx.operations[1].body.type: PAYMENT\n"
                "tx.operations[1].body.paymentOp.destination: GDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435DIAG\n"
                "tx.operations[1].body.paymentOp.asset: USD:GDFJHLAXAUMHA4OWPOB4P7YO72AQR2HMIUYFOXLXE2DZGM633K7HZDQP\n"
                "tx.operations[1].body.paymentOp.amount: 1 (0.0000001)\n"
                "tx.operations[2].sourceAccount._present: false\n"
                "tx.operations[2].body.type: MANAGE_DATA\n"
                "tx.operations[2].body.manageDataOp.dataName: \"k\"\n"
                "tx.operations[2].body.manageDataOp.dataValue._present: true\n"
                "tx.operations[2].body.manageDataOp.dataValue: 76\n"
                "tx.ext.v: 0\n"
                "signatures.len: 2\n"
                "signatures[0].hint: b40f6f5c\n"
                "signatures[0].signature: 789da1318b5ba215033cec9afc296418222167067b8de7ba78929a019a3b2468a453aa032d595ace89605f71783eecc6ab76eebfd79417092df7cf7fac58d108\n"
                "signatures[1].hint: 8fc9b394\n"
                "signatures[1].signature: f27a7b0c0704fb53715833a189f1893721cc97c54cea221d70437949794d33de5899e4dd031b91c710dc6e57460831485cf0ee8968b7c9218cba5a974af49f0c",
                "AAAAAIqI4910CfGV/VLbLTy6XXLKZwm/HZQSG/N0iAG0D29cAAABLAAAAR9x+wTMAAAAAQAAAABlU/EAAAAAAGtJ0gAAAAAD"
                "AAECAwQFBgcICQoLDA0ODxAREhMUFRYXGBkaGxwdHh8AAAADAAAAAAAAAAEAAAEAAAAAAAAAAAGBOXcOqH0XX1ajVGbDTH7M"
                "y42KkbTuN6Jd9g9bj8mzlAAAAAAAAAAABkIsQAAAAAEAAAEA///////////tSSjGKNHCxurpAziQWZVhKVknOlxj+TY2wUYU"
                "rIc30QAAAAEAAAAA7UkoxijRwsbq6QM4kFmVYSlZJzpcY/k2NsFGFKyHN9EAAAABVVNEAAAAAADKk6wXBRhwcdZ7g8f/Dv6B"
                "COjsRTBXXXcmh5Mz29q+fAAAAAAAAAABAAAAAAAAAAoAAAABawAAAAAAAAEAAAABdgAAAAAAAAAAAAACtA9vXAAAAEB4naEx"
                "i1uiFQM87Jr8KWQYIiFnBnuN57p4kpoBmjskaKRTqgMtWVrOiWBfcXg+7Mardu6/15QXCS33z3+sWNEIj8mzlAAAAEDyensM"
                "BwT7U3FYM6GJ8Yk3IcyXxUzqIh1wQ3lJeU0z3liZ5N0DG5HHENxuV0YIMUhc8O6JaLfJIYy6WpdK9J8M",
                "b947b21b2e56b52e2798d458d5b6ac92d19e168976b5757efef15e694ecada30" },
            { "v0_extremes",
                "type: ENVELOPE_TYPE_TX_V0\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 4294967295\n"
                "tx.seqNum: -9223372036854775807\n"
                "tx.cond.type: PRECOND_TIME\n"
                "tx.cond.timeBounds.minTime: 0 (1970-01-01 00:00:00.000000+00:00 (UTC))\n"
                "tx.cond.timeBounds.maxTime: 0 (1970-01-01 00:00:00.000000+00:00 (UTC))\n"
                "tx.memo.type: MEMO_TEXT\n"
                "tx.memo.text: \"v0 memo\"\n"
                "tx.operations.len: 1\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: CREATE_ACCOUNT\n"
                "tx.operations[0].body.createAccountOp.destination: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[0].body.createAccountOp.startingBalance: 125000000 (12.5)\n"
                "tx.ext.v: 0\n"
                "signatures.len: 1\n"
                "signatures[0].hint: a9dbf55f\n"
                "signatures[0].signature: 707265696d616765",
                "AAAAAIqI4910CfGV/VLbLTy6XXLKZwm/HZQSG/N0iAG0D29c/////4AAAAAAAAABAAAAAQAAAAAAAAAAAAAAAAAAAAAAAAAB"
                "AAAAB3YwIG1lbW8AAAAAAQAAAAAAAAAAAAAAAIE5dw6ofRdfVqNUZsNMfszLjYqRtO43ol32D1uPybOUAAAAAAdzWUAAAAAA"
                "AAAAAanb9V8AAAAIcHJlaW1hZ2U=",
                "1f1e80e8e6911d767b6a5fca1f8a28154f32f043b2f99bf57dfec00d03898b57" },
            { "fee_bump_plain",
                "type: ENVELOPE_TYPE_TX_FEE_BUMP\n"
                "feeBump.tx.feeSource: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "feeBump.tx.fee: 200\n"
                "feeBump.tx.innerTx.type: ENVELOPE_TYPE_TX\n"
                "feeBump.tx.innerTx.tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "feeBump.tx.innerTx.tx.fee: 100\n"
                "feeBump.tx.innerTx.tx.seqNum: 1234567890123\n"
                "feeBump.tx.innerTx.tx.cond.type: PRECOND_NONE\n"
                "feeBump.tx.innerTx.tx.memo.type: MEMO_NONE\n"
                "feeBump.tx.innerTx.tx.operations.len: 1\n"
                "feeBump.tx.innerTx.tx.operations[0].sourceAccount._present: false\n"
                "feeBump.tx.innerTx.tx.operations[0].body.type: BUMP_SEQUENCE\n"
                "feeBump.tx.innerTx.tx.operations[0].body.bumpSequenceOp.bumpTo: 1\n"
                "feeBump.tx.innerTx.tx.ext.v: 0\n"
                "feeBump.tx.innerTx.signatures.len: 0\n"
                "feeBump.tx.ext.v: 0\n"
                "feeBump.signatures.len: 0",
                "AAAABQAAAACBOXcOqH0XX1ajVGbDTH7My42KkbTuN6Jd9g9bj8mzlAAAAAAAAADIAAAAAgAAAACKiOPddAnxlf1S2y08ul1y"
                "ymcJvx2UEhvzdIgBtA9vXAAAAGQAAAEfcfsEywAAAAAAAAAAAAAAAQAAAAAAAAALAAAAAAAAAAEAAAAAAAAAAAAAAAAAAAAA",
                "4d29fdf26ae2ae9135b9de8941bae9c24dfca4991eaf6d293221af28b7b24d4a" },
            { "fee_bump_muxed_signed",
                "type: ENVELOPE_TYPE_TX_FEE_BUMP\n"
                "feeBump.tx.feeSource: MCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZIAAAAAAAAAAAAHZ7W (accountID: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U, accountMuxedID: 1)\n"
                "feeBump.tx.fee: 600\n"
                "feeBump.tx.innerTx.type: ENVELOPE_TYPE_TX\n"
                "feeBump.tx.innerTx.tx.sourceAccount: MCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYABAAAAAAAAAAFPM6 (accountID: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR, accountMuxedID: 9007199254740993)\n"
                "feeBump.tx.innerTx.tx.fee: 200\n"
                "feeBump.tx.innerTx.tx.seqNum: 1234567890125\n"
                "feeBump.tx.innerTx.tx.cond.type: PRECOND_TIME\n"
                "feeBump.tx.innerTx.tx.cond.timeBounds.minTime: 1700000000 (2023-11-14 22:13:20.000000+00:00 (UTC))\n"
                "feeBump.tx.innerTx.tx.cond.timeBounds.maxTime: 1800000000 (2027-01-15 08:00:00.000000+00:00 (UTC))\n"
                "feeBump.tx.innerTx.tx.memo.type: MEMO_TEXT\n"
                "feeBump.tx.innerTx.tx.memo.text: \"fee bump \\\"inner\\\"\"\n"
                "feeBump.tx.innerTx.tx.operations.len: 2\n"
                "feeBump.tx.innerTx.tx.operations[0].sourceAccount._present: false\n"
                "feeBump.tx.innerTx.tx.operations[0].body.type: PAYMENT\n"
                "feeBump.tx.innerTx.tx.operations[0].body.paymentOp.destination: MDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435D7777777777777RCC (accountID: GDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435DIAG, accountMuxedID: 18446744073709551615)\n"
                "feeBump.tx.innerTx.tx.operations[0].body.paymentOp.asset: native\n"
                "feeBump.tx.innerTx.tx.operations[0].body.paymentOp.amount: 105000000 (10.5)\n"
                "feeBump.tx.innerTx.tx.operations[1].sourceAccount._present: true\n"
                "feeBump.tx.innerTx.tx.operations[1].sourceAccount: MCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYABAAAAAAAAAAFPM6 (accountID: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR, accountMuxedID: 9007199254740993)\n"
                "feeBump.tx.innerTx.tx.operations[1].body.type: ACCOUNT_MERGE\n"
                "feeBump.tx.innerTx.tx.operations[1].body.destination: MCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZIAAAAAAAAAAAAHZ7W (accountID: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U, accountMuxedID: 1)\n"
                "feeBump.tx.innerTx.tx.ext.v: 0\n"
                "feeBump.tx.innerTx.signatures.len: 1\n"
                "feeBump.tx.innerTx.signatures[0].hint: b40f6f5c\n"
                "feeBump.tx.innerTx.signatures[0].signature: 2cb0625ba4e0abb898cd2b2785b4dc0a82d2bda62bcf14260df332738c7c09ec9087677511475acc338c730edd14569c7a8119263eb8e5bac3b659ff7685250c\n"
                "feeBump.tx.ext.v: 0\n"
                "feeBump.signatures.len: 2\n"
                "feeBump.signatures[0].hint: 8fc9b394\n"
                "feeBump.signatures[0].signature: 34726c5d38dc0c94d17a0283a984b811d4dac0b44081848210f916853db4c1106a58ec3af10875eb25eb71a3ef3218d01704ec545e9baed4bb4cda907bc85f0e\n"
                "feeBump.signatures[1].hint: 9adefc23\n"
                "feeBump.signatures[1].signature: 6665652062756d7020707265696d616765",
                "AAAABQAAAQAAAAAAAAAAAYE5dw6ofRdfVqNUZsNMfszLjYqRtO43ol32D1uPybOUAAAAAAAAAlgAAAACAAABAAAgAAAAAAAB"
                "iojj3XQJ8ZX9UtstPLpdcspnCb8dlBIb83SIAbQPb1wAAADIAAABH3H7BM0AAAABAAAAAGVT8QAAAAAAa0nSAAAAAAEAAAAQ"
                "ZmVlIGJ1bXAgImlubmVyIgAAAAIAAAAAAAAAAQAAAQD//////////+1JKMYo0cLG6ukDOJBZlWEpWSc6XGP5NjbBRhSshzfR"
                "AAAAAAAAAAAGQixAAAAAAQAAAQAAIAAAAAAAAYqI4910CfGV/VLbLTy6XXLKZwm/HZQSG/N0iAG0D29cAAAACAAAAQAAAAAA"
                "AAAAAYE5dw6ofRdfVqNUZsNMfszLjYqRtO43ol32D1uPybOUAAAAAAAAAAG0D29cAAAAQCywYluk4Ku4mM0rJ4W03AqC0r2m"
                "K88UJg3zMnOMfAnskIdndRFHWswzjHMO3RRWnHqBGSY+uOW6w7ZZ/3aFJQwAAAAAAAAAAo/Js5QAAABANHJsXTjcDJTRegKD"
                "qYS4EdTawLRAgYSCEPkWhT20wRBqWOw68Qh16yXrcaPvMhjQFwTsVF6brtS7TNqQe8hfDpre/CMAAAARZmVlIGJ1bXAgcHJl"
                "aW1hZ2UAAAA=",
                "4a89adef4ead663791c5c86b0b91cce0a2dc36710f1aeb2e9eaaf31b5b567017" },
            { "fee_bump_v2_inner",
                "type: ENVELOPE_TYPE_TX_FEE_BUMP\n"
                "feeBump.tx.feeSource: GDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435DIAG\n"
                "feeBump.tx.fee: 1000\n"
                "feeBump.tx.innerTx.type: ENVELOPE_TYPE_TX\n"
                "feeBump.tx.innerTx.tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "feeBump.tx.innerTx.tx.fee: 100\n"
                "feeBump.tx.innerTx.tx.seqNum: 1234567890126\n"
                "feeBump.tx.innerTx.tx.cond.type: PRECOND_V2\n"
                "feeBump.tx.innerTx.tx.cond.timeBounds._present: false\n"
                "feeBump.tx.innerTx.tx.cond.ledgerBounds._present: true\n"
                "feeBump.tx.innerTx.tx.cond.ledgerBounds.minLedger: 7\n"
                "feeBump.tx.innerTx.tx.cond.ledgerBounds.maxLedger: 8\n"
                "feeBump.tx.innerTx.tx.cond.minSeqNum._present: true\n"
                "feeBump.tx.innerTx.tx.cond.minSeqNum: 9\n"
                "feeBump.tx.innerTx.tx.cond.minSeqAge: 10\n"
                "feeBump.tx.innerTx.tx.cond.minSeqLedgerGap: 11\n"
                "feeBump.tx.innerTx.tx.cond.extraSigners.len: 1\n"
                "feeBump.tx.innerTx.tx.cond.extraSigners[0].type: SIGNER_KEY_TYPE_ED25519\n"
                "feeBump.tx.innerTx.tx.cond.extraSigners[0].ed25519: GDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435DIAG\n"
                "feeBump.tx.innerTx.tx.memo.type: MEMO_ID\n"
                "feeBump.tx.innerTx.tx.memo.id: 18446744073709551615\n"
                "feeBump.tx.innerTx.tx.operations.len: 1\n"
                "feeBump.tx.innerTx.tx.operations[0].sourceAccount._present: false\n"
                "feeBump.tx.innerTx.tx.operations[0].body.type: PAYMENT\n"
                "feeBump.tx.innerTx.tx.operations[0].body.paymentOp.destination: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "feeBump.tx.innerTx.tx.operations[0].body.paymentOp.asset: USD:GDFJHLAXAUMHA4OWPOB4P7YO72AQR2HMIUYFOXLXE2DZGM633K7HZDQP\n"
                "feeBump.tx.innerTx.tx.operations[0].body.paymentOp.amount: 10000000 (1)\n"
                "feeBump.tx.innerTx.tx.ext.v: 0\n"
                "feeBump.tx.innerTx.signatures.len: 2\n"
                "feeBump.tx.innerTx.signatures[0].hint: b40f6f5c\n"
                "feeBump.tx.innerTx.signatures[0].signature: 1f1a90ce8c719701e9ed8f316b5c37e7505d4805e1495a1f03237b196e54e7475aee021a64908133212ad4720552c54736ebcb27e20782cea16496d8bcfe7e06\n"
                "feeBump.tx.innerTx.signatures[1].hint: ac8737d1\n"
                "feeBump.tx.innerTx.signatures[1].signature: 25eb97c850677c4f9ec1c1ee5f42e693f62274473500f003504cd784f428c85fc2abb13d0ebdc37ab0044bea5a2c9cab16166aeceff1b3080d4d58d7142f080b\n"
                "feeBump.tx.ext.v: 0\n"
                "feeBump.signatures.len: 1\n"
                "feeBump.signatures[0].hint: ac8737d1\n"
                "feeBump.signatures[0].signature: 9db01b3db63edb5b966c940af75f740e78c2f740d7d9ffd2ba45d787c1d1c6dceb5359ace5b4321e155044042030f42dafc6d2c2812b4199ff1e8277721f5f03",
                "AAAABQAAAADtSSjGKNHCxurpAziQWZVhKVknOlxj+TY2wUYUrIc30QAAAAAAAAPoAAAAAgAAAACKiOPddAnxlf1S2y08ul1y"
                "ymcJvx2UEhvzdIgBtA9vXAAAAGQAAAEfcfsEzgAAAAIAAAAAAAAAAQAAAAcAAAAIAAAAAQAAAAAAAAAJAAAAAAAAAAoAAAAL"
                "AAAAAQAAAADtSSjGKNHCxurpAziQWZVhKVknOlxj+TY2wUYUrIc30QAAAAL//////////wAAAAEAAAAAAAAAAQAAAACBOXcO"
                "qH0XX1ajVGbDTH7My42KkbTuN6Jd9g9bj8mzlAAAAAFVU0QAAAAAAMqTrBcFGHBx1nuDx/8O/oEI6OxFMFdddyaHkzPb2r58"
                "AAAAAACYloAAAAAAAAAAArQPb1wAAABAHxqQzoxxlwHp7Y8xa1w351BdSAXhSVofAyN7GW5U50da7gIaZJCBMyEq1HIFUsVH"
                "NuvLJ+IHgs6hZJbYvP5+BqyHN9EAAABAJeuXyFBnfE+ewcHuX0Lmk/YidEc1APADUEzXhPQoyF/Cq7E9Dr3DerAES+paLJyr"
                "FhZq7O/xswgNTVjXFC8ICwAAAAAAAAABrIc30QAAAECdsBs9tj7bW5ZslAr3X3QOeML3QNfZ/9K6RdeHwdHG3OtTWazltDIe"
                "FVBEBCAw9C2vxtLCgStBmf8egndyH18D",
                "12b150ddba9724326709aa104caa14e350e998def77ac2540c3d89e4d589c246" },
            { "fee_bump_extremes",
                "type: ENVELOPE_TYPE_TX_FEE_BUMP\n"
                "feeBump.tx.feeSource: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "feeBump.tx.fee: 9223372036854775807\n"
                "feeBump.tx.innerTx.type: ENVELOPE_TYPE_TX\n"
                "feeBump.tx.innerTx.tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "feeBump.tx.innerTx.tx.fee: 4294967295\n"
                "feeBump.tx.innerTx.tx.seqNum: 9223372036854775807\n"
                "feeBump.tx.innerTx.tx.cond.type: PRECOND_NONE\n"
                "feeBump.tx.innerTx.tx.memo.type: MEMO_RETURN\n"
                "feeBump.tx.innerTx.tx.memo.retHash: 6465666768696a6b6c6d6e6f707172737475767778797a7b7c7d7e7f80818283\n"
                "feeBump.tx.innerTx.tx.operations.len: 1\n"
                "feeBump.tx.innerTx.tx.operations[0].sourceAccount._present: false\n"
                "feeBump.tx.innerTx.tx.operations[0].body.type: BUMP_SEQUENCE\n"
                "feeBump.tx.innerTx.tx.operations[0].body.bumpSequenceOp.bumpTo: -5\n"
                "feeBump.tx.innerTx.tx.ext.v: 0\n"
                "feeBump.tx.innerTx.signatures.len: 20\n"
                "feeBump.tx.innerTx.signatures[0].hint: 00000000\n"
                "feeBump.tx.innerTx.signatures[0].signature: 10101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010\n"
                "feeBump.tx.innerTx.signatures[1].hint: 01010101\n"
                "feeBump.tx.innerTx.signatures[1].signature: 11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111\n"
                "feeBump.tx.innerTx.signatures[2].hint: 02020202\n"
                "feeBump.tx.innerTx.signatures[2].signature: 12121212121212121212121212121212121212121212121212121212121212121212121212121212121212121212121212121212121212121212121212121212\n"
                "feeBump.tx.innerTx.signatures[3].hint: 03030303\n"
                "feeBump.tx.innerTx.signatures[3].signature: 13131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313131313\n"
                "feeBump.tx.innerTx.signatures[4].hint: 04040404\n"
                "feeBump.tx.innerTx.signatures[4].signature: 14141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414141414\n"
                "feeBump.tx.innerTx.signatures[5].hint: 05050505\n"
                "feeBump.tx.innerTx.signatures[5].signature: 15151515151515151515151515151515151515151515151515151515151515151515151515151515151515151515151515151515151515151515151515151515\n"
                "feeBump.tx.innerTx.signatures[6].hint: 06060606\n"
                "feeBump.tx.innerTx.signatures[6].signature: 16161616161616161616161616161616161616161616161616161616161616161616161616161616161616161616161616161616161616161616161616161616\n"
                "feeBump.tx.innerTx.signatures[7].hint: 07070707\n"
                "feeBump.tx.innerTx.signatures[7].signature: 17171717171717171717171717171717171717171717171717171717171717171717171717171717171717171717171717171717171717171717171717171717\n"
                "feeBump.tx.innerTx.signatures[8].hint: 08080808\n"
                "feeBump.tx.innerTx.signatures[8].signature: 18181818181818181818181818181818181818181818181818181818181818181818181818181818181818181818181818181818181818181818181818181818\n"
                "feeBump.tx.innerTx.signatures[9].hint: 09090909\n"
                "feeBump.tx.innerTx.signatures[9].signature: 19191919191919191919191919191919191919191919191919191919191919191919191919191919191919191919191919191919191919191919191919191919\n"
                "feeBump.tx.innerTx.signatures[10].hint: 0a0a0a0a\n"
                "feeBump.tx.innerTx.signatures[10].signature: 1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a1a\n"
                "feeBump.tx.innerTx.signatures[11].hint: 0b0b0b0b\n"
                "feeBump.tx.innerTx.signatures[11].signature: 1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b1b\n"
                "feeBump.tx.innerTx.signatures[12].hint: 0c0c0c0c\n"
                "feeBump.tx.innerTx.signatures[12].signature: 1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c1c\n"
                "feeBump.tx.innerTx.signatures[13].hint: 0d0d0d0d\n"
                "feeBump.tx.innerTx.signatures[13].signature: 1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d1d\n"
                "feeBump.tx.innerTx.signatures[14].hint: 0e0e0e0e\n"
                "feeBump.tx.innerTx.signatures[14].signature: 1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e1e\n"
                "feeBump.tx.innerTx.signatures[15].hint: 0f0f0f0f\n"
                "feeBump.tx.innerTx.signatures[15].signature: 1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f1f\n"
                "feeBump.tx.innerTx.signatures[16].hint: 10101010\n"
                "feeBump.tx.innerTx.signatures[16].signature: 20202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020\n"
                "feeBump.tx.innerTx.signatures[17].hint: 11111111\n"
                "feeBump.tx.innerTx.signatures[17].signature: 21212121212121212121212121212121212121212121212121212121212121212121212121212121212121212121212121212121212121212121212121212121\n"
                "feeBump.tx.innerTx.signatures[18].hint: 12121212\n"
                "feeBump.tx.innerTx.signatures[18].signature: 22222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222\n"
                "feeBump.tx.innerTx.signatures[19].hint: 13131313\n"
                "feeBump.tx.innerTx.signatures[19].signature: 23232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323232323\n"
                "feeBump.tx.ext.v: 0\n"
                "feeBump.signatures.len: 20\n"
                "feeBump.signatures[0].hint: 80808080\n"
                "feeBump.signatures[0].signature: 40\n"
                "feeBump.signatures[1].hint: 81818181\n"
                "feeBump.signatures[1].signature: 4141\n"
                "feeBump.signatures[2].hint: 82828282\n"
                "feeBump.signatures[2].signature: 424242\n"
                "feeBump.signatures[3].hint: 83838383\n"
                "feeBump.signatures[3].signature: 43434343\n"
                "feeBump.signatures[4].hint: 84848484\n"
                "feeBump.signatures[4].signature: 4444444444\n"
                "feeBump.signatures[5].hint: 85858585\n"
                "feeBump.signatures[5].signature: 454545454545\n"
                "feeBump.signatures[6].hint: 86868686\n"
                "feeBump.signatures[6].signature: 46464646464646\n"
                "feeBump.signatures[7].hint: 87878787\n"
                "feeBump.signatures[7].signature: 4747474747474747\n"
                "feeBump.signatures[8].hint: 88888888\n"
                "feeBump.signatures[8].signature: 484848484848484848\n"
                "feeBump.signatures[9].hint: 89898989\n"
                "feeBump.signatures[9].signature: 49494949494949494949\n"
                "feeBump.signatures[10].hint: 8a8a8a8a\n"
                "feeBump.signatures[10].signature: 4a4a4a4a4a4a4a4a4a4a4a\n"
                "feeBump.signatures[11].hint: 8b8b8b8b\n"
                "feeBump.signatures[11].signature: 4b4b4b4b4b4b4b4b4b4b4b4b\n"
                "feeBump.signatures[12].hint: 8c8c8c8c\n"
                "feeBump.signatures[12].signature: 4c4c4c4c4c4c4c4c4c4c4c4c4c\n"
                "feeBump.signatures[13].hint: 8d8d8d8d\n"
                "feeBump.signatures[13].signature: 4d4d4d4d4d4d4d4d4d4d4d4d4d4d\n"
                "feeBump.signatures[14].hint: 8e8e8e8e\n"
                "feeBump.signatures[14].signature: 4e4e4e4e4e4e4e4e4e4e4e4e4e4e4e\n"
                "feeBump.signatures[15].hint: 8f8f8f8f\n"
                "feeBump.signatures[15].signature: 4f4f4f4f4f4f4f4f4f4f4f4f4f4f4f4f\n"
                "feeBump.signatures[16].hint: 90909090\n"
                "feeBump.signatures[16].signature: 5050505050505050505050505050505050\n"
                "feeBump.signatures[17].hint: 91919191\n"
                "feeBump.signatures[17].signature: 515151515151515151515151515151515151\n"
                "feeBump.signatures[18].hint: 92929292\n"
                "feeBump.signatures[18].signature: 52525252525252525252525252525252525252\n"
                "feeBump.signatures[19].hint: 93939393\n"
                "feeBump.signatures[19].signature: 5353535353535353535353535353535353535353",
                "AAAABQAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXH//////////AAAAAgAAAACKiOPddAnxlf1S2y08ul1y"
                "ymcJvx2UEhvzdIgBtA9vXP////9//////////wAAAAAAAAAEZGVmZ2hpamtsbW5vcHFyc3R1dnd4eXp7fH1+f4CBgoMAAAAB"
                "AAAAAAAAAAv/////////+wAAAAAAAAAUAAAAAAAAAEAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQ"
                "EBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQAQEBAQAAAEARERERERERERERERERERERERERERERERERERERERERERERERERERER"
                "ERERERERERERERERERERERERERERERERAgICAgAAAEASEhISEhISEhISEhISEhISEhISEhISEhISEhISEhISEhISEhISEhIS"
                "EhISEhISEhISEhISEhISEhISEhISEhISAwMDAwAAAEATExMTExMTExMTExMTExMTExMTExMTExMTExMTExMTExMTExMTExMT"
                "ExMTExMTExMTExMTExMTExMTExMTExMTBAQEBAAAAEAUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQU"
                "FBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUBQUFBQAAAEAVFRUVFRUVFRUVFRUVFRUVFRUVFRUVFRUVFRUVFRUVFRUVFRUVFRUV"
                "FRUVFRUVFRUVFRUVFRUVFRUVFRUVFRUVBgYGBgAAAEAWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYW"
                "FhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWBwcHBwAAAEAXFxcXFxcXFxcXFxcXFxcXFxcXFxcXFxcXFxcXFxcXFxcXFxcXFxcX"
                "FxcXFxcXFxcXFxcXFxcXFxcXFxcXFxcXCAgICAAAAEAYGBgYGBgYGBgYGBgYGBgYGBgYGBgYGBgYGBgYGBgYGBgYGBgYGBgY"
                "GBgYGBgYGBgYGBgYGBgYGBgYGBgYGBgYCQkJCQAAAEAZGRkZGRkZGRkZGRkZGRkZGRkZGRkZGRkZGRkZGRkZGRkZGRkZGRkZ"
                "GRkZGRkZGRkZGRkZGRkZGRkZGRkZGRkZCgoKCgAAAEAaGhoaGhoaGhoaGhoaGhoaGhoaGhoaGhoaGhoaGhoaGhoaGhoaGhoa"
                "GhoaGhoaGhoaGhoaGhoaGhoaGhoaGhoaCwsLCwAAAEAbGxsbGxsbGxsbGxsbGxsbGxsbGxsbGxsbGxsbGxsbGxsbGxsbGxsb"
                "GxsbGxsbGxsbGxsbGxsbGxsbGxsbGxsbDAwMDAAAAEAcHBwcHBwcHBwcHBwcHBwcHBwcHBwcHBwcHBwcHBwcHBwcHBwcHBwc"
                "HBwcHBwcHBwcHBwcHBwcHBwcHBwcHBwcDQ0NDQAAAEAdHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0d"
                "HR0dHR0dHR0dHR0dHR0dHR0dHR0dHR0dDg4ODgAAAEAeHh4eHh4eHh4eHh4eHh4eHh4eHh4eHh4eHh4eHh4eHh4eHh4eHh4e"
                "Hh4eHh4eHh4eHh4eHh4eHh4eHh4eHh4eDw8PDwAAAEAfHx8fHx8fHx8fHx8fHx8fHx8fHx8fHx8fHx8fHx8fHx8fHx8fHx8f"
                "Hx8fHx8fHx8fHx8fHx8fHx8fHx8fHx8fEBAQEAAAAEAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAg"
                "ICAgICAgICAgICAgICAgICAgICAgICAgEREREQAAAEAhISEhISEhISEhISEhISEhISEhISEhISEhISEhISEhISEhISEhISEh"
                "ISEhISEhISEhISEhISEhISEhISEhISEhEhISEgAAAEAiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIi"
                "IiIiIiIiIiIiIiIiIiIiIiIiIiIiIiIiExMTEwAAAEAjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMj"
                "IyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjAAAAAAAAABSAgICAAAAAAUAAAACBgYGBAAAAAkFBAACCgoKCAAAAA0JCQgCDg4OD"
                "AAAABENDQ0OEhISEAAAABUREREREAAAAhYWFhQAAAAZFRUVFRUUAAIaGhoYAAAAHRkZGRkZGRgCHh4eHAAAACEdHR0dHR0dH"
                "iIiIiAAAAAlISEhISEhISEgAAACJiYmJAAAACklJSUlJSUlJSUkAAIqKiooAAAALSkpKSkpKSkpKSkoAi4uLiwAAAAxLS0tL"
                "S0tLS0tLS0uMjIyMAAAADUxMTExMTExMTExMTEwAAACNjY2NAAAADk1NTU1NTU1NTU1NTU1NAACOjo6OAAAAD05OTk5OTk5O"
                "Tk5OTk5OTgCPj4+PAAAAEE9PT09PT09PT09PT09PT0+QkJCQAAAAEVBQUFBQUFBQUFBQUFBQUFBQAAAAkZGRkQAAABJRUVFR"
                "UVFRUVFRUVFRUVFRUVEAAJKSkpIAAAATUlJSUlJSUlJSUlJSUlJSUlJSUgCTk5OTAAAAFFNTU1NTU1NTU1NTU1NTU1NTU1NT",
                "ee9b63a7bc742bdc6501809efe2ed24cf3580cf590c970c0b96962193899ffb9" },
            { "fee_bump_of_v0",
                "type: ENVELOPE_TYPE_TX_FEE_BUMP\n"
                "feeBump.tx.feeSource: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "feeBump.tx.fee: 300\n"
                "feeBump.tx.innerTx.type: ENVELOPE_TYPE_TX\n"
                "feeBump.tx.innerTx.tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "feeBump.tx.innerTx.tx.fee: 100\n"
                "feeBump.tx.innerTx.tx.seqNum: 1234567890127\n"
                "feeBump.tx.innerTx.tx.cond.type: PRECOND_TIME\n"
                "feeBump.tx.innerTx.tx.cond.timeBounds.minTime: 1 (1970-01-01 00:00:01.000000+00:00 (UTC))\n"
                "feeBump.tx.innerTx.tx.cond.timeBounds.maxTime: 2 (1970-01-01 00:00:02.000000+00:00 (UTC))\n"
                "feeBump.tx.innerTx.tx.memo.type: MEMO_NONE\n"
                "feeBump.tx.innerTx.tx.operations.len: 1\n"
                "feeBump.tx.innerTx.tx.operations[0].sourceAccount._present: false\n"
                "feeBump.tx.innerTx.tx.operations[0].body.type: BUMP_SEQUENCE\n"
                "feeBump.tx.innerTx.tx.operations[0].body.bumpSequenceOp.bumpTo: 2\n"
                "feeBump.tx.innerTx.tx.ext.v: 0\n"
                "feeBump.tx.innerTx.signatures.len: 1\n"
                "feeBump.tx.innerTx.signatures[0].hint: b40f6f5c\n"
                "feeBump.tx.innerTx.signatures[0].signature: f459d1451d4f040fe484c21492982e3d2360345c59e6d98d5664543de32eaa7a1f02b68f2460ee8def4a13b8be337c75c98fcbc4bfeacdf5d1fa7cced8a7ce07\n"
                "feeBump.tx.ext.v: 0\n"
                "feeBump.signatures.len: 1\n"
                "feeBump.signatures[0].hint: 8fc9b394\n"
                "feeBump.signatures[0].signature: be38ce8e8063ddf1fb7838853dfaf9a2a85f396340c31b2977490ec2f5abb46ad67b84349f7ab10d25e01b7f94fa9cc3e360d322350280456c762e29bf097b07",
                "AAAABQAAAACBOXcOqH0XX1ajVGbDTH7My42KkbTuN6Jd9g9bj8mzlAAAAAAAAAEsAAAAAgAAAACKiOPddAnxlf1S2y08ul1y"
                "ymcJvx2UEhvzdIgBtA9vXAAAAGQAAAEfcfsEzwAAAAEAAAAAAAAAAQAAAAAAAAACAAAAAAAAAAEAAAAAAAAACwAAAAAAAAAC"
                "AAAAAAAAAAG0D29cAAAAQPRZ0UUdTwQP5ITCFJKYLj0jYDRcWebZjVZkVD3jLqp6HwK2jyRg7o3vShO4vjN8dcmPy8S/6s31"
                "0fp8ztinzgcAAAAAAAAAAY/Js5QAAABAvjjOjoBj3fH7eDiFPfr5oqhfOWNAwxspd0kOwvWrtGrWe4Q0n3qxDSXgG3+U+pzD"
                "42DTIjUCgEVsdi4pvwl7Bw==",
                "baf8bdc90327af289c52d13709a1795d2feff7689c72de93dbe4ea2c5d91052b" },
            { "fee_bump_decoded",
                "type: ENVELOPE_TYPE_TX_FEE_BUMP\n"
                "feeBump.tx.feeSource: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "feeBump.tx.fee: 800\n"
                "feeBump.tx.innerTx.type: ENVELOPE_TYPE_TX\n"
                "feeBump.tx.innerTx.tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "feeBump.tx.innerTx.tx.fee: 300\n"
                "feeBump.tx.innerTx.tx.seqNum: 1234567890128\n"
                "feeBump.tx.innerTx.tx.cond.type: PRECOND_TIME\n"
                "feeBump.tx.innerTx.tx.cond.timeBounds.minTime: 0 (1970-01-01 00:00:00.000000+00:00 (UTC))\n"
                "feeBump.tx.innerTx.tx.cond.timeBounds.maxTime: 1800000000 (2027-01-15 08:00:00.000000+00:00 (UTC))\n"
                "feeBump.tx.innerTx.tx.memo.type: MEMO_NONE\n"
                "feeBump.tx.innerTx.tx.operations.len: 3\n"
                "feeBump.tx.innerTx.tx.operations[0].sourceAccount._present: false\n"
                "feeBump.tx.innerTx.tx.operations[0].body.type: PAYMENT\n"
                "feeBump.tx.innerTx.tx.operations[0].body.paymentOp.destination: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "feeBump.tx.innerTx.tx.operations[0].body.paymentOp.asset: native\n"
                "feeBump.tx.innerTx.tx.operations[0].body.paymentOp.amount: 1 (1E-7)\n"
                "feeBump.tx.innerTx.tx.operations[1].sourceAccount._present: false\n"
                "feeBump.tx.innerTx.tx.operations[1].body.type: PAYMENT\n"
                "feeBump.tx.innerTx.tx.operations[1].body.paymentOp.destination: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "feeBump.tx.innerTx.tx.operations[1].body.paymentOp.asset: USD:GDFJHLAXAUMHA4OWPOB4P7YO72AQR2HMIUYFOXLXE2DZGM633K7HZDQP\n"
                "feeBump.tx.innerTx.tx.operations[1].body.paymentOp.amount: 50 (0.000005)\n"
                "feeBump.tx.innerTx.tx.operations[2].sourceAccount._present: false\n"
                "feeBump.tx.innerTx.tx.operations[2].body.type: CREATE_ACCOUNT\n"
                "feeBump.tx.innerTx.tx.operations[2].body.createAccountOp.destination: GDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435DIAG\n"
                "feeBump.tx.innerTx.tx.operations[2].body.createAccountOp.startingBalance: 1000000000 (100)\n"
                "feeBump.tx.innerTx.tx.ext.v: 0\n"
                "feeBump.tx.innerTx.signatures.len: 1\n"
                "feeBump.tx.innerTx.signatures[0].hint: b40f6f5c\n"
                "feeBump.tx.innerTx.signatures[0].signature: c41e6ceed5d29ddbf2dad71dbbeb594a3a1391345c75eab011792a229043518ccf782d1850bf4a6d8c399d149d331d4da57a7ed70e795e38a59364f7eb68d80f\n"
                "feeBump.tx.ext.v: 0\n"
                "feeBump.signatures.len: 1\n"
                "feeBump.signatures[0].hint: 8fc9b394\n"
                "feeBump.signatures[0].signature: 3a0e5d1ccd1b026e60f1c3a3d689d18b7601d2e38c05f15d3d5ebdc185fb02be22ca0fcf3e244e8a69308fb41f1fdd196742840a660f225d0715ab5479b41508",
                "AAAABQAAAACBOXcOqH0XX1ajVGbDTH7My42KkbTuN6Jd9g9bj8mzlAAAAAAAAAMgAAAAAgAAAACKiOPddAnxlf1S2y08ul1y"
                "ymcJvx2UEhvzdIgBtA9vXAAAASwAAAEfcfsE0AAAAAEAAAAAAAAAAAAAAABrSdIAAAAAAAAAAAMAAAAAAAAAAQAAAACBOXcO"
                "qH0XX1ajVGbDTH7My42KkbTuN6Jd9g9bj8mzlAAAAAAAAAAAAAAAAQAAAAAAAAABAAAAAIE5dw6ofRdfVqNUZsNMfszLjYqR"
                "tO43ol32D1uPybOUAAAAAVVTRAAAAAAAypOsFwUYcHHWe4PH/w7+gQjo7EUwV113JoeTM9vavnwAAAAAAAAAMgAAAAAAAAAA"
                "AAAAAO1JKMYo0cLG6ukDOJBZlWEpWSc6XGP5NjbBRhSshzfRAAAAADuaygAAAAAAAAAAAbQPb1wAAABAxB5s7tXSndvy2tcd"
                "u+tZSjoTkTRcdeqwEXkqIpBDUYzPeC0YUL9KbYw5nRSdMx1NpXp+1w55Xjilk2T362jYDwAAAAAAAAABj8mzlAAAAEA6Dl0c"
                "zRsCbmDxw6PWidGLdgHS44wF8V09Xr3BhfsCviLKD88+JE6KaTCPtB8f3RlnQoQKZg8iXQcVq1R5tBUI",
                "f09ebe73d5cd4fb1fb3e3ab5e996d1c8d0c9e803b063b245b69ff7c24526a9a9" },
        };
        return cases;
    }

    // ─── The stc corpus ─────────────────────────────────────────────────

    /** A document written by stc, and the envelope and hash it is of. */
    struct StcCorpusCase
    {
        const char* name;
        bool publicNetwork;         // else the test network
        const char* txrep;
        const char* envelope;
        const char* hash;
    };

    static QString stcDocument(const char* name)
    {
        for (const StcCorpusCase& c : stcCorpus()) {
            if (qstrcmp(c.name, name) == 0)
                return QString::fromLatin1(c.txrep);
        }
        return QString();
    }

    /**
     * Written by stc v0.2.0 (github.com/xdrpp/stc, the latest release; its
     * txrep code is the same on master), through StellarNet.TxToRep, the
     * call the stc command prints with, from envelopes py-stellar-sdk 14.1.0
     * built with the keys and accounts of pyCorpus(), signed by stc. As the
     * stc command writes them for the test network (and, "_public", the
     * public one), with its signer cache holding the accounts the
     * transaction names, as the command fills it, and time.Local in UTC, as
     * where TZ=UTC (the command run on this machine writes the same, with
     * CET for UTC); "_unmatched" with an empty signer cache, and
     * "_plus0100", "_minus0930" and "_dubai" in zones Go names by their
     * offset. Every payment, trustline, claimable balance and offer amount is
     * 1000 units or more, or an extreme; signers_x_p adds a hash-x signer's
     * preimage and a signed-payload signature, which stc names; and
     * manage_data_empty_value (pyCorpus()'s envelope, with a signature of no
     * bytes added) an empty data value and signature, which stc writes as
     * "0 bytes". The "pubnet_" ones are transactions of the public network,
     * as Horizon serves them: 0f27112a... (ledger 64668018), whose maxTime
     * 1790626169993 is milliseconds given as seconds and which stc dates in
     * the year 58712, and 700851e7... (ledger 63550000), a payment whose
     * maxTime is in July; "_madrid" is what the stc command (v0.2.0, Go
     * 1.26.3) wrote on this Windows machine, set to Madrid's zone, and
     * "_madrid_tzdata" what stc writes in Madrid with the tz database, as on
     * Linux (time.Local set to Europe/Madrid).
     */
    static const std::vector<StcCorpusCase>& stcCorpus()
    {
        static const std::vector<StcCorpusCase> cases = {
            { "payments_big", false,
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 500\n"
                "tx.seqNum: 1234567890123\n"
                "tx.cond.type: PRECOND_TIME\n"
                "tx.cond.timeBounds.minTime: 1700000000 (Tue Nov 14 22:13:20 UTC 2023)\n"
                "tx.cond.timeBounds.maxTime: 1800000000 (Fri Jan 15 08:00:00 UTC 2027)\n"
                "tx.memo.type: MEMO_NONE\n"
                "tx.operations.len: 5\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: PAYMENT\n"
                "tx.operations[0].body.paymentOp.destination: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[0].body.paymentOp.asset: TestXLM\n"
                "tx.operations[0].body.paymentOp.amount: 10000000000 (1,000e7)\n"
                "tx.operations[1].sourceAccount._present: false\n"
                "tx.operations[1].body.type: PAYMENT\n"
                "tx.operations[1].body.paymentOp.destination: GDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435DIAG\n"
                "tx.operations[1].body.paymentOp.asset: USD:GDFJHLAXAUMHA4OWPOB4P7YO72AQR2HMIUYFOXLXE2DZGM633K7HZDQP\n"
                "tx.operations[1].body.paymentOp.amount: 123456789012 (12,345.6789012e7)\n"
                "tx.operations[2].sourceAccount._present: false\n"
                "tx.operations[2].body.type: PAYMENT\n"
                "tx.operations[2].body.paymentOp.destination: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[2].body.paymentOp.asset: LONGASSET012:GBXHUHG5FGYLPD6RHL2MKWMP572O6KUXCZXDZJXS4T57ZTMAKBN7DWXN\n"
                "tx.operations[2].body.paymentOp.amount: 9223372036854775807 (922,337,203,685.4775807e7)\n"
                "tx.operations[3].sourceAccount._present: false\n"
                "tx.operations[3].body.type: PAYMENT\n"
                "tx.operations[3].body.paymentOp.destination: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[3].body.paymentOp.asset: TestXLM\n"
                "tx.operations[3].body.paymentOp.amount: 1 (0.0000001e7)\n"
                "tx.operations[4].sourceAccount._present: false\n"
                "tx.operations[4].body.type: CREATE_ACCOUNT\n"
                "tx.operations[4].body.createAccountOp.destination: GDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435DIAG\n"
                "tx.operations[4].body.createAccountOp.startingBalance: 10000000000000 (1,000,000e7)\n"
                "tx.ext.v: 0\n"
                "signatures.len: 1\n"
                "signatures[0].hint: b40f6f5c (GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR)\n"
                "signatures[0].signature: 1f1a0da35d684b28e51aba77ec359d7306dd0956cce5bd902bd9a1589093f2aafc05d48ae29c4035e2e29c488865ba51aa647727a85cc37b03ba4f4144e5e404",
                "AAAAAgAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXAAAAfQAAAEfcfsEywAAAAEAAAAAZVPxAAAAAABrSdIA"
                "AAAAAAAAAAUAAAAAAAAAAQAAAACBOXcOqH0XX1ajVGbDTH7My42KkbTuN6Jd9g9bj8mzlAAAAAAAAAACVAvkAAAAAAAAAAAB"
                "AAAAAO1JKMYo0cLG6ukDOJBZlWEpWSc6XGP5NjbBRhSshzfRAAAAAVVTRAAAAAAAypOsFwUYcHHWe4PH/w7+gQjo7EUwV113"
                "JoeTM9vavnwAAAAcvpkaFAAAAAAAAAABAAAAAIE5dw6ofRdfVqNUZsNMfszLjYqRtO43ol32D1uPybOUAAAAAkxPTkdBU1NF"
                "VDAxMgAAAABuehzdKbC3j9E69MVZj+/07yqXFm48pvLk+/zNgFBb8X//////////AAAAAAAAAAEAAAAAgTl3Dqh9F19Wo1Rm"
                "w0x+zMuNipG07jeiXfYPW4/Js5QAAAAAAAAAAAAAAAEAAAAAAAAAAAAAAADtSSjGKNHCxurpAziQWZVhKVknOlxj+TY2wUYU"
                "rIc30QAACRhOcqAAAAAAAAAAAAG0D29cAAAAQB8aDaNdaEso5Rq6d+w1nXMG3QlWzOW9kCvZoViQk/Kq/AXUiuKcQDXi4pxI"
                "iGW6UapkdyeoXMN7A7pPQUTl5AQ=",
                "aefab24d24618e2a10f39c878a6a704200c0b7e9c9879f688ad279358c84e68a" },
            { "trustlines", false,
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 300\n"
                "tx.seqNum: 1234567890123\n"
                "tx.cond.type: PRECOND_TIME\n"
                "tx.cond.timeBounds.minTime: 0\n"
                "tx.cond.timeBounds.maxTime: 1800000000 (Fri Jan 15 08:00:00 UTC 2027)\n"
                "tx.memo.type: MEMO_TEXT\n"
                "tx.memo.text: \"trust\"\n"
                "tx.operations.len: 3\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: CHANGE_TRUST\n"
                "tx.operations[0].body.changeTrustOp.line.type: ASSET_TYPE_CREDIT_ALPHANUM4\n"
                "tx.operations[0].body.changeTrustOp.line.alphaNum4.assetCode: 55534400\n"
                "tx.operations[0].body.changeTrustOp.line.alphaNum4.issuer: GDFJHLAXAUMHA4OWPOB4P7YO72AQR2HMIUYFOXLXE2DZGM633K7HZDQP\n"
                "tx.operations[0].body.changeTrustOp.limit: 9223372036854775807 (922,337,203,685.4775807e7)\n"
                "tx.operations[1].sourceAccount._present: false\n"
                "tx.operations[1].body.type: CHANGE_TRUST\n"
                "tx.operations[1].body.changeTrustOp.line.type: ASSET_TYPE_CREDIT_ALPHANUM12\n"
                "tx.operations[1].body.changeTrustOp.line.alphaNum12.assetCode: 4c4f4e474153534554303132\n"
                "tx.operations[1].body.changeTrustOp.line.alphaNum12.issuer: GBXHUHG5FGYLPD6RHL2MKWMP572O6KUXCZXDZJXS4T57ZTMAKBN7DWXN\n"
                "tx.operations[1].body.changeTrustOp.limit: 50000000000 (5,000e7)\n"
                "tx.operations[2].sourceAccount._present: false\n"
                "tx.operations[2].body.type: CHANGE_TRUST\n"
                "tx.operations[2].body.changeTrustOp.line.type: ASSET_TYPE_CREDIT_ALPHANUM4\n"
                "tx.operations[2].body.changeTrustOp.line.alphaNum4.assetCode: 55534400\n"
                "tx.operations[2].body.changeTrustOp.line.alphaNum4.issuer: GDFJHLAXAUMHA4OWPOB4P7YO72AQR2HMIUYFOXLXE2DZGM633K7HZDQP\n"
                "tx.operations[2].body.changeTrustOp.limit: 0 (0e7)\n"
                "tx.ext.v: 0\n"
                "signatures.len: 1\n"
                "signatures[0].hint: b40f6f5c (GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR)\n"
                "signatures[0].signature: b8d3da773b8d448932025f1c44d5ce88f3b99e1f83fa1dd11fcb357cd4f944b154b3d0f5459ef6b6e92f8a5db1164e2330ce1249d584dae86c3eee601810a508",
                "AAAAAgAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXAAAASwAAAEfcfsEywAAAAEAAAAAAAAAAAAAAABrSdIA"
                "AAAAAQAAAAV0cnVzdAAAAAAAAAMAAAAAAAAABgAAAAFVU0QAAAAAAMqTrBcFGHBx1nuDx/8O/oEI6OxFMFdddyaHkzPb2r58"
                "f/////////8AAAAAAAAABgAAAAJMT05HQVNTRVQwMTIAAAAAbnoc3Smwt4/ROvTFWY/v9O8qlxZuPKby5Pv8zYBQW/EAAAAL"
                "pDt0AAAAAAAAAAAGAAAAAVVTRAAAAAAAypOsFwUYcHHWe4PH/w7+gQjo7EUwV113JoeTM9vavnwAAAAAAAAAAAAAAAAAAAAB"
                "tA9vXAAAAEC409p3O41EiTICXxxE1c6I87meH4P6HdEfyzV81PlEsVSz0PVFnva26S+KXbEWTiMwzhJJ1YTa6Gw+7mAYEKUI",
                "c34199b15ebea54051dd26518ba97bacc840f4c334e8b43989dc1de94062d0fb" },
            { "pool_trustline", false,
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 100\n"
                "tx.seqNum: 1234567890123\n"
                "tx.cond.type: PRECOND_NONE\n"
                "tx.memo.type: MEMO_NONE\n"
                "tx.operations.len: 1\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: CHANGE_TRUST\n"
                "tx.operations[0].body.changeTrustOp.line.type: ASSET_TYPE_POOL_SHARE\n"
                "tx.operations[0].body.changeTrustOp.line.liquidityPool.type: LIQUIDITY_POOL_CONSTANT_PRODUCT\n"
                "tx.operations[0].body.changeTrustOp.line.liquidityPool.constantProduct.assetA: TestXLM\n"
                "tx.operations[0].body.changeTrustOp.line.liquidityPool.constantProduct.assetB: USD:GDFJHLAXAUMHA4OWPOB4P7YO72AQR2HMIUYFOXLXE2DZGM633K7HZDQP\n"
                "tx.operations[0].body.changeTrustOp.line.liquidityPool.constantProduct.fee: 30\n"
                "tx.operations[0].body.changeTrustOp.limit: 9223372036854775807 (922,337,203,685.4775807e7)\n"
                "tx.ext.v: 0\n"
                "signatures.len: 1\n"
                "signatures[0].hint: b40f6f5c (GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR)\n"
                "signatures[0].signature: 974d7a6277ee00b7fad5409f06a6b604820a14a7e7458a70952e1e67d2e05a31cd80cd9ec0e0cd7ad36dd8d21d9053bae8c707593d5b03f09bb761c18e1cd60a",
                "AAAAAgAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXAAAAGQAAAEfcfsEywAAAAAAAAAAAAAAAQAAAAAAAAAG"
                "AAAAAwAAAAAAAAAAAAAAAVVTRAAAAAAAypOsFwUYcHHWe4PH/w7+gQjo7EUwV113JoeTM9vavnwAAAAef/////////8AAAAA"
                "AAAAAbQPb1wAAABAl016YnfuALf61UCfBqa2BIIKFKfnRYpwlS4eZ9LgWjHNgM2ewODNetNt2NIdkFO66McHWT1bA/Cbt2HB"
                "jhzWCg==",
                "29ba620b4f86c7bd216a6a8a49fa55f3c5e484223b383f504c951d06b1878ab3" },
            { "claimable", false,
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 300\n"
                "tx.seqNum: 1234567890123\n"
                "tx.cond.type: PRECOND_TIME\n"
                "tx.cond.timeBounds.minTime: 1700000000 (Tue Nov 14 22:13:20 UTC 2023)\n"
                "tx.cond.timeBounds.maxTime: 0\n"
                "tx.memo.type: MEMO_NONE\n"
                "tx.operations.len: 3\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: CREATE_CLAIMABLE_BALANCE\n"
                "tx.operations[0].body.createClaimableBalanceOp.asset: TestXLM\n"
                "tx.operations[0].body.createClaimableBalanceOp.amount: 15000000000 (1,500e7)\n"
                "tx.operations[0].body.createClaimableBalanceOp.claimants.len: 4\n"
                "tx.operations[0].body.createClaimableBalanceOp.claimants[0].type: CLAIMANT_TYPE_V0\n"
                "tx.operations[0].body.createClaimableBalanceOp.claimants[0].v0.destination: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[0].body.createClaimableBalanceOp.claimants[0].v0.predicate.type: CLAIM_PREDICATE_BEFORE_ABSOLUTE_TIME\n"
                "tx.operations[0].body.createClaimableBalanceOp.claimants[0].v0.predicate.absBefore: 1800000000 (180e7)\n"
                "tx.operations[0].body.createClaimableBalanceOp.claimants[1].type: CLAIMANT_TYPE_V0\n"
                "tx.operations[0].body.createClaimableBalanceOp.claimants[1].v0.destination: GDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435DIAG\n"
                "tx.operations[0].body.createClaimableBalanceOp.claimants[1].v0.predicate.type: CLAIM_PREDICATE_BEFORE_RELATIVE_TIME\n"
                "tx.operations[0].body.createClaimableBalanceOp.claimants[1].v0.predicate.relBefore: 86400 (0.00864e7)\n"
                "tx.operations[0].body.createClaimableBalanceOp.claimants[2].type: CLAIMANT_TYPE_V0\n"
                "tx.operations[0].body.createClaimableBalanceOp.claimants[2].v0.destination: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.operations[0].body.createClaimableBalanceOp.claimants[2].v0.predicate.type: CLAIM_PREDICATE_NOT\n"
                "tx.operations[0].body.createClaimableBalanceOp.claimants[2].v0.predicate.notPredicate._present: true\n"
                "tx.operations[0].body.createClaimableBalanceOp.claimants[2].v0.predicate.notPredicate.type: CLAIM_PREDICATE_BEFORE_ABSOLUTE_TIME\n"
                "tx.operations[0].body.createClaimableBalanceOp.claimants[2].v0.predicate.notPredicate.absBefore: 1700000000 (170e7)\n"
                "tx.operations[0].body.createClaimableBalanceOp.claimants[3].type: CLAIMANT_TYPE_V0\n"
                "tx.operations[0].body.createClaimableBalanceOp.claimants[3].v0.destination: GDFJHLAXAUMHA4OWPOB4P7YO72AQR2HMIUYFOXLXE2DZGM633K7HZDQP\n"
                "tx.operations[0].body.createClaimableBalanceOp.claimants[3].v0.predicate.type: CLAIM_PREDICATE_AND\n"
                "tx.operations[0].body.createClaimableBalanceOp.claimants[3].v0.predicate.andPredicates.len: 2\n"
                "tx.operations[0].body.createClaimableBalanceOp.claimants[3].v0.predicate.andPredicates[0].type: CLAIM_PREDICATE_BEFORE_RELATIVE_TIME\n"
                "tx.operations[0].body.createClaimableBalanceOp.claimants[3].v0.predicate.andPredicates[0].relBefore: 3600 (0.00036e7)\n"
                "tx.operations[0].body.createClaimableBalanceOp.claimants[3].v0.predicate.andPredicates[1].type: CLAIM_PREDICATE_OR\n"
                "tx.operations[0].body.createClaimableBalanceOp.claimants[3].v0.predicate.andPredicates[1].orPredicates.len: 2\n"
                "tx.operations[0].body.createClaimableBalanceOp.claimants[3].v0.predicate.andPredicates[1].orPredicates[0].type: CLAIM_PREDICATE_BEFORE_ABSOLUTE_TIME\n"
                "tx.operations[0].body.createClaimableBalanceOp.claimants[3].v0.predicate.andPredicates[1].orPredicates[0].absBefore: 1750000000 (175e7)\n"
                "tx.operations[0].body.createClaimableBalanceOp.claimants[3].v0.predicate.andPredicates[1].orPredicates[1].type: CLAIM_PREDICATE_UNCONDITIONAL\n"
                "tx.operations[1].sourceAccount._present: false\n"
                "tx.operations[1].body.type: CLAIM_CLAIMABLE_BALANCE\n"
                "tx.operations[1].body.claimClaimableBalanceOp.balanceID.type: CLAIMABLE_BALANCE_ID_TYPE_V0\n"
                "tx.operations[1].body.claimClaimableBalanceOp.balanceID.v0: abababababababababababababababababababababababababababababababab\n"
                "tx.operations[2].sourceAccount._present: false\n"
                "tx.operations[2].body.type: CLAWBACK_CLAIMABLE_BALANCE\n"
                "tx.operations[2].body.clawbackClaimableBalanceOp.balanceID.type: CLAIMABLE_BALANCE_ID_TYPE_V0\n"
                "tx.operations[2].body.clawbackClaimableBalanceOp.balanceID.v0: cdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcd\n"
                "tx.ext.v: 0\n"
                "signatures.len: 1\n"
                "signatures[0].hint: b40f6f5c (GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR)\n"
                "signatures[0].signature: 558f060274e1812d004c861385403eff1812ef1be9a6b7634202a72867d78eaa847403c7b3517bfe0f41225a47976dcb7b0ffe7a76b5f11256ca4d68bed2760d",
                "AAAAAgAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXAAAASwAAAEfcfsEywAAAAEAAAAAZVPxAAAAAAAAAAAA"
                "AAAAAAAAAAMAAAAAAAAADgAAAAAAAAADfhHWAAAAAAQAAAAAAAAAAIE5dw6ofRdfVqNUZsNMfszLjYqRtO43ol32D1uPybOU"
                "AAAABAAAAABrSdIAAAAAAAAAAADtSSjGKNHCxurpAziQWZVhKVknOlxj+TY2wUYUrIc30QAAAAUAAAAAAAFRgAAAAAAAAAAA"
                "iojj3XQJ8ZX9UtstPLpdcspnCb8dlBIb83SIAbQPb1wAAAADAAAAAQAAAAQAAAAAZVPxAAAAAAAAAAAAypOsFwUYcHHWe4PH"
                "/w7+gQjo7EUwV113JoeTM9vavnwAAAABAAAAAgAAAAUAAAAAAAAOEAAAAAIAAAACAAAABAAAAABoTuGAAAAAAAAAAAAAAAAP"
                "AAAAAKurq6urq6urq6urq6urq6urq6urq6urq6urq6urq6urAAAAAAAAABQAAAAAzc3Nzc3Nzc3Nzc3Nzc3Nzc3Nzc3Nzc3N"
                "zc3Nzc3Nzc0AAAAAAAAAAbQPb1wAAABAVY8GAnThgS0ATIYThUA+/xgS7xvpprdjQgKnKGfXjqqEdAPHs1F7/g9BIlpHl23L"
                "ew/+ena18RJWyk1ovtJ2DQ==",
                "b5c75092b9fd3191a9b3cf29c788e1ff64c0b3d9a0c7cf3732914e549e74b690" },
            { "muxed", false,
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: MCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYABAAAAAAAAAAFPM6\n"
                "tx.fee: 500\n"
                "tx.seqNum: 1234567890123\n"
                "tx.cond.type: PRECOND_TIME\n"
                "tx.cond.timeBounds.minTime: 1700000000 (Tue Nov 14 22:13:20 UTC 2023)\n"
                "tx.cond.timeBounds.maxTime: 1800000000 (Fri Jan 15 08:00:00 UTC 2027)\n"
                "tx.memo.type: MEMO_NONE\n"
                "tx.operations.len: 5\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: PAYMENT\n"
                "tx.operations[0].body.paymentOp.destination: MCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZIAAAAAAAAAAAAHZ7W\n"
                "tx.operations[0].body.paymentOp.asset: TestXLM\n"
                "tx.operations[0].body.paymentOp.amount: 25000000000 (2,500e7)\n"
                "tx.operations[1].sourceAccount._present: true\n"
                "tx.operations[1].sourceAccount: MCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYABAAAAAAAAAAFPM6\n"
                "tx.operations[1].body.type: ACCOUNT_MERGE\n"
                "tx.operations[1].body.destination: MDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435D7777777777777RCC\n"
                "tx.operations[2].sourceAccount._present: false\n"
                "tx.operations[2].body.type: PATH_PAYMENT_STRICT_SEND\n"
                "tx.operations[2].body.pathPaymentStrictSendOp.sendAsset: USD:GDFJHLAXAUMHA4OWPOB4P7YO72AQR2HMIUYFOXLXE2DZGM633K7HZDQP\n"
                "tx.operations[2].body.pathPaymentStrictSendOp.sendAmount: 10000000000 (1,000e7)\n"
                "tx.operations[2].body.pathPaymentStrictSendOp.destination: MCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZIAAAAAAAAAAAAHZ7W\n"
                "tx.operations[2].body.pathPaymentStrictSendOp.destAsset: EUR:GBXHUHG5FGYLPD6RHL2MKWMP572O6KUXCZXDZJXS4T57ZTMAKBN7DWXN\n"
                "tx.operations[2].body.pathPaymentStrictSendOp.destMin: 9995000000 (999.5e7)\n"
                "tx.operations[2].body.pathPaymentStrictSendOp.path.len: 1\n"
                "tx.operations[2].body.pathPaymentStrictSendOp.path[0]: TestXLM\n"
                "tx.operations[3].sourceAccount._present: false\n"
                "tx.operations[3].body.type: PATH_PAYMENT_STRICT_RECEIVE\n"
                "tx.operations[3].body.pathPaymentStrictReceiveOp.sendAsset: TestXLM\n"
                "tx.operations[3].body.pathPaymentStrictReceiveOp.sendMax: 50000000000 (5,000e7)\n"
                "tx.operations[3].body.pathPaymentStrictReceiveOp.destination: MDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435D7777777777777RCC\n"
                "tx.operations[3].body.pathPaymentStrictReceiveOp.destAsset: USD:GDFJHLAXAUMHA4OWPOB4P7YO72AQR2HMIUYFOXLXE2DZGM633K7HZDQP\n"
                "tx.operations[3].body.pathPaymentStrictReceiveOp.destAmount: 10000000000 (1,000e7)\n"
                "tx.operations[3].body.pathPaymentStrictReceiveOp.path.len: 0\n"
                "tx.operations[4].sourceAccount._present: false\n"
                "tx.operations[4].body.type: CLAWBACK\n"
                "tx.operations[4].body.clawbackOp.asset: USD:GDFJHLAXAUMHA4OWPOB4P7YO72AQR2HMIUYFOXLXE2DZGM633K7HZDQP\n"
                "tx.operations[4].body.clawbackOp.from: MCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZIAAAAAAAAAAAAHZ7W\n"
                "tx.operations[4].body.clawbackOp.amount: 10000000000 (1,000e7)\n"
                "tx.ext.v: 0\n"
                "signatures.len: 1\n"
                "signatures[0].hint: b40f6f5c (GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR)\n"
                "signatures[0].signature: a360dcf3e1d1f58c3358e0cf49d4cedbc1a81d1c221790ef8bdb357d3c9d56797dfa5b7749738b96f7ab7a08dce2c37d0cf426c46006ca249367f1c2f0ef340f",
                "AAAAAgAAAQAAIAAAAAAAAYqI4910CfGV/VLbLTy6XXLKZwm/HZQSG/N0iAG0D29cAAAB9AAAAR9x+wTLAAAAAQAAAABlU/EA"
                "AAAAAGtJ0gAAAAAAAAAABQAAAAAAAAABAAABAAAAAAAAAAABgTl3Dqh9F19Wo1Rmw0x+zMuNipG07jeiXfYPW4/Js5QAAAAA"
                "AAAABdIdugAAAAABAAABAAAgAAAAAAABiojj3XQJ8ZX9UtstPLpdcspnCb8dlBIb83SIAbQPb1wAAAAIAAABAP//////////"
                "7UkoxijRwsbq6QM4kFmVYSlZJzpcY/k2NsFGFKyHN9EAAAAAAAAADQAAAAFVU0QAAAAAAMqTrBcFGHBx1nuDx/8O/oEI6OxF"
                "MFdddyaHkzPb2r58AAAAAlQL5AAAAAEAAAAAAAAAAAGBOXcOqH0XX1ajVGbDTH7My42KkbTuN6Jd9g9bj8mzlAAAAAFFVVIA"
                "AAAAAG56HN0psLeP0Tr0xVmP7/TvKpcWbjym8uT7/M2AUFvxAAAAAlO/mMAAAAABAAAAAAAAAAAAAAACAAAAAAAAAAukO3QA"
                "AAABAP//////////7UkoxijRwsbq6QM4kFmVYSlZJzpcY/k2NsFGFKyHN9EAAAABVVNEAAAAAADKk6wXBRhwcdZ7g8f/Dv6B"
                "COjsRTBXXXcmh5Mz29q+fAAAAAJUC+QAAAAAAAAAAAAAAAATAAAAAVVTRAAAAAAAypOsFwUYcHHWe4PH/w7+gQjo7EUwV113"
                "JoeTM9vavnwAAAEAAAAAAAAAAAGBOXcOqH0XX1ajVGbDTH7My42KkbTuN6Jd9g9bj8mzlAAAAAJUC+QAAAAAAAAAAAG0D29c"
                "AAAAQKNg3PPh0fWMM1jgz0nUztvBqB0cIheQ74vbNX08nVZ5ffpbd0lzi5b3q3oI3OLDfQz0JsRgBsokk2fxwvDvNA8=",
                "58b2d2a3e12660d9cb101acd011ce2293b9f3caa3886663730b13ee3c4e96cd9" },
            { "offers", false,
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 400\n"
                "tx.seqNum: 1234567890123\n"
                "tx.cond.type: PRECOND_NONE\n"
                "tx.memo.type: MEMO_NONE\n"
                "tx.operations.len: 4\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: MANAGE_SELL_OFFER\n"
                "tx.operations[0].body.manageSellOfferOp.selling: TestXLM\n"
                "tx.operations[0].body.manageSellOfferOp.buying: USD:GDFJHLAXAUMHA4OWPOB4P7YO72AQR2HMIUYFOXLXE2DZGM633K7HZDQP\n"
                "tx.operations[0].body.manageSellOfferOp.amount: 10000000000 (1,000e7)\n"
                "tx.operations[0].body.manageSellOfferOp.price.n: 1\n"
                "tx.operations[0].body.manageSellOfferOp.price.d: 2\n"
                "tx.operations[0].body.manageSellOfferOp.offerID: 12345 (0.0012345e7)\n"
                "tx.operations[1].sourceAccount._present: false\n"
                "tx.operations[1].body.type: MANAGE_BUY_OFFER\n"
                "tx.operations[1].body.manageBuyOfferOp.selling: USD:GDFJHLAXAUMHA4OWPOB4P7YO72AQR2HMIUYFOXLXE2DZGM633K7HZDQP\n"
                "tx.operations[1].body.manageBuyOfferOp.buying: LONGASSET012:GBXHUHG5FGYLPD6RHL2MKWMP572O6KUXCZXDZJXS4T57ZTMAKBN7DWXN\n"
                "tx.operations[1].body.manageBuyOfferOp.buyAmount: 20000000000 (2,000e7)\n"
                "tx.operations[1].body.manageBuyOfferOp.price.n: 3\n"
                "tx.operations[1].body.manageBuyOfferOp.price.d: 1\n"
                "tx.operations[1].body.manageBuyOfferOp.offerID: 0 (0e7)\n"
                "tx.operations[2].sourceAccount._present: false\n"
                "tx.operations[2].body.type: CREATE_PASSIVE_SELL_OFFER\n"
                "tx.operations[2].body.createPassiveSellOfferOp.selling: EUR:GBXHUHG5FGYLPD6RHL2MKWMP572O6KUXCZXDZJXS4T57ZTMAKBN7DWXN\n"
                "tx.operations[2].body.createPassiveSellOfferOp.buying: TestXLM\n"
                "tx.operations[2].body.createPassiveSellOfferOp.amount: 10005000000 (1,000.5e7)\n"
                "tx.operations[2].body.createPassiveSellOfferOp.price.n: 2\n"
                "tx.operations[2].body.createPassiveSellOfferOp.price.d: 1\n"
                "tx.operations[3].sourceAccount._present: false\n"
                "tx.operations[3].body.type: MANAGE_SELL_OFFER\n"
                "tx.operations[3].body.manageSellOfferOp.selling: USD:GDFJHLAXAUMHA4OWPOB4P7YO72AQR2HMIUYFOXLXE2DZGM633K7HZDQP\n"
                "tx.operations[3].body.manageSellOfferOp.buying: EUR:GBXHUHG5FGYLPD6RHL2MKWMP572O6KUXCZXDZJXS4T57ZTMAKBN7DWXN\n"
                "tx.operations[3].body.manageSellOfferOp.amount: 0 (0e7)\n"
                "tx.operations[3].body.manageSellOfferOp.price.n: 1\n"
                "tx.operations[3].body.manageSellOfferOp.price.d: 1\n"
                "tx.operations[3].body.manageSellOfferOp.offerID: 9223372036854775807 (922,337,203,685.4775807e7)\n"
                "tx.ext.v: 0\n"
                "signatures.len: 1\n"
                "signatures[0].hint: b40f6f5c (GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR)\n"
                "signatures[0].signature: fc466d52cca0a93bf752119b8bbb0fe1585cd206ef17ead06e8ad1f42b479ea74866694563887d81969760dfb85bb2daf12619be5aa361d22d275f6d41fb2b08",
                "AAAAAgAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXAAAAZAAAAEfcfsEywAAAAAAAAAAAAAABAAAAAAAAAAD"
                "AAAAAAAAAAFVU0QAAAAAAMqTrBcFGHBx1nuDx/8O/oEI6OxFMFdddyaHkzPb2r58AAAAAlQL5AAAAAABAAAAAgAAAAAAADA5"
                "AAAAAAAAAAwAAAABVVNEAAAAAADKk6wXBRhwcdZ7g8f/Dv6BCOjsRTBXXXcmh5Mz29q+fAAAAAJMT05HQVNTRVQwMTIAAAAA"
                "bnoc3Smwt4/ROvTFWY/v9O8qlxZuPKby5Pv8zYBQW/EAAAAEqBfIAAAAAAMAAAABAAAAAAAAAAAAAAAAAAAABAAAAAFFVVIA"
                "AAAAAG56HN0psLeP0Tr0xVmP7/TvKpcWbjym8uT7/M2AUFvxAAAAAAAAAAJUWC9AAAAAAgAAAAEAAAAAAAAAAwAAAAFVU0QA"
                "AAAAAMqTrBcFGHBx1nuDx/8O/oEI6OxFMFdddyaHkzPb2r58AAAAAUVVUgAAAAAAbnoc3Smwt4/ROvTFWY/v9O8qlxZuPKby"
                "5Pv8zYBQW/EAAAAAAAAAAAAAAAEAAAABf/////////8AAAAAAAAAAbQPb1wAAABA/EZtUsygqTv3UhGbi7sP4Vhc0gbvF+rQ"
                "borR9CtHnqdIZmlFY4h9gZaXYN+4W7La8SYZvlqjYdItJ19tQfsrCA==",
                "40b69149027b2eb79fb16a6492e2f9cca5e9a95fe01775e0791e8012e168cd8d" },
            { "options_data", false,
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 700\n"
                "tx.seqNum: 1234567890123\n"
                "tx.cond.type: PRECOND_NONE\n"
                "tx.memo.type: MEMO_ID\n"
                "tx.memo.id: 18446744073709551615\n"
                "tx.operations.len: 7\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: SET_OPTIONS\n"
                "tx.operations[0].body.setOptionsOp.inflationDest._present: true\n"
                "tx.operations[0].body.setOptionsOp.inflationDest: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[0].body.setOptionsOp.clearFlags._present: true\n"
                "tx.operations[0].body.setOptionsOp.clearFlags: 2\n"
                "tx.operations[0].body.setOptionsOp.setFlags._present: true\n"
                "tx.operations[0].body.setOptionsOp.setFlags: 1\n"
                "tx.operations[0].body.setOptionsOp.masterWeight._present: true\n"
                "tx.operations[0].body.setOptionsOp.masterWeight: 255\n"
                "tx.operations[0].body.setOptionsOp.lowThreshold._present: true\n"
                "tx.operations[0].body.setOptionsOp.lowThreshold: 1\n"
                "tx.operations[0].body.setOptionsOp.medThreshold._present: true\n"
                "tx.operations[0].body.setOptionsOp.medThreshold: 2\n"
                "tx.operations[0].body.setOptionsOp.highThreshold._present: true\n"
                "tx.operations[0].body.setOptionsOp.highThreshold: 3\n"
                "tx.operations[0].body.setOptionsOp.homeDomain._present: true\n"
                "tx.operations[0].body.setOptionsOp.homeDomain: \"example.com\"\n"
                "tx.operations[0].body.setOptionsOp.signer._present: true\n"
                "tx.operations[0].body.setOptionsOp.signer.key: GDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435DIAG\n"
                "tx.operations[0].body.setOptionsOp.signer.weight: 1\n"
                "tx.operations[1].sourceAccount._present: false\n"
                "tx.operations[1].body.type: MANAGE_DATA\n"
                "tx.operations[1].body.manageDataOp.dataName: \"k\"\n"
                "tx.operations[1].body.manageDataOp.dataValue._present: true\n"
                "tx.operations[1].body.manageDataOp.dataValue: 76\n"
                "tx.operations[2].sourceAccount._present: false\n"
                "tx.operations[2].body.type: MANAGE_DATA\n"
                "tx.operations[2].body.manageDataOp.dataName: \"gone\"\n"
                "tx.operations[2].body.manageDataOp.dataValue._present: false\n"
                "tx.operations[3].sourceAccount._present: false\n"
                "tx.operations[3].body.type: BUMP_SEQUENCE\n"
                "tx.operations[3].body.bumpSequenceOp.bumpTo: 1234567890999\n"
                "tx.operations[4].sourceAccount._present: false\n"
                "tx.operations[4].body.type: SET_TRUST_LINE_FLAGS\n"
                "tx.operations[4].body.setTrustLineFlagsOp.trustor: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[4].body.setTrustLineFlagsOp.asset: USD:GDFJHLAXAUMHA4OWPOB4P7YO72AQR2HMIUYFOXLXE2DZGM633K7HZDQP\n"
                "tx.operations[4].body.setTrustLineFlagsOp.clearFlags: 1\n"
                "tx.operations[4].body.setTrustLineFlagsOp.setFlags: 4\n"
                "tx.operations[5].sourceAccount._present: false\n"
                "tx.operations[5].body.type: BEGIN_SPONSORING_FUTURE_RESERVES\n"
                "tx.operations[5].body.beginSponsoringFutureReservesOp.sponsoredID: GDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435DIAG\n"
                "tx.operations[6].sourceAccount._present: true\n"
                "tx.operations[6].sourceAccount: GDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435DIAG\n"
                "tx.operations[6].body.type: END_SPONSORING_FUTURE_RESERVES\n"
                "tx.ext.v: 0\n"
                "signatures.len: 1\n"
                "signatures[0].hint: b40f6f5c (GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR)\n"
                "signatures[0].signature: 52062f42269dcf075f65308e7b0a6ead84bae75b18daa63594fcd393dee014e292bd453baa3e743728ce29d5df22fb70b6559c766db2104d2d5915f8158dbe0e",
                "AAAAAgAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXAAAArwAAAEfcfsEywAAAAAAAAAC//////////8AAAAH"
                "AAAAAAAAAAUAAAABAAAAAIE5dw6ofRdfVqNUZsNMfszLjYqRtO43ol32D1uPybOUAAAAAQAAAAIAAAABAAAAAQAAAAEAAAD/"
                "AAAAAQAAAAEAAAABAAAAAgAAAAEAAAADAAAAAQAAAAtleGFtcGxlLmNvbQAAAAABAAAAAO1JKMYo0cLG6ukDOJBZlWEpWSc6"
                "XGP5NjbBRhSshzfRAAAAAQAAAAAAAAAKAAAAAWsAAAAAAAABAAAAAXYAAAAAAAAAAAAACgAAAARnb25lAAAAAAAAAAAAAAAL"
                "AAABH3H7CDcAAAAAAAAAFQAAAACBOXcOqH0XX1ajVGbDTH7My42KkbTuN6Jd9g9bj8mzlAAAAAFVU0QAAAAAAMqTrBcFGHBx"
                "1nuDx/8O/oEI6OxFMFdddyaHkzPb2r58AAAAAQAAAAQAAAAAAAAAEAAAAADtSSjGKNHCxurpAziQWZVhKVknOlxj+TY2wUYU"
                "rIc30QAAAAEAAAAA7UkoxijRwsbq6QM4kFmVYSlZJzpcY/k2NsFGFKyHN9EAAAARAAAAAAAAAAG0D29cAAAAQFIGL0Imnc8H"
                "X2UwjnsKbq2EuudbGNqmNZT805Pe4BTikr1FO6o+dDcozinV3yL7cLZVnHZtshBNLVkV+BWNvg4=",
                "9bbcc754e64f3268617944fa19644762c27ec02caf37c1f3bbd5fa909ae156e2" },
            { "pools", false,
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 200\n"
                "tx.seqNum: 1234567890123\n"
                "tx.cond.type: PRECOND_NONE\n"
                "tx.memo.type: MEMO_HASH\n"
                "tx.memo.hash: 000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f\n"
                "tx.operations.len: 2\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: LIQUIDITY_POOL_DEPOSIT\n"
                "tx.operations[0].body.liquidityPoolDepositOp.liquidityPoolID: 2c2c2c2c2c2c2c2c2c2c2c2c2c2c2c2c2c2c2c2c2c2c2c2c2c2c2c2c2c2c2c2c\n"
                "tx.operations[0].body.liquidityPoolDepositOp.maxAmountA: 10000000000 (1,000e7)\n"
                "tx.operations[0].body.liquidityPoolDepositOp.maxAmountB: 20000000000 (2,000e7)\n"
                "tx.operations[0].body.liquidityPoolDepositOp.minPrice.n: 1\n"
                "tx.operations[0].body.liquidityPoolDepositOp.minPrice.d: 2\n"
                "tx.operations[0].body.liquidityPoolDepositOp.maxPrice.n: 2\n"
                "tx.operations[0].body.liquidityPoolDepositOp.maxPrice.d: 1\n"
                "tx.operations[1].sourceAccount._present: false\n"
                "tx.operations[1].body.type: LIQUIDITY_POOL_WITHDRAW\n"
                "tx.operations[1].body.liquidityPoolWithdrawOp.liquidityPoolID: 2c2c2c2c2c2c2c2c2c2c2c2c2c2c2c2c2c2c2c2c2c2c2c2c2c2c2c2c2c2c2c2c\n"
                "tx.operations[1].body.liquidityPoolWithdrawOp.amount: 15000000000 (1,500e7)\n"
                "tx.operations[1].body.liquidityPoolWithdrawOp.minAmountA: 10000000000 (1,000e7)\n"
                "tx.operations[1].body.liquidityPoolWithdrawOp.minAmountB: 0 (0e7)\n"
                "tx.ext.v: 0\n"
                "signatures.len: 1\n"
                "signatures[0].hint: b40f6f5c (GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR)\n"
                "signatures[0].signature: 2fdada0b99a168fe1a28ab8a46a7d237d13be1198b4be9058edeeb79e9a2d1e14107b7618be213dddd2805341513b7f0a64145d9c0183265409974050d0ccc0f",
                "AAAAAgAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXAAAAMgAAAEfcfsEywAAAAAAAAADAAECAwQFBgcICQoL"
                "DA0ODxAREhMUFRYXGBkaGxwdHh8AAAACAAAAAAAAABYsLCwsLCwsLCwsLCwsLCwsLCwsLCwsLCwsLCwsLCwsLAAAAAJUC+QA"
                "AAAABKgXyAAAAAABAAAAAgAAAAIAAAABAAAAAAAAABcsLCwsLCwsLCwsLCwsLCwsLCwsLCwsLCwsLCwsLCwsLAAAAAN+EdYA"
                "AAAAAlQL5AAAAAAAAAAAAAAAAAAAAAABtA9vXAAAAEAv2toLmaFo/hooq4pGp9I30TvhGYtL6QWO3ut56aLR4UEHt2GL4hPd"
                "3SgFNBUTt/CmQUXZwBgyZUCZdAUNDMwP",
                "f9b5f5a22eb38f4f62e962ae145a9aa80d8c85726a61f6dd9d9787c4f90c9b35" },
            { "revokes", false,
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 500\n"
                "tx.seqNum: 1234567890123\n"
                "tx.cond.type: PRECOND_NONE\n"
                "tx.memo.type: MEMO_RETURN\n"
                "tx.memo.retHash: 202122232425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f\n"
                "tx.operations.len: 5\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: REVOKE_SPONSORSHIP\n"
                "tx.operations[0].body.revokeSponsorshipOp.type: REVOKE_SPONSORSHIP_LEDGER_ENTRY\n"
                "tx.operations[0].body.revokeSponsorshipOp.ledgerKey.type: ACCOUNT\n"
                "tx.operations[0].body.revokeSponsorshipOp.ledgerKey.account.accountID: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[1].sourceAccount._present: false\n"
                "tx.operations[1].body.type: REVOKE_SPONSORSHIP\n"
                "tx.operations[1].body.revokeSponsorshipOp.type: REVOKE_SPONSORSHIP_LEDGER_ENTRY\n"
                "tx.operations[1].body.revokeSponsorshipOp.ledgerKey.type: TRUSTLINE\n"
                "tx.operations[1].body.revokeSponsorshipOp.ledgerKey.trustLine.accountID: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[1].body.revokeSponsorshipOp.ledgerKey.trustLine.asset: USD:GDFJHLAXAUMHA4OWPOB4P7YO72AQR2HMIUYFOXLXE2DZGM633K7HZDQP\n"
                "tx.operations[2].sourceAccount._present: false\n"
                "tx.operations[2].body.type: REVOKE_SPONSORSHIP\n"
                "tx.operations[2].body.revokeSponsorshipOp.type: REVOKE_SPONSORSHIP_LEDGER_ENTRY\n"
                "tx.operations[2].body.revokeSponsorshipOp.ledgerKey.type: OFFER\n"
                "tx.operations[2].body.revokeSponsorshipOp.ledgerKey.offer.sellerID: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[2].body.revokeSponsorshipOp.ledgerKey.offer.offerID: 12345 (0.0012345e7)\n"
                "tx.operations[3].sourceAccount._present: false\n"
                "tx.operations[3].body.type: REVOKE_SPONSORSHIP\n"
                "tx.operations[3].body.revokeSponsorshipOp.type: REVOKE_SPONSORSHIP_LEDGER_ENTRY\n"
                "tx.operations[3].body.revokeSponsorshipOp.ledgerKey.type: CLAIMABLE_BALANCE\n"
                "tx.operations[3].body.revokeSponsorshipOp.ledgerKey.claimableBalance.balanceID.type: CLAIMABLE_BALANCE_ID_TYPE_V0\n"
                "tx.operations[3].body.revokeSponsorshipOp.ledgerKey.claimableBalance.balanceID.v0: abababababababababababababababababababababababababababababababab\n"
                "tx.operations[4].sourceAccount._present: false\n"
                "tx.operations[4].body.type: REVOKE_SPONSORSHIP\n"
                "tx.operations[4].body.revokeSponsorshipOp.type: REVOKE_SPONSORSHIP_SIGNER\n"
                "tx.operations[4].body.revokeSponsorshipOp.signer.accountID: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[4].body.revokeSponsorshipOp.signer.signerKey: GDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435DIAG\n"
                "tx.ext.v: 0\n"
                "signatures.len: 1\n"
                "signatures[0].hint: b40f6f5c (GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR)\n"
                "signatures[0].signature: ec8a3647942669ffea5f82d4111f2ae9474e730ffd7e4aa43f6449a4a1d1eceb9532d40a028b3ec5c31418939bbdf093fa6483bb97505c490751ad9603d30100",
                "AAAAAgAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXAAAAfQAAAEfcfsEywAAAAAAAAAEICEiIyQlJicoKSor"
                "LC0uLzAxMjM0NTY3ODk6Ozw9Pj8AAAAFAAAAAAAAABIAAAAAAAAAAAAAAACBOXcOqH0XX1ajVGbDTH7My42KkbTuN6Jd9g9b"
                "j8mzlAAAAAAAAAASAAAAAAAAAAEAAAAAgTl3Dqh9F19Wo1Rmw0x+zMuNipG07jeiXfYPW4/Js5QAAAABVVNEAAAAAADKk6wX"
                "BRhwcdZ7g8f/Dv6BCOjsRTBXXXcmh5Mz29q+fAAAAAAAAAASAAAAAAAAAAIAAAAAgTl3Dqh9F19Wo1Rmw0x+zMuNipG07jei"
                "XfYPW4/Js5QAAAAAAAAwOQAAAAAAAAASAAAAAAAAAAQAAAAAq6urq6urq6urq6urq6urq6urq6urq6urq6urq6urq6sAAAAA"
                "AAAAEgAAAAEAAAAAgTl3Dqh9F19Wo1Rmw0x+zMuNipG07jeiXfYPW4/Js5QAAAAA7UkoxijRwsbq6QM4kFmVYSlZJzpcY/k2"
                "NsFGFKyHN9EAAAAAAAAAAbQPb1wAAABA7Io2R5Qmaf/qX4LUER8q6UdOcw/9fkqkP2RJpKHR7OuVMtQKAos+xcMUGJObvfCT"
                "+mSDu5dQXEkHUa2WA9MBAA==",
                "78695df081682a84d07cb3816930dde1f39fd7cd156ee40238ac68d821631526" },
            { "v0_untimed", false,
                "type: ENVELOPE_TYPE_TX_V0\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 100\n"
                "tx.seqNum: 1234567890123\n"
                "tx.timeBounds._present: false\n"
                "tx.memo.type: MEMO_NONE\n"
                "tx.operations.len: 1\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: BUMP_SEQUENCE\n"
                "tx.operations[0].body.bumpSequenceOp.bumpTo: 7\n"
                "tx.ext.v: 0\n"
                "signatures.len: 1\n"
                "signatures[0].hint: b40f6f5c (GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR)\n"
                "signatures[0].signature: 95736e29cf416378aa3c24aac4807adbfd1e580d8e9fd6839b7f768a46bd553e40c5478d33374f9f494d778fed0e2435cbfadb74f3bdf8295a7cdce40f2d7800",
                "AAAAAIqI4910CfGV/VLbLTy6XXLKZwm/HZQSG/N0iAG0D29cAAAAZAAAAR9x+wTLAAAAAAAAAAAAAAABAAAAAAAAAAsAAAAA"
                "AAAABwAAAAAAAAABtA9vXAAAAECVc24pz0FjeKo8JKrEgHrb/R5YDY6f1oObf3aKRr1VPkDFR40zN0+fSU13j+0OJDXL+tt0"
                "8734KVp83OQPLXgA",
                "dc1e078d707bf0e68ab71b7832593b41c68dc9303ca2fdf96b519fc0b7c34026" },
            { "signers_x_p", false,
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 100\n"
                "tx.seqNum: 1234567890123\n"
                "tx.cond.type: PRECOND_TIME\n"
                "tx.cond.timeBounds.minTime: 0\n"
                "tx.cond.timeBounds.maxTime: 1800000000 (Fri Jan 15 08:00:00 UTC 2027)\n"
                "tx.memo.type: MEMO_NONE\n"
                "tx.operations.len: 1\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: PAYMENT\n"
                "tx.operations[0].body.paymentOp.destination: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[0].body.paymentOp.asset: TestXLM\n"
                "tx.operations[0].body.paymentOp.amount: 10000000000 (1,000e7)\n"
                "tx.ext.v: 0\n"
                "signatures.len: 3\n"
                "signatures[0].hint: b40f6f5c (GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR)\n"
                "signatures[0].signature: 18a309578fef5385abab0efc9f2e9696d4d5af7516dbe02cdfd72ef2c190524dd954e955ba50a7634fbb37bdf52faf0d4aecebde76fea36c465776d15c0ac403\n"
                "signatures[1].hint: 8ff0c80f (XBOEAYOXRWI6ABHPKTWSFWIT3J2JDOFSVX6SHR677YMKOWEP6DEA6CWO)\n"
                "signatures[1].signature: 73746320707265696d616765\n"
                "signatures[2].hint: eaa5dffb (PCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZIAAAAACWQZLMNRXQAAAAMGTA)\n"
                "signatures[2].signature: 6d8917c1f25b002129e0bc390ec18f58ca859a2f84459deff3151d5bb11dfec5806f7c321fda77611a59bea4c4f56d8d8813290e85faeb4bda05293c34376d0e",
                "AAAAAgAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXAAAAGQAAAEfcfsEywAAAAEAAAAAAAAAAAAAAABrSdIA"
                "AAAAAAAAAAEAAAAAAAAAAQAAAACBOXcOqH0XX1ajVGbDTH7My42KkbTuN6Jd9g9bj8mzlAAAAAAAAAACVAvkAAAAAAAAAAAD"
                "tA9vXAAAAEAYowlXj+9ThaurDvyfLpaW1NWvdRbb4Czf1y7ywZBSTdlU6VW6UKdjT7s3vfUvrw1K7Ovedv6jbEZXdtFcCsQD"
                "j/DIDwAAAAxzdGMgcHJlaW1hZ2Xqpd/7AAAAQG2JF8HyWwAhKeC8OQ7Bj1jKhZovhEWd7/MVHVuxHf7FgG98Mh/ad2EaWb6k"
                "xPVtjYgTKQ6F+utL2gUpPDQ3bQ4=",
                "0c4b68c900d80518cd7da8905bf70fbffa04a9b3044b6c8a61c0d9c7a62972f8" },
            { "v2_cond", false,
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 100\n"
                "tx.seqNum: 1234567890123\n"
                "tx.cond.type: PRECOND_V2\n"
                "tx.cond.v2.timeBounds._present: true\n"
                "tx.cond.v2.timeBounds.minTime: 1700000000 (Tue Nov 14 22:13:20 UTC 2023)\n"
                "tx.cond.v2.timeBounds.maxTime: 1800000000 (Fri Jan 15 08:00:00 UTC 2027)\n"
                "tx.cond.v2.ledgerBounds._present: true\n"
                "tx.cond.v2.ledgerBounds.minLedger: 7\n"
                "tx.cond.v2.ledgerBounds.maxLedger: 8\n"
                "tx.cond.v2.minSeqNum._present: true\n"
                "tx.cond.v2.minSeqNum: 9\n"
                "tx.cond.v2.minSeqAge: 10\n"
                "tx.cond.v2.minSeqLedgerGap: 11\n"
                "tx.cond.v2.extraSigners.len: 1\n"
                "tx.cond.v2.extraSigners[0]: GDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435DIAG\n"
                "tx.memo.type: MEMO_NONE\n"
                "tx.operations.len: 1\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: PAYMENT\n"
                "tx.operations[0].body.paymentOp.destination: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[0].body.paymentOp.asset: USD:GDFJHLAXAUMHA4OWPOB4P7YO72AQR2HMIUYFOXLXE2DZGM633K7HZDQP\n"
                "tx.operations[0].body.paymentOp.amount: 10000000000 (1,000e7)\n"
                "tx.ext.v: 0\n"
                "signatures.len: 2\n"
                "signatures[0].hint: b40f6f5c (GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR)\n"
                "signatures[0].signature: 0e5a78ede12c60abf04601403bb481986b1d6727df31607152b69a1a7a26f3568aefc0716d08fda4b1cc99c95edd8d33968f25e8dea5028c8cf61d742ea66b08\n"
                "signatures[1].hint: ac8737d1 (GDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435DIAG)\n"
                "signatures[1].signature: 11711419d63bcfa26c8dfc612349cb9d7d44b28431d64271d0a129684f11d6409298205c3abed886300f54a36b232d67e0500a0479034b2ca0e16d89d18d8207",
                "AAAAAgAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXAAAAGQAAAEfcfsEywAAAAIAAAABAAAAAGVT8QAAAAAA"
                "a0nSAAAAAAEAAAAHAAAACAAAAAEAAAAAAAAACQAAAAAAAAAKAAAACwAAAAEAAAAA7UkoxijRwsbq6QM4kFmVYSlZJzpcY/k2"
                "NsFGFKyHN9EAAAAAAAAAAQAAAAAAAAABAAAAAIE5dw6ofRdfVqNUZsNMfszLjYqRtO43ol32D1uPybOUAAAAAVVTRAAAAAAA"
                "ypOsFwUYcHHWe4PH/w7+gQjo7EUwV113JoeTM9vavnwAAAACVAvkAAAAAAAAAAACtA9vXAAAAEAOWnjt4Sxgq/BGAUA7tIGY"
                "ax1nJ98xYHFStpoaeibzVorvwHFtCP2kscyZyV7djTOWjyXo3qUCjIz2HXQupmsIrIc30QAAAEARcRQZ1jvPomyN/GEjScud"
                "fUSyhDHWQnHQoSloTxHWQJKYIFw6vtiGMA9Uo2sjLWfgUAoEeQNLLKDhbYnRjYIH",
                "905865086a33db14f8c7f0d7041f5a9aec5ac191b567e5c9b416608ca7bdc27a" },
            { "v0_signed", false,
                "type: ENVELOPE_TYPE_TX_V0\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 100\n"
                "tx.seqNum: 1234567890123\n"
                "tx.timeBounds._present: true\n"
                "tx.timeBounds.minTime: 1700000000 (Tue Nov 14 22:13:20 UTC 2023)\n"
                "tx.timeBounds.maxTime: 1800000000 (Fri Jan 15 08:00:00 UTC 2027)\n"
                "tx.memo.type: MEMO_NONE\n"
                "tx.operations.len: 1\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: PAYMENT\n"
                "tx.operations[0].body.paymentOp.destination: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[0].body.paymentOp.asset: TestXLM\n"
                "tx.operations[0].body.paymentOp.amount: 10000000000 (1,000e7)\n"
                "tx.ext.v: 0\n"
                "signatures.len: 2\n"
                "signatures[0].hint: b40f6f5c (GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR)\n"
                "signatures[0].signature: ede7eaa7cbe850476fb68840f54ed0143e7ee4c709d452704468b8482d177c953e12c66414a5c7cc24dcd3e287537fe22889223930ffa170488edd802b61f302\n"
                "signatures[1].hint: 8fc9b394 (GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U)\n"
                "signatures[1].signature: cd93817b6e111b813fa024f9af269de08e3a592db1a655ca4f011d673e8ba98dc205d23ac42e85607c16981cae85ddac1095f81517c12463fedc8ce867ca5e0a",
                "AAAAAIqI4910CfGV/VLbLTy6XXLKZwm/HZQSG/N0iAG0D29cAAAAZAAAAR9x+wTLAAAAAQAAAABlU/EAAAAAAGtJ0gAAAAAA"
                "AAAAAQAAAAAAAAABAAAAAIE5dw6ofRdfVqNUZsNMfszLjYqRtO43ol32D1uPybOUAAAAAAAAAAJUC+QAAAAAAAAAAAK0D29c"
                "AAAAQO3n6qfL6FBHb7aIQPVO0BQ+fuTHCdRScERouEgtF3yVPhLGZBSlx8wk3NPih1N/4iiJIjkw/6FwSI7dgCth8wKPybOU"
                "AAAAQM2TgXtuERuBP6Ak+a8mneCOOlktsaZVyk8BHWc+i6mNwgXSOsQuhWB8FpgcroXdrBCV+BUXwSRj/tyM6GfKXgo=",
                "b68fe2d389cbf0b3c287640d773a65457237101b511e0bd43e47c7f43378f5c5" },
            { "fee_bump", false,
                "type: ENVELOPE_TYPE_TX_FEE_BUMP\n"
                "feeBump.tx.feeSource: MCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZIAAAAAAAAAAAAHZ7W\n"
                "feeBump.tx.fee: 400 (0.00004e7)\n"
                "feeBump.tx.innerTx.type: ENVELOPE_TYPE_TX\n"
                "feeBump.tx.innerTx.tx.sourceAccount: MCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYABAAAAAAAAAAFPM6\n"
                "feeBump.tx.innerTx.tx.fee: 100\n"
                "feeBump.tx.innerTx.tx.seqNum: 1234567890123\n"
                "feeBump.tx.innerTx.tx.cond.type: PRECOND_TIME\n"
                "feeBump.tx.innerTx.tx.cond.timeBounds.minTime: 1700000000 (Tue Nov 14 22:13:20 UTC 2023)\n"
                "feeBump.tx.innerTx.tx.cond.timeBounds.maxTime: 1800000000 (Fri Jan 15 08:00:00 UTC 2027)\n"
                "feeBump.tx.innerTx.tx.memo.type: MEMO_NONE\n"
                "feeBump.tx.innerTx.tx.operations.len: 1\n"
                "feeBump.tx.innerTx.tx.operations[0].sourceAccount._present: false\n"
                "feeBump.tx.innerTx.tx.operations[0].body.type: PAYMENT\n"
                "feeBump.tx.innerTx.tx.operations[0].body.paymentOp.destination: GDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435DIAG\n"
                "feeBump.tx.innerTx.tx.operations[0].body.paymentOp.asset: TestXLM\n"
                "feeBump.tx.innerTx.tx.operations[0].body.paymentOp.amount: 100000000000 (10,000e7)\n"
                "feeBump.tx.innerTx.tx.ext.v: 0\n"
                "feeBump.tx.innerTx.signatures.len: 1\n"
                "feeBump.tx.innerTx.signatures[0].hint: b40f6f5c (bad signature/unknown key/test is wrong network)\n"
                "feeBump.tx.innerTx.signatures[0].signature: d65ac986e8427c0132d7af895d9887f3e05175c6c97cf6983adb5e500aa4eb732b10f8d7b0c1f5f1da81acee75214335644c811396c28e4a8fc055b265019c06\n"
                "feeBump.tx.ext.v: 0\n"
                "feeBump.signatures.len: 1\n"
                "feeBump.signatures[0].hint: 8fc9b394 (GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U)\n"
                "feeBump.signatures[0].signature: 7b61da333dd622ef8fe90f1d941aed823244174459ca3384d0ee2d89fa6b2ae7c6d4056823f04ad96e4b95e3910ee921db349ca1416ed88979dd33ff94d5d300",
                "AAAABQAAAQAAAAAAAAAAAYE5dw6ofRdfVqNUZsNMfszLjYqRtO43ol32D1uPybOUAAAAAAAAAZAAAAACAAABAAAgAAAAAAAB"
                "iojj3XQJ8ZX9UtstPLpdcspnCb8dlBIb83SIAbQPb1wAAABkAAABH3H7BMsAAAABAAAAAGVT8QAAAAAAa0nSAAAAAAAAAAAB"
                "AAAAAAAAAAEAAAAA7UkoxijRwsbq6QM4kFmVYSlZJzpcY/k2NsFGFKyHN9EAAAAAAAAAF0h26AAAAAAAAAAAAbQPb1wAAABA"
                "1lrJhuhCfAEy16+JXZiH8+BRdcbJfPaYOtteUAqk63MrEPjXsMH18dqBrO51IUM1ZEyBE5bCjkqPwFWyZQGcBgAAAAAAAAAB"
                "j8mzlAAAAEB7YdozPdYi74/pDx2UGu2CMkQXRFnKM4TQ7i2J+msq58bUBWgj8ErZbkuV45EO6SHbNJyhQW7YiXndM/+U1dMA",
                "75cfb12055d97b44d451ea26afcd151fdf60c3113da7f580e19bc96082a8498f" },
            { "payments_big_unmatched", false,
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 500\n"
                "tx.seqNum: 1234567890123\n"
                "tx.cond.type: PRECOND_TIME\n"
                "tx.cond.timeBounds.minTime: 1700000000 (Tue Nov 14 22:13:20 UTC 2023)\n"
                "tx.cond.timeBounds.maxTime: 1800000000 (Fri Jan 15 08:00:00 UTC 2027)\n"
                "tx.memo.type: MEMO_NONE\n"
                "tx.operations.len: 5\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: PAYMENT\n"
                "tx.operations[0].body.paymentOp.destination: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[0].body.paymentOp.asset: TestXLM\n"
                "tx.operations[0].body.paymentOp.amount: 10000000000 (1,000e7)\n"
                "tx.operations[1].sourceAccount._present: false\n"
                "tx.operations[1].body.type: PAYMENT\n"
                "tx.operations[1].body.paymentOp.destination: GDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435DIAG\n"
                "tx.operations[1].body.paymentOp.asset: USD:GDFJHLAXAUMHA4OWPOB4P7YO72AQR2HMIUYFOXLXE2DZGM633K7HZDQP\n"
                "tx.operations[1].body.paymentOp.amount: 123456789012 (12,345.6789012e7)\n"
                "tx.operations[2].sourceAccount._present: false\n"
                "tx.operations[2].body.type: PAYMENT\n"
                "tx.operations[2].body.paymentOp.destination: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[2].body.paymentOp.asset: LONGASSET012:GBXHUHG5FGYLPD6RHL2MKWMP572O6KUXCZXDZJXS4T57ZTMAKBN7DWXN\n"
                "tx.operations[2].body.paymentOp.amount: 9223372036854775807 (922,337,203,685.4775807e7)\n"
                "tx.operations[3].sourceAccount._present: false\n"
                "tx.operations[3].body.type: PAYMENT\n"
                "tx.operations[3].body.paymentOp.destination: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[3].body.paymentOp.asset: TestXLM\n"
                "tx.operations[3].body.paymentOp.amount: 1 (0.0000001e7)\n"
                "tx.operations[4].sourceAccount._present: false\n"
                "tx.operations[4].body.type: CREATE_ACCOUNT\n"
                "tx.operations[4].body.createAccountOp.destination: GDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435DIAG\n"
                "tx.operations[4].body.createAccountOp.startingBalance: 10000000000000 (1,000,000e7)\n"
                "tx.ext.v: 0\n"
                "signatures.len: 1\n"
                "signatures[0].hint: b40f6f5c (bad signature/unknown key/test is wrong network)\n"
                "signatures[0].signature: 1f1a0da35d684b28e51aba77ec359d7306dd0956cce5bd902bd9a1589093f2aafc05d48ae29c4035e2e29c488865ba51aa647727a85cc37b03ba4f4144e5e404",
                "AAAAAgAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXAAAAfQAAAEfcfsEywAAAAEAAAAAZVPxAAAAAABrSdIA"
                "AAAAAAAAAAUAAAAAAAAAAQAAAACBOXcOqH0XX1ajVGbDTH7My42KkbTuN6Jd9g9bj8mzlAAAAAAAAAACVAvkAAAAAAAAAAAB"
                "AAAAAO1JKMYo0cLG6ukDOJBZlWEpWSc6XGP5NjbBRhSshzfRAAAAAVVTRAAAAAAAypOsFwUYcHHWe4PH/w7+gQjo7EUwV113"
                "JoeTM9vavnwAAAAcvpkaFAAAAAAAAAABAAAAAIE5dw6ofRdfVqNUZsNMfszLjYqRtO43ol32D1uPybOUAAAAAkxPTkdBU1NF"
                "VDAxMgAAAABuehzdKbC3j9E69MVZj+/07yqXFm48pvLk+/zNgFBb8X//////////AAAAAAAAAAEAAAAAgTl3Dqh9F19Wo1Rm"
                "w0x+zMuNipG07jeiXfYPW4/Js5QAAAAAAAAAAAAAAAEAAAAAAAAAAAAAAADtSSjGKNHCxurpAziQWZVhKVknOlxj+TY2wUYU"
                "rIc30QAACRhOcqAAAAAAAAAAAAG0D29cAAAAQB8aDaNdaEso5Rq6d+w1nXMG3QlWzOW9kCvZoViQk/Kq/AXUiuKcQDXi4pxI"
                "iGW6UapkdyeoXMN7A7pPQUTl5AQ=",
                "aefab24d24618e2a10f39c878a6a704200c0b7e9c9879f688ad279358c84e68a" },
            { "payments_big_public", true,
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 500\n"
                "tx.seqNum: 1234567890123\n"
                "tx.cond.type: PRECOND_TIME\n"
                "tx.cond.timeBounds.minTime: 1700000000 (Tue Nov 14 22:13:20 UTC 2023)\n"
                "tx.cond.timeBounds.maxTime: 1800000000 (Fri Jan 15 08:00:00 UTC 2027)\n"
                "tx.memo.type: MEMO_NONE\n"
                "tx.operations.len: 5\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: PAYMENT\n"
                "tx.operations[0].body.paymentOp.destination: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[0].body.paymentOp.asset: XLM\n"
                "tx.operations[0].body.paymentOp.amount: 10000000000 (1,000e7)\n"
                "tx.operations[1].sourceAccount._present: false\n"
                "tx.operations[1].body.type: PAYMENT\n"
                "tx.operations[1].body.paymentOp.destination: GDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435DIAG\n"
                "tx.operations[1].body.paymentOp.asset: USD:GDFJHLAXAUMHA4OWPOB4P7YO72AQR2HMIUYFOXLXE2DZGM633K7HZDQP\n"
                "tx.operations[1].body.paymentOp.amount: 123456789012 (12,345.6789012e7)\n"
                "tx.operations[2].sourceAccount._present: false\n"
                "tx.operations[2].body.type: PAYMENT\n"
                "tx.operations[2].body.paymentOp.destination: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[2].body.paymentOp.asset: LONGASSET012:GBXHUHG5FGYLPD6RHL2MKWMP572O6KUXCZXDZJXS4T57ZTMAKBN7DWXN\n"
                "tx.operations[2].body.paymentOp.amount: 9223372036854775807 (922,337,203,685.4775807e7)\n"
                "tx.operations[3].sourceAccount._present: false\n"
                "tx.operations[3].body.type: PAYMENT\n"
                "tx.operations[3].body.paymentOp.destination: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[3].body.paymentOp.asset: XLM\n"
                "tx.operations[3].body.paymentOp.amount: 1 (0.0000001e7)\n"
                "tx.operations[4].sourceAccount._present: false\n"
                "tx.operations[4].body.type: CREATE_ACCOUNT\n"
                "tx.operations[4].body.createAccountOp.destination: GDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435DIAG\n"
                "tx.operations[4].body.createAccountOp.startingBalance: 10000000000000 (1,000,000e7)\n"
                "tx.ext.v: 0\n"
                "signatures.len: 1\n"
                "signatures[0].hint: b40f6f5c (GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR)\n"
                "signatures[0].signature: 92de6d3c4af6207fb8900157cc2dd82e354bcad8ea7fac252a8394cc2718b7d12207f6699c4dc65455c23eb9d2b1c6a335d14271278e0cc2141aa43f4f92630a",
                "AAAAAgAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXAAAAfQAAAEfcfsEywAAAAEAAAAAZVPxAAAAAABrSdIA"
                "AAAAAAAAAAUAAAAAAAAAAQAAAACBOXcOqH0XX1ajVGbDTH7My42KkbTuN6Jd9g9bj8mzlAAAAAAAAAACVAvkAAAAAAAAAAAB"
                "AAAAAO1JKMYo0cLG6ukDOJBZlWEpWSc6XGP5NjbBRhSshzfRAAAAAVVTRAAAAAAAypOsFwUYcHHWe4PH/w7+gQjo7EUwV113"
                "JoeTM9vavnwAAAAcvpkaFAAAAAAAAAABAAAAAIE5dw6ofRdfVqNUZsNMfszLjYqRtO43ol32D1uPybOUAAAAAkxPTkdBU1NF"
                "VDAxMgAAAABuehzdKbC3j9E69MVZj+/07yqXFm48pvLk+/zNgFBb8X//////////AAAAAAAAAAEAAAAAgTl3Dqh9F19Wo1Rm"
                "w0x+zMuNipG07jeiXfYPW4/Js5QAAAAAAAAAAAAAAAEAAAAAAAAAAAAAAADtSSjGKNHCxurpAziQWZVhKVknOlxj+TY2wUYU"
                "rIc30QAACRhOcqAAAAAAAAAAAAG0D29cAAAAQJLebTxK9iB/uJABV8wt2C41S8rY6n+sJSqDlMwnGLfRIgf2aZxNxlRVwj65"
                "0rHGozXRQnEnjgzCFBqkP0+SYwo=",
                "9e7e3c2e5b21fb59f1104666eba1d3c3163674b3ee76d368453ece7ae85fb6ea" },
            { "payments_big_plus0100", false,
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 500\n"
                "tx.seqNum: 1234567890123\n"
                "tx.cond.type: PRECOND_TIME\n"
                "tx.cond.timeBounds.minTime: 1700000000 (Tue Nov 14 23:13:20 +0100 2023)\n"
                "tx.cond.timeBounds.maxTime: 1800000000 (Fri Jan 15 09:00:00 +0100 2027)\n"
                "tx.memo.type: MEMO_NONE\n"
                "tx.operations.len: 5\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: PAYMENT\n"
                "tx.operations[0].body.paymentOp.destination: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[0].body.paymentOp.asset: TestXLM\n"
                "tx.operations[0].body.paymentOp.amount: 10000000000 (1,000e7)\n"
                "tx.operations[1].sourceAccount._present: false\n"
                "tx.operations[1].body.type: PAYMENT\n"
                "tx.operations[1].body.paymentOp.destination: GDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435DIAG\n"
                "tx.operations[1].body.paymentOp.asset: USD:GDFJHLAXAUMHA4OWPOB4P7YO72AQR2HMIUYFOXLXE2DZGM633K7HZDQP\n"
                "tx.operations[1].body.paymentOp.amount: 123456789012 (12,345.6789012e7)\n"
                "tx.operations[2].sourceAccount._present: false\n"
                "tx.operations[2].body.type: PAYMENT\n"
                "tx.operations[2].body.paymentOp.destination: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[2].body.paymentOp.asset: LONGASSET012:GBXHUHG5FGYLPD6RHL2MKWMP572O6KUXCZXDZJXS4T57ZTMAKBN7DWXN\n"
                "tx.operations[2].body.paymentOp.amount: 9223372036854775807 (922,337,203,685.4775807e7)\n"
                "tx.operations[3].sourceAccount._present: false\n"
                "tx.operations[3].body.type: PAYMENT\n"
                "tx.operations[3].body.paymentOp.destination: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[3].body.paymentOp.asset: TestXLM\n"
                "tx.operations[3].body.paymentOp.amount: 1 (0.0000001e7)\n"
                "tx.operations[4].sourceAccount._present: false\n"
                "tx.operations[4].body.type: CREATE_ACCOUNT\n"
                "tx.operations[4].body.createAccountOp.destination: GDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435DIAG\n"
                "tx.operations[4].body.createAccountOp.startingBalance: 10000000000000 (1,000,000e7)\n"
                "tx.ext.v: 0\n"
                "signatures.len: 1\n"
                "signatures[0].hint: b40f6f5c (GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR)\n"
                "signatures[0].signature: 1f1a0da35d684b28e51aba77ec359d7306dd0956cce5bd902bd9a1589093f2aafc05d48ae29c4035e2e29c488865ba51aa647727a85cc37b03ba4f4144e5e404",
                "AAAAAgAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXAAAAfQAAAEfcfsEywAAAAEAAAAAZVPxAAAAAABrSdIA"
                "AAAAAAAAAAUAAAAAAAAAAQAAAACBOXcOqH0XX1ajVGbDTH7My42KkbTuN6Jd9g9bj8mzlAAAAAAAAAACVAvkAAAAAAAAAAAB"
                "AAAAAO1JKMYo0cLG6ukDOJBZlWEpWSc6XGP5NjbBRhSshzfRAAAAAVVTRAAAAAAAypOsFwUYcHHWe4PH/w7+gQjo7EUwV113"
                "JoeTM9vavnwAAAAcvpkaFAAAAAAAAAABAAAAAIE5dw6ofRdfVqNUZsNMfszLjYqRtO43ol32D1uPybOUAAAAAkxPTkdBU1NF"
                "VDAxMgAAAABuehzdKbC3j9E69MVZj+/07yqXFm48pvLk+/zNgFBb8X//////////AAAAAAAAAAEAAAAAgTl3Dqh9F19Wo1Rm"
                "w0x+zMuNipG07jeiXfYPW4/Js5QAAAAAAAAAAAAAAAEAAAAAAAAAAAAAAADtSSjGKNHCxurpAziQWZVhKVknOlxj+TY2wUYU"
                "rIc30QAACRhOcqAAAAAAAAAAAAG0D29cAAAAQB8aDaNdaEso5Rq6d+w1nXMG3QlWzOW9kCvZoViQk/Kq/AXUiuKcQDXi4pxI"
                "iGW6UapkdyeoXMN7A7pPQUTl5AQ=",
                "aefab24d24618e2a10f39c878a6a704200c0b7e9c9879f688ad279358c84e68a" },
            { "payments_big_minus0930", false,
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 500\n"
                "tx.seqNum: 1234567890123\n"
                "tx.cond.type: PRECOND_TIME\n"
                "tx.cond.timeBounds.minTime: 1700000000 (Tue Nov 14 12:43:20 -0930 2023)\n"
                "tx.cond.timeBounds.maxTime: 1800000000 (Thu Jan 14 22:30:00 -0930 2027)\n"
                "tx.memo.type: MEMO_NONE\n"
                "tx.operations.len: 5\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: PAYMENT\n"
                "tx.operations[0].body.paymentOp.destination: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[0].body.paymentOp.asset: TestXLM\n"
                "tx.operations[0].body.paymentOp.amount: 10000000000 (1,000e7)\n"
                "tx.operations[1].sourceAccount._present: false\n"
                "tx.operations[1].body.type: PAYMENT\n"
                "tx.operations[1].body.paymentOp.destination: GDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435DIAG\n"
                "tx.operations[1].body.paymentOp.asset: USD:GDFJHLAXAUMHA4OWPOB4P7YO72AQR2HMIUYFOXLXE2DZGM633K7HZDQP\n"
                "tx.operations[1].body.paymentOp.amount: 123456789012 (12,345.6789012e7)\n"
                "tx.operations[2].sourceAccount._present: false\n"
                "tx.operations[2].body.type: PAYMENT\n"
                "tx.operations[2].body.paymentOp.destination: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[2].body.paymentOp.asset: LONGASSET012:GBXHUHG5FGYLPD6RHL2MKWMP572O6KUXCZXDZJXS4T57ZTMAKBN7DWXN\n"
                "tx.operations[2].body.paymentOp.amount: 9223372036854775807 (922,337,203,685.4775807e7)\n"
                "tx.operations[3].sourceAccount._present: false\n"
                "tx.operations[3].body.type: PAYMENT\n"
                "tx.operations[3].body.paymentOp.destination: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[3].body.paymentOp.asset: TestXLM\n"
                "tx.operations[3].body.paymentOp.amount: 1 (0.0000001e7)\n"
                "tx.operations[4].sourceAccount._present: false\n"
                "tx.operations[4].body.type: CREATE_ACCOUNT\n"
                "tx.operations[4].body.createAccountOp.destination: GDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435DIAG\n"
                "tx.operations[4].body.createAccountOp.startingBalance: 10000000000000 (1,000,000e7)\n"
                "tx.ext.v: 0\n"
                "signatures.len: 1\n"
                "signatures[0].hint: b40f6f5c (GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR)\n"
                "signatures[0].signature: 1f1a0da35d684b28e51aba77ec359d7306dd0956cce5bd902bd9a1589093f2aafc05d48ae29c4035e2e29c488865ba51aa647727a85cc37b03ba4f4144e5e404",
                "AAAAAgAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXAAAAfQAAAEfcfsEywAAAAEAAAAAZVPxAAAAAABrSdIA"
                "AAAAAAAAAAUAAAAAAAAAAQAAAACBOXcOqH0XX1ajVGbDTH7My42KkbTuN6Jd9g9bj8mzlAAAAAAAAAACVAvkAAAAAAAAAAAB"
                "AAAAAO1JKMYo0cLG6ukDOJBZlWEpWSc6XGP5NjbBRhSshzfRAAAAAVVTRAAAAAAAypOsFwUYcHHWe4PH/w7+gQjo7EUwV113"
                "JoeTM9vavnwAAAAcvpkaFAAAAAAAAAABAAAAAIE5dw6ofRdfVqNUZsNMfszLjYqRtO43ol32D1uPybOUAAAAAkxPTkdBU1NF"
                "VDAxMgAAAABuehzdKbC3j9E69MVZj+/07yqXFm48pvLk+/zNgFBb8X//////////AAAAAAAAAAEAAAAAgTl3Dqh9F19Wo1Rm"
                "w0x+zMuNipG07jeiXfYPW4/Js5QAAAAAAAAAAAAAAAEAAAAAAAAAAAAAAADtSSjGKNHCxurpAziQWZVhKVknOlxj+TY2wUYU"
                "rIc30QAACRhOcqAAAAAAAAAAAAG0D29cAAAAQB8aDaNdaEso5Rq6d+w1nXMG3QlWzOW9kCvZoViQk/Kq/AXUiuKcQDXi4pxI"
                "iGW6UapkdyeoXMN7A7pPQUTl5AQ=",
                "aefab24d24618e2a10f39c878a6a704200c0b7e9c9879f688ad279358c84e68a" },
            { "payments_big_dubai", false,
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 500\n"
                "tx.seqNum: 1234567890123\n"
                "tx.cond.type: PRECOND_TIME\n"
                "tx.cond.timeBounds.minTime: 1700000000 (Wed Nov 15 02:13:20 +04 2023)\n"
                "tx.cond.timeBounds.maxTime: 1800000000 (Fri Jan 15 12:00:00 +04 2027)\n"
                "tx.memo.type: MEMO_NONE\n"
                "tx.operations.len: 5\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: PAYMENT\n"
                "tx.operations[0].body.paymentOp.destination: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[0].body.paymentOp.asset: TestXLM\n"
                "tx.operations[0].body.paymentOp.amount: 10000000000 (1,000e7)\n"
                "tx.operations[1].sourceAccount._present: false\n"
                "tx.operations[1].body.type: PAYMENT\n"
                "tx.operations[1].body.paymentOp.destination: GDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435DIAG\n"
                "tx.operations[1].body.paymentOp.asset: USD:GDFJHLAXAUMHA4OWPOB4P7YO72AQR2HMIUYFOXLXE2DZGM633K7HZDQP\n"
                "tx.operations[1].body.paymentOp.amount: 123456789012 (12,345.6789012e7)\n"
                "tx.operations[2].sourceAccount._present: false\n"
                "tx.operations[2].body.type: PAYMENT\n"
                "tx.operations[2].body.paymentOp.destination: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[2].body.paymentOp.asset: LONGASSET012:GBXHUHG5FGYLPD6RHL2MKWMP572O6KUXCZXDZJXS4T57ZTMAKBN7DWXN\n"
                "tx.operations[2].body.paymentOp.amount: 9223372036854775807 (922,337,203,685.4775807e7)\n"
                "tx.operations[3].sourceAccount._present: false\n"
                "tx.operations[3].body.type: PAYMENT\n"
                "tx.operations[3].body.paymentOp.destination: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U\n"
                "tx.operations[3].body.paymentOp.asset: TestXLM\n"
                "tx.operations[3].body.paymentOp.amount: 1 (0.0000001e7)\n"
                "tx.operations[4].sourceAccount._present: false\n"
                "tx.operations[4].body.type: CREATE_ACCOUNT\n"
                "tx.operations[4].body.createAccountOp.destination: GDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435DIAG\n"
                "tx.operations[4].body.createAccountOp.startingBalance: 10000000000000 (1,000,000e7)\n"
                "tx.ext.v: 0\n"
                "signatures.len: 1\n"
                "signatures[0].hint: b40f6f5c (GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR)\n"
                "signatures[0].signature: 1f1a0da35d684b28e51aba77ec359d7306dd0956cce5bd902bd9a1589093f2aafc05d48ae29c4035e2e29c488865ba51aa647727a85cc37b03ba4f4144e5e404",
                "AAAAAgAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXAAAAfQAAAEfcfsEywAAAAEAAAAAZVPxAAAAAABrSdIA"
                "AAAAAAAAAAUAAAAAAAAAAQAAAACBOXcOqH0XX1ajVGbDTH7My42KkbTuN6Jd9g9bj8mzlAAAAAAAAAACVAvkAAAAAAAAAAAB"
                "AAAAAO1JKMYo0cLG6ukDOJBZlWEpWSc6XGP5NjbBRhSshzfRAAAAAVVTRAAAAAAAypOsFwUYcHHWe4PH/w7+gQjo7EUwV113"
                "JoeTM9vavnwAAAAcvpkaFAAAAAAAAAABAAAAAIE5dw6ofRdfVqNUZsNMfszLjYqRtO43ol32D1uPybOUAAAAAkxPTkdBU1NF"
                "VDAxMgAAAABuehzdKbC3j9E69MVZj+/07yqXFm48pvLk+/zNgFBb8X//////////AAAAAAAAAAEAAAAAgTl3Dqh9F19Wo1Rm"
                "w0x+zMuNipG07jeiXfYPW4/Js5QAAAAAAAAAAAAAAAEAAAAAAAAAAAAAAADtSSjGKNHCxurpAziQWZVhKVknOlxj+TY2wUYU"
                "rIc30QAACRhOcqAAAAAAAAAAAAG0D29cAAAAQB8aDaNdaEso5Rq6d+w1nXMG3QlWzOW9kCvZoViQk/Kq/AXUiuKcQDXi4pxI"
                "iGW6UapkdyeoXMN7A7pPQUTl5AQ=",
                "aefab24d24618e2a10f39c878a6a704200c0b7e9c9879f688ad279358c84e68a" },
            { "manage_data_empty_value", false,
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
                "tx.fee: 100\n"
                "tx.seqNum: 1234567890123\n"
                "tx.cond.type: PRECOND_NONE\n"
                "tx.memo.type: MEMO_NONE\n"
                "tx.operations.len: 1\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: MANAGE_DATA\n"
                "tx.operations[0].body.manageDataOp.dataName: \"k\"\n"
                "tx.operations[0].body.manageDataOp.dataValue._present: true\n"
                "tx.operations[0].body.manageDataOp.dataValue: 0 bytes\n"
                "tx.ext.v: 0\n"
                "signatures.len: 1\n"
                "signatures[0].hint: b40f6f5c (bad signature/unknown key/test is wrong network)\n"
                "signatures[0].signature: 0 bytes",
                "AAAAAgAAAACKiOPddAnxlf1S2y08ul1yymcJvx2UEhvzdIgBtA9vXAAAAGQAAAEfcfsEywAAAAAAAAAAAAAAAQAAAAAAAAAK"
                "AAAAAWsAAAAAAAABAAAAAAAAAAAAAAABtA9vXAAAAAA=",
                "08968439eaedf3156644673afc343acb39568a51cd0946b6d9c74ed25ef61d42" },
            { "pubnet_58712", true,
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GD66ELTX5H4E6NUPC3JRANZ4YE45BZ34SC77MQZJPZJDOF54T2UIRP3H\n"
                "tx.fee: 100\n"
                "tx.seqNum: 183923332977305567\n"
                "tx.cond.type: PRECOND_TIME\n"
                "tx.cond.timeBounds.minTime: 1000 (Thu Jan  1 00:16:40 UTC 1970)\n"
                "tx.cond.timeBounds.maxTime: 1790626169993 (Sat Sep  7 22:19:53 UTC 58712)\n"
                "tx.memo.type: MEMO_TEXT\n"
                "tx.memo.text: \"\"\n"
                "tx.operations.len: 1\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: MANAGE_SELL_OFFER\n"
                "tx.operations[0].body.manageSellOfferOp.selling: SynthetixGen:GCNKZUSVXM5OTOMRUXTNWJP6IQJVZ35BJTNDAL4VJTUCQSQBD3H7CVHG\n"
                "tx.operations[0].body.manageSellOfferOp.buying: XLM\n"
                "tx.operations[0].body.manageSellOfferOp.amount: 8911726037868 (891,172.6037868e7)\n"
                "tx.operations[0].body.manageSellOfferOp.price.n: 52657\n"
                "tx.operations[0].body.manageSellOfferOp.price.d: 50000000\n"
                "tx.operations[0].body.manageSellOfferOp.offerID: 1706003401 (170.6003401e7)\n"
                "tx.ext.v: 0\n"
                "signatures.len: 1\n"
                "signatures[0].hint: bc9ea888 (GD66ELTX5H4E6NUPC3JRANZ4YE45BZ34SC77MQZJPZJDOF54T2UIRP3H)\n"
                "signatures[0].signature: e8686eed3e6685dba7aa40de22ce0bbbd49b5f7425d0b8405106e6b3fc2c19f01f615ab228f5dc582afed490331f19aacccc475cc9ca2d6bc62bdc262ac9a00d",
                "AAAAAgAAAAD94i536fhPNo8W0xA3PME50Od8kL/2Qyl+UjcXvJ6oiAAAAGQCjW1MAASn3wAAAAEAAAAAAAAD6AAAAaDpowSJ"
                "AAAAAQAAAAAAAAABAAAAAAAAAAMAAAACU3ludGhldGl4R2VuAAAAAJqs0lW7Oum5kaXm2yX+RBNc76FM2jAvlUzoKEoBHs/x"
                "AAAAAAAACBrsQ8tsAADNsQL68IAAAAAAZa+LyQAAAAAAAAABvJ6oiAAAAEDoaG7tPmaF26eqQN4izgu71JtfdCXQuEBRBuaz"
                "/CwZ8B9hWrIo9dxYKv7UkDMfGarMzEdcycota8Yr3CYqyaAN",
                "0f27112a246d29f40ee17e8283f2574b2ccd12937f19e1c54262b45882b621d8" },
            { "pubnet_58712_madrid", true,
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GD66ELTX5H4E6NUPC3JRANZ4YE45BZ34SC77MQZJPZJDOF54T2UIRP3H\n"
                "tx.fee: 100\n"
                "tx.seqNum: 183923332977305567\n"
                "tx.cond.type: PRECOND_TIME\n"
                "tx.cond.timeBounds.minTime: 1000 (Thu Jan  1 01:16:40 CET 1970)\n"
                "tx.cond.timeBounds.maxTime: 1790626169993 (Sat Sep  7 23:19:53 CET 58712)\n"
                "tx.memo.type: MEMO_TEXT\n"
                "tx.memo.text: \"\"\n"
                "tx.operations.len: 1\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: MANAGE_SELL_OFFER\n"
                "tx.operations[0].body.manageSellOfferOp.selling: SynthetixGen:GCNKZUSVXM5OTOMRUXTNWJP6IQJVZ35BJTNDAL4VJTUCQSQBD3H7CVHG\n"
                "tx.operations[0].body.manageSellOfferOp.buying: XLM\n"
                "tx.operations[0].body.manageSellOfferOp.amount: 8911726037868 (891,172.6037868e7)\n"
                "tx.operations[0].body.manageSellOfferOp.price.n: 52657\n"
                "tx.operations[0].body.manageSellOfferOp.price.d: 50000000\n"
                "tx.operations[0].body.manageSellOfferOp.offerID: 1706003401 (170.6003401e7)\n"
                "tx.ext.v: 0\n"
                "signatures.len: 1\n"
                "signatures[0].hint: bc9ea888 (GD66ELTX5H4E6NUPC3JRANZ4YE45BZ34SC77MQZJPZJDOF54T2UIRP3H)\n"
                "signatures[0].signature: e8686eed3e6685dba7aa40de22ce0bbbd49b5f7425d0b8405106e6b3fc2c19f01f615ab228f5dc582afed490331f19aacccc475cc9ca2d6bc62bdc262ac9a00d",
                "AAAAAgAAAAD94i536fhPNo8W0xA3PME50Od8kL/2Qyl+UjcXvJ6oiAAAAGQCjW1MAASn3wAAAAEAAAAAAAAD6AAAAaDpowSJ"
                "AAAAAQAAAAAAAAABAAAAAAAAAAMAAAACU3ludGhldGl4R2VuAAAAAJqs0lW7Oum5kaXm2yX+RBNc76FM2jAvlUzoKEoBHs/x"
                "AAAAAAAACBrsQ8tsAADNsQL68IAAAAAAZa+LyQAAAAAAAAABvJ6oiAAAAEDoaG7tPmaF26eqQN4izgu71JtfdCXQuEBRBuaz"
                "/CwZ8B9hWrIo9dxYKv7UkDMfGarMzEdcycota8Yr3CYqyaAN",
                "0f27112a246d29f40ee17e8283f2574b2ccd12937f19e1c54262b45882b621d8" },
            { "pubnet_58712_madrid_tzdata", true,
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GD66ELTX5H4E6NUPC3JRANZ4YE45BZ34SC77MQZJPZJDOF54T2UIRP3H\n"
                "tx.fee: 100\n"
                "tx.seqNum: 183923332977305567\n"
                "tx.cond.type: PRECOND_TIME\n"
                "tx.cond.timeBounds.minTime: 1000 (Thu Jan  1 01:16:40 CET 1970)\n"
                "tx.cond.timeBounds.maxTime: 1790626169993 (Sun Sep  8 00:19:53 CEST 58712)\n"
                "tx.memo.type: MEMO_TEXT\n"
                "tx.memo.text: \"\"\n"
                "tx.operations.len: 1\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: MANAGE_SELL_OFFER\n"
                "tx.operations[0].body.manageSellOfferOp.selling: SynthetixGen:GCNKZUSVXM5OTOMRUXTNWJP6IQJVZ35BJTNDAL4VJTUCQSQBD3H7CVHG\n"
                "tx.operations[0].body.manageSellOfferOp.buying: XLM\n"
                "tx.operations[0].body.manageSellOfferOp.amount: 8911726037868 (891,172.6037868e7)\n"
                "tx.operations[0].body.manageSellOfferOp.price.n: 52657\n"
                "tx.operations[0].body.manageSellOfferOp.price.d: 50000000\n"
                "tx.operations[0].body.manageSellOfferOp.offerID: 1706003401 (170.6003401e7)\n"
                "tx.ext.v: 0\n"
                "signatures.len: 1\n"
                "signatures[0].hint: bc9ea888 (GD66ELTX5H4E6NUPC3JRANZ4YE45BZ34SC77MQZJPZJDOF54T2UIRP3H)\n"
                "signatures[0].signature: e8686eed3e6685dba7aa40de22ce0bbbd49b5f7425d0b8405106e6b3fc2c19f01f615ab228f5dc582afed490331f19aacccc475cc9ca2d6bc62bdc262ac9a00d",
                "AAAAAgAAAAD94i536fhPNo8W0xA3PME50Od8kL/2Qyl+UjcXvJ6oiAAAAGQCjW1MAASn3wAAAAEAAAAAAAAD6AAAAaDpowSJ"
                "AAAAAQAAAAAAAAABAAAAAAAAAAMAAAACU3ludGhldGl4R2VuAAAAAJqs0lW7Oum5kaXm2yX+RBNc76FM2jAvlUzoKEoBHs/x"
                "AAAAAAAACBrsQ8tsAADNsQL68IAAAAAAZa+LyQAAAAAAAAABvJ6oiAAAAEDoaG7tPmaF26eqQN4izgu71JtfdCXQuEBRBuaz"
                "/CwZ8B9hWrIo9dxYKv7UkDMfGarMzEdcycota8Yr3CYqyaAN",
                "0f27112a246d29f40ee17e8283f2574b2ccd12937f19e1c54262b45882b621d8" },
            { "pubnet_summer_madrid", true,
                "type: ENVELOPE_TYPE_TX\n"
                "tx.sourceAccount: GAH5JW67EDWOZ445RYCIQ2U72MBTSDAOAK3HB6A45UA43DBAC4XXZPAY\n"
                "tx.fee: 10000\n"
                "tx.seqNum: 267765084619559391\n"
                "tx.cond.type: PRECOND_TIME\n"
                "tx.cond.timeBounds.minTime: 0\n"
                "tx.cond.timeBounds.maxTime: 1784469319 (Sun Jul 19 15:55:19 CEST 2026)\n"
                "tx.memo.type: MEMO_NONE\n"
                "tx.operations.len: 1\n"
                "tx.operations[0].sourceAccount._present: false\n"
                "tx.operations[0].body.type: PAYMENT\n"
                "tx.operations[0].body.paymentOp.destination: GB7NY4TIICID3KJNGMLWXGDMR6YFWWZOHCDJNLG47AT6OPIBEXGTD7TD\n"
                "tx.operations[0].body.paymentOp.asset: PAYEUR:GAH5JW67EDWOZ445RYCIQ2U72MBTSDAOAK3HB6A45UA43DBAC4XXZPAY\n"
                "tx.operations[0].body.paymentOp.amount: 19125000000000 (1,912,500e7)\n"
                "tx.ext.v: 0\n"
                "signatures.len: 1\n"
                "signatures[0].hint: 20172f7c (GAH5JW67EDWOZ445RYCIQ2U72MBTSDAOAK3HB6A45UA43DBAC4XXZPAY)\n"
                "signatures[0].signature: 74ae9f897b8f657b9c9b5afa395bf4b92f4aff5e227f3e162caa2e9a1b1cb05648e0528d3f5255d52747e2b2041717544471ede977129ddc0defb87116bb3a07",
                "AAAAAgAAAAAP1NvfIOzs852OBIhqn9MDOQwOArZw+BztAc2MIBcvfAAAJxADt0rtAABR3wAAAAEAAAAAAAAAAAAAAABqXNdH"
                "AAAAAAAAAAEAAAAAAAAAAQAAAAB+3HJoQJA9qS0zF2uYbI+wW1suOIaWrNz4J+c9ASXNMQAAAAJQQVlFVVIAAAAAAAAAAAAA"
                "D9Tb3yDs7POdjgSIap/TAzkMDgK2cPgc7QHNjCAXL3wAABFk4tTSAAAAAAAAAAABIBcvfAAAAEB0rp+Je49le5ybWvo5W/S5"
                "L0r/XiJ/PhYsqi6aGxywVkjgUo0/UlXVJ0fisgQXF1REce3pdxKd3A3vuHEWuzoH",
                "700851e730fdb2e01d4a1a763e135c69fb105c91f0a9855afbf1be467330eb34" },
        };
        return cases;
    }
};

ADD_TEST(Sep11TxrepTest)
#endif // SEP11TXREPTEST_H
