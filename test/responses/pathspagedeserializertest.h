#ifndef PATHSPAGEDESERIALIZERTEST_H
#define PATHSPAGEDESERIALIZERTEST_H
#include <QObject>


#include <QtTest>
#include "../testcollector.h"
#include "../../src/responses/page.h"
#include "../../src/responses/pathresponse.h"
#include "../../src/keypair.h"
#include "../../src/asset.h"
class PathsPageDeserializerTest: public QObject
{
    Q_OBJECT

public:


private slots:

    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }
    void testDeserialize() {
        Page<PathResponse> pathsPage;
        pathsPage.loadFromJson(json);

        QVERIFY(pathsPage.getNextPage()==nullptr);

        QCOMPARE(pathsPage.get(0).getDestinationAmount(), QString("20.0000000"));
        QVERIFY(pathsPage.get(0).getDestinationAsset().equals( Asset::createNonNativeAsset("EUR", KeyPair::fromAccountId(QString("GDSBCQO34HWPGUGQSP3QBFEXVTSR2PW46UIGTHVWGWJGQKH3AFNHXHXN")))));
        QCOMPARE(pathsPage.get(0).getPath().size(), 0);
        QCOMPARE(pathsPage.get(0).getSourceAmount(), QString("30.0000000"));
        QVERIFY(pathsPage.get(0).getSourceAsset().equals(Asset::createNonNativeAsset("USD", KeyPair::fromAccountId(QString("GDSBCQO34HWPGUGQSP3QBFEXVTSR2PW46UIGTHVWGWJGQKH3AFNHXHXN")))));

        QCOMPARE(pathsPage.get(1).getDestinationAmount(), QString("50.0000000"));
        QVERIFY(pathsPage.get(1).getDestinationAsset().equals( Asset::createNonNativeAsset("EUR", KeyPair::fromAccountId(QString("GBFMFKDUFYYITWRQXL4775CVUV3A3WGGXNJUAP4KTXNEQ2HG7JRBITGH")))));
        QCOMPARE(pathsPage.get(1).getPath().size(), 1);
        QVERIFY(pathsPage.get(1).getPath().at(0)->equals( Asset::createNonNativeAsset("GBP", KeyPair::fromAccountId(QString("GDSBCQO34HWPGUGQSP3QBFEXVTSR2PW46UIGTHVWGWJGQKH3AFNHXHXN")))));
        QCOMPARE(pathsPage.get(1).getSourceAmount(), QString("60.0000000"));
        QVERIFY(pathsPage.get(1).getSourceAsset().equals( Asset::createNonNativeAsset("USD", KeyPair::fromAccountId(QString("GBRAOXQDNQZRDIOK64HZI4YRDTBFWNUYH3OIHQLY4VEK5AIGMQHCLGXI")))));

        QCOMPARE(pathsPage.get(2).getDestinationAmount(), QString("200.0000000"));
        QVERIFY(pathsPage.get(2).getDestinationAsset().equals( Asset::createNonNativeAsset("EUR", KeyPair::fromAccountId(QString("GBRCOBK7C7UE72PB5JCPQU3ZI45ZCEM7HKQ3KYV3YD3XB7EBOPBEDN2G")))));
        QCOMPARE(pathsPage.get(2).getPath().size(), 2);
        QVERIFY(pathsPage.get(2).getPath().at(0)->equals( Asset::createNonNativeAsset("GBP", KeyPair::fromAccountId(QString("GAX7B3ZT3EOZW5POAMV4NGPPKCYUOYW2QQDIAF23JAXF72NMGRYPYOPM")))));
        QVERIFY(pathsPage.get(2).getPath().at(1)->equals( Asset::createNonNativeAsset("PLN", KeyPair::fromAccountId(QString("GACWIA2XGDFWWN3WKPX63JTK4S2J5NDPNOIVYMZY6RVTS7LWF2VHZLV3")))));
        QCOMPARE(pathsPage.get(2).getSourceAmount(), QString("300.0000000"));
        QVERIFY(pathsPage.get(2).getSourceAsset().equals(Asset::createNonNativeAsset("USD", KeyPair::fromAccountId(QString("GC7J5IHS3GABSX7AZLRINXWLHFTL3WWXLU4QX2UGSDEAIAQW2Q72U3KH")))));
    }
