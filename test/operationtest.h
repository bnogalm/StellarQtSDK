#ifndef OPERATIONTEST_H
#define OPERATIONTEST_H
#include <QObject>
#include <QtTest>
#include <limits>
#include "testcollector.h"


#include "src/strkey.h"
#include "src/util.h"
#include "src/operation.h"
#include "src/price.h"

#include "src/createaccountoperation.h"
#include "src/paymentoperation.h"
#include "src/asset.h"
#include "src/assettypenative.h"
#include "src/accountmergeoperation.h"
#include "src/assettypecreditalphanum4.h"
#include "src/assettypecreditalphanum12.h"
#include "src/pathpaymentoperation.h"
#include "src/allowtrustoperation.h"
#include "src/changetrustoperation.h"
#include "src/createpassivesellofferoperation.h"
#include "src/managedataoperation.h"
#include "src/signer.h"
#include "src/setoptionsoperation.h"
#include "src/txrep.h"
#include "src/account.h"
#include "src/network.h"
#include "src/transaction.h"
#include "src/managesellofferoperation.h"
#include "src/managebuyofferoperation.h"
#include "src/bumpsequenceoperation.h"
#include "src/pathpaymentstrictreceiveoperation.h"
#include "src/pathpaymentstrictsendoperation.h"


#include "src/revokeaccountsponsorshipoperation.h"
#include "src/revokedatasponsorshipoperation.h"
#include "src/revokeclaimablebalancesponsorshipoperation.h"
#include "src/revokeliquiditypoolsponsorshipoperation.h"
#include "src/revokeoffersponsorshipoperation.h"
#include "src/revokesignersponsorshipoperation.h"
#include "src/revoketrustlinesponsorshipoperation.h"

#include "src/beginsponsoringfuturereservesoperation.h"
#include "src/endsponsoringfuturereservesoperation.h"

#include "src/claimclaimablebalanceoperation.h"
#include "src/clawbackoperation.h"
#include <functional>
#include "src/createclaimablebalanceoperation.h"
#include "src/liquiditypoolwithdrawoperation.h"
#include "src/clawbackclaimablebalanceoperation.h"
#include "src/settrustlineflagsoperation.h"

