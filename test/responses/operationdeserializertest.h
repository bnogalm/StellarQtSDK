#ifndef OPERATIONDESERIALIZERTEST_H
#define OPERATIONDESERIALIZERTEST_H
#include <QObject>


#include <QtTest>
#include "../testcollector.h"
#include "../../src/responses/operations/accountmergeoperationresponse.h"
#include "../../src/responses/operations/createaccountoperationresponse.h"
#include "../../src/responses/operations/paymentoperationresponse.h"
#include "../../src/responses/operations/pathpaymentoperationresponse.h"

#include "../../src/responses/operations/pathpaymentstrictreceiveoperationresponse.h"
#include "../../src/responses/operations/pathpaymentstrictsendoperationresponse.h"

#include "../../src/responses/operations/inflationoperationresponse.h"
#include "../../src/responses/operations/allowtrustoperationresponse.h"
#include "../../src/responses/operations/changetrustoperationresponse.h"
#include "../../src/responses/operations/createpassiveofferoperationresponse.h"
#include "../../src/responses/operations/managedataoperationresponse.h"
#include "../../src/responses/operations/setoptionsoperationresponse.h"
#include "../../src/responses/operations/managesellofferoperationresponse.h"
#include "../../src/responses/operations/managebuyofferoperationresponse.h"
#include "../../src/responses/operations/bumpsequenceoperationresponse.h"

#include "../../src/responses/operations/clawbackclaimablebalanceoperationresponse.h"
#include "../../src/responses/operations/clawbackoperationresponse.h"
#include "../../src/responses/operations/settrustlineflagsoperationresponse.h"

#include "../../src/responses/genericoperation.h"

#include "../../src/asset.h"
#include "../../src/assettypenative.h"
#include "../../src/keypair.h"
class OperationDeserializerTest: public QObject
{
    Q_OBJECT
private slots:
    void initTestCase()
    {
    }
    void cleanupTestCase()
    {
    }

