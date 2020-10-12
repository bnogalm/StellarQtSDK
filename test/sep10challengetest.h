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
        qint64 nowPrev = QDateTime::currentMSecsSinceEpoch() / 1000L;
        Transaction* transaction = Sep10Challenge::buildChallengeTx(
                    server,
                    client->getAccountId(),
                    domainName,
                    300
                    );

        qint64 now = QDateTime::currentMSecsSinceEpoch() / 1000L;
        qint64 end = now + 300;
        QCOMPARE(transaction->getNetwork(),Network::current());
        QCOMPARE(transaction->getFee(),100);
        //QCOMPARE(transaction->getTimeBounds()->getMaxTime(),end);
        QVERIFY(transaction->getTimeBounds()->getMaxTime()>=nowPrev+300);
        QVERIFY(transaction->getTimeBounds()->getMaxTime()<=end);
        QVERIFY(transaction->getTimeBounds()->getMinTime()<=now);
        QVERIFY(transaction->getTimeBounds()->getMinTime()>=nowPrev);
        QCOMPARE(transaction->getSourceAccount(),server->getAccountId());
        QCOMPARE(transaction->getSequenceNumber(),0);

        QCOMPARE(transaction->getOperations().length() , 1);


        ManageDataOperation* op = (ManageDataOperation*) transaction->getOperations().at(0);
        QCOMPARE(op->getSourceAccount(),client->getAccountId());
        QCOMPARE(op->getName(),domainName + " auth");

        QCOMPARE(op->getValue().length(), 64);

        QVERIFY(QByteArray::fromBase64(op->getValue()).size()>0);


        QCOMPARE(transaction->getSignatures().size(),1);
        QVERIFY(
                    server->verify(transaction->hash(), transaction->getSignatures().at(0).signature.binary())
                    );
    }


    void testNewChallengeRejectsMuxedClientAccount(){
        try {
            KeyPair* server = KeyPair::random();

            QString domainName = "example.com";

            Sep10Challenge::buildChallengeTx(
                        server,
                        "MCAAAAAAAAAAAAB7BQ2L7E5NBWMXDUCMZSIPOBKRDSBYVLMXGSSKF6YNPIB7Y77ITKNOG",
                        domainName,
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

        Transaction* transaction = Sep10Challenge::buildChallengeTx(
                    server,
                    client->getAccountId(),
                    domainName,
                    300
                    );

        Sep10Challenge::ChallengeTransaction* challengeTransaction = Sep10Challenge::readChallengeTransaction(transaction->toEnvelopeXdrBase64(), server->getAccountId(), domainName);
        QVERIFY(challengeTransaction->equals(new Sep10Challenge::ChallengeTransaction(transaction, client->getAccountId())));
    }


    void testReadChallengeTransactionAcceptsBothV0AndV1()  {
        KeyPair* server = KeyPair::random();
        KeyPair* client = KeyPair::random();

        QString domainName = "example.com";

        Transaction* transaction = Sep10Challenge::buildChallengeTx(
                    server,
                    client->getAccountId(),
                    domainName,
                    300
                    );

        transaction->setEnvelopeType(stellar::EnvelopeType::ENVELOPE_TYPE_TX_V0);
        stellar::TransactionEnvelope v0 = transaction->toEnvelopeXdr();
        QString v0Base64 = transaction->toEnvelopeXdrBase64();
        QCOMPARE(v0.type,stellar::EnvelopeType::ENVELOPE_TYPE_TX_V0);
        Sep10Challenge::ChallengeTransaction* v0ChallengeTransaction = Sep10Challenge::readChallengeTransaction(
                    v0Base64,
                    server->getAccountId(),
                    domainName
                    );

        transaction->setEnvelopeType(stellar::EnvelopeType::ENVELOPE_TYPE_TX);
        stellar::TransactionEnvelope v1 = transaction->toEnvelopeXdr();
        QString v1Base64 = transaction->toEnvelopeXdrBase64();
        QCOMPARE(v1.type,stellar::EnvelopeType::ENVELOPE_TYPE_TX);
        Sep10Challenge::ChallengeTransaction* v1ChallengeTransaction = Sep10Challenge::readChallengeTransaction(
                    v1Base64,
                    server->getAccountId(),
                    domainName
                    );

        QVERIFY(v0ChallengeTransaction->equals(v1ChallengeTransaction));


        Sep10Challenge::ChallengeTransaction* challengeTransactionV0 = Sep10Challenge::readChallengeTransaction(
                    v0Base64,
                    server->getAccountId(),
                    domainName
                    );
        QVERIFY(challengeTransactionV0->equals(new Sep10Challenge::ChallengeTransaction(transaction, client->getAccountId())));
        Sep10Challenge::ChallengeTransaction* challengeTransactionV1 = Sep10Challenge::readChallengeTransaction(
                    v1Base64,
                    server->getAccountId(),
                    domainName
                    );
        QVERIFY(challengeTransactionV1->equals(new Sep10Challenge::ChallengeTransaction(transaction, client->getAccountId())));
    }


    void testReadChallengeTransactionRejectsMuxedServer(){
        KeyPair* server = KeyPair::random();
        KeyPair* client = KeyPair::random();

        QString domainName = "example.com";


        Transaction* transaction = Sep10Challenge::buildChallengeTx(
                    server,
                    client->getAccountId(),
                    domainName,
                    300
                    );

        try {
            Sep10Challenge::readChallengeTransaction(
                        transaction->toEnvelopeXdrBase64(),
                        "MCAAAAAAAAAAAAB7BQ2L7E5NBWMXDUCMZSIPOBKRDSBYVLMXGSSKF6YNPIB7Y77ITKNOG",
                        domainName
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

        Transaction* transaction = Sep10Challenge::buildChallengeTx(
                    server,
                    client->getAccountId(),
                    domainName,
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
                        domainName
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


        Transaction* transaction = Sep10Challenge::buildChallengeTx(
                    server,
                    client->getAccountId(),
                    domainName,
                    300
                    );

        transaction->sign(client);

        Sep10Challenge::ChallengeTransaction* challengeTransaction = Sep10Challenge::readChallengeTransaction(transaction->toEnvelopeXdrBase64(), server->getAccountId(), domainName);
        QVERIFY(challengeTransaction->equals(new Sep10Challenge::ChallengeTransaction(transaction, client->getAccountId())));
    }

    void testReadChallengeTransactionInvalidNotSignedByServer() {
        KeyPair* server = KeyPair::random();
        KeyPair* client = KeyPair::random();
        qint64 now = QDateTime::currentMSecsSinceEpoch() / 1000L;
        qint64 end = now + 300;

        QString domainName = "example.com";

        QByteArray nonce(48,'\0');

        QRandomGenerator *r = QRandomGenerator::global();
        r->fillRange(reinterpret_cast<quint32*>(nonce.data()),48/static_cast<int>(sizeof(quint32)));

        QByteArray encodedNonce = nonce.toBase64();

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
            Sep10Challenge::readChallengeTransaction(transaction->toEnvelopeXdrBase64(), server->getAccountId(), domainName);
            QFAIL("Missing exception");
        } catch (std::runtime_error e) {
            //Transaction not signed by server
        }
    }


    void testReadChallengeTransactionInvalidCorrupted() {
        KeyPair* server = KeyPair::random();
        KeyPair* client = KeyPair::random();

        QString domainName = "example.com";

        Transaction* transaction = Sep10Challenge::buildChallengeTx(
                    server,
                    client->getAccountId(),
                    domainName,
                    300
                    );

        QString challenge = transaction->toEnvelopeXdrBase64().replace("A", "B");

        try {
            Sep10Challenge::readChallengeTransaction(challenge, server->getAccountId(),  domainName);
            QFAIL("Missing exception");
        } catch (std::runtime_error) {

        }
    }


    void testReadChallengeTransactionInvalidServerAccountIDMismatch() {
        KeyPair* server = KeyPair::random();
        KeyPair* client = KeyPair::random();


        QString domainName = "example.com";

        Transaction* transaction = Sep10Challenge::buildChallengeTx(
                    server,
                    client->getAccountId(),
                    domainName,
                    300
                    );

        // assertThrows requires Java 8+
        try {
            Sep10Challenge::readChallengeTransaction(transaction->toEnvelopeXdrBase64(), KeyPair::random()->getAccountId(), domainName);
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

        QByteArray nonce(48,'\0');

        QRandomGenerator *r = QRandomGenerator::global();
        r->fillRange(reinterpret_cast<quint32*>(nonce.data()),48/static_cast<int>(sizeof(quint32)));

        QByteArray encodedNonce = nonce.toBase64();

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
            Sep10Challenge::readChallengeTransaction(challenge, server->getAccountId(), domainName);
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

        QByteArray nonce(48,'\0');

        QRandomGenerator *r = QRandomGenerator::global();
        r->fillRange(reinterpret_cast<quint32*>(nonce.data()),48/static_cast<int>(sizeof(quint32)));

        QByteArray encodedNonce = nonce.toBase64();

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
            Sep10Challenge::readChallengeTransaction(challenge, server->getAccountId(), domainName);
            QFAIL("Missing exception");
        } catch (std::runtime_error) {
            //"Transaction requires non-infinite timebounds."
        }
    }


    void testReadChallengeTransactionInvalidNoTimeBounds(){
        KeyPair* server = KeyPair::random();
        KeyPair* client = KeyPair::random();


        QString domainName = "example.com";

        QByteArray nonce(48,'\0');

        QRandomGenerator *r = QRandomGenerator::global();
        r->fillRange(reinterpret_cast<quint32*>(nonce.data()),48/static_cast<int>(sizeof(quint32)));

        QByteArray encodedNonce = nonce.toBase64();

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

        QByteArray nonce(48,'\0');

        QRandomGenerator *r = QRandomGenerator::global();
        r->fillRange(reinterpret_cast<quint32*>(nonce.data()),48/static_cast<int>(sizeof(quint32)));

        QByteArray encodedNonce = nonce.toBase64();

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
            Sep10Challenge::readChallengeTransaction(challenge, server->getAccountId(), domainName);
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

        QByteArray nonce(48,'\0');

        QRandomGenerator *r = QRandomGenerator::global();
        r->fillRange(reinterpret_cast<quint32*>(nonce.data()),48/static_cast<int>(sizeof(quint32)));

        QByteArray encodedNonce = nonce.toBase64();

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
            Sep10Challenge::readChallengeTransaction(challenge, server->getAccountId(), domainName);
            QFAIL("Missing exception");
        } catch (std::runtime_error) {
            //Transaction is not within range of the specified timebounds."
        }
    }

    void testReadChallengeTransactionInvalidTooManyOperations(){
        KeyPair* server = KeyPair::random();
        KeyPair* client = KeyPair::random();
        qint64 now = QDateTime::currentMSecsSinceEpoch() / 1000L;
        qint64 end = now + 300;

        QString domainName = "example.com";

        QByteArray nonce(48,'\0');

        QRandomGenerator *r = QRandomGenerator::global();
        r->fillRange(reinterpret_cast<quint32*>(nonce.data()),48/static_cast<int>(sizeof(quint32)));

        QByteArray encodedNonce = nonce.toBase64();

        Account* sourceAccount = new Account(server, -1L);
        ManageDataOperation* operation1 = ManageDataOperation::create(domainName + " auth", encodedNonce);
        operation1->setSourceAccount(client->getAccountId());
        ManageDataOperation* operation2 = ManageDataOperation::create(domainName + " auth", encodedNonce);
        operation2->setSourceAccount(client->getAccountId());

        Transaction* transaction = Transaction::Builder(sourceAccount)
                .setBaseFee(100)
                .addMemo(Memo::none())
                .addTimeBounds(new TimeBounds(now,end))
                .addOperation(operation1)
                .addOperation(operation2)
                .build();
        transaction->sign(server);
        QString challenge = transaction->toEnvelopeXdrBase64();

        try {
            Sep10Challenge::readChallengeTransaction(challenge, server->getAccountId(), domainName);
            QFAIL("Missing exception");
        } catch (std::runtime_error) {
            //"Transaction requires a single ManageData operation."
        }
    }


    void testReadChallengeTransactionInvalidOperationWrongType(){
        KeyPair* server = KeyPair::random();
        KeyPair* client = KeyPair::random();
        qint64 now = QDateTime::currentMSecsSinceEpoch() / 1000L;
        qint64 end = now + 300;

        QString domainName = "example.com";

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
            Sep10Challenge::readChallengeTransaction(challenge, server->getAccountId(), domainName);
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
        TimeBounds* timeBounds = new TimeBounds(now, end);

        QByteArray nonce(48,'\0');

        QRandomGenerator *r = QRandomGenerator::global();
        r->fillRange(reinterpret_cast<quint32*>(nonce.data()),48/static_cast<int>(sizeof(quint32)));

        QByteArray encodedNonce = nonce.toBase64();

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
            Sep10Challenge::readChallengeTransaction(challenge, server->getAccountId(), domainName);
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
        TimeBounds* timeBounds = new TimeBounds(now, end);

        QByteArray nonce(32,'\0');

        QRandomGenerator *r = QRandomGenerator::global();
        r->fillRange(reinterpret_cast<quint32*>(nonce.data()),32/static_cast<int>(sizeof(quint32)));

        QByteArray encodedNonce = nonce.toBase64();

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
            Sep10Challenge::readChallengeTransaction(challenge, server->getAccountId(), domainName);
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
        TimeBounds* timeBounds = new TimeBounds(now, end);

        QByteArray nonce(48,'\0');

        QRandomGenerator *r = QRandomGenerator::global();
        r->fillRange(reinterpret_cast<quint32*>(nonce.data()),48/static_cast<int>(sizeof(quint32)));

        QByteArray encodedNonce = nonce.toBase64();
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
            Sep10Challenge::readChallengeTransaction(challenge, server->getAccountId(), domainName);
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
        TimeBounds* timeBounds = new TimeBounds(now, end);

        QByteArray nonce(47,'\0');

        QRandomGenerator *r = QRandomGenerator::global();
        r->fillRange(reinterpret_cast<quint32*>(nonce.data()),47/static_cast<int>(sizeof(quint32)));

        QByteArray encodedNonce = nonce.toBase64();

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
            Sep10Challenge::readChallengeTransaction(challenge, server->getAccountId(), domainName);
            QFAIL("Missing exception");
        } catch (std::runtime_error e) {
            //"Random nonce before encoding as base64 should be 48 bytes long."
        }

    }


      void testReadChallengeTransactionInvalidDomainNameMismatch() {
          KeyPair* server = KeyPair::random();
          KeyPair* client = KeyPair::random();
          qint64 now = QDateTime::currentMSecsSinceEpoch() / 1000L;
          qint64 end = now + 300;
          QString domainName = "example.com";
          QString mismatchDomainName = "mismatch_example.com";
          TimeBounds* timeBounds = new TimeBounds(now, end);

          QByteArray nonce(48,'\0');

          QRandomGenerator *r = QRandomGenerator::global();
          r->fillRange(reinterpret_cast<quint32*>(nonce.data()),48/static_cast<int>(sizeof(quint32)));

          QByteArray encodedNonce = nonce.toBase64();

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
              Sep10Challenge::readChallengeTransaction(challenge, server->getAccountId(), mismatchDomainName);
              QFAIL("Missing exception");
          } catch (std::runtime_error e) {
              //The transaction's operation key name does not include the expected home domain.
          }

      }

      void testVerifyChallengeTransactionThresholdInvalidNotSignedByServer(){

        KeyPair* server = KeyPair::random();
        KeyPair* masterClient = KeyPair::random();
        KeyPair* signerClient1 = KeyPair::random();
        KeyPair* signerClient2 = KeyPair::random();
        QString domainName = "example.com";

        qint64 now = QDateTime::currentMSecsSinceEpoch()  / 1000L;
        qint64 end = now + 300;
        TimeBounds* timeBounds = new TimeBounds(now, end);
        QByteArray nonce(48,'\0');

        QRandomGenerator *r = QRandomGenerator::global();
        r->fillRange(reinterpret_cast<quint32*>(nonce.data()),48/static_cast<int>(sizeof(quint32)));

        QByteArray encodedNonce = nonce.toBase64();

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
          Sep10Challenge::verifyChallengeTransactionThreshold(transaction->toEnvelopeXdrBase64(), server->getAccountId(), domainName, threshold, signers);
          QFAIL("Missing exception");
        } catch (std::runtime_error e) {
          //"Transaction not signed by server
        }
      }

      void testVerifyChallengeTransactionThresholdValidServerAndClientKeyMeetingThreshold() {

          KeyPair* server = KeyPair::random();
          KeyPair* masterClient = KeyPair::random();

          QString domainName = "example.com";

          Transaction* transaction = Sep10Challenge::buildChallengeTx(server,masterClient->getAccountId(),domainName,300);
          transaction->sign(masterClient);

          QSet<Sep10Challenge::Signer> signers;
          signers.insert(Sep10Challenge::Signer(masterClient->getAccountId(), 255));
          int threshold=255;
          QSet<QString> signersFound = Sep10Challenge::verifyChallengeTransactionThreshold(transaction->toEnvelopeXdrBase64(), server->getAccountId(), domainName, threshold, signers);
          QSet<QString> expected;
          expected.insert(masterClient->getAccountId());
          QCOMPARE(signersFound,expected);
      }

};
ADD_TEST(Sep10ChallengeTest)
#endif // SEP10CHALLENGETEST_H
