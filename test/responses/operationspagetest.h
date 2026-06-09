#ifndef OPERATIONSPAGETEST_H
#define OPERATIONSPAGETEST_H
#include <QObject>


#include <QtTest>
#include "../testcollector.h"
#include "../../src/responses/operationpage.h"
#include "../../src/responses/operations/createaccountoperationresponse.h"
#include "../../src/responses/operations/paymentoperationresponse.h"
#include "../../src/responses/operations/revokesponsorshipoperationresponse.h"
#include "../../src/responses/operations/liquiditypooldepositoperationresponse.h"
#include "../../src/responses/operations/liquiditypoolwithdrawoperationresponse.h"

#include "../../src/keypair.h"
#include "../../src/assettypenative.h"

class OperationsPageTest: public QObject
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
            OperationPage operationsPage(0);
            operationsPage.loadFromJson(json);

            CreateAccountOperationResponse* createAccountOperation = (CreateAccountOperationResponse*) operationsPage.at(0);
            QCOMPARE(createAccountOperation->getStartingBalance(), QString("10000.0"));
            QCOMPARE(createAccountOperation->getPagingToken(), QString("3717508943056897"));
            QCOMPARE(createAccountOperation->getAccount().getAccountId(), QString("GDFH4NIYMIIAKRVEJJZOIGWKXGQUF3XHJG6ZM6CEA64AMTVDN44LHOQE"));
            QCOMPARE(createAccountOperation->getFunder().getAccountId(), QString("GBS43BF24ENNS3KPACUZVKK2VYPOZVBQO2CISGZ777RYGOPYC2FT6S3K"));
            QCOMPARE(createAccountOperation->getCreatedAt(), QString("2018-01-22T21:30:53Z"));
            QCOMPARE(createAccountOperation->getTransactionHash(), QString("dd9d10c80a344f4464df3ecaa63705a5ef4a0533ff2f2099d5ef371ab5e1c046"));

            PaymentOperationResponse* paymentOperation = (PaymentOperationResponse*) operationsPage.at(4);
            QCOMPARE(paymentOperation->getAmount(), QString("10.123"));
            QVERIFY(dynamic_cast<AssetTypeNative*>(paymentOperation->getAsset()));
            QCOMPARE(paymentOperation->getFrom().getAccountId(), QString("GCYK67DDGBOANS6UODJ62QWGLEB2A7JQ3XUV25HCMLT7CI23PMMK3W6R"));
            QCOMPARE(paymentOperation->getTo().getAccountId(), QString("GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H"));
    }