    void testDeserializeGenericCreateAccountOperation(){
        QByteArray json = "{\n"
                "  \"_links\": {\n"
                "    \"effects\": {\n"
                "      \"href\": \"/operations/3936840037961729/effects{?cursor,limit,order}\",\n"
                "      \"templated\": true\n"
                "    },\n"
                "    \"precedes\": {\n"
                "      \"href\": \"/operations?cursor=3936840037961729\\u0026order=asc\"\n"
                "    },\n"
                "    \"self\": {\n"
                "      \"href\": \"/operations/3936840037961729\"\n"
                "    },\n"
                "    \"succeeds\": {\n"
                "      \"href\": \"/operations?cursor=3936840037961729\\u0026order=desc\"\n"
                "    },\n"
                "    \"transaction\": {\n"
                "      \"href\": \"/transactions/75608563ae63757ffc0650d84d1d13c0f3cd4970a294a2a6b43e3f454e0f9e6d\"\n"
                "    }\n"
                "  },\n"
                "  \"account\": \"GAR4DDXYNSN2CORG3XQFLAPWYKTUMLZYHYWV4Y2YJJ4JO6ZJFXMJD7PT\",\n"
                "  \"funder\": \"GD6WU64OEP5C4LRBH6NK3MHYIA2ADN6K6II6EXPNVUR3ERBXT4AN4ACD\",\n"
                "  \"id\": 3936840037961729,\n"
                "  \"paging_token\": \"3936840037961729\",\n"
                "  \"source_account\": \"GD6WU64OEP5C4LRBH6NK3MHYIA2ADN6K6II6EXPNVUR3ERBXT4AN4ACD\",\n"
                "  \"starting_balance\": \"299454.904954\",\n"
                "  \"type\": \"create_account\",\n"
                "  \"type_i\": 0\n"
                "}";
        GenericOperation genericOperation;
        genericOperation.loadFromJson(json);
        CreateAccountOperationResponse& operation= * static_cast<CreateAccountOperationResponse*>(genericOperation.operation());

        QCOMPARE(operation.getSourceAccount().getAccountId(), QString("GD6WU64OEP5C4LRBH6NK3MHYIA2ADN6K6II6EXPNVUR3ERBXT4AN4ACD"));
        QCOMPARE(operation.getPagingToken(), QString("3936840037961729"));
        QCOMPARE(operation.getId(), 3936840037961729L);

        QCOMPARE(operation.getAccount().getAccountId(), QString("GAR4DDXYNSN2CORG3XQFLAPWYKTUMLZYHYWV4Y2YJJ4JO6ZJFXMJD7PT"));
        QCOMPARE(operation.getStartingBalance(), QString("299454.904954"));
        QCOMPARE(operation.getFunder().getAccountId(), QString("GD6WU64OEP5C4LRBH6NK3MHYIA2ADN6K6II6EXPNVUR3ERBXT4AN4ACD"));

        QCOMPARE(operation.getLinks().getEffects().getHref(), QString("/operations/3936840037961729/effects{?cursor,limit,order}"));
        QCOMPARE(operation.getLinks().getPrecedes().getHref(), QString("/operations?cursor=3936840037961729&order=asc"));
        QCOMPARE(operation.getLinks().getSelf().getHref(), QString("/operations/3936840037961729"));
        QCOMPARE(operation.getLinks().getSucceeds().getHref(), QString("/operations?cursor=3936840037961729&order=desc"));
        QCOMPARE(operation.getLinks().getTransaction().getHref(), QString("/transactions/75608563ae63757ffc0650d84d1d13c0f3cd4970a294a2a6b43e3f454e0f9e6d"));
    }
    void testDeserializeCreateAccountOperation() {
        QByteArray json = "{\n"
                "  \"_links\": {\n"
                "    \"effects\": {\n"
                "      \"href\": \"/operations/3936840037961729/effects{?cursor,limit,order}\",\n"
                "      \"templated\": true\n"
                "    },\n"
                "    \"precedes\": {\n"
                "      \"href\": \"/operations?cursor=3936840037961729\\u0026order=asc\"\n"
                "    },\n"
                "    \"self\": {\n"
                "      \"href\": \"/operations/3936840037961729\"\n"
                "    },\n"
                "    \"succeeds\": {\n"
                "      \"href\": \"/operations?cursor=3936840037961729\\u0026order=desc\"\n"
                "    },\n"
                "    \"transaction\": {\n"
                "      \"href\": \"/transactions/75608563ae63757ffc0650d84d1d13c0f3cd4970a294a2a6b43e3f454e0f9e6d\"\n"
                "    }\n"
                "  },\n"
                "  \"account\": \"GAR4DDXYNSN2CORG3XQFLAPWYKTUMLZYHYWV4Y2YJJ4JO6ZJFXMJD7PT\",\n"
                "  \"funder\": \"GD6WU64OEP5C4LRBH6NK3MHYIA2ADN6K6II6EXPNVUR3ERBXT4AN4ACD\",\n"
                "  \"id\": 3936840037961729,\n"
                "  \"paging_token\": \"3936840037961729\",\n"
                "  \"source_account\": \"GD6WU64OEP5C4LRBH6NK3MHYIA2ADN6K6II6EXPNVUR3ERBXT4AN4ACD\",\n"
                "  \"starting_balance\": \"299454.904954\",\n"
                "  \"type\": \"create_account\",\n"
                "  \"type_i\": 0\n"
                "}";
        CreateAccountOperationResponse operation;
        operation.loadFromJson(json);

        QCOMPARE(operation.getSourceAccount().getAccountId(), QString("GD6WU64OEP5C4LRBH6NK3MHYIA2ADN6K6II6EXPNVUR3ERBXT4AN4ACD"));
        QCOMPARE(operation.getPagingToken(), QString("3936840037961729"));
        QCOMPARE(operation.getId(), 3936840037961729L);

        QVERIFY(operation.isTransactionSuccessful().isNull());

        QCOMPARE(operation.getAccount().getAccountId(), QString("GAR4DDXYNSN2CORG3XQFLAPWYKTUMLZYHYWV4Y2YJJ4JO6ZJFXMJD7PT"));
        QCOMPARE(operation.getStartingBalance(), QString("299454.904954"));
        QCOMPARE(operation.getFunder().getAccountId(), QString("GD6WU64OEP5C4LRBH6NK3MHYIA2ADN6K6II6EXPNVUR3ERBXT4AN4ACD"));

        QCOMPARE(operation.getLinks().getEffects().getHref(), QString("/operations/3936840037961729/effects{?cursor,limit,order}"));
        QCOMPARE(operation.getLinks().getPrecedes().getHref(), QString("/operations?cursor=3936840037961729&order=asc"));
        QCOMPARE(operation.getLinks().getSelf().getHref(), QString("/operations/3936840037961729"));
        QCOMPARE(operation.getLinks().getSucceeds().getHref(), QString("/operations?cursor=3936840037961729&order=desc"));
        QCOMPARE(operation.getLinks().getTransaction().getHref(), QString("/transactions/75608563ae63757ffc0650d84d1d13c0f3cd4970a294a2a6b43e3f454e0f9e6d"));
    }
    void testDeserializePaymentOperation() {
        QByteArray json = "{\n"
                "        \"_links\": {\n"
                "          \"effects\": {\n"
                "            \"href\": \"/operations/3940808587743233/effects{?cursor,limit,order}\",\n"
                "            \"templated\": true\n"
                "          },\n"
                "          \"precedes\": {\n"
                "            \"href\": \"/operations?cursor=3940808587743233\\u0026order=asc\"\n"
                "          },\n"
                "          \"self\": {\n"
                "            \"href\": \"/operations/3940808587743233\"\n"
                "          },\n"
                "          \"succeeds\": {\n"
                "            \"href\": \"/operations?cursor=3940808587743233\\u0026order=desc\"\n"
                "          },\n"
                "          \"transaction\": {\n"
                "            \"href\": \"/transactions/9a140b8942da440c27b65658061c2d7fafe4d0de8424fa832568f3282793fa33\"\n"
                "          }\n"
                "        },\n"
                "        \"amount\": \"100.0\",\n"
                "        \"asset_type\": \"native\",\n"
                "        \"transaction_successful\": false,\n"
                "        \"from\": \"GB6NVEN5HSUBKMYCE5ZOWSK5K23TBWRUQLZY3KNMXUZ3AQ2ESC4MY4AQ\",\n"
                "        \"id\": 3940808587743233,\n"
                "        \"paging_token\": \"3940808587743233\",\n"
                "        \"source_account\": \"GB6NVEN5HSUBKMYCE5ZOWSK5K23TBWRUQLZY3KNMXUZ3AQ2ESC4MY4AQ\",\n"
                "        \"to\": \"GDWNY2POLGK65VVKIH5KQSH7VWLKRTQ5M6ADLJAYC2UEHEBEARCZJWWI\",\n"
                "        \"type\": \"payment\",\n"
                "        \"type_i\": 1\n"
                "      }";
        PaymentOperationResponse operation;
        operation.loadFromJson(json);
        QCOMPARE(operation.getSourceAccount().getAccountId(), QString("GB6NVEN5HSUBKMYCE5ZOWSK5K23TBWRUQLZY3KNMXUZ3AQ2ESC4MY4AQ"));
        QCOMPARE(operation.getId(), 3940808587743233L);
        QVERIFY(!operation.isTransactionSuccessful());
        QVERIFY(!operation.isTransactionSuccessful().isNull());
        QCOMPARE(operation.isTransactionSuccessful(),Boolean(false));
        QCOMPARE(operation.getFrom().getAccountId(), QString("GB6NVEN5HSUBKMYCE5ZOWSK5K23TBWRUQLZY3KNMXUZ3AQ2ESC4MY4AQ"));
        QCOMPARE(operation.getTo().getAccountId(), QString("GDWNY2POLGK65VVKIH5KQSH7VWLKRTQ5M6ADLJAYC2UEHEBEARCZJWWI"));
        QCOMPARE(operation.getAmount(), QString("100.0"));
        QVERIFY(operation.getAsset()->equals(new AssetTypeNative()));

    }
    void testDeserializeNonNativePaymentOperation() {
        QByteArray json = "{\n"
                "        \"_links\": {\n"
                "          \"self\": {\n"
                "            \"href\": \"//horizon-testnet.stellar.org/operations/3603043769651201\"\n"
                "          },\n"
                "          \"transaction\": {\n"
                "            \"href\": \"//horizon-testnet.stellar.org/transactions/b59dee0f85bc7efdefa1a6eec7a0b6f602d567cca6e7f587056d41d42ca48879\"\n"
                "          },\n"
                "          \"effects\": {\n"
                "            \"href\": \"//horizon-testnet.stellar.org/operations/3603043769651201/effects\"\n"
                "          },\n"
                "          \"succeeds\": {\n"
                "            \"href\": \"//horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=3603043769651201\"\n"
                "          },\n"
                "          \"precedes\": {\n"
                "            \"href\": \"//horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=3603043769651201\"\n"
                "          }\n"
                "        },\n"
                "        \"id\": \"3603043769651201\",\n"
                "        \"paging_token\": \"3603043769651201\",\n"
                "        \"transaction_successful\": true,\n"
                "        \"source_account\": \"GAZN3PPIDQCSP5JD4ETQQQ2IU2RMFYQTAL4NNQZUGLLO2XJJJ3RDSDGA\",\n"
                "        \"type\": \"payment\",\n"
                "        \"type_i\": 1,\n"
                "        \"asset_type\": \"credit_alphanum4\",\n"
                "        \"asset_code\": \"EUR\",\n"
                "        \"asset_issuer\": \"GAZN3PPIDQCSP5JD4ETQQQ2IU2RMFYQTAL4NNQZUGLLO2XJJJ3RDSDGA\",\n"
                "        \"from\": \"GAZN3PPIDQCSP5JD4ETQQQ2IU2RMFYQTAL4NNQZUGLLO2XJJJ3RDSDGA\",\n"
                "        \"to\": \"GBHUSIZZ7FS2OMLZVZ4HLWJMXQ336NFSXHYERD7GG54NRITDTEWWBBI6\",\n"
                "        \"amount\": \"1000000000.0\"\n"
                "      }";

        PaymentOperationResponse operation;
        operation.loadFromJson(json);

        QVERIFY(operation.isTransactionSuccessful());
        QVERIFY(!operation.isTransactionSuccessful().isNull());
        QCOMPARE(operation.isTransactionSuccessful(),Boolean(true));

        QCOMPARE(operation.getFrom().getAccountId(), QString("GAZN3PPIDQCSP5JD4ETQQQ2IU2RMFYQTAL4NNQZUGLLO2XJJJ3RDSDGA"));
        QCOMPARE(operation.getTo().getAccountId(), QString("GBHUSIZZ7FS2OMLZVZ4HLWJMXQ336NFSXHYERD7GG54NRITDTEWWBBI6"));
        QCOMPARE(operation.getAmount(), QString("1000000000.0"));
        QVERIFY(operation.getAsset()->equals(Asset::createNonNativeAsset("EUR", KeyPair::fromAccountId(QString("GAZN3PPIDQCSP5JD4ETQQQ2IU2RMFYQTAL4NNQZUGLLO2XJJJ3RDSDGA")))));
    }
    void testDeserializeAllowTrustOperation() {
        QByteArray json = "{\n"
                "        \"_links\": {\n"
                "          \"self\": {\n"
                "            \"href\": \"//horizon-testnet.stellar.org/operations/3602979345141761\"\n"
                "          },\n"
                "          \"transaction\": {\n"
                "            \"href\": \"//horizon-testnet.stellar.org/transactions/1f265c075e8559ee4c21a32ae53337658e52d7504841adad4144c37143ea01a2\"\n"
                "          },\n"
                "          \"effects\": {\n"
                "            \"href\": \"//horizon-testnet.stellar.org/operations/3602979345141761/effects\"\n"
                "          },\n"
                "          \"succeeds\": {\n"
                "            \"href\": \"//horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=3602979345141761\"\n"
                "          },\n"
                "          \"precedes\": {\n"
                "            \"href\": \"//horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=3602979345141761\"\n"
                "          }\n"
                "        },\n"
                "        \"id\": \"3602979345141761\",\n"
                "        \"paging_token\": \"3602979345141761\",\n"
                "        \"source_account\": \"GDIROJW2YHMSFZJJ4R5XWWNUVND5I45YEWS5DSFKXCHMADZ5V374U2LM\",\n"
                "        \"type\": \"allow_trust\",\n"
                "        \"type_i\": 7,\n"
                "        \"asset_type\": \"credit_alphanum4\",\n"
                "        \"asset_code\": \"EUR\",\n"
                "        \"asset_issuer\": \"GDIROJW2YHMSFZJJ4R5XWWNUVND5I45YEWS5DSFKXCHMADZ5V374U2LM\",\n"
                "        \"trustee\": \"GDIROJW2YHMSFZJJ4R5XWWNUVND5I45YEWS5DSFKXCHMADZ5V374U2LM\",\n"
                "        \"trustor\": \"GDZ55LVXECRTW4G36EZPTHI4XIYS5JUC33TUS22UOETVFVOQ77JXWY4F\",\n"
                "        \"authorize\": true,\n"
                "        \"authorize_to_maintain_liabilities\": false\n"
                "      }";

        AllowTrustOperationResponse operation;
        operation.loadFromJson(json);
        QCOMPARE(operation.getTrustee().getAccountId(), QString("GDIROJW2YHMSFZJJ4R5XWWNUVND5I45YEWS5DSFKXCHMADZ5V374U2LM"));
        QCOMPARE(operation.getTrustor().getAccountId(), QString("GDZ55LVXECRTW4G36EZPTHI4XIYS5JUC33TUS22UOETVFVOQ77JXWY4F"));
        QCOMPARE(operation.isAuthorize(), true);
        QCOMPARE(operation.isAuthorizedToMaintainLiabilities(), false);
        QVERIFY(operation.getAsset()->equals(Asset::createNonNativeAsset("EUR", KeyPair::fromAccountId(QString("GDIROJW2YHMSFZJJ4R5XWWNUVND5I45YEWS5DSFKXCHMADZ5V374U2LM")))));
    }

