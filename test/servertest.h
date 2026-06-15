#ifndef SERVERTEST_H
#define SERVERTEST_H
#include <QObject>
#include "src/keypair.h"
#include "src/util.h"

#include <QtTest>
#include "testcollector.h"
#include "../src/server.h"
#include "../src/network.h"
#include "../src/transaction.h"
#include "../src/account.h"
#include "../src/createaccountoperation.h"
#include "../src/responses/submittransactionresponse.h"
#include "../src/paymentoperation.h"
#include "../src/pathpaymentstrictreceiveoperation.h"
#include "../src/pathpaymentstrictsendoperation.h"
#include "../src/accountmergeoperation.h"
#include "../src/memo.h"
#include "../src/assettypecreditalphanum4.h"
#include "../src/assettypecreditalphanum12.h"
#include "../src/managedataoperation.h"
#include "../src/responses/operationpage.h"
#include "../src/responses/operations/paymentoperationresponse.h"

#include "fakeserver.h"

class ServerTest: public QObject
{
    Q_OBJECT


    Server *m_server;
    Transaction * m_transaction=nullptr;
    Transaction * m_transactionWrong=nullptr;

    void setUp()  {
        Network::useTestNetwork();
        //m_server = new Server("https://horizon.stellar.org");
        m_server = new Server("https://horizon-testnet.stellar.org");
    }


    void resetNetwork() {
        Network::use(nullptr);
    }

    /**
     * The following tests are related to SEP-0029.
     */
    QString DESTINATION_ACCOUNT_MEMO_REQUIRED_A = "GCMDQXJJGQE6TJ5XUHJMJUUIWECC5S6VANRAOWIQMMV4ALW43JOY2SEB";
    QString DESTINATION_ACCOUNT_MEMO_REQUIRED_B = "GDUR2DMT5AQ7DJUGBIBB45NKRNQXGRJTWTQ7DPRP37EKBELSMK57RMZK";
    QString DESTINATION_ACCOUNT_MEMO_REQUIRED_C = "GCS36NBLT6OKYN5EUQOQ7ZZIM6WXXNX5ME4JGTCG3HVZOYXRRMNUHNMM";
    QString DESTINATION_ACCOUNT_MEMO_REQUIRED_D = "GAKQNN6GNGNPLYBVEDCD5QAIEHAZVNCQET3HAUR4YWQAP5RPBLU2W7UG";
    QString DESTINATION_ACCOUNT_NO_MEMO_REQUIRED = "GDYC2D4P2SRC5DCEDDK2OUFESSPCTZYLDOEF6NYHR2T7X5GUTEABCQC2";
    QString DESTINATION_ACCOUNT_NO_FOUND = "GD2OVSQPGD5FBJPMW4YN3FGDJ7JDFKNOMJT35T4H52FLHXJK5MFSR5RA";
    QString DESTINATION_ACCOUNT_FETCH_ERROR = "GB7WNQUTDLD6YJ4MR3KQN3Y6ZIDIGTA7GRKNH47HOGMP2ETFGRSLD6OG";
    QString DESTINATION_ACCOUNT_MEMO_ID = "MCAAAAAAAAAAAAB7BQ2L7E5NBWMXDUCMZSIPOBKRDSBYVLMXGSSKF6YNPIB7Y77ITKNOG";

    QString successTransactionResponse ="{\n"
                            "  \"_links\": {\n"
                            "    \"transaction\": {\n"
                            "      \"href\": \"/transactions/2634d2cf5adcbd3487d1df042166eef53830115844fdde1588828667bf93ff42\"\n"
                            "    }\n"
                            "  },\n"
                            "  \"hash\": \"2634d2cf5adcbd3487d1df042166eef53830115844fdde1588828667bf93ff42\",\n"
                            "  \"ledger\": 826150,\n"
                            "  \"envelope_xdr\": \"AAAAAKu3N77S+cHLEDfVD2eW/CqRiN9yvAKH+qkeLjHQs1u+AAAAZAAMkoMAAAADAAAAAAAAAAAAAAABAAAAAAAAAAAAAAAAbYQq8ek1GitmNBUloGnetfWxSpxlsgK48Xi66dIL3MoAAAAAC+vCAAAAAAAAAAAB0LNbvgAAAEDadQ25SNHWTg0L+2wr/KNWd8/EwSNFkX/ncGmBGA3zkNGx7lAow78q8SQmnn2IsdkD9MwICirhsOYDNbaqShwO\",\n"
                            "  \"result_xdr\": \"AAAAAAAAAGQAAAAAAAAAAQAAAAAAAAAAAAAAAAAAAAA=\",\n"
                            "  \"result_meta_xdr\": \"AAAAAAAAAAEAAAACAAAAAAAMmyYAAAAAAAAAAG2EKvHpNRorZjQVJaBp3rX1sUqcZbICuPF4uunSC9zKAAAAAAvrwgAADJsmAAAAAAAAAAAAAAAAAAAAAAAAAAABAAAAAAAAAAAAAAAAAAAAAAAAAQAMmyYAAAAAAAAAAKu3N77S+cHLEDfVD2eW/CqRiN9yvAKH+qkeLjHQs1u+AAAAFzCfYtQADJKDAAAAAwAAAAAAAAAAAAAAAAAAAAABAAAAAAAAAAAAAAAAAAAA\"\n"
                            "}";
    QString memoRequiredResponse = "{\n"
               "    \"data\": {\n"
               "        \"config.memo_required\": \"MQ==\"\n"
               "    }\n"
               "}";
    QString noMemoRequiredResponse = "{\n"
               "    \"data\": {\n"
               "    }\n"
               "}";

