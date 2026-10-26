#ifndef OPERATIONDESERIALIZERTEST_H
#define OPERATIONDESERIALIZERTEST_H
#include <QObject>


#include <QtTest>
#include "../testcollector.h"
#include "../../src/responses/operations/accountmergeoperationresponse.h"
#include "../../src/responses/operations/endsponsoringfuturereservesoperationresponse.h"
#include "../../src/responses/operations/revokesponsorshipoperationresponse.h"
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
#include "../../src/responses/operations/claimclaimablebalanceoperationresponse.h"
#include "../../src/responses/operations/clawbackoperationresponse.h"
#include "../../src/responses/operations/settrustlineflagsoperationresponse.h"

#include "../../src/responses/genericoperation.h"

#include "../../src/asset.h"
#include "../../src/assettypenative.h"
#include "../../src/keypair.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "../../src/responses/genericoperation.h"
#include "../../src/responses/operations/liquiditypooldepositoperationresponse.h"
#include "../../src/responses/operations/createclaimablebalanceoperationresponse.h"
#include "../../src/predicate.h"
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
                "  \"created_at\": \"2026-09-28T15:16:12Z\",\n"
                "  \"transaction_hash\": \"f2c44d20a4164557ad5d35ed4e41c49623084c38406d61ed5af94be5b7c3009c\",\n"
                "  \"transaction_successful\": true,\n"
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
                "  \"created_at\": \"2026-09-28T15:16:12Z\",\n"
                "  \"transaction_hash\": \"f2c44d20a4164557ad5d35ed4e41c49623084c38406d61ed5af94be5b7c3009c\",\n"
                "  \"transaction_successful\": true,\n"
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

        // Horizon always sends transaction_successful (since 0.18): this
        // expected it read null when missing; a missing one is refused now.
        QCOMPARE(operation.isTransactionSuccessful(), Boolean(true));
        QByteArray withoutOutcome = json;
        withoutOutcome.replace("\"transaction_successful\": true,", "");
        CreateAccountOperationResponse refused;
        QVERIFY_EXCEPTION_THROWN(refused.loadFromJson(withoutOutcome), std::runtime_error);

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
                "        \"created_at\": \"2026-09-28T15:16:12Z\",\n"
                "        \"transaction_hash\": \"c4a7490b9a630e8dcb6762ef0c99e06859a119e6e206436ae8e23ab47ef3ec1a\",\n"
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
                "        \"created_at\": \"2026-09-28T15:16:12Z\",\n"
                "        \"transaction_hash\": \"cae78de95c0d4d715d774201d84f8f9f518730b454fbeb98292359af6a2b029f\",\n"
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
    // AllowTrustOperationResponse is deprecated; these two tests cover it on purpose.