class OperationTest: public QObject
{
    Q_OBJECT


private slots:
    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }

    void testConversion(){
        QCOMPARE(Operation::fromXdrAmount(1234567890),QString("123.456789"));
        QCOMPARE(Operation::fromXdrAmount(1234500001),QString("123.4500001"));
        QVERIFY(Operation::toXdrAmount("123.4500001")==1234500001UL);
        QVERIFY(Operation::toXdrAmount("123.45")==1234500000UL);
        QVERIFY(Operation::toXdrAmount("123")==1230000000UL);
    }
    void testCreateAccountOperation(){
        // GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF
        KeyPair *source = KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));
        // GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR
        KeyPair *destination = KeyPair::fromSecretSeed(QString("SDHZGHURAYXKU2KMVHPOXI6JG2Q4BSQUQCEOY72O3QQTCLR2T455PMII"));

        QString startingAmount("1000");
        CreateAccountOperation* operation = new CreateAccountOperation(destination, startingAmount);
        operation->setSourceAccount(source->getAccountId());


        stellar::Operation xdr = operation->toXdr();
        CreateAccountOperation* parsedOperation = static_cast<CreateAccountOperation*>(Operation::fromXdr(xdr));

        QVERIFY(10000000000L==xdr.operationCreateAccount.startingBalance);

        QVERIFY(source->getAccountId()==parsedOperation->getSourceAccount());

        QVERIFY(destination->getAccountId()==parsedOperation->getDestination()->getAccountId());
        QVERIFY(startingAmount==parsedOperation->getStartingBalance());
        QString expected("AAAAAQAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAAAAAAAA7eBSYbzcL5UKo7oXO24y1ckX+XuCtkDsyNHOp1n1bxAAAAACVAvkAA==");

        QVERIFY(expected == operation->toXdrBase64());



    }


    void testPaymentOperation() {
        // GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF
        KeyPair* source = KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));
        // GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR
        KeyPair* destination = KeyPair::fromSecretSeed(QString("SDHZGHURAYXKU2KMVHPOXI6JG2Q4BSQUQCEOY72O3QQTCLR2T455PMII"));

        Asset* asset = new AssetTypeNative();
        QString amount = "1000";

        Operation* operation = PaymentOperation::create(destination, asset, amount)->setSourceAccount(source->getAccountId());

        stellar::Operation xdr = operation->toXdr();


        PaymentOperation* parsedOperation = static_cast<PaymentOperation*>(Operation::fromXdr(xdr));

        QVERIFY(10000000000L==xdr.operationPayment.amount);
        QVERIFY(source->getAccountId()==parsedOperation->getSourceAccount());
        //QVERIFY(destination->getAccountId()== parsedOperation->getDestination());
        QCOMPARE(parsedOperation->getDestination(),destination->getAccountId());
        QVERIFY(dynamic_cast<AssetTypeNative*>(parsedOperation->getAsset()));
        QVERIFY(amount == parsedOperation->getAmount());
        QString expected("AAAAAQAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAAEAAAAA7eBSYbzcL5UKo7oXO24y1ckX+XuCtkDsyNHOp1n1bxAAAAAAAAAAAlQL5AA=");
        //qDebug() << QByteArray::fromBase64(expected.toLatin1()).toHex();
        //qDebug() << QByteArray::fromBase64(operation->toXdrBase64().toLatin1()).toHex();

        QVERIFY(expected == operation->toXdrBase64());
    }

    void testPathPaymentOperation(){
        // GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF
        KeyPair* source = KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));
        // GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR
        KeyPair* destination = KeyPair::fromSecretSeed(QString("SDHZGHURAYXKU2KMVHPOXI6JG2Q4BSQUQCEOY72O3QQTCLR2T455PMII"));
        // GCGZLB3X2B3UFOFSHHQ6ZGEPEX7XYPEH6SBFMIV74EUDOFZJA3VNL6X4
        KeyPair* issuer = KeyPair::fromSecretSeed(QString("SBOBVZUN6WKVMI6KIL2GHBBEETEV6XKQGILITNH6LO6ZA22DBMSDCPAG"));

        // GAVAQKT2M7B4V3NN7RNNXPU5CWNDKC27MYHKLF5UNYXH4FNLFVDXKRSV
        KeyPair* pathIssuer1 = KeyPair::fromSecretSeed(QString("SALDLG5XU5AEJWUOHAJPSC4HJ2IK3Z6BXXP4GWRHFT7P7ILSCFFQ7TC5"));
        // GBCP5W2VS7AEWV2HFRN7YYC623LTSV7VSTGIHFXDEJU7S5BAGVCSETRR
        KeyPair* pathIssuer2 = KeyPair::fromSecretSeed(QString("SA64U7C5C7BS5IHWEPA7YWFN3Z6FE5L6KAMYUIT4AQ7KVTVLD23C6HEZ"));

        Asset* sendAsset = new AssetTypeNative();
        QString sendMax = "0.0001";
        Asset* destAsset = new AssetTypeCreditAlphaNum4("USD", issuer);
        QString destAmount = "0.0001";
        QList<Asset*> path;
        path.append(new AssetTypeCreditAlphaNum4("USD", pathIssuer1));
        path.append( new AssetTypeCreditAlphaNum12("TESTTEST", pathIssuer2));


        PathPaymentOperation* operation = new PathPaymentOperation(
                    sendAsset, sendMax, destination->getAccountId(), destAsset, destAmount, path);
        operation->setSourceAccount(source->getAccountId());



        auto xdr = operation->toXdr();
        PathPaymentOperation* parsedOperation = static_cast<PathPaymentOperation*>(Operation::fromXdr(xdr));

        QVERIFY(1000L== xdr.operationPathPaymentStrictReceive.sendMax);
        QVERIFY(1000L== xdr.operationPathPaymentStrictReceive.destAmount);
        QVERIFY(dynamic_cast<AssetTypeNative*>(parsedOperation->getSendAsset()));
        QCOMPARE(source->getAccountId(), parsedOperation->getSourceAccount());
        QCOMPARE(destination->getAccountId(), parsedOperation->getDestination());
        QCOMPARE(sendMax, parsedOperation->getSendMax());
        QVERIFY(dynamic_cast<AssetTypeCreditAlphaNum4*>(parsedOperation->getDestAsset()));
        QCOMPARE(destAmount, parsedOperation->getDestAmount());
        QCOMPARE(path.length(), parsedOperation->getPath().length());

        QString expected = QString("AAAAAQAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAAIAAAAAAAAAAAAAA+gAAAAA7eBSYbzcL5UKo7oXO24y1ckX+XuCtkDsyNHOp1n1bxAAAAABVVNEAAAAAACNlYd30HdCuLI54eyYjyX/fDyH9IJWIr/hKDcXKQbq1QAAAAAAAAPoAAAAAgAAAAFVU0QAAAAAACoIKnpnw8rtrfxa276dFZo1C19mDqWXtG4ufhWrLUd1AAAAAlRFU1RURVNUAAAAAAAAAABE/ttVl8BLV0csW/xgXtbXOVf1lMyDluMiafl0IDVFIg==");

        QVERIFY(expected==operation->toXdrBase64());
    }

    void testPathPaymentEmptyPathOperation() {
        // GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF
        KeyPair* source = KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));
        // GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR
        KeyPair* destination = KeyPair::fromSecretSeed(QString("SDHZGHURAYXKU2KMVHPOXI6JG2Q4BSQUQCEOY72O3QQTCLR2T455PMII"));
        // GCGZLB3X2B3UFOFSHHQ6ZGEPEX7XYPEH6SBFMIV74EUDOFZJA3VNL6X4
        KeyPair* issuer = KeyPair::fromSecretSeed(QString("SBOBVZUN6WKVMI6KIL2GHBBEETEV6XKQGILITNH6LO6ZA22DBMSDCPAG"));


        Asset* sendAsset = new AssetTypeNative();
        QString sendMax = "0.0001";
        Asset* destAsset = new AssetTypeCreditAlphaNum4("USD", issuer);
        QString destAmount = "0.0001";

        PathPaymentOperation* operation = new PathPaymentOperation(
                    sendAsset, sendMax, destination->getAccountId(), destAsset, destAmount);
        operation->setSourceAccount(source->getAccountId());

        auto xdr = operation->toXdr();
        PathPaymentOperation* parsedOperation = static_cast<PathPaymentOperation*>(Operation::fromXdr(xdr));

        QVERIFY(1000L== xdr.operationPathPaymentStrictReceive.sendMax);

        QVERIFY(1000L== xdr.operationPathPaymentStrictReceive.destAmount);
        QVERIFY(dynamic_cast<AssetTypeNative*>(parsedOperation->getSendAsset()));
        QCOMPARE(source->getAccountId(), parsedOperation->getSourceAccount());
        QCOMPARE(destination->getAccountId(), parsedOperation->getDestination());
        QCOMPARE(sendMax, parsedOperation->getSendMax());
        QVERIFY(dynamic_cast<AssetTypeCreditAlphaNum4*>(parsedOperation->getDestAsset()));
        QCOMPARE(destAmount, parsedOperation->getDestAmount());
        QCOMPARE(0, parsedOperation->getPath().size());

        QCOMPARE(operation->toXdrBase64(),
                 QString("AAAAAQAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAAIAAAAAAAAAAAAAA+gAAAAA7eBSYbzcL5UKo7oXO24y1ckX+XuCtkDsyNHOp1n1bxAAAAABVVNEAAAAAACNlYd30HdCuLI54eyYjyX/fDyH9IJWIr/hKDcXKQbq1QAAAAAAAAPoAAAAAA=="));
    }

    void testPathPaymentStrictSendOperation()  {
        // GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF
        KeyPair* source = KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));
        // GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR
        KeyPair* destination = KeyPair::fromSecretSeed(QString("SDHZGHURAYXKU2KMVHPOXI6JG2Q4BSQUQCEOY72O3QQTCLR2T455PMII"));
        // GCGZLB3X2B3UFOFSHHQ6ZGEPEX7XYPEH6SBFMIV74EUDOFZJA3VNL6X4
        KeyPair* issuer = KeyPair::fromSecretSeed(QString("SBOBVZUN6WKVMI6KIL2GHBBEETEV6XKQGILITNH6LO6ZA22DBMSDCPAG"));

        // GAVAQKT2M7B4V3NN7RNNXPU5CWNDKC27MYHKLF5UNYXH4FNLFVDXKRSV
        KeyPair* pathIssuer1 = KeyPair::fromSecretSeed(QString("SALDLG5XU5AEJWUOHAJPSC4HJ2IK3Z6BXXP4GWRHFT7P7ILSCFFQ7TC5"));
        // GBCP5W2VS7AEWV2HFRN7YYC623LTSV7VSTGIHFXDEJU7S5BAGVCSETRR
        KeyPair* pathIssuer2 = KeyPair::fromSecretSeed(QString("SA64U7C5C7BS5IHWEPA7YWFN3Z6FE5L6KAMYUIT4AQ7KVTVLD23C6HEZ"));

        Asset* sendAsset = new AssetTypeNative();
        QString sendAmount = "0.0001";
        Asset* destAsset = new AssetTypeCreditAlphaNum4("USD", issuer);
        QString destMin = "0.0009";
        QList<Asset*> path = {new AssetTypeCreditAlphaNum4("USD", pathIssuer1), new AssetTypeCreditAlphaNum12("TESTTEST", pathIssuer2)};

        PathPaymentStrictSendOperation* operation = PathPaymentStrictSendOperation::create(
                    sendAsset, sendAmount, destination, destAsset, destMin)
                ->setPath(path)
                ->setSourceAccount(source->getAccountId());

        auto xdr = operation->toXdr();
        PathPaymentStrictSendOperation* parsedOperation = static_cast<PathPaymentStrictSendOperation*>(Operation::fromXdr(xdr));

        QCOMPARE(xdr.operationPathPaymentStrictSend.sendAmount,1000L);
        QCOMPARE(xdr.operationPathPaymentStrictSend.destMin,9000L);
        QVERIFY(dynamic_cast<AssetTypeNative*>(parsedOperation->getSendAsset()));
        QCOMPARE(source->getAccountId(), parsedOperation->getSourceAccount());
        QCOMPARE(destination->getAccountId(), parsedOperation->getDestination());
        QCOMPARE(sendAmount, parsedOperation->getSendAmount());
        QVERIFY(dynamic_cast<AssetTypeCreditAlphaNum4*>(parsedOperation->getDestAsset()));
        QCOMPARE(destMin, parsedOperation->getDestMin());
        QVERIFY((path.size() == parsedOperation->getPath().size()) && std::equal(path.begin(),path.end(),parsedOperation->getPath().begin(),[](Asset* a, Asset* b){return a->equals(b);}));

        QCOMPARE(operation->toXdrBase64(),QString("AAAAAQAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAA0AAAAAAAAAAAAAA+gAAAAA7eBSYbzcL5UKo7oXO24y1ckX+XuCtkDsyNHOp1n1bxAAAAABVVNEAAAAAACNlYd30HdCuLI54eyYjyX/fDyH9IJWIr/hKDcXKQbq1QAAAAAAACMoAAAAAgAAAAFVU0QAAAAAACoIKnpnw8rtrfxa276dFZo1C19mDqWXtG4ufhWrLUd1AAAAAlRFU1RURVNUAAAAAAAAAABE/ttVl8BLV0csW/xgXtbXOVf1lMyDluMiafl0IDVFIg=="));
    }

    void testPathPaymentStrictSendEmptyPathOperation()
    {
        // GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF
        KeyPair* source = KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));
        // GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR
        KeyPair* destination = KeyPair::fromSecretSeed(QString("SDHZGHURAYXKU2KMVHPOXI6JG2Q4BSQUQCEOY72O3QQTCLR2T455PMII"));
        // GCGZLB3X2B3UFOFSHHQ6ZGEPEX7XYPEH6SBFMIV74EUDOFZJA3VNL6X4
        KeyPair* issuer = KeyPair::fromSecretSeed(QString("SBOBVZUN6WKVMI6KIL2GHBBEETEV6XKQGILITNH6LO6ZA22DBMSDCPAG"));

        //         // GAVAQKT2M7B4V3NN7RNNXPU5CWNDKC27MYHKLF5UNYXH4FNLFVDXKRSV
        //         KeyPair* pathIssuer1 = KeyPair::fromSecretSeed(QString("SALDLG5XU5AEJWUOHAJPSC4HJ2IK3Z6BXXP4GWRHFT7P7ILSCFFQ7TC5"));
        //         // GBCP5W2VS7AEWV2HFRN7YYC623LTSV7VSTGIHFXDEJU7S5BAGVCSETRR
        //         KeyPair* pathIssuer2 = KeyPair::fromSecretSeed(QString("SA64U7C5C7BS5IHWEPA7YWFN3Z6FE5L6KAMYUIT4AQ7KVTVLD23C6HEZ"));

        Asset* sendAsset = new AssetTypeNative();
        QString sendAmount = "0.0001";
        Asset* destAsset = new AssetTypeCreditAlphaNum4("USD", issuer);
        QString destMin = "0.0009";

        PathPaymentStrictSendOperation* operation = PathPaymentStrictSendOperation::create(
                    sendAsset, sendAmount, destination, destAsset, destMin)->setSourceAccount(source->getAccountId());

        auto xdr = operation->toXdr();
        PathPaymentStrictSendOperation* parsedOperation = static_cast<PathPaymentStrictSendOperation*>(Operation::fromXdr(xdr));

        QCOMPARE(xdr.operationPathPaymentStrictSend.sendAmount,1000L);
        QCOMPARE(xdr.operationPathPaymentStrictSend.destMin,9000L);
        QVERIFY(dynamic_cast<AssetTypeNative*>(parsedOperation->getSendAsset()));
        QCOMPARE(source->getAccountId(), parsedOperation->getSourceAccount());
        QCOMPARE(destination->getAccountId(), parsedOperation->getDestination());
        QCOMPARE(sendAmount, parsedOperation->getSendAmount());
        QVERIFY(dynamic_cast<AssetTypeCreditAlphaNum4*>(parsedOperation->getDestAsset()));
        QCOMPARE(destMin, parsedOperation->getDestMin());
        QCOMPARE(parsedOperation->getPath().length(), 0);

        QCOMPARE(operation->toXdrBase64(),QString("AAAAAQAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAA0AAAAAAAAAAAAAA+gAAAAA7eBSYbzcL5UKo7oXO24y1ckX+XuCtkDsyNHOp1n1bxAAAAABVVNEAAAAAACNlYd30HdCuLI54eyYjyX/fDyH9IJWIr/hKDcXKQbq1QAAAAAAACMoAAAAAA=="));
    }

    void testChangeTrustOperation() {
        // GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF
        KeyPair* source = KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));

        Asset* asset = new AssetTypeNative();
        QString limit = "922337203685.4775807";
        QCOMPARE(QString(ChangeTrustOperation::MAX_LIMIT), limit);

        ChangeTrustOperation* operation = new ChangeTrustOperation(asset, limit);
        operation->setSourceAccount(source->getAccountId());

        auto xdr = operation->toXdr();
        ChangeTrustOperation* parsedOperation = static_cast<ChangeTrustOperation*>(Operation::fromXdr(xdr));

        QCOMPARE(9223372036854775807L, xdr.operationChangeTrust.limit);
        QCOMPARE(source->getAccountId(), parsedOperation->getSourceAccount());
        QVERIFY(dynamic_cast<AssetTypeNative*>(parsedOperation->getAsset()));
        QCOMPARE(limit, parsedOperation->getLimit());

        QCOMPARE(operation->toXdrBase64(),
                 QString("AAAAAQAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAAYAAAAAf/////////8="));
    }


    // AllowTrustOperation is deprecated; these two tests exercise it on purpose.
#if defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable: 4996)
#elif defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
    void testAllowTrustOperation(){
        // GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF
        KeyPair* source = KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));
        // GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR
        KeyPair* trustor = KeyPair::fromSecretSeed(QString("SDHZGHURAYXKU2KMVHPOXI6JG2Q4BSQUQCEOY72O3QQTCLR2T455PMII"));

        QString assetCode = "USDA";
        bool authorize = true;

        AllowTrustOperation operation(trustor, assetCode, authorize, false);
        operation.setSourceAccount(source->getAccountId());


        auto xdr = operation.toXdr();
        AllowTrustOperation* parsedOperation = static_cast<AllowTrustOperation*>(Operation::fromXdr(xdr));

        QCOMPARE(source->getAccountId(), parsedOperation->getSourceAccount());
        QCOMPARE(trustor->getAccountId(), parsedOperation->getTrustor()->getAccountId());
        QCOMPARE(assetCode, parsedOperation->getAssetCode());
        QCOMPARE(authorize, parsedOperation->getAuthorize());
        QCOMPARE(operation.toXdrBase64(),
                 QString("AAAAAQAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAAcAAAAA7eBSYbzcL5UKo7oXO24y1ckX+XuCtkDsyNHOp1n1bxAAAAABVVNEQQAAAAE=")
                 );
    }

    void testAllowTrustOperationAssetCodeBuffer() {
        // GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF
        KeyPair* source = KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));
        // GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR
        KeyPair* trustor = KeyPair::fromSecretSeed(QString("SDHZGHURAYXKU2KMVHPOXI6JG2Q4BSQUQCEOY72O3QQTCLR2T455PMII"));

        QString assetCode = "USDABC";
        bool authorize = true;

        AllowTrustOperation* operation = AllowTrustOperation::create(trustor, assetCode, authorize)
                ->setSourceAccount(source->getAccountId());

        auto xdr = operation->toXdr();
        AllowTrustOperation* parsedOperation = static_cast<AllowTrustOperation*>(Operation::fromXdr(xdr));

        QCOMPARE(assetCode, parsedOperation->getAssetCode());
    }