    QString resourceMissingResponse = "{\n"
                                      " \"type\": \"https://stellar.org/horizon-errors/not_found\",\n"
                                      " \"title\": \"Resource Missing\",\n"
                                      " \"status\": 404,\n"
                                      " \"detail\": \"The resource at the url requested was not found.  This usually occurs for one of two reasons:  The url requested is not valid, or no data in our database could be found with the parameters provided.\"\n"
                                    "}";

    FeeBumpTransaction* feeBump(Transaction* inner) {
        KeyPair* signer = KeyPair::fromSecretSeed(QString("SA5ZEFDVFZ52GRU7YUGR6EDPBNRU2WLA6IQFQ7S2IH2DG3VFV3DOMV2Q"));
        FeeBumpTransaction* tx =  FeeBumpTransaction::Builder(AccountConverter().enableMuxed(), inner)
                .setFeeAccount(signer->getAccountId())
                .setBaseFee(FeeBumpTransaction::MIN_BASE_FEE*10)
                .build();
        tx->sign(signer);
        return tx;
    }

    // Builds one SSE payment event: "id: <id>\ndata: {payment ...}\n\n".
    static QString paymentEvent(const QString& id, const QString& amount) {
        return "id: " + id + "\n"
               "data: {\"type\":\"payment\",\"type_i\":1,\"id\":\"" + id + "\",\"paging_token\":\"" + id + "\","
               "\"source_account\":\"GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H\","
               "\"asset_type\":\"native\","
               "\"from\":\"GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H\","
               "\"to\":\"GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H\","
               "\"amount\":\"" + amount + "\"}\n"
               "\n";
    }
private slots:

    void initTestCase()
    {
        qDebug() <<"qtcpp stellar client name:"<<STELLAR_QT_SDK_CLIENT_NAME;
        qDebug() <<"qtcpp stellar client version:"<< STELLAR_QT_SDK_CLIENT_VERSION;
        this->setUp();
    }
    void cleanupTestCase()
    {
        qApp->processEvents();
    }

    void testBuildTransaction()
    {
        qint64 sequenceNumber=5026524780560386;
        KeyPair *source = KeyPair::fromSecretSeed(QString("SDPK5IBB57JY5SMMBUBOFSAHNNCETCZWLTKKDPWBDYXBX3B4ZAFOQYD5"));

        KeyPair *destination = KeyPair::fromSecretSeed(QByteArray::fromHex("268486538a268486538f268486538a268486538f268486538a268486538feedd"));


        Account* account = new Account(source, sequenceNumber);
        Transaction::Builder *builder = new Transaction::Builder(AccountConverter().enableMuxed(), account);
        builder->addOperation(new CreateAccountOperation(destination, "2000"))
                .addMemo(Memo::text("Hello world!"))
                .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                .setBaseFee(Transaction::Builder::BASE_FEE);

        QVERIFY(1 == builder->getOperationsCount());
        Transaction *transaction = builder->build();
        delete builder;
        QVERIFY((sequenceNumber+1) == transaction->getSequenceNumber());
        QVERIFY((sequenceNumber+1) == account->getSequenceNumber());
        transaction->sign(source);
        m_transaction=transaction;

    }

