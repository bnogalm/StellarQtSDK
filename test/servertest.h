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


private slots:

    void initTestCase()
    {
        qDebug() <<"qtcpp stellar client name:"<<STELLAR_QT_SDK_CLIENT_NAME;
        qDebug() <<"qtcpp stellar client version:"<< STELLAR_QT_SDK_CLIENT_VERSION;
        this->setUp();
    }
    void cleanupTestCase()
    {

    }

    void testBuildTransaction()
    {
        qint64 sequenceNumber=5026524780560386;
        KeyPair *source = KeyPair::fromSecretSeed(QString("SDPK5IBB57JY5SMMBUBOFSAHNNCETCZWLTKKDPWBDYXBX3B4ZAFOQYD5"));
        KeyPair *destination = KeyPair::fromSecretSeed(QByteArray::fromHex("268486538a268486538f268486538a268486538f268486538a268486538feedd"));


        Account* account = new Account(source, sequenceNumber);
        Transaction::Builder *builder = new Transaction::Builder(account);
        builder->addOperation(new CreateAccountOperation(destination, "2000"))
                .addMemo(Memo::text("Hello world!"))
                .setTimeout(Transaction::Builder::TIMEOUT_INFINITE);

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
         m_server->submitTransaction(this->m_transaction);

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
         Transaction::Builder *builder = new Transaction::Builder(account);
         builder->addOperation(new CreateAccountOperation(destination, "2000"))
                 .addMemo(Memo::text("Hello world!")).setTimeout(Transaction::Builder::TIMEOUT_INFINITE);

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
         m_server->submitTransaction(this->m_transactionWrong);
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

};

ADD_TEST(ServerTest)

#endif // SERVERTEST_H
