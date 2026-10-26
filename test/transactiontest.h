#ifndef TRANSACTIONTEST_H
#define TRANSACTIONTEST_H
#include <QObject>
#include "src/keypair.h"
#include "src/util.h"

#include <QtTest>
#include "testcollector.h"
#include "src/network.h"
#include "src/account.h"
#include "src/transaction.h"
#include "src/createaccountoperation.h"
#include "src/paymentoperation.h"
#include "src/assettypenative.h"
#include "src/transactionbuilder.h"
#include "src/revoketrustlinesponsorshipoperation.h"
#include "src/abstracttransaction.h"
#include <QScopedPointer>
#include <cstring>

class TransactionTest: public QObject
{
    Q_OBJECT
public:


private slots:

    void initTestCase(){
        Network::useTestNetwork();
    }
    void cleanupTestCase()
    {

    }
//    void testDefaultBaseFee(){
//      // GBPMKIRA2OQW2XZZQUCQILI5TMVZ6JNRKM423BSAISDM7ZFWQ6KWEBC4
//      KeyPair* source = KeyPair::fromSecretSeed(QString("SCH27VUZZ6UAKB67BDNF6FA42YMBMQCBKXWGMFD5TZ6S5ZZCZFLRXKHS"));
//      KeyPair* destination = KeyPair::fromAccountId(QString("GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR"));

//      Transaction::Builder::setDefaultOperationFee(2345);

//      Account* account = new Account(source, 2908908335136768L);
//      Transaction* transaction = Transaction::Builder(account)
//              .addOperation(new CreateAccountOperation(destination, "2000"))
//              .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
//              .build();

//      transaction->sign(source);

//      QCOMPARE(transaction->toEnvelopeXdrBase64(),QString("AAAAAF7FIiDToW1fOYUFBC0dmyufJbFTOa2GQESGz+S2h5ViAAAJKQAKVaMAAAABAAAAAAAAAAAAAAABAAAAAAAAAAAAAAAA7eBSYbzcL5UKo7oXO24y1ckX+XuCtkDsyNHOp1n1bxAAAAAEqBfIAAAAAAAAAAABtoeVYgAAAEDf8XAGz9uOmfL0KJBP29eSXz/CZqZtl0Mm8jHye3xwLgo2HJDfvCJdijGKsx34AfNl6hvX+Cq3IVk062sLSuoK"));

//      auto xdr = transaction->toEnvelopeXdr();
//      Transaction* transaction2 = dynamic_cast<Transaction*>(Transaction::fromEnvelopeXdr(xdr));

//      QCOMPARE(transaction->getSourceAccount(), transaction2->getSourceAccount());
//      QCOMPARE(transaction->getSequenceNumber(), transaction2->getSequenceNumber());
//      QCOMPARE(transaction->getFee(), transaction2->getFee());
//      QCOMPARE(
//              (static_cast<CreateAccountOperation*>(transaction->getOperations()[0]))->getStartingBalance(),
//              (static_cast<CreateAccountOperation*>(transaction2->getOperations()[0]))->getStartingBalance()
//      );


//      Transaction::Builder::setDefaultOperationFee(Transaction::Builder::BASE_FEE);//restart fee
//    }


//    void testDefaultBaseFeeThrows() {
//        try {
//            Transaction::Builder::setDefaultOperationFee(99);
//            QFAIL("expected IllegalArgumentException");
//        } catch (const std::runtime_error& e) {
//            // expected
//        }

//        // should succeed
//        Transaction::Builder::setDefaultOperationFee(100);
//    }
    void testMissingOperationFee() {
        Account* account = new Account(KeyPair::fromAccountId(QString("GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR")), 2908908335136768L);
        try {
            Transaction::Builder(AccountConverter().enableMuxed(), account, Network::current())
                    .addOperation(new CreateAccountOperation(KeyPair::fromAccountId(QString("GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR")), "2000"))
                    .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                    .build();
            QFAIL("missing exception");
        }
        catch (const std::runtime_error& e) {
            // expected
            Q_UNUSED(e)
        }
    }
    void testBuilderSuccessTestnet(){
        // GBPMKIRA2OQW2XZZQUCQILI5TMVZ6JNRKM423BSAISDM7ZFWQ6KWEBC4
        KeyPair *source = KeyPair::fromSecretSeed(QString("SCH27VUZZ6UAKB67BDNF6FA42YMBMQCBKXWGMFD5TZ6S5ZZCZFLRXKHS"));
        KeyPair *destination = KeyPair::fromAccountId(QString("GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR"));

        qint64 sequenceNumber = 2908908335136768;
        Account* account = new Account(source, sequenceNumber);
        Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), account)
                .addOperation(new CreateAccountOperation(destination, "2000"))
                .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                .setBaseFee(Transaction::Builder::BASE_FEE)
                .build();

        transaction->sign(source);
        //QString expected("AAAAAF7FIiDToW1fOYUFBC0dmyufJbFTOa2GQESGz+S2h5ViAAAAZAAKVaMAAAABAAAAAAAAAAAAAAABAAAAAAAAAAAAAAAA7eBSYbzcL5UKo7oXO24y1ckX+XuCtkDsyNHOp1n1bxAAAAAEqBfIAAAAAAAAAAABtoeVYgAAAEDLki9Oi700N60Lo8gUmEFHbKvYG4QSqXiLIt9T0ru2O5BphVl/jR9tYtHAD+UeDYhgXNgwUxqTEu1WukvEyYcD");
        // TIMEOUT_INFINITE is time bounds {0, 0}, as java and js write it: the
        // bytes py-stellar-sdk 14.1.0 signs with add_time_bounds(0, 0). Until
        // 1.0.4 it was PRECOND_NONE, another transaction with another hash.
        QString expected("AAAAAgAAAABexSIg06FtXzmFBQQtHZsrnyWxUzmthkBEhs/ktoeVYgAAAGQAClWjAAAAAQAAAAEAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEAAAAAAAAAAAAAAADt4FJhvNwvlQqjuhc7bjLVyRf5e4K2QOzI0c6nWfVvEAAAAASoF8gAAAAAAAAAAAG2h5ViAAAAQLJxvwao6eyNHaDX2QFhgdqlxJUqkpgA03UUOqf4DwOXSV9GN4ZWut2uzRuza4DWyVGBEHmmnQX+SQKFo0Sb/wA=");

        compareBase64(transaction->toEnvelopeXdrBase64(),expected);
        QCOMPARE(transaction->toEnvelopeXdrBase64(),expected);

        QVERIFY(transaction->getSourceAccount() == source->getAccountId());
        QVERIFY(transaction->getSequenceNumber()== sequenceNumber +1);
        QVERIFY(transaction->getFee()==100);
    }


    void testBuilderMemoText(){
        // GBPMKIRA2OQW2XZZQUCQILI5TMVZ6JNRKM423BSAISDM7ZFWQ6KWEBC4
        KeyPair *source = KeyPair::fromSecretSeed(QString("SCH27VUZZ6UAKB67BDNF6FA42YMBMQCBKXWGMFD5TZ6S5ZZCZFLRXKHS"));
        KeyPair *destination = KeyPair::fromAccountId(QString("GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR"));

        Account *account = new Account(source, 2908908335136768);
        Transaction *transaction = Transaction::Builder(AccountConverter().enableMuxed(), account)
                .addOperation(new CreateAccountOperation(destination, "2000"))
                .addMemo(Memo::text("Hello world!"))
                .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                .setBaseFee(Transaction::Builder::BASE_FEE)
                .build();

        transaction->sign(source);
        //QString expected("AAAAAF7FIiDToW1fOYUFBC0dmyufJbFTOa2GQESGz+S2h5ViAAAAZAAKVaMAAAABAAAAAAAAAAEAAAAMSGVsbG8gd29ybGQhAAAAAQAAAAAAAAAAAAAAAO3gUmG83C+VCqO6FztuMtXJF/l7grZA7MjRzqdZ9W8QAAAABKgXyAAAAAAAAAAAAbaHlWIAAABAxzofBhoayuUnz8t0T1UNWrTgmJ+lCh9KaeOGu2ppNOz9UGw0abGLhv+9oWQsstaHx6YjwWxL+8GBvwBUVWRlBQ==");
        // Time bounds {0, 0} for TIMEOUT_INFINITE (py-stellar-sdk's bytes).
        QString expected("AAAAAgAAAABexSIg06FtXzmFBQQtHZsrnyWxUzmthkBEhs/ktoeVYgAAAGQAClWjAAAAAQAAAAEAAAAAAAAAAAAAAAAAAAAAAAAAAQAAAAxIZWxsbyB3b3JsZCEAAAABAAAAAAAAAAAAAAAA7eBSYbzcL5UKo7oXO24y1ckX+XuCtkDsyNHOp1n1bxAAAAAEqBfIAAAAAAAAAAABtoeVYgAAAEA2U3MBHRAxe/nYUTFsL14hgWxcq1Z0yFcEd2LSEt+TbXbVXHl77k/sjLdUGw/0qMZMOn2n50MY+w1pnx6mjfsM");
        compareBase64(expected, transaction->toEnvelopeXdrBase64());
        QVERIFY(expected== transaction->toEnvelopeXdrBase64());
    }

    /**
     * A signature made elsewhere (a wallet, an HSM, a CAP-40 signed payload)
     * could not be attached: js has addSignature / addDecoratedSignature, py
     * envelope.signatures.append. addSignature(publicKey, base64) verifies it
     * against the hash, as js does; the decorated form is appended as given.
     */
    void testASignatureMadeElsewhereIsAttached()
    {
        const QString seed = QStringLiteral("SCH27VUZZ6UAKB67BDNF6FA42YMBMQCBKXWGMFD5TZ6S5ZZCZFLRXKHS");
        const QString dest = QStringLiteral("GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR");
        auto build = [&](qint64 seq) {
            Account* account = new Account(KeyPair::fromSecretSeed(seed), seq);
            Transaction* tx = Transaction::Builder(AccountConverter().enableMuxed(), account)
                    .addOperation(CreateAccountOperation::create(KeyPair::fromAccountId(dest), "2000"))
                    .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                    .setBaseFee(Transaction::Builder::BASE_FEE)
                    .build();
            delete account;
            return tx;
        };
        QScopedPointer<KeyPair> signer(KeyPair::fromSecretSeed(seed));
        QScopedPointer<Transaction> signedHere(build(2908908335136768L));
        signedHere->sign(signer.data());
        const stellar::DecoratedSignature made = signedHere->getSignatures().first();
        const QString base64 = QString::fromLatin1(
            QByteArray(reinterpret_cast<const char*>(made.signature.value.data()), made.signature.value.size()).toBase64());

        QScopedPointer<Transaction> attached(build(2908908335136768L));
        attached->addSignature(signer->getAccountId(), base64);
        QCOMPARE(attached->toEnvelopeXdrBase64(), signedHere->toEnvelopeXdrBase64());

        QScopedPointer<Transaction> other(build(2908908335136769L));
        QVERIFY_EXCEPTION_THROWN(other->addSignature(signer->getAccountId(), base64), std::runtime_error);
        QVERIFY_EXCEPTION_THROWN(other->addSignature(signer->getAccountId(), QStringLiteral("not base64!")), std::runtime_error);
        QVERIFY_EXCEPTION_THROWN(other->addSignature(dest, base64), std::runtime_error);
        QCOMPARE(other->getSignatures().size(), 0);

        const stellar::DecoratedSignature payload = signer->signPayloadDecorated(QByteArray("payload"));
        other->addSignature(payload);
        QCOMPARE(other->getSignatures().size(), 1);
        QCOMPARE(QByteArray(reinterpret_cast<const char*>(other->getSignatures().first().hint.signatureHint), 4),
                 QByteArray(reinterpret_cast<const char*>(payload.hint.signatureHint), 4));
    }

    void testBuilderTimeBounds()
    {
        // GBPMKIRA2OQW2XZZQUCQILI5TMVZ6JNRKM423BSAISDM7ZFWQ6KWEBC4
        KeyPair* source = KeyPair::fromSecretSeed(QString("SCH27VUZZ6UAKB67BDNF6FA42YMBMQCBKXWGMFD5TZ6S5ZZCZFLRXKHS"));
        KeyPair* destination = KeyPair::fromAccountId(QString("GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR"));

        Account* account = new Account(source, 2908908335136768L);
        Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), account)
                .addOperation(CreateAccountOperation::create(destination, "2000"))
                .addTimeBounds(new TimeBounds(42, 1337))
                .addMemo(Memo::hash(QString("abcdef0000000000000000000000000000000000000000000000000000000000")))
                .setBaseFee(Transaction::Builder::BASE_FEE)
                .build();

        transaction->sign(source);

        // Convert transaction to binary XDR and back again to make sure timebounds are correctly de/serialized.
        QByteArray data;
        QDataStream streamWrite(&data,QIODevice::WriteOnly);
        streamWrite << transaction->toV1Xdr();
        QDataStream streamRead(&data,QIODevice::ReadOnly);
        stellar::Transaction decodedTransaction;
        streamRead >> decodedTransaction;



        // CAP-21: tx.timeBounds was generalized to tx.cond. For a plain
        // time-bounded transaction the discriminant is PRECOND_TIME and the
        // TimeBounds lives in the `timeBounds` union member.
        QCOMPARE(static_cast<int>(decodedTransaction.cond.type),
                 static_cast<int>(stellar::PreconditionType::PRECOND_TIME));
        QCOMPARE(decodedTransaction.cond.timeBounds.minTime, 42UL);
        QCOMPARE(decodedTransaction.cond.timeBounds.maxTime, 1337UL);
        auto xdrEnvelope = transaction->toEnvelopeXdr();
        Transaction* transaction2 = dynamic_cast<Transaction*>(Transaction::fromEnvelopeXdr(xdrEnvelope));
        //qDebug() << "HASH "<<dynamic_cast<MemoHash*>(transaction->getMemo())->getHexValue();
        //qDebug() << "HASH "<<dynamic_cast<MemoHash*>(transaction2->getMemo())->getHexValue();
        QCOMPARE(transaction->getSourceAccount(), transaction2->getSourceAccount());
        QCOMPARE(transaction->getSequenceNumber(), transaction2->getSequenceNumber());
        QVERIFY(transaction->getMemo()->equals(transaction2->getMemo()));
        QVERIFY(transaction->getTimeBounds()->equals(transaction2->getTimeBounds()));
        QCOMPARE(transaction->getFee(), transaction2->getFee());
        QCOMPARE(
                (static_cast<CreateAccountOperation*>(transaction->getOperations()[0]))->getStartingBalance(),
                (static_cast<CreateAccountOperation*>(transaction2->getOperations()[0]))->getStartingBalance()
                );
    }



    void testBuilderWithTimeBoundsButNoTimeout(){
        Account* account = new Account(KeyPair::random(), 2908908335136768L);
        try {
            Transaction::Builder(AccountConverter().enableMuxed(), account)
                    .addOperation(CreateAccountOperation::create(KeyPair::random(), "2000"))
                    .addTimeBounds(new TimeBounds(42, 1337))
                    .addMemo(Memo::hash(QString("abcdef0000000000000000000000000000000000000000000000000000000000")))
                    .setBaseFee(Transaction::Builder::BASE_FEE)
                    .build();
        } catch (const std::runtime_error& exception) {
            // Should not throw as max_time is set
            QFAIL("Should not throw");
            Q_UNUSED(exception)
        }
    }

    void testBuilderRequiresTimeoutOrTimeBounds(){
        Account* account = new Account(KeyPair::random(), 2908908335136768L);
        try {
            Transaction::Builder(AccountConverter().enableMuxed(), account)
                    .addOperation(CreateAccountOperation::create(KeyPair::random(), "2000"))
                    .addMemo(Memo::hash(QString("abcdef0000000000000000000000000000000000000000000000000000000000")))
                    .setBaseFee(Transaction::Builder::BASE_FEE)
                    .build();
            QFAIL("missing exception");
        } catch (const std::runtime_error& exception) {
            QVERIFY(QString(exception.what()).compare("TimeBounds has to be set or you must call setTimeout(TIMEOUT_INFINITE).")==0);
        }
    }

    void testBuilderTimeoutNegative(){
        Account* account = new Account(KeyPair::random(), 2908908335136768L);
        try {
            Transaction::Builder(AccountConverter().enableMuxed(), account)
                    .addOperation(CreateAccountOperation::create(KeyPair::random(), "2000"))
                    .addMemo(Memo::hash(QString("abcdef0000000000000000000000000000000000000000000000000000000000")))
                    .setTimeout(-1)
                    .setBaseFee(Transaction::Builder::BASE_FEE)
                    .build();
            QFAIL("missing exception");
        } catch (const std::runtime_error& exception) {
            QVERIFY(QString(exception.what()).contains("timeout cannot be negative"));
            QCOMPARE(account->getSequenceNumber(),2908908335136768L);
        }
    }

    void testBuilderTimeoutSetsTimeBounds() {
        Account* account = new Account(KeyPair::random(), 2908908335136768L);
        Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), account)
                .addOperation(CreateAccountOperation::create(KeyPair::random(), "2000"))
                .setTimeout(10)
                .setBaseFee(Transaction::Builder::BASE_FEE)
                .build();

        QVERIFY(transaction->getTimeBounds());
        QCOMPARE(transaction->getTimeBounds()->getMinTime(),0);
        qint64 currentUnix = QDateTime::currentMSecsSinceEpoch() / 1000L;
        QCOMPARE(transaction->getTimeBounds()->getMaxTime(),currentUnix + 10);
    }

    void testBuilderFailsWhenSettingTimeoutAndMaxTimeAlreadySet() {
        Account* account = new Account(KeyPair::random(), 2908908335136768L);
        try {
            Transaction::Builder(AccountConverter().enableMuxed(), account)
                    .addOperation(CreateAccountOperation::create(KeyPair::random(), "2000"))
                    .addTimeBounds(new TimeBounds(42, 1337))
                    .setTimeout(10)
                    .setBaseFee(Transaction::Builder::BASE_FEE)
                    .build();
            QFAIL("missing exception");
        } catch (const std::runtime_error& exception) {
            QVERIFY(QString(exception.what()).contains("TimeBounds.max_time has been already set"));
            QCOMPARE(account->getSequenceNumber(),2908908335136768L);
        }
    }

    void testBuilderFailsWhenSettingTimeoutAndMaxTimeNotSet() {
        Account* account = new Account(KeyPair::random(), 2908908335136768L);
        Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), account)
                .addOperation(CreateAccountOperation::create(KeyPair::random(), "2000"))
                .addTimeBounds(new TimeBounds(42, 0))
                .setTimeout(10)
                .setBaseFee(Transaction::Builder::BASE_FEE)
                .build();

        QCOMPARE(transaction->getTimeBounds()->getMinTime(),42);
        // Should add max_time
        qint64 currentUnix = QDateTime::currentMSecsSinceEpoch() / 1000L;
        QCOMPARE(transaction->getTimeBounds()->getMaxTime(),currentUnix + 10);
    }


    void testBuilderTimeBoundsNoMaxTime()
    {
      // GBPMKIRA2OQW2XZZQUCQILI5TMVZ6JNRKM423BSAISDM7ZFWQ6KWEBC4
      KeyPair* source = KeyPair::fromSecretSeed(QString("SCH27VUZZ6UAKB67BDNF6FA42YMBMQCBKXWGMFD5TZ6S5ZZCZFLRXKHS"));
      KeyPair* destination = KeyPair::fromAccountId(QString("GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR"));
      Account* account = new Account(source, 2908908335136768L);
      Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), account)
              .addOperation(CreateAccountOperation::create(destination, "2000"))
              .addTimeBounds(new TimeBounds(42, 0))
              .addMemo(Memo::hash(QString("abcdef0000000000000000000000000000000000000000000000000000000000")))
              .setBaseFee(Transaction::Builder::BASE_FEE)
              .build();
      transaction->sign(source);
      // Convert transaction to binary XDR and back again to make sure timebounds are correctly de/serialized.

      QByteArray data;
      QDataStream streamWrite(&data,QIODevice::WriteOnly);      
      streamWrite << transaction->toV1Xdr();
      QDataStream streamRead(&data,QIODevice::ReadOnly);
      stellar::Transaction decodedTransaction;
      streamRead >> decodedTransaction;
      QCOMPARE(static_cast<int>(decodedTransaction.cond.type),
               static_cast<int>(stellar::PreconditionType::PRECOND_TIME));
      QCOMPARE(decodedTransaction.cond.timeBounds.minTime, 42UL);
      QCOMPARE(decodedTransaction.cond.timeBounds.maxTime, 0UL);
    }

    void testBuilderSuccessPublic() {
        Network::usePublicNetwork();

        // GBPMKIRA2OQW2XZZQUCQILI5TMVZ6JNRKM423BSAISDM7ZFWQ6KWEBC4
        KeyPair *source = KeyPair::fromSecretSeed(QString("SCH27VUZZ6UAKB67BDNF6FA42YMBMQCBKXWGMFD5TZ6S5ZZCZFLRXKHS"));
        KeyPair *destination = KeyPair::fromAccountId(QString("GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR"));

        Account *account = new Account(source, 2908908335136768);
        Transaction *transaction = Transaction::Builder(AccountConverter().enableMuxed(), account)
                .addOperation(new CreateAccountOperation(destination, "2000"))
                .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                .setBaseFee(Transaction::Builder::BASE_FEE)
                .build();

        transaction->sign(source);
        //QString expected("AAAAAF7FIiDToW1fOYUFBC0dmyufJbFTOa2GQESGz+S2h5ViAAAAZAAKVaMAAAABAAAAAAAAAAAAAAABAAAAAAAAAAAAAAAA7eBSYbzcL5UKo7oXO24y1ckX+XuCtkDsyNHOp1n1bxAAAAAEqBfIAAAAAAAAAAABtoeVYgAAAEDzfR5PgRFim5Wdvq9ImdZNWGBxBWwYkQPa9l5iiBdtPLzAZv6qj+iOfSrqinsoF0XrLkwdIcZQVtp3VRHhRoUE");
        // Time bounds {0, 0} for TIMEOUT_INFINITE (py-stellar-sdk's bytes).
        QString expected("AAAAAgAAAABexSIg06FtXzmFBQQtHZsrnyWxUzmthkBEhs/ktoeVYgAAAGQAClWjAAAAAQAAAAEAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEAAAAAAAAAAAAAAADt4FJhvNwvlQqjuhc7bjLVyRf5e4K2QOzI0c6nWfVvEAAAAASoF8gAAAAAAAAAAAG2h5ViAAAAQOFMVS3lQ1gtqY0a3VPsZXCNVGC/h8dANFiUS7hYPWSbyzi4Ob1ir5R256mOwX+B6vE552+y8JAFaAFPR0bDyAw=");
        QVERIFY(expected==transaction->toEnvelopeXdrBase64());
    }

    void testParseV0Transaction()  {

        // GBPMKIRA2OQW2XZZQUCQILI5TMVZ6JNRKM423BSAISDM7ZFWQ6KWEBC4
        KeyPair* source = KeyPair::fromSecretSeed(QString("SCH27VUZZ6UAKB67BDNF6FA42YMBMQCBKXWGMFD5TZ6S5ZZCZFLRXKHS"));
        KeyPair* destination = KeyPair::fromAccountId(QString("GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR"));

        Account* account = new Account(source, 2908908335136768L);
        Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), account, Network::current())
                .addOperation(new CreateAccountOperation(destination->getAccountId(), "2000"))
                .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                .setBaseFee(Transaction::MIN_BASE_FEE)
                .build();
        transaction->setEnvelopeType(stellar::EnvelopeType::ENVELOPE_TYPE_TX_V0);
        transaction->sign(source);


        QByteArray data;
        QDataStream streamWrite(&data,QIODevice::WriteOnly);
        streamWrite << transaction->toEnvelopeXdr();
        QDataStream streamRead(&data,QIODevice::ReadOnly);

        stellar::TransactionEnvelope decodedTransaction;

        streamRead >> decodedTransaction;
        QCOMPARE(decodedTransaction.type, stellar::EnvelopeType::ENVELOPE_TYPE_TX_V0);

        Transaction* parsed = (Transaction*) Transaction::fromEnvelopeXdr(transaction->toEnvelopeXdrBase64(), Network::current());
        QCOMPARE(parsed->hash(),transaction->hash());
        QCOMPARE(parsed->toEnvelopeXdr().type,stellar::EnvelopeType::ENVELOPE_TYPE_TX_V0);

        // Time bounds {0, 0} for TIMEOUT_INFINITE (py-stellar-sdk's bytes).
        QCOMPARE(transaction->toEnvelopeXdrBase64()
                 ,QString("AAAAAF7FIiDToW1fOYUFBC0dmyufJbFTOa2GQESGz+S2h5ViAAAAZAAKVaMAAAABAAAAAQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAQAAAAAAAAAAAAAAAO3gUmG83C+VCqO6FztuMtXJF/l7grZA7MjRzqdZ9W8QAAAABKgXyAAAAAAAAAAAAbaHlWIAAABA4UxVLeVDWC2pjRrdU+xlcI1UYL+Hx0A0WJRLuFg9ZJvLOLg5vWKvlHbnqY7Bf4Hq8Tnnb7LwkAVoAU9HRsPIDA=="));
        QCOMPARE(transaction->toEnvelopeXdrBase64(), parsed->toEnvelopeXdrBase64());
    }
    void testSha256HashSigning() {
        Network::usePublicNetwork();

        KeyPair *source = KeyPair::fromAccountId("GBBM6BKZPEHWYO3E3YKREDPQXMS4VK35YLNU7NFBRI26RAN7GI5POFBB");
        KeyPair *destination = KeyPair::fromAccountId("GDJJRRMBK4IWLEPJGIE6SXD2LP7REGZODU7WDC3I2D6MR37F4XSHBKX2");

        Account *account = new Account(source, 0);
        Transaction *transaction = Transaction::Builder(AccountConverter().enableMuxed(), account)
                .addOperation(new PaymentOperation(destination->getAccountId(), new AssetTypeNative(), "2000"))
                .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                .setBaseFee(Transaction::Builder::BASE_FEE)
                .build();
        quint8 preimage[64];
        for(int i=0;i<64;i++){
            preimage[i] = static_cast<quint8>(rand()%256);
        }
        QByteArray hash = Util::hash(QByteArray(reinterpret_cast<char*>(preimage),64));

        transaction->sign(QByteArray(reinterpret_cast<char*>(preimage),64));

        //qDebug() <<QByteArray(reinterpret_cast<char*>(preimage),64).toHex();
        //qDebug() << transaction->getSignatures()[0].signature.binary().toHex();
        QVERIFY(memcmp(preimage, transaction->getSignatures()[0].signature.binary().data(),sizeof(preimage))==0);
        QByteArray hint = hash.right(4);
        QVERIFY(memcmp(hint.data(),transaction->getSignatures()[0].hint.signatureHint,4)==0);



    }

    void testToBase64EnvelopeXdrBuilderNoSignatures() {
      // GBPMKIRA2OQW2XZZQUCQILI5TMVZ6JNRKM423BSAISDM7ZFWQ6KWEBC4
      KeyPair *source = KeyPair::fromSecretSeed(QString("SCH27VUZZ6UAKB67BDNF6FA42YMBMQCBKXWGMFD5TZ6S5ZZCZFLRXKHS"));
      KeyPair *destination = KeyPair::fromAccountId(QString("GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR"));

      Account *account = new Account(source, 2908908335136768L);
      Transaction *transaction = Transaction::Builder(AccountConverter().enableMuxed(), account)
              .addOperation(new CreateAccountOperation(destination, "2000"))
              .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
              .setBaseFee(Transaction::Builder::BASE_FEE)
              .build();

      QCOMPARE(transaction->toEnvelopeXdrBase64()
               //,QString("AAAAAF7FIiDToW1fOYUFBC0dmyufJbFTOa2GQESGz+S2h5ViAAAAZAAKVaMAAAABAAAAAAAAAAAAAAABAAAAAAAAAAAAAAAA7eBSYbzcL5UKo7oXO24y1ckX+XuCtkDsyNHOp1n1bxAAAAAEqBfIAAAAAAAAAAAA"));
               // Time bounds {0, 0} for TIMEOUT_INFINITE (py-stellar-sdk's bytes).
               ,QString("AAAAAgAAAABexSIg06FtXzmFBQQtHZsrnyWxUzmthkBEhs/ktoeVYgAAAGQAClWjAAAAAQAAAAEAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEAAAAAAAAAAAAAAADt4FJhvNwvlQqjuhc7bjLVyRf5e4K2QOzI0c6nWfVvEAAAAASoF8gAAAAAAAAAAAA="));

    }

    void testNoOperations() {
      // GBPMKIRA2OQW2XZZQUCQILI5TMVZ6JNRKM423BSAISDM7ZFWQ6KWEBC4
      KeyPair *source = KeyPair::fromSecretSeed(QString("SCH27VUZZ6UAKB67BDNF6FA42YMBMQCBKXWGMFD5TZ6S5ZZCZFLRXKHS"));

      Account *account = new Account(source, 2908908335136768);
      try {
        Transaction *transaction = Transaction::Builder(AccountConverter().enableMuxed(), account).setTimeout(Transaction::Builder::TIMEOUT_INFINITE).setBaseFee(Transaction::Builder::BASE_FEE).build();
        Q_UNUSED(transaction);
        QFAIL("missing exception");
      } catch (const std::runtime_error& exception) {
        QVERIFY(QString(exception.what()).contains("At least one operation required"));
        QVERIFY(2908908335136768== account->getSequenceNumber());
      }

    }

    void testTryingToAddMemoTwice() {
      // GBPMKIRA2OQW2XZZQUCQILI5TMVZ6JNRKM423BSAISDM7ZFWQ6KWEBC4
      KeyPair *source = KeyPair::fromSecretSeed(QString("SCH27VUZZ6UAKB67BDNF6FA42YMBMQCBKXWGMFD5TZ6S5ZZCZFLRXKHS"));
      KeyPair *destination = KeyPair::fromAccountId(QString("GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR"));

      try {
        Account *account = new Account(source, 2908908335136768L);
        Transaction::Builder(AccountConverter().enableMuxed(), account)
                .addOperation(new CreateAccountOperation(destination, "2000"))
                .addMemo(Memo::none())
                .addMemo(Memo::none());
        QFAIL("missing exception");
      } catch (const std::runtime_error& exception) {
        QVERIFY(QString(exception.what()).contains("Memo has been already added."));
      }
  }

    void testDeserializeXdr() {
      // GBPMKIRA2OQW2XZZQUCQILI5TMVZ6JNRKM423BSAISDM7ZFWQ6KWEBC4

        KeyPair *source = KeyPair::fromSecretSeed(QString("SCH27VUZZ6UAKB67BDNF6FA42YMBMQCBKXWGMFD5TZ6S5ZZCZFLRXKHS"));
        KeyPair *destination = KeyPair::fromAccountId(QString("GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR"));
        Account *account = new Account(source, 2908908335136768);
        Transaction *transaction = Transaction::Builder(AccountConverter().enableMuxed(), account)
                .addOperation(new CreateAccountOperation(destination, "2000"))
                .addMemo(Memo::text("Hello world!"))
                .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                .setBaseFee(Transaction::Builder::BASE_FEE)
                .build();

        transaction->sign(source);
        stellar::TransactionEnvelope envelope= transaction->toEnvelopeXdr();

        Transaction *deserialized = dynamic_cast<Transaction*>(Transaction::fromEnvelopeXdr(envelope));
        QVERIFY(deserialized->getFee()==transaction->getFee());
        QVERIFY(deserialized->getSequenceNumber()==transaction->getSequenceNumber());
        QVERIFY(dynamic_cast<MemoText*>(deserialized->getMemo())->getText() == dynamic_cast<MemoText*>(transaction->getMemo())->getText());
        QVERIFY(deserialized->getSourceAccount()== transaction->getSourceAccount());

    }

    /**
     * js-stellar-sdk 17.1.0: revokeTrustlineSponsorship({account, asset: new
     * LiquidityPoolId(id), source}), signed. A pool-share trustline's key
     * holds a TrustLineAsset POOL_SHARE, which the key's plain Asset could not
     * hold, so the envelope was refused. It now reads, keeps its hash, writes
     * back byte for byte, and says what it is.
     */
    void testAPoolShareTrustlineRevokeReadsAsJsWroteIt()
    {
        const QString envelope = QStringLiteral(
            "AAAAAgAAAADqSmxj4pxSCr71UHsTLsX5lUd2rr6+e5JCHuppFEbSLAAAAGQAAAAAAAAAKgAAAAEAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAEAAAABAAAAAGa+fjMsekUzMr2dCn99sFX1xe8aBq2mbZizn7aBDEc6AAAAEgAAAAAAAAABAAAAAP0XJDhaoMdb"
            "ZPt4zWAvodmR/ev3axPFjtcC6sg16fYYAAAAA917GrgxwnMxDdvsb5eHCqg8L714ziKt7Tfsv08zgPrHAAAAAAAAAAEURtIs"
            "AAAAQNbKe902TWv6qfc0JaX577RoaX4hq/nZO+P5L4rai4I1B5Su8tzIn79hUJug/v3WlGVoDMuRe1h8z0zVyuM0eAY=");
        QScopedPointer<AbstractTransaction> read(
            AbstractTransaction::fromEnvelopeXdr(envelope, Network::testnetNetwork()));
        QCOMPARE(read->toEnvelopeXdrBase64(), envelope);
        QCOMPARE(read->hashHex(), QStringLiteral("a35528ea2b2d563ab0b5d6d49bfebb0a964ad2538ff2eb5cd2f9fd9f447ba97c"));
        auto* tx = dynamic_cast<Transaction*>(read.data());
        QVERIFY(tx);
        QCOMPARE(tx->getOperations().size(), 1);
        auto* revoke = dynamic_cast<RevokeTrustlineSponsorshipOperation*>(tx->getOperations().at(0));
        QVERIFY(revoke);
        QVERIFY(revoke->isLiquidityPoolShare());
        QVERIFY(revoke->getAsset() == nullptr);          // pool shares are no asset
        QCOMPARE(revoke->getLiquidityPoolID(),
                 QByteArray::fromHex("dd7b1ab831c273310ddbec6f97870aa83c2fbd78ce22aded37ecbf4f3380fac7"));
        QCOMPARE(revoke->getAccountId(), QStringLiteral("GD6ROJBYLKQMOW3E7N4M2YBPUHMZD7PL65VRHRMO24BOVSBV5H3BQRSL"));
        QCOMPARE(revoke->getSourceAccount(), QStringLiteral("GBTL47RTFR5EKMZSXWOQU735WBK7LRPPDIDK3JTNTCZZ7NUBBRDTVSK2"));
    }

    /**
     * The same operation built here is js's bytes, and so is the whole signed
     * envelope. There was no way to build it: the trustline key could only
     * hold a plain Asset. A pool id that is not 32 bytes is refused.
     */
    void testAPoolShareTrustlineRevokeBuiltHereIsJsBytes()
    {
        const QByteArray poolId =
            QByteArray::fromHex("dd7b1ab831c273310ddbec6f97870aa83c2fbd78ce22aded37ecbf4f3380fac7");
        const QString trustor = QStringLiteral("GD6ROJBYLKQMOW3E7N4M2YBPUHMZD7PL65VRHRMO24BOVSBV5H3BQRSL");
        const QString opSource = QStringLiteral("GBTL47RTFR5EKMZSXWOQU735WBK7LRPPDIDK3JTNTCZZ7NUBBRDTVSK2");
        QScopedPointer<RevokeTrustlineSponsorshipOperation> alone(
            RevokeTrustlineSponsorshipOperation::create(trustor, poolId)->setSourceAccount(opSource));
        QCOMPARE(alone->toXdrBase64(), QStringLiteral(
            "AAAAAQAAAABmvn4zLHpFMzK9nQp/fbBV9cXvGgatpm2Ys5+2gQxHOgAAABIAAAAAAAAAAQAAAAD9FyQ4WqDHW2T7eM1gL6HZ"
            "kf3r92sTxY7XAurINen2GAAAAAPdexq4McJzMQ3b7G+XhwqoPC+9eM4ire037L9PM4D6xw=="));
        QVERIFY(alone->isLiquidityPoolShare());
        QVERIFY(alone->getAsset() == nullptr);
        QCOMPARE(alone->getLiquidityPoolID(), poolId);

        QScopedPointer<KeyPair> signer(KeyPair::fromSecretSeed(QByteArray(32, '\x07')));
        Account account(signer->getAccountId(), 41);   // the account owns its own key pair
        QScopedPointer<Transaction> tx(TransactionBuilder(&account, Network::testnetNetwork())
            .addOperation(RevokeTrustlineSponsorshipOperation::create(trustor, poolId)->setSourceAccount(opSource))
            .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
            .setBaseFee(100)
            .build());
        tx->sign(signer.data());
        QCOMPARE(tx->toEnvelopeXdrBase64(), QStringLiteral(
            "AAAAAgAAAADqSmxj4pxSCr71UHsTLsX5lUd2rr6+e5JCHuppFEbSLAAAAGQAAAAAAAAAKgAAAAEAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAEAAAABAAAAAGa+fjMsekUzMr2dCn99sFX1xe8aBq2mbZizn7aBDEc6AAAAEgAAAAAAAAABAAAAAP0XJDhaoMdb"
            "ZPt4zWAvodmR/ev3axPFjtcC6sg16fYYAAAAA917GrgxwnMxDdvsb5eHCqg8L714ziKt7Tfsv08zgPrHAAAAAAAAAAEURtIs"
            "AAAAQNbKe902TWv6qfc0JaX577RoaX4hq/nZO+P5L4rai4I1B5Su8tzIn79hUJug/v3WlGVoDMuRe1h8z0zVyuM0eAY="));

        for (const QByteArray& wrong : { poolId.left(31), poolId + QByteArray(1, '\0'), QByteArray() }) {
            bool threw = false;
            try { delete RevokeTrustlineSponsorshipOperation::create(trustor, wrong); }
            catch (const std::runtime_error&) { threw = true; }
            QVERIFY(threw);
        }
        // A credit trustline is still an asset and no pool.
        QScopedPointer<Asset> usd(Asset::createNonNativeAsset(QStringLiteral("USD"), opSource));
        QScopedPointer<RevokeTrustlineSponsorshipOperation> credit(
            RevokeTrustlineSponsorshipOperation::create(trustor, usd.data()));
        QVERIFY(!credit->isLiquidityPoolShare());
        QVERIFY(credit->getLiquidityPoolID().isEmpty());
        QVERIFY(credit->getAsset() && credit->getAsset()->equals(usd.data()));
    }

    /**
     * A transaction holding a value XDR has no encoding for (here a payment
     * whose asset names POOL_SHARE, which is no arm of a plain Asset) is
     * neither hashed, signed nor emitted. The writer failed half way, nobody
     * looked at the stream, and the half that was written was hashed, signed
     * and base64'd as if it were the transaction.
     */
    void testATransactionThatCannotBeEncodedIsNeitherSignedNorEmitted()
    {
        QScopedPointer<KeyPair> signer(KeyPair::fromSecretSeed(QByteArray(32, '\x07')));
        stellar::PaymentOp op;
        op.destination.type = stellar::CryptoKeyType::KEY_TYPE_ED25519;
        std::memset(op.destination.ed25519, 0x11, sizeof op.destination.ed25519);
        op.asset.type = stellar::AssetType::ASSET_TYPE_POOL_SHARE;
        op.amount = 1;
        Account account(signer->getAccountId(), 41);   // the account owns its own key pair
        QScopedPointer<Transaction> tx(TransactionBuilder(&account, Network::testnetNetwork())
            .addOperation(PaymentOperation::build(AccountConverter(), op))
            .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
            .setBaseFee(100)
            .build());
        auto throws = [](auto&& call) {
            try { call(); } catch (const std::runtime_error&) { return true; }
            return false;
        };
        QVERIFY(throws([&] { tx->signatureBase(); }));
        QVERIFY(throws([&] { tx->hash(); }));
        QVERIFY(throws([&] { tx->sign(signer.data()); }));
        QVERIFY(tx->getSignatures().isEmpty());
        QVERIFY(throws([&] { tx->toEnvelopeXdr(); }));
        QVERIFY(throws([&] { tx->toEnvelopeXdrBase64(); }));
        QVERIFY(throws([&] { tx->getOperations().at(0)->toXdrBase64(); }));
    }
    /** A transaction built with muxing off signed the hash of its muxed form
     *  (the M... destination kept) while its envelope carries the G...
     *  account: the network hashes the envelope, so the signature failed. */
    void testTheSignedHashIsTheHashOfTheEnvelopeSent()
    {
        Account account(KeyPair::random(), 41);
        QScopedPointer<Transaction> tx(
            TransactionBuilder(AccountConverter().disableMuxed(), &account, Network::testnetNetwork())
                .addOperation(new PaymentOperation(
                    QStringLiteral("MA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVAAAAAAAAAAAAAJLK"),
                    new AssetTypeNative(), QStringLiteral("1")))
                .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build());
        QScopedPointer<AbstractTransaction> sent(
            AbstractTransaction::fromEnvelopeXdr(tx->toEnvelopeXdrBase64(), Network::testnetNetwork()));
        QCOMPARE(tx->hash(), sent->hash());
    }

};

ADD_TEST(TransactionTest)

#endif // TRANSACTIONTEST_H