private:
    const QByteArray json = "{\n"
            "    \"_embedded\": {\n"
            "        \"records\": [\n"
            "            {\n"
            "                \"destination_amount\": \"20.0000000\",\n"
            "                \"destination_asset_code\": \"EUR\",\n"
            "                \"destination_asset_issuer\": \"GDSBCQO34HWPGUGQSP3QBFEXVTSR2PW46UIGTHVWGWJGQKH3AFNHXHXN\",\n"
            "                \"destination_asset_type\": \"credit_alphanum4\",\n"
            "                \"path\": [],\n"
            "                \"source_amount\": \"30.0000000\",\n"
            "                \"source_asset_code\": \"USD\",\n"
            "                \"source_asset_issuer\": \"GDSBCQO34HWPGUGQSP3QBFEXVTSR2PW46UIGTHVWGWJGQKH3AFNHXHXN\",\n"
            "                \"source_asset_type\": \"credit_alphanum4\"\n"
            "            },\n"
            "            {\n"
            "                \"destination_amount\": \"50.0000000\",\n"
            "                \"destination_asset_code\": \"EUR\",\n"
            "                \"destination_asset_issuer\": \"GBFMFKDUFYYITWRQXL4775CVUV3A3WGGXNJUAP4KTXNEQ2HG7JRBITGH\",\n"
            "                \"destination_asset_type\": \"credit_alphanum4\",\n"
            "                \"path\": [\n"
            "                    {\n"
            "                        \"asset_code\": \"GBP\",\n"
            "                        \"asset_issuer\": \"GDSBCQO34HWPGUGQSP3QBFEXVTSR2PW46UIGTHVWGWJGQKH3AFNHXHXN\",\n"
            "                        \"asset_type\": \"credit_alphanum4\"\n"
            "                    }\n"
            "                ],\n"
            "                \"source_amount\": \"60.0000000\",\n"
            "                \"source_asset_code\": \"USD\",\n"
            "                \"source_asset_issuer\": \"GBRAOXQDNQZRDIOK64HZI4YRDTBFWNUYH3OIHQLY4VEK5AIGMQHCLGXI\",\n"
            "                \"source_asset_type\": \"credit_alphanum4\"\n"
            "            },\n"
            "            {\n"
            "                \"destination_amount\": \"200.0000000\",\n"
            "                \"destination_asset_code\": \"EUR\",\n"
            "                \"destination_asset_issuer\": \"GBRCOBK7C7UE72PB5JCPQU3ZI45ZCEM7HKQ3KYV3YD3XB7EBOPBEDN2G\",\n"
            "                \"destination_asset_type\": \"credit_alphanum4\",\n"
            "                \"path\": [\n"
            "                    {\n"
            "                        \"asset_code\": \"GBP\",\n"
            "                        \"asset_issuer\": \"GAX7B3ZT3EOZW5POAMV4NGPPKCYUOYW2QQDIAF23JAXF72NMGRYPYOPM\",\n"
            "                        \"asset_type\": \"credit_alphanum4\"\n"
            "                    },\n"
            "                    {\n"
            "                        \"asset_code\": \"PLN\",\n"
            "                        \"asset_issuer\": \"GACWIA2XGDFWWN3WKPX63JTK4S2J5NDPNOIVYMZY6RVTS7LWF2VHZLV3\",\n"
            "                        \"asset_type\": \"credit_alphanum4\"\n"
            "                    }\n"
            "                ],\n"
            "                \"source_amount\": \"300.0000000\",\n"
            "                \"source_asset_code\": \"USD\",\n"
            "                \"source_asset_issuer\": \"GC7J5IHS3GABSX7AZLRINXWLHFTL3WWXLU4QX2UGSDEAIAQW2Q72U3KH\",\n"
            "                \"source_asset_type\": \"credit_alphanum4\"\n"
            "            }        ]\n"
            "    },\n"
            "    \"_links\": {\n"
            "        \"self\": {\n"
            "            \"href\": \"/paths\"\n"
            "        }\n"
            "    }\n"
            "}";

};





ADD_TEST(PathsPageDeserializerTest)
#endif // PATHSPAGEDESERIALIZERTEST_H