#if defined(_MSC_VER)
#  pragma warning(pop)
#elif defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic pop
#endif

    void testSetOptionsOperation(){
        // GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF
        KeyPair* source = KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));
        // GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR
        KeyPair* inflationDestination = KeyPair::fromSecretSeed(QString("SDHZGHURAYXKU2KMVHPOXI6JG2Q4BSQUQCEOY72O3QQTCLR2T455PMII"));
        // GBCP5W2VS7AEWV2HFRN7YYC623LTSV7VSTGIHFXDEJU7S5BAGVCSETRR
        auto signer = Signer::ed25519PublicKey(KeyPair::fromSecretSeed(QString("SA64U7C5C7BS5IHWEPA7YWFN3Z6FE5L6KAMYUIT4AQ7KVTVLD23C6HEZ")));

        Integer clearFlags = 1;
        Integer setFlags = 1;
        Integer masterKeyWeight = 1;
        Integer lowThreshold = 2;
        Integer mediumThreshold = 3;
        Integer highThreshold = 4;
        QString homeDomain = "stellar.org";
        Integer signerWeight = 1;

        SetOptionsOperation* operation = SetOptionsOperation::create()
                ->setInflationDestination(inflationDestination)
                ->setClearFlags(clearFlags)
                ->setSetFlags(setFlags)
                ->setMasterKeyWeight(masterKeyWeight)
                ->setLowThreshold(lowThreshold)
                ->setMediumThreshold(mediumThreshold)
                ->setHighThreshold(highThreshold)
                ->setHomeDomain(homeDomain)
                ->setSigner(signer, signerWeight)
                ->setSourceAccount(source->getAccountId());

        auto xdr = operation->toXdr();
        SetOptionsOperation* parsedOperation = static_cast<SetOptionsOperation*>(SetOptionsOperation::fromXdr(xdr));

        QCOMPARE(inflationDestination->getAccountId(), parsedOperation->getInflationDestination()->getAccountId());
        QCOMPARE(clearFlags, parsedOperation->getClearFlags());
        QCOMPARE(setFlags, parsedOperation->getSetFlags());
        QCOMPARE(masterKeyWeight, parsedOperation->getMasterKeyWeight());
        QCOMPARE(lowThreshold, parsedOperation->getLowThreshold());
        QCOMPARE(mediumThreshold, parsedOperation->getMediumThreshold());
        QCOMPARE(highThreshold, parsedOperation->getHighThreshold());
        QCOMPARE(homeDomain, parsedOperation->getHomeDomain());
        QCOMPARE(signer.type, parsedOperation->getSigner()->type);
        QVERIFY(memcmp(signer.ed25519, parsedOperation->getSigner()->ed25519,sizeof(signer.ed25519))==0);
        QCOMPARE(signerWeight, parsedOperation->getSignerWeight());
        QCOMPARE(source->getAccountId(), parsedOperation->getSourceAccount());

        QCOMPARE(operation->toXdrBase64()
                 ,QString("AAAAAQAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAAUAAAABAAAAAO3gUmG83C+VCqO6FztuMtXJF/l7grZA7MjRzqdZ9W8QAAAAAQAAAAEAAAABAAAAAQAAAAEAAAABAAAAAQAAAAIAAAABAAAAAwAAAAEAAAAEAAAAAQAAAAtzdGVsbGFyLm9yZwAAAAABAAAAAET+21WXwEtXRyxb/GBe1tc5V/WUzIOW4yJp+XQgNUUiAAAAAQ=="));
    }


    void testSetOptionsOperationSingleField() {
        // GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF
        KeyPair* source = KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));

        QString homeDomain = "stellar.org";

        SetOptionsOperation* operation = SetOptionsOperation::create()
                ->setHomeDomain(homeDomain)
                ->setSourceAccount(source->getAccountId());

        auto xdr = operation->toXdr();
        SetOptionsOperation* parsedOperation = static_cast<SetOptionsOperation*>(SetOptionsOperation::fromXdr(xdr));

        QVERIFY(parsedOperation->getInflationDestination()==nullptr);
        QVERIFY(parsedOperation->getClearFlags().isNull());
        QVERIFY(parsedOperation->getSetFlags().isNull());
        QVERIFY(parsedOperation->getMasterKeyWeight().isNull());
        QVERIFY(parsedOperation->getLowThreshold().isNull());
        QVERIFY(parsedOperation->getMediumThreshold().isNull());
        QVERIFY(parsedOperation->getHighThreshold().isNull());
        QCOMPARE(homeDomain, parsedOperation->getHomeDomain());
        QVERIFY(parsedOperation->getSigner()==nullptr);
        QVERIFY(parsedOperation->getSignerWeight().isNull());
        QCOMPARE(source->getAccountId(), parsedOperation->getSourceAccount());

        QCOMPARE(operation->toXdrBase64()
                 ,QString("AAAAAQAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAAUAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAQAAAAtzdGVsbGFyLm9yZwAAAAAA"));
    }


    void testSetOptionsOperationSignerSha256() {
        // GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF
        KeyPair* source = KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));

        QByteArray preimage = "stellar.org";
        QByteArray hash = Util::hash(preimage);

        SetOptionsOperation* operation = SetOptionsOperation::create()
                ->setSigner(Signer::sha256Hash(hash), 10)
                ->setSourceAccount(source->getAccountId());

        auto xdr = operation->toXdr();

        SetOptionsOperation* parsedOperation = static_cast<SetOptionsOperation*>(SetOptionsOperation::fromXdr(xdr));

        QVERIFY(!parsedOperation->getInflationDestination());

        QVERIFY(parsedOperation->getClearFlags().isNull());

        QVERIFY(parsedOperation->getSetFlags().isNull());
        QVERIFY(parsedOperation->getMasterKeyWeight().isNull());
        QVERIFY(parsedOperation->getLowThreshold().isNull());
        QVERIFY(parsedOperation->getMediumThreshold().isNull());
        QVERIFY(parsedOperation->getHighThreshold().isNull());
        QVERIFY(parsedOperation->getHomeDomain().isNull());
        QVERIFY(memcmp(hash.data(), parsedOperation->getSigner()->hashX,static_cast<size_t>(hash.size()))==0);
        QCOMPARE(10, static_cast<int>(parsedOperation->getSignerWeight()));
        QCOMPARE(source->getAccountId(), parsedOperation->getSourceAccount());

        QCOMPARE(operation->toXdrBase64()
                 ,QString("AAAAAQAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAAUAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEAAAACbpRqMkaQAfCYSk/n3xIl4fCoHfKqxF34ht2iuvSYEJQAAAAK"));
    }

    void testSetOptionsOperationPreAuthTxSigner() {
        Network::useTestNetwork();
        // GBPMKIRA2OQW2XZZQUCQILI5TMVZ6JNRKM423BSAISDM7ZFWQ6KWEBC4
        KeyPair* source = KeyPair::fromSecretSeed(QString("SCH27VUZZ6UAKB67BDNF6FA42YMBMQCBKXWGMFD5TZ6S5ZZCZFLRXKHS"));
        KeyPair* destination = KeyPair::fromAccountId(QString("GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR"));

        qint64 sequenceNumber = 2908908335136768L;
        Account* account = new Account(source, sequenceNumber);


        Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), account).addOperation(new CreateAccountOperation(destination,"2000"))
                .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                .setBaseFee(Transaction::Builder::BASE_FEE)
                .build();


        // GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF
        KeyPair* opSource = KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));

        SetOptionsOperation* operation = SetOptionsOperation::create()
                ->setSigner(Signer::preAuthTx(transaction), 10)
                ->setSourceAccount(opSource->getAccountId());

        auto xdr = operation->toXdr();
        SetOptionsOperation* parsedOperation = static_cast<SetOptionsOperation*>(SetOptionsOperation::fromXdr(xdr));

        QVERIFY(!parsedOperation->getInflationDestination());
        QVERIFY(parsedOperation->getClearFlags().isNull());
        QVERIFY(parsedOperation->getSetFlags().isNull());
        QVERIFY(parsedOperation->getMasterKeyWeight().isNull());
        QVERIFY(parsedOperation->getLowThreshold().isNull());
        QVERIFY(parsedOperation->getMediumThreshold().isNull());
        QVERIFY(parsedOperation->getHighThreshold().isNull());
        QVERIFY(parsedOperation->getHomeDomain().isNull());
        QVERIFY(memcmp(transaction->hash().data(), parsedOperation->getSigner()->preAuthTx,sizeof(parsedOperation->getSigner()->preAuthTx))==0);
        QCOMPARE(Integer(10), parsedOperation->getSignerWeight());
        QCOMPARE(opSource->getAccountId(), parsedOperation->getSourceAccount());

        // The pre-auth hash of a TIMEOUT_INFINITE transaction, now time bounds
        // {0, 0} as in js and java (py-stellar-sdk 14.1.0 gives these bytes).
        QCOMPARE(operation->toXdrBase64()
                 ,QString("AAAAAQAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAAUAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEAAAABD3LjpEQU2FIuahtjHnoJvAvRwe4U9DXFHaM/DdSpzHAAAAAK"));
    }

    /**
     * The full SetOptions constructor passed the home domain through
     * QUrl::toAce: "Example.COM" was signed as "example.com", and what Qt
     * takes for no host name ("stellar.org/", "a:b", "x..y") as an EMPTY home
     * domain, which clears the account's. A null QString cleared it too. It is
     * signed now as given, like setHomeDomain() and js/py.
     */
    void testSetOptionsCtorSignsTheHomeDomainAsGiven() {
        const stellar::SignerKey signer =
            Signer::ed25519PublicKey(KeyPair::fromAccountId(QString("GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR")));
        auto ctor = [&](const QString& domain) {
            return SetOptionsOperation(nullptr, Integer(), Integer(), Integer(), Integer(),
                                       Integer(), Integer(), domain, signer, 1).getHomeDomainBytes();
        };
        for (const QString& domain : {QString("stellar.org/"), QString("Example.COM"), QString("a:b"),
                                      QString("x..y"), QString("-bad-.com"),
                                      QString::fromUtf8("m\xc3\xbcnchen.de")}) {
            QScopedPointer<SetOptionsOperation> setter(SetOptionsOperation::create());
            setter->setHomeDomain(domain);
            QCOMPARE(ctor(domain), domain.toUtf8());
            QCOMPARE(ctor(domain), setter->getHomeDomainBytes());
        }
        QVERIFY(ctor(QString()).isNull());                  // unchanged
        QVERIFY(!ctor(QString("")).isNull());               // cleared
        QCOMPARE(ctor(QString("")), QByteArray(""));
        QVERIFY_EXCEPTION_THROWN(ctor(QString(33, QLatin1Char('a'))), std::runtime_error);
        QCOMPARE(ctor(QString(32, QLatin1Char('a'))), QByteArray(32, 'a'));
    }

    /**
     * The C++ constructors took 0 for amounts js-stellar-base and core
     * require above zero (payment, path payments, passive offer, claimable
     * balance, clawback, pool withdrawal): 0 was signed, and the network
     * refused the operation as malformed. Where 0 means something (a new
     * account's balance, deleting an offer, removing a trustline) it stays.
     */
    void testZeroIsRefusedWhereJsAndCoreRefuseIt()
    {
        const QString dest = QStringLiteral("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO");
        const QString issuer = QStringLiteral("GAJZR5RMNUNEK7CRXJVEWXZ5XUXWT7FJGILCDDOITF7EC26RPWJ4UVOE");
        auto usd = [&]() { return Asset::createNonNativeAsset(QStringLiteral("USD"), issuer); };
        auto refuses = [](const std::function<Operation*()>& make) {
            try { delete make(); } catch (const std::exception&) { return true; }
            return false;
        };
        for (const QString& zero : {QStringLiteral("0"), QStringLiteral("0.0000000"), QStringLiteral("-0")}) {
            QVERIFY(refuses([&]() { return PaymentOperation::create(dest, new AssetTypeNative(), zero); }));
            QVERIFY(refuses([&]() { return PathPaymentStrictReceiveOperation::create(new AssetTypeNative(), zero, dest, usd(), QStringLiteral("1")); }));
            QVERIFY(refuses([&]() { return PathPaymentStrictReceiveOperation::create(new AssetTypeNative(), QStringLiteral("1"), dest, usd(), zero); }));
            QVERIFY(refuses([&]() { return PathPaymentStrictSendOperation::create(new AssetTypeNative(), zero, dest, usd(), QStringLiteral("1")); }));
            QVERIFY(refuses([&]() { return PathPaymentStrictSendOperation::create(new AssetTypeNative(), QStringLiteral("1"), dest, usd(), zero); }));
            QVERIFY(refuses([&]() { return CreatePassiveSellOfferOperation::create(new AssetTypeNative(), usd(), zero, QStringLiteral("1")); }));
            QVERIFY(refuses([&]() { return ClawbackOperation::create(dest, usd(), zero); }));
            QVERIFY(refuses([&]() {
                QList<Claimant> claimants;
                claimants.append(Claimant(dest, new Predicate::Unconditional()));
                return CreateClaimableBalanceOperation::create(zero, new AssetTypeNative(), claimants);
            }));
            QVERIFY(refuses([&]() { return LiquidityPoolWithdrawOperation::create(QByteArray(32, '\x07'), zero, QStringLiteral("0"), QStringLiteral("0")); }));
        }
        // 0 where it is an instruction or a valid value, as js allows it.
        QVERIFY(!refuses([&]() { return CreateAccountOperation::create(dest, QStringLiteral("0")); }));
        QVERIFY(!refuses([&]() { return new ManageSellOfferOperation(new AssetTypeNative(), usd(), QStringLiteral("0"), Price(1, 1), 12345); }));
        QVERIFY(!refuses([&]() { return new ChangeTrustOperation(usd(), QStringLiteral("0")); }));
        QVERIFY(!refuses([&]() { return LiquidityPoolWithdrawOperation::create(QByteArray(32, '\x07'), QStringLiteral("1"), QStringLiteral("0"), QStringLiteral("0")); }));
        QVERIFY(!refuses([&]() { return PaymentOperation::create(dest, new AssetTypeNative(), QStringLiteral("0.0000001")); }));
    }

    /**
     * Amounts in the decimal forms js-stellar-base (BigNumber) and py
     * (Decimal) both read: ".5", "5.", "+1", "1e2", "5E-1". Only
     * [-]digits[.digits] was read, so these were refused where js and py sign
     * them; the stroops are js-stellar-sdk 17.1.0's. Still exact: more than
     * seven decimals, out of range, a comma, or js's hex and binary forms
     * (py refuses those) are refused.
     */
    void testAmountsInEveryDecimalFormJsAndPyRead()
    {
        const QList<QPair<QString, qint64>> read = {
            {QStringLiteral(".5"), 5000000}, {QStringLiteral("5."), 50000000}, {QStringLiteral("+1"), 10000000}, {QStringLiteral("1e2"), 1000000000},
            {QStringLiteral("1E-7"), 1}, {QStringLiteral("5e-1"), 5000000}, {QStringLiteral("1.5e1"), 150000000}, {QStringLiteral("1.23456789e1"), 123456789},
            {QStringLiteral("9.223372036854775807e11"), Q_INT64_C(9223372036854775807)}, {QStringLiteral("00012.5000000"), 125000000},
            {QStringLiteral("0.00000000"), 0}, {QStringLiteral("-0"), 0}, {QStringLiteral("0e99"), 0}, {QStringLiteral("-1.5"), -15000000}, {QStringLiteral("1.000000000e0"), 10000000},
            {QStringLiteral("12345678901234567890e-8"), Q_INT64_C(1234567890123456789)},
        };
        for (const auto& pair : read)
            QCOMPARE(Operation::toXdrAmount(pair.first), pair.second);
        for (const QString& bad : {QStringLiteral("1e-8"), QStringLiteral("922337203685.4775808"), QStringLiteral("1e1000"),
                                   QStringLiteral("0x10"), QStringLiteral("0b11"), QStringLiteral("1,5"), QStringLiteral("."),
                                   QStringLiteral("e5"), QStringLiteral("1e"), QStringLiteral("+"), QStringLiteral(""),
                                   QStringLiteral("1.5.1"), QStringLiteral("--1"), QStringLiteral("1e+"), QStringLiteral("inf")}) {
            bool threw = false;
            try { Operation::toXdrAmount(bad); } catch (const std::exception&) { threw = true; }
            QVERIFY2(threw, qPrintable(bad));
        }
    }

    /**
     * Refused on purpose, as the header says: digits grouped with '_', which
     * js and py both sign ("1_0" as 100000000 stroops, "1_000.5" as
     * 10005000000 in js-stellar-sdk 17.1.0 and py-stellar-sdk 14.1.0, which
     * disagree on "_1" and "1__0"), and js's radix forms (py refuses them;
     * js signs "0o7" as 7). Easy to misread in an amount about to be signed.
     */
    void testAmountsWithUnderscoresOrARadixPrefixAreRefused()
    {
        for (const QString& bad : {QStringLiteral("1_0"), QStringLiteral("1_000.5"), QStringLiteral("_1"),
                                   QStringLiteral("1__0"), QStringLiteral("1_"), QStringLiteral("1e1_0"),
                                   QStringLiteral("0x10"), QStringLiteral("0X10"), QStringLiteral("0b11"),
                                   QStringLiteral("0o7")}) {
            bool threw = false;
            try { Operation::toXdrAmount(bad); } catch (const std::exception&) { threw = true; }
            QVERIFY2(threw, qPrintable(bad));
        }
    }

    /**
     * A decoded PATH_PAYMENT_STRICT_RECEIVE was the deprecated
     * PathPaymentOperation, a separate copy of PathPaymentStrictReceiveOperation:
     * dynamic_cast<PathPaymentStrictReceiveOperation*> gave nullptr for every
     * one. It is a PathPaymentStrictReceiveOperation now, and still a
     * PathPaymentOperation for 1.0.x callers.
     */
    void testDecodedStrictReceiveIsAStrictReceiveOperation()
    {
        const QString dest = QStringLiteral("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO");
        QScopedPointer<PathPaymentStrictReceiveOperation> original(PathPaymentStrictReceiveOperation::create(
            new AssetTypeNative(), QStringLiteral("10"), dest,
            Asset::createNonNativeAsset(QStringLiteral("USD"), QStringLiteral("GAJZR5RMNUNEK7CRXJVEWXZ5XUXWT7FJGILCDDOITF7EC26RPWJ4UVOE")),
            QStringLiteral("5")));
        stellar::Operation xdr = original->toXdr();
        QScopedPointer<Operation> decoded(Operation::fromXdr(xdr));
        auto* strict = dynamic_cast<PathPaymentStrictReceiveOperation*>(decoded.data());
        QVERIFY(strict != nullptr);
        QVERIFY(dynamic_cast<PathPaymentOperation*>(decoded.data()) != nullptr);
        QCOMPARE(strict->getSendMax(), QStringLiteral("10"));
        QCOMPARE(strict->getDestAmount(), QStringLiteral("5"));
        QCOMPARE(strict->getDestination(), dest);
        QCOMPARE(decoded->toXdrBase64(), original->toXdrBase64());
    }

    /**
     * getHomeDomain went through QUrl::fromAce: "Example.COM" (which core
     * takes, and Horizon shows) read as "example.com", an xn-- name as its
     * Unicode form, and UTF-8 garbled. It is the signed text now, as js and
     * py read it.
     */
    void testSetOptionsHomeDomainReadsAsSigned() {
        for (const QString& domain : {QStringLiteral("Example.COM"), QStringLiteral("xn--and-6ma2c.es"),
                                      QString::fromUtf8("\xc3\xb1" "and\xc3\xba" ".es"), QStringLiteral("STELLAR.ORG")}) {
            QScopedPointer<SetOptionsOperation> op(SetOptionsOperation::create());
            op->setHomeDomain(domain);
            stellar::Operation xdr = op->toXdr();
            QScopedPointer<Operation> decoded(Operation::fromXdr(xdr));
            QCOMPARE(dynamic_cast<SetOptionsOperation*>(decoded.data())->getHomeDomain(), domain);
            QCOMPARE(op->getHomeDomain(), domain);
        }
        QScopedPointer<SetOptionsOperation> cleared(SetOptionsOperation::create());
        cleared->setHomeDomain(QStringLiteral(""));
        QVERIFY(!cleared->getHomeDomain().isNull());
        QVERIFY(cleared->getHomeDomain().isEmpty());
        QScopedPointer<SetOptionsOperation> unset(SetOptionsOperation::create());
        QVERIFY(unset->getHomeDomain().isNull());
    }

    /**
     * Master weight and thresholds above 255 were set and signed as given
     * (they are uint32 in the XDR, one byte to the network, which refuses the
     * operation). The setters and the constructor now refuse them, as
     * js-stellar-base does, and leave the operation as it was. setSigner
     * already refused a weight of 256, but only after it had filled in the
     * signer: the operation then carried that key with weight 0, a delete.
     */
    void testSetOptionsWeightsAboveAByteAreRefused() {
        SetOptionsOperation* operation = SetOptionsOperation::create();
        QVERIFY_EXCEPTION_THROWN(operation->setMasterKeyWeight(256), std::runtime_error);
        QVERIFY_EXCEPTION_THROWN(operation->setLowThreshold(256), std::runtime_error);
        QVERIFY_EXCEPTION_THROWN(operation->setMediumThreshold(1000), std::runtime_error);
        QVERIFY_EXCEPTION_THROWN(operation->setHighThreshold(0xFFFFFFFFu), std::runtime_error);
        auto signer = Signer::ed25519PublicKey(KeyPair::fromAccountId(QString("GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR")));
        QVERIFY_EXCEPTION_THROWN(operation->setSigner(signer, 256), std::runtime_error);
        QVERIFY(operation->getMasterKeyWeight().isNull());
        QVERIFY(operation->getLowThreshold().isNull());
        QVERIFY(operation->getMediumThreshold().isNull());
        QVERIFY(operation->getHighThreshold().isNull());
        QVERIFY(operation->getSigner() == nullptr);

        // 255 is a byte, and 0 a real instruction (retire the master key).
        operation->setMasterKeyWeight(255)->setLowThreshold(0)->setMediumThreshold(255)
                 ->setHighThreshold(255)->setSigner(signer, 255);
        QCOMPARE(static_cast<int>(operation->getMasterKeyWeight()), 255);
        QCOMPARE(static_cast<int>(operation->getLowThreshold()), 0);
        QCOMPARE(static_cast<int>(operation->getSignerWeight()), 255);
        delete operation;

        const stellar::SignerKey noSigner = signer;
        QVERIFY_EXCEPTION_THROWN(SetOptionsOperation(nullptr, Integer(), Integer(), Integer(256),
                                                     Integer(), Integer(), Integer(), QString(),
                                                     noSigner, 1),
                                 std::runtime_error);
        QVERIFY_EXCEPTION_THROWN(SetOptionsOperation(nullptr, Integer(), Integer(), Integer(),
                                                     Integer(), Integer(), Integer(300), QString(),
                                                     noSigner, 1),
                                 std::runtime_error);
    }

    /**
     * TxRep reads set-options through the same setters: a document with a
     * master weight of 256 was signed as 256. It is refused now, as a
     * std::runtime_error from fromTxrep.
     */
    void testSetOptionsTxrepWeightAboveAByteIsRefused() {
        KeyPair* source = KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));
        Account* account = new Account(source, 2908908335136768L);
        Transaction* transaction = Transaction::Builder(AccountConverter().enableMuxed(), account, Network::testnetNetwork())
                .addOperation(SetOptionsOperation::create()->setMasterKeyWeight(1)->setHighThreshold(2))
                .setTimeout(Transaction::Builder::TIMEOUT_INFINITE)
                .setBaseFee(Transaction::Builder::BASE_FEE)
                .build();
        const QString txrep = Txrep::toTxrep(transaction);
        Transaction* back = Txrep::fromTxrep(txrep, Network::testnetNetwork());
        QVERIFY(back);
        delete back;

        for (const char* field : {"masterWeight", "highThreshold"}) {
            const QString line = QStringLiteral("setOptionsOp.%1: ").arg(QLatin1String(field));
            const int at = txrep.indexOf(line);
            QVERIFY2(at >= 0, field);
            const int end = txrep.indexOf(QLatin1Char('\n'), at);
            QString tooBig = txrep;
            tooBig.replace(at, end - at, line + QStringLiteral("256"));
            QVERIFY_EXCEPTION_THROWN(Txrep::fromTxrep(tooBig, Network::testnetNetwork()),
                                     std::runtime_error);
        }
        delete transaction;
        delete account;
    }


    void testManageSellOfferOperation() {
        // GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF
        KeyPair* source = KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));
        // GBCP5W2VS7AEWV2HFRN7YYC623LTSV7VSTGIHFXDEJU7S5BAGVCSETRR
        KeyPair* issuer = KeyPair::fromSecretSeed(QString("SA64U7C5C7BS5IHWEPA7YWFN3Z6FE5L6KAMYUIT4AQ7KVTVLD23C6HEZ"));

        Asset* selling = new AssetTypeNative();
        Asset* buying = Asset::createNonNativeAsset("USD", issuer);
        QString amount = "0.00001";
        QString price = "0.85334384"; // n=5333399 d=6250000
        Price* priceObj = Price::fromString(price);
        qint64 offerId = 1;

        ManageSellOfferOperation* operation = ManageSellOfferOperation::create(selling, buying, amount, price)
                ->setOfferId(offerId)
                ->setSourceAccount(source->getAccountId());

        auto xdr = operation->toXdr();
        ManageSellOfferOperation* parsedOperation = (ManageSellOfferOperation*) ManageSellOfferOperation::fromXdr(xdr);


        QCOMPARE(xdr.operationManageSellOffer.amount,(qint64)100L);
        QVERIFY(dynamic_cast<AssetTypeNative*>(parsedOperation->getSelling()));
        QVERIFY(dynamic_cast<AssetTypeCreditAlphaNum4*>(parsedOperation->getBuying()));
        QVERIFY(parsedOperation->getBuying()->equals(buying));
        QCOMPARE(parsedOperation->getAmount(),amount);
        QCOMPARE(parsedOperation->getPrice(),price);
        QCOMPARE(priceObj->getNumerator(), 5333399);
        QCOMPARE(priceObj->getDenominator(), 6250000);
        QCOMPARE(parsedOperation->getOfferId(),offerId);

        QCOMPARE(operation->toXdrBase64()
                 ,QString("AAAAAQAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAAMAAAAAAAAAAVVTRAAAAAAARP7bVZfAS1dHLFv8YF7W1zlX9ZTMg5bjImn5dCA1RSIAAAAAAAAAZABRYZcAX14QAAAAAAAAAAE="));
    }

    void testManageBuyOfferOperation() {
        // GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF
        KeyPair* source = KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));
        // GBCP5W2VS7AEWV2HFRN7YYC623LTSV7VSTGIHFXDEJU7S5BAGVCSETRR
        KeyPair* issuer = KeyPair::fromSecretSeed(QString("SA64U7C5C7BS5IHWEPA7YWFN3Z6FE5L6KAMYUIT4AQ7KVTVLD23C6HEZ"));

        Asset* selling = new AssetTypeNative();
        Asset* buying = Asset::createNonNativeAsset("USD", issuer);
        QString amount = "0.00001";
        QString price = "0.85334384"; // n=5333399 d=6250000
        Price* priceObj = Price::fromString(price);
        qint64 offerId = 1;

        ManageBuyOfferOperation* operation = ManageBuyOfferOperation::create(selling, buying, amount, price)
                ->setOfferId(offerId)
                ->setSourceAccount(source->getAccountId());

        auto xdr = operation->toXdr();
        ManageBuyOfferOperation* parsedOperation = (ManageBuyOfferOperation*) ManageBuyOfferOperation::fromXdr(xdr);


        QCOMPARE(xdr.operationManageBuyOffer.buyAmount,(qint64)100L);
        QVERIFY(dynamic_cast<AssetTypeNative*>(parsedOperation->getSelling()));
        QVERIFY(dynamic_cast<AssetTypeCreditAlphaNum4*>(parsedOperation->getBuying()));
        QVERIFY(parsedOperation->getBuying()->equals(buying));
        QCOMPARE(parsedOperation->getAmount(),amount);
        QCOMPARE(parsedOperation->getPrice(),price);
        QCOMPARE(priceObj->getNumerator(), 5333399);
        QCOMPARE(priceObj->getDenominator(), 6250000);
        QCOMPARE(parsedOperation->getOfferId(),offerId);

        QCOMPARE(operation->toXdrBase64()
                 ,QString("AAAAAQAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAAwAAAAAAAAAAVVTRAAAAAAARP7bVZfAS1dHLFv8YF7W1zlX9ZTMg5bjImn5dCA1RSIAAAAAAAAAZABRYZcAX14QAAAAAAAAAAE="));
    }

    void testManageSellOfferOperation_BadArithmeticRegression(){
        // from https://github.com/stellar/java-stellar-sdk/issues/183

        QByteArray transactionEnvelopeToDecode = QByteArray::fromBase64("AAAAAButy5zasS3DLZ5uFpZHL25aiHUfKRwdv1+3Wp12Ce7XAAAAZAEyGwYAAAAOAAAAAAAAAAAAAAABAAAAAQAAAAAbrcuc2rEtwy2ebhaWRy9uWoh1HykcHb9ft1qddgnu1wAAAAMAAAAAAAAAAUtJTgAAAAAARkrT28ebM6YQyhVZi1ttlwq/dk6ijTpyTNuHIMgUp+EAAAAAAAARPSfDKZ0AAv7oAAAAAAAAAAAAAAAAAAAAAXYJ7tcAAABAbE8rEoFt0Hcv41iwVCl74C1Hyr+Lj8ZyaYn7zTJhezClbc+pTW1KgYFIZOJiGVth2xFnBT1pMXuQkVdTlB3FCw==");

        stellar::TransactionEnvelope transactionEnvelope;
        QDataStream stream(&transactionEnvelopeToDecode,QIODevice::ReadOnly);
        stream >> transactionEnvelope;
        QCOMPARE(transactionEnvelope.v0.tx.operations.value.length(),1);
        auto & opXdr = transactionEnvelope.v0.tx.operations.value[0];
        ManageSellOfferOperation* op = (ManageSellOfferOperation*)Operation::fromXdr(opXdr);

        // n/d exactly, as js-stellar-sdk prints it (it was a double's 16 digits).
        QCOMPARE(op->getPrice(),"3397.89330609999592518642");
    }

    void testManageBuyOfferOperation_BadArithmeticRegression(){
        // from https://github.com/stellar/java-stellar-sdk/issues/183

        QByteArray transactionEnvelopeToDecode = QByteArray::fromBase64("AAAAAButy5zasS3DLZ5uFpZHL25aiHUfKRwdv1+3Wp12Ce7XAAAAZAEyGwYAAAAxAAAAAAAAAAAAAAABAAAAAQAAAAAbrcuc2rEtwy2ebhaWRy9uWoh1HykcHb9ft1qddgnu1wAAAAwAAAABS0lOAAAAAABGStPbx5szphDKFVmLW22XCr92TqKNOnJM24cgyBSn4QAAAAAAAAAAACNyOCfDKZ0AAv7oAAAAAAABv1IAAAAAAAAAAA==");

        stellar::TransactionEnvelope transactionEnvelope;
        QDataStream stream(&transactionEnvelopeToDecode,QIODevice::ReadOnly);
        stream >> transactionEnvelope;

        QCOMPARE(transactionEnvelope.v0.tx.operations.value.length(),1);

        Transaction * t = dynamic_cast<Transaction*>(Transaction::fromEnvelopeXdr(transactionEnvelope));

        ManageBuyOfferOperation* op = (ManageBuyOfferOperation*)t->getOperations().first();//Operation::fromXdr(transactionEnvelope.tx.operations.value[0]);

        QCOMPARE(op->getPrice(),"3397.89330609999592518642");
    }

    void testCreatePassiveSellOfferOperation() {
        // GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF
        KeyPair* source = KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));
        // GBCP5W2VS7AEWV2HFRN7YYC623LTSV7VSTGIHFXDEJU7S5BAGVCSETRR
        KeyPair* issuer = KeyPair::fromSecretSeed(QString("SA64U7C5C7BS5IHWEPA7YWFN3Z6FE5L6KAMYUIT4AQ7KVTVLD23C6HEZ"));

        Asset* selling = new AssetTypeNative();
        Asset* buying = Asset::createNonNativeAsset("USD", issuer);
        QString amount = "0.00001";
        QString price = "2.93850088"; // n=36731261 d=12500000
        Price* priceObj = Price::fromString(price);

        CreatePassiveSellOfferOperation* operation = new CreatePassiveSellOfferOperation(selling, buying, amount, price);
        operation->setSourceAccount(source->getAccountId());

        auto xdr = operation->toXdr();
        CreatePassiveSellOfferOperation* parsedOperation = static_cast<CreatePassiveSellOfferOperation*>(CreatePassiveSellOfferOperation::fromXdr(xdr));

        QVERIFY(100L == xdr.operationCreatePassiveSellOffer.amount);
        QVERIFY(dynamic_cast<AssetTypeNative*>(parsedOperation->getSelling()));
        QVERIFY(dynamic_cast<AssetTypeCreditAlphaNum4*>(parsedOperation->getBuying()));
        QVERIFY(parsedOperation->getBuying()->equals(buying));
        QCOMPARE(amount, parsedOperation->getAmount());
        QCOMPARE(price, parsedOperation->getPrice());
        QCOMPARE(priceObj->getNumerator(), 36731261);
        QCOMPARE(priceObj->getDenominator(), 12500000);

        QCOMPARE(operation->toXdrBase64(),
                 QString("AAAAAQAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAAQAAAAAAAAAAVVTRAAAAAAARP7bVZfAS1dHLFv8YF7W1zlX9ZTMg5bjImn5dCA1RSIAAAAAAAAAZAIweX0Avrwg"));
    }


    void testAccountMergeOperation() {
        // GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF
        KeyPair* source = KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));
        // GDW6AUTBXTOC7FIKUO5BOO3OGLK4SF7ZPOBLMQHMZDI45J2Z6VXRB5NR
        KeyPair* destination = KeyPair::fromSecretSeed(QString("SDHZGHURAYXKU2KMVHPOXI6JG2Q4BSQUQCEOY72O3QQTCLR2T455PMII"));

        AccountMergeOperation* operation = new AccountMergeOperation(destination->getAccountId());
        operation->setSourceAccount(source->getAccountId());

        stellar::Operation xdr = operation->toXdr();

        AccountMergeOperation* parsedOperation = (AccountMergeOperation*) Operation::fromXdr(xdr);

        QCOMPARE(destination->getAccountId(), parsedOperation->getDestination());

        QCOMPARE(QString("AAAAAQAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAAgAAAAA7eBSYbzcL5UKo7oXO24y1ckX+XuCtkDsyNHOp1n1bxA="),
                 operation->toXdrBase64());
    }

    void testManageDataOperation() {
        // GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF
        KeyPair* source = KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));

        QByteArray data = QByteArray::fromHex("0001020304");

        ManageDataOperation* operation = new ManageDataOperation("test", data);
        operation->setSourceAccount(source->getAccountId());


        auto xdr = operation->toXdr();

        ManageDataOperation* parsedOperation = static_cast<ManageDataOperation*>(Operation::fromXdr(xdr));

        QCOMPARE(parsedOperation->getName(),QString("test"));
        QCOMPARE(parsedOperation->getValue(),data);

        QCOMPARE(operation->toXdrBase64(),
                 QString("AAAAAQAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAAoAAAAEdGVzdAAAAAEAAAAFAAECAwQAAAA="));
    }

    void testManageDataOperationEmptyValue() {
        // GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF
        KeyPair* source = KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));

        ManageDataOperation* operation = new ManageDataOperation("test");
        operation->setSourceAccount(source->getAccountId());

        auto xdr = operation->toXdr();

        ManageDataOperation* parsedOperation = static_cast<ManageDataOperation*>(Operation::fromXdr(xdr));

        QCOMPARE(parsedOperation->getName(),QString("test"));
        QCOMPARE(parsedOperation->getValue(),QByteArray());

        QCOMPARE(operation->toXdrBase64()
                 ,QString("AAAAAQAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAAoAAAAEdGVzdAAAAAA="));
    }

    /** A negative bumpTo is refused when built (core answers BAD_SEQ), and
     *  still read when decoded from an existing operation. */
    void testNegativeBumpToIsRefused() {
        QVERIFY_EXCEPTION_THROWN(delete BumpSequenceOperation::create(-1), std::runtime_error);
        QScopedPointer<BumpSequenceOperation> zero(BumpSequenceOperation::create(0));
        QCOMPARE(zero->getBumpTo(), qint64(0));
        stellar::BumpSequenceOp op;
        op.bumpTo = -5;
        QScopedPointer<BumpSequenceOperation> decoded(BumpSequenceOperation::build(op));
        QCOMPARE(decoded->getBumpTo(), qint64(-5));
    }

    void testBumpSequence() {
        // GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF
        KeyPair* source = KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));
        BumpSequenceOperation* operation = BumpSequenceOperation::create(156L)
                ->setSourceAccount(source->getAccountId());
        auto xdr = operation->toXdr();
        BumpSequenceOperation* parsedOperation = static_cast<BumpSequenceOperation*>(Operation::fromXdr(xdr));
        QCOMPARE(parsedOperation->getBumpTo(),156L);
        QCOMPARE(operation->toXdrBase64(),QString("AAAAAQAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAAsAAAAAAAAAnA=="));
    }

    void testToXdrAmount() {
        QVERIFY(0L== Operation::toXdrAmount("0"));
        QVERIFY(1L== Operation::toXdrAmount("0.0000001"));
        QVERIFY(10000000L== Operation::toXdrAmount("1"));
        QVERIFY(11234567L== Operation::toXdrAmount("1.1234567"));
        QVERIFY(729912843007381L== Operation::toXdrAmount("72991284.3007381"));
        QVERIFY(729912843007381L== Operation::toXdrAmount("72991284.30073810"));
        QVERIFY(1014016711446800155L== Operation::toXdrAmount("101401671144.6800155"));
        qDebug() << Operation::toXdrAmount("922337203685.4775807");
        QVERIFY(9223372036854775807UL== Operation::toXdrAmount("922337203685.4775807"));
        try {
            Operation::toXdrAmount("0.00000001");
            QFAIL("expected exception");
        }
        catch (const std::exception& e) {
            Q_UNUSED(e)
        }
        try {
            Operation::toXdrAmount("72991284.30073811");
            QFAIL("expected exception");
        }
        catch(const std::exception& e){
            Q_UNUSED(e)
        }
        // S2 regression: invalid / locale-ambiguous / overflowing amounts must
        // throw, not silently produce a wrong value.
        try { Operation::toXdrAmount("abc"); QFAIL("expected exception (non-numeric)"); }
        catch(const std::exception& e){ Q_UNUSED(e) }
        try { Operation::toXdrAmount("1,000.5"); QFAIL("expected exception (thousands comma)"); }
        catch(const std::exception& e){ Q_UNUSED(e) }
        try { Operation::toXdrAmount("92233720368547758.08"); QFAIL("expected exception (int64 overflow)"); }
        catch(const std::exception& e){ Q_UNUSED(e) }
        // sign is applied to the whole amount (was -5000000 before the S2 fix).
        QCOMPARE(Operation::toXdrAmount("-1.5"), (qint64)-15000000LL);
    }


    void testFromXdrAmount() {
        QCOMPARE(QString("0"), Operation::fromXdrAmount(0L));
        QCOMPARE(QString("0.0000001"), Operation::fromXdrAmount(1L));
        QCOMPARE(QString("1"), Operation::fromXdrAmount(10000000L));
        // Negative values (a balance change, a delta) keep their sign in
        // front: they came out as "-1.-5" and "0.-5".
        QCOMPARE(Operation::fromXdrAmount(Q_INT64_C(-15000000)), QString("-1.5"));
        QCOMPARE(Operation::fromXdrAmount(Q_INT64_C(-5000000)), QString("-0.5"));
        QCOMPARE(Operation::fromXdrAmount(Q_INT64_C(-1)), QString("-0.0000001"));
        QCOMPARE(Operation::fromXdrAmount(Q_INT64_C(-10000000)), QString("-1"));
        QCOMPARE(Operation::fromXdrAmount(std::numeric_limits<qint64>::min()), QString("-922337203685.4775808"));
        QCOMPARE(Operation::fromXdrAmount(std::numeric_limits<qint64>::max()), QString("922337203685.4775807"));
        QCOMPARE(Operation::toXdrAmount(Operation::fromXdrAmount(Q_INT64_C(-15000000))), Q_INT64_C(-15000000));
        QCOMPARE(QString("1.1234567"), Operation::fromXdrAmount(11234567L));
        QCOMPARE(QString("72991284.3007381"), Operation::fromXdrAmount(729912843007381L));
        QCOMPARE(QString("101401671144.6800155"), Operation::fromXdrAmount(1014016711446800155L));
        QCOMPARE(QString("922337203685.4775807"), Operation::fromXdrAmount(9223372036854775807L));
    }


    void testClaimClaimableBalanceOperationValid()
    {
        QString balanceId = "000000006d6a0c142516a9cc7885a85c5aba3a1f4af5181cf9e7a809ac7ae5e4a58c825f";
        QString accountId = "GABTTS6N4CT7AUN4LD7IFIUMRD5PSMCW6QTLIQNEFZDEI6ZQVUCQMCLN";
        ClaimClaimableBalanceOperation* operation = ClaimClaimableBalanceOperation::create(balanceId)->setSourceAccount(accountId);

        stellar::Operation xdr = operation->toXdr();
        ClaimClaimableBalanceOperation* parsedOperation = (ClaimClaimableBalanceOperation*) Operation::fromXdr(xdr);
        QCOMPARE(accountId, parsedOperation->getSourceAccount());
        QCOMPARE(balanceId, parsedOperation->getBalanceId());
        // Generated by js-stellar-base.
        QCOMPARE(operation->toXdrBase64()
                 ,"AAAAAQAAAAADOcvN4KfwUbxY/oKijIj6+TBW9Ca0QaQuRkR7MK0FBgAAAA8AAAAAbWoMFCUWqcx4hahcWro6H0r1GBz556gJrHrl5KWMgl8=");
    }

    void testClaimClaimableBalanceOperationInvalidEmptyBalanceId()
    {
        QString balanceId = "";
        QString accountId = "GABTTS6N4CT7AUN4LD7IFIUMRD5PSMCW6QTLIQNEFZDEI6ZQVUCQMCLN";
        try {
            ClaimClaimableBalanceOperation* operation = ClaimClaimableBalanceOperation::create(balanceId)->setSourceAccount(accountId);
            operation->toXdr();
            QFAIL("missing excepcion");
        } catch (const std::runtime_error& e) {
            //"invalid balanceId"
            Q_UNUSED(e)
        }
    }

    void testClaimClaimableBalanceOperationInvalidClaimableBalanceIDTypeMissing()
    {
        QString balanceId = "6d6a0c142516a9cc7885a85c5aba3a1f4af5181cf9e7a809ac7ae5e4a58c825f";
        QString accountId = "GABTTS6N4CT7AUN4LD7IFIUMRD5PSMCW6QTLIQNEFZDEI6ZQVUCQMCLN";
        try {
            ClaimClaimableBalanceOperation* operation = ClaimClaimableBalanceOperation::create(balanceId)->setSourceAccount(accountId);
            operation->toXdr();
            QFAIL("missing excepcion");
        } catch (const std::runtime_error& ignored) {
            Q_UNUSED(ignored)
        }
    }

    void testClaimClaimableBalanceOperationInvalidClaimableBalanceIDBodyMissing()
    {
        QString balanceId = "00000000";
        QString accountId = "GABTTS6N4CT7AUN4LD7IFIUMRD5PSMCW6QTLIQNEFZDEI6ZQVUCQMCLN";
        try {
            ClaimClaimableBalanceOperation* operation = ClaimClaimableBalanceOperation::create(balanceId)->setSourceAccount(accountId);
            operation->toXdr();
            QFAIL("missing excepcion");
        } catch (const std::runtime_error& e) {
            //"invalid balanceId"
            Q_UNUSED(e)
        }
    }

    void testRevokeAccountSponsorshipOperation() {
        QString source = "GA2N7NI5WEMJILMK4UPDTF2ZX2BIRQUM3HZUE27TRUNRFN5M5EXU6RQV";
        QString accountId = "GAGQ7DNQUVQR6OWYOI563L5EMJE6KCAHPQSFCZFLY5PDRYMRCA5UWCMP";
        RevokeAccountSponsorshipOperation* operation = RevokeAccountSponsorshipOperation::create(accountId)->setSourceAccount(source);
        QCOMPARE(operation->toXdrBase64(),"AAAAAQAAAAA037UdsRiULYrlHjmXWb6CiMKM2fNCa/ONGxK3rOkvTwAAABIAAAAAAAAAAAAAAAAND42wpWEfOthyO+2vpGJJ5QgHfCRRZKvHXjjhkRA7Sw==");

        stellar::Operation xdr = operation->toXdr();
        RevokeAccountSponsorshipOperation* parsedOperation = (RevokeAccountSponsorshipOperation*) Operation::fromXdr(xdr);
        QCOMPARE(accountId, parsedOperation->getAccountId());
        QCOMPARE(source, parsedOperation->getSourceAccount());
    }


    void testRevokeDataSponsorshipOperation() {
        QString source = "GA2N7NI5WEMJILMK4UPDTF2ZX2BIRQUM3HZUE27TRUNRFN5M5EXU6RQV";
        QString accountId = "GAGQ7DNQUVQR6OWYOI563L5EMJE6KCAHPQSFCZFLY5PDRYMRCA5UWCMP";
        QString dataName = "data_name";
        RevokeDataSponsorshipOperation* operation = RevokeDataSponsorshipOperation::create(accountId, dataName)->setSourceAccount(source);
        QCOMPARE(operation->toXdrBase64(), "AAAAAQAAAAA037UdsRiULYrlHjmXWb6CiMKM2fNCa/ONGxK3rOkvTwAAABIAAAAAAAAAAwAAAAAND42wpWEfOthyO+2vpGJJ5QgHfCRRZKvHXjjhkRA7SwAAAAlkYXRhX25hbWUAAAA=");

        stellar::Operation xdr = operation->toXdr();
        RevokeDataSponsorshipOperation* parsedOperation = (RevokeDataSponsorshipOperation*) Operation::fromXdr(xdr);
        QCOMPARE(accountId, parsedOperation->getAccountId());
        QCOMPARE(source, parsedOperation->getSourceAccount());
        QCOMPARE(dataName, parsedOperation->getDataName());
    }

    /**
     * A revoke of the data entry BOM + "k" read back as a revoke of "k", and
     * a home domain BOM + "example.com" as "example.com": QString::fromUtf8
     * drops a leading BOM. js and py keep U+FEFF (17.1.0 and 14.1.0 give back
     * both names whole), as getName did since G24. TxRep refuses that revoke
     * now for its hidden U+FEFF; it wrote it as a revoke of "k", which only
     * its read-back check refused (as no document that reads back).
     */
    void testRevokeDataNameAndHomeDomainKeepALeadingByteOrderMark() {
        const QString bom(QChar(0xFEFF));
        const QString account = QStringLiteral("GAGQ7DNQUVQR6OWYOI563L5EMJE6KCAHPQSFCZFLY5PDRYMRCA5UWCMP");
        const QStringList names = {bom + QStringLiteral("k"), bom + bom + QStringLiteral("k"),
                                   QStringLiteral("k") + bom, QStringLiteral("k")};
        for (const QString& name : names) {
            QScopedPointer<RevokeDataSponsorshipOperation> op(RevokeDataSponsorshipOperation::create(account, name));
            stellar::Operation xdr = op->toXdr();
            QScopedPointer<Operation> decoded(Operation::fromXdr(xdr));
            QCOMPARE(dynamic_cast<RevokeDataSponsorshipOperation*>(decoded.data())->getDataName(), name);

            QScopedPointer<SetOptionsOperation> options(SetOptionsOperation::create());
            options->setHomeDomain(name + QStringLiteral("example.com"));
            stellar::Operation optionsXdr = options->toXdr();
            QScopedPointer<Operation> readBack(Operation::fromXdr(optionsXdr));
            QCOMPARE(dynamic_cast<SetOptionsOperation*>(readBack.data())->getHomeDomain(),
                     name + QStringLiteral("example.com"));
        }

        Account source(KeyPair::random(), 41);
        QScopedPointer<Transaction> tx(Transaction::Builder(AccountConverter().enableMuxed(), &source, Network::testnetNetwork())
                .addOperation(RevokeDataSponsorshipOperation::create(account, bom + QStringLiteral("k")))
                .setTimeout(300)
                .setBaseFee(100)
                .build());
        QString refusal;
        try {
            Txrep::toTxrep(tx.data());
        } catch (const std::runtime_error& e) {
            refusal = QString::fromUtf8(e.what());
        }
        QVERIFY2(refusal.contains(QStringLiteral("revoke-data name")), qPrintable(refusal));
    }

    void testRevokeClaimableBalanceSponsorshipOperation() {
        QString source = "GA2N7NI5WEMJILMK4UPDTF2ZX2BIRQUM3HZUE27TRUNRFN5M5EXU6RQV";
        QString balanceId = "00000000550e14acbdafcd3089289363b3b0c8bec9b4edd87298c690655b4b2456d68ba0";
        RevokeClaimableBalanceSponsorshipOperation* operation = RevokeClaimableBalanceSponsorshipOperation::create(balanceId)->setSourceAccount(source);
        QCOMPARE(operation->toXdrBase64(), "AAAAAQAAAAA037UdsRiULYrlHjmXWb6CiMKM2fNCa/ONGxK3rOkvTwAAABIAAAAAAAAABAAAAABVDhSsva/NMIkok2OzsMi+ybTt2HKYxpBlW0skVtaLoA==");

        stellar::Operation xdr = operation->toXdr();
        RevokeClaimableBalanceSponsorshipOperation* parsedOperation = (RevokeClaimableBalanceSponsorshipOperation*) Operation::fromXdr(xdr);
        QCOMPARE(balanceId, parsedOperation->getBalanceId());
        QCOMPARE(source, parsedOperation->getSourceAccount());
    }

    void testRevokeLiquidityPoolSponsorshipOperation() {
        QString source = "GA2N7NI5WEMJILMK4UPDTF2ZX2BIRQUM3HZUE27TRUNRFN5M5EXU6RQV";
        QString liquidityPoolId = "dd7b1ab831c273a09e76784f4ff8df4b86bc0d8d6f4eef21d9bb5d6da7c2e6f1";
        RevokeLiquidityPoolSponsorshipOperation* operation = RevokeLiquidityPoolSponsorshipOperation::create(liquidityPoolId)->setSourceAccount(source);

        stellar::Operation xdr = operation->toXdr();
        RevokeLiquidityPoolSponsorshipOperation* parsedOperation = (RevokeLiquidityPoolSponsorshipOperation*) Operation::fromXdr(xdr);
        QCOMPARE(liquidityPoolId, parsedOperation->getLiquidityPoolId());
        QCOMPARE(source, parsedOperation->getSourceAccount());
    }

    void testRevokeOfferSponsorshipOperation() {
        QString source = "GA2N7NI5WEMJILMK4UPDTF2ZX2BIRQUM3HZUE27TRUNRFN5M5EXU6RQV";
        QString seller = "GAGQ7DNQUVQR6OWYOI563L5EMJE6KCAHPQSFCZFLY5PDRYMRCA5UWCMP";
        qint64 offerId = 123456L;
        RevokeOfferSponsorshipOperation* operation = RevokeOfferSponsorshipOperation::create(seller, offerId)->setSourceAccount(source);
        QCOMPARE(operation->toXdrBase64(), "AAAAAQAAAAA037UdsRiULYrlHjmXWb6CiMKM2fNCa/ONGxK3rOkvTwAAABIAAAAAAAAAAgAAAAAND42wpWEfOthyO+2vpGJJ5QgHfCRRZKvHXjjhkRA7SwAAAAAAAeJA");

        stellar::Operation xdr = operation->toXdr();
        RevokeOfferSponsorshipOperation* parsedOperation = (RevokeOfferSponsorshipOperation*) Operation::fromXdr(xdr);
        QCOMPARE(offerId, parsedOperation->getOfferId());
        QCOMPARE(seller, parsedOperation->getSeller());
        QCOMPARE(source, parsedOperation->getSourceAccount());
    }

    void testRevokeSignerSponsorshipOperation() {
        QString source = "GA2N7NI5WEMJILMK4UPDTF2ZX2BIRQUM3HZUE27TRUNRFN5M5EXU6RQV";
        QString accountId = "GAGQ7DNQUVQR6OWYOI563L5EMJE6KCAHPQSFCZFLY5PDRYMRCA5UWCMP";
        stellar::SignerKey signerKey = KeyPair::fromAccountId("GBOSQJIV4VJMWMPVPB7EFVIRJT7A7SAAAB4FA23ZDJRUMXMYHBYWY57L")->getXdrSignerKey();
        RevokeSignerSponsorshipOperation* operation = RevokeSignerSponsorshipOperation::create(accountId, signerKey)->setSourceAccount(source);
        QCOMPARE(operation->toXdrBase64(), "AAAAAQAAAAA037UdsRiULYrlHjmXWb6CiMKM2fNCa/ONGxK3rOkvTwAAABIAAAABAAAAAA0PjbClYR862HI77a+kYknlCAd8JFFkq8deOOGREDtLAAAAAF0oJRXlUssx9Xh+QtURTP4PyAAAeFBreRpjRl2YOHFs");

        stellar::Operation xdr = operation->toXdr();
        RevokeSignerSponsorshipOperation* parsedOperation = (RevokeSignerSponsorshipOperation*) Operation::fromXdr(xdr);
        QCOMPARE(accountId, parsedOperation->getAccountId());
        QCOMPARE(source, parsedOperation->getSourceAccount());
        QCOMPARE(signerKey, parsedOperation->getSigner());
    }


    void testRevokeTrustlineSponsorshipOperation() {
        QString source = "GA2N7NI5WEMJILMK4UPDTF2ZX2BIRQUM3HZUE27TRUNRFN5M5EXU6RQV";
        QString accountId = "GAGQ7DNQUVQR6OWYOI563L5EMJE6KCAHPQSFCZFLY5PDRYMRCA5UWCMP";
        Asset* asset = Asset::create("DEMO", "GCWPICV6IV35FQ2MVZSEDLORHEMMIAODRQPVDEIKZOW2GC2JGGDCXVVV");
        RevokeTrustlineSponsorshipOperation* operation = RevokeTrustlineSponsorshipOperation::create(accountId, asset)->setSourceAccount(source);
        QCOMPARE(operation->toXdrBase64() , "AAAAAQAAAAA037UdsRiULYrlHjmXWb6CiMKM2fNCa/ONGxK3rOkvTwAAABIAAAAAAAAAAQAAAAAND42wpWEfOthyO+2vpGJJ5QgHfCRRZKvHXjjhkRA7SwAAAAFERU1PAAAAAKz0Cr5Fd9LDTK5kQa3RORjEAcOMH1GRCsutowtJMYYr");

        stellar::Operation xdr = operation->toXdr();
        RevokeTrustlineSponsorshipOperation* parsedOperation = (RevokeTrustlineSponsorshipOperation*) Operation::fromXdr(xdr);
        QCOMPARE(accountId, parsedOperation->getAccountId());
        QCOMPARE(source, parsedOperation->getSourceAccount());
        QVERIFY(asset->equals(parsedOperation->getAsset()));
    }

    void testBeginSponsoringFutureReserves() {
        QString source = "GA2N7NI5WEMJILMK4UPDTF2ZX2BIRQUM3HZUE27TRUNRFN5M5EXU6RQV";
        QString accountId = "GAGQ7DNQUVQR6OWYOI563L5EMJE6KCAHPQSFCZFLY5PDRYMRCA5UWCMP";

        BeginSponsoringFutureReservesOperation* operation = BeginSponsoringFutureReservesOperation::create(accountId)->setSourceAccount(source);
        QCOMPARE(operation->toXdrBase64() , "AAAAAQAAAAA037UdsRiULYrlHjmXWb6CiMKM2fNCa/ONGxK3rOkvTwAAABAAAAAADQ+NsKVhHzrYcjvtr6RiSeUIB3wkUWSrx1444ZEQO0s=");

        stellar::Operation xdr = operation->toXdr();
        BeginSponsoringFutureReservesOperation* parsedOperation = (BeginSponsoringFutureReservesOperation*) Operation::fromXdr(xdr);
        QCOMPARE(accountId, parsedOperation->getSponsoredID());
        QCOMPARE(source, parsedOperation->getSourceAccount());

    }
    void testEndSponsoringFutureReserves() {
        QString source = "GA2N7NI5WEMJILMK4UPDTF2ZX2BIRQUM3HZUE27TRUNRFN5M5EXU6RQV";

        EndSponsoringFutureReservesOperation* operation = EndSponsoringFutureReservesOperation::create()->setSourceAccount(source);
        QCOMPARE(operation->toXdrBase64() , "AAAAAQAAAAA037UdsRiULYrlHjmXWb6CiMKM2fNCa/ONGxK3rOkvTwAAABE=");

        stellar::Operation xdr = operation->toXdr();
        EndSponsoringFutureReservesOperation* parsedOperation = (EndSponsoringFutureReservesOperation*) Operation::fromXdr(xdr);
        QCOMPARE(source, parsedOperation->getSourceAccount());


    }

    void testClawbackClaimableBalanceOperation()
    {
        QString source = "GA2N7NI5WEMJILMK4UPDTF2ZX2BIRQUM3HZUE27TRUNRFN5M5EXU6RQV";
        QString balanceId = "00000000929b20b72e5890ab51c24f1cc46fa01c4f318d8d33367d24dd614cfdf5491072";
        ClawbackClaimableBalanceOperation* operation = ClawbackClaimableBalanceOperation::create(balanceId)->setSourceAccount(source);
        QCOMPARE(operation->toXdrBase64(), QString("AAAAAQAAAAA037UdsRiULYrlHjmXWb6CiMKM2fNCa/ONGxK3rOkvTwAAABQAAAAAkpsgty5YkKtRwk8cxG+gHE8xjY0zNn0k3WFM/fVJEHI="));

        stellar::Operation xdr = operation->toXdr();
        ClawbackClaimableBalanceOperation* parsedOperation = (ClawbackClaimableBalanceOperation*) Operation::fromXdr(xdr);
        QCOMPARE(balanceId, parsedOperation->getBalanceId());
        QCOMPARE(source, parsedOperation->getSourceAccount());
    }

    void testClawbackOperation()
    {
        QString source = "GA2N7NI5WEMJILMK4UPDTF2ZX2BIRQUM3HZUE27TRUNRFN5M5EXU6RQV";
        QString accountId = "GAGQ7DNQUVQR6OWYOI563L5EMJE6KCAHPQSFCZFLY5PDRYMRCA5UWCMP";

        Asset* asset = Asset::create("DEMO", "GCWPICV6IV35FQ2MVZSEDLORHEMMIAODRQPVDEIKZOW2GC2JGGDCXVVV");
        QString amt = "100";

        ClawbackOperation* operation = ClawbackOperation::create(accountId, asset, amt)->setSourceAccount(source);
        QCOMPARE(operation->toXdrBase64(), QString("AAAAAQAAAAA037UdsRiULYrlHjmXWb6CiMKM2fNCa/ONGxK3rOkvTwAAABMAAAABREVNTwAAAACs9Aq+RXfSw0yuZEGt0TkYxAHDjB9RkQrLraMLSTGGKwAAAAAND42wpWEfOthyO+2vpGJJ5QgHfCRRZKvHXjjhkRA7SwAAAAA7msoA"));

        stellar::Operation xdr = operation->toXdr();
        ClawbackOperation* parsedOperation = (ClawbackOperation*) Operation::fromXdr(xdr);
        QCOMPARE(accountId, parsedOperation->getFrom());
        QVERIFY(asset->equals(parsedOperation->getAsset()));
        QCOMPARE(amt, parsedOperation->getAmount());

        QCOMPARE(source, parsedOperation->getSourceAccount());
    }

    void testCantClawbackNativeAsset()
    {
        try {
            QString accountId = "GAGQ7DNQUVQR6OWYOI563L5EMJE6KCAHPQSFCZFLY5PDRYMRCA5UWCMP";
            QString amt = "100";
            ClawbackOperation::create(accountId, Asset::create("native"), amt);
            QFAIL("missing excepcion");
        } catch (const std::runtime_error& e) {
            //"native assets are not supported"
            Q_UNUSED(e)
        }
    }

    void testSetTrustlineFlagsOperation()
    {
        QString source = "GA2N7NI5WEMJILMK4UPDTF2ZX2BIRQUM3HZUE27TRUNRFN5M5EXU6RQV";
        QString accountId = "GAGQ7DNQUVQR6OWYOI563L5EMJE6KCAHPQSFCZFLY5PDRYMRCA5UWCMP";
        Asset* asset = Asset::create("DEMO", "GCWPICV6IV35FQ2MVZSEDLORHEMMIAODRQPVDEIKZOW2GC2JGGDCXVVV");
        QFlags<stellar::TrustLineFlags> toClear = QFlags<stellar::TrustLineFlags>(stellar::TrustLineFlags::AUTHORIZED_FLAG);

        QFlags<stellar::TrustLineFlags> toSet = QFlags<stellar::TrustLineFlags>({ stellar::TrustLineFlags::AUTHORIZED_TO_MAINTAIN_LIABILITIES_FLAG, stellar::TrustLineFlags::TRUSTLINE_CLAWBACK_ENABLED_FLAG});

        SetTrustlineFlagsOperation* operation = SetTrustlineFlagsOperation::create(accountId, asset, toClear, toSet)->setSourceAccount(source);
        QCOMPARE(operation->toXdrBase64(), "AAAAAQAAAAA037UdsRiULYrlHjmXWb6CiMKM2fNCa/ONGxK3rOkvTwAAABUAAAAADQ+NsKVhHzrYcjvtr6RiSeUIB3wkUWSrx1444ZEQO0sAAAABREVNTwAAAACs9Aq+RXfSw0yuZEGt0TkYxAHDjB9RkQrLraMLSTGGKwAAAAEAAAAG");

        stellar::Operation xdr = operation->toXdr();
        QCOMPARE( (int)xdr.operationSetTrustLineFlags.clearFlags, (int)stellar::TrustLineFlags::AUTHORIZED_FLAG);
        QCOMPARE( (int)xdr.operationSetTrustLineFlags.setFlags, ((int)stellar::TrustLineFlags::AUTHORIZED_TO_MAINTAIN_LIABILITIES_FLAG | (int)stellar::TrustLineFlags::TRUSTLINE_CLAWBACK_ENABLED_FLAG));
        SetTrustlineFlagsOperation* parsedOperation = (SetTrustlineFlagsOperation*) Operation::fromXdr(xdr);
        QCOMPARE(accountId, parsedOperation->getTrustor());
        QVERIFY(asset->equals(parsedOperation->getAsset()));
        QCOMPARE(toClear, parsedOperation->getClearFlags());
        QCOMPARE(toSet, parsedOperation->getSetFlags());

        QCOMPARE(source, parsedOperation->getSourceAccount());

    }


    void testCantSetNativeTrustlineFlags() {
        try {
            QString source = "GA2N7NI5WEMJILMK4UPDTF2ZX2BIRQUM3HZUE27TRUNRFN5M5EXU6RQV";
            QString accountId = "GAGQ7DNQUVQR6OWYOI563L5EMJE6KCAHPQSFCZFLY5PDRYMRCA5UWCMP";
            QFlags<stellar::TrustLineFlags> toClear = QFlags<stellar::TrustLineFlags>(stellar::TrustLineFlags::AUTHORIZED_FLAG);
            QFlags<stellar::TrustLineFlags> toSet = QFlags<stellar::TrustLineFlags>(stellar::TrustLineFlags::AUTHORIZED_TO_MAINTAIN_LIABILITIES_FLAG);

            SetTrustlineFlagsOperation::create(accountId, Asset::create("native"), toClear, toSet)->setSourceAccount(source);
            QFAIL("missing excepcion");
        } catch (const std::runtime_error& e) {
            //"native assets are not supported"
            Q_UNUSED(e)
        }
    }
};

ADD_TEST(OperationTest)
#endif // OPERATIONTEST_H