    void testDeserializeAllowTrustOperationAuthorizeToMaintainLiabilities()
    {
        QByteArray json = "{\n"
            "        \"_links\": {\n"
            "          \"self\": {\n"
            "            \"href\": \"//horizon-testnet.stellar.org/operations/3602979345141761\"\n"
            "          },\n"
            "          \"transaction\": {\n"
            "            \"href\": \"//horizon-testnet.stellar.org/transactions/1f265c075e8559ee4c21a32ae53337658e52d7504841adad4144c37143ea01a2\"\n"
            "          },\n"
            "          \"effects\": {\n"
            "            \"href\": \"//horizon-testnet.stellar.org/operations/3602979345141761/effects\"\n"
            "          },\n"
            "          \"succeeds\": {\n"
            "            \"href\": \"//horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=3602979345141761\"\n"
            "          },\n"
            "          \"precedes\": {\n"
            "            \"href\": \"//horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=3602979345141761\"\n"
            "          }\n"
            "        },\n"
            "        \"id\": \"3602979345141761\",\n"
            "        \"paging_token\": \"3602979345141761\",\n"
            "        \"source_account\": \"GDIROJW2YHMSFZJJ4R5XWWNUVND5I45YEWS5DSFKXCHMADZ5V374U2LM\",\n"
            "        \"type\": \"allow_trust\",\n"
            "        \"type_i\": 7,\n"
            "        \"asset_type\": \"credit_alphanum4\",\n"
            "        \"asset_code\": \"EUR\",\n"
            "        \"asset_issuer\": \"GDIROJW2YHMSFZJJ4R5XWWNUVND5I45YEWS5DSFKXCHMADZ5V374U2LM\",\n"
            "        \"trustee\": \"GDIROJW2YHMSFZJJ4R5XWWNUVND5I45YEWS5DSFKXCHMADZ5V374U2LM\",\n"
            "        \"trustor\": \"GDZ55LVXECRTW4G36EZPTHI4XIYS5JUC33TUS22UOETVFVOQ77JXWY4F\",\n"
            "        \"authorize\": false\n,"
            "        \"authorize_to_maintain_liabilities\": true\n"
            "      }";

        AllowTrustOperationResponse operation;
        operation.loadFromJson(json);

        QCOMPARE(operation.getTrustee().getAccountId(), "GDIROJW2YHMSFZJJ4R5XWWNUVND5I45YEWS5DSFKXCHMADZ5V374U2LM");
        QCOMPARE(operation.getTrustor().getAccountId(), "GDZ55LVXECRTW4G36EZPTHI4XIYS5JUC33TUS22UOETVFVOQ77JXWY4F");
        QCOMPARE(operation.isAuthorize(), false);
        QCOMPARE(operation.isAuthorizedToMaintainLiabilities(), true);
        QVERIFY(operation.getAsset()->equals(Asset::createNonNativeAsset("EUR", "GDIROJW2YHMSFZJJ4R5XWWNUVND5I45YEWS5DSFKXCHMADZ5V374U2LM")));
      }

    void testDeserializeChangeTrustOperation() {
        QByteArray json = "{\n"
                "        \"_links\": {\n"
                "          \"self\": {\n"
                "            \"href\": \"//horizon-testnet.stellar.org/operations/3602970755207169\"\n"
                "          },\n"
                "          \"transaction\": {\n"
                "            \"href\": \"//horizon-testnet.stellar.org/transactions/8d409a788543895843d269c3f97a2d6a2ebca6e9f8f9a7ae593457b5c0ba6644\"\n"
                "          },\n"
                "          \"effects\": {\n"
                "            \"href\": \"//horizon-testnet.stellar.org/operations/3602970755207169/effects\"\n"
                "          },\n"
                "          \"succeeds\": {\n"
                "            \"href\": \"//horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=3602970755207169\"\n"
                "          },\n"
                "          \"precedes\": {\n"
                "            \"href\": \"//horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=3602970755207169\"\n"
                "          }\n"
                "        },\n"
                "        \"id\": \"3602970755207169\",\n"
                "        \"paging_token\": \"3602970755207169\",\n"
                "        \"source_account\": \"GDZ55LVXECRTW4G36EZPTHI4XIYS5JUC33TUS22UOETVFVOQ77JXWY4F\",\n"
                "        \"type\": \"change_trust\",\n"
                "        \"type_i\": 6,\n"
                "        \"asset_type\": \"credit_alphanum4\",\n"
                "        \"asset_code\": \"EUR\",\n"
                "        \"asset_issuer\": \"GDIROJW2YHMSFZJJ4R5XWWNUVND5I45YEWS5DSFKXCHMADZ5V374U2LM\",\n"
                "        \"limit\": \"922337203685.4775807\",\n"
                "        \"trustee\": \"GDIROJW2YHMSFZJJ4R5XWWNUVND5I45YEWS5DSFKXCHMADZ5V374U2LM\",\n"
                "        \"trustor\": \"GDZ55LVXECRTW4G36EZPTHI4XIYS5JUC33TUS22UOETVFVOQ77JXWY4F\"\n"
                "      }";

        ChangeTrustOperationResponse operation;
        operation.loadFromJson(json);
        QCOMPARE(operation.getTrustee().getAccountId(), QString("GDIROJW2YHMSFZJJ4R5XWWNUVND5I45YEWS5DSFKXCHMADZ5V374U2LM"));
        QCOMPARE(operation.getTrustor().getAccountId(), QString("GDZ55LVXECRTW4G36EZPTHI4XIYS5JUC33TUS22UOETVFVOQ77JXWY4F"));
        QCOMPARE(operation.getLimit(), QString("922337203685.4775807"));
        QVERIFY(operation.getAsset()->equals(Asset::createNonNativeAsset("EUR", KeyPair::fromAccountId(QString("GDIROJW2YHMSFZJJ4R5XWWNUVND5I45YEWS5DSFKXCHMADZ5V374U2LM")))));
    }
    void testDeserializeSetOptionsOperation() {
        QByteArray json = "{\n"
                "        \"_links\": {\n"
                "          \"self\": {\n"
                "            \"href\": \"http://horizon-testnet.stellar.org/operations/1253868457431041\"\n"
                "          },\n"
                "          \"transaction\": {\n"
                "            \"href\": \"http://horizon-testnet.stellar.org/transactions/cc392abf8a4e649f16eeba4f40c43a8d50001b14a98ccfc639783125950e99d8\"\n"
                "          },\n"
                "          \"effects\": {\n"
                "            \"href\": \"http://horizon-testnet.stellar.org/operations/1253868457431041/effects\"\n"
                "          },\n"
                "          \"succeeds\": {\n"
                "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=1253868457431041\"\n"
                "          },\n"
                "          \"precedes\": {\n"
                "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=1253868457431041\"\n"
                "          }\n"
                "        },\n"
                "        \"id\": \"1253868457431041\",\n"
                "        \"paging_token\": \"1253868457431041\",\n"
                "        \"source_account\": \"GB6FDI5Q46BJVPNVWJU527NH463N3EF3S6TXRA2YSCZHXANY3YLXB7MC\",\n"
                "        \"type\": \"set_options\",\n"
                "        \"type_i\": 5,\n"
                "        \"signer_key\": \"GD3ZYXVC7C3ECD5I4E5NGPBFJJSULJ6HJI2FBHGKYFV34DSIWB4YEKJZ\",\n"
                "        \"signer_weight\": 1,\n"
                "        \"home_domain\": \"stellar.org\","
                "        \"inflation_dest\": \"GBYWSY4NPLLPTP22QYANGTT7PEHND64P4D4B6LFEUHGUZRVYJK2H4TBE\","
                "        \"low_threshold\": 1,\n"
                "        \"med_threshold\": 2,\n"
                "        \"high_threshold\": 3,\n"
                "        \"master_key_weight\": 4,\n"
                "        \"set_flags_s\": [\n"
                "          \"auth_required_flag\"\n"
                "        ],"
                "        \"clear_flags_s\": [\n"
                "          \"auth_revocable_flag\"\n"
                "        ]"
                "      }";

        SetOptionsOperationResponse operation;
        operation.loadFromJson(json);

        //QCOMPARE(operation.getSigner()->getAccountId(), QString("GD3ZYXVC7C3ECD5I4E5NGPBFJJSULJ6HJI2FBHGKYFV34DSIWB4YEKJZ"));
        QCOMPARE(operation.getSignerKey(), QString("GD3ZYXVC7C3ECD5I4E5NGPBFJJSULJ6HJI2FBHGKYFV34DSIWB4YEKJZ"));
        QCOMPARE(operation.getSignerWeight(), quint32(1));
        QCOMPARE(operation.getHomeDomain(), QString("stellar.org"));
        QCOMPARE(operation.getInflationDestination()->getAccountId(), QString("GBYWSY4NPLLPTP22QYANGTT7PEHND64P4D4B6LFEUHGUZRVYJK2H4TBE"));
        QCOMPARE(operation.getLowThreshold(), quint32(1));
        QCOMPARE(operation.getMedThreshold(), quint32(2));
        QCOMPARE(operation.getHighThreshold(), quint32(3));
        QCOMPARE(operation.getMasterKeyWeight(), quint32(4));
        QCOMPARE(operation.getSetFlags()[0], QString("auth_required_flag"));
        QCOMPARE(operation.getClearFlags()[0], QString("auth_revocable_flag"));
    }
    void testDeserializeSetOptionsOperationWithNonEd25519Key() {
        QByteArray json = "{\n"
                          "        \"_links\": {\n"
                          "          \"self\": {\n"
                          "            \"href\": \"https://horizon-testnet.stellar.org/operations/44921793093312513\"\n"
                          "          },\n"
                          "          \"transaction\": {\n"
                          "            \"href\": \"https://horizon-testnet.stellar.org/transactions/d991075183f7740e1aa43700b824f2f404082632f1db9d8a54db00574f83393b\"\n"
                          "          },\n"
                          "          \"effects\": {\n"
                          "            \"href\": \"https://horizon-testnet.stellar.org/operations/44921793093312513/effects\"\n"
                          "          },\n"
                          "          \"succeeds\": {\n"
                          "            \"href\": \"https://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=44921793093312513\"\n"
                          "          },\n"
                          "          \"precedes\": {\n"
                          "            \"href\": \"https://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=44921793093312513\"\n"
                          "          }\n"
                          "        },\n"
                          "        \"id\": \"44921793093312513\",\n"
                          "        \"paging_token\": \"44921793093312513\",\n"
                          "        \"source_account\": \"GCWYUHCMWC2AATGAXXYZX7T45QZLTRCYNJDD3PC73NEMUXBOCO5F6T6Z\",\n"
                          "        \"type\": \"set_options\",\n"
                          "        \"type_i\": 5,\n"
                          "        \"created_at\": \"2018-08-09T15:36:24Z\",\n"
                          "        \"transaction_hash\": \"d991075183f7740e1aa43700b824f2f404082632f1db9d8a54db00574f83393b\",\n"
                          "        \"signer_key\": \"TBGFYVCU76LJ7GZOCGR4X7DG2NV42JPG5CKRL42LA5FZOFI3U2WU7ZAL\",\n"
                          "        \"signer_weight\": 1\n"
                          "      }";
        SetOptionsOperationResponse operation;
        operation.loadFromJson(json);
        try {
#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
            operation.getSigner();
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif
            QFAIL("not known key type signer");
        } catch (std::runtime_error e) {
            //QCOMPARE(QString::fromLatin1(e.what()),QString("Version byte is invalid"));
        }
        QCOMPARE(operation.getSignerKey(), QString("TBGFYVCU76LJ7GZOCGR4X7DG2NV42JPG5CKRL42LA5FZOFI3U2WU7ZAL"));
    }
    void testDeserializeAccountMergeOperation() {
        QByteArray json = "{\n"
                "        \"_links\": {\n"
                "          \"self\": {\n"
                "            \"href\": \"http://horizon-testnet.stellar.org/operations/3424497684189185\"\n"
                "          },\n"
                "          \"transaction\": {\n"
                "            \"href\": \"http://horizon-testnet.stellar.org/transactions/68f4d37780e2a85f5698b73977126a595dff99aed852f14a7eb39221ce1f96d5\"\n"
                "          },\n"
                "          \"effects\": {\n"
                "            \"href\": \"http://horizon-testnet.stellar.org/operations/3424497684189185/effects\"\n"
                "          },\n"
                "          \"succeeds\": {\n"
                "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=3424497684189185\"\n"
                "          },\n"
                "          \"precedes\": {\n"
                "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=3424497684189185\"\n"
                "          }\n"
                "        },\n"
                "        \"id\": \"3424497684189185\",\n"
                "        \"paging_token\": \"3424497684189185\",\n"
                "        \"source_account\": \"GD6GKRABNDVYDETEZJQEPS7IBQMERCN44R5RCI4LJNX6BMYQM2KPGGZ2\",\n"
                "        \"type\": \"account_merge\",\n"
                "        \"type_i\": 8,\n"
                "        \"account\": \"GD6GKRABNDVYDETEZJQEPS7IBQMERCN44R5RCI4LJNX6BMYQM2KPGGZ2\",\n"
                "        \"into\": \"GAZWSWPDQTBHFIPBY4FEDFW2J6E2LE7SZHJWGDZO6Q63W7DBSRICO2KN\"\n"
                "      }";

        AccountMergeOperationResponse operation;
        operation.loadFromJson(json);
        QCOMPARE(operation.getAccount().getAccountId(), QString("GD6GKRABNDVYDETEZJQEPS7IBQMERCN44R5RCI4LJNX6BMYQM2KPGGZ2"));
        QCOMPARE(operation.getInto().getAccountId(), QString("GAZWSWPDQTBHFIPBY4FEDFW2J6E2LE7SZHJWGDZO6Q63W7DBSRICO2KN"));
    }

