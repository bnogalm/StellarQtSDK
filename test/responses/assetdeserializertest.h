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


};

ADD_TEST(AssetDeserializerTest)
#endif // ASSETDESERIALIZERTEST_H
