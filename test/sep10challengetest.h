#ifndef SEP10CHALLENGETEST_H
#define SEP10CHALLENGETEST_H
#include <QObject>

#include "src/util.h"
#include "src/keypair.h"
#include "src/transaction.h"

#include <QtTest>
#include "testcollector.h"
#include "sep10challenge.h"
#include "account.h"
#include "setoptionsoperation.h"
#include "bumpsequenceoperation.h"
#include <QDateTime>

class Sep10ChallengeTest: public QObject
{
    Q_OBJECT
private slots:
    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }
    void testChallenge() {
        KeyPair* server = KeyPair::random();
        KeyPair* client = KeyPair::random();



        QString domainName = "example.com";
        QString webAuthDomain = "example.com";
        qint64 nowPrev = QDateTime::currentMSecsSinceEpoch() / 1000L;
        Transaction* transaction = Sep10Challenge::buildChallengeTx(
                    server,
                    client->getAccountId(),
                    domainName,
                    webAuthDomain,
                    300
                    );

        qint64 now = QDateTime::currentMSecsSinceEpoch() / 1000L;
        qint64 end = now + 300;
        QCOMPARE(transaction->getNetwork(),Network::current());
        QCOMPARE(transaction->getFee(),200);
        //QCOMPARE(transaction->getTimeBounds()->getMaxTime(),end);
        QVERIFY(transaction->getTimeBounds()->getMaxTime()>=nowPrev+300);
        QVERIFY(transaction->getTimeBounds()->getMaxTime()<=end);
        QVERIFY(transaction->getTimeBounds()->getMinTime()<=now);
        QVERIFY(transaction->getTimeBounds()->getMinTime()>=nowPrev);
        QCOMPARE(transaction->getSourceAccount(),server->getAccountId());
        QCOMPARE(transaction->getSequenceNumber(),0);

        QCOMPARE(transaction->getOperations().length() , 2);


        ManageDataOperation* homeDomainOp = (ManageDataOperation*) transaction->getOperations().at(0);
        QCOMPARE(homeDomainOp->getSourceAccount(),client->getAccountId());
        QCOMPARE(homeDomainOp->getName(),domainName + " auth");

        QCOMPARE(homeDomainOp->getValue().length(), 64);

        QVERIFY(QByteArray::fromBase64(homeDomainOp->getValue()).size()>0);

        ManageDataOperation* webAuthDomainOp = (ManageDataOperation*) transaction->getOperations().at(1);
        QCOMPARE(webAuthDomainOp->getSourceAccount(),server->getAccountId());
        QCOMPARE(webAuthDomainOp->getName(),"web_auth_domain");

        QCOMPARE(webAuthDomainOp->getValue(), domainName.toUtf8());

        QVERIFY(QByteArray::fromBase64(webAuthDomainOp->getValue()).size()>0);


        QCOMPARE(transaction->getSignatures().size(),1);
        QVERIFY(
                    server->verify(transaction->hash(), transaction->getSignatures().at(0).signature.binary())
                    );
    }


    void testNewChallengeRejectsMuxedClientAccount(){
        try {
            KeyPair* server = KeyPair::random();

            QString domainName = "example.com";
            QString webAuthDomain = "example.com";

            Sep10Challenge::buildChallengeTx(
                        server,
                        "MCAAAAAAAAAAAAB7BQ2L7E5NBWMXDUCMZSIPOBKRDSBYVLMXGSSKF6YNPIB7Y77ITKNOG",
                        domainName,
                        webAuthDomain,
                        300
                        );
            QFAIL("Missing exception");
        } catch (std::runtime_error e) {
            //"Version byte is invalid"
        }

    }


    void testReadChallengeTransactionValidSignedByServer(){
        KeyPair* server = KeyPair::random();
        KeyPair* client = KeyPair::random();

        QString domainName = "example.com";
        QString webAuthDomain = "example.com";

        Transaction* transaction = Sep10Challenge::buildChallengeTx(
                    server,
                    client->getAccountId(),
                    domainName,
                    webAuthDomain,
                    300
                    );

        Sep10Challenge::ChallengeTransaction* challengeTransaction = Sep10Challenge::readChallengeTransaction(transaction->toEnvelopeXdrBase64(), server->getAccountId(), domainName, webAuthDomain);
        QVERIFY(challengeTransaction->equals(new Sep10Challenge::ChallengeTransaction(transaction, client->getAccountId(), domainName)));
    }


    void testReadChallengeTransactionAcceptsBothV0AndV1()  {
        KeyPair* server = KeyPair::random();
        KeyPair* client = KeyPair::random();

        QString domainName = "example.com";
        QString webAuthDomain = "example.com";

        Transaction* transaction = Sep10Challenge::buildChallengeTx(
                    server,
                    client->getAccountId(),
                    domainName,
                    webAuthDomain,
                    300
                    );

        transaction->setEnvelopeType(stellar::EnvelopeType::ENVELOPE_TYPE_TX_V0);
        stellar::TransactionEnvelope v0 = transaction->toEnvelopeXdr();
        QString v0Base64 = transaction->toEnvelopeXdrBase64();
        QCOMPARE(v0.type,stellar::EnvelopeType::ENVELOPE_TYPE_TX_V0);
        Sep10Challenge::ChallengeTransaction* v0ChallengeTransaction = Sep10Challenge::readChallengeTransaction(
                    v0Base64,
                    server->getAccountId(),
                    domainName,
                    webAuthDomain
                    );

        transaction->setEnvelopeType(stellar::EnvelopeType::ENVELOPE_TYPE_TX);
        stellar::TransactionEnvelope v1 = transaction->toEnvelopeXdr();
        QString v1Base64 = transaction->toEnvelopeXdrBase64();
        QCOMPARE(v1.type,stellar::EnvelopeType::ENVELOPE_TYPE_TX);
        Sep10Challenge::ChallengeTransaction* v1ChallengeTransaction = Sep10Challenge::readChallengeTransaction(
                    v1Base64,
                    server->getAccountId(),
                    domainName,
                    webAuthDomain
                    );

        QVERIFY(v0ChallengeTransaction->equals(v1ChallengeTransaction));


        Sep10Challenge::ChallengeTransaction* challengeTransactionV0 = Sep10Challenge::readChallengeTransaction(
                    v0Base64,
                    server->getAccountId(),
                    domainName,
                    webAuthDomain
                    );
        QVERIFY(challengeTransactionV0->equals(new Sep10Challenge::ChallengeTransaction(transaction, client->getAccountId(), domainName)));
        Sep10Challenge::ChallengeTransaction* challengeTransactionV1 = Sep10Challenge::readChallengeTransaction(
                    v1Base64,
                    server->getAccountId(),
                    domainName,
                    webAuthDomain
                    );
        QVERIFY(challengeTransactionV1->equals(new Sep10Challenge::ChallengeTransaction(transaction, client->getAccountId(), domainName)));
    }


    void testReadChallengeTransactionRejectsMuxedServer(){
        KeyPair* server = KeyPair::random();
        KeyPair* client = KeyPair::random();

        QString domainName = "example.com";
        QString webAuthDomain = "example.com";


        Transaction* transaction = Sep10Challenge::buildChallengeTx(
                    server,
                    client->getAccountId(),
                    domainName,
                    webAuthDomain,
                    300
                    );

        try {
            Sep10Challenge::readChallengeTransaction(
                        transaction->toEnvelopeXdrBase64(),
                        "MCAAAAAAAAAAAAB7BQ2L7E5NBWMXDUCMZSIPOBKRDSBYVLMXGSSKF6YNPIB7Y77ITKNOG",
                        domainName,
                        webAuthDomain
                        );
            QFAIL("Missing exception");
        } catch (std::runtime_error e) {
            //"Version byte is invalid"
        }
    }

    void testReadChallengeTransactionRejectsMuxedClient() {
        KeyPair* server = KeyPair::random();
        KeyPair* client = KeyPair::random();

        QString domainName = "example.com";
        QString webAuthDomain = "example.com";

        Transaction* transaction = Sep10Challenge::buildChallengeTx(
                    server,
                    client->getAccountId(),
                    domainName,
                    webAuthDomain,
                    300
                    );
        QVector<Operation*> operations = transaction->getOperations();
        operations[0]->setSourceAccount("MCAAAAAAAAAAAAB7BQ2L7E5NBWMXDUCMZSIPOBKRDSBYVLMXGSSKF6YNPIB7Y77ITKNOG");

        Account* account = new Account(KeyPair::fromAccountId(transaction->getSourceAccount()), transaction->getSequenceNumber());
        Transaction* withMuxedClient = Transaction::Builder(account,transaction->getNetwork())
                .setBaseFee(transaction->getFee())
                .addOperation(operations[0])
                .addMemo(transaction->getMemo())
                .addTimeBounds(transaction->getTimeBounds())
                .build();


        withMuxedClient->getSignatures().append(transaction->getSignatures());

        try {
            Sep10Challenge::readChallengeTransaction(
                        withMuxedClient->toEnvelopeXdrBase64(),
                        "MCAAAAAAAAAAAAB7BQ2L7E5NBWMXDUCMZSIPOBKRDSBYVLMXGSSKF6YNPIB7Y77ITKNOG",
                        domainName,
                        webAuthDomain
                        );
            QFAIL("Missing exception");
        } catch (std::runtime_error e) {
            //"invalid address length: MCAAAAAAAAAAAAB7BQ2L7E5NBWMXDUCMZSIPOBKRDSBYVLMXGSSKF6YNPIB7Y77ITKNOG"
        }
    }

    void testReadChallengeTransactionValidSignedByServerAndClient() {
        KeyPair* server = KeyPair::random();
        KeyPair* client = KeyPair::random();

        QString domainName = "example.com";
        QString webAuthDomain = "example.com";


        Transaction* transaction = Sep10Challenge::buildChallengeTx(
                    server,
                    client->getAccountId(),
                    domainName,
                    webAuthDomain,
                    300
                    );

        transaction->sign(client);

        Sep10Challenge::ChallengeTransaction* challengeTransaction = Sep10Challenge::readChallengeTransaction(transaction->toEnvelopeXdrBase64(), server->getAccountId(), domainName, webAuthDomain);
        QVERIFY(challengeTransaction->equals(new Sep10Challenge::ChallengeTransaction(transaction, client->getAccountId(), domainName)));
    }

    void testReadChallengeTransactionInvalidNotSignedByServer() {
        KeyPair* server = KeyPair::random();
        KeyPair* client = KeyPair::random();
        qint64 now = QDateTime::currentMSecsSinceEpoch() / 1000L;
        qint64 end = now + 300;

        QString domainName = "example.com";
        QString webAuthDomain = "example.com";

        QByteArray nonce(48,'\0');

        QRandomGenerator *r = QRandomGenerator::global();
        r->fillRange(reinterpret_cast<quint32*>(nonce.data()),48/static_cast<int>(sizeof(quint32)));

        QByteArray encodedNonce = nonce.toBase64(QByteArray::Base64Option::Base64UrlEncoding| QByteArray::OmitTrailingEquals);

        Account* sourceAccount = new Account(KeyPair::fromAccountId(server->getAccountId()), -1L);
        ManageDataOperation* manageDataOperation1 = ManageDataOperation::create(domainName + " auth", encodedNonce);
        manageDataOperation1->setSourceAccount(client->getAccountId());


        Transaction* transaction = Transaction::Builder(sourceAccount)
                .setBaseFee(100)
                .addOperation(manageDataOperation1)
                .addMemo(Memo::none())
                .addTimeBounds(new TimeBounds(0,end))
                .build();

        transaction->sign(client);
        try {
            Sep10Challenge::readChallengeTransaction(transaction->toEnvelopeXdrBase64(), server->getAccountId(), domainName, webAuthDomain);
            QFAIL("Missing exception");
        } catch (std::runtime_error e) {
            //Transaction not signed by server
        }
    }


    void testReadChallengeTransactionInvalidCorrupted() {
        KeyPair* server = KeyPair::random();
        KeyPair* client = KeyPair::random();

        QString domainName = "example.com";
        QString webAuthDomain = "example.com";

        Transaction* transaction = Sep10Challenge::buildChallengeTx(
                    server,
                    client->getAccountId(),
                    domainName,
                    webAuthDomain,
                    300
                    );

        QString challenge = transaction->toEnvelopeXdrBase64().replace("A", "B");

        try {
            Sep10Challenge::readChallengeTransaction(challenge, server->getAccountId(),  domainName, webAuthDomain);
            QFAIL("Missing exception");
        } catch (std::runtime_error) {

        }
    }


    void testReadChallengeTransactionInvalidServerAccountIDMismatch() {
        KeyPair* server = KeyPair::random();
        KeyPair* client = KeyPair::random();


        QString domainName = "example.com";
        QString webAuthDomain = "example.com";
        Transaction* transaction = Sep10Challenge::buildChallengeTx(
                    server,
                    client->getAccountId(),
                    domainName,
                    webAuthDomain,
                    300
                    );

        // assertThrows requires Java 8+
        try {
            Sep10Challenge::readChallengeTransaction(transaction->toEnvelopeXdrBase64(), KeyPair::random()->getAccountId(), domainName, webAuthDomain);
            QFAIL("Missing exception");
        } catch (std::runtime_error) {
            //"Transaction source account is not equal to server's account."
        }
    }


    void testReadChallengeTransactionInvalidSeqNoNotZero() {
        KeyPair* server = KeyPair::random();
        KeyPair* client = KeyPair::random();
        qint64 now = QDateTime::currentMSecsSinceEpoch() / 1000L;
        qint64 end = now + 300;

        QString domainName = "example.com";
        QString webAuthDomain = "example.com";
        QByteArray nonce(48,'\0');

        QRandomGenerator *r = QRandomGenerator::global();
        r->fillRange(reinterpret_cast<quint32*>(nonce.data()),48/static_cast<int>(sizeof(quint32)));

        QByteArray encodedNonce = nonce.toBase64(QByteArray::Base64Option::Base64UrlEncoding| QByteArray::OmitTrailingEquals);

        Account* sourceAccount = new Account(KeyPair::fromAccountId(server->getAccountId()), 100L);
        ManageDataOperation* manageDataOperation1 = ManageDataOperation::create(domainName + " auth", encodedNonce);
        manageDataOperation1->setSourceAccount(client->getAccountId());

        Transaction* transaction = Transaction::Builder(sourceAccount)
                .setBaseFee(100)
                .addTimeBounds(new TimeBounds(now,end))
                .addMemo(Memo::none())
                .addOperation(manageDataOperation1)
                .build();
        transaction->sign(server);
        QString challenge = transaction->toEnvelopeXdrBase64();

        try {
            Sep10Challenge::readChallengeTransaction(challenge, server->getAccountId(), domainName , webAuthDomain);
            QFAIL("Missing exception");
        } catch (std::runtime_error) {
            //"The transaction sequence number should be zero."
        }
    }

    void testReadChallengeTransactionInvalidTimeboundsInfinite(){
        KeyPair* server = KeyPair::random();
        KeyPair* client = KeyPair::random();
        qint64 now = QDateTime::currentMSecsSinceEpoch() / 1000L;
        qint64 end = 0;

        QString domainName = "example.com";
        QString webAuthDomain = "example.com";
        QByteArray nonce(48,'\0');

        QRandomGenerator *r = QRandomGenerator::global();
        r->fillRange(reinterpret_cast<quint32*>(nonce.data()),48/static_cast<int>(sizeof(quint32)));

        QByteArray encodedNonce = nonce.toBase64(QByteArray::Base64Option::Base64UrlEncoding| QByteArray::OmitTrailingEquals);

        Account* sourceAccount = new Account(server, -1L);
        ManageDataOperation* operation = ManageDataOperation::create(domainName + " auth", encodedNonce);
        operation->setSourceAccount(client->getAccountId());


        Transaction* transaction = Transaction::Builder(sourceAccount)
                .setBaseFee(100)
                .addTimeBounds(new TimeBounds(now,end))
                .addMemo(Memo::none())
                .addOperation(operation)
                .build();
        transaction->sign(server);
        QString challenge = transaction->toEnvelopeXdrBase64();



        try {
            Sep10Challenge::readChallengeTransaction(challenge, server->getAccountId(), domainName ,webAuthDomain);
            QFAIL("Missing exception");
        } catch (std::runtime_error) {
            //"Transaction requires non-infinite timebounds."
        }
    }


    void testReadChallengeTransactionInvalidNoTimeBounds(){
        KeyPair* server = KeyPair::random();
        KeyPair* client = KeyPair::random();


        QString domainName = "example.com";
        QString webAuthDomain = "example.com";
        QByteArray nonce(48,'\0');

        QRandomGenerator *r = QRandomGenerator::global();
        r->fillRange(reinterpret_cast<quint32*>(nonce.data()),48/static_cast<int>(sizeof(quint32)));

        QByteArray encodedNonce = nonce.toBase64(QByteArray::Base64Option::Base64UrlEncoding| QByteArray::OmitTrailingEquals);

        Account* sourceAccount = new Account(server, -1L);
        ManageDataOperation* operation = ManageDataOperation::create(domainName + " auth", encodedNonce);
        operation->setSourceAccount(client->getAccountId());
        try {
            Transaction* transaction = Transaction::Builder(sourceAccount)
                    .setBaseFee(100)
                    .addMemo(Memo::none())
                    .addOperation(operation)
                    .build();
            Q_UNUSED(transaction)
            //            transaction->sign(server);
            //            QString challenge = transaction->toEnvelopeXdrBase64();
            //            Sep10Challenge::readChallengeTransaction(challenge, server->getAccountId(), domainName);
            QFAIL("Missing exception");
        } catch (std::runtime_error) {
            //"Transaction requires timebounds."
        }
    }


    void testReadChallengeTransactionInvalidTimeBoundsTooEarly(){
        KeyPair* server = KeyPair::random();
        KeyPair* client = KeyPair::random();
        qint64 now = QDateTime::currentMSecsSinceEpoch() / 1000L + 300;
        qint64 end = now + 300;

        QString domainName = "example.com";
        QString webAuthDomain = "example.com";
        QByteArray nonce(48,'\0');

        QRandomGenerator *r = QRandomGenerator::global();
        r->fillRange(reinterpret_cast<quint32*>(nonce.data()),48/static_cast<int>(sizeof(quint32)));

        QByteArray encodedNonce = nonce.toBase64(QByteArray::Base64Option::Base64UrlEncoding| QByteArray::OmitTrailingEquals);

        Account* sourceAccount = new Account(server, -1L);
        ManageDataOperation* operation = ManageDataOperation::create(domainName + " auth", encodedNonce);
        operation->setSourceAccount(client->getAccountId());


        Transaction* transaction = Transaction::Builder(sourceAccount)
                .setBaseFee(100)
                .addMemo(Memo::none())
                .addTimeBounds(new TimeBounds(now,end))
                .addOperation(operation)
                .build();
        transaction->sign(server);
        QString challenge = transaction->toEnvelopeXdrBase64();



        try {
            Sep10Challenge::readChallengeTransaction(challenge, server->getAccountId(), domainName , webAuthDomain);
            QFAIL("Missing exception");
        } catch (std::runtime_error) {
            //"Transaction is not within range of the specified timebounds."
        }

    }

    void testReadChallengeTransactionInvalidTimeBoundsTooLate(){
        KeyPair* server = KeyPair::random();
        KeyPair* client = KeyPair::random();
        qint64 now = QDateTime::currentMSecsSinceEpoch() / 1000L - 600;
        qint64 end = now + 300;

        QString domainName = "example.com";
        QString webAuthDomain = "example.com";
        QByteArray nonce(48,'\0');

        QRandomGenerator *r = QRandomGenerator::global();
        r->fillRange(reinterpret_cast<quint32*>(nonce.data()),48/static_cast<int>(sizeof(quint32)));

        QByteArray encodedNonce = nonce.toBase64(QByteArray::Base64Option::Base64UrlEncoding| QByteArray::OmitTrailingEquals);

        Account* sourceAccount = new Account(server, -1L);
        ManageDataOperation* operation = ManageDataOperation::create(domainName + " auth", encodedNonce);
        operation->setSourceAccount(client->getAccountId());


        Transaction* transaction = Transaction::Builder(sourceAccount)
                .setBaseFee(100)
                .addMemo(Memo::none())
                .addTimeBounds(new TimeBounds(now,end))
                .addOperation(operation)
                .build();
        transaction->sign(server);
        QString challenge = transaction->toEnvelopeXdrBase64();



        try {
            Sep10Challenge::readChallengeTransaction(challenge, server->getAccountId(), domainName , webAuthDomain);
            QFAIL("Missing exception");
        } catch (std::runtime_error) {
            //Transaction is not within range of the specified timebounds."
        }
    }

    void testReadChallengeTransactionOperationShouldHaveASourceAcount(){
        KeyPair* server = KeyPair::random();

        qint64 now = QDateTime::currentMSecsSinceEpoch() / 1000L;
        qint64 end = now + 300;

        QString domainName = "example.com";
        QString webAuthDomain = "example.com";
        QByteArray nonce(48,'\0');

        QRandomGenerator *r = QRandomGenerator::global();
        r->fillRange(reinterpret_cast<quint32*>(nonce.data()),48/static_cast<int>(sizeof(quint32)));

        QByteArray encodedNonce = nonce.toBase64(QByteArray::Base64Option::Base64UrlEncoding| QByteArray::OmitTrailingEquals);

        Account* sourceAccount = new Account(server, -1L);
        ManageDataOperation* operation1 = ManageDataOperation::create(domainName + " auth", encodedNonce);

        Transaction* transaction = Transaction::Builder(sourceAccount)
                .setBaseFee(100)
                .addMemo(Memo::none())
                .addTimeBounds(new TimeBounds(now,end))
                .addOperation(operation1)
                .build();
        transaction->sign(server);
        QString challenge = transaction->toEnvelopeXdrBase64();

        try {
            Sep10Challenge::readChallengeTransaction(challenge, server->getAccountId(), domainName , webAuthDomain);
            QFAIL("Missing exception");
        } catch (std::runtime_error) {
            //"Operation should have a source account."
        }
    }


    void testReadChallengeTransactionInvalidOperationWrongType(){
        KeyPair* server = KeyPair::random();
        KeyPair* client = KeyPair::random();
        qint64 now = QDateTime::currentMSecsSinceEpoch() / 1000L;
        qint64 end = now + 300;

        QString domainName = "example.com";
        QString webAuthDomain = "example.com";
        Account* sourceAccount = new Account(server, -1L);
        SetOptionsOperation* setOptionsOperation = SetOptionsOperation::create();
        setOptionsOperation->setSourceAccount(client->getAccountId());

        Transaction* transaction = Transaction::Builder(sourceAccount)
                .setBaseFee(100)
                .addMemo(Memo::none())
                .addTimeBounds(new TimeBounds(now,end))
                .addOperation(setOptionsOperation)
                .build();
        transaction->sign(server);
        QString challenge = transaction->toEnvelopeXdrBase64();

        try {
            Sep10Challenge::readChallengeTransaction(challenge, server->getAccountId(), domainName ,webAuthDomain);
            QFAIL("Missing exception");
        } catch (std::runtime_error) {
            //"Operation type should be ManageData."
        }
    }

    void testReadChallengeTransactionInvalidOperationNoSourceAccount(){
        KeyPair* server = KeyPair::random();

        qint64 now = QDateTime::currentMSecsSinceEpoch() / 1000L;
        qint64 end = now + 300;
        QString domainName = "example.com";
        QString webAuthDomain = "example.com";
        TimeBounds* timeBounds = new TimeBounds(now, end);

        QByteArray nonce(48,'\0');

        QRandomGenerator *r = QRandomGenerator::global();
        r->fillRange(reinterpret_cast<quint32*>(nonce.data()),48/static_cast<int>(sizeof(quint32)));

        QByteArray encodedNonce = nonce.toBase64(QByteArray::Base64Option::Base64UrlEncoding| QByteArray::OmitTrailingEquals);

        Account* sourceAccount = new Account(server, -1L);
        ManageDataOperation* manageDataOperation1 = ManageDataOperation::create(domainName + " auth", encodedNonce);

        Transaction* transaction = Transaction::Builder(sourceAccount)
                .setBaseFee(100)
                .addMemo(Memo::none())
                .addTimeBounds(timeBounds)
                .addOperation(manageDataOperation1)
                .build();
        transaction->sign(server);
        QString challenge = transaction->toEnvelopeXdrBase64();

        try {
            Sep10Challenge::readChallengeTransaction(challenge, server->getAccountId(), domainName , webAuthDomain);
            QFAIL("Missing exception");
        } catch (std::runtime_error e) {
            //"Operation should have a source account."
        }
    }


    void testReadChallengeTransactionInvalidDataValueWrongEncodedLength(){
        KeyPair* server = KeyPair::random();
        KeyPair* client = KeyPair::random();
        qint64 now = QDateTime::currentMSecsSinceEpoch() / 1000L;
        qint64 end = now + 300;
        QString domainName = "example.com";
        QString webAuthDomain = "example.com";
        TimeBounds* timeBounds = new TimeBounds(now, end);

        QByteArray nonce(32,'\0');

        QRandomGenerator *r = QRandomGenerator::global();
        r->fillRange(reinterpret_cast<quint32*>(nonce.data()),32/static_cast<int>(sizeof(quint32)));

        QByteArray encodedNonce = nonce.toBase64(QByteArray::Base64Option::Base64UrlEncoding| QByteArray::OmitTrailingEquals);

        Account* sourceAccount = new Account(server, -1L);
        ManageDataOperation* manageDataOperation1 = ManageDataOperation::create(domainName + " auth", encodedNonce);
        manageDataOperation1->setSourceAccount(client->getAccountId());

        Transaction* transaction = Transaction::Builder(sourceAccount)
                .setBaseFee(100)
                .addMemo(Memo::none())
                .addTimeBounds(timeBounds)
                .addOperation(manageDataOperation1)
                .build();
        transaction->sign(server);
        QString challenge = transaction->toEnvelopeXdrBase64();

        try {
            Sep10Challenge::readChallengeTransaction(challenge, server->getAccountId(), domainName, webAuthDomain);
            QFAIL("Missing exception");
        } catch (std::runtime_error e) {
            //"Random nonce encoded as base64 should be 64 bytes long."
        }

    }


    void testReadChallengeTransactionInvalidDataValueCorruptBase64(){
        KeyPair* server = KeyPair::random();
        KeyPair* client = KeyPair::random();
        qint64 now = QDateTime::currentMSecsSinceEpoch() / 1000L;
        qint64 end = now + 300;
        QString domainName = "example.com";
        QString webAuthDomain = "example.com";
        TimeBounds* timeBounds = new TimeBounds(now, end);

        QByteArray nonce(48,'\0');

        QRandomGenerator *r = QRandomGenerator::global();
        r->fillRange(reinterpret_cast<quint32*>(nonce.data()),48/static_cast<int>(sizeof(quint32)));

        QByteArray encodedNonce = nonce.toBase64(QByteArray::Base64Option::Base64UrlEncoding| QByteArray::OmitTrailingEquals);
        encodedNonce[10]='?';

        Account* sourceAccount = new Account(server, -1L);
        ManageDataOperation* manageDataOperation1 = ManageDataOperation::create(domainName + " auth", encodedNonce);
        manageDataOperation1->setSourceAccount(client->getAccountId());

        Transaction* transaction = Transaction::Builder(sourceAccount)
                .setBaseFee(100)
                .addMemo(Memo::none())
                .addTimeBounds(timeBounds)
                .addOperation(manageDataOperation1)
                .build();
        transaction->sign(server);
        QString challenge = transaction->toEnvelopeXdrBase64();

        try {
            Sep10Challenge::readChallengeTransaction(challenge, server->getAccountId(), domainName, webAuthDomain);
            QFAIL("Missing exception");
        } catch (std::runtime_error e) {
            //"Failed to decode random nonce provided in ManageData operation."
        }
    }


    void testReadChallengeTransactionInvalidDataValueWrongByteLength() {
        KeyPair* server = KeyPair::random();
        KeyPair* client = KeyPair::random();
        qint64 now = QDateTime::currentMSecsSinceEpoch() / 1000L;
        qint64 end = now + 300;
        QString domainName = "example.com";
        QString webAuthDomain = "example.com";
        TimeBounds* timeBounds = new TimeBounds(now, end);

        QByteArray nonce(47,'\0');

        QRandomGenerator *r = QRandomGenerator::global();
        r->fillRange(reinterpret_cast<quint32*>(nonce.data()),47/static_cast<int>(sizeof(quint32)));

        QByteArray encodedNonce = nonce.toBase64(QByteArray::Base64Option::Base64UrlEncoding| QByteArray::OmitTrailingEquals);

        Account* sourceAccount = new Account(server, -1L);
        ManageDataOperation* manageDataOperation1 = ManageDataOperation::create(domainName + " auth", encodedNonce);
        manageDataOperation1->setSourceAccount(client->getAccountId());

        Transaction* transaction = Transaction::Builder(sourceAccount)
                .setBaseFee(100)
                .addMemo(Memo::none())
                .addTimeBounds(timeBounds)
                .addOperation(manageDataOperation1)
                .build();
        transaction->sign(server);
        QString challenge = transaction->toEnvelopeXdrBase64();

        try {
            Sep10Challenge::readChallengeTransaction(challenge, server->getAccountId(), domainName, webAuthDomain);
            QFAIL("Missing exception");
        } catch (std::runtime_error e) {
            //"Random nonce before encoding as base64 should be 48 bytes long."
        }

    }


    void testReadChallengeTransactionInvalidDataValueIsNull() {
        KeyPair* server = KeyPair::random();
        KeyPair* client = KeyPair::random();
        qint64 now = QDateTime::currentMSecsSinceEpoch() / 1000L;
        qint64 end = now + 300;
        QString domainName = "example.com";
        QString webAuthDomain = "example.com";
        TimeBounds* timeBounds = new TimeBounds(now, end);

        Account* sourceAccount = new Account(server, -1L);
        ManageDataOperation* manageDataOperation1 = ManageDataOperation::create(domainName + " auth");
        manageDataOperation1->setSourceAccount(client->getAccountId());

        Transaction* transaction = Transaction::Builder(sourceAccount)
                .setBaseFee(100)
                .addMemo(Memo::none())
                .addTimeBounds(timeBounds)
                .addOperation(manageDataOperation1)
                .build();
        transaction->sign(server);
        QString challenge = transaction->toEnvelopeXdrBase64();
        try {
            Sep10Challenge::readChallengeTransaction(challenge, server->getAccountId(), domainName, webAuthDomain);
            QFAIL("Missing exception");
        } catch (std::runtime_error) {
            //"The transaction's operation value should not be null."
        }
    }

    void testReadChallengeTransactionValidAdditionalManageDataOpsWithSourceAccountSetToServerAccount()  {
        KeyPair* server = KeyPair::random();
        KeyPair* client = KeyPair::random();
        QString domainName = "example.com";
        QString webAuthDomain = "example.com";

        qint64 now = QDateTime::currentMSecsSinceEpoch()  / 1000L;
        qint64 end = now + 300;
        TimeBounds* timeBounds = new TimeBounds(now, end);
        QByteArray nonce(48,'\0');

        QRandomGenerator *r = QRandomGenerator::global();
        r->fillRange(reinterpret_cast<quint32*>(nonce.data()),48/static_cast<int>(sizeof(quint32)));

        QByteArray encodedNonce = nonce.toBase64(QByteArray::Base64Option::Base64UrlEncoding| QByteArray::OmitTrailingEquals);

        Account* sourceAccount = new Account(server, -1L);
        ManageDataOperation* manageDataOperation1 = ManageDataOperation::create(domainName + " auth", encodedNonce);
        manageDataOperation1->setSourceAccount(client->getAccountId());

        ManageDataOperation* manageDataOperation2 = ManageDataOperation::create("key", "value");
        manageDataOperation2->setSourceAccount(server->getAccountId());


        Transaction* transaction = Transaction::Builder(sourceAccount)
                .setBaseFee(100)
                .addMemo(Memo::none())
                .addTimeBounds(timeBounds)
                .addOperation(manageDataOperation1)
                .addOperation(manageDataOperation2)
                .build();
        transaction->sign(server);

        QString challenge = transaction->toEnvelopeXdrBase64();

        Sep10Challenge::ChallengeTransaction* challengeTransaction = Sep10Challenge::readChallengeTransaction(challenge, server->getAccountId(), domainName, webAuthDomain);
        QVERIFY(Sep10Challenge::ChallengeTransaction(transaction, client->getAccountId(), domainName).equals(challengeTransaction));
    }


    void testReadChallengeTransactionInvalidAdditionalManageDataOpsWithoutSourceAccountSetToServerAccount()  {
        KeyPair* server = KeyPair::random();
        KeyPair* client = KeyPair::random();
        QString domainName = "example.com";
        QString webAuthDomain = "example.com";

        qint64 now = QDateTime::currentMSecsSinceEpoch()  / 1000L;
        qint64 end = now + 300;
        TimeBounds* timeBounds = new TimeBounds(now, end);
        QByteArray nonce(48,'\0');

        QRandomGenerator *r = QRandomGenerator::global();
        r->fillRange(reinterpret_cast<quint32*>(nonce.data()),48/static_cast<int>(sizeof(quint32)));

        QByteArray encodedNonce = nonce.toBase64(QByteArray::Base64Option::Base64UrlEncoding| QByteArray::OmitTrailingEquals);

        Account* sourceAccount = new Account(server, -1L);
        ManageDataOperation* manageDataOperation1 = ManageDataOperation::create(domainName + " auth", encodedNonce);
        manageDataOperation1->setSourceAccount(client->getAccountId());

        ManageDataOperation* manageDataOperation2 = ManageDataOperation::create("key", "value");
        manageDataOperation2->setSourceAccount(client->getAccountId());//<--


        Transaction* transaction = Transaction::Builder(sourceAccount)
                .setBaseFee(100)
                .addMemo(Memo::none())
                .addTimeBounds(timeBounds)
                .addOperation(manageDataOperation1)
                .addOperation(manageDataOperation2)
                .build();
        transaction->sign(server);

        QString challenge = transaction->toEnvelopeXdrBase64();
        try {
            Sep10Challenge::readChallengeTransaction(challenge, server->getAccountId(), domainName, webAuthDomain);
            QFAIL("Missing exception");
        }
        catch(std::runtime_error)
        {
            //Subsequent operations are unrecognized
        }
    }

    void testReadChallengeTransactionInvalidAdditionalManageDataOpsWithSourceAccountSetToNull() {
        KeyPair* server = KeyPair::random();
        KeyPair* client = KeyPair::random();
        QString domainName = "example.com";
        QString webAuthDomain = "example.com";

        qint64 now = QDateTime::currentMSecsSinceEpoch()  / 1000L;
        qint64 end = now + 300;
        TimeBounds* timeBounds = new TimeBounds(now, end);
        QByteArray nonce(48,'\0');

        QRandomGenerator *r = QRandomGenerator::global();
        r->fillRange(reinterpret_cast<quint32*>(nonce.data()),48/static_cast<int>(sizeof(quint32)));

        QByteArray encodedNonce = nonce.toBase64(QByteArray::Base64Option::Base64UrlEncoding| QByteArray::OmitTrailingEquals);

        Account* sourceAccount = new Account(server, -1L);
        ManageDataOperation* manageDataOperation1 = ManageDataOperation::create(domainName + " auth", encodedNonce);
        manageDataOperation1->setSourceAccount(client->getAccountId());

        ManageDataOperation* manageDataOperation2 = ManageDataOperation::create("key", "value");


        Transaction* transaction = Transaction::Builder(sourceAccount)
                .setBaseFee(100)
                .addMemo(Memo::none())
                .addTimeBounds(timeBounds)
                .addOperation(manageDataOperation1)
                .addOperation(manageDataOperation2)
                .build();
        transaction->sign(server);

        QString challenge = transaction->toEnvelopeXdrBase64();
        try {
            Sep10Challenge::readChallengeTransaction(challenge, server->getAccountId(), domainName, webAuthDomain);
            QFAIL("Missing exception");
        }
        catch(std::runtime_error)
        {
            //"Operation should have a source account."
        }
    }
    void testReadChallengeTransactionInvalidAdditionalOpsOfOtherTypes(){
        KeyPair* server = KeyPair::random();
        KeyPair* client = KeyPair::random();
        QString domainName = "example.com";
        QString webAuthDomain = "example.com";

        qint64 now = QDateTime::currentMSecsSinceEpoch()  / 1000L;
        qint64 end = now + 300;
        TimeBounds* timeBounds = new TimeBounds(now, end);
        QByteArray nonce(48,'\0');

        QRandomGenerator *r = QRandomGenerator::global();
        r->fillRange(reinterpret_cast<quint32*>(nonce.data()),48/static_cast<int>(sizeof(quint32)));

        QByteArray encodedNonce = nonce.toBase64(QByteArray::Base64Option::Base64UrlEncoding| QByteArray::OmitTrailingEquals);

        Account* sourceAccount = new Account(server, -1L);
        ManageDataOperation* manageDataOperation1 = ManageDataOperation::create(domainName + " auth", encodedNonce);
        manageDataOperation1->setSourceAccount(client->getAccountId());

        BumpSequenceOperation* operation2 = BumpSequenceOperation::create(0L);
        operation2->setSourceAccount(server->getAccountId());

        Transaction* transaction = Transaction::Builder(sourceAccount)
                .setBaseFee(100)
                .addMemo(Memo::none())
                .addTimeBounds(timeBounds)
                .addOperation(manageDataOperation1)
                .addOperation(operation2)
                .build();
        transaction->sign(server);

        QString challenge = transaction->toEnvelopeXdrBase64();
        try {
            Sep10Challenge::readChallengeTransaction(challenge, server->getAccountId(), domainName, webAuthDomain);
            QFAIL("Missing exception");
        }
        catch(std::runtime_error)
        {
            //"Operation type should be ManageData."
        }
    }


     void testReadChallengeTransactionValidMultipleDomainNames() {
       KeyPair* server = KeyPair::random();
       KeyPair* client = KeyPair::random();
       QString domainName = "example.com";
       QString webAuthDomain = "example.com";

       Transaction* transaction;
       try {
         transaction = Sep10Challenge::buildChallengeTx(
           server,
           client->getAccountId(),
           domainName,
           webAuthDomain,
           300
         );
       } catch (std::runtime_error e) {
         QFAIL("Should not have thrown any exception.");
       }

       Sep10Challenge::ChallengeTransaction* challengeTransaction = Sep10Challenge::readChallengeTransaction(transaction->toEnvelopeXdrBase64(), server->getAccountId(), QStringList()<< "example3.com"<< "example2.com"<< "example.com",webAuthDomain);
       QVERIFY(challengeTransaction->equals(new Sep10Challenge::ChallengeTransaction(transaction, client->getAccountId(), domainName)));
     }


     void testReadChallengeTransactionInvalidDomainNamesMismatch() {
         KeyPair* server = KeyPair::random();
         KeyPair* client = KeyPair::random();
         QString domainName = "example.com";
         QString webAuthDomain = "example.com";

         Transaction* transaction;
         try {
             transaction = Sep10Challenge::buildChallengeTx(
                         server,
                         client->getAccountId(),
                         domainName,
                         webAuthDomain,
                         300
                         );
         } catch (std::runtime_error e) {
             QFAIL("Should not have thrown any exception.");
         }
         try {
             Sep10Challenge::readChallengeTransaction(transaction->toEnvelopeXdrBase64(), server->getAccountId(), QStringList()<< "example3.com"<< "example2.com",webAuthDomain);
             QFAIL("Missing exception");
         } catch (std::runtime_error) {
             //"The transaction's operation key name does not include one of the expected home domains.";
         }
     }

     void testReadChallengeTransactionInvalidDomainNamesEmpty(){
         KeyPair* server = KeyPair::random();
         KeyPair* client = KeyPair::random();
         QString domainName = "example.com";
         QString webAuthDomain = "example.com";

         Transaction* transaction;
         try {
             transaction = Sep10Challenge::buildChallengeTx(
                         server,
                         client->getAccountId(),
                         domainName,
                         webAuthDomain,
                         300
                         );
         } catch (std::runtime_error e) {
             QFAIL("Should not have thrown any exception.");
         }
         try {
             Sep10Challenge::readChallengeTransaction(transaction->toEnvelopeXdrBase64(), server->getAccountId(), QStringList(), webAuthDomain);
             QFAIL("Missing exception");
         } catch (std::runtime_error) {
             //"The transaction's operation key name does not include one of the expected home domains."
         }
     }


    void testVerifyChallengeTransactionThresholdValidMultipleDomainNames() {
        KeyPair* server = KeyPair::random();
        KeyPair* masterClient = KeyPair::random();

        QString domainName = "example.com";
        QString webAuthDomain = "example.com";


        Transaction* transaction = Sep10Challenge::buildChallengeTx(
                    server,
                    masterClient->getAccountId(),
                    domainName,
                    webAuthDomain,
                    300
                    );
        transaction->sign(masterClient);

        QSet<Sep10Challenge::Signer> signers;
        signers.insert(Sep10Challenge::Signer(masterClient->getAccountId(),255));
        int threshold = 255;
        QSet<QString> signersFound = Sep10Challenge::verifyChallengeTransactionThreshold(transaction->toEnvelopeXdrBase64(), server->getAccountId(), QStringList()<<"example3.com"<< "example2.com"<< "example.com", webAuthDomain, threshold, signers);
        QSet<QString> expected;
        expected.insert(masterClient->getAccountId());
        QCOMPARE(signersFound, expected);
    }
      void testVerifyChallengeTransactionSignersValidMultipleDomainNames() {
          KeyPair* server = KeyPair::random();
          KeyPair* masterClient = KeyPair::random();

          QString domainName = "example.com";
          QString webAuthDomain = "example.com";


          Transaction* transaction = Sep10Challenge::buildChallengeTx(
                      server,
                      masterClient->getAccountId(),
                      domainName,
                      webAuthDomain,
                      300
                      );
          transaction->sign(masterClient);

          QSet<QString> signers;
          signers.insert(masterClient->getAccountId());
          QSet<QString> signersFound = Sep10Challenge::verifyChallengeTransactionSigners(transaction->toEnvelopeXdrBase64(), server->getAccountId(), QStringList()<<"example3.com"<< "example2.com"<< "example.com", webAuthDomain, signers);
          QCOMPARE(signers, signersFound);
      }

    void testVerifyChallengeTransactionValidAdditionalManageDataOpsWithSourceAccountSetToServerAccount() {
        KeyPair* server = KeyPair::random();
        KeyPair* client = KeyPair::random();
        QString domainName = "example.com";
        QString webAuthDomain = "example.com";

        qint64 now = QDateTime::currentMSecsSinceEpoch()  / 1000L;
        qint64 end = now + 300;
        TimeBounds* timeBounds = new TimeBounds(now, end);
        QByteArray nonce(48,'\0');

        QRandomGenerator *r = QRandomGenerator::global();
        r->fillRange(reinterpret_cast<quint32*>(nonce.data()),48/static_cast<int>(sizeof(quint32)));

        QByteArray encodedNonce = nonce.toBase64(QByteArray::Base64Option::Base64UrlEncoding| QByteArray::OmitTrailingEquals);

        Account* sourceAccount = new Account(server, -1L);
        ManageDataOperation* manageDataOperation1 = ManageDataOperation::create(domainName + " auth", encodedNonce);
        manageDataOperation1->setSourceAccount(client->getAccountId());

        ManageDataOperation* manageDataOperation2 = ManageDataOperation::create("key", "value");
        manageDataOperation2->setSourceAccount(server->getAccountId());


        Transaction* transaction = Transaction::Builder(sourceAccount)
                .setBaseFee(100)
                .addMemo(Memo::none())
                .addTimeBounds(timeBounds)
                .addOperation(manageDataOperation1)
                .addOperation(manageDataOperation2)
                .build();
        transaction->sign(server);
        transaction->sign(client);

        QString challenge = transaction->toEnvelopeXdrBase64();
        QSet<QString> signers;
        signers.insert(client->getAccountId());
        QSet<QString> signersFound = Sep10Challenge::verifyChallengeTransactionSigners(transaction->toEnvelopeXdrBase64(), server->getAccountId(), domainName, webAuthDomain, signers);
        QCOMPARE(signers, signersFound);
    }

    void testVerifyChallengeTransactionInvalidAdditionalManageDataOpsWithoutSourceAccountSetToServerAccount() {
        KeyPair* server = KeyPair::random();
        KeyPair* client = KeyPair::random();
        QString domainName = "example.com";
        QString webAuthDomain = "example.com";

        qint64 now = QDateTime::currentMSecsSinceEpoch()  / 1000L;
        qint64 end = now + 300;
        TimeBounds* timeBounds = new TimeBounds(now, end);
        QByteArray nonce(48,'\0');

        QRandomGenerator *r = QRandomGenerator::global();
        r->fillRange(reinterpret_cast<quint32*>(nonce.data()),48/static_cast<int>(sizeof(quint32)));

        QByteArray encodedNonce = nonce.toBase64(QByteArray::Base64Option::Base64UrlEncoding| QByteArray::OmitTrailingEquals);

        Account* sourceAccount = new Account(server, -1L);
        ManageDataOperation* manageDataOperation1 = ManageDataOperation::create(domainName + " auth", encodedNonce);
        manageDataOperation1->setSourceAccount(client->getAccountId());

        ManageDataOperation* manageDataOperation2 = ManageDataOperation::create("key", "value");
        manageDataOperation2->setSourceAccount(client->getAccountId());


        Transaction* transaction = Transaction::Builder(sourceAccount)
                .setBaseFee(100)
                .addMemo(Memo::none())
                .addTimeBounds(timeBounds)
                .addOperation(manageDataOperation1)
                .addOperation(manageDataOperation2)
                .build();
        transaction->sign(server);
        transaction->sign(client);

        QString challenge = transaction->toEnvelopeXdrBase64();
        QSet<QString> signers;
        signers.insert(client->getAccountId());
        try {
            Sep10Challenge::verifyChallengeTransactionSigners(transaction->toEnvelopeXdrBase64(), server->getAccountId(), domainName, webAuthDomain, signers);
            QFAIL("Missing exception");
        } catch (std::runtime_error) {
            //"Subsequent operations are unrecognized."
        }
    }

    void testVerifyChallengeTransactionInvalidAdditionalManageDataOpsWithSourceAccountSetToNull() {
        KeyPair* server = KeyPair::random();
        KeyPair* client = KeyPair::random();
        QString domainName = "example.com";
        QString webAuthDomain = "example.com";

        qint64 now = QDateTime::currentMSecsSinceEpoch()  / 1000L;
        qint64 end = now + 300;
        TimeBounds* timeBounds = new TimeBounds(now, end);
        QByteArray nonce(48,'\0');

        QRandomGenerator *r = QRandomGenerator::global();
        r->fillRange(reinterpret_cast<quint32*>(nonce.data()),48/static_cast<int>(sizeof(quint32)));

        QByteArray encodedNonce = nonce.toBase64(QByteArray::Base64Option::Base64UrlEncoding| QByteArray::OmitTrailingEquals);

        Account* sourceAccount = new Account(server, -1L);
        ManageDataOperation* manageDataOperation1 = ManageDataOperation::create(domainName + " auth", encodedNonce);
        manageDataOperation1->setSourceAccount(client->getAccountId());

        ManageDataOperation* manageDataOperation2 = ManageDataOperation::create("key", "value");

        Transaction* transaction = Transaction::Builder(sourceAccount)
                .setBaseFee(100)
                .addMemo(Memo::none())
                .addTimeBounds(timeBounds)
                .addOperation(manageDataOperation1)
                .addOperation(manageDataOperation2)
                .build();
        transaction->sign(server);
        transaction->sign(client);

        QString challenge = transaction->toEnvelopeXdrBase64();
        QSet<QString> signers;
        signers.insert(client->getAccountId());
        try {
            Sep10Challenge::verifyChallengeTransactionSigners(transaction->toEnvelopeXdrBase64(), server->getAccountId(), domainName, webAuthDomain, signers);
            QFAIL("Missing exception");
        } catch (std::runtime_error) {
            //"Operation should have a source account."
        }

    }


    void testVerifyChallengeTransactionInvalidAdditionalOpsOfOtherTypes() {
        KeyPair* server = KeyPair::random();
        KeyPair* client = KeyPair::random();
        QString domainName = "example.com";
        QString webAuthDomain = "example.com";

        qint64 now = QDateTime::currentMSecsSinceEpoch()  / 1000L;
        qint64 end = now + 300;
        TimeBounds* timeBounds = new TimeBounds(now, end);
        QByteArray nonce(48,'\0');

        QRandomGenerator *r = QRandomGenerator::global();
        r->fillRange(reinterpret_cast<quint32*>(nonce.data()),48/static_cast<int>(sizeof(quint32)));

        QByteArray encodedNonce = nonce.toBase64(QByteArray::Base64Option::Base64UrlEncoding| QByteArray::OmitTrailingEquals);

        Account* sourceAccount = new Account(server, -1L);
        ManageDataOperation* manageDataOperation1 = ManageDataOperation::create(domainName + " auth", encodedNonce);
        manageDataOperation1->setSourceAccount(client->getAccountId());

        BumpSequenceOperation* op2 = BumpSequenceOperation::create(0L);
        op2->setSourceAccount(server->getAccountId());


        Transaction* transaction = Transaction::Builder(sourceAccount)
                .setBaseFee(100)
                .addMemo(Memo::none())
                .addTimeBounds(timeBounds)
                .addOperation(manageDataOperation1)
                .addOperation(op2)
                .build();
        transaction->sign(server);
        transaction->sign(client);

        QString challenge = transaction->toEnvelopeXdrBase64();
        QSet<QString> signers;
        signers.insert(client->getAccountId());
        try {
            Sep10Challenge::verifyChallengeTransactionSigners(transaction->toEnvelopeXdrBase64(), server->getAccountId(), domainName, webAuthDomain, signers);
            QFAIL("Missing exception");
        } catch (std::runtime_error) {
            //"Operation type should be ManageData."
        }
    }

      void testReadChallengeTransactionValidWebAuthDomainNotEqualHomeDomain()
      {
          KeyPair* server = KeyPair::random();
          KeyPair* client = KeyPair::random();

        QString domainName = "example.com";
        QString webAuthDomain = "auth.example.com";

        Transaction* transaction = Sep10Challenge::buildChallengeTx(
            server,
            client->getAccountId(),
            domainName,
            webAuthDomain,
            300
        );
        Sep10Challenge::ChallengeTransaction* challengeTransaction = Sep10Challenge::readChallengeTransaction(transaction->toEnvelopeXdrBase64(), server->getAccountId(), domainName, webAuthDomain);
        QCOMPARE(transaction->toEnvelopeXdrBase64(), challengeTransaction->getTransaction()->toEnvelopeXdrBase64());
        QCOMPARE(client->getAccountId(), challengeTransaction->getClientAccountId());
        QCOMPARE(domainName, challengeTransaction->getMatchedHomeDomain());
      }


      void testReadChallengeTransactionInvalidWebAuthDomainMismatch()
      {
          KeyPair* server = KeyPair::random();
          KeyPair* client = KeyPair::random();


        QString domainName = "example.com";
        QString webAuthDomain = "example.com";
        QString invalidWebAuthDomain = "invalid.example.com";

        Transaction* transaction = nullptr;
        try {
          transaction = Sep10Challenge::buildChallengeTx(
              server,
              client->getAccountId(),
              domainName,
              invalidWebAuthDomain,
              300
          );
        } catch (std::runtime_error e) {
          QFAIL("Should not have thrown any exception.");
        }

        try {
          Sep10Challenge::readChallengeTransaction(transaction->toEnvelopeXdrBase64(), server->getAccountId(), domainName, webAuthDomain);
          QFAIL("Missing excepcion");
        } catch (std::runtime_error e)
        {
          QCOMPARE(QString("'web_auth_domain' operation value does not match."), QString(e.what()));
        }
      }

      void testReadChallengeTransactionInvalidWebAuthDomainOperationValueIsNull()
      {
          KeyPair* server = KeyPair::random();
          KeyPair* client = KeyPair::random();
        QString domainName = "example.com";
        QString webAuthDomain = "example.com";

        qint64 now = QDateTime::currentMSecsSinceEpoch()  / 1000L;
        qint64 end = now + 300;
        TimeBounds* timeBounds = new TimeBounds(now, end);

        QByteArray nonce(48,'\0');

        QRandomGenerator *r = QRandomGenerator::global();
        r->fillRange(reinterpret_cast<quint32*>(nonce.data()),48/static_cast<int>(sizeof(quint32)));

        QByteArray encodedNonce = nonce.toBase64(QByteArray::Base64Option::Base64UrlEncoding| QByteArray::OmitTrailingEquals);

        Account* sourceAccount = new Account(server, -1L);

        ManageDataOperation* domainNameOperation = ManageDataOperation::create(domainName + " auth", encodedNonce)
            ->setSourceAccount(client->getAccountId());
        ManageDataOperation* webAuthDomainOperation = ManageDataOperation::create("web_auth_domain")
            ->setSourceAccount(server->getAccountId());

        Transaction* transaction = Transaction::Builder(sourceAccount)
                .addOperation(domainNameOperation)
                .addOperation(webAuthDomainOperation)
                .addMemo(Memo::none())
                .addTimeBounds(timeBounds)
                .setBaseFee(100)
                .build();


        transaction->sign(server);
        QString challenge = transaction->toEnvelopeXdrBase64();

        try {
          Sep10Challenge::readChallengeTransaction(challenge, server->getAccountId(), domainName, webAuthDomain);
          QFAIL("Missing excepcion");
        } catch (std::runtime_error e) {
            QCOMPARE(QString(e.what()),QString("'web_auth_domain' operation value should not be null."));
        }


      }

      void testReadChallengeTransactionValidWebAuthDomainAllowSubsequentManageDataOperationsToHaveNullValue()
      {
          KeyPair* server = KeyPair::random();
          KeyPair* client = KeyPair::random();
        QString domainName = "example.com";
        QString webAuthDomain = "example.com";


        qint64 now = QDateTime::currentMSecsSinceEpoch()  / 1000L;
        qint64 end = now + 300;
        TimeBounds* timeBounds = new TimeBounds(now, end);

        QByteArray nonce(48,'\0');

        QRandomGenerator *r = QRandomGenerator::global();
        r->fillRange(reinterpret_cast<quint32*>(nonce.data()),48/static_cast<int>(sizeof(quint32)));

        QByteArray encodedNonce = nonce.toBase64(QByteArray::Base64Option::Base64UrlEncoding| QByteArray::OmitTrailingEquals);

        Account* sourceAccount = new Account(server, -1L);
        ManageDataOperation* domainNameOperation = ManageDataOperation::create(domainName + " auth", encodedNonce)
            ->setSourceAccount(client->getAccountId());
        ManageDataOperation* webAuthDomainOperation = ManageDataOperation::create("web_auth_domain", webAuthDomain.toUtf8())
            ->setSourceAccount(server->getAccountId());
        ManageDataOperation* otherDomainOperation = ManageDataOperation::create("subsequent_op")
            ->setSourceAccount(server->getAccountId());

        Transaction* transaction = Transaction::Builder(sourceAccount)
                .addOperation(domainNameOperation)
                .addOperation(webAuthDomainOperation)
                .addOperation(otherDomainOperation)
                .addMemo(Memo::none())
                .addTimeBounds(timeBounds)
                .setBaseFee(100)
                .build();

        transaction->sign(server);
        QString challenge = transaction->toEnvelopeXdrBase64();

        Sep10Challenge::ChallengeTransaction* challengeTransaction = Sep10Challenge::readChallengeTransaction(challenge, server->getAccountId(), domainName, webAuthDomain);
        QCOMPARE(transaction->toEnvelopeXdrBase64(), challengeTransaction->getTransaction()->toEnvelopeXdrBase64());
        QCOMPARE(client->getAccountId(), challengeTransaction->getClientAccountId());
        QCOMPARE(domainName, challengeTransaction->getMatchedHomeDomain());
      }

    void testVerifyChallengeTransactionThresholdInvalidNotSignedByServer(){

        KeyPair* server = KeyPair::random();
        KeyPair* masterClient = KeyPair::random();
        KeyPair* signerClient1 = KeyPair::random();
        KeyPair* signerClient2 = KeyPair::random();
        QString domainName = "example.com";
        QString webAuthDomain = "example.com";

        qint64 now = QDateTime::currentMSecsSinceEpoch()  / 1000L;
        qint64 end = now + 300;
        TimeBounds* timeBounds = new TimeBounds(now, end);
        QByteArray nonce(48,'\0');

        QRandomGenerator *r = QRandomGenerator::global();
        r->fillRange(reinterpret_cast<quint32*>(nonce.data()),48/static_cast<int>(sizeof(quint32)));

        QByteArray encodedNonce = nonce.toBase64(QByteArray::Base64Option::Base64UrlEncoding| QByteArray::OmitTrailingEquals);

        Account* sourceAccount = new Account(server, -1L);
        ManageDataOperation* manageDataOperation1 = ManageDataOperation::create(domainName + " auth", encodedNonce);
        manageDataOperation1->setSourceAccount(masterClient->getAccountId());

        Transaction* transaction = Transaction::Builder(sourceAccount)
                .setBaseFee(100)
                .addMemo(Memo::none())
                .addTimeBounds(timeBounds)
                .addOperation(manageDataOperation1)
                .build();
        transaction->sign(masterClient);

        QSet<Sep10Challenge::Signer> signers;
        signers.insert(Sep10Challenge::Signer(masterClient->getAccountId(), 1));
        signers.insert(Sep10Challenge::Signer(signerClient1->getAccountId(), 2));
        signers.insert(Sep10Challenge::Signer(signerClient2->getAccountId(), 4));

        int threshold = 6;
        try {
            Sep10Challenge::verifyChallengeTransactionThreshold(transaction->toEnvelopeXdrBase64(), server->getAccountId(), domainName, webAuthDomain, threshold, signers);
            QFAIL("Missing exception");
        } catch (std::runtime_error e) {
            //"Transaction not signed by server
        }
    }

    void testVerifyChallengeTransactionThresholdValidServerAndClientKeyMeetingThreshold() {

        KeyPair* server = KeyPair::random();
        KeyPair* masterClient = KeyPair::random();

        QString domainName = "example.com";
        QString webAuthDomain = "example.com";

        Transaction* transaction = Sep10Challenge::buildChallengeTx(server,masterClient->getAccountId(),domainName,webAuthDomain,300);
        transaction->sign(masterClient);

        QSet<Sep10Challenge::Signer> signers;
        signers.insert(Sep10Challenge::Signer(masterClient->getAccountId(), 255));
        int threshold=255;
        QSet<QString> signersFound = Sep10Challenge::verifyChallengeTransactionThreshold(transaction->toEnvelopeXdrBase64(), server->getAccountId(), domainName, webAuthDomain, threshold, signers);
        QSet<QString> expected;
        expected.insert(masterClient->getAccountId());
        QCOMPARE(signersFound,expected);
    }


    void testVerifyChallengeTransactionThresholdValidServerAndMultipleClientKeyMeetingThreshold(){
        KeyPair* server = KeyPair::random();
        KeyPair* masterClient = KeyPair::random();
        KeyPair* signerClient1 = KeyPair::random();
        KeyPair* signerClient2 = KeyPair::random();
        QString domainName = "example.com";
        QString webAuthDomain = "example.com";


        Transaction* transaction = Sep10Challenge::buildChallengeTx(
                    server,
                    masterClient->getAccountId(),
                    domainName,
                    webAuthDomain,
                    300
                    );

        transaction->sign(masterClient);
        transaction->sign(signerClient1);
        transaction->sign(signerClient2);

        QSet<Sep10Challenge::Signer> signers;
        signers.insert(Sep10Challenge::Signer(masterClient->getAccountId(),1));
        signers.insert(Sep10Challenge::Signer(signerClient1->getAccountId(),2));
        signers.insert(Sep10Challenge::Signer(signerClient2->getAccountId(),4));

        int threshold = 7;
        QSet<QString> signersFound = Sep10Challenge::verifyChallengeTransactionThreshold(transaction->toEnvelopeXdrBase64(), server->getAccountId(), domainName, webAuthDomain, threshold, signers);
        QSet<QString> expected;
        expected.insert(masterClient->getAccountId());
        expected.insert(signerClient1->getAccountId());
        expected.insert(signerClient2->getAccountId());
        QCOMPARE(signersFound,expected);
    }


    void testVerifyChallengeTransactionThresholdValidServerAndMultipleClientKeyMeetingThresholdSomeUnused(){
        KeyPair* server = KeyPair::random();
        KeyPair* masterClient = KeyPair::random();
        KeyPair* signerClient1 = KeyPair::random();
        KeyPair* signerClient2 = KeyPair::random();
        QString domainName = "example.com";
        QString webAuthDomain = "example.com";
        Transaction* transaction = Sep10Challenge::buildChallengeTx(
                    server,
                    masterClient->getAccountId(),
                    domainName,
                    webAuthDomain,
                    300
                    );


        transaction->sign(masterClient);
        transaction->sign(signerClient1);
        QSet<Sep10Challenge::Signer> signers;
        signers.insert(Sep10Challenge::Signer(masterClient->getAccountId(),1));
        signers.insert(Sep10Challenge::Signer(signerClient1->getAccountId(),2));
        signers.insert(Sep10Challenge::Signer(signerClient2->getAccountId(),4));

        int threshold = 3;
        QSet<QString> signersFound = Sep10Challenge::verifyChallengeTransactionThreshold(transaction->toEnvelopeXdrBase64(), server->getAccountId(), domainName, webAuthDomain, threshold, signers);
        QSet<QString> expected;
        expected.insert(masterClient->getAccountId());
        expected.insert(signerClient1->getAccountId());
        QCOMPARE(signersFound,expected);
    }


    void testVerifyChallengeTransactionThresholdValidServerAndMultipleClientKeyMeetingThresholdSomeUnusedIgnorePreauthTxHashAndXHash() {

        KeyPair* server = KeyPair::random();
        KeyPair* masterClient = KeyPair::random();
        KeyPair* signerClient1 = KeyPair::random();
        KeyPair* signerClient2 = KeyPair::random();
        QString domainName = "example.com";
        QString webAuthDomain = "example.com";
        Transaction* transaction = Sep10Challenge::buildChallengeTx(
                    server,
                    masterClient->getAccountId(),
                    domainName,
                    webAuthDomain,
                    300
                    );


        transaction->sign(masterClient);
        transaction->sign(signerClient1);

        QString preauthTxHash = "TAQCSRX2RIDJNHFIFHWD63X7D7D6TRT5Y2S6E3TEMXTG5W3OECHZ2OG4";
        QString xHash = "XDRPF6NZRR7EEVO7ESIWUDXHAOMM2QSKIQQBJK6I2FB7YKDZES5UCLWD";
        QString unknownSignerType = "?ARPF6NZRR7EEVO7ESIWUDXHAOMM2QSKIQQBJK6I2FB7YKDZES5UCLWD";

        QSet<Sep10Challenge::Signer> signers;
        signers.insert(Sep10Challenge::Signer(masterClient->getAccountId(),1));
        signers.insert(Sep10Challenge::Signer(signerClient1->getAccountId(),2));
        signers.insert(Sep10Challenge::Signer(signerClient2->getAccountId(),4));
        signers.insert(Sep10Challenge::Signer(preauthTxHash,10));
        signers.insert(Sep10Challenge::Signer(xHash,10));
        signers.insert(Sep10Challenge::Signer(unknownSignerType,10));


        int threshold = 3;
        QSet<QString> signersFound = Sep10Challenge::verifyChallengeTransactionThreshold(transaction->toEnvelopeXdrBase64(), server->getAccountId(), domainName, webAuthDomain, threshold, signers);
        QSet<QString> expected;
        expected.insert(masterClient->getAccountId());
        expected.insert(signerClient1->getAccountId());
        QCOMPARE(signersFound,expected);
    }


    void testVerifyChallengeTransactionThresholdInvalidServerAndMultipleClientKeyNotMeetingThreshold(){
        KeyPair* server = KeyPair::random();
        KeyPair* masterClient = KeyPair::random();
        KeyPair* signerClient1 = KeyPair::random();
        KeyPair* signerClient2 = KeyPair::random();
        QString domainName = "example.com";
        QString webAuthDomain = "example.com";
        Transaction* transaction = Sep10Challenge::buildChallengeTx(
                    server,
                    masterClient->getAccountId(),
                    domainName,
                    webAuthDomain,
                    300
                    );

        transaction->sign(masterClient);
        transaction->sign(signerClient1);
        QSet<Sep10Challenge::Signer> signers;
        signers.insert(Sep10Challenge::Signer(masterClient->getAccountId(),1));
        signers.insert(Sep10Challenge::Signer(signerClient1->getAccountId(),2));
        signers.insert(Sep10Challenge::Signer(signerClient2->getAccountId(),4));

        int threshold = 7;

        try {
            Sep10Challenge::verifyChallengeTransactionThreshold(transaction->toEnvelopeXdrBase64(), server->getAccountId(), domainName, webAuthDomain, threshold, signers);
            QFAIL("Missing exception");
        } catch (std::runtime_error e) {
            //"Signers with weight 3 do not meet threshold 7."
        }
    }


    void testVerifyChallengeTransactionThresholdInvalidClientKeyUnrecognized() {
        KeyPair* server = KeyPair::random();
        KeyPair* masterClient = KeyPair::random();
        KeyPair* signerClient1 = KeyPair::random();
        KeyPair* signerClient2 = KeyPair::random();
        QString domainName = "example.com";
        QString webAuthDomain = "example.com";
        Transaction* transaction = Sep10Challenge::buildChallengeTx(
                    server,
                    masterClient->getAccountId(),
                    domainName,
                    webAuthDomain,
                    300
                    );

        transaction->sign(masterClient);
        transaction->sign(signerClient1);
        transaction->sign(signerClient2);
        transaction->sign(KeyPair::random());
        QSet<Sep10Challenge::Signer> signers;
        signers.insert(Sep10Challenge::Signer(masterClient->getAccountId(),1));
        signers.insert(Sep10Challenge::Signer(signerClient1->getAccountId(),2));
        signers.insert(Sep10Challenge::Signer(signerClient2->getAccountId(),4));

        int threshold = 7;

        try {
            Sep10Challenge::verifyChallengeTransactionThreshold(transaction->toEnvelopeXdrBase64(), server->getAccountId(), domainName, webAuthDomain, threshold, signers);
            QFAIL("Missing exception");
        } catch (std::runtime_error e) {
            //"Transaction has unrecognized signatures."
        }

    }

    void testVerifyChallengeTransactionThresholdInvalidNoSigners() {
        KeyPair* server = KeyPair::random();
        KeyPair* masterClient = KeyPair::random();
        KeyPair* signerClient1 = KeyPair::random();
        KeyPair* signerClient2 = KeyPair::random();
        QString domainName = "example.com";
        QString webAuthDomain = "example.com";
        Transaction* transaction = Sep10Challenge::buildChallengeTx(
                    server,
                    masterClient->getAccountId(),
                    domainName,
                    webAuthDomain,
                    300
                    );

        transaction->sign(masterClient);
        transaction->sign(signerClient1);
        transaction->sign(signerClient2);
        QSet<Sep10Challenge::Signer> signers;

        int threshold = 7;

        try {
            Sep10Challenge::verifyChallengeTransactionThreshold(transaction->toEnvelopeXdrBase64(), server->getAccountId(), domainName, webAuthDomain, threshold, signers);
            QFAIL("Missing exception");
        } catch (std::runtime_error e) {
            //"No verifiable signers provided, at least one G... address must be provided."
        }
    }


    void testVerifyChallengeTransactionThresholdInvalidNoPublicKeySigners() {
        KeyPair* server = KeyPair::random();
        KeyPair* masterClient = KeyPair::random();
        KeyPair* signerClient1 = KeyPair::random();
        KeyPair* signerClient2 = KeyPair::random();
        QString domainName = "example.com";
        QString webAuthDomain = "example.com";
        Transaction* transaction = Sep10Challenge::buildChallengeTx(
                    server,
                    masterClient->getAccountId(),
                    domainName,
                    webAuthDomain,
                    300
                    );

        transaction->sign(masterClient);
        transaction->sign(signerClient1);
        transaction->sign(signerClient2);



        QString preauthTxHash = "TAQCSRX2RIDJNHFIFHWD63X7D7D6TRT5Y2S6E3TEMXTG5W3OECHZ2OG4";
        QString xHash = "XDRPF6NZRR7EEVO7ESIWUDXHAOMM2QSKIQQBJK6I2FB7YKDZES5UCLWD";
        QString unknownSignerType = "?ARPF6NZRR7EEVO7ESIWUDXHAOMM2QSKIQQBJK6I2FB7YKDZES5UCLWD";
        QSet<Sep10Challenge::Signer> signers;
        signers.insert(Sep10Challenge::Signer(preauthTxHash, 1));
        signers.insert(Sep10Challenge::Signer(xHash, 2));
        signers.insert(Sep10Challenge::Signer(unknownSignerType, 2));

        int threshold = 3;

        try {
            Sep10Challenge::verifyChallengeTransactionThreshold(transaction->toEnvelopeXdrBase64(), server->getAccountId(), domainName, webAuthDomain, threshold, signers);
            QFAIL("Missing exception");
        } catch (std::runtime_error e) {
            //"No verifiable signers provided, at least one G... address must be provided."
        }
    }


    void testVerifyChallengeTransactionThresholdWeightsAddToMoreThan8Bits() {
        KeyPair* server = KeyPair::random();
        KeyPair* masterClient = KeyPair::random();
        KeyPair* signerClient1 = KeyPair::random();

        QString domainName = "example.com";
        QString webAuthDomain = "example.com";
        Transaction* transaction = Sep10Challenge::buildChallengeTx(
                    server,
                    masterClient->getAccountId(),
                    domainName,
                    webAuthDomain,
                    300
                    );

        transaction->sign(masterClient);
        transaction->sign(signerClient1);
        QSet<Sep10Challenge::Signer> signers;
        signers.insert(Sep10Challenge::Signer(masterClient->getAccountId(),255));
        signers.insert(Sep10Challenge::Signer(signerClient1->getAccountId(),1));


        int threshold = 1;
        QSet<QString> signersFound = Sep10Challenge::verifyChallengeTransactionThreshold(transaction->toEnvelopeXdrBase64(), server->getAccountId(), domainName, webAuthDomain, threshold, signers);

        QSet<QString> expected;
        expected.insert(masterClient->getAccountId());
        expected.insert(signerClient1->getAccountId());
        QCOMPARE(signersFound,expected);
    }

    void testVerifyChallengeTransactionSignersInvalidServer() {
        KeyPair* server = KeyPair::random();
        KeyPair* masterClient = KeyPair::random();
        KeyPair* signerClient1 = KeyPair::random();
        KeyPair* signerClient2 = KeyPair::random();
        qint64 now = QDateTime::currentMSecsSinceEpoch() / 1000L;
        qint64 end = now + 300;
        QString domainName = "example.com";
        QString webAuthDomain = "example.com";
        TimeBounds* timeBounds = new TimeBounds(now, end);

        QByteArray nonce(48,'\0');

        QRandomGenerator *r = QRandomGenerator::global();
        r->fillRange(reinterpret_cast<quint32*>(nonce.data()),48/static_cast<int>(sizeof(quint32)));

        QByteArray encodedNonce = nonce.toBase64(QByteArray::Base64Option::Base64UrlEncoding| QByteArray::OmitTrailingEquals);

        Account* sourceAccount = new Account(server, -1L);
        ManageDataOperation* manageDataOperation1 = ManageDataOperation::create(domainName + " auth", encodedNonce);
        manageDataOperation1->setSourceAccount(masterClient->getAccountId());

        Transaction* transaction = Transaction::Builder(sourceAccount)
                .setBaseFee(100)
                .addMemo(Memo::none())
                .addTimeBounds(timeBounds)
                .addOperation(manageDataOperation1)
                .build();

        transaction->sign(masterClient);
        transaction->sign(signerClient1);
        transaction->sign(signerClient2);

        QSet<QString> signers;

        signers.insert(masterClient->getAccountId());
        signers.insert(signerClient1->getAccountId());
        signers.insert(signerClient2->getAccountId());
        signers.insert(KeyPair::random()->getAccountId());

        try {
            Sep10Challenge::verifyChallengeTransactionSigners(transaction->toEnvelopeXdrBase64(), server->getAccountId(), domainName, webAuthDomain, signers);
            QFAIL("Missing exception");
        } catch (std::runtime_error e) {
            //"Transaction not signed by server
        }
    }

    void testVerifyChallengeTransactionSignersValidServerAndClientMasterKey() {
        KeyPair* server = KeyPair::random();
        KeyPair* masterClient = KeyPair::random();

        QString domainName = "example.com";
        QString webAuthDomain = "example.com";

        Transaction* transaction = Sep10Challenge::buildChallengeTx(
                    server,
                    masterClient->getAccountId(),
                    domainName,
                    webAuthDomain,
                    300
                    );

        transaction->sign(masterClient);

        QSet<QString> signers;
        signers.insert(masterClient->getAccountId());
        QSet<QString> signersFound = Sep10Challenge::verifyChallengeTransactionSigners(transaction->toEnvelopeXdrBase64(), server->getAccountId(), domainName, webAuthDomain, signers);
        QCOMPARE(signers, signersFound);
    }

    void testVerifyChallengeTransactionSignersInvalidServerAndNoClient() {
        KeyPair* server = KeyPair::random();
        KeyPair* masterClient = KeyPair::random();
        KeyPair* signerClient1 = KeyPair::random();
        KeyPair* signerClient2 = KeyPair::random();


        QString domainName = "example.com";
        QString webAuthDomain = "example.com";

        Transaction* transaction = Sep10Challenge::buildChallengeTx(
                    server,
                    masterClient->getAccountId(),
                    domainName,
                    webAuthDomain,
                    300
                    );

        QSet<QString> signers;
        signers.insert(masterClient->getAccountId());
        signers.insert(signerClient1->getAccountId());
        signers.insert(signerClient2->getAccountId());
        signers.insert(KeyPair::random()->getAccountId());

        try {
            Sep10Challenge::verifyChallengeTransactionSigners(transaction->toEnvelopeXdrBase64(), server->getAccountId(), domainName, webAuthDomain, signers);
            QFAIL("Missing exception");
        } catch (std::runtime_error e) {
            //"Transaction not signed by any client signer."
        }
    }

    void testVerifyChallengeTransactionSignersInvalidServerAndClientKeyUnrecognized(){
        KeyPair* server = KeyPair::random();
        KeyPair* masterClient = KeyPair::random();
        KeyPair* signerClient1 = KeyPair::random();
        KeyPair* signerClient2 = KeyPair::random();
        QString domainName = "example.com";
        QString webAuthDomain = "example.com";

        Transaction* transaction = Sep10Challenge::buildChallengeTx(
                    server,
                    masterClient->getAccountId(),
                    domainName,
                    webAuthDomain,
                    300
                    );

        transaction->sign(masterClient);
        transaction->sign(signerClient1);
        transaction->sign(signerClient2);
        transaction->sign(KeyPair::random());

        QSet<QString> signers;
        signers.insert(masterClient->getAccountId());
        signers.insert(signerClient1->getAccountId());
        signers.insert(signerClient2->getAccountId());
        signers.insert(KeyPair::random()->getAccountId());
        try {
            Sep10Challenge::verifyChallengeTransactionSigners(transaction->toEnvelopeXdrBase64(), server->getAccountId(), domainName, webAuthDomain, signers);
            QFAIL("Missing exception");
        } catch (std::runtime_error e) {
            //"Transaction has unrecognized signatures."
        }
    }

    void testVerifyChallengeTransactionSignersValidServerAndMultipleClientSigners() {
        KeyPair* server = KeyPair::random();
        KeyPair* masterClient = KeyPair::random();
        KeyPair* signerClient1 = KeyPair::random();
        KeyPair* signerClient2 = KeyPair::random();
        QString domainName = "example.com";
        QString webAuthDomain = "example.com";

        Transaction* transaction = Sep10Challenge::buildChallengeTx(
                    server,
                    masterClient->getAccountId(),
                    domainName,
                    webAuthDomain,
                    300
                    );

        transaction->sign(signerClient1);
        transaction->sign(signerClient2);

        QSet<QString> signers;
        signers.insert(masterClient->getAccountId());
        signers.insert(signerClient1->getAccountId());
        signers.insert(signerClient2->getAccountId());
        signers.insert(KeyPair::random()->getAccountId());


        QSet<QString> signersFound = Sep10Challenge::verifyChallengeTransactionSigners(transaction->toEnvelopeXdrBase64(), server->getAccountId(), domainName, webAuthDomain, signers);

        QSet<QString> expected;
        expected.insert(signerClient1->getAccountId());
        expected.insert(signerClient2->getAccountId());
        QCOMPARE(signersFound,expected);

    }



    void testVerifyChallengeTransactionSignersValidServerAndMultipleClientSignersReverseOrder() {
        KeyPair* server = KeyPair::random();
        KeyPair* masterClient = KeyPair::random();
        KeyPair* signerClient1 = KeyPair::random();
        KeyPair* signerClient2 = KeyPair::random();
        QString domainName = "example.com";
        QString webAuthDomain = "example.com";

        Transaction* transaction = Sep10Challenge::buildChallengeTx(
                    server,
                    masterClient->getAccountId(),
                    domainName,
                    webAuthDomain,
                    300
                    );
        transaction->sign(signerClient2);
        transaction->sign(signerClient1);


        QSet<QString> signers;
        signers.insert(masterClient->getAccountId());
        signers.insert(signerClient1->getAccountId());
        signers.insert(signerClient2->getAccountId());
        signers.insert(KeyPair::random()->getAccountId());


        QSet<QString> signersFound = Sep10Challenge::verifyChallengeTransactionSigners(transaction->toEnvelopeXdrBase64(), server->getAccountId(), domainName,webAuthDomain, signers);

        QSet<QString> expected;
        expected.insert(signerClient1->getAccountId());
        expected.insert(signerClient2->getAccountId());
        QCOMPARE(signersFound,expected);
    }


    void testVerifyChallengeTransactionSignersValidServerAndClientSignersNotMasterKey(){
        KeyPair* server = KeyPair::random();
        KeyPair* masterClient = KeyPair::random();
        KeyPair* signerClient1 = KeyPair::random();
        QString domainName = "example.com";
        QString webAuthDomain = "example.com";

        Transaction* transaction = Sep10Challenge::buildChallengeTx(
                    server,
                    masterClient->getAccountId(),
                    domainName,
                    webAuthDomain,
                    300
                    );
        transaction->sign(signerClient1);

        QSet<QString> signers;
        signers.insert(masterClient->getAccountId());
        signers.insert(signerClient1->getAccountId());

        QSet<QString> signersFound = Sep10Challenge::verifyChallengeTransactionSigners(transaction->toEnvelopeXdrBase64(), server->getAccountId(), domainName,webAuthDomain, signers);

        QSet<QString> expected;
        expected.insert(signerClient1->getAccountId());
        QCOMPARE(signersFound,expected);
    }


    void testVerifyChallengeTransactionSignersValidServerAndClientSignersIgnoresServerSigner() {
        KeyPair* server = KeyPair::random();
        KeyPair* masterClient = KeyPair::random();
        KeyPair* signerClient1 = KeyPair::random();
        QString domainName = "example.com";
        QString webAuthDomain = "example.com";

        Transaction* transaction = Sep10Challenge::buildChallengeTx(
                    server,
                    masterClient->getAccountId(),
                    domainName,
                    webAuthDomain,
                    300
                    );
        transaction->sign(signerClient1);

        QSet<QString> signers;
        signers.insert(signerClient1->getAccountId());

        QSet<QString> signersFound = Sep10Challenge::verifyChallengeTransactionSigners(transaction->toEnvelopeXdrBase64(), server->getAccountId(), domainName,webAuthDomain, signers);

        QSet<QString> expected;
        expected.insert(signerClient1->getAccountId());
        QCOMPARE(signersFound,expected);
    }


    void testVerifyChallengeTransactionSignersInvalidServerNoClientSignersIgnoresServerSigner() {

        KeyPair* server = KeyPair::random();
        KeyPair* masterClient = KeyPair::random();
        KeyPair* signerClient1 = KeyPair::random();
        QString domainName = "example.com";
        QString webAuthDomain = "example.com";

        Transaction* transaction = Sep10Challenge::buildChallengeTx(
                    server,
                    masterClient->getAccountId(),
                    domainName,
                    webAuthDomain,
                    300
                    );


        QSet<QString> signers;
        signers.insert(masterClient->getAccountId());
        signers.insert(signerClient1->getAccountId());
        signers.insert(server->getAccountId());

        try{
            Sep10Challenge::verifyChallengeTransactionSigners(transaction->toEnvelopeXdrBase64(), server->getAccountId(), domainName,webAuthDomain, signers);
            QFAIL("Missing exception");
        }
        catch(std::runtime_error)
        {
            //"Transaction not signed by any client signer."
        }
    }



    void testVerifyChallengeTransactionSignersValidIgnorePreauthTxHashAndXHash() {
        KeyPair* server = KeyPair::random();
        KeyPair* masterClient = KeyPair::random();
        KeyPair* signerClient1 = KeyPair::random();
        QString domainName = "example.com";
        QString webAuthDomain = "example.com";

        Transaction* transaction = Sep10Challenge::buildChallengeTx(
                    server,
                    masterClient->getAccountId(),
                    domainName,
                    webAuthDomain,
                    300
                    );
        transaction->sign(signerClient1);

        QString preauthTxHash = "TAQCSRX2RIDJNHFIFHWD63X7D7D6TRT5Y2S6E3TEMXTG5W3OECHZ2OG4";
        QString xHash = "XDRPF6NZRR7EEVO7ESIWUDXHAOMM2QSKIQQBJK6I2FB7YKDZES5UCLWD";
        QString unknownSignerType = "?ARPF6NZRR7EEVO7ESIWUDXHAOMM2QSKIQQBJK6I2FB7YKDZES5UCLWD";

        QSet<QString> signers;
        signers.insert(masterClient->getAccountId());
        signers.insert(signerClient1->getAccountId());
        signers.insert(preauthTxHash);
        signers.insert(xHash);
        signers.insert(unknownSignerType);

        QSet<QString> signersFound = Sep10Challenge::verifyChallengeTransactionSigners(transaction->toEnvelopeXdrBase64(), server->getAccountId(), domainName,webAuthDomain, signers);

        QSet<QString> expected;
        expected.insert(signerClient1->getAccountId());
        QCOMPARE(signersFound,expected);
    }

    void testVerifyChallengeTransactionSignersInvalidServerAndClientSignersFailsDuplicateSignatures() {
        KeyPair* server = KeyPair::random();
        KeyPair* masterClient = KeyPair::random();
        KeyPair* signerClient1 = KeyPair::random();
        QString domainName = "example.com";
        QString webAuthDomain = "example.com";

        Transaction* transaction = Sep10Challenge::buildChallengeTx(
                    server,
                    masterClient->getAccountId(),
                    domainName,
                    webAuthDomain,
                    300
                    );
        transaction->sign(signerClient1);
        transaction->sign(signerClient1);

        QSet<QString> signers;
        signers.insert(masterClient->getAccountId());
        signers.insert(signerClient1->getAccountId());


        try {
            Sep10Challenge::verifyChallengeTransactionSigners(transaction->toEnvelopeXdrBase64(), server->getAccountId(), domainName,webAuthDomain, signers);
            QFAIL("Missing exception");
        } catch (std::runtime_error) {
            //"Transaction has unrecognized signatures."
        }
    }

    void testVerifyChallengeTransactionSignersInvalidServerAndClientSignersFailsSignerSeed(){

        KeyPair* server = KeyPair::random();
        KeyPair* masterClient = KeyPair::random();
        KeyPair* signerClient1 = KeyPair::random();
        QString domainName = "example.com";
        QString webAuthDomain = "example.com";

        Transaction* transaction = Sep10Challenge::buildChallengeTx(
                    server,
                    masterClient->getAccountId(),
                    domainName,
                    webAuthDomain,
                    300
                    );
        transaction->sign(signerClient1);

        QSet<QString> signers;
        signers.insert(signerClient1->getSecretSeed());


        try {
            Sep10Challenge::verifyChallengeTransactionSigners(transaction->toEnvelopeXdrBase64(), server->getAccountId(), domainName,webAuthDomain, signers);
            QFAIL("Missing exception");
        } catch (std::runtime_error) {
            //"No verifiable signers provided, at least one G... address must be provided."
        }
    }


    void testVerifyChallengeTransactionSignersInvalidNoSignersNull(){
        KeyPair* server = KeyPair::random();
        KeyPair* masterClient = KeyPair::random();
        KeyPair* signerClient1 = KeyPair::random();
        QString domainName = "example.com";
        QString webAuthDomain = "example.com";

        Transaction* transaction = Sep10Challenge::buildChallengeTx(
                    server,
                    masterClient->getAccountId(),
                    domainName,
                    webAuthDomain,
                    300
                    );
        transaction->sign(signerClient1);

        QSet<QString> signers;
        try {
            Sep10Challenge::verifyChallengeTransactionSigners(transaction->toEnvelopeXdrBase64(), server->getAccountId(), domainName, webAuthDomain,  signers);
            QFAIL("Missing exception");
        } catch (std::runtime_error) {
            //"No verifiable signers provided, at least one G... address must be provided."
        }
    }


    void testVerifyChallengeTransactionSignersInvalidNoSignersEmptyString(){
        KeyPair* server = KeyPair::random();
        KeyPair* masterClient = KeyPair::random();
        KeyPair* signerClient1 = KeyPair::random();
        QString domainName = "example.com";
        QString webAuthDomain = "example.com";

        Transaction* transaction = Sep10Challenge::buildChallengeTx(
                    server,
                    masterClient->getAccountId(),
                    domainName,
                    webAuthDomain,
                    300
                    );
        transaction->sign(signerClient1);

        QSet<QString> signers;
        signers.insert(QString());
        try {
            Sep10Challenge::verifyChallengeTransactionSigners(transaction->toEnvelopeXdrBase64(), server->getAccountId(), domainName, webAuthDomain, signers);
            QFAIL("Missing exception");
        } catch (std::runtime_error) {
            //"No verifiable signers provided, at least one G... address must be provided."
        }
    }
};
ADD_TEST(Sep10ChallengeTest)
#endif // SEP10CHALLENGETEST_H
