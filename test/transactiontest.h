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

    void testBuilderSuccessTestnet(){
        // GBPMKIRA2OQW2XZZQUCQILI5TMVZ6JNRKM423BSAISDM7ZFWQ6KWEBC4
        KeyPair *source = KeyPair::fromSecretSeed(QString("SCH27VUZZ6UAKB67BDNF6FA42YMBMQCBKXWGMFD5TZ6S5ZZCZFLRXKHS"));
        KeyPair *destination = KeyPair::fromAccountId(QString("GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR"));

        quint64 sequenceNumber = 2908908335136768;
        Account* account = new Account(source, sequenceNumber);
        Transaction* transaction = Transaction::Builder(account)
                .addOperation(new CreateAccountOperation(destination, "2000"))
                .build();

        transaction->sign(source);
        QString expected("AAAAAF7FIiDToW1fOYUFBC0dmyufJbFTOa2GQESGz+S2h5ViAAAAZAAKVaMAAAABAAAAAAAAAAAAAAABAAAAAAAAAAAAAAAA7eBSYbzcL5UKo7oXO24y1ckX+XuCtkDsyNHOp1n1bxAAAAAEqBfIAAAAAAAAAAABtoeVYgAAAEDLki9Oi700N60Lo8gUmEFHbKvYG4QSqXiLIt9T0ru2O5BphVl/jR9tYtHAD+UeDYhgXNgwUxqTEu1WukvEyYcD");
        //compareBase64(expected,transaction->toEnvelopeXdrBase64());
        QVERIFY(expected
                ==transaction->toEnvelopeXdrBase64());

        QVERIFY(transaction->getSourceAccount()==source);
        QVERIFY(transaction->getSequenceNumber()== sequenceNumber +1);
        QVERIFY(transaction->getFee()==100);
    }


    void testBuilderMemoText(){
        // GBPMKIRA2OQW2XZZQUCQILI5TMVZ6JNRKM423BSAISDM7ZFWQ6KWEBC4
        KeyPair *source = KeyPair::fromSecretSeed(QString("SCH27VUZZ6UAKB67BDNF6FA42YMBMQCBKXWGMFD5TZ6S5ZZCZFLRXKHS"));
        KeyPair *destination = KeyPair::fromAccountId(QString("GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR"));

        Account *account = new Account(source, 2908908335136768);
        Transaction *transaction = Transaction::Builder(account)
                .addOperation(new CreateAccountOperation(destination, "2000"))
                .addMemo(Memo::text("Hello world!"))
                .build();

        transaction->sign(source);
        QString expected("AAAAAF7FIiDToW1fOYUFBC0dmyufJbFTOa2GQESGz+S2h5ViAAAAZAAKVaMAAAABAAAAAAAAAAEAAAAMSGVsbG8gd29ybGQhAAAAAQAAAAAAAAAAAAAAAO3gUmG83C+VCqO6FztuMtXJF/l7grZA7MjRzqdZ9W8QAAAABKgXyAAAAAAAAAAAAbaHlWIAAABAxzofBhoayuUnz8t0T1UNWrTgmJ+lCh9KaeOGu2ppNOz9UGw0abGLhv+9oWQsstaHx6YjwWxL+8GBvwBUVWRlBQ==");
        compareBase64(expected, transaction->toEnvelopeXdrBase64());
        QVERIFY(expected== transaction->toEnvelopeXdrBase64());
    }


    void testBuilderSuccessPublic() {
        Network::usePublicNetwork();

        // GBPMKIRA2OQW2XZZQUCQILI5TMVZ6JNRKM423BSAISDM7ZFWQ6KWEBC4
        KeyPair *source = KeyPair::fromSecretSeed(QString("SCH27VUZZ6UAKB67BDNF6FA42YMBMQCBKXWGMFD5TZ6S5ZZCZFLRXKHS"));
        KeyPair *destination = KeyPair::fromAccountId(QString("GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR"));

        Account *account = new Account(source, 2908908335136768);
        Transaction *transaction = Transaction::Builder(account)
                .addOperation(new CreateAccountOperation(destination, "2000"))
                .build();

        transaction->sign(source);
        QString expected("AAAAAF7FIiDToW1fOYUFBC0dmyufJbFTOa2GQESGz+S2h5ViAAAAZAAKVaMAAAABAAAAAAAAAAAAAAABAAAAAAAAAAAAAAAA7eBSYbzcL5UKo7oXO24y1ckX+XuCtkDsyNHOp1n1bxAAAAAEqBfIAAAAAAAAAAABtoeVYgAAAEDzfR5PgRFim5Wdvq9ImdZNWGBxBWwYkQPa9l5iiBdtPLzAZv6qj+iOfSrqinsoF0XrLkwdIcZQVtp3VRHhRoUE");
        QVERIFY(expected==transaction->toEnvelopeXdrBase64());
    }

    void testSha256HashSigning() {
        Network::usePublicNetwork();

        KeyPair *source = KeyPair::fromAccountId("GBBM6BKZPEHWYO3E3YKREDPQXMS4VK35YLNU7NFBRI26RAN7GI5POFBB");
        KeyPair *destination = KeyPair::fromAccountId("GDJJRRMBK4IWLEPJGIE6SXD2LP7REGZODU7WDC3I2D6MR37F4XSHBKX2");

        Account *account = new Account(source, 0);
        Transaction *transaction = Transaction::Builder(account)
                .addOperation(new PaymentOperation(destination, new AssetTypeNative(), "2000"))
                .build();

        quint8 preimage[64];
        for(int i=0;i<64;i++){
            preimage[i] = rand()%256;
        }
        QByteArray hash = Util::hash(QByteArray((char*)preimage,64));

        transaction->sign(QByteArray((char*)preimage,64));

        qDebug() <<QByteArray((char*)preimage,64).toHex();
        qDebug() << transaction->getSignatures()[0].signature.binary().toHex();
        QVERIFY(memcmp(preimage, transaction->getSignatures()[0].signature.binary().data(),sizeof(preimage))==0);
        QByteArray hint = hash.right(4);
        QVERIFY(memcmp(hint.data(),transaction->getSignatures()[0].hint.signatureHint,4)==0);

    }

    void testToBase64EnvelopeXdrBuilderNoSignatures() {
      // GBPMKIRA2OQW2XZZQUCQILI5TMVZ6JNRKM423BSAISDM7ZFWQ6KWEBC4
      KeyPair *source = KeyPair::fromSecretSeed(QString("SCH27VUZZ6UAKB67BDNF6FA42YMBMQCBKXWGMFD5TZ6S5ZZCZFLRXKHS"));
      KeyPair *destination = KeyPair::fromAccountId(QString("GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR"));

      Account *account = new Account(source, 2908908335136768L);
      Transaction *transaction = Transaction::Builder(account)
              .addOperation(new CreateAccountOperation(destination, "2000"))
              .build();

      try {
        transaction->toEnvelopeXdrBase64();
        QFAIL("missing exception");
      } catch (std::runtime_error exception) {
        QVERIFY(QString(exception.what()).contains("Transaction must be signed by at least one signer."));
      }
    }

    void testNoOperations() {
      // GBPMKIRA2OQW2XZZQUCQILI5TMVZ6JNRKM423BSAISDM7ZFWQ6KWEBC4
      KeyPair *source = KeyPair::fromSecretSeed(QString("SCH27VUZZ6UAKB67BDNF6FA42YMBMQCBKXWGMFD5TZ6S5ZZCZFLRXKHS"));

      Account *account = new Account(source, 2908908335136768);
      try {
        Transaction *transaction = Transaction::Builder(account).build();
        Q_UNUSED(transaction);
        QFAIL("missing exception");
      } catch (std::runtime_error exception) {
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
        Transaction::Builder(account)
                .addOperation(new CreateAccountOperation(destination, "2000"))
                .addMemo(Memo::none())
                .addMemo(Memo::none());
        QFAIL("missing exception");
      } catch (std::runtime_error exception) {
        QVERIFY(QString(exception.what()).contains("Memo has been already added."));
      }
  }

    void testDeserializeXdr() {
      // GBPMKIRA2OQW2XZZQUCQILI5TMVZ6JNRKM423BSAISDM7ZFWQ6KWEBC4

        KeyPair *source = KeyPair::fromSecretSeed(QString("SCH27VUZZ6UAKB67BDNF6FA42YMBMQCBKXWGMFD5TZ6S5ZZCZFLRXKHS"));
        KeyPair *destination = KeyPair::fromAccountId(QString("GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR"));
        Account *account = new Account(source, 2908908335136768);
        Transaction *transaction = Transaction::Builder(account)
                .addOperation(new CreateAccountOperation(destination, "2000"))
                .addMemo(Memo::text("Hello world!"))
                .build();

        transaction->sign(source);
        stellar::TransactionEnvelope envelope= transaction->toEnvelopeXdr();

        Transaction *deserialized = Transaction::fromXdrEnvelope(envelope);
        QVERIFY(deserialized->getFee()==transaction->getFee());
        QVERIFY(deserialized->getSequenceNumber()==transaction->getSequenceNumber());
        QVERIFY(dynamic_cast<MemoText*>(deserialized->getMemo())->getText() == dynamic_cast<MemoText*>(transaction->getMemo())->getText());
        QVERIFY(deserialized->getSourceAccount()->getPublicKey()== transaction->getSourceAccount()->getPublicKey());
        QVERIFY(memcmp((char*)deserialized->getSignatures().first().hint.signatureHint, (char*)transaction->getSignatures().first().hint.signatureHint,4)==0);
        QVERIFY(memcmp((char*)deserialized->getSignatures().first().signature.value.data(),(char*)transaction->getSignatures().first().signature.value.data(),64)==0);
    }
};

ADD_TEST(TransactionTest)

#endif // TRANSACTIONTEST_H
