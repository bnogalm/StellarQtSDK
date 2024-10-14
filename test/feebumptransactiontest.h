#ifndef FEEBUMPTRANSACTIONTEST_H
#define FEEBUMPTRANSACTIONTEST_H
#include <QObject>
#include "src/keypair.h"
#include "src/account.h"

#include "src/feebumptransaction.h"
#include "src/paymentoperation.h"
#include "src/assettypenative.h"
#include <QtTest>
#include "testcollector.h"

class FeeBumpTransactionTest: public QObject
{
    Q_OBJECT
public:
    Transaction* createInnerTransaction(int baseFee) {
        Network::useTestNetwork();
        KeyPair* source = KeyPair::fromSecretSeed(QString("SCH27VUZZ6UAKB67BDNF6FA42YMBMQCBKXWGMFD5TZ6S5ZZCZFLRXKHS"));

        Account* account = new Account(source, 2908908335136768L);
        Transaction* inner = Transaction::Builder(AccountConverter().enableMuxed(), account, Network::current())
                .addOperation(new PaymentOperation(
                                  "GA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVSGZ",
                                  new AssetTypeNative(),
                                  "200"
                                  ))
                .setBaseFee(baseFee)
                .addTimeBounds(new TimeBounds(10, 11))
                .build();

        inner->setEnvelopeType(stellar::EnvelopeType::ENVELOPE_TYPE_TX);
        inner->sign(source);
        return inner;
    }
    Transaction* createInnerTransaction() {
        return createInnerTransaction(Transaction::MIN_BASE_FEE);
    }
private slots:

    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }

    void testRequiresFeeAccount() {
        Transaction* inner = createInnerTransaction();

        try {
            FeeBumpTransaction::Builder(AccountConverter().enableMuxed(), inner)
                    .setBaseFee(Transaction::MIN_BASE_FEE * 2)
                    .build();
            QFAIL("missing exception");
        } catch (const std::runtime_error& e) {
            QCOMPARE(QString(e.what()), "fee account has to be set. you must call setFeeAccount().");
        }
    }

    void testSetFeeAccountMultipleTimes() {
        Transaction* inner = createInnerTransaction();

        try {
            FeeBumpTransaction::Builder(AccountConverter().enableMuxed(), inner)
                    .setBaseFee(Transaction::MIN_BASE_FEE * 2)
                    .setFeeAccount("GDQNY3PBOJOKYZSRMK2S7LHHGWZIUISD4QORETLMXEWXBI7KFZZMKTL3")
                    .setFeeAccount("GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR")
                    .build();
            QFAIL("missing exception");
        } catch (const std::runtime_error& e) {
            QCOMPARE(QString(e.what()), "fee account has been already been set.");
        }
    }


    void testRequiresBaseFee() {
        Transaction* inner = createInnerTransaction();

        try {
            FeeBumpTransaction::Builder(AccountConverter().enableMuxed(), inner)
                    .setFeeAccount("GDQNY3PBOJOKYZSRMK2S7LHHGWZIUISD4QORETLMXEWXBI7KFZZMKTL3")
                    .build();
            QFAIL("missing exception");
        } catch (const std::runtime_error& e) {
            QCOMPARE(QString(e.what()), "base fee has to be set. you must call setBaseFee().");
        }
    }


    void testSetBaseFeeMultipleTimes() {
        Transaction* inner = createInnerTransaction();

        try {
            FeeBumpTransaction::Builder(AccountConverter().enableMuxed(), inner)
                    .setBaseFee(Transaction::MIN_BASE_FEE * 2)
                    .setFeeAccount("GDQNY3PBOJOKYZSRMK2S7LHHGWZIUISD4QORETLMXEWXBI7KFZZMKTL3")
                    .setBaseFee(Transaction::MIN_BASE_FEE)
                    .build();
            QFAIL("missing exception");
        } catch (const std::runtime_error& e) {
            QCOMPARE(QString(e.what()), "base fee has been already set.");

        }
    }

    void testSetBaseFeeBelowNetworkMinimum() {
        Transaction* inner = createInnerTransaction();

        try {
            FeeBumpTransaction::Builder(AccountConverter().enableMuxed(), inner)
                    .setBaseFee(Transaction::MIN_BASE_FEE -1)
                    .setFeeAccount("GDQNY3PBOJOKYZSRMK2S7LHHGWZIUISD4QORETLMXEWXBI7KFZZMKTL3")
                    .build();
            QFAIL("missing exception");
        } catch (const std::runtime_error& e) {
            QCOMPARE(QString(e.what()), "baseFee cannot be smaller than the BASE_FEE");
        }
    }

    void testSetBaseFeeBelowInner() {
        Transaction* inner = createInnerTransaction(Transaction::MIN_BASE_FEE+1);

        try {
            FeeBumpTransaction::Builder(AccountConverter().enableMuxed(), inner)
                    .setBaseFee(Transaction::MIN_BASE_FEE)
                    .setFeeAccount("GDQNY3PBOJOKYZSRMK2S7LHHGWZIUISD4QORETLMXEWXBI7KFZZMKTL3")
                    .build();
            QFAIL("missing exception");
        } catch (const std::runtime_error& e) {
            QCOMPARE(QString(e.what()), "base fee cannot be lower than provided inner transaction base fee");
        }
    }

    void testSetBaseFeeOverflowsLong() {
        Transaction* inner = createInnerTransaction(Transaction::MIN_BASE_FEE+1);

        try {
            FeeBumpTransaction::Builder(AccountConverter().enableMuxed(), inner)
                    .setBaseFee(std::numeric_limits<qint64>::max())
                    .setFeeAccount("GDQNY3PBOJOKYZSRMK2S7LHHGWZIUISD4QORETLMXEWXBI7KFZZMKTL3")
                    .build();
            QFAIL("missing exception");
        } catch (const std::runtime_error& e) {
            QCOMPARE(QString(e.what()), "fee overflows 64 bit int");
        }
    }
    void testSetBaseFeeEqualToInner() {
        Transaction* inner = createInnerTransaction();

        FeeBumpTransaction* feeBump = FeeBumpTransaction::Builder(AccountConverter().enableMuxed(), inner)
                .setBaseFee(Transaction::MIN_BASE_FEE)
                .setFeeAccount("GDQNY3PBOJOKYZSRMK2S7LHHGWZIUISD4QORETLMXEWXBI7KFZZMKTL3")
                .build();

        QCOMPARE(feeBump->getFee(),Transaction::MIN_BASE_FEE*2);
    }

    void testFromEnvelopeXdr()
    {
        QString xdr ="AAAABQAAAADgSJG2GOUMy/H9lHyjYZOwyuyytH8y0wWaoc596L+bEgAAAAAAAADIAAAAAgAAAABzdv3ojkzWHMD7KUoXhrPx0GH18vHKV0ZfqpMiEblG1gAAAGQAAAAAAAAACAAAAAEAAAAAAAAAAAAAAAAAAAAAAAAAAQAAAA9IYXBweSBiaXJ0aGRheSEAAAAAAQAAAAAAAAABAAAAAOBIkbYY5QzL8f2UfKNhk7DK7LK0fzLTBZqhzn3ov5sSAAAAAAAAAASoF8gAAAAAAAAAAAERuUbWAAAAQK933Dnt1pxXlsf1B5CYn81PLxeYsx+MiV9EGbMdUfEcdDWUySyIkdzJefjpR5ejdXVp/KXosGmNUQ+DrIBlzg0AAAAAAAAAAei/mxIAAABAijIIQpL6KlFefiL4FP8UWQktWEz4wFgGNSaXe7mZdVMuiREntehi1b7MRqZ1h+W+Y0y+Z2HtMunsilT2yS5mAA==";
        auto tx = AbstractTransaction::fromEnvelopeXdr(xdr, Network::current());
        compareBase64( tx->toEnvelopeXdrBase64(),xdr);
        QCOMPARE( tx->toEnvelopeXdrBase64(),xdr);
    }


    void testHash() {
        Transaction* inner = createInnerTransaction();
        QCOMPARE( inner->hashHex(),"2a8ead3351faa7797b284f59027355ddd69c21adb8e4da0b9bb95531f7f32681");

        FeeBumpTransaction* feeBump = FeeBumpTransaction::Builder(AccountConverter().enableMuxed(), inner)
                .setBaseFee(Transaction::MIN_BASE_FEE * 2)
                .setFeeAccount("GDQNY3PBOJOKYZSRMK2S7LHHGWZIUISD4QORETLMXEWXBI7KFZZMKTL3")
                .build();

        QCOMPARE(feeBump->hashHex(),"58266712c0c1d1cd98faa0e0159605a361cf2a5ca44ad69650eeb1d27ee62334");
    }
    void testRoundTripXdr()  {
        Transaction* inner = createInnerTransaction();

        FeeBumpTransaction* feeBump = FeeBumpTransaction::Builder(AccountConverter().enableMuxed(), inner)
                .setBaseFee(Transaction::MIN_BASE_FEE * 2)
                .setFeeAccount("GDQNY3PBOJOKYZSRMK2S7LHHGWZIUISD4QORETLMXEWXBI7KFZZMKTL3")
                .build();

        QCOMPARE(Transaction::MIN_BASE_FEE * 4, feeBump->getFee());
        QCOMPARE("GDQNY3PBOJOKYZSRMK2S7LHHGWZIUISD4QORETLMXEWXBI7KFZZMKTL3", feeBump->getFeeAccount());
        QCOMPARE(inner, feeBump->getInnerTransaction());
        QCOMPARE(0, feeBump->getSignatures().size());

        FeeBumpTransaction* fromXdr = (FeeBumpTransaction*) AbstractTransaction::fromEnvelopeXdr(
                    feeBump->toEnvelopeXdrBase64(), Network::current()
                    );


        QCOMPARE(feeBump->toEnvelopeXdrBase64(), fromXdr->toEnvelopeXdrBase64());


        KeyPair* signer = KeyPair::random();
        feeBump->sign(signer);
        fromXdr = (FeeBumpTransaction*) AbstractTransaction::fromEnvelopeXdr(
                    feeBump->toEnvelopeXdrBase64(), Network::current()
                    );
        //QCOMPARE(feeBump, fromXdr);
        QCOMPARE(feeBump->toEnvelopeXdrBase64(), fromXdr->toEnvelopeXdrBase64());

        QCOMPARE(1, feeBump->getSignatures().size());
        signer->verify(feeBump->hash(), feeBump->getSignatures()[0].signature.binary());


        QCOMPARE(inner->hash(), fromXdr->getInnerTransaction()->hash());

        QCOMPARE(feeBump->getSignatures().size(), 1);
        signer->verify(feeBump->hash(), feeBump->getSignatures()[0].signature.binary());

    }
    void testFeeBumpUpgradesInnerToV1()
    {
      Transaction* innerV0 = createInnerTransaction();
      innerV0->setEnvelopeType(stellar::EnvelopeType::ENVELOPE_TYPE_TX_V0);

      FeeBumpTransaction* feeBump = FeeBumpTransaction::Builder(AccountConverter().enableMuxed(), innerV0)
          .setBaseFee(Transaction::MIN_BASE_FEE * 2)
          .setFeeAccount("GDQNY3PBOJOKYZSRMK2S7LHHGWZIUISD4QORETLMXEWXBI7KFZZMKTL3")
          .build();

      QCOMPARE(Transaction::MIN_BASE_FEE * 4, feeBump->getFee());
      QCOMPARE("GDQNY3PBOJOKYZSRMK2S7LHHGWZIUISD4QORETLMXEWXBI7KFZZMKTL3", feeBump->getFeeAccount());
      QCOMPARE(0, feeBump->getSignatures().size());

      QCOMPARE(stellar::EnvelopeType::ENVELOPE_TYPE_TX_V0, innerV0->toEnvelopeXdr().type);
      //QVERIFY(innerV0->hash()!= feeBump->getInnerTransaction()->hash());
      QVERIFY(innerV0->hash()== feeBump->getInnerTransaction()->hash());
      QVERIFY(innerV0->envelopeType()!= feeBump->getInnerTransaction()->envelopeType());
      innerV0->setEnvelopeType(stellar::EnvelopeType::ENVELOPE_TYPE_TX);
      QCOMPARE(innerV0->hash(), feeBump->getInnerTransaction()->hash());
      QVERIFY(innerV0->envelopeType()== feeBump->getInnerTransaction()->envelopeType());

      FeeBumpTransaction* fromXdr = (FeeBumpTransaction*) AbstractTransaction::fromEnvelopeXdr(
          feeBump->toEnvelopeXdrBase64(), Network::current()
      );

      QCOMPARE(feeBump->hash(), fromXdr->hash());


      KeyPair* signer = KeyPair::random();
      feeBump->sign(signer);
      fromXdr = (FeeBumpTransaction*) AbstractTransaction::fromEnvelopeXdr(
          feeBump->toEnvelopeXdrBase64(), Network::current()
      );
      QVERIFY(feeBump->hash()== fromXdr->hash());
      QVERIFY(feeBump->getInnerTransaction()->hash()== fromXdr->getInnerTransaction()->hash());

      QCOMPARE(feeBump->getSignatures().size(), 1);
      signer->verify(feeBump->hash(), feeBump->getSignatures()[0].signature.binary());
    }
};

ADD_TEST(FeeBumpTransactionTest)
#endif // FEEBUMPTRANSACTIONTEST_H