    // P1.11 — first SSE streaming test (FakeServer streaming mode).
    // Uses a LOCAL Server that is deleted at the end so the stream's reconnect
    // timer is torn down (no lingering reconnect loop after the test).
    void testStreamReceivesData() {
        FakeServer* fakeServer = new FakeServer();
        QString round1 =
            "id: 1\n"
            "data: {\"type\":\"payment\",\"type_i\":1,\"id\":\"1\",\"paging_token\":\"1\","
            "\"source_account\":\"GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H\","
            "\"asset_type\":\"native\","
            "\"from\":\"GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H\","
            "\"to\":\"GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H\","
            "\"amount\":\"10.0000000\"}\n"
            "\n";
        fakeServer->addStream("/payments", QStringList() << round1);

        Server* server = new Server(fakeServer->baseUrl());
        OperationPage* stream = server->payments().stream().execute();

        bool gotEvent = false;
        QObject::connect(stream, &Response::ready, [&gotEvent](){ gotEvent = true; });
        WAIT_FOR(!gotEvent)

        QVERIFY(gotEvent);
        QVERIFY(stream->size() >= 1);
        PaymentOperationResponse* p = dynamic_cast<PaymentOperationResponse*>(stream->streamedElement());
        QVERIFY(p != nullptr);
        QCOMPARE(p->getAmount(), QString("10.0000000"));

        delete server;            // tears down the streaming response + reconnect timer
        fakeServer->deleteLater();
    }

    // The stream closes after round 1; the SDK reconnects (after >= RECONNECT_DELAY)
    // sending `Last-Event-ID`, and round 2 is served.
    void testStreamReconnectsWithLastEventId() {
        FakeServer* fakeServer = new FakeServer();
        fakeServer->addStream("/payments", QStringList()
            << paymentEvent("5", "10.0000000")
            << paymentEvent("6", "20.0000000"));

        Server* server = new Server(fakeServer->baseUrl());
        OperationPage* stream = server->payments().stream().execute();

        int events = 0;
        QString lastAmount;
        QObject::connect(stream, &Response::ready, [&](){
            events++;
            if (auto* p = dynamic_cast<PaymentOperationResponse*>(stream->streamedElement()))
                lastAmount = p->getAmount();
        });
        WAIT_FOR(events < 2)   // the 2nd event only arrives after a reconnect (~1s)

        QVERIFY(events >= 2);                            // reconnected
        QCOMPARE(lastAmount, QString("20.0000000"));      // round 2 served
        // The reconnect must carry the last seen id. The SDK sets the raw header
        // as "Last-Event-ID", but Qt re-cases the name on the wire and the casing
        // differs by version: Qt 6.8-6.10 send it lowercase ("last-event-id"),
        // Qt 6.11 title-cases it ("Last-Event-Id"). HTTP header names are
        // case-insensitive (RFC 7230 §3.2), so match case-insensitively rather
        // than pinning one Qt version's casing.
        QVERIFY(fakeServer->lastRequestHeaders().toLower().contains("last-event-id: 5"));

        delete server;
        fakeServer->deleteLater();
    }

