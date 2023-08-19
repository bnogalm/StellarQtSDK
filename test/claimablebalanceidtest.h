#ifndef CLAIMABLEBALANCEIDTEST_H
#define CLAIMABLEBALANCEIDTEST_H
#include <QObject>
#include "src/keypair.h"
#include "src/util.h"

#include <QtTest>
#include "testcollector.h"
#include "../src/asset.h"
#include "../src/assettypenative.h"
#include "../src/keypair.h"
#include "../src/transaction.h"
#include "../src/bumpsequenceoperation.h"
#include "../src/account.h"



#include "../src/createclaimablebalanceoperation.h"


class ClaimableBalanceIdTest: public QObject
{
    Q_OBJECT

private slots:

    void initTestCase()
    {
    }
    void cleanupTestCase()
    {
    }

    void testClaimableBalanceIds() {
        Network::useTestNetwork();
        QString sourceAccount = KeyPair::fromSecretSeed(QString("SCZANGBA5YHTNYVVV4C3U252E2B6P6F5T3U6MM63WBSBZATAQI3EBTQ4"))->getAccountId();
        CreateClaimableBalanceOperation* op0 = CreateClaimableBalanceOperation::create(
                    "420",
                    new AssetTypeNative(),
                    QList<Claimant>() << Claimant("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG",new Predicate::Unconditional())
                    );

        Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), new Account(KeyPair::fromAccountId(sourceAccount), 123l), Network::current())
                .addOperation(op0)
                .addOperation(BumpSequenceOperation::create(2l))
                .addOperation(op0)
                .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                .setBaseFee(Transaction::MIN_BASE_FEE)
                .build();

        QString expectedIdIndex0 = "0000000095001252ab3b4d16adbfa5364ce526dfcda03cb2258b827edbb2e0450087be51";
        QString expectedIdIndex2 = "000000009ce17215cc7280e4f4f31fdff4084c9da90ae8236e157acf09d2ce4bf20ac9fb";
        QString expectedIdSeq124 = "000000004cadb397acbcd75f56ff7a623470ecd52d8cf7d1e7bae26271405a1b6233bdfa";

        QCOMPARE(transaction->getClaimableBalanceId(0),expectedIdIndex0);
        // different index changes the claimable balance id
        QCOMPARE(transaction->getClaimableBalanceId(2),expectedIdIndex2);

        CreateClaimableBalanceOperation* opWithSourceAccount = CreateClaimableBalanceOperation::create(
                    "420",
                    new AssetTypeNative(),
                    QList<Claimant>()<< Claimant(
                        "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG",
                        new Predicate::Unconditional()
                        )

                    )->setSourceAccount("GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7");

        transaction = Transaction::Builder(AccountConverter().enableMuxed(), new Account(KeyPair::fromAccountId(sourceAccount), 123l), Network::current())
                .addOperation(opWithSourceAccount)
                .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                .setBaseFee(Transaction::MIN_BASE_FEE)
                .build();

        // operation source account does not affect claimable balance id
        QCOMPARE(transaction->getClaimableBalanceId(0), expectedIdIndex0);

        transaction = Transaction::Builder(AccountConverter().enableMuxed(), new Account(KeyPair::fromAccountId(sourceAccount), 124l), Network::current())
                .addOperation(opWithSourceAccount)
                .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                .setBaseFee(Transaction::MIN_BASE_FEE)
                .build();

        // different sequence number changes the claimable balance id
        QCOMPARE(transaction->getClaimableBalanceId(0), expectedIdSeq124);

        // MuxedAccount muxedAccount = ;
        stellar::MuxedAccount muxedAccount;
        muxedAccount.type = stellar::CryptoKeyType::KEY_TYPE_MUXED_ED25519;
        auto temp = StrKey::encodeToXDRMuxedAccount(sourceAccount);
        memcpy((char*)muxedAccount.med25519.ed25519,(char*)(temp.ed25519), sizeof(temp.ed25519));
        muxedAccount.med25519.id = 41l;


        transaction = Transaction::Builder(AccountConverter().enableMuxed(), new Account(StrKey::encodeStellarMuxedAccount(muxedAccount), 123l), Network::current())
                .addOperation(op0)
                .addOperation(BumpSequenceOperation::create(2))
                .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                .setBaseFee(Transaction::MIN_BASE_FEE)
                .build();


        // muxed id does not affect claimable balance id
        QCOMPARE(transaction->getClaimableBalanceId(0), expectedIdIndex0);

        try {
            transaction->getClaimableBalanceId(4);
            QFAIL("Missing exception");
        } catch (const std::runtime_error& e) {
            QCOMPARE(e.what() , "index is outside the bounds of the operations within this transaction");
        }

        try {
            transaction->getClaimableBalanceId(1);
            QFAIL("Missing exception");
        } catch (const std::runtime_error& e) {
            QCOMPARE(e.what(), "operation at index is not of type CreateClaimableBalanceOperation");
        }
    }

};

ADD_TEST(ClaimableBalanceIdTest)
#endif // CLAIMABLEBALANCEIDTEST_H
