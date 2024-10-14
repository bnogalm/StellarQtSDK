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
        QString expected("AAAAAgAAAABexSIg06FtXzmFBQQtHZsrnyWxUzmthkBEhs/ktoeVYgAAAGQAClWjAAAAAQAAAAAAAAAAAAAAAQAAAAAAAAAAAAAAAO3gUmG83C+VCqO6FztuMtXJF/l7grZA7MjRzqdZ9W8QAAAABKgXyAAAAAAAAAAAAbaHlWIAAABAy5IvTou9NDetC6PIFJhBR2yr2BuEEql4iyLfU9K7tjuQaYVZf40fbWLRwA/lHg2IYFzYMFMakxLtVrpLxMmHAw==");

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
        QString expected("AAAAAgAAAABexSIg06FtXzmFBQQtHZsrnyWxUzmthkBEhs/ktoeVYgAAAGQAClWjAAAAAQAAAAAAAAABAAAADEhlbGxvIHdvcmxkIQAAAAEAAAAAAAAAAAAAAADt4FJhvNwvlQqjuhc7bjLVyRf5e4K2QOzI0c6nWfVvEAAAAASoF8gAAAAAAAAAAAG2h5ViAAAAQMc6HwYaGsrlJ8/LdE9VDVq04JifpQofSmnjhrtqaTTs/VBsNGmxi4b/vaFkLLLWh8emI8FsS/vBgb8AVFVkZQU=");
        compareBase64(expected, transaction->toEnvelopeXdrBase64());
        QVERIFY(expected== transaction->toEnvelopeXdrBase64());
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
                .addMemo(Memo::hash(QString("abcdef")))
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



        QCOMPARE(decodedTransaction.timeBounds.value.minTime, 42UL);
        QCOMPARE(decodedTransaction.timeBounds.value.maxTime, 1337UL);
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
                    .addMemo(Memo::hash(QString("abcdef")))
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
                    .addMemo(Memo::hash(QString("abcdef")))
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
                    .addMemo(Memo::hash(QString("abcdef")))
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
              .addMemo(Memo::hash(QString("abcdef")))
              .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
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
      QCOMPARE(decodedTransaction.timeBounds.value.minTime, 42UL);
      QCOMPARE(decodedTransaction.timeBounds.value.maxTime, 0UL);
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
        QString expected("AAAAAgAAAABexSIg06FtXzmFBQQtHZsrnyWxUzmthkBEhs/ktoeVYgAAAGQAClWjAAAAAQAAAAAAAAAAAAAAAQAAAAAAAAAAAAAAAO3gUmG83C+VCqO6FztuMtXJF/l7grZA7MjRzqdZ9W8QAAAABKgXyAAAAAAAAAAAAbaHlWIAAABA830eT4ERYpuVnb6vSJnWTVhgcQVsGJED2vZeYogXbTy8wGb+qo/ojn0q6op7KBdF6y5MHSHGUFbad1UR4UaFBA==");
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

        QCOMPARE(transaction->toEnvelopeXdrBase64()
                 ,QString("AAAAAF7FIiDToW1fOYUFBC0dmyufJbFTOa2GQESGz+S2h5ViAAAAZAAKVaMAAAABAAAAAAAAAAAAAAABAAAAAAAAAAAAAAAA7eBSYbzcL5UKo7oXO24y1ckX+XuCtkDsyNHOp1n1bxAAAAAEqBfIAAAAAAAAAAABtoeVYgAAAEDzfR5PgRFim5Wdvq9ImdZNWGBxBWwYkQPa9l5iiBdtPLzAZv6qj+iOfSrqinsoF0XrLkwdIcZQVtp3VRHhRoUE"));
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
               ,QString("AAAAAgAAAABexSIg06FtXzmFBQQtHZsrnyWxUzmthkBEhs/ktoeVYgAAAGQAClWjAAAAAQAAAAAAAAAAAAAAAQAAAAAAAAAAAAAAAO3gUmG83C+VCqO6FztuMtXJF/l7grZA7MjRzqdZ9W8QAAAABKgXyAAAAAAAAAAAAA=="));

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
};

ADD_TEST(TransactionTest)

#endif // TRANSACTIONTEST_H