    // Two SSE events delivered in a single connection chunk → two `ready()`s.
    void testStreamParsesMultipleEvents() {
        FakeServer* fakeServer = new FakeServer();
        fakeServer->addStream("/payments", QStringList()
            << (paymentEvent("1", "10.0000000") + paymentEvent("2", "20.0000000")));

        Server* server = new Server(fakeServer->baseUrl());
        OperationPage* stream = server->payments().stream().execute();

        int events = 0;
        QString lastAmount;
        QObject::connect(stream, &Response::ready, [&](){
            events++;
            if (auto* p = dynamic_cast<PaymentOperationResponse*>(stream->streamedElement()))
                lastAmount = p->getAmount();
        });
        WAIT_FOR(events < 2)

        QVERIFY(events >= 2);
        QCOMPARE(lastAmount, QString("20.0000000"));

        delete server;
        fakeServer->deleteLater();
    }
#ifndef STELLAR_SKIP_LIVE_TESTS
    //it will fail because sequence number, it should be catched before creating the transaction
     void testSubmitTransactionSuccess()  {
         QVERIFY(m_transaction);
         SubmitTransactionResponse *r=nullptr;
         QMetaObject::Connection c = QObject::connect(m_server,&Server::transactionResponse,[&r](SubmitTransactionResponse *response){
             qDebug() << "RECEIVED ANSWER";
             r = response;

         });         
         m_server->submitTransaction(this->m_transaction,true);         
         WAIT_FOR(!r)
         QObject::disconnect(c);
         QVERIFY(r!=nullptr);
         QVERIFY(r->isSuccess());
         QVERIFY(r->getLedger()==1676526);
         QString expected("732ac60a45c9bc86fcf14292bb7c7ebf6fec4a83767c1fc09148ac946e4b2ef1");
         QVERIFY(r->getHash() == expected);
         QVERIFY(r->getExtras().getTransactionResultCode().isEmpty());
     }
#endif
     void testBuildWrongTransaction()
     {
         qint64 sequenceNumber=1;
         KeyPair *source = KeyPair::fromSecretSeed(QString("SDPK5IBB57JY5SMMBUBOFSAHNNCETCZWLTKKDPWBDYXBX3B4ZAFOQYD5"));
         KeyPair *destination = KeyPair::fromSecretSeed(QByteArray::fromHex("268486538a268486538f268486538a268486538f268486538a268486538feedd"));


         Account* account = new Account(source, sequenceNumber);
         Transaction::Builder *builder = new Transaction::Builder(AccountConverter().enableMuxed(), account);
         builder->addOperation(new CreateAccountOperation(destination, "2000"))
                 .addMemo(Memo::text("Hello world!")).setTimeout(Transaction::Builder::TIMEOUT_INFINITE).setBaseFee(Transaction::Builder::BASE_FEE);

         QVERIFY(1 == builder->getOperationsCount());
         Transaction *transaction = builder->build();
         delete builder;
         QVERIFY((sequenceNumber+1) == transaction->getSequenceNumber());
         QVERIFY((sequenceNumber+1) == account->getSequenceNumber());
         transaction->sign(source);
         m_transactionWrong=transaction;

     }
#ifndef STELLAR_SKIP_LIVE_TESTS
     void testSubmitTransactionFail() {
         QVERIFY(m_transactionWrong);
         SubmitTransactionResponse *r=nullptr;
         QMetaObject::Connection c = QObject::connect(m_server,&Server::transactionResponse,[&r](SubmitTransactionResponse *response){
                 qDebug() << "RECEIVED ANSWER";
                 r = response;

        });
         m_server->submitTransaction(this->m_transactionWrong,true);
         WAIT_FOR(!r)
                 QObject::disconnect(c);
         QVERIFY(r!=nullptr);
         QVERIFY(!r->isSuccess());
         QVERIFY(r->getLedger()==0);
         QVERIFY(r->getHash().isEmpty());
         QVERIFY(r->getEnvelopeXdr()== "AAAAAJClcQcwAoPH4NIUph8BzrQftysWUPdiDVrFVaU5BDMxAAAAZAAAAAAAAAACAAAAAAAAAAEAAAAMSGVsbG8gd29ybGQhAAAAAQAAAAAAAAAAAAAAADpuIIngCOUxqmiav7VlDapL++s8fBxj/DwSy2GyjpInAAAABKgXyAAAAAAAAAAAATkEMzEAAABAMJ4mVb8xU+o4qr6z+y74qjEvPgcU2tigQVMc98pCMucdE4xjfV6cWlMjLPy7OdvhgTB/QcEOw6VoCS42xhMeBQ==");
         QVERIFY(r->getExtras().getResultXdr() =="AAAAAAAAAAD////7AAAAAA==");
         QVERIFY(r->getExtras().getResultCodes().getTransactionResultCode()=="tx_bad_seq");
     }
#endif