private:

    const QByteArray json = "{\n"
            "  \"_links\": {\n"
            "    \"self\": {\n"
            "      \"href\": \"http://horizon-testnet.stellar.org/operations?order=desc\\u0026limit=10\\u0026cursor=\"\n"
            "    },\n"
            "    \"next\": {\n"
            "      \"href\": \"http://horizon-testnet.stellar.org/operations?order=desc\\u0026limit=10\\u0026cursor=3695540185337857\"\n"
            "    },\n"
            "    \"prev\": {\n"
            "      \"href\": \"http://horizon-testnet.stellar.org/operations?order=asc\\u0026limit=10\\u0026cursor=3717508943056897\"\n"
            "    }\n"
            "  },\n"
            "  \"_embedded\": {\n"
            "    \"records\": [\n"
            "      {\n"
            "        \"_links\": {\n"
            "          \"self\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/operations/3717508943056897\"\n"
            "          },\n"
            "          \"transaction\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/transactions/ce81d957352501a46d9b938462cbef76283dcba8108d2649e0d79279a8f36488\"\n"
            "          },\n"
            "          \"effects\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/operations/3717508943056897/effects\"\n"
            "          },\n"
            "          \"succeeds\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=3717508943056897\"\n"
            "          },\n"
            "          \"precedes\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=3717508943056897\"\n"
            "          }\n"
            "        },\n"
            "        \"id\": \"3717508943056897\",\n"
            "        \"paging_token\": \"3717508943056897\",\n"
            "        \"source_account\": \"GBS43BF24ENNS3KPACUZVKK2VYPOZVBQO2CISGZ777RYGOPYC2FT6S3K\",\n"
            "        \"type\": \"create_account\",\n"
            "        \"type_i\": 0,\n"
            "        \"created_at\": \"2018-01-22T21:30:53Z\",\n"
            "        \"transaction_hash\": \"dd9d10c80a344f4464df3ecaa63705a5ef4a0533ff2f2099d5ef371ab5e1c046\","
            "        \"starting_balance\": \"10000.0\",\n"
            "        \"funder\": \"GBS43BF24ENNS3KPACUZVKK2VYPOZVBQO2CISGZ777RYGOPYC2FT6S3K\",\n"
            "        \"account\": \"GDFH4NIYMIIAKRVEJJZOIGWKXGQUF3XHJG6ZM6CEA64AMTVDN44LHOQE\"\n"
            "      },\n"
            "      {\n"
            "        \"_links\": {\n"
            "          \"self\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/operations/3715417293983745\"\n"
            "          },\n"
            "          \"transaction\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/transactions/57d3ff20b53a21bd2a5c24838401e01fc13abc0193437d050dbdb8b7784cd674\"\n"
            "          },\n"
            "          \"effects\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/operations/3715417293983745/effects\"\n"
            "          },\n"
            "          \"succeeds\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=3715417293983745\"\n"
            "          },\n"
            "          \"precedes\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=3715417293983745\"\n"
            "          }\n"
            "        },\n"
            "        \"id\": \"3715417293983745\",\n"
            "        \"paging_token\": \"3715417293983745\",\n"
            "        \"source_account\": \"GBS43BF24ENNS3KPACUZVKK2VYPOZVBQO2CISGZ777RYGOPYC2FT6S3K\",\n"
            "        \"type\": \"create_account\",\n"
            "        \"type_i\": 0,\n"
            "        \"starting_balance\": \"10000.0\",\n"
            "        \"funder\": \"GBS43BF24ENNS3KPACUZVKK2VYPOZVBQO2CISGZ777RYGOPYC2FT6S3K\",\n"
            "        \"account\": \"GA4GQOVE7SQCPGDCXOKIUWGZYJCMMA3TCJUB54ZYYCNMZD7MVILGEADL\"\n"
            "      },\n"
            "      {\n"
            "        \"_links\": {\n"
            "          \"self\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/operations/3711620542894081\"\n"
            "          },\n"
            "          \"transaction\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/transactions/e37b9dd7f36397e7a06ef121fb5446431585d30f8f3cf1d63a6d283e8f7b5a8c\"\n"
            "          },\n"
            "          \"effects\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/operations/3711620542894081/effects\"\n"
            "          },\n"
            "          \"succeeds\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=3711620542894081\"\n"
            "          },\n"
            "          \"precedes\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=3711620542894081\"\n"
            "          }\n"
            "        },\n"
            "        \"id\": \"3711620542894081\",\n"
            "        \"paging_token\": \"3711620542894081\",\n"
            "        \"source_account\": \"GBS43BF24ENNS3KPACUZVKK2VYPOZVBQO2CISGZ777RYGOPYC2FT6S3K\",\n"
            "        \"type\": \"create_account\",\n"
            "        \"type_i\": 0,\n"
            "        \"starting_balance\": \"10000.0\",\n"
            "        \"funder\": \"GBS43BF24ENNS3KPACUZVKK2VYPOZVBQO2CISGZ777RYGOPYC2FT6S3K\",\n"
            "        \"account\": \"GDD2PUGSEGWN6TSHQNE6EDHVUYH6I37Y7727V3WFXUS2GGO4ZJ72EPEZ\"\n"
            "      },\n"
            "      {\n"
            "        \"_links\": {\n"
            "          \"self\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/operations/3709305555521537\"\n"
            "          },\n"
            "          \"transaction\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/transactions/af6ab37cfbeefc62a215ab7c4f64b007b666eed0c12dd92abbe0af08461d7b7f\"\n"
            "          },\n"
            "          \"effects\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/operations/3709305555521537/effects\"\n"
            "          },\n"
            "          \"succeeds\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=3709305555521537\"\n"
            "          },\n"
            "          \"precedes\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=3709305555521537\"\n"
            "          }\n"
            "        },\n"
            "        \"id\": \"3709305555521537\",\n"
            "        \"paging_token\": \"3709305555521537\",\n"
            "        \"source_account\": \"GBS43BF24ENNS3KPACUZVKK2VYPOZVBQO2CISGZ777RYGOPYC2FT6S3K\",\n"
            "        \"type\": \"create_account\",\n"
            "        \"type_i\": 0,\n"
            "        \"starting_balance\": \"10000.0\",\n"
            "        \"funder\": \"GBS43BF24ENNS3KPACUZVKK2VYPOZVBQO2CISGZ777RYGOPYC2FT6S3K\",\n"
            "        \"account\": \"GDPMWVUTK7T5NG2OHDAZLGLT7QS5GCL23CFVNUR3BNKVPDLW2RULWE7Z\"\n"
            "      },\n"
            "      {\n"
            "        \"_links\": {\n"
            "          \"self\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/operations/3704821609664513\"\n"
            "          },\n"
            "          \"transaction\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/transactions/d4cb66d51cf773a4126ef8d535f03ba08cdc2389dc067e05c5d2ba1b335f19f2\"\n"
            "          },\n"
            "          \"effects\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/operations/3704821609664513/effects\"\n"
            "          },\n"
            "          \"succeeds\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=3704821609664513\"\n"
            "          },\n"
            "          \"precedes\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=3704821609664513\"\n"
            "          }\n"
            "        },\n"
            "        \"id\": \"3704821609664513\",\n"
            "        \"paging_token\": \"3704821609664513\",\n"
            "        \"source_account\": \"GCYK67DDGBOANS6UODJ62QWGLEB2A7JQ3XUV25HCMLT7CI23PMMK3W6R\",\n"
            "        \"type\": \"payment\",\n"
            "        \"type_i\": 1,\n"
            "        \"asset_type\": \"native\",\n"
            "        \"from\": \"GCYK67DDGBOANS6UODJ62QWGLEB2A7JQ3XUV25HCMLT7CI23PMMK3W6R\",\n"
            "        \"to\": \"GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H\",\n"
            "        \"amount\": \"10.123\"\n"
            "      },\n"
            "      {\n"
            "        \"_links\": {\n"
            "          \"self\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/operations/3704778659991553\"\n"
            "          },\n"
            "          \"transaction\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/transactions/741a66dee5bafdefa1803bd80108fb86b075bbca80165bc4137c8f8ad1959efa\"\n"
            "          },\n"
            "          \"effects\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/operations/3704778659991553/effects\"\n"
            "          },\n"
            "          \"succeeds\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=3704778659991553\"\n"
            "          },\n"
            "          \"precedes\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=3704778659991553\"\n"
            "          }\n"
            "        },\n"
            "        \"id\": \"3704778659991553\",\n"
            "        \"paging_token\": \"3704778659991553\",\n"
            "        \"source_account\": \"GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H\",\n"
            "        \"type\": \"payment\",\n"
            "        \"type_i\": 1,\n"
            "        \"asset_type\": \"native\",\n"
            "        \"from\": \"GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H\",\n"
            "        \"to\": \"GCYK67DDGBOANS6UODJ62QWGLEB2A7JQ3XUV25HCMLT7CI23PMMK3W6R\",\n"
            "        \"amount\": \"10.123\"\n"
            "      },\n"
            "      {\n"
            "        \"_links\": {\n"
            "          \"self\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/operations/3704435062607873\"\n"
            "          },\n"
            "          \"transaction\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/transactions/9bf15f7003c098c935d03bd178eda02b39cecb7a6eb53b4dd278aa9d4620c45b\"\n"
            "          },\n"
            "          \"effects\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/operations/3704435062607873/effects\"\n"
            "          },\n"
            "          \"succeeds\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=3704435062607873\"\n"
            "          },\n"
            "          \"precedes\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=3704435062607873\"\n"
            "          }\n"
            "        },\n"
            "        \"id\": \"3704435062607873\",\n"
            "        \"paging_token\": \"3704435062607873\",\n"
            "        \"source_account\": \"GBS43BF24ENNS3KPACUZVKK2VYPOZVBQO2CISGZ777RYGOPYC2FT6S3K\",\n"
            "        \"type\": \"create_account\",\n"
            "        \"type_i\": 0,\n"
            "        \"starting_balance\": \"10000.0\",\n"
            "        \"funder\": \"GBS43BF24ENNS3KPACUZVKK2VYPOZVBQO2CISGZ777RYGOPYC2FT6S3K\",\n"
            "        \"account\": \"GCYK67DDGBOANS6UODJ62QWGLEB2A7JQ3XUV25HCMLT7CI23PMMK3W6R\"\n"
            "      },\n"
            "      {\n"
            "        \"_links\": {\n"
            "          \"self\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/operations/3700453627924481\"\n"
            "          },\n"
            "          \"transaction\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/transactions/aae8986da322e9405fd27f0e284817cb2e86618151ac54ef8734f629d8cf9446\"\n"
            "          },\n"
            "          \"effects\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/operations/3700453627924481/effects\"\n"
            "          },\n"
            "          \"succeeds\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=3700453627924481\"\n"
            "          },\n"
            "          \"precedes\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=3700453627924481\"\n"
            "          }\n"
            "        },\n"
            "        \"id\": \"3700453627924481\",\n"
            "        \"paging_token\": \"3700453627924481\",\n"
            "        \"source_account\": \"GBS43BF24ENNS3KPACUZVKK2VYPOZVBQO2CISGZ777RYGOPYC2FT6S3K\",\n"
            "        \"type\": \"create_account\",\n"
            "        \"type_i\": 0,\n"
            "        \"starting_balance\": \"10000.0\",\n"
            "        \"funder\": \"GBS43BF24ENNS3KPACUZVKK2VYPOZVBQO2CISGZ777RYGOPYC2FT6S3K\",\n"
            "        \"account\": \"GA3J26UB775RW6M5INM75MJDPG72PMSUNZYBYU7IJL75UKMOIMJSHVEY\"\n"
            "      },\n"
            "      {\n"
            "        \"_links\": {\n"
            "          \"self\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/operations/3696369114025985\"\n"
            "          },\n"
            "          \"transaction\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/transactions/e852c069fbd0f3eafa691c276b0b57a4d0fad833e979fa192ad6b7a741892083\"\n"
            "          },\n"
            "          \"effects\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/operations/3696369114025985/effects\"\n"
            "          },\n"
            "          \"succeeds\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=3696369114025985\"\n"
            "          },\n"
            "          \"precedes\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=3696369114025985\"\n"
            "          }\n"
            "        },\n"
            "        \"id\": \"3696369114025985\",\n"
            "        \"paging_token\": \"3696369114025985\",\n"
            "        \"source_account\": \"GBS43BF24ENNS3KPACUZVKK2VYPOZVBQO2CISGZ777RYGOPYC2FT6S3K\",\n"
            "        \"type\": \"create_account\",\n"
            "        \"type_i\": 0,\n"
            "        \"starting_balance\": \"10000.0\",\n"
            "        \"funder\": \"GBS43BF24ENNS3KPACUZVKK2VYPOZVBQO2CISGZ777RYGOPYC2FT6S3K\",\n"
            "        \"account\": \"GDIEI4DXAV57TDMNJSYWU7WDNVVS4GYU65YFMP7KQRGOU4TQALFYZUIJ\"\n"
            "      },\n"
            "      {\n"
            "        \"_links\": {\n"
            "          \"self\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/operations/3695540185337857\"\n"
            "          },\n"
            "          \"transaction\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/transactions/0028e0d640a74b372c4195575c785da61605b6a7da95998cbb56553850e97963\"\n"
            "          },\n"
            "          \"effects\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/operations/3695540185337857/effects\"\n"
            "          },\n"
            "          \"succeeds\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=3695540185337857\"\n"
            "          },\n"
            "          \"precedes\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=3695540185337857\"\n"
            "          }\n"
            "        },\n"
            "        \"id\": \"3695540185337857\",\n"
            "        \"paging_token\": \"3695540185337857\",\n"
            "        \"source_account\": \"GBS43BF24ENNS3KPACUZVKK2VYPOZVBQO2CISGZ777RYGOPYC2FT6S3K\",\n"
            "        \"type\": \"create_account\",\n"
            "        \"type_i\": 0,\n"
            "        \"starting_balance\": \"10000.0\",\n"
            "        \"funder\": \"GBS43BF24ENNS3KPACUZVKK2VYPOZVBQO2CISGZ777RYGOPYC2FT6S3K\",\n"
            "        \"account\": \"GB3ZROOPBSDASLJUYCV7FJVGRWDJRB3MZAEY5CUZTRHLADEE5WW4AOIK\"\n"
            "      }\n"
            "    ]\n"
            "  }\n"
            "}";