    void testDeserializeManageSellOfferOperation() {
        QByteArray json = "{\n"
                "        \"_links\": {\n"
                "          \"self\": {\n"
                "            \"href\": \"http://horizon-testnet.stellar.org/operations/3320426331639809\"\n"
                "          },\n"
                "          \"transaction\": {\n"
                "            \"href\": \"http://horizon-testnet.stellar.org/transactions/1f8fc03b26110e917d124381645d7dcf85927f17e46d8390d254a0bd99cfb0ad\"\n"
                "          },\n"
                "          \"effects\": {\n"
                "            \"href\": \"http://horizon-testnet.stellar.org/operations/3320426331639809/effects\"\n"
                "          },\n"
                "          \"succeeds\": {\n"
                "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=3320426331639809\"\n"
                "          },\n"
                "          \"precedes\": {\n"
                "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=3320426331639809\"\n"
                "          }\n"
                "        },\n"
                "        \"id\": \"3320426331639809\",\n"
                "        \"paging_token\": \"3320426331639809\",\n"
                "        \"source_account\": \"GCR6QXX7IRIJVIM5WA5ASQ6MWDOEJNBW3V6RTC5NJXEMOLVTUVKZ725X\",\n"
                "        \"type\": \"manage_offer\",\n"
                "        \"type_i\": 3,\n"
                "        \"offer_id\": 0,\n"
                "        \"amount\": \"100.0\",\n"
                "        \"buying_asset_type\": \"credit_alphanum4\",\n"
                "        \"buying_asset_code\": \"CNY\",\n"
                "        \"buying_asset_issuer\": \"GAZWSWPDQTBHFIPBY4FEDFW2J6E2LE7SZHJWGDZO6Q63W7DBSRICO2KN\",\n"
                "        \"selling_asset_type\": \"native\"\n"
                "      }";

        ManageSellOfferOperationResponse operation;
        operation.loadFromJson(json);

        QCOMPARE(operation.getOfferId(), 0);
        QCOMPARE(operation.getAmount(), QString("100.0"));
        QVERIFY(operation.getBuyingAsset()->equals( Asset::createNonNativeAsset("CNY", KeyPair::fromAccountId(QString("GAZWSWPDQTBHFIPBY4FEDFW2J6E2LE7SZHJWGDZO6Q63W7DBSRICO2KN")))));
        QVERIFY(operation.getSellingAsset()->equals( new AssetTypeNative()));
    }


