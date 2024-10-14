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
#ifndef STELLAR_SKIP_LIVE_TESTS
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
         m_server = new Server("http://localhost:8080");

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


     void testCheckMemoRequiredWithMemoIdAddress()
     {
         FakeServer* fakeServer = new FakeServer();

         fakeServer->addPost("/transactions",successTransactionResponse);
         m_server = new Server("http://localhost:8080");

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
         m_server = new Server("http://localhost:8080");

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
         m_server = new Server("http://localhost:8080");

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
         m_server = new Server("http://localhost:8080");

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
         m_server = new Server("http://localhost:8080");

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
         m_server = new Server("http://localhost:8080");

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
         m_server = new Server("http://localhost:8080");

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
         m_server = new Server("http://localhost:8080");

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
         m_server = new Server("http://localhost:8080");

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
         m_server = new Server("http://localhost:8080");

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

         m_server = new Server("http://localhost:8080");

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