     void testCheckMemoRequiredWithMemo() {

         FakeServer* fakeServer = new FakeServer();

         fakeServer->addPost("/transactions",successTransactionResponse);
         m_server = new Server(fakeServer->baseUrl());

         KeyPair* source = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));
         Account* account = new Account(source, 1L);
         Transaction *transaction =  Transaction::Builder(AccountConverter().enableMuxed(), account)
                 .addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_A), new AssetTypeNative(), "10"))
                 .addOperation(PathPaymentStrictReceiveOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_B), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(PathPaymentStrictSendOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_C), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(AccountMergeOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_D)))
                 .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                 .addMemo(new MemoText("Hello, Stellar."))
                 .setBaseFee(100)
                 .build();

         transaction->sign(source);

         SubmitTransactionResponse *r=nullptr;
         QMetaObject::Connection c = QObject::connect(m_server,&Server::transactionResponse,[&r](SubmitTransactionResponse *response){
             r = response;
         });
         m_server->submitTransaction(transaction);

         WAIT_FOR(!r)

         QVERIFY(r);

         r=nullptr;

         m_server->submitTransaction(feeBump(transaction));
         WAIT_FOR(!r)

         QVERIFY(r);
         fakeServer->deleteLater();

     }


     void testSubmitTimeout504IsDetected() {
         // S3 (fund-safety): a Horizon 504 Gateway Timeout MUST be detectable so
         // callers resend the SAME signed envelope (idempotent) instead of
         // rebuilding with a new sequence — rebuilding after a 504 can apply the
         // payment twice. Regression guard: Response::m_status was never
         // populated from the HTTP status code (it defaulted to 0), so
         // isTimeout() always returned false until preprocessResponse() was
         // fixed to set it.

         // --- 504 path: delivered via transactionError, flagged as a timeout ---
         FakeServer* timeoutServer = new FakeServer();
         // Body intentionally has NO "status" field, so getStatusCode()==504 can
         // only come from the HTTP status line, not from JSON reflection.
         timeoutServer->addPost("/transactions",
                                QStringLiteral("{\"title\":\"Gateway Timeout\"}"),
                                QStringLiteral("504 Gateway Timeout"));
         Server* tServer = new Server(timeoutServer->baseUrl());

         KeyPair* source1 = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));
         Account* a1 = new Account(source1, 1L);
         Transaction* tx1 = Transaction::Builder(AccountConverter().enableMuxed(), a1)
                 .addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED), new AssetTypeNative(), "10"))
                 .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                 .setBaseFee(100)
                 .build();
         tx1->sign(source1);

         SubmitTransactionResponse* rErr=nullptr;
         QObject::connect(tServer,&Server::transactionError,[&rErr](SubmitTransactionResponse* response){ rErr = response; });
         tServer->submitTransaction(tx1, true);
         WAIT_FOR(!rErr)
         QVERIFY(rErr);                        // arrived on the error path
         QVERIFY(rErr->isTimeout());           // 504 detected
         QCOMPARE(rErr->getStatusCode(), 504);
         delete tServer;
         timeoutServer->deleteLater();

         // --- 2xx path: real code surfaced, NOT a timeout ---
         FakeServer* okServer = new FakeServer();
         okServer->addPost("/transactions", successTransactionResponse);
         Server* oServer = new Server(okServer->baseUrl());

         KeyPair* source2 = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));
         Account* a2 = new Account(source2, 1L);
         Transaction* tx2 = Transaction::Builder(AccountConverter().enableMuxed(), a2)
                 .addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED), new AssetTypeNative(), "10"))
                 .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                 .setBaseFee(100)
                 .build();
         tx2->sign(source2);

         SubmitTransactionResponse* rOk=nullptr;
         QObject::connect(oServer,&Server::transactionResponse,[&rOk](SubmitTransactionResponse* response){ rOk = response; });
         oServer->submitTransaction(tx2, true);
         WAIT_FOR(!rOk)
         QVERIFY(rOk);
         QVERIFY(!rOk->isTimeout());           // 2xx is not a timeout
         QCOMPARE(rOk->getStatusCode(), 200);  // and surfaces the real HTTP code
         delete oServer;
         okServer->deleteLater();
     }
     void testCheckMemoRequiredWithMemoIdAddress()
     {
         FakeServer* fakeServer = new FakeServer();

         fakeServer->addPost("/transactions",successTransactionResponse);
         m_server = new Server(fakeServer->baseUrl());

         KeyPair* source = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));
         Account* account = new Account(source, 1L);
         Transaction* transaction =nullptr;
         try{
             transaction = Transaction::Builder(AccountConverter().enableMuxed(), account, Network::current())
                     .addOperation(new PaymentOperation(DESTINATION_ACCOUNT_MEMO_ID, new AssetTypeNative(), "10"))
                     .addOperation(new PathPaymentStrictReceiveOperation(new AssetTypeNative(), "10", DESTINATION_ACCOUNT_MEMO_ID, new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                     .addOperation(new PathPaymentStrictSendOperation(new AssetTypeNative(), "10", DESTINATION_ACCOUNT_MEMO_ID, new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                     .addOperation(new AccountMergeOperation(DESTINATION_ACCOUNT_MEMO_ID))
                     .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                     .setBaseFee(100)
                     .build();
             //QFAIL("Missing exception");
         } catch (const std::runtime_error& e) {
             //qDebug() << "EXCEPTION "<< e.what();
             //QCOMPARE("invalid address length", QString(e.what()));
             QFAIL("Muxed accounts not working");
             Q_UNUSED(e)
         }

         //QVERIFY(transaction==nullptr);//we dont even allow to build the transaction if destination is wrong.
         QVERIFY(transaction);//muxed accounts enabled
//         transaction->sign(source);

//         SubmitTransactionResponse *r=nullptr;
//         QMetaObject::Connection c = QObject::connect(m_server,&Server::transactionResponse,[&r](SubmitTransactionResponse *response){
//             r = response;

//         });
//         m_server->submitTransaction(transaction);

//         WAIT_FOR(!r)

//         QVERIFY(r);

//         r=nullptr;

//         m_server->submitTransaction(feeBump(transaction));
//         WAIT_FOR(!r)

//         QVERIFY(r);
         fakeServer->deleteLater();

     }

     void testCheckMemoRequiredWithSkipCheck(){
         FakeServer* fakeServer = new FakeServer();

         fakeServer->addPost("/transactions",successTransactionResponse);
         m_server = new Server(fakeServer->baseUrl());

         KeyPair* source = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));
         Account* account = new Account(source, 1L);
         Transaction *transaction =  Transaction::Builder(AccountConverter().enableMuxed(), account)
                 .addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_A), new AssetTypeNative(), "10"))
                 .addOperation(PathPaymentStrictReceiveOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_B), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(PathPaymentStrictSendOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_C), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(AccountMergeOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_D)))
                 .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                 .setBaseFee(100)
                 .build();

         transaction->sign(source);

         SubmitTransactionResponse *r=nullptr;
         QMetaObject::Connection c = QObject::connect(m_server,&Server::transactionResponse,[&r](SubmitTransactionResponse *response){
             r = response;

         });
         m_server->submitTransaction(transaction,true);

         WAIT_FOR(!r)


         QVERIFY(r);
         r=nullptr;

         m_server->submitTransaction(feeBump(transaction),true);
         WAIT_FOR(!r)

         QVERIFY(r);
         fakeServer->deleteLater();
     }

     void testCheckMemoRequiredWithPaymentOperationNoMemo() {
         FakeServer* fakeServer = new FakeServer();

         fakeServer->addPost("/transactions",successTransactionResponse);
         m_server = new Server(fakeServer->baseUrl());

         KeyPair* source = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));
         Account* account = new Account(source, 1L);
         Transaction *transaction =  Transaction::Builder(AccountConverter().enableMuxed(), account)
                 .addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_A), new AssetTypeNative(), "10"))
                 .addOperation(PathPaymentStrictReceiveOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(PathPaymentStrictSendOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(AccountMergeOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED)))
                 .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                 .setBaseFee(100)
                 .build();

         transaction->sign(source);

         SubmitTransactionResponse *r=nullptr;

         QMetaObject::Connection c = QObject::connect(m_server,&Server::transactionError,[&r](SubmitTransactionResponse *response){
             r = response;
         });
         m_server->submitTransaction(transaction);

         WAIT_FOR(!r)


         QVERIFY(r);//r is set because we received an error

         r=nullptr;

         m_server->submitTransaction(feeBump(transaction));
         WAIT_FOR(!r)

         QVERIFY(r);
         fakeServer->deleteLater();

     }
     void testCheckMemoRequiredWithPathPaymentStrictReceiveOperationNoMemo() {
         FakeServer* fakeServer = new FakeServer();

         fakeServer->addPost("/transactions",successTransactionResponse);
         m_server = new Server(fakeServer->baseUrl());

         KeyPair* source = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));
         Account* account = new Account(source, 1L);
         Transaction *transaction =  Transaction::Builder(AccountConverter().enableMuxed(), account)
                 .addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED), new AssetTypeNative(), "10"))
                 .addOperation(PathPaymentStrictReceiveOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_B), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(PathPaymentStrictSendOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(AccountMergeOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED)))
                 .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                 .setBaseFee(100)
                 .build();

         transaction->sign(source);

         SubmitTransactionResponse *r=nullptr;

         QMetaObject::Connection c = QObject::connect(m_server,&Server::transactionError,[&r](SubmitTransactionResponse *response){
             r = response;
         });
         m_server->submitTransaction(transaction);

         WAIT_FOR(!r)


         QVERIFY(r);//r is set because we received an error

         r=nullptr;

         m_server->submitTransaction(feeBump(transaction));
         WAIT_FOR(!r)

         QVERIFY(r);
         fakeServer->deleteLater();
     }


     void testCheckMemoRequiredWithPathPaymentStrictSendOperationNoMemo(){
         FakeServer* fakeServer = new FakeServer();

         fakeServer->addPost("/transactions",successTransactionResponse);
         m_server = new Server(fakeServer->baseUrl());

         KeyPair* source = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));
         Account* account = new Account(source, 1L);
         Transaction *transaction =  Transaction::Builder(AccountConverter().enableMuxed(), account)
                 .addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED), new AssetTypeNative(), "10"))
                 .addOperation(PathPaymentStrictReceiveOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(PathPaymentStrictSendOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_C), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(AccountMergeOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED)))
                 .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                 .setBaseFee(100)
                 .build();

         transaction->sign(source);

         SubmitTransactionResponse *r=nullptr;

         QMetaObject::Connection c = QObject::connect(m_server,&Server::transactionError,[&r](SubmitTransactionResponse *response){
             r = response;
         });
         m_server->submitTransaction(transaction);

         WAIT_FOR(!r)


         QVERIFY(r);//r is set because we received an error

         r=nullptr;

         m_server->submitTransaction(feeBump(transaction));
         WAIT_FOR(!r)

         QVERIFY(r);
         fakeServer->deleteLater();
     }

     void testCheckMemoRequiredWithAccountMergeOperationNoMemo() {
         FakeServer* fakeServer = new FakeServer();

         fakeServer->addPost("/transactions",successTransactionResponse);
         m_server = new Server(fakeServer->baseUrl());

         KeyPair* source = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));
         Account* account = new Account(source, 1L);
         Transaction *transaction =  Transaction::Builder(AccountConverter().enableMuxed(), account)
                 .addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED), new AssetTypeNative(), "10"))
                 .addOperation(PathPaymentStrictReceiveOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(PathPaymentStrictSendOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(AccountMergeOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_D)))
                 .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                 .setBaseFee(100)
                 .build();

         transaction->sign(source);

         SubmitTransactionResponse *r=nullptr;

         QMetaObject::Connection c = QObject::connect(m_server,&Server::transactionError,[&r](SubmitTransactionResponse *response){
             r = response;
         });
         m_server->submitTransaction(transaction);

         WAIT_FOR(!r)


         QVERIFY(r);//r is set because we received an error

         r=nullptr;

         m_server->submitTransaction(feeBump(transaction));
         WAIT_FOR(!r)

         QVERIFY(r);
         fakeServer->deleteLater();
     }

     void testCheckMemoRequiredTwoOperationsWithSameDestination(){
         FakeServer* fakeServer = new FakeServer();

         fakeServer->addPost("/transactions",successTransactionResponse);
         m_server = new Server(fakeServer->baseUrl());

         KeyPair* source = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));
         Account* account = new Account(source, 1L);
         Transaction *transaction =  Transaction::Builder(AccountConverter().enableMuxed(), account)
                 .addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED), new AssetTypeNative(), "10"))
                 .addOperation(PathPaymentStrictReceiveOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_MEMO_REQUIRED), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(PathPaymentStrictSendOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_C), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(AccountMergeOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_D)))
                 .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                 .setBaseFee(100)
                 .build();

         transaction->sign(source);

         SubmitTransactionResponse *r=nullptr;

         QMetaObject::Connection c = QObject::connect(m_server,&Server::transactionError,[&r](SubmitTransactionResponse *response){
             r = response;
         });
         m_server->submitTransaction(transaction);

         WAIT_FOR(!r)


         QVERIFY(r);//r is set because we received an error

         r=nullptr;

         m_server->submitTransaction(feeBump(transaction));
         WAIT_FOR(!r)

         QVERIFY(r);
         fakeServer->deleteLater();
     }


     void testCheckMemoRequiredNoDestinationOperation() {
         FakeServer* fakeServer = new FakeServer();

         fakeServer->addPost("/transactions",successTransactionResponse);
         m_server = new Server(fakeServer->baseUrl());

         KeyPair* source = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));
         Account* account = new Account(source, 1L);
         Transaction *transaction =  Transaction::Builder(AccountConverter().enableMuxed(), account)
                 .addOperation(ManageDataOperation::create("Hello", "Stellar"))
                 .addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_A), new AssetTypeNative(), "10"))
                 .addOperation(PathPaymentStrictReceiveOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_A), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(PathPaymentStrictSendOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_C), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(AccountMergeOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_D)))
                 .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                 .setBaseFee(100)
                 .build();

         transaction->sign(source);

         SubmitTransactionResponse *r=nullptr;

         QMetaObject::Connection c = QObject::connect(m_server,&Server::transactionError,[&r](SubmitTransactionResponse *response){
             r = response;
         });
         m_server->submitTransaction(transaction);

         WAIT_FOR(!r)


         QVERIFY(r);//r is set because we received an error

         r=nullptr;

         m_server->submitTransaction(feeBump(transaction));
         WAIT_FOR(!r)

         QVERIFY(r);
         fakeServer->deleteLater();
     }


     void testCheckMemoRequiredAccountNotFound(){
         FakeServer* fakeServer = new FakeServer();

         fakeServer->addPost("/transactions",successTransactionResponse);
         fakeServer->addGet("/accounts/"+DESTINATION_ACCOUNT_NO_FOUND,resourceMissingResponse,"404 Not Found");
         m_server = new Server(fakeServer->baseUrl());

         KeyPair* source = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));
         Account* account = new Account(source, 1L);
         Transaction *transaction =  Transaction::Builder(AccountConverter().enableMuxed(), account)
                 .addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_FOUND), new AssetTypeNative(), "10"))
                 .addOperation(PathPaymentStrictReceiveOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_FOUND), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(PathPaymentStrictSendOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_FOUND), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(AccountMergeOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_FOUND)))
                 .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                 .setBaseFee(100)
                 .build();

         transaction->sign(source);

         SubmitTransactionResponse *r=nullptr;

         QMetaObject::Connection c = QObject::connect(m_server,&Server::transactionResponse,[&r](SubmitTransactionResponse *response){
             r = response;
         });
         m_server->submitTransaction(transaction);

         WAIT_FOR(!r)



         QVERIFY(r);
         fakeServer->deleteLater();

     }

     void testCheckMemoRequiredAccountNotFoundBump(){
         FakeServer* fakeServer = new FakeServer();

         fakeServer->addPost("/transactions",successTransactionResponse);
         fakeServer->addGet("/accounts/"+DESTINATION_ACCOUNT_NO_FOUND,resourceMissingResponse,"404 Not Found");
         m_server = new Server(fakeServer->baseUrl());

         KeyPair* source = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));
         Account* account = new Account(source, 1L);
         Transaction *transaction =  Transaction::Builder(AccountConverter().enableMuxed(), account)
                 .addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_FOUND), new AssetTypeNative(), "10"))
                 .addOperation(PathPaymentStrictReceiveOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_FOUND), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(PathPaymentStrictSendOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_FOUND), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(AccountMergeOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_NO_FOUND)))
                 .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                 .setBaseFee(100)
                 .build();

         transaction->sign(source);

         SubmitTransactionResponse *r=nullptr;

         QMetaObject::Connection c = QObject::connect(m_server,&Server::transactionResponse,[&r](SubmitTransactionResponse *response){
             r = response;
         });

         m_server->submitTransaction(feeBump(transaction));

         WAIT_FOR(!r)

         QVERIFY(r);
         fakeServer->deleteLater();
     }


     void testCheckMemoRequiredFetchAccountError() {
         FakeServer* fakeServer = new FakeServer();

         fakeServer->addPost("/transactions",successTransactionResponse);
         fakeServer->addGet("/accounts/"+DESTINATION_ACCOUNT_NO_FOUND,resourceMissingResponse,"404 Not Found");
         fakeServer->addGet("/accounts/"+DESTINATION_ACCOUNT_MEMO_REQUIRED_A,memoRequiredResponse);
         fakeServer->addGet("/accounts/"+DESTINATION_ACCOUNT_MEMO_REQUIRED_B,memoRequiredResponse);
         fakeServer->addGet("/accounts/"+DESTINATION_ACCOUNT_MEMO_REQUIRED_C,memoRequiredResponse);
         fakeServer->addGet("/accounts/"+DESTINATION_ACCOUNT_MEMO_REQUIRED_D,memoRequiredResponse);

         m_server = new Server(fakeServer->baseUrl());

         KeyPair* source = KeyPair::fromSecretSeed(QString("SDQXFKA32UVQHUTLYJ42N56ZUEM5PNVVI4XE7EA5QFMLA2DHDCQX3GPY"));
         Account* account = new Account(source, 1L);
         Transaction *transaction =  Transaction::Builder(AccountConverter().enableMuxed(), account)
                 .addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_FETCH_ERROR), new AssetTypeNative(), "10"))
                 .addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_A), new AssetTypeNative(), "10"))
                 .addOperation(PaymentOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_B), new AssetTypeNative(), "10"))
                 .addOperation(PathPaymentStrictReceiveOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_C), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(PathPaymentStrictSendOperation::create(new AssetTypeNative(), "10", KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_D), new AssetTypeCreditAlphaNum4("BTC", QString("GA7GYB3QGLTZNHNGXN3BMANS6TC7KJT3TCGTR763J4JOU4QHKL37RVV2")), "5"))
                 .addOperation(AccountMergeOperation::create(KeyPair::fromAccountId(DESTINATION_ACCOUNT_MEMO_REQUIRED_D)))
                 .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                 .setBaseFee(100)
                 .build();

         transaction->sign(source);

         SubmitTransactionResponse *r=nullptr;

         QMetaObject::Connection c = QObject::connect(m_server,&Server::transactionError,[&r](SubmitTransactionResponse *response){
             r = response;
         });
         m_server->submitTransaction(transaction);         
         WAIT_FOR(!r)


         QVERIFY(r);//r is set because we received an error

         r=nullptr;

         m_server->submitTransaction(feeBump(transaction));        
         WAIT_FOR(!r)

         QVERIFY(r);
         fakeServer->deleteLater();
     }




};

ADD_TEST(ServerTest)

#endif // SERVERTEST_H