    void testDeserializeManageSellOfferOperationOfferIDString() {
        QByteArray json = "{\n"
                "        \"_links\": {\n"
                "          \"self\": {\n"
                "            \"href\": \"http://horizon-testnet.stellar.org/operations/3320426331639809\"\n"
                "          },\n"
                "          \"transaction\": {\n"
                "            \"href\": \"http://horizon-testnet.stellar.org/transactions/1f8fc03b26110e917d124381645d7dcf85927f17e46d8390d254a0bd99cfb0ad\"\n"
                "          },\n"
                "          \"effects\": {\n"
                "            \"href\": \"http://horizon-testnet.stellar.org/operations/3320426331639809/effects\"\n"
                "          },\n"
                "          \"succeeds\": {\n"
                "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=3320426331639809\"\n"
                "          },\n"
                "          \"precedes\": {\n"
                "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=3320426331639809\"\n"
                "          }\n"
                "        },\n"
                "        \"id\": \"3320426331639809\",\n"
                "        \"paging_token\": \"3320426331639809\",\n"
                "        \"source_account\": \"GCR6QXX7IRIJVIM5WA5ASQ6MWDOEJNBW3V6RTC5NJXEMOLVTUVKZ725X\",\n"
                "        \"type\": \"manage_offer\",\n"
                "        \"type_i\": 3,\n"
                "        \"offer_id\": \"10\",\n"
                "        \"amount\": \"100.0\",\n"
                "        \"buying_asset_type\": \"credit_alphanum4\",\n"
                "        \"buying_asset_code\": \"CNY\",\n"
                "        \"buying_asset_issuer\": \"GAZWSWPDQTBHFIPBY4FEDFW2J6E2LE7SZHJWGDZO6Q63W7DBSRICO2KN\",\n"
                "        \"selling_asset_type\": \"native\"\n"
                "      }";

        ManageSellOfferOperationResponse operation;
        operation.loadFromJson(json);

        QCOMPARE(operation.getOfferId(), 10);
        QCOMPARE(operation.getAmount(), QString("100.0"));
        QVERIFY(operation.getBuyingAsset()->equals( Asset::createNonNativeAsset("CNY", KeyPair::fromAccountId(QString("GAZWSWPDQTBHFIPBY4FEDFW2J6E2LE7SZHJWGDZO6Q63W7DBSRICO2KN")))));
        QVERIFY(operation.getSellingAsset()->equals( new AssetTypeNative()));
    }

    void testDeserializeManageBuyOfferOperation() {
        QByteArray json = "{\n"
                          "        \"_links\": {\n"
                          "          \"self\": {\n"
                          "            \"href\": \"http://horizon-testnet.stellar.org/operations/3320426331639809\"\n"
                          "          },\n"
                          "          \"transaction\": {\n"
                          "            \"href\": \"http://horizon-testnet.stellar.org/transactions/1f8fc03b26110e917d124381645d7dcf85927f17e46d8390d254a0bd99cfb0ad\"\n"
                          "          },\n"
                          "          \"effects\": {\n"
                          "            \"href\": \"http://horizon-testnet.stellar.org/operations/3320426331639809/effects\"\n"
                          "          },\n"
                          "          \"succeeds\": {\n"
                          "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=3320426331639809\"\n"
                          "          },\n"
                          "          \"precedes\": {\n"
                          "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=3320426331639809\"\n"
                          "          }\n"
                          "        },\n"
                          "        \"id\": \"3320426331639809\",\n"
                          "        \"paging_token\": \"3320426331639809\",\n"
                          "        \"source_account\": \"GCR6QXX7IRIJVIM5WA5ASQ6MWDOEJNBW3V6RTC5NJXEMOLVTUVKZ725X\",\n"
                          "        \"type\": \"manage_buy_offer\",\n"
                          "        \"type_i\": 12,\n"
                          "        \"offer_id\": 0,\n"
                          "        \"amount\": \"100.0\",\n"
                          "        \"buying_asset_type\": \"credit_alphanum4\",\n"
                          "        \"buying_asset_code\": \"CNY\",\n"
                          "        \"buying_asset_issuer\": \"GAZWSWPDQTBHFIPBY4FEDFW2J6E2LE7SZHJWGDZO6Q63W7DBSRICO2KN\",\n"
                          "        \"selling_asset_type\": \"native\"\n"
                          "      }";

        ManageBuyOfferOperationResponse operation;
        operation.loadFromJson(json);
        QCOMPARE(operation.getOfferId(), 0);
        QCOMPARE(operation.getAmount(), QString("100.0"));
        QVERIFY(operation.getBuyingAsset()->equals( Asset::createNonNativeAsset("CNY", KeyPair::fromAccountId(QString("GAZWSWPDQTBHFIPBY4FEDFW2J6E2LE7SZHJWGDZO6Q63W7DBSRICO2KN")))));
        QVERIFY(operation.getSellingAsset()->equals( new AssetTypeNative()));
    }


    void testDeserializeManageBuyOfferOperationOfferIDString() {
        QByteArray json = "{\n"
                          "        \"_links\": {\n"
                          "          \"self\": {\n"
                          "            \"href\": \"http://horizon-testnet.stellar.org/operations/3320426331639809\"\n"
                          "          },\n"
                          "          \"transaction\": {\n"
                          "            \"href\": \"http://horizon-testnet.stellar.org/transactions/1f8fc03b26110e917d124381645d7dcf85927f17e46d8390d254a0bd99cfb0ad\"\n"
                          "          },\n"
                          "          \"effects\": {\n"
                          "            \"href\": \"http://horizon-testnet.stellar.org/operations/3320426331639809/effects\"\n"
                          "          },\n"
                          "          \"succeeds\": {\n"
                          "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=3320426331639809\"\n"
                          "          },\n"
                          "          \"precedes\": {\n"
                          "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=3320426331639809\"\n"
                          "          }\n"
                          "        },\n"
                          "        \"id\": \"3320426331639809\",\n"
                          "        \"paging_token\": \"3320426331639809\",\n"
                          "        \"source_account\": \"GCR6QXX7IRIJVIM5WA5ASQ6MWDOEJNBW3V6RTC5NJXEMOLVTUVKZ725X\",\n"
                          "        \"type\": \"manage_buy_offer\",\n"
                          "        \"type_i\": 12,\n"
                          "        \"offer_id\": \"11\",\n"
                          "        \"amount\": \"100.0\",\n"
                          "        \"buying_asset_type\": \"credit_alphanum4\",\n"
                          "        \"buying_asset_code\": \"CNY\",\n"
                          "        \"buying_asset_issuer\": \"GAZWSWPDQTBHFIPBY4FEDFW2J6E2LE7SZHJWGDZO6Q63W7DBSRICO2KN\",\n"
                          "        \"selling_asset_type\": \"native\"\n"
                          "      }";

        ManageBuyOfferOperationResponse operation;
        operation.loadFromJson(json);
        QCOMPARE(operation.getOfferId(), 11);
        QCOMPARE(operation.getAmount(), QString("100.0"));
        QVERIFY(operation.getBuyingAsset()->equals( Asset::createNonNativeAsset("CNY", KeyPair::fromAccountId(QString("GAZWSWPDQTBHFIPBY4FEDFW2J6E2LE7SZHJWGDZO6Q63W7DBSRICO2KN")))));
        QVERIFY(operation.getSellingAsset()->equals( new AssetTypeNative()));
    }