#if defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable: 4996)
#elif defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
    void testDeserializeAllowTrustOperation() {
        QByteArray json = "{\n"
                "        \"created_at\": \"2026-09-28T15:16:12Z\",\n"
                "        \"transaction_hash\": \"0177ddb807652b8abcd2535048d056a2a64da7fcbf11b4858c302c89edbcbfbe\",\n"
                "        \"transaction_successful\": true,\n"
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
            "        \"created_at\": \"2026-09-28T15:16:12Z\",\n"
            "        \"transaction_hash\": \"0177ddb807652b8abcd2535048d056a2a64da7fcbf11b4858c302c89edbcbfbe\",\n"
            "        \"transaction_successful\": true,\n"
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
#if defined(_MSC_VER)
#  pragma warning(pop)
#elif defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic pop
#endif

    void testDeserializeChangeTrustOperation() {
        QByteArray json = "{\n"
                "        \"created_at\": \"2026-09-28T15:16:12Z\",\n"
                "        \"transaction_hash\": \"7a6c1aa609b9690b5d9315f194539a78178588e8d533c147df1b50db732ea387\",\n"
                "        \"transaction_successful\": true,\n"
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
    /**
     * Horizon leaves out every set_options field the operation does not
     * change. The weights and thresholds were never initialised, so for this
     * real testnet record (a signer added, nothing else) they read as heap
     * garbage: low_threshold 7929928, master_key_weight 4259917. They read 0
     * now, and has...() says the operation does not set them (js: undefined).
     */
    void testSetOptionsFieldsLeftOutAreNotSet()
    {
        const QByteArray json(
            "{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21257352221057029\"},\"transaction\":{\"href\":\"https://ho"
            "rizon-testnet.stellar.org/transactions/aa7765506cdd839ac5d9379c29018d53dba6107e07052e577c32ee129e8d5166\"},\"effects\":{\"href\":\"https://h"
            "orizon-testnet.stellar.org/operations/21257352221057029/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?orde"
            "r=desc&cursor=21257352221057029\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21257352221057029\""
            "}},\"id\":\"21257352221057029\",\"paging_token\":\"21257352221057029\",\"transaction_successful\":true,\"source_account\":\"GBVO5STN7FUCPH3Q"
            "NIQZXVBNUISKC3Z2YWTTOQZ3AI5XIKYGKQVG5X42\",\"type\":\"set_options\",\"type_i\":5,\"created_at\":\"2026-09-30T12:13:22Z\",\"transaction_hash\""
            ":\"aa7765506cdd839ac5d9379c29018d53dba6107e07052e577c32ee129e8d5166\",\"sponsor\":\"GA24PJEU4I5RHAMQ2KKKCNLH2D5SWMZ3KK3EJONCWHLF4JQVD3S76HCK"
            "\",\"signer_key\":\"GDNF6H2HLUNAKZ3WGCOOOCX6U3Y3TW5OL33P77KSACARDG6G53HC33YG\",\"signer_weight\":1}");
        for (int round = 0; round < 3; ++round) {
            SetOptionsOperationResponse* operation = new SetOptionsOperationResponse();
            operation->loadFromJson(json);
            QVERIFY(!operation->hasLowThreshold());
            QVERIFY(!operation->hasMedThreshold());
            QVERIFY(!operation->hasHighThreshold());
            QVERIFY(!operation->hasMasterKeyWeight());
            QCOMPARE(operation->getLowThreshold(), quint32(0));
            QCOMPARE(operation->getMedThreshold(), quint32(0));
            QCOMPARE(operation->getHighThreshold(), quint32(0));
            QCOMPARE(operation->getMasterKeyWeight(), quint32(0));
            QVERIFY(operation->hasSignerWeight());
            QCOMPARE(operation->getSignerWeight(), quint32(1));
            QCOMPARE(operation->getSignerKey(), QStringLiteral("GDNF6H2HLUNAKZ3WGCOOOCX6U3Y3TW5OL33P77KSACARDG6G53HC33YG"));
            delete operation;
        }
        // Set to 0 is set: a master weight of 0 retires the master key.
        QJsonObject retire = QJsonDocument::fromJson(json).object();
        retire.remove(QStringLiteral("signer_key"));
        retire.remove(QStringLiteral("signer_weight"));
        retire.insert(QStringLiteral("master_key_weight"), 0);
        SetOptionsOperationResponse operation;
        operation.loadFromJson(QJsonDocument(retire).toJson());
        QVERIFY(operation.hasMasterKeyWeight());
        QCOMPARE(operation.getMasterKeyWeight(), quint32(0));
        QVERIFY(!operation.hasSignerWeight());
    }

    /**
     * Revoking a pool-share trustline's sponsorship: Horizon names the line by
     * trustline_liquidity_pool_id (no trustline_asset), which was dropped.
     * Real Horizon testnet record.
     */
    void testRevokePoolShareTrustlineSponsorshipNamesItsPool()
    {
        const QByteArray json(
            "{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21272539225403395\"},\"transaction\":{\"href\":\"https://ho"
            "rizon-testnet.stellar.org/transactions/5538ea27fd165b25bfbab7417dae0056a457e6afe34973fbfee40e922081cd10\"},\"effects\":{\"href\":\"https://h"
            "orizon-testnet.stellar.org/operations/21272539225403395/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?orde"
            "r=desc&cursor=21272539225403395\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21272539225403395\""
            "}},\"id\":\"21272539225403395\",\"paging_token\":\"21272539225403395\",\"transaction_successful\":true,\"source_account\":\"GA4A3QBXIUNW5ULE"
            "GZFMPHFVTTW24Z4MGSLWVDPKVAK3Q57Q22SZW4YU\",\"type\":\"revoke_sponsorship\",\"type_i\":18,\"created_at\":\"2026-09-30T17:08:02Z\",\"transacti"
            "on_hash\":\"5538ea27fd165b25bfbab7417dae0056a457e6afe34973fbfee40e922081cd10\",\"trustline_account_id\":\"GCEJZIZWHKXS4A3NIVZPDCUJGISPZWHIEL"
            "JXQ2WCKO7RJRRIGFB4HVAW\",\"trustline_liquidity_pool_id\":\"bb705851b1b05afc25094774a5f3a2278435e43124a485fa183d6ac9248102c7\"}");
        RevokeSponsorshipOperationResponse operation;
        operation.loadFromJson(json);
        QCOMPARE(operation.getTrustlineLiquidityPoolID(), QStringLiteral("bb705851b1b05afc25094774a5f3a2278435e43124a485fa183d6ac9248102c7"));
        QCOMPARE(operation.getTrustlineAccountID(), QStringLiteral("GCEJZIZWHKXS4A3NIVZPDCUJGISPZWHIELJXQ2WCKO7RJRRIGFB4HVAW"));
        QVERIFY(operation.getTrustlineAsset().isEmpty());
    }

    /**
     * Horizon adds <account>_muxed and <account>_muxed_id when an operation
     * names a muxed account; four response classes dropped them (js types
     * them, py returns them). Real records, with the muxed members added.
     */
    void testMuxedMembersAreRead()
    {
#if defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable: 4996)
#elif defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
        {
            ChangeTrustOperationResponse operation;
            operation.loadFromJson(QByteArray(
                "{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21254727996035073\"},\"transaction\":{\"href\":\"https://ho"
                "rizon-testnet.stellar.org/transactions/64c03aedd6a12773cedfd0439a116b40c7309ccbdd4af68200fe3fe75f67b4a7\"},\"effects\":{\"href\":\"https://h"
                "orizon-testnet.stellar.org/operations/21254727996035073/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?orde"
                "r=desc&cursor=21254727996035073\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21254727996035073\""
                "}},\"id\":\"21254727996035073\",\"paging_token\":\"21254727996035073\",\"transaction_successful\":true,\"source_account\":\"GBUXQKGGE3SL7IGD"
                "X6ZFMGACBBDDS6JSAPALZEMRVDO7M65YSNKHWICN\",\"type\":\"change_trust\",\"type_i\":6,\"created_at\":\"2026-09-30T11:22:27Z\",\"transaction_hash"
                "\":\"64c03aedd6a12773cedfd0439a116b40c7309ccbdd4af68200fe3fe75f67b4a7\",\"asset_type\":\"credit_alphanum4\",\"asset_code\":\"TST\",\"asset_i"
                "ssuer\":\"GD3HFT4Z5YHOVQYDFHDWOWF6CB7WFIQAWBRNYGFD3G2VX54GZXPDPPQ6\",\"limit\":\"922337203685.4775807\",\"trustee\":\"GD3HFT4Z5YHOVQYDFHDWOW"
                "F6CB7WFIQAWBRNYGFD3G2VX54GZXPDPPQ6\",\"trustor\":\"GBUXQKGGE3SL7IGDX6ZFMGACBBDDS6JSAPALZEMRVDO7M65YSNKHWICN\",\"trustor_muxed\":\"MA7QYNF7SO"
                "WQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVAAAAAAAAAAAACJUQ\",\"trustor_muxed_id\":\"1234\"}"));
            QCOMPARE(operation.getTrustorMuxed(), QStringLiteral("MA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVAAAAAAAAAAAACJUQ"));
            QCOMPARE(operation.getTrustorMuxedId(), QStringLiteral("1234"));
        }
        {
            ClaimClaimableBalanceOperationResponse operation;
            operation.loadFromJson(QByteArray(
                "{\"_links\":{\"self\":{\"href\":\"https://horizon.stellar.org/operations/277867865607139329\"},\"transaction\":{\"href\":\"https://horizon.s"
                "tellar.org/transactions/4b7f600adac468f1e868e31d8f20d228e34321e2fdb1f0d7eb9eaa3396303fd3\"},\"effects\":{\"href\":\"https://horizon.stellar."
                "org/operations/277867865607139329/effects\"},\"succeeds\":{\"href\":\"https://horizon.stellar.org/effects?order=desc&cursor=2778678656071393"
                "29\"},\"precedes\":{\"href\":\"https://horizon.stellar.org/effects?order=asc&cursor=277867865607139329\"}},\"id\":\"277867865607139329\",\"p"
                "aging_token\":\"277867865607139329\",\"transaction_successful\":false,\"source_account\":\"GDQZ4KGVYF2LWX3KR5IYIEJOBMBQORHUTUWVBE3HYSDYSYGCJ"
                "MUIYYBL\",\"type\":\"claim_claimable_balance\",\"type_i\":15,\"created_at\":\"2026-09-30T11:09:12Z\",\"transaction_hash\":\"4b7f600adac468f1"
                "e868e31d8f20d228e34321e2fdb1f0d7eb9eaa3396303fd3\",\"balance_id\":\"00000000421b6473ab0244ecb6032ef1e28603be565922f48d789a5a77b5dd593b8fa44e"
                "\",\"claimant\":\"GDQZ4KGVYF2LWX3KR5IYIEJOBMBQORHUTUWVBE3HYSDYSYGCJMUIYYBL\",\"claimant_muxed\":\"MA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7M"
                "GXUA74P7UJVAAAAAAAAAAAACJUQ\",\"claimant_muxed_id\":\"1234\"}"));
            QCOMPARE(operation.getClaimantMuxed(), QStringLiteral("MA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVAAAAAAAAAAAACJUQ"));
            QCOMPARE(operation.getClaimantMuxedId(), QStringLiteral("1234"));
        }
        {
            EndSponsoringFutureReservesOperationResponse operation;
            operation.loadFromJson(QByteArray(
                "{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21254732290990083\"},\"transaction\":{\"href\":\"https://ho"
                "rizon-testnet.stellar.org/transactions/c8342db6542b7661bea0013ca2fefc089c16ffc8794d983ed43b9f61cad28ce0\"},\"effects\":{\"href\":\"https://h"
                "orizon-testnet.stellar.org/operations/21254732290990083/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?orde"
                "r=desc&cursor=21254732290990083\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21254732290990083\""
                "}},\"id\":\"21254732290990083\",\"paging_token\":\"21254732290990083\",\"transaction_successful\":true,\"source_account\":\"GBUXQKGGE3SL7IGD"
                "X6ZFMGACBBDDS6JSAPALZEMRVDO7M65YSNKHWICN\",\"type\":\"end_sponsoring_future_reserves\",\"type_i\":17,\"created_at\":\"2026-09-30T11:22:32Z\""
                ",\"transaction_hash\":\"c8342db6542b7661bea0013ca2fefc089c16ffc8794d983ed43b9f61cad28ce0\",\"begin_sponsor\":\"GD3N5KQ4TM6N7IQZFBPTQSKCI3D5H"
                "DAOPXM33PG5HNP7LQVSWHT4NAIF\",\"begin_sponsor_muxed\":\"MA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVAAAAAAAAAAAACJUQ\",\"begin_spon"
                "sor_muxed_id\":\"1234\"}"));
            QCOMPARE(operation.getBeginSponsorMuxed(), QStringLiteral("MA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVAAAAAAAAAAAACJUQ"));
            QCOMPARE(operation.getBeginSponsorMuxedId(), QStringLiteral("1234"));
        }
        {
            AllowTrustOperationResponse operation;
            operation.loadFromJson(QByteArray(
                "{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21253959196921857\"},\"transaction\":{\"href\":\"https://ho"
                "rizon-testnet.stellar.org/transactions/2f73439822be9311486bf0685b2324542a131d5bae92688b524b8e85189fa5b0\"},\"effects\":{\"href\":\"https://h"
                "orizon-testnet.stellar.org/operations/21253959196921857/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?orde"
                "r=desc&cursor=21253959196921857\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21253959196921857\""
                "}},\"id\":\"21253959196921857\",\"paging_token\":\"21253959196921857\",\"transaction_successful\":true,\"source_account\":\"GBDIC3YONQA6RW7O"
                "UGAFYRGFFOFHMU6GXHHGD6HLP422RWFJJUUP367K\",\"type\":\"allow_trust\",\"type_i\":7,\"created_at\":\"2026-09-30T11:07:32Z\",\"transaction_hash\""
                ":\"2f73439822be9311486bf0685b2324542a131d5bae92688b524b8e85189fa5b0\",\"asset_type\":\"credit_alphanum12\",\"asset_code\":\"ASTRO\",\"asset_"
                "issuer\":\"GBDIC3YONQA6RW7OUGAFYRGFFOFHMU6GXHHGD6HLP422RWFJJUUP367K\",\"trustee\":\"GBDIC3YONQA6RW7OUGAFYRGFFOFHMU6GXHHGD6HLP422RWFJJUUP367K"
                "\",\"trustor\":\"GDMQ6AYIKS7QIX7SYAHUZSU33BIHWJNBF5IS3POOZBBW6LVTEHEYNE6Z\",\"authorize\":true,\"authorize_to_maintain_liabilities\":true,\""
                "trustee_muxed\":\"MA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVAAAAAAAAAAAACJUQ\",\"trustee_muxed_id\":\"1234\"}"));
            QCOMPARE(operation.getTrusteeMuxed(), QStringLiteral("MA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVAAAAAAAAAAAACJUQ"));
            QCOMPARE(operation.getTrusteeMuxedId(), QStringLiteral("1234"));
        }
#if defined(_MSC_VER)
#  pragma warning(pop)
#elif defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic pop
#endif
    }

    void testDeserializeSetOptionsOperation() {
        QByteArray json = "{\n"
                "        \"created_at\": \"2026-09-28T15:16:12Z\",\n"
                "        \"transaction_hash\": \"a01d74e2cd51d962775838ea46d05ffa0f55e2ec67ad5eb687d5dd56e5d46d23\",\n"
                "        \"transaction_successful\": true,\n"
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
                "        \"set_flags\": [1],"
                "        \"set_flags_s\": [\n"
                "          \"auth_required_flag\"\n"
                "        ],"
                "        \"clear_flags\": [2],"
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
        // The values js reads (record.set_flags / clear_flags); not read before.
        QCOMPARE(operation.getSetFlagValues(), QList<qint32>() << 1);
        QCOMPARE(operation.getClearFlagValues(), QList<qint32>() << 2);
    }
    void testDeserializeSetOptionsOperationWithNonEd25519Key() {
        QByteArray json = "{\n"
                          "        \"transaction_successful\": true,\n"
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
        } catch (const std::runtime_error& e) {
            //QCOMPARE(QString::fromLatin1(e.what()),QString("Version byte is invalid"));
            Q_UNUSED(e)
        }
        QCOMPARE(operation.getSignerKey(), QString("TBGFYVCU76LJ7GZOCGR4X7DG2NV42JPG5CKRL42LA5FZOFI3U2WU7ZAL"));
    }
    void testDeserializeAccountMergeOperation() {
        QByteArray json = "{\n"
                "        \"created_at\": \"2026-09-28T15:16:12Z\",\n"
                "        \"transaction_hash\": \"460ad2e1599b562a335761fbceeff7296c94720528b4cd016fc9e31b5061f3c1\",\n"
                "        \"transaction_successful\": true,\n"
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
                "        \"created_at\": \"2026-09-28T15:16:12Z\",\n"
                "        \"transaction_hash\": \"a48c6744ed58a603a816784208ec0978ce894099787a2218de6be3cbcaccc989\",\n"
                "        \"transaction_successful\": true,\n"
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
                "        \"price\": \"32.6639943\",\n"
                "        \"price_r\": { \"n\": 326639943, \"d\": 10000000 },\n"
                "        \"buying_asset_type\": \"credit_alphanum4\",\n"
                "        \"buying_asset_code\": \"CNY\",\n"
                "        \"buying_asset_issuer\": \"GAZWSWPDQTBHFIPBY4FEDFW2J6E2LE7SZHJWGDZO6Q63W7DBSRICO2KN\",\n"
                "        \"selling_asset_type\": \"native\"\n"
                "      }";

        ManageSellOfferOperationResponse operation;
        operation.loadFromJson(json);

        QCOMPARE(operation.getOfferId(), 0);
        QCOMPARE(operation.getAmount(), QString("100.0"));
        QCOMPARE(operation.getPrice(), QString("32.6639943"));
        QCOMPARE(operation.getPriceR().getNumerator(), 326639943);
        QCOMPARE(operation.getPriceR().getDenominator(), 10000000);
        QVERIFY(operation.getBuyingAsset()->equals( Asset::createNonNativeAsset("CNY", KeyPair::fromAccountId(QString("GAZWSWPDQTBHFIPBY4FEDFW2J6E2LE7SZHJWGDZO6Q63W7DBSRICO2KN")))));
        QVERIFY(operation.getSellingAsset()->equals( new AssetTypeNative()));
    }


    void testDeserializeManageSellOfferOperationOfferIDString() {
        QByteArray json = "{\n"
                "        \"created_at\": \"2026-09-28T15:16:12Z\",\n"
                "        \"transaction_hash\": \"a48c6744ed58a603a816784208ec0978ce894099787a2218de6be3cbcaccc989\",\n"
                "        \"transaction_successful\": true,\n"
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
                "        \"selling_asset_type\": \"native\",\n"
                "        \"price\": \"0.5000000\",\n"
                "        \"price_r\": {\"n\": 1, \"d\": 2}\n"
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
                          "        \"created_at\": \"2026-09-28T15:16:12Z\",\n"
                          "        \"transaction_hash\": \"a48c6744ed58a603a816784208ec0978ce894099787a2218de6be3cbcaccc989\",\n"
                          "        \"transaction_successful\": true,\n"
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
                          "        \"price\": \"2.3058668\",\n"
                          "        \"price_r\": { \"n\": 5764667, \"d\": 2500000 },\n"
                          "        \"buying_asset_type\": \"credit_alphanum4\",\n"
                          "        \"buying_asset_code\": \"CNY\",\n"
                          "        \"buying_asset_issuer\": \"GAZWSWPDQTBHFIPBY4FEDFW2J6E2LE7SZHJWGDZO6Q63W7DBSRICO2KN\",\n"
                          "        \"selling_asset_type\": \"native\"\n"
                          "      }";

        ManageBuyOfferOperationResponse operation;
        operation.loadFromJson(json);
        QCOMPARE(operation.getOfferId(), 0);
        QCOMPARE(operation.getAmount(), QString("100.0"));
        QCOMPARE(operation.getPrice(), QString("2.3058668"));
        QCOMPARE(operation.getPriceR().getNumerator(), 5764667);
        QCOMPARE(operation.getPriceR().getDenominator(), 2500000);
        QVERIFY(operation.getBuyingAsset()->equals( Asset::createNonNativeAsset("CNY", KeyPair::fromAccountId(QString("GAZWSWPDQTBHFIPBY4FEDFW2J6E2LE7SZHJWGDZO6Q63W7DBSRICO2KN")))));
        QVERIFY(operation.getSellingAsset()->equals( new AssetTypeNative()));
    }


    void testDeserializeManageBuyOfferOperationOfferIDString() {
        QByteArray json = "{\n"
                          "        \"created_at\": \"2026-09-28T15:16:12Z\",\n"
                          "        \"transaction_hash\": \"a48c6744ed58a603a816784208ec0978ce894099787a2218de6be3cbcaccc989\",\n"
                          "        \"transaction_successful\": true,\n"
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
                          "        \"selling_asset_type\": \"native\",\n"
                          "        \"price\": \"0.5000000\",\n"
                          "        \"price_r\": {\"n\": 1, \"d\": 2}\n"
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
                "  \"source_account\": \"GDTBRJDRRQHHH4EPXOIBEAXTDJVETYVBOO53UGD4NDTW5P7YKG2DAWBD\",\n"
                "  \"created_at\": \"2026-09-28T15:16:12Z\",\n"
                "  \"transaction_hash\": \"abb1aa022a41f2c2c043757a7123ff661260f1f23557c6677b121b97ab515512\",\n"
                "  \"transaction_successful\": true,\n"
                "  \"path\": [],\n"
                "  \"source_amount\": \"10.0\",\n"
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
                          "  \"transaction_successful\": true,\n"
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
              "  \"transaction_successful\": true,\n"
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
                "  \"source_account\": \"GDTBRJDRRQHHH4EPXOIBEAXTDJVETYVBOO53UGD4NDTW5P7YKG2DAWBD\",\n"
                "  \"created_at\": \"2026-09-28T15:16:12Z\",\n"
                "  \"transaction_hash\": \"a31dfdbc3b5af20f44680b009207a7e25fdf9c12bcb8988c8e01733c084dc49b\",\n"
                "  \"transaction_successful\": true,\n"
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
        QCOMPARE(operation.getPrice(), QString("1.0"));
        QCOMPARE(operation.getPriceR().getNumerator(), 1);
        QCOMPARE(operation.getPriceR().getDenominator(), 1);
        QVERIFY(operation.getBuyingAsset()->equals(Asset::createNonNativeAsset("USD", KeyPair::fromAccountId(QString("GDS5JW5E6DRSSN5XK4LW7E6VUMFKKE2HU5WCOVFTO7P2RP7OXVCBLJ3Y")))));
        QVERIFY(operation.getSellingAsset()->equals( new AssetTypeNative()));
    }
    void testDeserializeInflationOperation() {
        QByteArray json = "{\n"
                "  \"source_account\": \"GDTBRJDRRQHHH4EPXOIBEAXTDJVETYVBOO53UGD4NDTW5P7YKG2DAWBD\",\n"
                "  \"created_at\": \"2026-09-28T15:16:12Z\",\n"
                "  \"transaction_hash\": \"605b53665fcf113cb68c3f434c6dc9e9d23563f279be0588ceff2cd6ca64bbc6\",\n"
                "  \"transaction_successful\": true,\n"
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
                "  \"created_at\": \"2026-09-28T15:16:12Z\",\n"
                "  \"transaction_hash\": \"0a5e57075147f9a48f89ef12055a16f49edb58fd379fd39af06d46c034449b2e\",\n"
                "  \"transaction_successful\": true,\n"
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
                "  \"created_at\": \"2026-09-28T15:16:12Z\",\n"
                "  \"transaction_hash\": \"0a5e57075147f9a48f89ef12055a16f49edb58fd379fd39af06d46c034449b2e\",\n"
                "  \"transaction_successful\": true,\n"
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
                "  \"value\": \"\"\n"
                "}";

        // Horizon sends a deleted entry's value as "", never null (its Go
        // struct holds a string); null, which Horizon never sends, is refused
        // now, as for every member Horizon always sends. This read null.
        ManageDataOperationResponse operation;
        operation.loadFromJson(json);

        QCOMPARE(operation.getValue(), QString(""));
        QByteArray nulled = json;
        nulled.replace("\"value\": \"\"", "\"value\": null");
        ManageDataOperationResponse refused;
        QVERIFY_EXCEPTION_THROWN(refused.loadFromJson(nulled), std::runtime_error);
    }

    void testDeserializeBumpSequenceOperation() {
        QByteArray json = "{\n"
              "  \"source_account\": \"GDTBRJDRRQHHH4EPXOIBEAXTDJVETYVBOO53UGD4NDTW5P7YKG2DAWBD\",\n"
              "  \"created_at\": \"2026-09-28T15:16:12Z\",\n"
              "  \"transaction_hash\": \"605b53665fcf113cb68c3f434c6dc9e9d23563f279be0588ceff2cd6ca64bbc6\",\n"
              "  \"transaction_successful\": true,\n"
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
          "  \"source_account\": \"GDTBRJDRRQHHH4EPXOIBEAXTDJVETYVBOO53UGD4NDTW5P7YKG2DAWBD\",\n"
          "  \"created_at\": \"2026-09-28T15:16:12Z\",\n"
          "  \"transaction_hash\": \"605b53665fcf113cb68c3f434c6dc9e9d23563f279be0588ceff2cd6ca64bbc6\",\n"
          "  \"transaction_successful\": true,\n"
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
          "  \"source_account\": \"GDTBRJDRRQHHH4EPXOIBEAXTDJVETYVBOO53UGD4NDTW5P7YKG2DAWBD\",\n"
          "  \"created_at\": \"2026-09-28T15:16:12Z\",\n"
          "  \"transaction_hash\": \"605b53665fcf113cb68c3f434c6dc9e9d23563f279be0588ceff2cd6ca64bbc6\",\n"
          "  \"transaction_successful\": true,\n"
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
          "  \"amount\": \"100.0000000\",\n"
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
          "  \"source_account\": \"GDTBRJDRRQHHH4EPXOIBEAXTDJVETYVBOO53UGD4NDTW5P7YKG2DAWBD\",\n"
          "  \"created_at\": \"2026-09-28T15:16:12Z\",\n"
          "  \"transaction_hash\": \"605b53665fcf113cb68c3f434c6dc9e9d23563f279be0588ceff2cd6ca64bbc6\",\n"
          "  \"transaction_successful\": true,\n"
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

    // Mirrors java-stellar-sdk OperationResponseTest#testClaimClaimableBalanceOperation
    // (same balance_id / claimant values → expects the same parsed response).
    void testDeserializeClaimClaimableBalanceOperation() {
      QByteArray json = "{\n"
          "  \"source_account\": \"GDTBRJDRRQHHH4EPXOIBEAXTDJVETYVBOO53UGD4NDTW5P7YKG2DAWBD\",\n"
          "  \"created_at\": \"2026-09-28T15:16:12Z\",\n"
          "  \"transaction_hash\": \"605b53665fcf113cb68c3f434c6dc9e9d23563f279be0588ceff2cd6ca64bbc6\",\n"
          "  \"transaction_successful\": true,\n"
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
          "  \"type_i\": 15,\n"
          "  \"type\": \"claim_claimable_balance\",\n"
          "  \"balance_id\": \"00000000a5c8c85c12a32ec1b30fc1792a542ca38702afd78eb4fe524d028887cf6b6952\",\n"
          "  \"claimant\": \"GAEY7JFLBBDD6PAUPVRVKMBNSL5W6GYMUOGJKNGHGFSFGJU6CT2IUARS\"\n"
          "}";

      ClaimClaimableBalanceOperationResponse operation;
      operation.loadFromJson(json);

      QCOMPARE(operation.getId(), 12884914177L);
      QCOMPARE(operation.getType(), "claim_claimable_balance");
      QCOMPARE(operation.getBalanceID(), "00000000a5c8c85c12a32ec1b30fc1792a542ca38702afd78eb4fe524d028887cf6b6952");
      QCOMPARE(operation.getClaimant(), "GAEY7JFLBBDD6PAUPVRVKMBNSL5W6GYMUOGJKNGHGFSFGJU6CT2IUARS");
    }

    // Mirrors java-stellar-sdk OperationResponseTest#testChangeTrustOperationWithLiquidityPoolId.
    // (Muxed trustor fields asserted by Java are CAP-27 / roadmap P1.5, not yet ported.)
    void testDeserializeChangeTrustOperationLiquidityPool() {
      QByteArray json = "{\n"
          "  \"created_at\": \"2026-09-28T15:16:12Z\",\n"
          "  \"transaction_hash\": \"7a6c1aa609b9690b5d9315f194539a78178588e8d533c147df1b50db732ea387\",\n"
          "  \"transaction_successful\": true,\n"
          "  \"_links\": {\n"
          "    \"self\": {\n"
          "      \"href\": \"//horizon-testnet.stellar.org/operations/3602970755207169\"\n"
          "    },\n"
          "    \"transaction\": {\n"
          "      \"href\": \"//horizon-testnet.stellar.org/transactions/8d409a788543895843d269c3f97a2d6a2ebca6e9f8f9a7ae593457b5c0ba6644\"\n"
          "    },\n"
          "    \"effects\": {\n"
          "      \"href\": \"//horizon-testnet.stellar.org/operations/3602970755207169/effects\"\n"
          "    },\n"
          "    \"succeeds\": {\n"
          "      \"href\": \"//horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=3602970755207169\"\n"
          "    },\n"
          "    \"precedes\": {\n"
          "      \"href\": \"//horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=3602970755207169\"\n"
          "    }\n"
          "  },\n"
          "  \"id\": \"3602970755207169\",\n"
          "  \"paging_token\": \"3602970755207169\",\n"
          "  \"source_account\": \"GAQXAWHCM4A7SQCT3BOSVEGRI2OOB7LO2CMFOYFF6YRXU4VQSB5V2V2K\",\n"
          "  \"type\": \"change_trust\",\n"
          "  \"type_i\": 6,\n"
          "  \"asset_type\": \"liquidity_pool_shares\",\n"
          "  \"liquidity_pool_id\": \"2c0bfa623845dd101cbf074a1ca1ae4b2458cc8d0104ad65939ebe2cd9054355\",\n"
          "  \"limit\": \"922337203685.4775807\",\n"
          "  \"trustor\": \"GAQXAWHCM4A7SQCT3BOSVEGRI2OOB7LO2CMFOYFF6YRXU4VQSB5V2V2K\"\n"
          "}";

      ChangeTrustOperationResponse operation;
      operation.loadFromJson(json);

      QCOMPARE(operation.getType(), "change_trust");
      QCOMPARE(operation.assetType(), QString("liquidity_pool_shares"));
      QCOMPARE(operation.getLiquidityPoolId(), QString("2c0bfa623845dd101cbf074a1ca1ae4b2458cc8d0104ad65939ebe2cd9054355"));
      QCOMPARE(operation.getLimit(), QString("922337203685.4775807"));
      QCOMPARE(operation.getTrustor().getAccountId(), QString("GAQXAWHCM4A7SQCT3BOSVEGRI2OOB7LO2CMFOYFF6YRXU4VQSB5V2V2K"));
    }

    // ---- Exact reading (1.0.4) -------------------------------------------
private:
    /** `json` with the member at `path` ("a.b[1].c") set to `value`, or removed when undefined. */
    static QJsonValue edited(const QJsonValue& json, QStringList path, const QJsonValue& value)
    {
        const QString step = path.takeFirst();
        const int bracket = step.indexOf(QLatin1Char('['));
        const QString key = bracket < 0 ? step : step.left(bracket);
        QJsonObject object = json.toObject();
        if (bracket >= 0) {
            const int index = step.mid(bracket + 1, step.size() - bracket - 2).toInt();
            QJsonArray array = object.value(key).toArray();
            array[index] = path.isEmpty() ? value : edited(array.at(index), path, value);
            object[key] = array;
        } else if (path.isEmpty()) {
            if (value.isUndefined())
                object.remove(key);
            else
                object[key] = value;
        } else {
            object[key] = edited(object.value(key), path, value);
        }
        return object;
    }

    /** What a T loading `json` with `member` set to `value` throws; empty if it loads. */
    template<class T>
    static QString refusalOf(const QByteArray& json, const QString& member, const QJsonValue& value)
    {
        const QJsonValue changed = edited(QJsonDocument::fromJson(json).object(),
                                          member.split(QLatin1Char('.')), value);
        T response;
        try {
            response.loadFromJson(QJsonDocument(changed.toObject()).toJson());
        } catch (const std::exception& e) {
            return QString::fromUtf8(e.what());
        }
        return QString();
    }



private:
    static QByteArray realPayment()
    {
        return QByteArray(
            "{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118341309550593\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/f4ae1f4eb66fe406aa8bd52572c58eaed9d8fe5dfc8b80d83588bc7a3b4cbb61\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118341309550593/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118341309550593\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118341309550593\"}},\"id\":\"21118341309550593\",\"paging_token\":\"21118341309550593\",\"transaction_successful\":true,\"source_account\":\"GDTBRJDRRQHHH4EPXOIBEAXTDJVETYVBOO53UGD4NDTW5P7YKG2DAWBD\",\"type\":\"payment\",\"type_i\":1,\"created_at\":\"2026-09-28T15:16:12Z\",\"transaction_hash\":\"f4ae1f4eb66fe406aa8bd52572c58eaed9d8fe5dfc8b80d83588bc7a3b4cbb61\",\"asset_type\":\"credit_alphanum12\",\"asset_code\":\"LWTEST\",\"asset_issuer\":\"GAPIM2WRYS65WEWE2PAJYNGST35ZQXEJJQKWG7EUVMEZ2LWP7SXSNTNO\",\"from\":\"GDTBRJDRRQHHH4EPXOIBEAXTDJVETYVBOO53UGD4NDTW5P7YKG2DAWBD\",\"to\":\"GAPIM2WRYS65WEWE2PAJYNGST35ZQXEJJQKWG7EUVMEZ2LWP7SXSNTNO\",\"amount\":\"99.9999000\"}");
    }
    static QByteArray realManageSellOffer()
    {
        return QByteArray(
            "{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118040661839873\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/46dd02c512d947df0a6afbd6bdc97fb05e8413689a753a53691297671090dd2d\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118040661839873/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118040661839873\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118040661839873\"}},\"id\":\"21118040661839873\",\"paging_token\":\"21118040661839873\",\"transaction_successful\":true,\"source_account\":\"GBCVBQDCFPOTNQX4A4ZTIJNCARSRK24C7TGG5QZBKOVQL3FDA4HVJ3MJ\",\"type\":\"manage_sell_offer\",\"type_i\":3,\"created_at\":\"2026-09-28T15:10:22Z\",\"transaction_hash\":\"46dd02c512d947df0a6afbd6bdc97fb05e8413689a753a53691297671090dd2d\",\"amount\":\"3222.8235339\",\"price\":\"0.2177380\",\"price_r\":{\"n\":108869,\"d\":500000},\"buying_asset_type\":\"credit_alphanum12\",\"buying_asset_code\":\"CITEST\",\"buying_asset_issuer\":\"GBMMZMK2DC4FFP4CAI6KCVNCQ7WLO5A7DQU7EC7WGHRDQBZB763X4OQI\",\"selling_asset_type\":\"native\",\"offer_id\":\"828697\"}");
    }
    static QByteArray realPathPaymentStrictSend()
    {
        return QByteArray(
            "{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118341309562881\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/4d6a195a7377e04f0b3c1866c5e725c3ec6f48d248a91a2dd474e619f99aa7a5\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118341309562881/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118341309562881\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118341309562881\"}},\"id\":\"21118341309562881\",\"paging_token\":\"21118341309562881\",\"transaction_successful\":true,\"source_account\":\"GBMXZ62UDDWDWZU464HDDFGO6LZ4IAFPHZTIU3JPODNZQFAUSCIRMHCM\",\"type\":\"path_payment_strict_send\",\"type_i\":13,\"created_at\":\"2026-09-28T15:16:12Z\",\"transaction_hash\":\"4d6a195a7377e04f0b3c1866c5e725c3ec6f48d248a91a2dd474e619f99aa7a5\",\"asset_type\":\"credit_alphanum4\",\"asset_code\":\"USDC\",\"asset_issuer\":\"GCDXYIOQJXFYG42J6C4K4NN7TGMX4TFF5DZK2ENDMNHONRR5D3KIPDS5\",\"from\":\"GBMXZ62UDDWDWZU464HDDFGO6LZ4IAFPHZTIU3JPODNZQFAUSCIRMHCM\",\"to\":\"GBMXZ62UDDWDWZU464HDDFGO6LZ4IAFPHZTIU3JPODNZQFAUSCIRMHCM\",\"amount\":\"3.7153200\",\"path\":[],\"source_amount\":\"1793.5502249\",\"destination_min\":\"3.5295541\",\"source_asset_type\":\"credit_alphanum12\",\"source_asset_code\":\"SMAJDV\",\"source_asset_issuer\":\"GBHOK32DC5HPBQUQTUPK5Y66A27MSBQGK4KDRLP4BGEQFL5O3AFTHIH5\"}");
    }
    static QByteArray realLiquidityPoolDeposit()
    {
        return QByteArray(
            "{\"_links\":{\"self\":{\"href\":\"https://horizon.stellar.org/operations/277731706554052609\"},\"transaction\":{\"href\":\"https://horizon.stellar.org/transactions/1b55f31a4cd7312a4782da47520b5e46cb405269fc5679505094e403884fdad9\"},\"effects\":{\"href\":\"https://horizon.stellar.org/operations/277731706554052609/effects\"},\"succeeds\":{\"href\":\"https://horizon.stellar.org/effects?order=desc&cursor=277731706554052609\"},\"precedes\":{\"href\":\"https://horizon.stellar.org/effects?order=asc&cursor=277731706554052609\"}},\"id\":\"277731706554052609\",\"paging_token\":\"277731706554052609\",\"transaction_successful\":true,\"source_account\":\"GDQVJIGZ5TPOQLMVUQ6AWX5XZNE3YDEL5UXAOM3574U4HGNKS2LE4RMD\",\"type\":\"liquidity_pool_deposit\",\"type_i\":22,\"created_at\":\"2026-09-28T15:07:21Z\",\"transaction_hash\":\"1b55f31a4cd7312a4782da47520b5e46cb405269fc5679505094e403884fdad9\",\"liquidity_pool_id\":\"ad45b80a3a90ccaa251375e208006651a9f7242adc73e0d6928b0f3b2b0255c2\",\"reserves_max\":[{\"asset\":\"SSLX:GBHFGY3ZNEJWLNO4LBUKLYOCEK4V7ENEBJGPRHHX7JU47GWHBREH37UR\",\"amount\":\"2128.4295531\"},{\"asset\":\"STARDUST:GCPLNANLXTV2G6NPIRDPOLM5NAO5XH2IPYQBZCTTAKPNV4IRTU5X5B4S\",\"amount\":\"1267631.6231215\"}],\"min_price\":\"0.0016622\",\"min_price_r\":{\"n\":8311,\"d\":5000000},\"max_price\":\"0.0016958\",\"max_price_r\":{\"n\":8479,\"d\":5000000},\"reserves_deposited\":[{\"asset\":\"SSLX:GBHFGY3ZNEJWLNO4LBUKLYOCEK4V7ENEBJGPRHHX7JU47GWHBREH37UR\",\"amount\":\"2128.4295531\"},{\"asset\":\"STARDUST:GCPLNANLXTV2G6NPIRDPOLM5NAO5XH2IPYQBZCTTAKPNV4IRTU5X5B4S\",\"amount\":\"1267626.0350791\"}],\"shares_received\":\"21585.6866624\"}");
    }
    /** A real testnet create_claimable_balance whose claimant's time is past the year 9999. */
    static QByteArray realCreateClaimableBalance()
    {
        return QByteArray(
            "{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118938309996547\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/634d896ffe54cdd6b0d1a4b639ab47e1853a5f24f684d5ebccd7ea5fe955af3c\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118938309996547/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118938309996547\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118938309996547\"}},\"id\":\"21118938309996547\",\"paging_token\":\"21118938309996547\",\"transaction_successful\":true,\"source_account\":\"GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD\",\"type\":\"create_claimable_balance\",\"type_i\":14,\"created_at\":\"2026-09-28T15:27:47Z\",\"transaction_hash\":\"634d896ffe54cdd6b0d1a4b639ab47e1853a5f24f684d5ebccd7ea5fe955af3c\",\"sponsor\":\"GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD\",\"asset\":\"native\",\"amount\":\"3.5000000\",\"claimants\":[{\"destination\":\"GBTTX7QVGGB6VTNNAKMXIFR6YL3RYA6UU3YR7FVBLKNOB2SWSA7XMQ7H\",\"predicate\":{\"abs_before\":\"+146138514283-06-19T07:45:04Z\",\"abs_before_epoch\":\"4611686018427387904\"}}]}");
    }

private slots:
    /**
     * operations().operation(id) read Horizon's "type", which is a name
     * ("payment"), as a number: every operation became a
     * CreateAccountOperationResponse, which had no place for a payment's
     * amount or asset, and one of type 13 or later was refused. It reads
     * type_i, like a page of operations.
     */
    void testGenericOperationReadsItsType()
    {
        GenericOperation payment;
        payment.loadFromJson(realPayment());
        auto* read = dynamic_cast<PaymentOperationResponse*>(payment.operation());
        QVERIFY(read != nullptr);
        QCOMPARE(read->getAmount(), QString("99.9999000"));
        QCOMPARE(read->getAsset()->getType(), QString("credit_alphanum12"));

        GenericOperation claimable;
        claimable.loadFromJson(realCreateClaimableBalance());
        auto* create = dynamic_cast<CreateClaimableBalanceOperationResponse*>(claimable.operation());
        QVERIFY(create != nullptr);
        QCOMPARE(create->getAmount(), QString("3.5000000"));
        Predicate::AbsBefore absBefore(dynamic_cast<const Predicate::AbsBefore&>(create->getClaimants().at(0).getPredicate()));
        QCOMPARE(absBefore.getTimestampSeconds(), Q_INT64_C(4611686018427387904));

        // Read again, it holds the new operation.
        claimable.loadFromJson(realPayment());
        QVERIFY(dynamic_cast<PaymentOperationResponse*>(claimable.operation()) != nullptr);

        QVERIFY(refusalOf<GenericOperation>(realPayment(), "type_i", QJsonValue(QJsonValue::Undefined)).startsWith("type_i: "));
        QVERIFY(refusalOf<GenericOperation>(realPayment(), "type_i", QString("payment")).startsWith("type_i: "));
        // A type this SDK does not know is kept as a plain OperationResponse.
        QVERIFY(refusalOf<GenericOperation>(realPayment(), "type_i", 99).isEmpty());
        QVERIFY(refusalOf<GenericOperation>(realPayment(), "type_i", -1).startsWith("type_i: "));
    }

    /**
     * What an operation moved, and at what price, exactly: an amount given
     * as a number or with 8 decimals, a price term that is not an int32, an
     * asset type the code does not fit, a bump_to that is not a number (it
     * read as 0), a threshold past a byte.
     */
    void testOperationsAreReadExactly()
    {
        QVERIFY(refusalOf<PaymentOperationResponse>(realPayment(), "amount", 99.9999).startsWith("amount: "));
        QVERIFY(refusalOf<PaymentOperationResponse>(realPayment(), "amount", QString("99.99990001")).startsWith("amount: "));
        QVERIFY(refusalOf<PaymentOperationResponse>(realPayment(), "amount", QJsonValue(QJsonValue::Undefined)).startsWith("amount: "));
        QVERIFY(refusalOf<PaymentOperationResponse>(realPayment(), "asset_type", QString("credit_alphanum4")).startsWith("asset_code: "));
        QVERIFY(refusalOf<PaymentOperationResponse>(realPayment(), "asset_issuer", QJsonValue(QJsonValue::Undefined)).startsWith("asset_issuer: "));
        QVERIFY(refusalOf<PaymentOperationResponse>(realPayment(), "id", QString("2111834130955059x")).startsWith("id: "));
        QVERIFY(refusalOf<PaymentOperationResponse>(realPayment(), "transaction_successful", QString("true")).startsWith("transaction_successful: "));
        QVERIFY(refusalOf<ManageSellOfferOperationResponse>(realManageSellOffer(), "price_r.n", QString("108869.5")).startsWith("price_r.n: "));
        QVERIFY(refusalOf<ManageSellOfferOperationResponse>(realManageSellOffer(), "price_r.d", 0).startsWith("price_r.d: "));
        QVERIFY(refusalOf<ManageSellOfferOperationResponse>(realManageSellOffer(), "offer_id", QString("828697x")).startsWith("offer_id: "));
        QVERIFY(refusalOf<ManageSellOfferOperationResponse>(realManageSellOffer(), "selling_asset_type", QString("native4")).startsWith("selling_asset_type: "));
        QVERIFY(refusalOf<PathPaymentStrictSendOperationResponse>(realPathPaymentStrictSend(), "path",
                    QJsonArray{QJsonObject{{"asset_type", "credit_alphanum12"}, {"asset_code", "USD"},
                                           {"asset_issuer", "GCDXYIOQJXFYG42J6C4K4NN7TGMX4TFF5DZK2ENDMNHONRR5D3KIPDS5"}}})
                    .startsWith("path[0].asset_code: "));
        QVERIFY(refusalOf<PathPaymentStrictSendOperationResponse>(realPathPaymentStrictSend(), "path", QJsonArray{QString("native")}).startsWith("path[0]: "));
        QVERIFY(refusalOf<PathPaymentStrictSendOperationResponse>(realPathPaymentStrictSend(), "destination_min", 3.5).startsWith("destination_min: "));
        QVERIFY(refusalOf<LiquidityPoolDepositOperationResponse>(realLiquidityPoolDeposit(), "min_price_r.d", QString("0")).startsWith("min_price_r.d: "));
        QVERIFY(refusalOf<LiquidityPoolDepositOperationResponse>(realLiquidityPoolDeposit(), "reserves_max[1].amount", 5).startsWith("reserves_max[1].amount: "));
        QVERIFY(refusalOf<BumpSequenceOperationResponse>(realPayment(), "bump_to", QString("7947x")).startsWith("bump_to: "));
        QVERIFY(refusalOf<SetOptionsOperationResponse>(realPayment(), "high_threshold", 256).startsWith("high_threshold: "));
        QVERIFY(refusalOf<SetOptionsOperationResponse>(realPayment(), "master_key_weight", QString("heavy")).startsWith("master_key_weight: "));
        QVERIFY(refusalOf<CreateClaimableBalanceOperationResponse>(realCreateClaimableBalance(),
                    "claimants[0].predicate.abs_before_epoch", QString("4611686018427387904.5"))
                    .startsWith("claimants[0].predicate.abs_before_epoch: "));

        LiquidityPoolDepositOperationResponse deposit;
        deposit.loadFromJson(realLiquidityPoolDeposit());
        QCOMPARE(deposit.getMinPriceR().getNumerator(), 8311);
        QCOMPARE(deposit.getMaxPriceR().getDenominator(), 5000000);
        // The getters read their terms exactly too: toInt() made these 0.
        deposit.setProperty("min_price_r", QVariantMap{{"n", QString("x")}, {"d", 1}});
        QVERIFY_EXCEPTION_THROWN(deposit.getMinPriceR(), std::runtime_error);
        ManageSellOfferOperationResponse offer;
        offer.loadFromJson(realManageSellOffer());
        QCOMPARE(offer.getPriceR().getNumerator(), 108869);
        offer.setProperty("price_r", QVariantMap{{"n", 1}, {"d", 0}});
        QVERIFY_EXCEPTION_THROWN(offer.getPriceR(), std::runtime_error);
    }
};
ADD_TEST(OperationDeserializerTest)
#endif // OPERATIONDESERIALIZERTEST_H
