#ifndef OFFERPAGEDESERIALIZERTEST_H
#define OFFERPAGEDESERIALIZERTEST_H

#include <QObject>


#include <QtTest>
#include "../testcollector.h"
#include "../../src/responses/offerresponse.h"
#include "../../src/responses/page.h"
#include "../../src/keypair.h"
#include "../../src/asset.h"

class OfferPageDeserializerTest: public QObject
{
    Q_OBJECT
    const QByteArray json ="{\n"
            "  \"_links\": {\n"
            "    \"self\": {\n"
            "      \"href\": \"https://horizon-testnet.stellar.org/accounts/GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD/offers?order=asc\\u0026limit=10\\u0026cursor=\"\n"
            "    },\n"
            "    \"next\": {\n"
            "      \"href\": \"https://horizon-testnet.stellar.org/accounts/GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD/offers?order=asc\\u0026limit=10\\u0026cursor=241\"\n"
            "    },\n"
            "    \"prev\": {\n"
            "      \"href\": \"https://horizon-testnet.stellar.org/accounts/GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD/offers?order=desc\\u0026limit=10\\u0026cursor=241\"\n"
            "    }\n"
            "  },\n"
            "  \"_embedded\": {\n"
            "    \"records\": [\n"
            "      {\n"
            "        \"_links\": {\n"
            "          \"self\": {\n"
            "            \"href\": \"https://horizon-testnet.stellar.org/offers/241\"\n"
            "          },\n"
            "          \"offer_maker\": {\n"
            "            \"href\": \"https://horizon-testnet.stellar.org/accounts/GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD\"\n"
            "          }\n"
            "        },\n"
            "        \"id\": 241,\n"
            "        \"paging_token\": \"241\",\n"
            "        \"seller\": \"GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD\",\n"
            "        \"selling\": {\n"
            "          \"asset_type\": \"credit_alphanum4\",\n"
            "          \"asset_code\": \"INR\",\n"
            "          \"asset_issuer\": \"GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD\"\n"
            "        },\n"
            "        \"buying\": {\n"
            "          \"asset_type\": \"credit_alphanum4\",\n"
            "          \"asset_code\": \"USD\",\n"
            "          \"asset_issuer\": \"GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD\"\n"
            "        },\n"
            "        \"amount\": \"10.0000000\",\n"
            "        \"price_r\": {\n"
            "          \"n\": 10,\n"
            "          \"d\": 1\n"
            "        },\n"
            "        \"price\": \"11.0000000\"\n,"
            "        \"last_modified_ledger\": 22200794,\n"
            "        \"last_modified_time\": \"2019-01-28T12:30:38Z\"\n"
            "      }\n"
            "    ]\n"
            "  }\n"
  "}";


