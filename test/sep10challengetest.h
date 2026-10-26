#ifndef SEP10CHALLENGETEST_H
#define SEP10CHALLENGETEST_H
#include <QObject>

#include "src/keypair.h"
#include "src/transaction.h"

#include <QtTest>
#include "testcollector.h"
#include "sep10challenge.h"
#include "account.h"
#include "setoptionsoperation.h"
#include "bumpsequenceoperation.h"
#include "txrep.h"
#include "memo.h"
#include "muxedaccount.h"
#include <QDateTime>
#include <QScopedPointer>
#include <limits>

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
        qDebug() << transaction->getTimeBounds()->getMaxTime()<< (nowPrev+300);
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


    /**
     * A muxed client account (M...) used to be refused here ("Version byte is
     * invalid"), and this test expected that. js and py build its challenge
     * with the M address as the first operation's source, and so does this
     * now; a memo is refused with it, as there, since the M address already
     * names the sub-account.
     */
    void testNewChallengeWithMuxedClientAccount(){
        QScopedPointer<KeyPair> server(KeyPair::random());
        QScopedPointer<KeyPair> client(KeyPair::random());
        const QString muxed = MuxedAccount::fromAccountIdAndMuxedId(client->getAccountId(), 1234).toAddress();

        QScopedPointer<Transaction> tx(Sep10Challenge::buildChallengeTx(
            server.data(), muxed, "example.com", "auth.example.com", 300, Network::testnetNetwork()));
        auto* first = dynamic_cast<ManageDataOperation*>(tx->getOperations().at(0));
        QVERIFY(first);
        QCOMPARE(first->getSourceAccount(), muxed);
        // Written as the M address, not folded into its G: it reads back as M.
        QScopedPointer<Sep10Challenge::ChallengeTransaction> read(Sep10Challenge::readChallengeTransaction(
            tx->toEnvelopeXdrBase64(), server->getAccountId(), QStringList{"example.com"},
            "auth.example.com", Network::testnetNetwork()));
        QCOMPARE(read->getClientAccountId(), muxed);

        try {
            delete Sep10Challenge::buildChallengeTx(server.data(), muxed, "example.com", "auth.example.com", 300,
                                                    Network::testnetNetwork(), QString(), QString(), Memo::id(7));
            QFAIL("Missing exception");
        } catch (const std::runtime_error& e) {
            QCOMPARE(QString(e.what()), QString("memo cannot be used if clientAccountID is a muxed account"));
        }

        // Still refused: a key that is neither, and an M address that does not decode.
        QString corrupt = muxed;
        corrupt[10] = corrupt.at(10) == QLatin1Char('A') ? QLatin1Char('B') : QLatin1Char('A');
        for (const QString& bad : {server->getSecretSeed(), corrupt}) {
            try {
                delete Sep10Challenge::buildChallengeTx(server.data(), bad, "example.com", "auth.example.com", 300,
                                                        Network::testnetNetwork());
                QFAIL("Missing exception");
            } catch (const std::runtime_error&) {}
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
        } catch (const std::runtime_error& e) {
            //"Version byte is invalid"
            Q_UNUSED(e)
        }
    }

    // What is refused here is the muxed SERVER account id this passes (and
    // an envelope that no longer carries the server's signature). A muxed
    // client is read now: testReadAndVerifyChallengeForAMuxedClient.
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
        Transaction* withMuxedClient = Transaction::Builder(AccountConverter().enableMuxed(), account,transaction->getNetwork())
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
        } catch (const std::runtime_error& e) {
            //"invalid address length: MCAAAAAAAAAAAAB7BQ2L7E5NBWMXDUCMZSIPOBKRDSBYVLMXGSSKF6YNPIB7Y77ITKNOG"
            Q_UNUSED(e)
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


        Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), sourceAccount)
                .setBaseFee(100)
                .addOperation(manageDataOperation1)
                .addMemo(Memo::none())
                .addTimeBounds(new TimeBounds(0,end))
                .build();

        transaction->sign(client);
        try {
            Sep10Challenge::readChallengeTransaction(transaction->toEnvelopeXdrBase64(), server->getAccountId(), domainName, webAuthDomain);
            QFAIL("Missing exception");
        } catch (const std::runtime_error& e) {
            //Transaction not signed by server
            Q_UNUSED(e)
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
        } catch (const std::runtime_error&) {

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
        } catch (const std::runtime_error&) {
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

        Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), sourceAccount)
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
        } catch (const std::runtime_error&) {
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


        Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), sourceAccount)
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
        } catch (const std::runtime_error&) {
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
            Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), sourceAccount)
                    .setBaseFee(100)
                    .addMemo(Memo::none())
                    .addOperation(operation)
                    .build();
            Q_UNUSED(transaction)
            //            transaction->sign(server);
            //            QString challenge = transaction->toEnvelopeXdrBase64();
            //            Sep10Challenge::readChallengeTransaction(challenge, server->getAccountId(), domainName);
            QFAIL("Missing exception");
        } catch (const std::runtime_error&) {
            //"Transaction requires timebounds."
        }
    }


    void testReadChallengeTransactionInvalidTimeBoundsTooEarly(){
        KeyPair* server = KeyPair::random();
        KeyPair* client = KeyPair::random();
        qint64 now = QDateTime::currentMSecsSinceEpoch() / 1000L + 3000;
        qint64 end = now + 6000;

        QString domainName = "example.com";
        QString webAuthDomain = "example.com";
        QByteArray nonce(48,'\0');

        QRandomGenerator *r = QRandomGenerator::global();
        r->fillRange(reinterpret_cast<quint32*>(nonce.data()),48/static_cast<int>(sizeof(quint32)));

        QByteArray encodedNonce = nonce.toBase64(QByteArray::Base64Option::Base64UrlEncoding| QByteArray::OmitTrailingEquals);

        Account* sourceAccount = new Account(server, -1L);
        ManageDataOperation* operation = ManageDataOperation::create(domainName + " auth", encodedNonce);
        operation->setSourceAccount(client->getAccountId());


        Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), sourceAccount)
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
        } catch (const std::runtime_error&) {
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


        Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), sourceAccount)
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
        } catch (const std::runtime_error&) {
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

        Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), sourceAccount)
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
        } catch (const std::runtime_error&) {
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

        Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), sourceAccount)
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
        } catch (const std::runtime_error&) {
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

        Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), sourceAccount)
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
        } catch (const std::runtime_error& e) {
            //"Operation should have a source account."
            Q_UNUSED(e)
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

        Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), sourceAccount)
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
        } catch (const std::runtime_error& e) {
            //"Random nonce encoded as base64 should be 64 bytes long."
            Q_UNUSED(e)
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

        Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), sourceAccount)
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
        } catch (const std::runtime_error& e) {
            //"Failed to decode random nonce provided in ManageData operation."
            Q_UNUSED(e)
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

        Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), sourceAccount)
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
        } catch (const std::runtime_error& e) {
            //"Random nonce before encoding as base64 should be 48 bytes long."
            Q_UNUSED(e)
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

        Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), sourceAccount)
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
        } catch (const std::runtime_error&) {
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


        Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), sourceAccount)
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


        Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), sourceAccount)
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
        catch(const std::runtime_error&)
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


        Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), sourceAccount)
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
        catch(const std::runtime_error&)
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

        Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), sourceAccount)
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
        catch(const std::runtime_error&)
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
       } catch (const std::runtime_error& e) {
         QFAIL("Should not have thrown any exception.");
         Q_UNUSED(e)
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
         } catch (const std::runtime_error& e) {
             QFAIL("Should not have thrown any exception.");
             Q_UNUSED(e)
         }
         try {
             Sep10Challenge::readChallengeTransaction(transaction->toEnvelopeXdrBase64(), server->getAccountId(), QStringList()<< "example3.com"<< "example2.com",webAuthDomain);
             QFAIL("Missing exception");
         } catch (const std::runtime_error&) {
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
         } catch (const std::runtime_error& e) {
             QFAIL("Should not have thrown any exception.");
             Q_UNUSED(e)
         }
         try {
             Sep10Challenge::readChallengeTransaction(transaction->toEnvelopeXdrBase64(), server->getAccountId(), QStringList(), webAuthDomain);
             QFAIL("Missing exception");
         } catch (const std::runtime_error&) {
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


        Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), sourceAccount)
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


        Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), sourceAccount)
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
        } catch (const std::runtime_error&) {
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

        Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), sourceAccount)
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
        } catch (const std::runtime_error&) {
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


        Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), sourceAccount)
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
        } catch (const std::runtime_error&) {
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
        } catch (const std::runtime_error& e) {
          QFAIL("Should not have thrown any exception.");
          Q_UNUSED(e)
        }

        try {
          Sep10Challenge::readChallengeTransaction(transaction->toEnvelopeXdrBase64(), server->getAccountId(), domainName, webAuthDomain);
          QFAIL("Missing excepcion");
        } catch (const std::runtime_error& e)
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

        Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), sourceAccount)
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
        } catch (const std::runtime_error& e) {
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

        Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), sourceAccount)
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

    void testChallengeWithClientDomain() {
        KeyPair* server = KeyPair::random();
        KeyPair* client = KeyPair::random();
        KeyPair* clientDomainSigner = KeyPair::random();

        long now = QDateTime::currentMSecsSinceEpoch() / 1000L;
        long end = now + 300;
        TimeBounds* timeBounds = new TimeBounds(now, end);
        QString domainName = "example.com";
        QString webAuthDomain = "auth.example.com";
        QString clientDomain = "client.domain.com";

        Transaction* transaction =
            Sep10Challenge::buildChallengeTx(
                server,
                client->getAccountId(),
                domainName,
                webAuthDomain,
                timeBounds,
                Network::current(),
                clientDomain,
                clientDomainSigner->getAccountId());

        QCOMPARE(transaction->getNetwork(), Network::TESTNET());
        QCOMPARE(transaction->getFee(), 300);
        QCOMPARE(transaction->getTimeBounds(), timeBounds);
        QCOMPARE(transaction->getSourceAccount(), server->getAccountId());
        QCOMPARE(transaction->getSequenceNumber(), 0);

        QCOMPARE(transaction->getOperations().length(), 3);
        ManageDataOperation* homeDomainOp = (ManageDataOperation*) transaction->getOperations()[0];
        QCOMPARE(client->getAccountId(), homeDomainOp->getSourceAccount());
        QCOMPARE(homeDomainOp->getName(), domainName + " auth");

        QCOMPARE(homeDomainOp->getValue().length(), 64);

        //Base64.getDecoder().decode(QString(homeDomainOp->getValue()));

        ManageDataOperation* webAuthDomainOp = (ManageDataOperation*) transaction->getOperations()[1];
        QCOMPARE(server->getAccountId(), webAuthDomainOp->getSourceAccount());
        QCOMPARE(webAuthDomainOp->getName(), "web_auth_domain");
        QCOMPARE(webAuthDomain, webAuthDomainOp->getValue());

        ManageDataOperation* clientAuthDomainOp = (ManageDataOperation*) transaction->getOperations()[2];
        QCOMPARE(clientDomainSigner->getAccountId(), clientAuthDomainOp->getSourceAccount());
        QCOMPARE(clientAuthDomainOp->getName(), "client_domain");
        QCOMPARE(clientDomain, clientAuthDomainOp->getValue());

        QCOMPARE(transaction->getSignatures().size(), 1);
        QVERIFY(
            server->verify(
                transaction->hash(), transaction->getSignatures().at(0).signature.binary()));
      }

    void testChallengeWithClientDomainButWithoutClientDomainSigner() {
        KeyPair* server = KeyPair::random();
        KeyPair* client = KeyPair::random();
        KeyPair* clientDomainSigner = KeyPair::random();

        long now = QDateTime::currentMSecsSinceEpoch() / 1000L;
        long end = now + 300;
        QString domainName = "example.com";
        QString webAuthDomain = "auth.example.com";
        QString clientDomain = "client.domain.com";

        // A fresh TimeBounds per call: buildChallengeTx owns (and frees) the
        // one it is given even when it refuses; the one shared here used to
        // be leaked by the first refusal, then reused.
        try {
            Sep10Challenge::buildChallengeTx(
                server,
                client->getAccountId(),
                domainName,
                webAuthDomain,
                new TimeBounds(now, end),
                Network::TESTNET(),
                "",
                clientDomainSigner->getAccountId());
            QFAIL("Missing exception");
        } catch (std::runtime_error e) {
            QCOMPARE(e.what(), "clientDomain is required if clientSigningKey is provided");
        }

        try {
            Sep10Challenge::buildChallengeTx(
                server,
                client->getAccountId(),
                domainName,
                webAuthDomain,
                new TimeBounds(now, end),
                Network::TESTNET(),
                clientDomain,
                "");
            QFAIL("Missing exception");
        } catch (std::runtime_error e) {
            QCOMPARE(e.what(), "clientDomain is required if clientSigningKey is provided");
        }
    }


    void testVerifyChallengeWithClientDomain() {
        KeyPair* server = KeyPair::random();
        KeyPair* client = KeyPair::random();
        KeyPair* clientDomainSigner = KeyPair::random();

        long now = QDateTime::currentMSecsSinceEpoch() / 1000L;
        long end = now + 300;
        TimeBounds* timeBounds = new TimeBounds(now, end);
        QString domainName = "example.com";
        QString webAuthDomain = "auth.example.com";
        QString clientDomain = "client.domain.com";

        Transaction* transaction =
            Sep10Challenge::buildChallengeTx(
                server,
                client->getAccountId(),
                domainName,
                webAuthDomain,
                timeBounds,
                Network::TESTNET(),
                clientDomain,
                clientDomainSigner->getAccountId());

        transaction->sign(client);
        transaction->sign(clientDomainSigner);

        // should pass if clientDomainSigner is omitted from signers set
        QSet<QString> signers({client->getAccountId()});
        Sep10Challenge::verifyChallengeTransactionSigners(
            transaction->toEnvelopeXdrBase64(),
            server->getAccountId(),
            domainName,
            webAuthDomain,
            signers,
            Network::TESTNET());

        // should pass if clientDomainSigner is included in signers set
        signers = QSet<QString>({client->getAccountId(), clientDomainSigner->getAccountId()});
        Sep10Challenge::verifyChallengeTransactionSigners(
            transaction->toEnvelopeXdrBase64(),
            server->getAccountId(),
            domainName,
            webAuthDomain,
            signers,
            Network::TESTNET());
    }


    void testVerifyChallengeWithClientDomainMissingSignature(){
        KeyPair* server = KeyPair::random();
        KeyPair* client = KeyPair::random();
        KeyPair* clientDomainSigner = KeyPair::random();

        long now = QDateTime::currentMSecsSinceEpoch() / 1000L;
        long end = now + 300;
        TimeBounds* timeBounds = new TimeBounds(now, end);
        QString domainName = "example.com";
        QString webAuthDomain = "auth.example.com";
        QString clientDomain = "client.domain.com";

        Transaction* transaction =
            Sep10Challenge::buildChallengeTx(
                server,
                client->getAccountId(),
                domainName,
                webAuthDomain,
                timeBounds,
                Network::TESTNET(),
                clientDomain,
                clientDomainSigner->getAccountId());

        transaction->sign(client);

        QSet<QString> signers({client->getAccountId()});
        try {
            Sep10Challenge::verifyChallengeTransactionSigners(
                transaction->toEnvelopeXdrBase64(),
                server->getAccountId(),
                domainName,
                webAuthDomain,
                signers,
                Network::TESTNET());
            QFAIL("Missing exception");
        } catch (std::runtime_error e) {
            QCOMPARE(e.what(), "Transaction not signed by by the source account of the 'client_domain'.");
        }
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

        Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), sourceAccount)
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
        } catch (const std::runtime_error& e) {
            //"Transaction not signed by server
            Q_UNUSED(e)
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
        } catch (const std::runtime_error& e) {
            //"Signers with weight 3 do not meet threshold 7."
            Q_UNUSED(e)
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
        } catch (const std::runtime_error& e) {
            //"Transaction has unrecognized signatures."
            Q_UNUSED(e)
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
        } catch (const std::runtime_error& e) {
            //"No verifiable signers provided, at least one G... address must be provided."
            Q_UNUSED(e)
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
        } catch (const std::runtime_error& e) {
            //"No verifiable signers provided, at least one G... address must be provided."
            Q_UNUSED(e)
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

        Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), sourceAccount)
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
        } catch (const std::runtime_error& e) {
            //"Transaction not signed by server
            Q_UNUSED(e)
        }
    }

    /**
     * A secret seed among the signers is refused (js skips it; py 14.1.0
     * refuses it with Ed25519PublicKeyInvalidError, repeating the seed): it
     * is a caller's mistake, and skipping it dropped the account's own signer
     * from the check. The message does not repeat the seed.
     */
    void testASecretSeedAsASignerIsRefused() {
        std::unique_ptr<KeyPair> server(KeyPair::random());
        std::unique_ptr<KeyPair> client(KeyPair::random());
        std::unique_ptr<Transaction> transaction(Sep10Challenge::buildChallengeTx(
                    server.get(), client->getAccountId(), "example.com", "example.com", 300));
        transaction->sign(client.get());

        QSet<QString> signers;
        signers.insert(client->getAccountId());
        signers.insert(client->getSecretSeed());
        try {
            Sep10Challenge::verifyChallengeTransactionSigners(transaction->toEnvelopeXdrBase64(),
                server->getAccountId(), "example.com", "example.com", signers);
            QFAIL("a secret seed was accepted as a signer");
        } catch (const std::runtime_error& e) {
            const QString message = QString::fromUtf8(e.what());
            QVERIFY2(message.contains("secret seed"), qPrintable(message));
            QVERIFY(!message.contains(client->getSecretSeed()));
        }

        // Without it, the same challenge verifies.
        signers.remove(client->getSecretSeed());
        QCOMPARE(Sep10Challenge::verifyChallengeTransactionSigners(transaction->toEnvelopeXdrBase64(),
                     server->getAccountId(), "example.com", "example.com", signers), signers);
    }

    /**
     * A G... signer that is not a valid account id is refused, naming it, as
     * py refuses it (js too, when it reaches it). "GZ" for "GA" gives an
     * unknown version byte (0x36), and such a signer was skipped without a
     * word; a bad checksum is refused the same way.
     */
    void testAGSignerThatIsNotAnAccountIdIsRefused() {
        std::unique_ptr<KeyPair> server(KeyPair::random());
        std::unique_ptr<KeyPair> client(KeyPair::random());
        std::unique_ptr<Transaction> transaction(Sep10Challenge::buildChallengeTx(
                    server.get(), client->getAccountId(), "example.com", "example.com", 300));
        transaction->sign(client.get());

        const QString accountId = client->getAccountId();
        const QString badVersionByte = QStringLiteral("GZ") + accountId.mid(2);
        const QString badChecksum = accountId.left(accountId.size() - 1)
                + (accountId.endsWith(QLatin1Char('A')) ? QStringLiteral("B") : QStringLiteral("A"));
        const QStringList bads = QStringList() << badVersionByte << badChecksum;
        for (const QString& bad : bads) {
            QSet<QString> signers;
            signers.insert(accountId);
            signers.insert(bad);
            try {
                Sep10Challenge::verifyChallengeTransactionSigners(transaction->toEnvelopeXdrBase64(),
                    server->getAccountId(), "example.com", "example.com", signers);
                QFAIL(qPrintable(bad + " was accepted as a signer"));
            } catch (const std::runtime_error& e) {
                const QString message = QString::fromUtf8(e.what());
                QVERIFY2(message.contains("not a valid account id") && message.contains(bad),
                         qPrintable(message));
            }
        }

        // Without it, the same challenge verifies.
        QSet<QString> signers;
        signers.insert(accountId);
        QCOMPARE(Sep10Challenge::verifyChallengeTransactionSigners(transaction->toEnvelopeXdrBase64(),
                     server->getAccountId(), "example.com", "example.com", signers), signers);
    }

    /** SEP-10's endpoint and key from a stellar.toml: an https URL and an account id, or refused. */
    void testWebAuthIsReadFromAToml() {
        std::unique_ptr<KeyPair> server(KeyPair::random());
        QVariantMap toml;
        toml.insert("WEB_AUTH_ENDPOINT", "https://auth.example.com/sep10");
        toml.insert("SIGNING_KEY", server->getAccountId());
        const Sep10Challenge::WebAuth found = Sep10Challenge::webAuthFromToml(toml);
        QCOMPARE(found.endpoint, QString("https://auth.example.com/sep10"));
        QCOMPARE(found.signingKey, server->getAccountId());

        auto refused = [&](const char* key, const QVariant& value) {
            QVariantMap changed = toml;
            if (value.isValid())
                changed.insert(key, value);
            else
                changed.remove(key);
            try {
                Sep10Challenge::webAuthFromToml(changed);
            } catch (const std::exception& e) {
                return QString::fromUtf8(e.what()).contains(key);
            }
            return false;
        };
        QVERIFY(refused("WEB_AUTH_ENDPOINT", QVariant()));
        QVERIFY(refused("WEB_AUTH_ENDPOINT", "http://auth.example.com/sep10"));
        QVERIFY(refused("WEB_AUTH_ENDPOINT", "auth.example.com"));
        QVERIFY(refused("WEB_AUTH_ENDPOINT", 7));
        QVERIFY(refused("SIGNING_KEY", QVariant()));
        QVERIFY(refused("SIGNING_KEY", server->getSecretSeed()));
        QVERIFY(refused("SIGNING_KEY", server->getAccountId().toLower()));
    }

    /**
     * A challenge read with another network than the one it was signed for
     * says which network it was signed for: it failed as a bare "not signed
     * by server".
     */
    void testAChallengeReadOnTheWrongNetworkSaysWhich() {
        std::unique_ptr<KeyPair> server(KeyPair::random());
        std::unique_ptr<KeyPair> client(KeyPair::random());
        std::unique_ptr<Transaction> transaction(Sep10Challenge::buildChallengeTx(
                    server.get(), client->getAccountId(), "example.com", "example.com", 300,
                    Network::testnetNetwork()));
        try {
            delete Sep10Challenge::readChallengeTransaction(transaction->toEnvelopeXdrBase64(), server->getAccountId(),
                                                            "example.com", "example.com", Network::publicNetwork());
            QFAIL("read on the wrong network");
        } catch (const std::exception& e) {
            const QString message = QString::fromUtf8(e.what());
            QVERIFY2(message.startsWith("Transaction not signed by server: it is signed for testnet"), qPrintable(message));
            QVERIFY(message.contains("Public Global Stellar Network ; September 2015"));
        }
        // On its own network it reads.
        std::unique_ptr<Sep10Challenge::ChallengeTransaction> read(Sep10Challenge::readChallengeTransaction(
            transaction->toEnvelopeXdrBase64(), server->getAccountId(), "example.com", "example.com",
            Network::testnetNetwork()));
        QVERIFY(read);
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
        } catch (const std::runtime_error& e) {
            //"Transaction not signed by any client signer."
            Q_UNUSED(e)
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
        } catch (const std::runtime_error& e) {
            //"Transaction has unrecognized signatures."
            Q_UNUSED(e)
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
        catch(const std::runtime_error&)
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
        } catch (const std::runtime_error&) {
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
        } catch (const std::runtime_error&) {
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
        } catch (const std::runtime_error&) {
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
        } catch (const std::runtime_error&) {
            //"No verifiable signers provided, at least one G... address must be provided."
        }
    }

    /**
     * A client_domain challenge signed by the server and the client_domain key
     * only: the client never signed. The client_domain signature used to be
     * counted as the client's by the "any client signer" check, then removed,
     * and verifySigners returned {} without throwing — which threshold 0 (a new
     * account's) accepted as a login.
     */
    void testVerifyChallengeRefusesClientDomainSignatureWithoutTheClient()
    {
        QScopedPointer<KeyPair> server(KeyPair::random());
        QScopedPointer<KeyPair> client(KeyPair::random());
        QScopedPointer<KeyPair> clientDomainSigner(KeyPair::random());
        QScopedPointer<KeyPair> other(KeyPair::random());
        const QString domainName = "example.com";
        const QString webAuthDomain = "auth.example.com";

        QScopedPointer<Transaction> transaction(Sep10Challenge::buildChallengeTx(
            server.data(), client->getAccountId(), domainName, webAuthDomain, 300,
            Network::testnetNetwork(), "wallet.example", clientDomainSigner->getAccountId()));
        transaction->sign(clientDomainSigner.data());
        const QString xdr = transaction->toEnvelopeXdrBase64();

        for (const QSet<QString>& offered : {QSet<QString>{client->getAccountId()},
                                             QSet<QString>{client->getAccountId(), other->getAccountId()}}) {
            try {
                Sep10Challenge::verifyChallengeTransactionSigners(
                    xdr, server->getAccountId(), domainName, webAuthDomain, offered,
                    Network::testnetNetwork());
                QFAIL("Missing exception");
            } catch (const std::runtime_error& e) {
                QCOMPARE(QString(e.what()), QString("Transaction not signed by any client signer."));
            }
        }
        QSet<Sep10Challenge::Signer> weighted;
        weighted.insert(Sep10Challenge::Signer(client->getAccountId(), 1));
        try {
            Sep10Challenge::verifyChallengeTransactionThreshold(
                xdr, server->getAccountId(), domainName, webAuthDomain, 0, weighted,
                Network::testnetNetwork());
            QFAIL("Missing exception");
        } catch (const std::runtime_error& e) {
            QCOMPARE(QString(e.what()), QString("Transaction not signed by any client signer."));
        }

        // Signed by the client as well, it verifies, and only the client counts.
        transaction->sign(client.data());
        QCOMPARE(Sep10Challenge::verifyChallengeTransactionSigners(
                     transaction->toEnvelopeXdrBase64(), server->getAccountId(), domainName,
                     webAuthDomain, QSet<QString>{client->getAccountId()}, Network::testnetNetwork()),
                 QSet<QString>{client->getAccountId()});
    }

    /**
     * Two client_domain operations: only the first key used to be checked, so
     * a second one that never signed went through. js refuses the challenge.
     */
    void testVerifyChallengeRefusesASecondClientDomainOperation()
    {
        QScopedPointer<KeyPair> server(KeyPair::random());
        QScopedPointer<KeyPair> client(KeyPair::random());
        QScopedPointer<KeyPair> clientDomainSigner(KeyPair::random());
        QScopedPointer<KeyPair> other(KeyPair::random());
        const QString sid = server->getAccountId();

        const QString signedFirst = challengeWith(server.data(), {
            rawManageData("example.com auth", nonce64(), client->getAccountId()),
            rawManageData("web_auth_domain", "auth.example.com", sid),
            rawManageData("client_domain", "a.example", clientDomainSigner->getAccountId()),
            rawManageData("client_domain", "b.example", other->getAccountId())},
            {clientDomainSigner.data(), client.data()});
        // The unsigned one first, and both from the key that did sign.
        const QString signedSecond = challengeWith(server.data(), {
            rawManageData("example.com auth", nonce64(), client->getAccountId()),
            rawManageData("web_auth_domain", "auth.example.com", sid),
            rawManageData("client_domain", "b.example", other->getAccountId()),
            rawManageData("client_domain", "a.example", clientDomainSigner->getAccountId())},
            {clientDomainSigner.data(), client.data()});
        const QString sameKey = challengeWith(server.data(), {
            rawManageData("example.com auth", nonce64(), client->getAccountId()),
            rawManageData("web_auth_domain", "auth.example.com", sid),
            rawManageData("client_domain", "a.example", clientDomainSigner->getAccountId()),
            rawManageData("client_domain", "b.example", clientDomainSigner->getAccountId())},
            {clientDomainSigner.data(), client.data()});
        for (const QString& xdr : {signedFirst, signedSecond, sameKey}) {
            try {
                Sep10Challenge::verifyChallengeTransactionSigners(
                    xdr, sid, "example.com", "auth.example.com",
                    QSet<QString>{client->getAccountId()}, Network::testnetNetwork());
                QFAIL("Missing exception");
            } catch (const std::runtime_error& e) {
                QCOMPARE(QString(e.what()), QString("Found more than one client_domain operation"));
            }
        }
    }

    /**
     * QString::fromUtf8 drops a leading UTF-8 BOM, so a name that is signed as
     * EF BB BF "client_domain" read back as "client_domain". The BOM is part of
     * the name, as js, java and py read it.
     */
    void testManageDataNameKeepsALeadingByteOrderMark()
    {
        const QByteArray bom("\xEF\xBB\xBF", 3);
        const QByteArray names[] = {bom + "client_domain", QByteArray("client_") + bom + "domain",
                                    bom + bom + "client_domain", QByteArray("client_domain")};
        for (const QByteArray& bytes : names) {
            QScopedPointer<ManageDataOperation> op(rawManageData(bytes, "v", QString()));
            QCOMPARE(op->getNameBytes(), bytes);
            QCOMPARE(op->getName().toUtf8(), bytes);
        }
        QScopedPointer<ManageDataOperation> op(rawManageData(bom + "client_domain", "v", QString()));
        QCOMPARE(op->getName().size(), 14);
        QCOMPARE(op->getName().at(0), QChar(0xFEFF));
        QVERIFY(op->getName() != Sep10Challenge::CLIENT_DOMAIN_DATA_NAME);

        // TxRep writes the name through getName: it came back without the BOM,
        // a different transaction with a different hash.
        QScopedPointer<KeyPair> source(KeyPair::random());
        Account account(new KeyPair(*source), 41);
        QScopedPointer<Transaction> tx(TransactionBuilder(&account, Network::testnetNetwork())
                                           .addOperation(rawManageData(bom + "k", "v", QString()))
                                           .setTimeout(300)
                                           .setBaseFee(100)
                                           .build());
        QScopedPointer<Transaction> back(Txrep::fromTxrep(Txrep::toTxrep(tx.data()), Network::testnetNetwork()));
        QCOMPARE(back->hashHex(), tx->hashHex());
    }

    /**
     * A first key of BOM + "example.com auth" matched home domain example.com,
     * and a BOM + "client_domain" operation from a key that is not the server's
     * passed as a client_domain one. js and py refuse both.
     */
    void testReadChallengeComparesNamesAsTheSignedBytes()
    {
        QScopedPointer<KeyPair> server(KeyPair::random());
        QScopedPointer<KeyPair> client(KeyPair::random());
        QScopedPointer<KeyPair> other(KeyPair::random());
        const QString sid = server->getAccountId();
        const QByteArray bom("\xEF\xBB\xBF", 3);

        const QString bomHome = challengeWith(server.data(), {
            rawManageData(bom + "example.com auth", nonce64(), client->getAccountId()),
            rawManageData("web_auth_domain", "auth.example.com", sid)});
        try {
            delete Sep10Challenge::readChallengeTransaction(bomHome, sid, QStringList{"example.com"},
                                                            "auth.example.com", Network::testnetNetwork());
            QFAIL("Missing exception");
        } catch (const std::runtime_error& e) {
            QCOMPARE(QString(e.what()), QString("The transaction's operation key name does not include one of the expected home domains."));
        }

        const QString bomClientDomain = challengeWith(server.data(), {
            rawManageData("example.com auth", nonce64(), client->getAccountId()),
            rawManageData("web_auth_domain", "auth.example.com", sid),
            rawManageData(bom + "client_domain", "wallet.example", other->getAccountId())});
        try {
            delete Sep10Challenge::readChallengeTransaction(bomClientDomain, sid, QStringList{"example.com"},
                                                            "auth.example.com", Network::testnetNetwork());
            QFAIL("Missing exception");
        } catch (const std::runtime_error& e) {
            QCOMPARE(QString(e.what()), QString("subsequent operations are unrecognized"));
        }

        // The same challenges without the BOM read fine.
        const QString clean = challengeWith(server.data(), {
            rawManageData("example.com auth", nonce64(), client->getAccountId()),
            rawManageData("web_auth_domain", "auth.example.com", sid),
            rawManageData("client_domain", "wallet.example", other->getAccountId())});
        QScopedPointer<Sep10Challenge::ChallengeTransaction> read(Sep10Challenge::readChallengeTransaction(
            clean, sid, QStringList{"example.com"}, "auth.example.com", Network::testnetNetwork()));
        QCOMPARE(read->getClientAccountId(), client->getAccountId());
    }

    /** The client_domain value was written as Latin-1: "bücher.example" went
     *  out as an invalid UTF-8 byte and "例.jp" as "?.jp", signed by the server. */
    void testChallengeWritesTheClientDomainAsUtf8()
    {
        QScopedPointer<KeyPair> server(KeyPair::random());
        QScopedPointer<KeyPair> client(KeyPair::random());
        QScopedPointer<KeyPair> clientDomainSigner(KeyPair::random());
        for (const QString& clientDomain : {QString::fromUtf8("b\xC3\xBC" "cher.example"),
                                            QString::fromUtf8("\xE4\xBE\x8B.jp"),
                                            QString("wallet.example.com")}) {
            QScopedPointer<Transaction> tx(Sep10Challenge::buildChallengeTx(
                server.data(), client->getAccountId(), "example.com", "auth.example.com", 300,
                Network::testnetNetwork(), clientDomain, clientDomainSigner->getAccountId()));
            auto* op = dynamic_cast<ManageDataOperation*>(tx->getOperations().at(2));
            QVERIFY(op);
            QCOMPARE(op->getNameBytes(), QByteArray("client_domain"));
            QCOMPARE(op->getValue(), clientDomain.toUtf8());
        }
    }

    /**
     * buildChallengeTx(TimeBounds*) signed bounds no reader accepts: never
     * expiring (maxTime 0), already expired, not valid for an hour. Neither
     * overload refused a blank domain (key " auth") or a null webAuthDomain (a
     * web_auth_domain with no value). All of these are refused now.
     */
    void testBuildChallengeRefusesWhatNoReaderAccepts()
    {
        QScopedPointer<KeyPair> server(KeyPair::random());
        QScopedPointer<KeyPair> client(KeyPair::random());
        const QString cid = client->getAccountId();
        const qint64 now = QDateTime::currentMSecsSinceEpoch() / 1000;

        auto refusedWith = [&](TimeBounds* bounds, const QString& domain, const QString& webAuth) {
            try {
                delete Sep10Challenge::buildChallengeTx(server.data(), cid, domain, webAuth, bounds,
                                                        Network::testnetNetwork());
                return false;
            } catch (const std::runtime_error&) {
                return true;
            }
        };
        QVERIFY(refusedWith(nullptr, "example.com", "auth.example.com"));
        QVERIFY(refusedWith(new TimeBounds(now, 0), "example.com", "auth.example.com"));
        QVERIFY(refusedWith(new TimeBounds(0, 0), "example.com", "auth.example.com"));
        QVERIFY(refusedWith(new TimeBounds(now - 1000, now - 500), "example.com", "auth.example.com"));
        QVERIFY(refusedWith(new TimeBounds(now + 1000, now + 2000), "example.com", "auth.example.com"));
        for (const QString& blank : {QString(), QString(""), QString(" "), QString("\t")})
            QVERIFY2(refusedWith(new TimeBounds(now, now + 300), blank, "auth.example.com"), qPrintable(blank));
        QVERIFY(refusedWith(new TimeBounds(now, now + 300), "example.com", QString()));

        for (const QString& blank : {QString(""), QString(" ")}) {
            try {
                delete Sep10Challenge::buildChallengeTx(server.data(), cid, blank, "auth.example.com", 300,
                                                        Network::testnetNetwork());
                QFAIL("Missing exception");
            } catch (const std::runtime_error&) {}
        }
        try {
            delete Sep10Challenge::buildChallengeTx(server.data(), cid, "example.com", QString(), 300,
                                                    Network::testnetNetwork());
            QFAIL("Missing exception");
        } catch (const std::runtime_error&) {}

        // What a reader accepts still builds, and reads back.
        QScopedPointer<Transaction> ok(Sep10Challenge::buildChallengeTx(
            server.data(), cid, "example.com", "auth.example.com", new TimeBounds(now, now + 300),
            Network::testnetNetwork()));
        QScopedPointer<Sep10Challenge::ChallengeTransaction> read(Sep10Challenge::readChallengeTransaction(
            ok->toEnvelopeXdrBase64(), server->getAccountId(), QStringList{"example.com"},
            "auth.example.com", Network::testnetNetwork()));
        QCOMPARE(read->getClientAccountId(), cid);
    }

    /**
     * A refused challenge leaked the memo it was handed, and the caller could
     * not free it either: had the build gone on, the challenge would own it.
     * Both overloads now free it when they refuse.
     */
    void testBuildChallengeFreesTheMemoOfARefusedChallenge()
    {
        QScopedPointer<KeyPair> server(KeyPair::random());
        QScopedPointer<KeyPair> client(KeyPair::random());
        const QString cid = client->getAccountId();
        const qint64 now = QDateTime::currentMSecsSinceEpoch() / 1000;
        int deleted = 0;

        try {
            delete Sep10Challenge::buildChallengeTx(server.data(), cid, "example.com", "auth.example.com",
                                                    qint64(0), Network::testnetNetwork(), QString(),
                                                    QString(), new CountedMemoId(&deleted));
            QFAIL("Missing exception");
        } catch (const std::runtime_error&) {}
        QCOMPARE(deleted, 1);
        try {
            delete Sep10Challenge::buildChallengeTx(server.data(), cid, " ", "auth.example.com",
                                                    new TimeBounds(now, now + 300), Network::testnetNetwork(),
                                                    QString(), QString(), new CountedMemoId(&deleted));
            QFAIL("Missing exception");
        } catch (const std::runtime_error&) {}
        QCOMPARE(deleted, 2);

        // Built, the challenge owns it, once.
        QScopedPointer<Transaction> ok(Sep10Challenge::buildChallengeTx(
            server.data(), cid, "example.com", "auth.example.com", 300, Network::testnetNetwork(),
            QString(), QString(), new CountedMemoId(&deleted)));
        QCOMPARE(deleted, 2);
        ok.reset();
        QCOMPARE(deleted, 3);
    }

    /**
     * A challenge for a muxed client (M...) was refused on reading
     * ("clientAccountId is not a valid account id"), so no server on this SDK
     * could log one in and no client could read one. It reads back as the M
     * address and verifies against the signers of its G account; a memo next
     * to it is refused, as js refuses it.
     */
    void testReadAndVerifyChallengeForAMuxedClient()
    {
        QScopedPointer<KeyPair> server(KeyPair::random());
        QScopedPointer<KeyPair> client(KeyPair::random());
        QScopedPointer<KeyPair> other(KeyPair::random());
        const QString sid = server->getAccountId();
        const QString cid = client->getAccountId();
        const QString muxed = MuxedAccount::fromAccountIdAndMuxedId(cid, 18446744073709551615ULL).toAddress();

        const QString signedByClient = challengeWith(server.data(), {
            rawManageData("example.com auth", nonce64(), muxed),
            rawManageData("web_auth_domain", "auth.example.com", sid)}, {client.data()});
        QScopedPointer<Sep10Challenge::ChallengeTransaction> read(Sep10Challenge::readChallengeTransaction(
            signedByClient, sid, QStringList{"example.com"}, "auth.example.com", Network::testnetNetwork()));
        QCOMPARE(read->getClientAccountId(), muxed);

        QCOMPARE(Sep10Challenge::verifyChallengeTransactionSigners(
                     signedByClient, sid, "example.com", "auth.example.com", QSet<QString>{cid},
                     Network::testnetNetwork()),
                 QSet<QString>{cid});
        QSet<Sep10Challenge::Signer> weighted;
        weighted.insert(Sep10Challenge::Signer(cid, 1));
        QCOMPARE(Sep10Challenge::verifyChallengeTransactionThreshold(
                     signedByClient, sid, QStringList{"example.com"}, "auth.example.com", 1, weighted,
                     Network::testnetNetwork()),
                 QSet<QString>{cid});

        // The M address is not a signer, as js does not take it for one.
        try {
            Sep10Challenge::verifyChallengeTransactionSigners(
                signedByClient, sid, "example.com", "auth.example.com", QSet<QString>{muxed},
                Network::testnetNetwork());
            QFAIL("Missing exception");
        } catch (const std::runtime_error& e) {
            QCOMPARE(QString(e.what()), QString("No verifiable signers provided, at least one G... address must be provided."));
        }
        // Signed by a key that is not the G's: not a login of this account.
        const QString signedByOther = challengeWith(server.data(), {
            rawManageData("example.com auth", nonce64(), muxed),
            rawManageData("web_auth_domain", "auth.example.com", sid)}, {other.data()});
        try {
            Sep10Challenge::verifyChallengeTransactionSigners(
                signedByOther, sid, "example.com", "auth.example.com", QSet<QString>{cid},
                Network::testnetNetwork());
            QFAIL("Missing exception");
        } catch (const std::runtime_error& e) {
            QCOMPARE(QString(e.what()), QString("Transaction not signed by any client signer."));
        }

        const QString withMemo = challengeWith(server.data(), {
            rawManageData("example.com auth", nonce64(), muxed),
            rawManageData("web_auth_domain", "auth.example.com", sid)}, {}, Memo::id(7));
        try {
            delete Sep10Challenge::readChallengeTransaction(withMemo, sid, QStringList{"example.com"},
                                                            "auth.example.com", Network::testnetNetwork());
            QFAIL("Missing exception");
        } catch (const std::runtime_error& e) {
            QCOMPARE(QString(e.what()), QString("The transaction has a memo but the client account ID is a muxed account"));
        }
    }

    /**
     * Two client_domain operations were refused by the verifier only: a client
     * reading the challenge was shown one and signed for both. The reader
     * refuses them too now.
     */
    void testReadChallengeRefusesASecondClientDomainOperation()
    {
        QScopedPointer<KeyPair> server(KeyPair::random());
        QScopedPointer<KeyPair> client(KeyPair::random());
        QScopedPointer<KeyPair> wallet(KeyPair::random());
        QScopedPointer<KeyPair> other(KeyPair::random());
        const QString sid = server->getAccountId();
        const QString wid = wallet->getAccountId();

        const QString twoKeys = challengeWith(server.data(), {
            rawManageData("example.com auth", nonce64(), client->getAccountId()),
            rawManageData("web_auth_domain", "auth.example.com", sid),
            rawManageData("client_domain", "wallet.example", wid),
            rawManageData("client_domain", "b.example", other->getAccountId())});
        const QString sameKey = challengeWith(server.data(), {
            rawManageData("example.com auth", nonce64(), client->getAccountId()),
            rawManageData("web_auth_domain", "auth.example.com", sid),
            rawManageData("client_domain", "b.example", wid),
            rawManageData("client_domain", "wallet.example", wid)});
        for (const QString& xdr : {twoKeys, sameKey}) {
            for (const QString& expected : {QString(), QString("wallet.example")}) {
                try {
                    delete Sep10Challenge::readChallengeTransaction(
                        xdr, sid, QStringList{"example.com"}, "auth.example.com", expected,
                        expected.isEmpty() ? QString() : wid, Network::testnetNetwork());
                    QFAIL("Missing exception");
                } catch (const std::runtime_error& e) {
                    QCOMPARE(QString(e.what()), QString("Found more than one client_domain operation"));
                }
            }
        }
    }

    /**
     * The client_domain operation was accepted from any key and naming any
     * domain, so a client that asked for wallet.example signed a challenge
     * vouching for another domain, or for wallet.example with a key that is
     * not its SIGNING_KEY. Told what it asked for, the reader checks both; not
     * told, it reads as before and reports the operation.
     */
    void testReadChallengeBindsTheClientDomainToItsKey()
    {
        QScopedPointer<KeyPair> server(KeyPair::random());
        QScopedPointer<KeyPair> client(KeyPair::random());
        QScopedPointer<KeyPair> wallet(KeyPair::random());
        QScopedPointer<KeyPair> other(KeyPair::random());
        const QString sid = server->getAccountId();
        const QString wid = wallet->getAccountId();

        auto challenge = [&](const QByteArray& domain, const QString& source) {
            return challengeWith(server.data(), {
                rawManageData("example.com auth", nonce64(), client->getAccountId()),
                rawManageData("web_auth_domain", "auth.example.com", sid),
                rawManageData("client_domain", domain, source)});
        };
        auto read = [&](const QString& xdr, const QString& domain, const QString& key) {
            return Sep10Challenge::readChallengeTransaction(xdr, sid, QStringList{"example.com"},
                                                            "auth.example.com", domain, key,
                                                            Network::testnetNetwork());
        };
        auto refusal = [&](const QString& xdr, const QString& domain, const QString& key) {
            try {
                delete read(xdr, domain, key);
                return QString();
            } catch (const std::runtime_error& e) {
                return QString(e.what());
            }
        };

        const QString good = challenge("wallet.example", wid);
        QScopedPointer<Sep10Challenge::ChallengeTransaction> ok(read(good, "wallet.example", wid));
        QCOMPARE(ok->getClientDomain(), QString("wallet.example"));
        QCOMPARE(ok->getClientDomainSigningKey(), wid);

        QCOMPARE(refusal(challenge("evil.example", wid), "wallet.example", wid),
                 QString("'client_domain' operation value does not match wallet.example"));
        QCOMPARE(refusal(challenge("wallet.example.", wid), "wallet.example", wid),
                 QString("'client_domain' operation value does not match wallet.example"));
        QCOMPARE(refusal(challenge("wallet.example", other->getAccountId()), "wallet.example", wid),
                 QString("'client_domain' operation source is not %1, the signing key of wallet.example").arg(wid));
        const QString none = challengeWith(server.data(), {
            rawManageData("example.com auth", nonce64(), client->getAccountId()),
            rawManageData("web_auth_domain", "auth.example.com", sid)});
        QCOMPARE(refusal(none, "wallet.example", wid),
                 QString("The transaction has no client_domain operation, but wallet.example was expected"));
        // The two halves together or not at all.
        QCOMPARE(refusal(good, "wallet.example", QString()),
                 QString("expectedClientDomainSigningKey is required if expectedClientDomain is provided"));
        QCOMPARE(refusal(good, QString(), wid),
                 QString("expectedClientDomain is required if expectedClientDomainSigningKey is provided"));
        QCOMPARE(refusal(good, "wallet.example", "GWALLET"),
                 QString("expectedClientDomainSigningKey is not a valid account id"));

        // Not told: read as before, and the operation is reported.
        QScopedPointer<Sep10Challenge::ChallengeTransaction> unchecked(Sep10Challenge::readChallengeTransaction(
            challenge("evil.example", other->getAccountId()), sid, QStringList{"example.com"},
            "auth.example.com", Network::testnetNetwork()));
        QCOMPARE(unchecked->getClientDomain(), QString("evil.example"));
        QCOMPARE(unchecked->getClientDomainSigningKey(), other->getAccountId());
        QScopedPointer<Sep10Challenge::ChallengeTransaction> plain(read(none, QString(), QString()));
        QVERIFY(plain->getClientDomain().isEmpty());
        QVERIFY(plain->getClientDomainSigningKey().isEmpty());

        // A client_domain operation from a muxed source: no key can sign for it.
        const QString muxedSource = MuxedAccount::fromAccountIdAndMuxedId(wid, 5).toAddress();
        QCOMPARE(refusal(challenge("wallet.example", muxedSource), QString(), QString()),
                 QString("The client_domain operation's source must be an account id (G...)"));
    }

    /**
     * Challenges built by py-stellar-sdk 14.1.0's build_challenge_transaction
     * (testnet, home domain example.com, web_auth_domain auth.example.com,
     * time.time() fixed at 1761350400, timeout up to 4102444800, a nonce
     * from os.urandom), then signed in py: without a client_domain, with one
     * (wallet.example.org, its key cds) and with an id memo. Each reads back
     * as py's read_challenge_transaction and js-stellar-sdk 17.1.0's
     * readChallengeTx read it (client, matched home domain, memo, the
     * 48-byte nonce), and is refused where both refuse it.
     */
    void testReadChallengesBuiltByPy()
    {
        const QString server = QStringLiteral("GBOJY3PSMHE4XBAEOV3WVLX43FCLIBJSR6VSR6NTVFPPIBEQ2PPIJIWH");
        const QString client = QStringLiteral("GBHNGL3DX427B3XPZMS7FCROD664Q45OFA2WOGYMSRQPL4JOIVLKQEHE");
        const QString cds = QStringLiteral("GAN2IB23O7E6H6Z6ZXQVZWXVEIPTYEBXHZRD66YOD33WGZVQV5YTODGN");
        const QString stranger = QStringLiteral("GBTXU5GZMTSKQT72KAS2RNKTPNOEUN5NNUKG5BKHQOWHGRUEUID4JP7N");
        const QString c1Client = QStringLiteral(
            "AAAAAgAAAABcnG3yYcnLhAR1d2qu/NlEtAUyj6so+bOpXvQEkNPehAAAAMgAAAAAAAAAAAAAAAEAAAAAaPwTAAAAAAD0hlcAAAAA"
            "AAAAAAIAAAABAAAAAE7TL2O/NfDu78sl8oouH73Ic64oNWcbDJRg9fEuRVaoAAAACgAAABBleGFtcGxlLmNvbSBhdXRoAAAAAQAA"
            "AEAxK1doNmZzdHBibGZXTHYxWEcxVEN0eXZPbEVJbFB6TDdzbGRpUzRoRENGNEcyT2YzSGRibythcTlobWF5TVJlAAAAAQAAAABc"
            "nG3yYcnLhAR1d2qu/NlEtAUyj6so+bOpXvQEkNPehAAAAAoAAAAPd2ViX2F1dGhfZG9tYWluAAAAAAEAAAAQYXV0aC5leGFtcGxl"
            "LmNvbQAAAAAAAAACkNPehAAAAEBjDaQkTa0kJLac7hMu4Y+4Y2mV+Eww6h2M/tpMBWzCocXccyXmQ32rm/qPOAez6YzMXxjDoE8C"
            "Oj5miBWHMloOLkVWqAAAAEC9NozD4ch0L1SnRedQ5/XXMx1WjgbGzhV8AJAkqlxQ6Y/K4ujwrqh1fVrLKs3xZiHiWwmRXEmLn6EJ"
            "Nw089akI");
        const QString c2ClientCds = QStringLiteral(
            "AAAAAgAAAABcnG3yYcnLhAR1d2qu/NlEtAUyj6so+bOpXvQEkNPehAAAASwAAAAAAAAAAAAAAAEAAAAAaPwTAAAAAAD0hlcAAAAA"
            "AAAAAAMAAAABAAAAAE7TL2O/NfDu78sl8oouH73Ic64oNWcbDJRg9fEuRVaoAAAACgAAABBleGFtcGxlLmNvbSBhdXRoAAAAAQAA"
            "AEAwYUphS3MxSHRORHBqZTBXYzU3V2ZtRGcwSjRQc2NmdThmd1hXekJkMStzdGhZM1VlcG5ZalBsb2NaZHUvWEpaAAAAAQAAAABc"
            "nG3yYcnLhAR1d2qu/NlEtAUyj6so+bOpXvQEkNPehAAAAAoAAAAPd2ViX2F1dGhfZG9tYWluAAAAAAEAAAAQYXV0aC5leGFtcGxl"
            "LmNvbQAAAAEAAAAAG6QHW3fJ4/s+zeFc2vUiHzwQNz5iP3sOHvdjZrCvcTcAAAAKAAAADWNsaWVudF9kb21haW4AAAAAAAABAAAA"
            "EndhbGxldC5leGFtcGxlLm9yZwAAAAAAAAAAAAOQ096EAAAAQBpR30iSo0qmNWBIjlli/YT3Ij6rI77IFmWJmJZt/AAzvlsCWy8q"
            "VcuwhRVBITUwj4YzWbFq0ADwaHkjaaGF/gwuRVaoAAAAQCzBC8HDtKcfLkzxnN2erTbAJyNBvjxbn85VQr1MtGspYWYlVff+vxW2"
            "4r8G/xiyNWCwi8WixfYvXgs82kveJgewr3E3AAAAQPYwp1NUVtKJhC6M+d5pfV+UVciMc3lPZjCBvzsNmHpwADR2DYB9FEMmHk3z"
            "+O1Rf1XoLK1KQqozsSSWj7AH6Ac=");
        const QString c3Client = QStringLiteral(
            "AAAAAgAAAABcnG3yYcnLhAR1d2qu/NlEtAUyj6so+bOpXvQEkNPehAAAAMgAAAAAAAAAAAAAAAEAAAAAaPwTAAAAAAD0hlcAAAAA"
            "AgAAAAAAEtaHAAAAAgAAAAEAAAAATtMvY7818O7vyyXyii4fvchzrig1ZxsMlGD18S5FVqgAAAAKAAAAEGV4YW1wbGUuY29tIGF1"
            "dGgAAAABAAAAQHh3bDBsOW55WExuTldIT3RRendCNWpMVTlFT2FEYUx0amFjMGlPVzcvNFlId0ozblZDRnJyMWVtLzAycUpDYlQA"
            "AAABAAAAAFycbfJhycuEBHV3aq782US0BTKPqyj5s6le9ASQ096EAAAACgAAAA93ZWJfYXV0aF9kb21haW4AAAAAAQAAABBhdXRo"
            "LmV4YW1wbGUuY29tAAAAAAAAAAKQ096EAAAAQHMpbM4jLpFKkd/B9NcSvZ8gjVG6Kc1XmFTvIUVXKUar1mlrw2vYO1H9LJwEkARn"
            "LkRUtF6yaWNNkx+gm9DrjwUuRVaoAAAAQGmgx6YAA26Wnxnx84Kj3cJm7MFUEX8vZjrR+dd6g7skhzDd7TsBdgWoty5lCY3alNaa"
            "ujasKxVbeo69up583gs=");
        Network* testnet = Network::testnetNetwork();

        QScopedPointer<Sep10Challenge::ChallengeTransaction> read(Sep10Challenge::readChallengeTransaction(
            c2ClientCds, server, QStringList{QStringLiteral("other.com"), QStringLiteral("example.com")},
            QStringLiteral("auth.example.com"), testnet));
        QCOMPARE(read->getClientAccountId(), client);
        QCOMPARE(read->getMatchedHomeDomain(), QStringLiteral("example.com"));
        QCOMPARE(read->getClientDomain(), QStringLiteral("wallet.example.org"));
        QCOMPARE(read->getClientDomainSigningKey(), cds);
        QCOMPARE(read->getTransaction()->getTimeBounds()->getMinTime(), qint64(1761350400));
        QCOMPARE(read->getTransaction()->getTimeBounds()->getMaxTime(), qint64(4102444800LL));
        QCOMPARE(read->getTransaction()->getOperations().size(), 3);
        ManageDataOperation* nonceOp = dynamic_cast<ManageDataOperation*>(read->getTransaction()->getOperations().at(0));
        QVERIFY(nonceOp);
        QCOMPARE(nonceOp->getName(), QStringLiteral("example.com auth"));
        QCOMPARE(nonceOp->getValue().size(), 64);
        QCOMPARE(QByteArray::fromBase64(nonceOp->getValue()).size(), 48);

        QScopedPointer<Sep10Challenge::ChallengeTransaction> withMemo(Sep10Challenge::readChallengeTransaction(
            c3Client, server, QStringLiteral("example.com"), QStringLiteral("auth.example.com"), testnet));
        QCOMPARE(withMemo->getClientAccountId(), client);
        MemoId* memo = dynamic_cast<MemoId*>(withMemo->getTransaction()->getMemo());
        QVERIFY(memo);
        QCOMPARE(memo->getId(), quint64(1234567));

        // Refused by py and js alike: another home domain, the public
        // network (the server signed for testnet), another server account,
        // another web_auth_domain.
        const struct { QString server; QString domain; QString webAuth; Network* network; } refused[] = {
            {server, QStringLiteral("other.com"), QStringLiteral("auth.example.com"), testnet},
            {server, QStringLiteral("example.com"), QStringLiteral("auth.example.com"), Network::publicNetwork()},
            {stranger, QStringLiteral("example.com"), QStringLiteral("auth.example.com"), testnet},
            {server, QStringLiteral("example.com"), QStringLiteral("wallet.example.org"), testnet},
        };
        for (const auto& r : refused) {
            bool threw = false;
            try {
                delete Sep10Challenge::readChallengeTransaction(c1Client, r.server, r.domain, r.webAuth, r.network);
            } catch (const std::runtime_error&) {
                threw = true;
            }
            QVERIFY2(threw, qPrintable(r.domain + QStringLiteral(" / ") + r.webAuth));
        }
    }

    /**
     * Server-side verification of challenges built and signed by
     * py-stellar-sdk 14.1.0 (see testReadChallengesBuiltByPy), for four
     * signer sets and four weighted sets: the signers found, or a refusal
     * (none), are those py's verify_challenge_transaction_signers /
     * _threshold and js-stellar-sdk 17.1.0's verifyChallengeTxSigners /
     * verifyChallengeTxThreshold give; the two agree on every case, only
     * their messages differ. The challenges: signed by the client; by it and
     * s2; by no client; by it and a stranger; by s2 and s3 only; with a
     * client_domain signed by the client and cds, by the client only, by
     * cds only; with an id memo.
     */
    void testVerifyChallengesBuiltByPyAsJsAndPy()
    {
        const QString server = QStringLiteral("GBOJY3PSMHE4XBAEOV3WVLX43FCLIBJSR6VSR6NTVFPPIBEQ2PPIJIWH");
        const QString client = QStringLiteral("GBHNGL3DX427B3XPZMS7FCROD664Q45OFA2WOGYMSRQPL4JOIVLKQEHE");
        const QString s2 = QStringLiteral("GCEEXCCX6TVKCYJ4MFIE3M2NJPVPGRSRPIHDDXR43XKNTNBADWOQWDYC");
        const QString s3 = QStringLiteral("GCQJVJPUPJTVTABP7FK7RXBNFIKKLSM5EO7JP6DECJ77SOBUKWSPB64N");
        const QString cds = QStringLiteral("GAN2IB23O7E6H6Z6ZXQVZWXVEIPTYEBXHZRD66YOD33WGZVQV5YTODGN");
        const QString stranger = QStringLiteral("GBTXU5GZMTSKQT72KAS2RNKTPNOEUN5NNUKG5BKHQOWHGRUEUID4JP7N");
        const QSet<QString> none;
        const struct { const char* name; QString xdr; QSet<QString> expected[8]; } cases[] = {
            {"c1_client", QStringLiteral(
                 "AAAAAgAAAABcnG3yYcnLhAR1d2qu/NlEtAUyj6so+bOpXvQEkNPehAAAAMgAAAAAAAAAAAAAAAEAAAAAaPwTAAAAAAD0hlcAAAAA"
                 "AAAAAAIAAAABAAAAAE7TL2O/NfDu78sl8oouH73Ic64oNWcbDJRg9fEuRVaoAAAACgAAABBleGFtcGxlLmNvbSBhdXRoAAAAAQAA"
                 "AEAxK1doNmZzdHBibGZXTHYxWEcxVEN0eXZPbEVJbFB6TDdzbGRpUzRoRENGNEcyT2YzSGRibythcTlobWF5TVJlAAAAAQAAAABc"
                 "nG3yYcnLhAR1d2qu/NlEtAUyj6so+bOpXvQEkNPehAAAAAoAAAAPd2ViX2F1dGhfZG9tYWluAAAAAAEAAAAQYXV0aC5leGFtcGxl"
                 "LmNvbQAAAAAAAAACkNPehAAAAEBjDaQkTa0kJLac7hMu4Y+4Y2mV+Eww6h2M/tpMBWzCocXccyXmQ32rm/qPOAez6YzMXxjDoE8C"
                 "Oj5miBWHMloOLkVWqAAAAEC9NozD4ch0L1SnRedQ5/XXMx1WjgbGzhV8AJAkqlxQ6Y/K4ujwrqh1fVrLKs3xZiHiWwmRXEmLn6EJ"
                 "Nw089akI"),
             {QSet<QString>{client}, QSet<QString>{client}, QSet<QString>{client}, none,
              none, none, QSet<QString>{client}, none}},
            {"c1_client_s2", QStringLiteral(
                 "AAAAAgAAAABcnG3yYcnLhAR1d2qu/NlEtAUyj6so+bOpXvQEkNPehAAAAMgAAAAAAAAAAAAAAAEAAAAAaPwTAAAAAAD0hlcAAAAA"
                 "AAAAAAIAAAABAAAAAE7TL2O/NfDu78sl8oouH73Ic64oNWcbDJRg9fEuRVaoAAAACgAAABBleGFtcGxlLmNvbSBhdXRoAAAAAQAA"
                 "AEAxK1doNmZzdHBibGZXTHYxWEcxVEN0eXZPbEVJbFB6TDdzbGRpUzRoRENGNEcyT2YzSGRibythcTlobWF5TVJlAAAAAQAAAABc"
                 "nG3yYcnLhAR1d2qu/NlEtAUyj6so+bOpXvQEkNPehAAAAAoAAAAPd2ViX2F1dGhfZG9tYWluAAAAAAEAAAAQYXV0aC5leGFtcGxl"
                 "LmNvbQAAAAAAAAADkNPehAAAAEBjDaQkTa0kJLac7hMu4Y+4Y2mV+Eww6h2M/tpMBWzCocXccyXmQ32rm/qPOAez6YzMXxjDoE8C"
                 "Oj5miBWHMloOLkVWqAAAAEC9NozD4ch0L1SnRedQ5/XXMx1WjgbGzhV8AJAkqlxQ6Y/K4ujwrqh1fVrLKs3xZiHiWwmRXEmLn6EJ"
                 "Nw089akIIB2dCwAAAEDxxPdrXELUjeWvmaQl4Y4jYXz4KPDiD2GPkczp0eI9YkYH/3k0wiU8Xe1w+REicsKwe5lRevgyBtHJm9iR"
                 "c10G"),
             {none, QSet<QString>{client, s2}, none, none,
              QSet<QString>{client, s2}, none, none, none}},
            {"c1_server", QStringLiteral(
                 "AAAAAgAAAABcnG3yYcnLhAR1d2qu/NlEtAUyj6so+bOpXvQEkNPehAAAAMgAAAAAAAAAAAAAAAEAAAAAaPwTAAAAAAD0hlcAAAAA"
                 "AAAAAAIAAAABAAAAAE7TL2O/NfDu78sl8oouH73Ic64oNWcbDJRg9fEuRVaoAAAACgAAABBleGFtcGxlLmNvbSBhdXRoAAAAAQAA"
                 "AEAxK1doNmZzdHBibGZXTHYxWEcxVEN0eXZPbEVJbFB6TDdzbGRpUzRoRENGNEcyT2YzSGRibythcTlobWF5TVJlAAAAAQAAAABc"
                 "nG3yYcnLhAR1d2qu/NlEtAUyj6so+bOpXvQEkNPehAAAAAoAAAAPd2ViX2F1dGhfZG9tYWluAAAAAAEAAAAQYXV0aC5leGFtcGxl"
                 "LmNvbQAAAAAAAAABkNPehAAAAEBjDaQkTa0kJLac7hMu4Y+4Y2mV+Eww6h2M/tpMBWzCocXccyXmQ32rm/qPOAez6YzMXxjDoE8C"
                 "Oj5miBWHMloO"),
             {none, none, none, none,
              none, none, none, none}},
            {"c1_client_stranger", QStringLiteral(
                 "AAAAAgAAAABcnG3yYcnLhAR1d2qu/NlEtAUyj6so+bOpXvQEkNPehAAAAMgAAAAAAAAAAAAAAAEAAAAAaPwTAAAAAAD0hlcAAAAA"
                 "AAAAAAIAAAABAAAAAE7TL2O/NfDu78sl8oouH73Ic64oNWcbDJRg9fEuRVaoAAAACgAAABBleGFtcGxlLmNvbSBhdXRoAAAAAQAA"
                 "AEAxK1doNmZzdHBibGZXTHYxWEcxVEN0eXZPbEVJbFB6TDdzbGRpUzRoRENGNEcyT2YzSGRibythcTlobWF5TVJlAAAAAQAAAABc"
                 "nG3yYcnLhAR1d2qu/NlEtAUyj6so+bOpXvQEkNPehAAAAAoAAAAPd2ViX2F1dGhfZG9tYWluAAAAAAEAAAAQYXV0aC5leGFtcGxl"
                 "LmNvbQAAAAAAAAADkNPehAAAAEBjDaQkTa0kJLac7hMu4Y+4Y2mV+Eww6h2M/tpMBWzCocXccyXmQ32rm/qPOAez6YzMXxjDoE8C"
                 "Oj5miBWHMloOLkVWqAAAAEC9NozD4ch0L1SnRedQ5/XXMx1WjgbGzhV8AJAkqlxQ6Y/K4ujwrqh1fVrLKs3xZiHiWwmRXEmLn6EJ"
                 "Nw089akIhKIHxAAAAECDjmSXWASzmSiyw8g9oPMNLVKFU1bhQWpbzQirUGfMLAXbQHlJHBNKTjMA6gf92RJ05DFMiZRPn6esgfQo"
                 "ck0L"),
             {none, none, none, none,
              none, none, none, none}},
            {"c1_s2_s3", QStringLiteral(
                 "AAAAAgAAAABcnG3yYcnLhAR1d2qu/NlEtAUyj6so+bOpXvQEkNPehAAAAMgAAAAAAAAAAAAAAAEAAAAAaPwTAAAAAAD0hlcAAAAA"
                 "AAAAAAIAAAABAAAAAE7TL2O/NfDu78sl8oouH73Ic64oNWcbDJRg9fEuRVaoAAAACgAAABBleGFtcGxlLmNvbSBhdXRoAAAAAQAA"
                 "AEAxK1doNmZzdHBibGZXTHYxWEcxVEN0eXZPbEVJbFB6TDdzbGRpUzRoRENGNEcyT2YzSGRibythcTlobWF5TVJlAAAAAQAAAABc"
                 "nG3yYcnLhAR1d2qu/NlEtAUyj6so+bOpXvQEkNPehAAAAAoAAAAPd2ViX2F1dGhfZG9tYWluAAAAAAEAAAAQYXV0aC5leGFtcGxl"
                 "LmNvbQAAAAAAAAADkNPehAAAAEBjDaQkTa0kJLac7hMu4Y+4Y2mV+Eww6h2M/tpMBWzCocXccyXmQ32rm/qPOAez6YzMXxjDoE8C"
                 "Oj5miBWHMloOIB2dCwAAAEDxxPdrXELUjeWvmaQl4Y4jYXz4KPDiD2GPkczp0eI9YkYH/3k0wiU8Xe1w+REicsKwe5lRevgyBtHJ"
                 "m9iRc10GNFWk8AAAAECW/YLRr5rkeJRd0L2kJ9Yu5r6D2kjhWU/V9VQ8S6s5jT/Dk4jsAt6yTSnj/VUXveBwoOFw1p1TgXaVLPKT"
                 "rPIL"),
             {none, QSet<QString>{s2, s3}, none, QSet<QString>{s2, s3},
              QSet<QString>{s2, s3}, QSet<QString>{s2, s3}, none, QSet<QString>{s2, s3}}},
            {"c2_client_cds", QStringLiteral(
                 "AAAAAgAAAABcnG3yYcnLhAR1d2qu/NlEtAUyj6so+bOpXvQEkNPehAAAASwAAAAAAAAAAAAAAAEAAAAAaPwTAAAAAAD0hlcAAAAA"
                 "AAAAAAMAAAABAAAAAE7TL2O/NfDu78sl8oouH73Ic64oNWcbDJRg9fEuRVaoAAAACgAAABBleGFtcGxlLmNvbSBhdXRoAAAAAQAA"
                 "AEAwYUphS3MxSHRORHBqZTBXYzU3V2ZtRGcwSjRQc2NmdThmd1hXekJkMStzdGhZM1VlcG5ZalBsb2NaZHUvWEpaAAAAAQAAAABc"
                 "nG3yYcnLhAR1d2qu/NlEtAUyj6so+bOpXvQEkNPehAAAAAoAAAAPd2ViX2F1dGhfZG9tYWluAAAAAAEAAAAQYXV0aC5leGFtcGxl"
                 "LmNvbQAAAAEAAAAAG6QHW3fJ4/s+zeFc2vUiHzwQNz5iP3sOHvdjZrCvcTcAAAAKAAAADWNsaWVudF9kb21haW4AAAAAAAABAAAA"
                 "EndhbGxldC5leGFtcGxlLm9yZwAAAAAAAAAAAAOQ096EAAAAQBpR30iSo0qmNWBIjlli/YT3Ij6rI77IFmWJmJZt/AAzvlsCWy8q"
                 "VcuwhRVBITUwj4YzWbFq0ADwaHkjaaGF/gwuRVaoAAAAQCzBC8HDtKcfLkzxnN2erTbAJyNBvjxbn85VQr1MtGspYWYlVff+vxW2"
                 "4r8G/xiyNWCwi8WixfYvXgs82kveJgewr3E3AAAAQPYwp1NUVtKJhC6M+d5pfV+UVciMc3lPZjCBvzsNmHpwADR2DYB9FEMmHk3z"
                 "+O1Rf1XoLK1KQqozsSSWj7AH6Ac="),
             {QSet<QString>{client}, QSet<QString>{client}, QSet<QString>{client}, none,
              none, none, QSet<QString>{client}, none}},
            {"c2_client", QStringLiteral(
                 "AAAAAgAAAABcnG3yYcnLhAR1d2qu/NlEtAUyj6so+bOpXvQEkNPehAAAASwAAAAAAAAAAAAAAAEAAAAAaPwTAAAAAAD0hlcAAAAA"
                 "AAAAAAMAAAABAAAAAE7TL2O/NfDu78sl8oouH73Ic64oNWcbDJRg9fEuRVaoAAAACgAAABBleGFtcGxlLmNvbSBhdXRoAAAAAQAA"
                 "AEAwYUphS3MxSHRORHBqZTBXYzU3V2ZtRGcwSjRQc2NmdThmd1hXekJkMStzdGhZM1VlcG5ZalBsb2NaZHUvWEpaAAAAAQAAAABc"
                 "nG3yYcnLhAR1d2qu/NlEtAUyj6so+bOpXvQEkNPehAAAAAoAAAAPd2ViX2F1dGhfZG9tYWluAAAAAAEAAAAQYXV0aC5leGFtcGxl"
                 "LmNvbQAAAAEAAAAAG6QHW3fJ4/s+zeFc2vUiHzwQNz5iP3sOHvdjZrCvcTcAAAAKAAAADWNsaWVudF9kb21haW4AAAAAAAABAAAA"
                 "EndhbGxldC5leGFtcGxlLm9yZwAAAAAAAAAAAAKQ096EAAAAQBpR30iSo0qmNWBIjlli/YT3Ij6rI77IFmWJmJZt/AAzvlsCWy8q"
                 "VcuwhRVBITUwj4YzWbFq0ADwaHkjaaGF/gwuRVaoAAAAQCzBC8HDtKcfLkzxnN2erTbAJyNBvjxbn85VQr1MtGspYWYlVff+vxW2"
                 "4r8G/xiyNWCwi8WixfYvXgs82kveJgc="),
             {none, none, none, none,
              none, none, none, none}},
            {"c2_cds", QStringLiteral(
                 "AAAAAgAAAABcnG3yYcnLhAR1d2qu/NlEtAUyj6so+bOpXvQEkNPehAAAASwAAAAAAAAAAAAAAAEAAAAAaPwTAAAAAAD0hlcAAAAA"
                 "AAAAAAMAAAABAAAAAE7TL2O/NfDu78sl8oouH73Ic64oNWcbDJRg9fEuRVaoAAAACgAAABBleGFtcGxlLmNvbSBhdXRoAAAAAQAA"
                 "AEAwYUphS3MxSHRORHBqZTBXYzU3V2ZtRGcwSjRQc2NmdThmd1hXekJkMStzdGhZM1VlcG5ZalBsb2NaZHUvWEpaAAAAAQAAAABc"
                 "nG3yYcnLhAR1d2qu/NlEtAUyj6so+bOpXvQEkNPehAAAAAoAAAAPd2ViX2F1dGhfZG9tYWluAAAAAAEAAAAQYXV0aC5leGFtcGxl"
                 "LmNvbQAAAAEAAAAAG6QHW3fJ4/s+zeFc2vUiHzwQNz5iP3sOHvdjZrCvcTcAAAAKAAAADWNsaWVudF9kb21haW4AAAAAAAABAAAA"
                 "EndhbGxldC5leGFtcGxlLm9yZwAAAAAAAAAAAAKQ096EAAAAQBpR30iSo0qmNWBIjlli/YT3Ij6rI77IFmWJmJZt/AAzvlsCWy8q"
                 "VcuwhRVBITUwj4YzWbFq0ADwaHkjaaGF/gywr3E3AAAAQPYwp1NUVtKJhC6M+d5pfV+UVciMc3lPZjCBvzsNmHpwADR2DYB9FEMm"
                 "Hk3z+O1Rf1XoLK1KQqozsSSWj7AH6Ac="),
             {none, none, none, none,
              none, none, none, none}},
            {"c3_client", QStringLiteral(
                 "AAAAAgAAAABcnG3yYcnLhAR1d2qu/NlEtAUyj6so+bOpXvQEkNPehAAAAMgAAAAAAAAAAAAAAAEAAAAAaPwTAAAAAAD0hlcAAAAA"
                 "AgAAAAAAEtaHAAAAAgAAAAEAAAAATtMvY7818O7vyyXyii4fvchzrig1ZxsMlGD18S5FVqgAAAAKAAAAEGV4YW1wbGUuY29tIGF1"
                 "dGgAAAABAAAAQHh3bDBsOW55WExuTldIT3RRendCNWpMVTlFT2FEYUx0amFjMGlPVzcvNFlId0ozblZDRnJyMWVtLzAycUpDYlQA"
                 "AAABAAAAAFycbfJhycuEBHV3aq782US0BTKPqyj5s6le9ASQ096EAAAACgAAAA93ZWJfYXV0aF9kb21haW4AAAAAAQAAABBhdXRo"
                 "LmV4YW1wbGUuY29tAAAAAAAAAAKQ096EAAAAQHMpbM4jLpFKkd/B9NcSvZ8gjVG6Kc1XmFTvIUVXKUar1mlrw2vYO1H9LJwEkARn"
                 "LkRUtF6yaWNNkx+gm9DrjwUuRVaoAAAAQGmgx6YAA26Wnxnx84Kj3cJm7MFUEX8vZjrR+dd6g7skhzDd7TsBdgWoty5lCY3alNaa"
                 "ujasKxVbeo69up583gs="),
             {QSet<QString>{client}, QSet<QString>{client}, QSet<QString>{client}, none,
              none, none, QSet<QString>{client}, none}},
        };
        const QSet<QString> signerSets[4] = {
            QSet<QString>{client}, QSet<QString>{client, s2, s3, server},
            QSet<QString>{client, cds}, QSet<QString>{s2, s3}};
        QSet<Sep10Challenge::Signer> heavy;
        heavy.insert(Sep10Challenge::Signer(client, 1));
        heavy.insert(Sep10Challenge::Signer(s2, 2));
        heavy.insert(Sep10Challenge::Signer(s3, 4));
        QSet<Sep10Challenge::Signer> master;
        master.insert(Sep10Challenge::Signer(client, 1));
        QSet<Sep10Challenge::Signer> others;
        others.insert(Sep10Challenge::Signer(s2, 2));
        others.insert(Sep10Challenge::Signer(s3, 4));
        const QSet<Sep10Challenge::Signer> weightedSigners[4] = {heavy, heavy, master, others};
        const int thresholds[4] = {3, 4, 1, 6};
        Network* testnet = Network::testnetNetwork();
        const QString home = QStringLiteral("example.com");
        const QString webAuth = QStringLiteral("auth.example.com");

        for (const auto& c : cases) {
            for (int i = 0; i < 8; ++i) {
                QSet<QString> found;
                try {
                    if (i < 4)
                        found = Sep10Challenge::verifyChallengeTransactionSigners(
                            c.xdr, server, home, webAuth, signerSets[i], testnet);
                    else
                        found = Sep10Challenge::verifyChallengeTransactionThreshold(
                            c.xdr, server, home, webAuth, thresholds[i - 4], weightedSigners[i - 4], testnet);
                    QVERIFY2(!found.isEmpty(), c.name);
                } catch (const std::runtime_error&) {
                    found.clear();
                }
                QVERIFY2(found == c.expected[i],
                         qPrintable(QStringLiteral("%1, set %2: %3 signers found, %4 expected")
                                        .arg(QLatin1String(c.name)).arg(i)
                                        .arg(found.size()).arg(c.expected[i].size())));
            }
        }
    }

    /**
     * Signer weights as Horizon gives them: an int32, past 255 for a signer
     * added before protocol 10, and negative from 2^31 on (3000000000 comes
     * as -1294967296). stellar-core counts a weight past a byte as 255. Here
     * a negative weight went into a quint32, 2^32 minus it, and on into an
     * int sum, which wrapped: one such signer, or two of 2^31-1, came to less
     * than the threshold, and signers that meet it were refused.
     */
    void testVerifyChallengeTransactionThresholdCountsWeightsAsCore()
    {
        QScopedPointer<KeyPair> server(KeyPair::random());
        QScopedPointer<KeyPair> client(KeyPair::random());
        QScopedPointer<KeyPair> second(KeyPair::random());
        QScopedPointer<KeyPair> third(KeyPair::random());
        const QString sid = server->getAccountId();
        const QString cid = client->getAccountId();
        const QString domainName = "example.com";
        const QString webAuthDomain = "auth.example.com";
        const int heaviest = std::numeric_limits<int>::max();
        const QString notMet("Signers weight do not meet threshold.");

        auto signedBy = [&](const QList<KeyPair*>& signers) {
            QScopedPointer<Transaction> challenge(Sep10Challenge::buildChallengeTx(
                server.data(), cid, domainName, webAuthDomain, 300, Network::testnetNetwork()));
            for (KeyPair* signer : signers)
                challenge->sign(signer);
            return challenge->toEnvelopeXdrBase64();
        };
        auto refusal = [&](const QString& xdr, int threshold, const QSet<Sep10Challenge::Signer>& signers) {
            try {
                Sep10Challenge::verifyChallengeTransactionThreshold(xdr, sid, domainName, webAuthDomain, threshold,
                                                                    signers, Network::testnetNetwork());
                return QString();
            } catch (const std::runtime_error& e) {
                return QString(e.what());
            }
        };

        const QString byOne = signedBy({client.data()});
        for (const int weight : { 255, 300, -1294967296, -1, std::numeric_limits<int>::min(), heaviest }) {
            const QString why = refusal(byOne, 255, {Sep10Challenge::Signer(cid, weight)});
            QVERIFY2(why.isEmpty(), qPrintable(QString::number(weight) + ": " + why));
        }
        // 255, as core counts it, and no more.
        QCOMPARE(refusal(byOne, 256, {Sep10Challenge::Signer(cid, 300)}), notMet);
        QCOMPARE(refusal(byOne, 256, {Sep10Challenge::Signer(cid, -1294967296)}), notMet);
        QCOMPARE(refusal(byOne, 1, {Sep10Challenge::Signer(cid, 0)}), notMet);

        // Two of 2^31-1 wrapped the int sum to -2; three of them, to 2^31-3.
        const QString byTwo = signedBy({client.data(), second.data()});
        const QSet<Sep10Challenge::Signer> two{Sep10Challenge::Signer(cid, heaviest),
                                               Sep10Challenge::Signer(second->getAccountId(), heaviest)};
        QVERIFY(refusal(byTwo, 255, two).isEmpty());
        QVERIFY(refusal(byTwo, 510, two).isEmpty());
        QCOMPARE(refusal(byTwo, 511, two), notMet);
        const QString byThree = signedBy({client.data(), second.data(), third.data()});
        const QSet<Sep10Challenge::Signer> three = QSet<Sep10Challenge::Signer>(two)
                                                   << Sep10Challenge::Signer(third->getAccountId(), heaviest);
        QVERIFY(refusal(byThree, 255, three).isEmpty());
        QVERIFY(refusal(byThree, 765, three).isEmpty());
        QCOMPARE(refusal(byThree, 766, three), notMet);
        // Mixed: 3000000000 (as Horizon gives it) and 1.
        const QSet<Sep10Challenge::Signer> mixed{Sep10Challenge::Signer(cid, -1294967296),
                                                 Sep10Challenge::Signer(second->getAccountId(), 1)};
        QVERIFY(refusal(byTwo, 256, mixed).isEmpty());
        QCOMPARE(refusal(byTwo, 257, mixed), notMet);
    }