    void testDeserializePathPaymentOperation() {
        QByteArray json = "{\n"
                "  \"_links\": {\n"
                "    \"effects\": {\n"
                "      \"href\": \"/operations/25769807873/effects/{?cursor,limit,order}\",\n"
                "      \"templated\": true\n"
                "    },\n"
                "    \"precedes\": {\n"
                "      \"href\": \"/operations?cursor=25769807873\\u0026order=asc\"\n"
                "    },\n"
                "    \"self\": {\n"
                "      \"href\": \"/operations/25769807873\"\n"
                "    },\n"
                "    \"succeeds\": {\n"
                "      \"href\": \"/operations?cursor=25769807873\\u0026order=desc\"\n"
                "    },\n"
                "    \"transaction\": {\n"
                "      \"href\": \"/transactions/25769807872\"\n"
                "    }\n"
                "  },\n"
                "  \"amount\": \"10.0\",\n"
                "  \"asset_code\": \"EUR\",\n"
                "  \"asset_issuer\": \"GCQPYGH4K57XBDENKKX55KDTWOTK5WDWRQOH2LHEDX3EKVIQRLMESGBG\",\n"
                "  \"asset_type\": \"credit_alphanum4\",\n"
                "  \"from\": \"GCXKG6RN4ONIEPCMNFB732A436Z5PNDSRLGWK7GBLCMQLIFO4S7EYWVU\",\n"
                "  \"id\": 25769807873,\n"
                "  \"paging_token\": \"25769807873\",\n"
                "  \"source_asset_code\": \"USD\",\n"
                "  \"source_asset_issuer\": \"GC23QF2HUE52AMXUFUH3AYJAXXGXXV2VHXYYR6EYXETPKDXZSAW67XO4\",\n"
                "  \"source_asset_type\": \"credit_alphanum4\",\n"
                "  \"source_max\": \"100.0\",\n"
                "  \"to\": \"GA5WBPYA5Y4WAEHXWR2UKO2UO4BUGHUQ74EUPKON2QHV4WRHOIRNKKH2\",\n"
                "  \"type_i\": 2,\n"
                "  \"type\": \"path_payment_strict_receive\"\n"
                "}";

        PathPaymentOperationResponse operation;
        operation.loadFromJson(json);
        QCOMPARE(operation.getFrom().getAccountId(), QString("GCXKG6RN4ONIEPCMNFB732A436Z5PNDSRLGWK7GBLCMQLIFO4S7EYWVU"));
        QCOMPARE(operation.getTo().getAccountId(), QString("GA5WBPYA5Y4WAEHXWR2UKO2UO4BUGHUQ74EUPKON2QHV4WRHOIRNKKH2"));
        QCOMPARE(operation.getAmount(), QString("10.0"));
        QCOMPARE(operation.getSourceMax(), QString("100.0"));
        QVERIFY(operation.getAsset()->equals(Asset::createNonNativeAsset("EUR", KeyPair::fromAccountId(QString("GCQPYGH4K57XBDENKKX55KDTWOTK5WDWRQOH2LHEDX3EKVIQRLMESGBG")))));
        QVERIFY(operation.getSourceAsset()->equals(Asset::createNonNativeAsset("USD", KeyPair::fromAccountId(QString("GC23QF2HUE52AMXUFUH3AYJAXXGXXV2VHXYYR6EYXETPKDXZSAW67XO4")))));
    }
    void testDeserializePathPaymentOperationSourceAssetNative()
    {
        QByteArray json = "{\n"
                          "  \"_links\": {\n"
                          "    \"self\": {\n"
                          "      \"href\": \"https://horizon.stellar.org/operations/75252830662840321\"\n"
                          "    },\n"
                          "    \"transaction\": {\n"
                          "      \"href\": \"https://horizon.stellar.org/transactions/fb2f5655c70a459220ac09eb3d6870422b58dcf5c5ffb5e5b21817b4d248826e\"\n"
                          "    },\n"
                          "    \"effects\": {\n"
                          "      \"href\": \"https://horizon.stellar.org/operations/75252830662840321/effects\"\n"
                          "    },\n"
                          "    \"succeeds\": {\n"
                          "      \"href\": \"https://horizon.stellar.org/effects?order=desc\\u0026cursor=75252830662840321\"\n"
                          "    },\n"
                          "    \"precedes\": {\n"
                          "      \"href\": \"https://horizon.stellar.org/effects?order=asc\\u0026cursor=75252830662840321\"\n"
                          "    }\n"
                          "  },\n"
                          "  \"id\": \"75252830662840321\",\n"
                          "  \"paging_token\": \"75252830662840321\",\n"
                          "  \"source_account\": \"GC45JH537XZD4DY4WTV5PCUJL4KPOIE4WMGX5OP5KSPS2OLGRUOVVIGD\",\n"
                          "  \"type\": \"path_payment_strict_receive\",\n"
                          "  \"type_i\": 2,\n"
                          "  \"created_at\": \"2018-04-24T12:58:12Z\",\n"
                          "  \"transaction_hash\": \"fb2f5655c70a459220ac09eb3d6870422b58dcf5c5ffb5e5b21817b4d248826e\",\n"
                          "  \"asset_type\": \"credit_alphanum4\",\n"
                          "  \"asset_code\": \"XRP\",\n"
                          "  \"asset_issuer\": \"GBVOL67TMUQBGL4TZYNMY3ZQ5WGQYFPFD5VJRWXR72VA33VFNL225PL5\",\n"
                          "  \"from\": \"GC45JH537XZD4DY4WTV5PCUJL4KPOIE4WMGX5OP5KSPS2OLGRUOVVIGD\",\n"
                          "  \"to\": \"GC45JH537XZD4DY4WTV5PCUJL4KPOIE4WMGX5OP5KSPS2OLGRUOVVIGD\",\n"
                          "  \"amount\": \"2.5000000\",\n"                          
                          "  \"path\": [{\"asset_type\": \"native\"}, {\"asset_type\": \"credit_alphanum4\", \"asset_code\": \"CNY\", \"asset_issuer\": \"GAREELUB43IRHWEASCFBLKHURCGMHE5IF6XSE7EXDLACYHGRHM43RFOX\"}, {\"asset_type\": \"credit_alphanum12\", \"asset_code\": \"CNYMNL\", \"asset_issuer\": \"GAREELUB43IRHWEASCFBLKHURCGMHE5IF6XSE7EXDLACYHGRHM43RFOX\"}],\n"
                          "  \"source_amount\": \"1.1777000\",\n"
                          "  \"source_max\": \"1.1779523\",\n"
                          "  \"source_asset_type\": \"native\"\n"
                          "}";
        PathPaymentOperationResponse operation;
        operation.loadFromJson(json);
        QCOMPARE(operation.getFrom().getAccountId(), QString("GC45JH537XZD4DY4WTV5PCUJL4KPOIE4WMGX5OP5KSPS2OLGRUOVVIGD"));
        QCOMPARE(operation.getTo().getAccountId(), QString("GC45JH537XZD4DY4WTV5PCUJL4KPOIE4WMGX5OP5KSPS2OLGRUOVVIGD"));
        QCOMPARE(operation.getAmount(), QString("2.5000000"));
        QCOMPARE(operation.getSourceAmount(), QString("1.1777000"));
        QCOMPARE(operation.getSourceMax(), QString("1.1779523"));
        QVERIFY(operation.getSourceAsset()->equals(new AssetTypeNative()));

        QList<Asset*> expectedAssetsPath = {
                    new AssetTypeNative(),
                    Asset::createNonNativeAsset("CNY", KeyPair::fromAccountId("GAREELUB43IRHWEASCFBLKHURCGMHE5IF6XSE7EXDLACYHGRHM43RFOX")),
                    Asset::createNonNativeAsset("CNYMNL", KeyPair::fromAccountId("GAREELUB43IRHWEASCFBLKHURCGMHE5IF6XSE7EXDLACYHGRHM43RFOX"))
            };
        QVERIFY((expectedAssetsPath.size() == operation.getPath().size()) && std::equal(operation.getPath().begin(),operation.getPath().end(),expectedAssetsPath.begin(),[](Asset* a, Asset* b){return a->equals(b);}));


        QVERIFY(operation.getAsset()->equals(Asset::createNonNativeAsset("XRP", KeyPair::fromAccountId(QString("GBVOL67TMUQBGL4TZYNMY3ZQ5WGQYFPFD5VJRWXR72VA33VFNL225PL5")))));
    }

    void testDeserializePathPaymentStrictSendOperation()
    {
      QByteArray json = "{\n"
              "  \"_links\": {\n"
              "    \"self\": {\n"
              "      \"href\": \"https://horizon.stellar.org/operations/75252830662840321\"\n"
              "    },\n"
              "    \"transaction\": {\n"
              "      \"href\": \"https://horizon.stellar.org/transactions/fb2f5655c70a459220ac09eb3d6870422b58dcf5c5ffb5e5b21817b4d248826e\"\n"
              "    },\n"
              "    \"effects\": {\n"
              "      \"href\": \"https://horizon.stellar.org/operations/75252830662840321/effects\"\n"
              "    },\n"
              "    \"succeeds\": {\n"
              "      \"href\": \"https://horizon.stellar.org/effects?order=desc\\u0026cursor=75252830662840321\"\n"
              "    },\n"
              "    \"precedes\": {\n"
              "      \"href\": \"https://horizon.stellar.org/effects?order=asc\\u0026cursor=75252830662840321\"\n"
              "    }\n"
              "  },\n"
              "  \"id\": \"75252830662840321\",\n"
              "  \"paging_token\": \"75252830662840321\",\n"
              "  \"source_account\": \"GC45JH537XZD4DY4WTV5PCUJL4KPOIE4WMGX5OP5KSPS2OLGRUOVVIGD\",\n"
              "  \"type\": \"path_payment_strict_send\",\n"
              "  \"type_i\": 13,\n"
              "  \"created_at\": \"2018-04-24T12:58:12Z\",\n"
              "  \"transaction_hash\": \"fb2f5655c70a459220ac09eb3d6870422b58dcf5c5ffb5e5b21817b4d248826e\",\n"
              "  \"asset_type\": \"native\",\n"
              "  \"from\": \"GC45JH537XZD4DY4WTV5PCUJL4KPOIE4WMGX5OP5KSPS2OLGRUOVVIGD\",\n"
              "  \"to\": \"GC45JH537XZD4DY4WTV5PCUJL4KPOIE4WMGX5OP5KSPS2OLGRUOVVIGD\",\n"
              "  \"amount\": \"2.5000000\",\n"
              "  \"path\": [{\"asset_type\": \"native\"}, {\"asset_type\": \"credit_alphanum4\", \"asset_code\": \"CNY\", \"asset_issuer\": \"GAREELUB43IRHWEASCFBLKHURCGMHE5IF6XSE7EXDLACYHGRHM43RFOX\"}, {\"asset_type\": \"credit_alphanum12\", \"asset_code\": \"CNYMNL\", \"asset_issuer\": \"GAREELUB43IRHWEASCFBLKHURCGMHE5IF6XSE7EXDLACYHGRHM43RFOX\"}],\n"
              "  \"source_amount\": \"1.1777000\",\n"
              "  \"destination_min\": \"1.1779523\",\n"
              "  \"source_asset_type\": \"credit_alphanum4\",\n"
              "  \"source_asset_code\": \"XRP\",\n"
              "  \"source_asset_issuer\": \"GBVOL67TMUQBGL4TZYNMY3ZQ5WGQYFPFD5VJRWXR72VA33VFNL225PL5\"\n"
              "}";

      PathPaymentStrictSendOperationResponse operation;
      operation.loadFromJson(json);

      QCOMPARE(operation.getFrom().getAccountId(), QString("GC45JH537XZD4DY4WTV5PCUJL4KPOIE4WMGX5OP5KSPS2OLGRUOVVIGD"));
      QCOMPARE(operation.getTo().getAccountId(), QString("GC45JH537XZD4DY4WTV5PCUJL4KPOIE4WMGX5OP5KSPS2OLGRUOVVIGD"));
      QCOMPARE(operation.getAmount(), QString("2.5000000"));
      QCOMPARE(operation.getSourceAmount(), QString("1.1777000"));
      QCOMPARE(operation.getDestinationMin(), QString("1.1779523"));
      QVERIFY(operation.getAsset()->equals( new AssetTypeNative()));
      QList<Asset*> expectedAssetsPath = {
                  new AssetTypeNative(),
                  Asset::createNonNativeAsset("CNY", KeyPair::fromAccountId("GAREELUB43IRHWEASCFBLKHURCGMHE5IF6XSE7EXDLACYHGRHM43RFOX")),
                  Asset::createNonNativeAsset("CNYMNL", KeyPair::fromAccountId("GAREELUB43IRHWEASCFBLKHURCGMHE5IF6XSE7EXDLACYHGRHM43RFOX"))
          };
      QVERIFY((expectedAssetsPath.size() == operation.getPath().size()) && std::equal(operation.getPath().begin(),operation.getPath().end(),expectedAssetsPath.begin(),[](Asset* a, Asset* b){return a->equals(b);}));

      QVERIFY(operation.getSourceAsset()->equals(Asset::createNonNativeAsset("XRP", KeyPair::fromAccountId("GBVOL67TMUQBGL4TZYNMY3ZQ5WGQYFPFD5VJRWXR72VA33VFNL225PL5"))));
    }

