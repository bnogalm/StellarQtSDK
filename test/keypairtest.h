#ifndef KEYPAIRTEST_H
#define KEYPAIRTEST_H
#include <QObject>
#include "src/keypair.h"
#include "src/util.h"

#include <QtTest>
#include "testcollector.h"
static const char SEED[] = "1123740522f11bfef6b3671f51e159ccf589ccf8965262dd5f97d1721d383dd4";


class KeyPairTest: public QObject
{
    Q_OBJECT
public:


private slots:

    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }
    void testSign(){
        QString expectedSig = "587d4b472eeef7d07aafcd0b049640b0bb3f39784118c2e2b73a04fa2f64c9c538b4b2d0f5335e968a480021fdc23e98c0ddf424cb15d8131df8cb6c4bb58309";

        KeyPair* keypair = KeyPair::fromSecretSeed(Util::hexToBytes(QString(SEED)));
        QByteArray data = "hello world";
        QByteArray sig = keypair->sign(data);
        //qDebug() <<"generated:"<< sig.toHex() ;
        //qDebug() <<"expected :"<< expectedSig;
        QVERIFY(Util::hexToBytes(expectedSig)==sig);

    }
    void testSignAndVerify(){

        KeyPair* keypair = KeyPair::fromSecretSeed(Util::hexToBytes(QString(SEED)));
        QByteArray data = "hello world";
        QByteArray sig = keypair->sign(data);
        QVERIFY(keypair->verify(data,sig));
    }


    void testVerifyFalse() {
      QString badSig = "687d4b472eeef7d07aafcd0b049640b0bb3f39784118c2e2b73a04fa2f64c9c538b4b2d0f5335e968a480021fdc23e98c0ddf424cb15d8131df8cb6c4bb58309";
      QByteArray corrupt;
      corrupt.append('\0');
      QByteArray data = "hello world";
      KeyPair* keypair = KeyPair::fromSecretSeed(Util::hexToBytes(SEED));
      QVERIFY(!keypair->verify(data , Util::hexToBytes(badSig)));
      QVERIFY(!keypair->verify(data , corrupt));
  }

    void testFromSecretSeed() {
        QHash<QString, QString> keypairs;
        keypairs.insert("SDJHRQF4GCMIIKAAAQ6IHY42X73FQFLHUULAPSKKD4DFDM7UXWWCRHBE", "GCZHXL5HXQX5ABDM26LHYRCQZ5OJFHLOPLZX47WEBP3V2PF5AVFK2A5D");
        keypairs.insert("SDTQN6XUC3D2Z6TIG3XLUTJIMHOSON2FMSKCTM2OHKKH2UX56RQ7R5Y4", "GDEAOZWTVHQZGGJY6KG4NAGJQ6DXATXAJO3AMW7C4IXLKMPWWB4FDNFZ");
        keypairs.insert("SDIREFASXYQVEI6RWCQW7F37E6YNXECQJ4SPIOFMMMJRU5CMDQVW32L5", "GD2EVR7DGDLNKWEG366FIKXO2KCUAIE3HBUQP4RNY7LEZR5LDKBYHMM6");
        keypairs.insert("SDAPE6RHEJ7745VQEKCI2LMYKZB3H6H366I33A42DG7XKV57673XLCC2", "GDLXVH2BTLCLZM53GF7ELZFF4BW4MHH2WXEA4Z5Z3O6DPNZNR44A56UJ");
        keypairs.insert("SDYZ5IYOML3LTWJ6WIAC2YWORKVO7GJRTPPGGNJQERH72I6ZCQHDAJZN", "GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7");

        for (QString seed : keypairs.keys()) {

            QString accountId = keypairs.value(seed);
            KeyPair *keypair = KeyPair::fromSecretSeed(seed);
            //qDebug() << accountId <<" ==? "<< keypair->getAccountId();
            QVERIFY(accountId== keypair->getAccountId());
            //qDebug() << seed <<" ==? "<< keypair->getSecretSeed();
            QVERIFY(seed== keypair->getSecretSeed());
        }
    }
    void testRandomMix() {
         QByteArray rand;
         for(int i=0;i<32;i++){
             rand.append(i);
         }
         KeyPair* issuer = KeyPair::random(rand);
         QVERIFY(issuer);
         rand = rand.remove(0,1);
         try{
            KeyPair* issuer = KeyPair::random(rand);
            Q_UNUSED(issuer)
            QFAIL("missing exception");
         }
         catch(const std::runtime_error& e)
         {
            Q_UNUSED(e)
         }
    }


    void testCanSign() {
      KeyPair* keypair;
      keypair = KeyPair::fromSecretSeed(QString("SDJHRQF4GCMIIKAAAQ6IHY42X73FQFLHUULAPSKKD4DFDM7UXWWCRHBE"));
      QVERIFY(keypair->canSign());
      keypair = KeyPair::fromAccountId(QString("GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7"));
      QVERIFY(!keypair->canSign());
    }

    void testSignWithoutSecret() {
        KeyPair* keypair = KeyPair::fromAccountId(QString("GDEAOZWTVHQZGGJY6KG4NAGJQ6DXATXAJO3AMW7C4IXLKMPWWB4FDNFZ"));
        QString data = "hello world";
        try {
            keypair->sign(data.toUtf8());
            QFAIL("missing exception");
        } catch (const std::runtime_error& e) {
            QCOMPARE(QString("KeyPair does not contain secret key. Use KeyPair::fromSecretSeed method to create a new KeyPair with a secret key."), QString(e.what()));
        }
    }

};

ADD_TEST(KeyPairTest)

#endif // KEYPAIRTEST_H
