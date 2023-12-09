#ifndef UTILTEST_H
#define UTILTEST_H
#include <QObject>

#include "src/util.h"
#include "src/keypair.h"
#include "src/transaction.h"

#include <QtTest>
#include "testcollector.h"
#include "slip10.h"

class UtilTest: public QObject
{
    Q_OBJECT


private slots:
    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }
//    void testBuildChallengeTx() {
//        auto serverKeypair = KeyPair::random();
//        auto clientKeypair = KeyPair::random();

//        // infinite timebound
//        auto tx = Util::buildChallengeTx(serverKeypair,clientKeypair->getAccountId(),"SDF",0);
//        auto xdr = tx->toEnvelopeXdr();
//        delete tx;


////        QCOMPARE(xdr.v0.tx.seqNum,0);
////        QCOMPARE(xdr.v0.tx.fee,static_cast<quint32>(100));
////        QCOMPARE(xdr.v0.tx.operations.value.size(),1);
////        QCOMPARE(xdr.v0.tx.timeBounds.value.minTime,static_cast<quint64>(0));
////        QCOMPARE(xdr.v0.tx.timeBounds.value.maxTime,static_cast<quint64>(0));
////        auto op = xdr.v0.tx.operations.value.at(0);


//        QCOMPARE(xdr.v1.tx.seqNum,0);
//        QCOMPARE(xdr.v1.tx.fee,static_cast<quint32>(100));
//        QCOMPARE(xdr.v1.tx.operations.value.size(),1);
//        QCOMPARE(xdr.v1.tx.timeBounds.value.minTime,static_cast<quint64>(0));
//        QCOMPARE(xdr.v1.tx.timeBounds.value.maxTime,static_cast<quint64>(0));
//        auto op = xdr.v1.tx.operations.value.at(0);

//        QCOMPARE(op.type,stellar::OperationType::MANAGE_DATA);
//        QCOMPARE(op.operationManageData.dataName.binary(),QByteArray("SDF auth"));
//        //48 random bytes enconded in base64
//        QCOMPARE(QByteArray::fromBase64(op.operationManageData.dataValue.value.binary()
//                                        ,QByteArray::Base64Option::Base64UrlEncoding|QByteArray::Base64Option::OmitTrailingEquals).size(),48);


//        // 5 minutes timebound

//        tx = Util::buildChallengeTx(serverKeypair,clientKeypair->getAccountId(),"SDF1",5*60);
//        xdr = tx->toEnvelopeXdr();
//        delete tx;
////        QCOMPARE(xdr.v0.tx.seqNum,0);
////        QCOMPARE(xdr.v0.tx.fee,static_cast<quint32>(100));
////        QCOMPARE(xdr.v0.tx.operations.value.size(),1);
////        QCOMPARE(xdr.v0.tx.timeBounds.value.maxTime- xdr.v0.tx.timeBounds.value.minTime,static_cast<quint64>(300));
////        op = xdr.v0.tx.operations.value.at(0);


//        QCOMPARE(xdr.v1.tx.seqNum,0);
//        QCOMPARE(xdr.v1.tx.fee,static_cast<quint32>(100));
//        QCOMPARE(xdr.v1.tx.operations.value.size(),1);
//        QCOMPARE(xdr.v1.tx.timeBounds.value.maxTime- xdr.v1.tx.timeBounds.value.minTime,static_cast<quint64>(300));
//        op = xdr.v1.tx.operations.value.at(0);

//        QCOMPARE(op.type,stellar::OperationType::MANAGE_DATA);
//        QCOMPARE(op.operationManageData.dataName.binary(),QByteArray("SDF1 auth"));
//        //48 random bytes enconded in base64
//        QCOMPARE(QByteArray::fromBase64(op.operationManageData.dataValue.value.binary()
//                                        ,QByteArray::Base64Option::Base64UrlEncoding|QByteArray::Base64Option::OmitTrailingEquals).size(),48);

//    }

    void testMnemonicToBIP39Seed()
    {

        //12
        QByteArray bip39seed = Util::mnemonicToBIP39Seed("illness spike retreat truth genius clock brain pass fit cave bargain toe");
        QCOMPARE(bip39seed,QByteArray::fromHex("e4a5a632e70943ae7f07659df1332160937fad82587216a4c64315a0fb39497ee4a01f76ddab4cba68147977f3a147b6ad584c41808e8238a07f6cc4b582f186"));

        //15
        bip39seed = Util::mnemonicToBIP39Seed("resource asthma orphan phone ice canvas fire useful arch jewel impose vague theory cushion top");
        QCOMPARE(bip39seed,QByteArray::fromHex("7b36d4e725b48695c3ffd2b4b317d5552cb157c1a26c46d36a05317f0d3053eb8b3b6496ba39ebd9312d10e3f9937b47a6790541e7c577da027a564862e92811"));

        //24
        bip39seed = Util::mnemonicToBIP39Seed("bench hurt jump file august wise shallow faculty impulse spring exact slush thunder author capable act festival slice deposit sauce coconut afford frown better");
        QCOMPARE(bip39seed,QByteArray::fromHex("937ae91f6ab6f12461d9936dfc1375ea5312d097f3f1eb6fed6a82fbe38c85824da8704389831482db0433e5f6c6c9700ff1946aa75ad8cc2654d6e40f567866"));

        //24 +  password
        bip39seed = Util::mnemonicToBIP39Seed("cable spray genius state float twenty onion head street palace net private method loan turn phrase state blanket interest dry amazing dress blast tube","p4ssphr4se");
        QCOMPARE(bip39seed,QByteArray::fromHex("d425d39998fb42ce4cf31425f0eaec2f0a68f47655ea030d6d26e70200d8ff8bd4326b4bdf562ea8640a1501ae93ccd0fd7992116da5dfa24900e570a742a489"));


        QCOMPARE(SLIP10::deriveEd25519PrivateKey(bip39seed,QVector<int>() << 44<<148),QByteArray::fromHex("c83c61dc97d37832f0f20e258c3ba4040a258800fd14abaff124a4dee114b17e"));

        KeyPair *account = KeyPair::fromBip39Seed(bip39seed, 0);
        QCOMPARE(account->getAccountId(),QString("GDAHPZ2NSYIIHZXM56Y36SBVTV5QKFIZGYMMBHOU53ETUSWTP62B63EQ"));
        QCOMPARE(account->getSecretSeed(),QString("SAFWTGXVS7ELMNCXELFWCFZOPMHUZ5LXNBGUVRCY3FHLFPXK4QPXYP2X"));

        try {
            Util::mnemonicToBIP39Seed("illness spike retreat truth genius clock brain pass fit cave bargain");
            QFAIL("expected exception, not enough words");
        } catch (const std::runtime_error& err) {
            Q_UNUSED(err)
        }
        try {
            Util::mnemonicToBIP39Seed("illness spieke retreat truth genius clock brain pass fit cave bargain bargain");
            QFAIL("expected exception, invalid word");
        } catch (const std::runtime_error& err) {
            Q_UNUSED(err)
        }

    }
};

ADD_TEST(UtilTest)
#endif // UTILTEST_H