private slots:
    void testDeserializeRevokeSponsorship() {
        QByteArray revokeSponsorshipJSON = "{\n"
            "  \"_links\": {\n"
            "    \"self\": {\n"
            "      \"href\": \"https://horizon-testnet.stellar.org/transactions/02a69bb0dc83d004ae918aab2d10c9dbc2cbf4451ab12927a691b87e5c6c5079/operations?cursor=\\u0026limit=10\\u0026order=asc\"\n"
            "    },\n"
            "    \"next\": {\n"
            "      \"href\": \"https://horizon-testnet.stellar.org/transactions/02a69bb0dc83d004ae918aab2d10c9dbc2cbf4451ab12927a691b87e5c6c5079/operations?cursor=4458463816060929\\u0026limit=10\\u0026order=asc\"\n"
            "    },\n"
            "    \"prev\": {\n"
            "      \"href\": \"https://horizon-testnet.stellar.org/transactions/02a69bb0dc83d004ae918aab2d10c9dbc2cbf4451ab12927a691b87e5c6c5079/operations?cursor=4458463816060929\\u0026limit=10\\u0026order=desc\"\n"
            "    }\n"
            "  },\n"
            "  \"_embedded\": {\n"
            "    \"records\": [\n"
            "      {\n"
            "        \"_links\": {\n"
            "          \"self\": {\n"
            "            \"href\": \"https://horizon-testnet.stellar.org/operations/4458463816060929\"\n"
            "          },\n"
            "          \"transaction\": {\n"
            "            \"href\": \"https://horizon-testnet.stellar.org/transactions/02a69bb0dc83d004ae918aab2d10c9dbc2cbf4451ab12927a691b87e5c6c5079\"\n"
            "          },\n"
            "          \"effects\": {\n"
            "            \"href\": \"https://horizon-testnet.stellar.org/operations/4458463816060929/effects\"\n"
            "          },\n"
            "          \"succeeds\": {\n"
            "            \"href\": \"https://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=4458463816060929\"\n"
            "          },\n"
            "          \"precedes\": {\n"
            "            \"href\": \"https://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=4458463816060929\"\n"
            "          }\n"
            "        },\n"
            "        \"id\": \"4458463816060929\",\n"
            "        \"paging_token\": \"4458463816060929\",\n"
            "        \"transaction_successful\": true,\n"
            "        \"source_account\": \"GB6QDNU47MYBR4NDTRP7M3FW27DAFOEADN5KDQI2DAVWW6YVKKG4QJS7\",\n"
            "        \"type\": \"revoke_sponsorship\",\n"
            "        \"type_i\": 18,\n"
            "        \"created_at\": \"2020-10-02T20:35:22Z\",\n"
            "        \"transaction_hash\": \"02a69bb0dc83d004ae918aab2d10c9dbc2cbf4451ab12927a691b87e5c6c5079\",\n"
            "        \"offer_id\": \"8822470\"\n"
            "      }\n"
            "    ]\n"
            "  }\n"
            "}";

        OperationPage operationsPage(0);
        operationsPage.loadFromJson(revokeSponsorshipJSON);

        RevokeSponsorshipOperationResponse* revokeOp = (RevokeSponsorshipOperationResponse*) operationsPage.at(0);


        QVERIFY(revokeOp->getAccountID().isNull());
        QVERIFY(revokeOp->getClaimableBalanceID().isNull());
        QVERIFY(revokeOp->getDataAccountID().isNull());
        QVERIFY(revokeOp->getDataName().isNull());
        QVERIFY(revokeOp->getSignerAccountID().isNull());
        QVERIFY(revokeOp->getSignerKey().isNull());
        QVERIFY(revokeOp->getTrustlineAccountID().isNull());
        QVERIFY(revokeOp->getTrustlineAsset().isNull());
        QCOMPARE(revokeOp->getOfferID(), "8822470");
        QCOMPARE(revokeOp->getSourceAccount().getAccountId(), "GB6QDNU47MYBR4NDTRP7M3FW27DAFOEADN5KDQI2DAVWW6YVKKG4QJS7");
    }

    void testDeserializeLiquidityPoolDepositOperation() {
        QByteArray liquidityPoolDepositJSON = "{"
            "  \"_links\": {"
            "    \"self\": { \"href\": \"https://horizon-testnet.stellar.org/operations?cursor=\\u0026limit=10\\u0026order=asc\" },"
            "    \"next\": { \"href\": \"https://horizon-testnet.stellar.org/operations?cursor=124042211741474817\\u0026limit=10\\u0026order=asc\" },"
            "    \"prev\": { \"href\": \"https://horizon-testnet.stellar.org/operations?cursor=124042211741474817\\u0026limit=10\\u0026order=desc\" }"
            "  },"
            "  \"_embedded\": {"
            "    \"records\": ["
            "      {"
            "        \"_links\": {"
            "          \"self\": { \"href\": \"https://horizon-testnet.stellar.org/operations/124042211741474817\" },"
            "          \"transaction\": { \"href\": \"https://horizon-testnet.stellar.org/transactions/abc\" },"
            "          \"effects\": { \"href\": \"https://horizon-testnet.stellar.org/operations/124042211741474817/effects\" },"
            "          \"succeeds\": { \"href\": \"https://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=124042211741474817\" },"
            "          \"precedes\": { \"href\": \"https://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=124042211741474817\" }"
            "        },"
            "        \"id\": \"124042211741474817\","
            "        \"paging_token\": \"124042211741474817\","
            "        \"transaction_successful\": true,"
            "        \"source_account\": \"GBS43BF24ENNS3KPACUZVKK2VYPOZVBQO2CISGZ777RYGOPYC2FT6S3K\","
            "        \"type\": \"liquidity_pool_deposit\","
            "        \"type_i\": 22,"
            "        \"created_at\": \"2021-11-18T03:25:38Z\","
            "        \"transaction_hash\": \"f0d2b8e8b3a3f1c1b5d6e7a8c9d0e1f2031425364758697a8b9c0d1e2f304150\","
            "        \"liquidity_pool_id\": \"a468d41d8e9b8f3c7209d816eb46be1bff71c907dbf4540e6493f7d3d6e3b8d6\","
            "        \"reserves_max\": ["
            "          { \"asset\": \"native\", \"amount\": \"1000.0000000\" },"
            "          { \"asset\": \"USDC:GA5ZSEJYB37JRC5AVCIA5MOP4RHTM335X2KGX3IHOJAPP5RE34K4KZVN\", \"amount\": \"2000.0000000\" }"
            "        ],"
            "        \"min_price\": \"0.4000000\","
            "        \"min_price_r\": { \"n\": 2, \"d\": 5 },"
            "        \"max_price\": \"0.6000000\","
            "        \"max_price_r\": { \"n\": 3, \"d\": 5 },"
            "        \"reserves_deposited\": ["
            "          { \"asset\": \"native\", \"amount\": \"983.0000000\" },"
            "          { \"asset\": \"USDC:GA5ZSEJYB37JRC5AVCIA5MOP4RHTM335X2KGX3IHOJAPP5RE34K4KZVN\", \"amount\": \"2000.0000000\" }"
            "        ],"
            "        \"shares_received\": \"1402.8005982\""
            "      }"
            "    ]"
            "  }"
            "}";

        OperationPage operationsPage(0);
        operationsPage.loadFromJson(liquidityPoolDepositJSON);

        LiquidityPoolDepositOperationResponse* op =
            (LiquidityPoolDepositOperationResponse*) operationsPage.at(0);

        QVERIFY(op != nullptr);
        QCOMPARE(op->getType(), QString("liquidity_pool_deposit"));
        QCOMPARE(op->sourceAccount(), QString("GBS43BF24ENNS3KPACUZVKK2VYPOZVBQO2CISGZ777RYGOPYC2FT6S3K"));
        QCOMPARE(op->getLiquidityPoolId(), QString("a468d41d8e9b8f3c7209d816eb46be1bff71c907dbf4540e6493f7d3d6e3b8d6"));
        QCOMPARE(op->getMinPrice(), QString("0.4000000"));
        QCOMPARE(op->getMaxPrice(), QString("0.6000000"));
        QCOMPARE(op->getSharesReceived(), QString("1402.8005982"));
        QCOMPARE(op->getReservesMax().size(), 2);
        QCOMPARE(op->getReservesMax().at(0).getAsset(), QString("native"));
        QCOMPARE(op->getReservesMax().at(0).getAmount(), QString("1000.0000000"));
        QCOMPARE(op->getReservesDeposited().size(), 2);
        QCOMPARE(op->getReservesDeposited().at(0).getAmount(), QString("983.0000000"));
        QCOMPARE(op->getReservesDeposited().at(1).getAsset(), QString("USDC:GA5ZSEJYB37JRC5AVCIA5MOP4RHTM335X2KGX3IHOJAPP5RE34K4KZVN"));
    }

    void testDeserializeLiquidityPoolWithdrawOperation() {
        QByteArray liquidityPoolWithdrawJSON = "{"
            "  \"_links\": {"
            "    \"self\": { \"href\": \"https://horizon-testnet.stellar.org/operations?cursor=\\u0026limit=10\\u0026order=asc\" },"
            "    \"next\": { \"href\": \"https://horizon-testnet.stellar.org/operations?cursor=124042220331409409\\u0026limit=10\\u0026order=asc\" },"
            "    \"prev\": { \"href\": \"https://horizon-testnet.stellar.org/operations?cursor=124042220331409409\\u0026limit=10\\u0026order=desc\" }"
            "  },"
            "  \"_embedded\": {"
            "    \"records\": ["
            "      {"
            "        \"_links\": {"
            "          \"self\": { \"href\": \"https://horizon-testnet.stellar.org/operations/124042220331409409\" },"
            "          \"transaction\": { \"href\": \"https://horizon-testnet.stellar.org/transactions/def\" },"
            "          \"effects\": { \"href\": \"https://horizon-testnet.stellar.org/operations/124042220331409409/effects\" },"
            "          \"succeeds\": { \"href\": \"https://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=124042220331409409\" },"
            "          \"precedes\": { \"href\": \"https://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=124042220331409409\" }"
            "        },"
            "        \"id\": \"124042220331409409\","
            "        \"paging_token\": \"124042220331409409\","
            "        \"transaction_successful\": true,"
            "        \"source_account\": \"GBS43BF24ENNS3KPACUZVKK2VYPOZVBQO2CISGZ777RYGOPYC2FT6S3K\","
            "        \"type\": \"liquidity_pool_withdraw\","
            "        \"type_i\": 23,"
            "        \"created_at\": \"2021-11-18T03:27:10Z\","
            "        \"transaction_hash\": \"1122334455667788990011223344556677889900112233445566778899001122\","
            "        \"liquidity_pool_id\": \"a468d41d8e9b8f3c7209d816eb46be1bff71c907dbf4540e6493f7d3d6e3b8d6\","
            "        \"reserves_min\": ["
            "          { \"asset\": \"native\", \"amount\": \"100.0000000\" },"
            "          { \"asset\": \"USDC:GA5ZSEJYB37JRC5AVCIA5MOP4RHTM335X2KGX3IHOJAPP5RE34K4KZVN\", \"amount\": \"200.0000000\" }"
            "        ],"
            "        \"shares\": \"500.0000000\","
            "        \"reserves_received\": ["
            "          { \"asset\": \"native\", \"amount\": \"105.5000000\" },"
            "          { \"asset\": \"USDC:GA5ZSEJYB37JRC5AVCIA5MOP4RHTM335X2KGX3IHOJAPP5RE34K4KZVN\", \"amount\": \"210.7000000\" }"
            "        ]"
            "      }"
            "    ]"
            "  }"
            "}";

        OperationPage operationsPage(0);
        operationsPage.loadFromJson(liquidityPoolWithdrawJSON);

        LiquidityPoolWithdrawOperationResponse* op =
            (LiquidityPoolWithdrawOperationResponse*) operationsPage.at(0);

        QVERIFY(op != nullptr);
        QCOMPARE(op->getType(), QString("liquidity_pool_withdraw"));
        QCOMPARE(op->sourceAccount(), QString("GBS43BF24ENNS3KPACUZVKK2VYPOZVBQO2CISGZ777RYGOPYC2FT6S3K"));
        QCOMPARE(op->getLiquidityPoolId(), QString("a468d41d8e9b8f3c7209d816eb46be1bff71c907dbf4540e6493f7d3d6e3b8d6"));
        QCOMPARE(op->getShares(), QString("500.0000000"));
        QCOMPARE(op->getReservesMin().size(), 2);
        QCOMPARE(op->getReservesMin().at(1).getAsset(), QString("USDC:GA5ZSEJYB37JRC5AVCIA5MOP4RHTM335X2KGX3IHOJAPP5RE34K4KZVN"));
        QCOMPARE(op->getReservesMin().at(1).getAmount(), QString("200.0000000"));
        QCOMPARE(op->getReservesReceived().size(), 2);
        QCOMPARE(op->getReservesReceived().at(0).getAsset(), QString("native"));
        QCOMPARE(op->getReservesReceived().at(0).getAmount(), QString("105.5000000"));
    }
};





ADD_TEST(OperationsPageTest)
#endif // OPERATIONSPAGETEST_H