private:
    /** A MemoId that counts its deletions, to see who frees it. */
    class CountedMemoId : public MemoId {
        int* m_deleted;
    public:
        explicit CountedMemoId(int* deleted) : MemoId(7), m_deleted(deleted) {}
        ~CountedMemoId() override { ++*m_deleted; }
    };

    /** A ManageData operation carrying exactly these name bytes. */
    static ManageDataOperation* rawManageData(const QByteArray& name, const QByteArray& value,
                                              const QString& source)
    {
        stellar::ManageDataOp op;
        op.dataName.set(const_cast<char*>(name.data()), static_cast<int>(name.size()));
        op.dataValue.value.set(reinterpret_cast<uchar*>(const_cast<char*>(value.data())), static_cast<int>(value.size()));
        op.dataValue.filled = 1;
        auto* md = new ManageDataOperation(op);
        if (!source.isEmpty())
            md->setSourceAccount(source);
        return md;
    }

    static QByteArray nonce64() { return QByteArray(48, '\x5a').toBase64(); }

    /** A challenge from `ops`, signed by the server and then by `signers`. */
    static QString challengeWith(KeyPair* server, const QList<ManageDataOperation*>& ops,
                                 const QList<KeyPair*>& signers = QList<KeyPair*>(),
                                 Memo* memo = nullptr)
    {
        const qint64 now = QDateTime::currentMSecsSinceEpoch() / 1000;
        Account account(new KeyPair(*server), -1);
        TransactionBuilder builder(&account, Network::testnetNetwork());
        for (ManageDataOperation* op : ops)
            builder.addOperation(op);
        if (memo)
            builder.addMemo(memo);
        builder.addTimeBounds(new TimeBounds(now, now + 300)).setBaseFee(100);
        QScopedPointer<Transaction> tx(builder.build());
        tx->sign(server);
        for (KeyPair* signer : signers)
            tx->sign(signer);
        return tx->toEnvelopeXdrBase64();
    }
};
ADD_TEST(Sep10ChallengeTest)
#endif // SEP10CHALLENGETEST_H
