#ifndef ASSETDESERIALIZERTEST_H
#define ASSETDESERIALIZERTEST_H
#include <QObject>


#include <QtTest>
#include "../testcollector.h"
#include "../../src/responses/common.h"
#include "../../src/assettypecreditalphanum.h"

class AssetDeserializerTest: public QObject
{
    Q_OBJECT

public:


private slots:

    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }
    void testDeserializeNative() {
        QByteArray json = "{\"asset_type\": \"native\"}";
        QJsonParseError check;
        QJsonDocument doc = QJsonDocument::fromJson(json,&check);

        if(check.error){
            QFAIL("Fail parsing json");
        }
        QVariantMap variant  = doc.object().toVariantMap();//we parse them passing them to variantmap
        Asset* asset = assetFromVariantMap(variant);

        QCOMPARE(asset->getType(), QString("native"));
    }
    void testDeserializeCredit() {
        QByteArray json = "{\n"
                "  \"asset_type\": \"credit_alphanum4\",\n"
                "  \"asset_code\": \"CNY\",\n"
                "  \"asset_issuer\": \"GAREELUB43IRHWEASCFBLKHURCGMHE5IF6XSE7EXDLACYHGRHM43RFOX\"\n"
                "}";
        QJsonParseError check;
        QJsonDocument doc = QJsonDocument::fromJson(json,&check);

        if(check.error){
            QFAIL("Fail parsing json");
        }
        QVariantMap variant  = doc.object().toVariantMap();//we parse them passing them to variantmap
        Asset* asset = assetFromVariantMap(variant);

        QCOMPARE(asset->getType(), QString("credit_alphanum4"));
        AssetTypeCreditAlphaNum* creditAsset = (AssetTypeCreditAlphaNum*) asset;
        QCOMPARE(creditAsset->getCode(), QString("CNY"));
        QCOMPARE(creditAsset->getIssuer().getAccountId(), QString("GAREELUB43IRHWEASCFBLKHURCGMHE5IF6XSE7EXDLACYHGRHM43RFOX"));
    }



    /**
     * The asset a map names, exactly: every type but "native" became the
     * asset the code's length made, so a credit_alphanum4 type with a
     * 5-letter code read as an alphanum12 asset, and a liquidity_pool_shares
     * type as a credit asset with an empty code.
     */
    void testAssetMapMustAgreeWithItsType()
    {
        const QString issuer = QStringLiteral("GAREELUB43IRHWEASCFBLKHURCGMHE5IF6XSE7EXDLACYHGRHM43RFOX");
        QVariantMap wrongLength{{"asset_type", "credit_alphanum4"}, {"asset_code", "ABCDE"}, {"asset_issuer", issuer}};
        QVERIFY_EXCEPTION_THROWN(assetFromVariantMap(wrongLength), std::runtime_error);
        QVariantMap tooShort{{"asset_type", "credit_alphanum12"}, {"asset_code", "ABC"}, {"asset_issuer", issuer}};
        QVERIFY_EXCEPTION_THROWN(assetFromVariantMap(tooShort), std::runtime_error);
        QVariantMap shares{{"asset_type", "liquidity_pool_shares"}};
        QVERIFY_EXCEPTION_THROWN(assetFromVariantMap(shares), std::runtime_error);
        QVariantMap nativeWithCode{{"asset_type", "native"}, {"asset_code", "XLM"}};
        QVERIFY_EXCEPTION_THROWN(assetFromVariantMap(nativeWithCode), std::runtime_error);
        QVariantMap twelve{{"asset_type", "credit_alphanum12"}, {"asset_code", "ABCDE"}, {"asset_issuer", issuer}};
        QScopedPointer<Asset> asset(assetFromVariantMap(twelve));
        QCOMPARE(asset->getType(), QString("credit_alphanum12"));
    }
};

ADD_TEST(AssetDeserializerTest)
#endif // ASSETDESERIALIZERTEST_H
