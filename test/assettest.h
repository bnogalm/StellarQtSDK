#ifndef ASSETTEST_H
#define ASSETTEST_H
#include <QObject>
#include "src/keypair.h"
#include "src/util.h"

#include <QtTest>
#include "testcollector.h"
#include "../src/asset.h"
#include "../src/assettypenative.h"
#include "../src/assettypecreditalphanum.h"
#include "../src/keypair.h"
#include "../src/assettypecreditalphanum4.h"
#include "../src/assettypecreditalphanum12.h"

class AssetTest: public QObject
{
    Q_OBJECT

private slots:

    void initTestCase()
    {
    }
    void cleanupTestCase()
    {
    }

    void testAssetTypeNative() {
        AssetTypeNative asset;
        auto xdr = asset.toXdr();
        Asset* parsedAsset = Asset::fromXdr(xdr);
        QVERIFY(dynamic_cast<AssetTypeNative*>(parsedAsset));
    }

    void testAssetTypeCreditAlphaNum4() {
        QString code = "USDA";
        KeyPair* issuer = KeyPair::random();
        AssetTypeCreditAlphaNum4 asset(code, issuer);
        auto xdr = asset.toXdr();
        AssetTypeCreditAlphaNum4* parsedAsset = (AssetTypeCreditAlphaNum4*) Asset::fromXdr(xdr);
        QCOMPARE(code, asset.getCode());
        QCOMPARE(issuer->getAccountId(), parsedAsset->getIssuer().getAccountId());
    }

    void testAssetTypeCreditAlphaNum12() {
        QString code = "TESTTEST";
        KeyPair* issuer = KeyPair::random();
        AssetTypeCreditAlphaNum12 asset(code, issuer);
        auto xdr = asset.toXdr();
        AssetTypeCreditAlphaNum12* parsedAsset = (AssetTypeCreditAlphaNum12*) Asset::fromXdr(xdr);
        QCOMPARE(code, asset.getCode());
        QCOMPARE(issuer->getAccountId(), parsedAsset->getIssuer().getAccountId());
    }


    void testHashCode() {
        KeyPair* issuer1 = KeyPair::random();
        KeyPair* issuer2 = KeyPair::random();

        // Equal
        QCOMPARE(AssetTypeNative().hashCode(), AssetTypeNative().hashCode());
        QCOMPARE(AssetTypeCreditAlphaNum4("USD", *issuer1).hashCode(), AssetTypeCreditAlphaNum4("USD", *issuer1).hashCode());
        QCOMPARE(AssetTypeCreditAlphaNum12("ABCDE", *issuer1).hashCode(), AssetTypeCreditAlphaNum12("ABCDE", *issuer1).hashCode());

        // Not equal

        QVERIFY(AssetTypeNative().hashCode()!= AssetTypeCreditAlphaNum4("USD", *issuer1).hashCode());
        QVERIFY(AssetTypeNative().hashCode()!= AssetTypeCreditAlphaNum12("ABCDE", *issuer1).hashCode());
        QVERIFY(AssetTypeCreditAlphaNum4("EUR", *issuer1).hashCode()!= AssetTypeCreditAlphaNum4("USD", *issuer1).hashCode());
        QVERIFY(AssetTypeCreditAlphaNum4("EUR", *issuer1).hashCode()!= AssetTypeCreditAlphaNum4("EUR", *issuer2).hashCode());
        QVERIFY(AssetTypeCreditAlphaNum4("EUR", *issuer1).hashCode()!= AssetTypeCreditAlphaNum12("EUROPE", *issuer1).hashCode());
        QVERIFY(AssetTypeCreditAlphaNum4("EUR", *issuer1).hashCode()!= AssetTypeCreditAlphaNum12("EUROPE", *issuer2).hashCode());
        QVERIFY(AssetTypeCreditAlphaNum12("ABCDE", *issuer1).hashCode()!= AssetTypeCreditAlphaNum12("EDCBA", *issuer1).hashCode());
        QVERIFY(AssetTypeCreditAlphaNum12("ABCDE", *issuer1).hashCode()!= AssetTypeCreditAlphaNum12("ABCDE", *issuer2).hashCode());
    }

    void testAssetEquals() {
        KeyPair* issuer1 = KeyPair::random();
        KeyPair* issuer2 = KeyPair::random();
        qDebug()<< "KEYS? ";
        qDebug() << issuer1->getAccountId();
        qDebug() << issuer2->getAccountId();
        QVERIFY(AssetTypeNative().equals(new AssetTypeNative()));
        QVERIFY(AssetTypeCreditAlphaNum4("USD", *issuer1).equals(new AssetTypeCreditAlphaNum4("USD", *issuer1)));
        QVERIFY(AssetTypeCreditAlphaNum12("ABCDE", *issuer1).equals(new AssetTypeCreditAlphaNum12("ABCDE", *issuer1)));

        QVERIFY(!AssetTypeNative().equals(new AssetTypeCreditAlphaNum4("USD", *issuer1)));
        QVERIFY(!AssetTypeNative().equals(new AssetTypeCreditAlphaNum12("ABCDE", *issuer1)));
        QVERIFY(!AssetTypeCreditAlphaNum4("EUR", *issuer1).equals(new AssetTypeCreditAlphaNum4("USD", *issuer1)));
        QVERIFY(!AssetTypeCreditAlphaNum4("EUR", *issuer1).equals(new AssetTypeCreditAlphaNum4("EUR", *issuer2)));
        QVERIFY(!AssetTypeCreditAlphaNum12("ABCDE", *issuer1).equals(new AssetTypeCreditAlphaNum12("EDCBA", *issuer1)));
        QVERIFY(!AssetTypeCreditAlphaNum12("ABCDE", *issuer1).equals(new AssetTypeCreditAlphaNum12("ABCDE", *issuer2)));
    }
};

ADD_TEST(AssetTest)
#endif // ASSETTEST_H