    QByteArray withSponsor = "{\n"
        "  \"_links\": {\n"
        "    \"self\": {\n"
        "      \"href\": \"https://horizon-testnet.stellar.org/accounts/GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD/offers?order=asc\\u0026limit=10\\u0026cursor=\"\n"
        "    },\n"
        "    \"next\": {\n"
        "      \"href\": \"https://horizon-testnet.stellar.org/accounts/GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD/offers?order=asc\\u0026limit=10\\u0026cursor=241\"\n"
        "    },\n"
        "    \"prev\": {\n"
        "      \"href\": \"https://horizon-testnet.stellar.org/accounts/GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD/offers?order=desc\\u0026limit=10\\u0026cursor=241\"\n"
        "    }\n"
        "  },\n"
        "  \"_embedded\": {\n"
        "    \"records\": [\n"
        "      {\n"
        "        \"_links\": {\n"
        "          \"self\": {\n"
        "            \"href\": \"https://horizon-testnet.stellar.org/offers/241\"\n"
        "          },\n"
        "          \"offer_maker\": {\n"
        "            \"href\": \"https://horizon-testnet.stellar.org/accounts/GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD\"\n"
        "          }\n"
        "        },\n"
        "        \"id\": 241,\n"
        "        \"paging_token\": \"241\",\n"
        "        \"seller\": \"GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD\",\n"
        "        \"selling\": {\n"
        "          \"asset_type\": \"credit_alphanum4\",\n"
        "          \"asset_code\": \"INR\",\n"
        "          \"asset_issuer\": \"GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD\"\n"
        "        },\n"
        "        \"buying\": {\n"
        "          \"asset_type\": \"credit_alphanum4\",\n"
        "          \"asset_code\": \"USD\",\n"
        "          \"asset_issuer\": \"GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD\"\n"
        "        },\n"
        "        \"amount\": \"10.0000000\",\n"
        "        \"price_r\": {\n"
        "          \"n\": 10,\n"
        "          \"d\": 1\n"
        "        },\n"
        "        \"price\": \"11.0000000\"\n,"
        "        \"last_modified_ledger\": 22200794,\n"
        "        \"sponsor\": \"GCA7RXNKN7FGBLJVETJCUUXGXTCR6L2SJQFXDGMQCDET5YUE6KFNHQHO\",\n"
        "        \"last_modified_time\": \"2019-01-28T12:30:38Z\"\n"
        "      }\n"
        "    ]\n"
        "  }\n"
        "}";
public:


private slots:

    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }
    void testDeserialize() {
        Page<OfferResponse> offerPage;
        offerPage.loadFromJson(json);

        QCOMPARE(offerPage.get(0).getId(), 241);
        QCOMPARE(offerPage.get(0).getSeller()->getAccountId(), QString("GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD"));
        QCOMPARE(offerPage.get(0).getPagingToken(), QString("241"));
        QVERIFY(offerPage.get(0).getSelling()->equals(Asset::createNonNativeAsset("INR", KeyPair::fromAccountId(QString("GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD")))));
        QVERIFY(offerPage.get(0).getBuying()->equals( Asset::createNonNativeAsset("USD", KeyPair::fromAccountId(QString("GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD")))));
        QCOMPARE(offerPage.get(0).getAmount(), QString("10.0000000"));
        QCOMPARE(offerPage.get(0).getPrice(), QString("11.0000000"));
        QCOMPARE(offerPage.get(0).getLastModifiedLedger(), 22200794);
        QCOMPARE(offerPage.get(0).getLastModifiedTime(), "2019-01-28T12:30:38Z");
        QCOMPARE(offerPage.getLinks().getNext().getHref(), QString("https://horizon-testnet.stellar.org/accounts/GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD/offers?order=asc&limit=10&cursor=241"));
        QCOMPARE(offerPage.getLinks().getPrev().getHref(), QString("https://horizon-testnet.stellar.org/accounts/GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD/offers?order=desc&limit=10&cursor=241"));
        QCOMPARE(offerPage.getLinks().getSelf().getHref(), QString("https://horizon-testnet.stellar.org/accounts/GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD/offers?order=asc&limit=10&cursor="));
        QVERIFY(offerPage.get(0).getSponsor().isNull());

    }
    void testDeserializeWithSponsor() {
       Page<OfferResponse> offerPage;
       offerPage.loadFromJson(withSponsor);

       QCOMPARE(offerPage.get(0).getId(),  241);
       QCOMPARE(offerPage.get(0).getSeller()->getAccountId(), QString("GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD"));
       QCOMPARE(offerPage.get(0).getPagingToken(), "241");
       QVERIFY(offerPage.get(0).getSelling()->equals(Asset::createNonNativeAsset("INR", "GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD")));
       QVERIFY(offerPage.get(0).getBuying()->equals(Asset::createNonNativeAsset("USD", "GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD")));
       QCOMPARE(offerPage.get(0).getAmount(), "10.0000000");
       QCOMPARE(offerPage.get(0).getPrice(), "11.0000000");
       QCOMPARE(offerPage.get(0).getLastModifiedLedger(), 22200794);
       QCOMPARE(offerPage.get(0).getLastModifiedTime(), "2019-01-28T12:30:38Z");
       QCOMPARE(offerPage.get(0).getSponsor(), "GCA7RXNKN7FGBLJVETJCUUXGXTCR6L2SJQFXDGMQCDET5YUE6KFNHQHO");

       QCOMPARE(offerPage.getLinks().getNext().getHref(), "https://horizon-testnet.stellar.org/accounts/GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD/offers?order=asc&limit=10&cursor=241");
       QCOMPARE(offerPage.getLinks().getPrev().getHref(), "https://horizon-testnet.stellar.org/accounts/GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD/offers?order=desc&limit=10&cursor=241");
       QCOMPARE(offerPage.getLinks().getSelf().getHref(), "https://horizon-testnet.stellar.org/accounts/GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD/offers?order=asc&limit=10&cursor=");
     }

};

ADD_TEST(OfferPageDeserializerTest)



#endif // OFFERPAGEDESERIALIZERTEST_H