    void testDeserializeCreatePassiveOfferOperation() {
        QByteArray json = "{\n"
                "  \"_links\": {\n"
                "    \"effects\": {\n"
                "      \"href\": \"/operations/1127729562914817/effects{?cursor,limit,order}\",\n"
                "      \"templated\": true\n"
                "    },\n"
                "    \"precedes\": {\n"
                "      \"href\": \"/operations?cursor=1127729562914817\\u0026order=asc\"\n"
                "    },\n"
                "    \"self\": {\n"
                "      \"href\": \"/operations/1127729562914817\"\n"
                "    },\n"
                "    \"succeeds\": {\n"
                "      \"href\": \"/operations?cursor=1127729562914817\\u0026order=desc\"\n"
                "    },\n"
                "    \"transaction\": {\n"
                "      \"href\": \"/transactions/1127729562914816\"\n"
                "    }\n"
                "  },\n"
                "  \"amount\": \"11.27827\",\n"
                "  \"buying_asset_code\": \"USD\",\n"
                "  \"buying_asset_issuer\": \"GDS5JW5E6DRSSN5XK4LW7E6VUMFKKE2HU5WCOVFTO7P2RP7OXVCBLJ3Y\",\n"
                "  \"buying_asset_type\": \"credit_alphanum4\",\n"
                "  \"id\": 1127729562914817,\n"
                "  \"offer_id\": 9,\n"
                "  \"paging_token\": \"1127729562914817\",\n"
                "  \"price\": \"1.0\",\n"
                "  \"price_r\": {\n"
                "    \"d\": 1,\n"
                "    \"n\": 1\n"
                "  },\n"
                "  \"selling_asset_type\": \"native\",\n"
                "  \"type_i\": 4,\n"
                "  \"type\": \"create_passive_sell_offer\"\n"
                "}";

        CreatePassiveSellOfferOperationResponse operation;
        operation.loadFromJson(json);

        QCOMPARE(operation.getAmount(), QString("11.27827"));
        QVERIFY(operation.getBuyingAsset()->equals(Asset::createNonNativeAsset("USD", KeyPair::fromAccountId(QString("GDS5JW5E6DRSSN5XK4LW7E6VUMFKKE2HU5WCOVFTO7P2RP7OXVCBLJ3Y")))));
        QVERIFY(operation.getSellingAsset()->equals( new AssetTypeNative()));
    }
    void testDeserializeInflationOperation() {
        QByteArray json = "{\n"
                "  \"_links\": {\n"
                "    \"effects\": {\n"
                "      \"href\": \"/operations/12884914177/effects/{?cursor,limit,order}\",\n"
                "      \"templated\": true\n"
                "    },\n"
                "    \"precedes\": {\n"
                "      \"href\": \"/operations?cursor=12884914177\\u0026order=asc\"\n"
                "    },\n"
                "    \"self\": {\n"
                "      \"href\": \"/operations/12884914177\"\n"
                "    },\n"
                "    \"succeeds\": {\n"
                "      \"href\": \"/operations?cursor=12884914177\\u0026order=desc\"\n"
                "    },\n"
                "    \"transaction\": {\n"
                "      \"href\": \"/transactions/12884914176\"\n"
                "    }\n"
                "  },\n"
                "  \"id\": 12884914177,\n"
                "  \"paging_token\": \"12884914177\",\n"
                "  \"type_i\": 9,\n"
                "  \"type\": \"inflation\"\n"
                "}";

        InflationOperationResponse operation;
        operation.loadFromJson(json);
        QCOMPARE(operation.getId(), 12884914177L);
    }
    void testDeserializeManageDataOperation() {
        QByteArray json = "{\n"
                "  \"_links\": {\n"
                "    \"self\": {\n"
                "      \"href\": \"https://horizon-testnet.stellar.org/operations/14336188517191688\"\n"
                "    },\n"
                "    \"transaction\": {\n"
                "      \"href\": \"https://horizon-testnet.stellar.org/transactions/ad99999615f653528e67d8c8783e14044519c3e5233b73633aa88050c5856103\"\n"
                "    },\n"
                "    \"effects\": {\n"
                "      \"href\": \"https://horizon-testnet.stellar.org/operations/14336188517191688/effects\"\n"
                "    },\n"
                "    \"succeeds\": {\n"
                "      \"href\": \"https://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=14336188517191688\"\n"
                "    },\n"
                "    \"precedes\": {\n"
                "      \"href\": \"https://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=14336188517191688\"\n"
                "    }\n"
                "  },\n"
                "  \"id\": \"14336188517191688\",\n"
                "  \"paging_token\": \"14336188517191688\",\n"
                "  \"source_account\": \"GAC3U2W4KPZJX7GX5AZPOEWBJVUTJQ4TZNO74YN24ETTFIJVY7EMMANP\",\n"
                "  \"type\": \"manage_data\",\n"
                "  \"type_i\": 10,\n"
                "  \"name\": \"CollateralValue\",\n"
                "  \"value\": \"MjAwMA==\"\n"
                "}";

        ManageDataOperationResponse operation;
        operation.loadFromJson(json);
        QCOMPARE(operation.getId(), 14336188517191688L);
        QCOMPARE(operation.getName(), QString("CollateralValue"));
        QCOMPARE(operation.getValue(), QString("MjAwMA=="));
    }

    void testDeserializeManageDataOperationValueEmpty() {
        QByteArray json = "{\n"
                "  \"_links\": {\n"
                "    \"self\": {\n"
                "      \"href\": \"https://horizon-testnet.stellar.org/operations/14336188517191688\"\n"
                "    },\n"
                "    \"transaction\": {\n"
                "      \"href\": \"https://horizon-testnet.stellar.org/transactions/ad99999615f653528e67d8c8783e14044519c3e5233b73633aa88050c5856103\"\n"
                "    },\n"
                "    \"effects\": {\n"
                "      \"href\": \"https://horizon-testnet.stellar.org/operations/14336188517191688/effects\"\n"
                "    },\n"
                "    \"succeeds\": {\n"
                "      \"href\": \"https://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=14336188517191688\"\n"
                "    },\n"
                "    \"precedes\": {\n"
                "      \"href\": \"https://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=14336188517191688\"\n"
                "    }\n"
                "  },\n"
                "  \"id\": \"14336188517191688\",\n"
                "  \"paging_token\": \"14336188517191688\",\n"
                "  \"source_account\": \"GAC3U2W4KPZJX7GX5AZPOEWBJVUTJQ4TZNO74YN24ETTFIJVY7EMMANP\",\n"
                "  \"type\": \"manage_data\",\n"
                "  \"type_i\": 10,\n"
                "  \"name\": \"CollateralValue\",\n"
                "  \"value\": null\n"
                "}";

        ManageDataOperationResponse operation;
        operation.loadFromJson(json);

        QCOMPARE(operation.getValue(), QString());
    }

