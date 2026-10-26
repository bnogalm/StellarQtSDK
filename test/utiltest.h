#ifndef UTILTEST_H
#define UTILTEST_H
#include <QObject>

#include "src/util.h"
#include "src/keypair.h"
#include "src/transaction.h"
#include "src/strkey.h"

#include <QtTest>
#include "testcollector.h"
#include <memory>
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

    /**
     * Why a phrase is refused, for a C++ caller that has only had a bool:
     * the count, the position of a word not in the list (never the word),
     * or the checksum. mnemonicToBIP39Seed says the same.
     */
    void testAMnemonicSaysWhyItIsRefused()
    {
        const QString good = QStringLiteral("abandon abandon abandon abandon abandon abandon abandon abandon "
                                            "abandon abandon abandon about");
        QCOMPARE(Util::mnemonicProblem(good), QString());
        QVERIFY(Util::mnemonicProblem(QStringLiteral("abandon about")).contains(QStringLiteral("2 words")));
        QString unknown = good;
        unknown.replace(QStringLiteral("abandon abandon abandon about"), QStringLiteral("abandon abandon zzzzq about"));
        QCOMPARE(Util::mnemonicProblem(unknown), QStringLiteral("word 11 is not in the wordlist"));
        QString swapped = good;
        swapped.replace(QStringLiteral("abandon about"), QStringLiteral("about abandon"));
        QVERIFY(Util::mnemonicProblem(swapped).contains(QStringLiteral("checksum")));
        try {
            Util::mnemonicToBIP39Seed(unknown);
            QFAIL("accepted");
        } catch (const std::exception& e) {
            QVERIFY(QString::fromUtf8(e.what()).contains(QStringLiteral("word 11")));
            QVERIFY(!QString::fromUtf8(e.what()).contains(QStringLiteral("zzzzq")));
        }
    }

    /**
     * The eight BIP-39 lists py-stellar-sdk has. For each, two entropies give
     * the phrase py-stellar-sdk 14.1.0 writes, byte for byte (NFKD words;
     * U+3000 between Japanese ones), and the phrase opens the account py
     * opens (index 1, passphrase "pé"). A phrase is read against its own
     * list: in another, it is refused.
     */
    void testEveryBip39ListPyHas()
    {
        struct Vector {
            Util::SupportedMnemonicLanguages language;
            const char* entropyHex;
            QString phrase;
            const char* account;
        };
        const QList<Vector> vectors = {
            {Util::SupportedMnemonicLanguages::JA, "000102030405060708090a0b0c0d0e0f",
             QString::fromUtf8("\xe3\x81\x82\xe3\x81\x84\xe3\x81\x93\xe3\x81\x8f\xe3\x81\x97\xe3\x82\x93\xe3\x80\x80\xe3\x81\x84\xe3\x81\x8f\xe3\x81\xb5\xe3\x82\x99\xe3\x82\x93\xe3\x80\x80\xe3\x81\x9d\xe3\x81\xaa\xe3\x81\x9f\xe3\x80\x80\xe3\x81\x84\xe3\x81\x8f\xe3\x81\xb5\xe3\x82\x99\xe3\x82\x93\xe3\x80\x80\xe3\x81\x93\xe3\x81\x9b\xe3\x82\x99\xe3\x82\x93\xe3\x80\x80\xe3\x81\x82\xe3\x81\xb5\xe3\x82\x99\xe3\x82\x89\xe3\x80\x80\xe3\x81\x8a\xe3\x81\x8a\xe3\x81\x86\xe3\x80\x80\xe3\x81\x8a\xe3\x81\x8d\xe3\x82\x8b\xe3\x80\x80\xe3\x81\x84\xe3\x81\x9f\xe3\x81\xbf\xe3\x80\x80\xe3\x81\x95\xe3\x82\x93\xe3\x81\x99\xe3\x81\x86\xe3\x80\x80\xe3\x81\x91\xe3\x81\x9f\xe3\x81\xaf\xe3\x82\x99\xe3\x80\x80\xe3\x81\x8a\xe3\x81\x86\xe3\x81\x9f\xe3\x81\x84"),
             "GB6S36L3LPP2JFU34CE6HN4YOHHW66HC6EIBSLER7X7O3YXWKT2CETCW"},
            {Util::SupportedMnemonicLanguages::JA, "fffefdfcfbfaf9f8f7f6f5f4f3f2f1f0efeeedecebeae9e8e7e6e5e4e3e2e1e0",
             QString::fromUtf8("\xe3\x82\x8f\xe3\x82\x8c\xe3\x82\x8b\xe3\x80\x80\xe3\x82\x8a\xe3\x81\x8f\xe3\x81\xa4\xe3\x80\x80\xe3\x81\x9d\xe3\x81\x9d\xe3\x81\x8f\xe3\x82\x99\xe3\x80\x80\xe3\x82\x8a\xe3\x81\x8f\xe3\x81\xa4\xe3\x80\x80\xe3\x81\xab\xe3\x81\x82\xe3\x81\x86\xe3\x80\x80\xe3\x82\x8c\xe3\x82\x93\xe3\x81\x95\xe3\x81\x84\xe3\x80\x80\xe3\x81\xbb\xe3\x82\x93\xe3\x82\x84\xe3\x81\x8f\xe3\x80\x80\xe3\x81\xbb\xe3\x81\xa3\xe3\x81\x8d\xe3\x82\x87\xe3\x81\x8f\xe3\x80\x80\xe3\x82\x88\xe3\x81\xa3\xe3\x81\x8b\xe3\x80\x80\xe3\x81\xa6\xe3\x81\xa1\xe3\x81\x8b\xe3\x82\x99\xe3\x81\x84\xe3\x80\x80\xe3\x81\xae\xe3\x81\x8b\xe3\x82\x99\xe3\x81\x99\xe3\x80\x80\xe3\x81\xbe\xe3\x81\xbb\xe3\x81\x86\xe3\x80\x80\xe3\x82\x8f\xe3\x81\x8b\xe3\x81\x99\xe3\x80\x80\xe3\x81\x9b\xe3\x81\xa4\xe3\x82\x81\xe3\x81\x84\xe3\x80\x80\xe3\x81\xab\xe3\x82\x93\xe3\x81\x8b\xe3\x80\x80\xe3\x81\x9d\xe3\x81\x86\xe3\x81\x9f\xe3\x82\x99\xe3\x82\x93\xe3\x80\x80\xe3\x82\x80\xe3\x81\xae\xe3\x81\x86\xe3\x80\x80\xe3\x81\x91\xe3\x82\x80\xe3\x81\x97\xe3\x80\x80\xe3\x82\x8a\xe3\x82\x87\xe3\x81\x93\xe3\x81\x86\xe3\x80\x80\xe3\x81\xb2\xe3\x81\x9f\xe3\x82\x8b\xe3\x80\x80\xe3\x81\x93\xe3\x82\x99\xe3\x81\x8b\xe3\x81\x84\xe3\x80\x80\xe3\x82\x86\xe3\x81\x91\xe3\x81\xa4\xe3\x80\x80\xe3\x81\x9f\xe3\x81\x84\xe3\x82\x88\xe3\x81\x86\xe3\x80\x80\xe3\x81\x82\xe3\x81\xa4\xe3\x82\x81\xe3\x82\x8b"),
             "GB3LXAVVA2A4FD6I6BBB32XC4HIVEFAA5AQ4JS6QWGXKDATCLJIMHYCB"},
            {Util::SupportedMnemonicLanguages::FR, "000102030405060708090a0b0c0d0e0f",
             QString::fromUtf8("\x61\x62\x61\x69\x73\x73\x65\x72\x20\x61\x67\x72\x65\xcc\x81\x61\x62\x6c\x65\x20\x69\x6e\x64\x75\x63\x74\x69\x66\x20\x61\x67\x72\x65\xcc\x81\x61\x62\x6c\x65\x20\x65\xcc\x81\x6c\x69\x67\x69\x62\x6c\x65\x20\x61\x63\x68\x61\x74\x20\x62\x6f\x6c\x69\x64\x65\x20\x62\x6f\x75\x63\x6c\x65\x20\x61\x6d\x61\x74\x65\x75\x72\x20\x65\x78\x69\x73\x74\x65\x72\x20\x64\x65\xcc\x81\x72\x6f\x62\x65\x72\x20\x62\x6c\x6f\x71\x75\x65\x72"),
             "GC64IO5QSAUS3YRUVXNQB6MY6VBGSGX4IUEESEF4EVLU7XNKOYHBY5H5"},
            {Util::SupportedMnemonicLanguages::FR, "fffefdfcfbfaf9f8f7f6f5f4f3f2f1f0efeeedecebeae9e8e7e6e5e4e3e2e1e0",
             QString::fromUtf8("\x7a\x6f\x6f\x6c\x6f\x67\x69\x65\x20\x76\x65\xcc\x81\x6c\x6f\x63\x65\x20\x69\x6d\x70\x61\x63\x74\x20\x76\x65\xcc\x81\x6c\x6f\x63\x65\x20\x6f\x75\x74\x72\x61\x67\x65\x72\x20\x76\x69\x73\x75\x65\x6c\x20\x73\x6f\x6e\x6e\x65\x74\x74\x65\x20\x73\x6f\x6c\x64\x61\x74\x20\x75\x74\x6f\x70\x69\x65\x20\x6d\x6f\x75\x66\x6c\x65\x20\x70\x6f\x63\x68\x65\x74\x74\x65\x20\x73\x70\x65\xcc\x81\x63\x69\x61\x6c\x20\x76\x6f\x79\x61\x67\x65\x20\x67\x75\x65\xcc\x81\x70\x61\x72\x64\x20\x70\x61\x73\x73\x69\x6f\x6e\x20\x68\x79\x64\x72\x6f\x6d\x65\x6c\x20\x74\x65\x72\x6d\x69\x6e\x65\x72\x20\x64\x69\x72\x69\x67\x65\x72\x20\x76\x65\xcc\x81\x74\x75\x73\x74\x65\x20\x72\x65\x6d\x6f\x6e\x74\x65\x72\x20\x65\xcc\x81\x63\x75\x72\x65\x75\x69\x6c\x20\x74\x75\x6c\x69\x70\x65\x20\x6a\x6f\x6e\x67\x6c\x65\x72\x20\x61\x63\x63\x6f\x6c\x61\x64\x65"),
             "GBHQ3TA4OQXYEIYA4LC3WV3YOLWCIRZMAJZNGIC43LQZMONLZJF6DSEV"},
            {Util::SupportedMnemonicLanguages::EN, "000102030405060708090a0b0c0d0e0f",
             QString::fromUtf8("abandon amount liar amount expire adjust cage candy arch gather drum buyer"),
             "GCBCJDTTIHBVU7GMJG7A5OHU5XWYXYGDEZTYTMNCVLC5ZDFXXCXVR7H5"},
            {Util::SupportedMnemonicLanguages::EN, "fffefdfcfbfaf9f8f7f6f5f4f3f2f1f0efeeedecebeae9e8e7e6e5e4e3e2e1e0",
             QString::fromUtf8("zoo wave left wave question wise thank team visual panel round tide year ivory recipe later try elbow whale slim evolve vapor maple adapt"),
             "GA7BHRFELHT2KWR7BGHLU6Z4GVSLNX6P7X5NIT3JBKZFUDETLDLCCK7Q"},
            {Util::SupportedMnemonicLanguages::ES, "000102030405060708090a0b0c0d0e0f",
             QString::fromUtf8("\x61\xcc\x81\x62\x61\x63\x6f\x20\x61\xcc\x81\x6c\x62\x75\x6d\x20\x6c\x69\xcc\x81\x71\x75\x69\x64\x6f\x20\x61\xcc\x81\x6c\x62\x75\x6d\x20\x65\x73\x70\x75\x6d\x61\x20\x61\x63\x75\x64\x69\x72\x20\x62\x6f\x6c\x65\x72\x6f\x20\x62\x6f\x73\x71\x75\x65\x20\x61\x6d\x61\x6e\x74\x65\x20\x67\x61\x69\x74\x61\x20\x64\x69\x63\x74\x61\x72\x20\x62\x6f\x63\x61"),
             "GATHYL7BFRH2B2XILW7URHXIJW7UA5JPZ6UQSSDR3QF55AORGVFD6UAA"},
            {Util::SupportedMnemonicLanguages::ES, "fffefdfcfbfaf9f8f7f6f5f4f3f2f1f0efeeedecebeae9e8e7e6e5e4e3e2e1e0",
             QString::fromUtf8("\x7a\x75\x72\x64\x6f\x20\x76\x65\x6e\x74\x61\x20\x6c\x69\x65\x6e\x7a\x6f\x20\x76\x65\x6e\x74\x61\x20\x70\x65\x6e\x61\x20\x76\x69\x76\x65\x72\x6f\x20\x74\x61\x6c\x61\x72\x20\x73\x75\x73\x74\x6f\x20\x76\x61\x6c\x65\x20\x6f\x63\x68\x65\x6e\x74\x61\x20\x70\x72\x6f\x6e\x74\x6f\x20\x74\x61\x72\x69\x66\x61\x20\x79\x6f\x67\x75\x72\x20\x6a\x75\x65\x76\x65\x73\x20\x70\x69\x6c\x6f\x74\x6f\x20\x6c\x65\x6e\x67\x75\x61\x20\x74\x6f\x67\x61\x20\x64\x75\x63\x68\x61\x20\x76\x69\xcc\x81\x63\x74\x69\x6d\x61\x20\x72\x6f\x64\x61\x72\x20\x65\x6e\x74\x72\x61\x72\x20\x74\x75\x72\x69\x73\x6d\x6f\x20\x6d\x61\x6e\x64\x6f\x20\x61\x63\x74\x69\x76\x6f"),
             "GC6GVNDCLNHXFNEZR5P2MSGEYDK4GILFOQZMCN4QSN6FETLIBOFRH2WX"},
            {Util::SupportedMnemonicLanguages::IT, "000102030405060708090a0b0c0d0e0f",
             QString::fromUtf8("abaco alogeno mitigare alogeno fenomeno affetto bravura bronzina ampio gonfio elaborato bottino"),
             "GBXMR53LVXP7YLT2RYRT5FJOUCSONCN3FGXBHRF2NTQCYKYY6VCRWFKA"},
            {Util::SupportedMnemonicLanguages::IT, "fffefdfcfbfaf9f8f7f6f5f4f3f2f1f0efeeedecebeae9e8e7e6e5e4e3e2e1e0",
             QString::fromUtf8("zuppa vero milano vero rastrello vivido sussurro sudore vano pianta rosolare taciturno zinco madama ribelle meschino totano epocale vidimare setola fare unte nastrare adipe"),
             "GBORCJ7ZDNYP7QBEE2L2CBKB6DDXNQSZVHHDJ353UAE4UZNG4ZDLHLKD"},
            {Util::SupportedMnemonicLanguages::KO, "000102030405060708090a0b0c0d0e0f",
             QString::fromUtf8("\xe1\x84\x80\xe1\x85\xa1\xe1\x84\x80\xe1\x85\xa7\xe1\x86\xa8\x20\xe1\x84\x80\xe1\x85\xa5\xe1\x86\xa8\xe1\x84\x8c\xe1\x85\xa5\xe1\x86\xbc\x20\xe1\x84\x89\xe1\x85\xb5\xe1\x86\xb7\xe1\x84\x87\xe1\x85\xae\xe1\x84\x85\xe1\x85\xb3\xe1\x86\xb7\x20\xe1\x84\x80\xe1\x85\xa5\xe1\x86\xa8\xe1\x84\x8c\xe1\x85\xa5\xe1\x86\xbc\x20\xe1\x84\x87\xe1\x85\xa7\xe1\x86\xaf\xe1\x84\x83\xe1\x85\xa9\x20\xe1\x84\x80\xe1\x85\xa1\xe1\x86\xaf\xe1\x84\x89\xe1\x85\xa2\xe1\x86\xa8\x20\xe1\x84\x80\xe1\x85\xb5\xe1\x84\x87\xe1\x85\xa5\xe1\x86\xb8\x20\xe1\x84\x80\xe1\x85\xb5\xe1\x84\x8b\xe1\x85\xae\xe1\x86\xab\x20\xe1\x84\x80\xe1\x85\xa7\xe1\x86\xbc\xe1\x84\x85\xe1\x85\xa7\xe1\x86\xa8\x20\xe1\x84\x89\xe1\x85\xa1\xe1\x86\xab\xe1\x84\x80\xe1\x85\xb5\xe1\x86\xaf\x20\xe1\x84\x86\xe1\x85\xb5\xe1\x84\x89\xe1\x85\xae\xe1\x86\xaf\x20\xe1\x84\x80\xe1\x85\xb5\xe1\x84\x82\xe1\x85\xa7\xe1\x86\xb7"),
             "GAHHOP3ASFVABSZ63NT5N3A7ESWW2FYTGTG6F6IWVEMOD6YFNNEIPKJ2"},
            {Util::SupportedMnemonicLanguages::KO, "fffefdfcfbfaf9f8f7f6f5f4f3f2f1f0efeeedecebeae9e8e7e6e5e4e3e2e1e0",
             QString::fromUtf8("\xe1\x84\x92\xe1\x85\xb5\xe1\x86\xb7\xe1\x84\x81\xe1\x85\xa5\xe1\x86\xba\x20\xe1\x84\x92\xe1\x85\xa7\xe1\x86\xbc\xe1\x84\x8c\xe1\x85\xa6\x20\xe1\x84\x89\xe1\x85\xb5\xe1\x86\xaf\xe1\x84\x89\xe1\x85\xb3\xe1\x86\xb8\x20\xe1\x84\x92\xe1\x85\xa7\xe1\x86\xbc\xe1\x84\x8c\xe1\x85\xa6\x20\xe1\x84\x8c\xe1\x85\xa1\xe1\x84\x83\xe1\x85\xa9\xe1\x86\xbc\x20\xe1\x84\x92\xe1\x85\xac\xe1\x84\x80\xe1\x85\xa7\xe1\x86\xab\x20\xe1\x84\x8f\xe1\x85\xb3\xe1\x86\xab\xe1\x84\x8c\xe1\x85\xa5\xe1\x86\xaf\x20\xe1\x84\x8f\xe1\x85\xab\xe1\x84\x80\xe1\x85\xa1\xe1\x86\xb7\x20\xe1\x84\x92\xe1\x85\xa2\xe1\x86\xbc\xe1\x84\x87\xe1\x85\xa9\xe1\x86\xa8\x20\xe1\x84\x8b\xe1\x85\xaf\xe1\x86\xaf\xe1\x84\x83\xe1\x85\xb3\xe1\x84\x8f\xe1\x85\xa5\xe1\x86\xb8\x20\xe1\x84\x8c\xe1\x85\xa5\xe1\x86\xb7\xe1\x84\x8e\xe1\x85\xa1\x20\xe1\x84\x90\xe1\x85\xa6\xe1\x84\x82\xe1\x85\xb5\xe1\x84\x89\xe1\x85\xb3\x20\xe1\x84\x92\xe1\x85\xb3\xe1\x86\xab\xe1\x84\x8c\xe1\x85\xa5\xe1\x86\xa8\x20\xe1\x84\x89\xe1\x85\xb3\xe1\x86\xb8\xe1\x84\x80\xe1\x85\xaa\xe1\x86\xab\x20\xe1\x84\x8c\xe1\x85\xa1\xe1\x86\xbc\xe1\x84\x85\xe1\x85\xa8\x20\xe1\x84\x89\xe1\x85\xb5\xe1\x86\xab\xe1\x84\x87\xe1\x85\xa1\xe1\x86\xaf\x20\xe1\x84\x91\xe1\x85\xad\xe1\x84\x92\xe1\x85\xa7\xe1\x86\xab\x20\xe1\x84\x87\xe1\x85\xa1\xe1\x86\xab\xe1\x84\x87\xe1\x85\xa1\xe1\x86\xaf\x20\xe1\x84\x92\xe1\x85\xa9\xe1\x86\xbc\xe1\x84\x89\xe1\x85\xae\x20\xe1\x84\x8c\xe1\x85\xb5\xe1\x86\xa8\xe1\x84\x89\xe1\x85\xa5\xe1\x86\xab\x20\xe1\x84\x87\xe1\x85\xa5\xe1\x86\xaf\xe1\x84\x80\xe1\x85\xb3\xe1\x86\xb7\x20\xe1\x84\x92\xe1\x85\xa1\xe1\x86\xab\xe1\x84\x8b\xe1\x85\xa7\xe1\x84\x85\xe1\x85\xb3\xe1\x86\xb7\x20\xe1\x84\x8b\xe1\x85\xa3\xe1\x86\xa8\xe1\x84\x89\xe1\x85\xae\x20\xe1\x84\x80\xe1\x85\xa1\xe1\x86\xab\xe1\x84\x8c\xe1\x85\xa5\xe1\x86\xb8"),
             "GBCU53RFER2HRSR5I2QCCRQQ5BIV4QCZL2ASVU5BLX6336KDHM7CUT2N"},
            {Util::SupportedMnemonicLanguages::ZH_HANS, "000102030405060708090a0b0c0d0e0f",
             QString::fromUtf8("\xe7\x9a\x84\x20\xe4\xb8\x89\x20\xe6\xac\xa7\x20\xe4\xb8\x89\x20\xe8\x80\x83\x20\xe4\xba\x8e\x20\xe6\x8d\xae\x20\xe4\xbf\x9d\x20\xe9\x87\x8f\x20\xe6\x8d\x9f\x20\xe7\xa0\xb4\x20\xe6\x88\x98"),
             "GAMDYEVYMXJBKNR3ZWG3CTKBFSHQPRRGL3UVB2QKPZKDQRZ57GW2JJOW"},
            {Util::SupportedMnemonicLanguages::ZH_HANS, "fffefdfcfbfaf9f8f7f6f5f4f3f2f1f0efeeedecebeae9e8e7e6e5e4e3e2e1e0",
             QString::fromUtf8("\xe6\xad\x87\x20\xe4\xbf\xa9\x20\xe5\xa0\x82\x20\xe4\xbf\xa9\x20\xe4\xbc\x8d\x20\xe6\xb2\xab\x20\xe9\x9c\x89\x20\xe5\xb1\x88\x20\xe6\x91\x98\x20\xe6\x97\x85\x20\xe7\x81\x8c\x20\xe7\x83\x83\x20\xe5\xb0\x9d\x20\xe8\x80\x90\x20\xe6\x8a\xb1\x20\xe7\x9b\x98\x20\xe9\x85\x9a\x20\xe8\xa7\x86\x20\xe6\xa1\xa3\x20\xe8\xb6\x8b\x20\xe8\xa7\x89\x20\xe8\xb4\xaa\x20\xe5\x9c\x88\x20\xe7\x94\x9f"),
             "GCZXR3FLJGPCH6OBF2H53MLAT2KZWCXUQ7UCLJFRUVUGHQE2EJJP4UBE"},
            {Util::SupportedMnemonicLanguages::ZH_HANT, "000102030405060708090a0b0c0d0e0f",
             QString::fromUtf8("\xe7\x9a\x84\x20\xe4\xb8\x89\x20\xe6\xad\x90\x20\xe4\xb8\x89\x20\xe8\x80\x83\x20\xe6\x96\xbc\x20\xe6\x93\x9a\x20\xe4\xbf\x9d\x20\xe9\x87\x8f\x20\xe6\x90\x8d\x20\xe7\xa0\xb4\x20\xe6\x88\xb0"),
             "GBHOO7GIHB3UMZ5YECEH2WZXWKOHHTUJ3AEJP6MEZPZS7MAB7LCWOSUP"},
            {Util::SupportedMnemonicLanguages::ZH_HANT, "fffefdfcfbfaf9f8f7f6f5f4f3f2f1f0efeeedecebeae9e8e7e6e5e4e3e2e1e0",
             QString::fromUtf8("\xe6\xad\x87\x20\xe5\x80\x86\x20\xe5\xa0\x82\x20\xe5\x80\x86\x20\xe4\xbc\x8d\x20\xe6\xb2\xab\x20\xe9\xbb\xb4\x20\xe5\xb1\x88\x20\xe6\x91\x98\x20\xe6\x97\x85\x20\xe7\x81\x8c\x20\xe7\x83\xb4\x20\xe5\x98\x97\x20\xe8\x80\x90\x20\xe6\x8a\xb1\x20\xe7\x9b\xa4\x20\xe9\x85\x9a\x20\xe8\xa6\x96\x20\xe6\xaa\x94\x20\xe8\xb6\xa8\x20\xe8\xa6\xba\x20\xe8\xb2\xaa\x20\xe5\x9c\x88\x20\xe7\x94\x9f"),
             "GBAXILQXALBN7UM3JYKA74HFVVCDHBBCHRLOVO744B3DM75CVLS2VV4B"},
        };
        QCOMPARE(vectors.size(), 16);
        for (const Vector& v : vectors) {
            QCOMPARE(Util::mnemonicWords(v.language).size(), 2048);
            QCOMPARE(Util::entropyToMnemonic(QByteArray::fromHex(v.entropyHex), v.language), v.phrase);
            QVERIFY(Util::validateMnemonic(v.phrase, v.language));
            std::unique_ptr<KeyPair> key(KeyPair::fromMnemonic(v.phrase, QString::fromUtf8("p\xc3\xa9"), 1, v.language));
            QCOMPARE(key->getAccountId(), QString::fromLatin1(v.account));
            // The same phrase composed (NFC), as a keyboard types it, is the same phrase.
            std::unique_ptr<KeyPair> typed(KeyPair::fromMnemonic(v.phrase.normalized(QString::NormalizationForm_C),
                                                                 QString::fromUtf8("p\xc3\xa9"), 1, v.language));
            QCOMPARE(typed->getAccountId(), QString::fromLatin1(v.account));
            if (v.language != Util::SupportedMnemonicLanguages::EN) {
                QVERIFY(!Util::validateMnemonic(v.phrase));
                QVERIFY_EXCEPTION_THROWN(Util::mnemonicToBIP39Seed(v.phrase), std::runtime_error);
            }
        }
        const QString generated = Util::generateMnemonic(256, Util::SupportedMnemonicLanguages::KO);
        QVERIFY(Util::validateMnemonic(generated, Util::SupportedMnemonicLanguages::KO));
    }

    /**
     * The checksum is checked on the way to a wallet, as py-stellar-sdk does:
     * two words swapped derived another, valid-looking wallet. The legacy
     * recovery derivation still opens such a phrase (1.0.3 did not check).
     * Words may be separated by any whitespace. KeyPair::fromMnemonic gives
     * SEP-5's account 0.
     */
    void testTheMnemonicChecksumIsChecked()
    {
        const QString good = "illness spike retreat truth genius clock brain pass fit cave bargain toe";
        const QString swapped = "spike illness retreat truth genius clock brain pass fit cave bargain toe";
        QVERIFY(!Util::validateMnemonic(swapped));
        QVERIFY_EXCEPTION_THROWN(Util::mnemonicToBIP39Seed(swapped), std::runtime_error);
        QVERIFY_EXCEPTION_THROWN(delete KeyPair::fromMnemonic(swapped), std::runtime_error);
        QCOMPARE(Util::mnemonicToBIP39SeedLegacyPassphrase(swapped).size(), 64);
        QScopedPointer<KeyPair> account0(KeyPair::fromMnemonic(good));
        QCOMPARE(account0->getAccountId(), QString("GDRXE2BQUC3AZNPVFSCEZ76NJ3WWL25FYFK6RGZGIEKWE4SOOHSUJUJ6"));
        QScopedPointer<KeyPair> account1(KeyPair::fromMnemonic(QString(good).replace(' ', '\n'), QString(), 1));
        QCOMPARE(account1->getAccountId(), QString("GBAW5XGWORWVFE2XTJYDTLDHXTY2Q2MO73HYCGB3XMFMQ562Q2W2GJQX"));
        QCOMPARE(Util::mnemonicToBIP39Seed(QString(good).replace(' ', '\t')), Util::mnemonicToBIP39Seed(good));
    }

    /**
     * The contract id preimage is encoded checked. A pool-share asset has no
     * encoding in it: only the preimage's type was written, and the hash of
     * that truncated preimage came back as the asset's contract id.
     */
    void testContractIdOfAnUnencodableAssetThrows()
    {
        const QString testnet = QStringLiteral("Test SDF Network ; September 2015");
        stellar::Asset native;
        // js-stellar-sdk 17.1.0: Asset.native().contractId(Networks.TESTNET).
        QCOMPARE(StrKey::encodeContract(Util::getContractAddressFromAsset(testnet, native)),
                 QStringLiteral("CDLZFC3SYJYDZT7K67VZ75HPJVIEUVNIXF47ZG2FB2RMQQVU2HHGCYSC"));

        stellar::Asset poolShare;
        poolShare.type = stellar::AssetType::ASSET_TYPE_POOL_SHARE;
        QVERIFY_EXCEPTION_THROWN(Util::getContractAddressFromAsset(testnet, poolShare),
                                 std::runtime_error);
    }
};

ADD_TEST(UtilTest)
#endif // UTILTEST_H