    void testDeserializeBumpSequenceOperation() {
        QByteArray json = "{\n"
              "  \"_links\": {\n"
              "    \"effects\": {\n"
              "      \"href\": \"/operations/12884914177/effects/{?cursor,limit,order}\",\n"
              "      \"templated\": true\n"
              "    },\n"
              "    \"precedes\": {\n"
              "      \"href\": \"/operations?cursor=12884914177\\u0026order=asc\"\n"
              "    },\n"
              "    \"self\": {\n"
              "      \"href\": \"/operations/12884914177\"\n"
              "    },\n"
              "    \"succeeds\": {\n"
              "      \"href\": \"/operations?cursor=12884914177\\u0026order=desc\"\n"
              "    },\n"
              "    \"transaction\": {\n"
              "      \"href\": \"/transactions/12884914176\"\n"
              "    }\n"
              "  },\n"
              "  \"id\": 12884914177,\n"
              "  \"paging_token\": \"12884914177\",\n"
              "  \"type_i\": 11,\n"
              "  \"type\": \"bump_sequence\",\n"
              "  \"bump_to\": \"79473726952833048\"\n"
              "}";

      BumpSequenceOperationResponse operation;
      operation.loadFromJson(json);

      QCOMPARE(operation.getId(), 12884914177L);
      QCOMPARE(operation.getBumpTo(), 79473726952833048L);
    }

    void testDeserializeClawbackClaimableBalanceOperation() {
      QByteArray json = "{\n"
          "  \"_links\": {\n"
          "    \"effects\": {\n"
          "      \"href\": \"/operations/12884914177/effects/{?cursor,limit,order}\",\n"
          "      \"templated\": true\n"
          "    },\n"
          "    \"precedes\": {\n"
          "      \"href\": \"/operations?cursor=12884914177\\u0026order=asc\"\n"
          "    },\n"
          "    \"self\": {\n"
          "      \"href\": \"/operations/12884914177\"\n"
          "    },\n"
          "    \"succeeds\": {\n"
          "      \"href\": \"/operations?cursor=12884914177\\u0026order=desc\"\n"
          "    },\n"
          "    \"transaction\": {\n"
          "      \"href\": \"/transactions/12884914176\"\n"
          "    }\n"
          "  },\n"
          "  \"id\": 12884914177,\n"
          "  \"paging_token\": \"12884914177\",\n"
          "  \"type_i\": 20,\n"
          "  \"type\": \"clawback_claimable_balance\",\n"
          "  \"balance_id\": \"00000000178826fbfe339e1f5c53417c6fedfe2c05e8bec14303143ec46b38981b09c3f9\"\n"
          "}";

      ClawbackClaimableBalanceOperationResponse operation;
      operation.loadFromJson(json);

      QCOMPARE(operation.getId(), 12884914177L);
      QCOMPARE(operation.getBalanceID(), "00000000178826fbfe339e1f5c53417c6fedfe2c05e8bec14303143ec46b38981b09c3f9");
      QCOMPARE(operation.getType(), "clawback_claimable_balance");
    }

    void testDeserializeClawbackOperation() {
      QByteArray json = "{\n"
          "  \"_links\": {\n"
          "    \"effects\": {\n"
          "      \"href\": \"/operations/12884914177/effects/{?cursor,limit,order}\",\n"
          "      \"templated\": true\n"
          "    },\n"
          "    \"precedes\": {\n"
          "      \"href\": \"/operations?cursor=12884914177\\u0026order=asc\"\n"
          "    },\n"
          "    \"self\": {\n"
          "      \"href\": \"/operations/12884914177\"\n"
          "    },\n"
          "    \"succeeds\": {\n"
          "      \"href\": \"/operations?cursor=12884914177\\u0026order=desc\"\n"
          "    },\n"
          "    \"transaction\": {\n"
          "      \"href\": \"/transactions/12884914176\"\n"
          "    }\n"
          "  },\n"
          "  \"id\": 12884914177,\n"
          "  \"paging_token\": \"12884914177\",\n"
          "  \"type_i\": 19,\n"
          "  \"type\": \"clawback\",\n"
          "  \"asset_type\": \"credit_alphanum4\",\n"
          "  \"asset_code\": \"EUR\",\n"
          "  \"asset_issuer\": \"GCWVFBJ24754I5GXG4JOEB72GJCL3MKWC7VAEYWKGQHPVH3ENPNBSKWS\",\n"
          "  \"from\": \"GDPFGP4IPE5DXG6XRXC4ZBUI43PAGRQ5VVNJ3LJTBXDBZ4ITO6HBHNSF\"\n"
          "}";

      ClawbackOperationResponse operation;
      operation.loadFromJson(json);

      QCOMPARE(operation.getId(), 12884914177L);
      QCOMPARE(operation.getFrom(), "GDPFGP4IPE5DXG6XRXC4ZBUI43PAGRQ5VVNJ3LJTBXDBZ4ITO6HBHNSF");
      QCOMPARE(operation.getType(), "clawback");
      QVERIFY(operation.getAsset()->equals(Asset::createNonNativeAsset("EUR", "GCWVFBJ24754I5GXG4JOEB72GJCL3MKWC7VAEYWKGQHPVH3ENPNBSKWS")));
      QCOMPARE(operation.getAssetIssuer(), "GCWVFBJ24754I5GXG4JOEB72GJCL3MKWC7VAEYWKGQHPVH3ENPNBSKWS");
      QCOMPARE(operation.getAssetCode(), "EUR");
      QCOMPARE(operation.getAssetType(), "credit_alphanum4");
    }

    void testDeserializeSetTrustlineFlagsOperation() {
      QByteArray json = "{\n"
          "  \"_links\": {\n"
          "    \"effects\": {\n"
          "      \"href\": \"/operations/12884914177/effects/{?cursor,limit,order}\",\n"
          "      \"templated\": true\n"
          "    },\n"
          "    \"precedes\": {\n"
          "      \"href\": \"/operations?cursor=12884914177\\u0026order=asc\"\n"
          "    },\n"
          "    \"self\": {\n"
          "      \"href\": \"/operations/12884914177\"\n"
          "    },\n"
          "    \"succeeds\": {\n"
          "      \"href\": \"/operations?cursor=12884914177\\u0026order=desc\"\n"
          "    },\n"
          "    \"transaction\": {\n"
          "      \"href\": \"/transactions/12884914176\"\n"
          "    }\n"
          "  },\n"
          "  \"id\": 12884914177,\n"
          "  \"paging_token\": \"12884914177\",\n"
          "  \"type_i\": 21,\n"
          "  \"type\": \"set_trust_line_flags\",\n"
          "  \"asset_type\": \"credit_alphanum4\",\n"
          "  \"asset_code\": \"EUR\",\n"
          "  \"asset_issuer\": \"GCWVFBJ24754I5GXG4JOEB72GJCL3MKWC7VAEYWKGQHPVH3ENPNBSKWS\",\n"
          "  \"set_flags\": [\n"
          "     4\n"
          "  ],"
          "  \"clear_flags\": [\n"
          "     2\n"
          "  ],"
          "  \"set_flags_s\": [\n"
          "     \"clawback_enabled\"\n"
          "  ],"
          "  \"clear_flags_s\": [\n"
          "     \"authorized_to_maintain_liabilites\"\n"
          "  ],"
          "  \"trustor\": \"GDPFGP4IPE5DXG6XRXC4ZBUI43PAGRQ5VVNJ3LJTBXDBZ4ITO6HBHNSF\"\n"
          "}";

      SetTrustLineFlagsOperationResponse operation;
      operation.loadFromJson(json);

      QCOMPARE(operation.getId(), 12884914177L);
      QCOMPARE(operation.getTrustor(), "GDPFGP4IPE5DXG6XRXC4ZBUI43PAGRQ5VVNJ3LJTBXDBZ4ITO6HBHNSF");
      QCOMPARE(operation.getType(), "set_trust_line_flags");
      QVERIFY(operation.getAsset()->equals(Asset::createNonNativeAsset("EUR", "GCWVFBJ24754I5GXG4JOEB72GJCL3MKWC7VAEYWKGQHPVH3ENPNBSKWS")));
      QCOMPARE(operation.getAssetIssuer(), "GCWVFBJ24754I5GXG4JOEB72GJCL3MKWC7VAEYWKGQHPVH3ENPNBSKWS");
      QCOMPARE(operation.getAssetCode(), "EUR");
      QCOMPARE(operation.getAssetType(), "credit_alphanum4");
      QCOMPARE(operation.getSetFlags(), QList<qint32>() << 4);
      QCOMPARE(operation.getClearFlags(), QList<qint32>() << 2);
      QCOMPARE(operation.getSetFlagStrings(), QStringList() << "clawback_enabled");
      QCOMPARE(operation.getClearFlagStrings(), QStringList()<< "authorized_to_maintain_liabilites");
    }
};
ADD_TEST(OperationDeserializerTest)
#endif // OPERATIONDESERIALIZERTEST_H
