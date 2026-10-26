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
#include "../../src/responses/operations/createclaimablebalanceoperationresponse.h"
#include "../../src/responses/operations/beginsponsoringfuturereservesoperationresponse.h"
#include "../../src/responses/operations/endsponsoringfuturereservesoperationresponse.h"
#include "../../src/responses/operations/setoptionsoperationresponse.h"
#include "../../src/responses/genericoperation.h"
#include "../../src/server.h"
#include "../fakeserver.h"
#include "../../src/predicate.h"
#include "../../src/claimant.h"

#include "../../src/keypair.h"
#include "../../src/assettypenative.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <limits>

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
            "        \"transaction_hash\": \"dd9d10c80a344f4464df3ecaa63705a5ef4a0533ff2f2099d5ef371ab5e1c046\",\n"
            "        \"starting_balance\": \"10000.0\",\n"
            "        \"funder\": \"GBS43BF24ENNS3KPACUZVKK2VYPOZVBQO2CISGZ777RYGOPYC2FT6S3K\",\n"
            "        \"account\": \"GDFH4NIYMIIAKRVEJJZOIGWKXGQUF3XHJG6ZM6CEA64AMTVDN44LHOQE\",\n"
            "        \"transaction_successful\": true\n"
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
            "        \"account\": \"GA4GQOVE7SQCPGDCXOKIUWGZYJCMMA3TCJUB54ZYYCNMZD7MVILGEADL\",\n"
            "        \"created_at\": \"2026-09-28T15:16:12Z\",\n"
            "        \"transaction_hash\": \"96989f07e61c4b262b142236c70157acbf764c9af4a470348d435c96e9530ae1\",\n"
            "        \"transaction_successful\": true\n"
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
            "        \"account\": \"GDD2PUGSEGWN6TSHQNE6EDHVUYH6I37Y7727V3WFXUS2GGO4ZJ72EPEZ\",\n"
            "        \"created_at\": \"2026-09-28T15:16:12Z\",\n"
            "        \"transaction_hash\": \"d4c7f58a5d69d7d82712d56faa6830077696799a6a5c7036bd4bcb49977e1809\",\n"
            "        \"transaction_successful\": true\n"
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
            "        \"account\": \"GDPMWVUTK7T5NG2OHDAZLGLT7QS5GCL23CFVNUR3BNKVPDLW2RULWE7Z\",\n"
            "        \"created_at\": \"2026-09-28T15:16:12Z\",\n"
            "        \"transaction_hash\": \"87f3ef5d7e9ad11a03212bbd9bfc66d6431197d5492af4af6c94d464c70979bb\",\n"
            "        \"transaction_successful\": true\n"
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
            "        \"amount\": \"10.123\",\n"
            "        \"created_at\": \"2026-09-28T15:16:12Z\",\n"
            "        \"transaction_hash\": \"9f2e106c44bcb5d3cc27746e892d3edcadcead37ce863b274fe698e47865a91a\",\n"
            "        \"transaction_successful\": true\n"
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
            "        \"amount\": \"10.123\",\n"
            "        \"created_at\": \"2026-09-28T15:16:12Z\",\n"
            "        \"transaction_hash\": \"450949cf4f7f7833ebd464791623e45b59fa2591b59f735385a1486534129ff6\",\n"
            "        \"transaction_successful\": true\n"
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
            "        \"account\": \"GCYK67DDGBOANS6UODJ62QWGLEB2A7JQ3XUV25HCMLT7CI23PMMK3W6R\",\n"
            "        \"created_at\": \"2026-09-28T15:16:12Z\",\n"
            "        \"transaction_hash\": \"e5de54d06e17d3407c1772427614e691d1134327bbfc2efb09f0fc938c3e2f07\",\n"
            "        \"transaction_successful\": true\n"
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
            "        \"account\": \"GA3J26UB775RW6M5INM75MJDPG72PMSUNZYBYU7IJL75UKMOIMJSHVEY\",\n"
            "        \"created_at\": \"2026-09-28T15:16:12Z\",\n"
            "        \"transaction_hash\": \"546de2a16a9235834f19adf4bd26dafc5e891c73709611684898683dccb795e0\",\n"
            "        \"transaction_successful\": true\n"
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
            "        \"account\": \"GDIEI4DXAV57TDMNJSYWU7WDNVVS4GYU65YFMP7KQRGOU4TQALFYZUIJ\",\n"
            "        \"created_at\": \"2026-09-28T15:16:12Z\",\n"
            "        \"transaction_hash\": \"b638a5204c3e72f5a103f3f575d477d663fe27e0bf4609dc6d69f7e0ba3424b4\",\n"
            "        \"transaction_successful\": true\n"
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
            "        \"account\": \"GB3ZROOPBSDASLJUYCV7FJVGRWDJRB3MZAEY5CUZTRHLADEE5WW4AOIK\",\n"
            "        \"created_at\": \"2026-09-28T15:16:12Z\",\n"
            "        \"transaction_hash\": \"0ec266e46d6931adf9f9c2b0eab2409febbfe9793ede49d260aa6c1e46b779f4\",\n"
            "        \"transaction_successful\": true\n"
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
        QCOMPARE(op->getMinPriceR().getNumerator(), 2);
        QCOMPARE(op->getMinPriceR().getDenominator(), 5);
        QCOMPARE(op->getMaxPriceR().getNumerator(), 3);
        QCOMPARE(op->getMaxPriceR().getDenominator(), 5);
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

    void testDeserializeCreateClaimableBalanceOperation() {
        QByteArray createClaimableBalanceJSON = "{"
            "  \"_embedded\": {"
            "    \"records\": ["
            "      {\"_links\":{\"self\":{\"href\":\"https://horizon.stellar.org/operations/124042211741474817\"},\"transaction\":{\"href\":\"https://horizon.stellar.org/transactions/f0d2b8e8b3a3f1c1b5d6e7a8c9d0e1f2031425364758697a8b9c0d1e2f304150\"},\"effects\":{\"href\":\"https://horizon.stellar.org/operations/124042211741474817/effects\"},\"succeeds\":{\"href\":\"https://horizon.stellar.org/effects?order=desc&cursor=124042211741474817\"},\"precedes\":{\"href\":\"https://horizon.stellar.org/effects?order=asc&cursor=124042211741474817\"}},"
            "        \"id\": \"124042211741474817\","
            "        \"paging_token\": \"124042211741474817\","
            "        \"transaction_successful\": true,"
            "        \"source_account\": \"GBS43BF24ENNS3KPACUZVKK2VYPOZVBQO2CISGZ777RYGOPYC2FT6S3K\","
            "        \"type\": \"create_claimable_balance\","
            "        \"type_i\": 14,"
            "        \"created_at\": \"2020-10-02T20:35:22Z\","
            "        \"transaction_hash\": \"f0d2b8e8b3a3f1c1b5d6e7a8c9d0e1f2031425364758697a8b9c0d1e2f304150\","
            "        \"asset\": \"USDC:GA5ZSEJYB37JRC5AVCIA5MOP4RHTM335X2KGX3IHOJAPP5RE34K4KZVN\","
            "        \"amount\": \"100.0000000\","
            "        \"claimants\": ["
            "          {"
            "            \"destination\": \"GB56OJGSA6VHEUFZDX6AL2YDVG2TS5JDZYQJHDYHBDH7PCD5NIQKLSDO\","
            "            \"predicate\": {"
            "              \"or\": ["
            "                { \"abs_before\": \"2020-09-28T17:57:04Z\" },"
            "                { \"rel_before\": \"12\" }"
            "              ]"
            "            }"
            "          }"
            "        ]"
            "      }"
            "    ]"
            "  }"
            "}";

        OperationPage operationsPage(0);
        operationsPage.loadFromJson(createClaimableBalanceJSON);

        CreateClaimableBalanceOperationResponse* op =
            (CreateClaimableBalanceOperationResponse*) operationsPage.at(0);

        QVERIFY(op != nullptr);
        QCOMPARE(op->getType(), QString("create_claimable_balance"));
        QCOMPARE(op->sourceAccount(), QString("GBS43BF24ENNS3KPACUZVKK2VYPOZVBQO2CISGZ777RYGOPYC2FT6S3K"));
        QCOMPARE(op->asset(), QString("USDC:GA5ZSEJYB37JRC5AVCIA5MOP4RHTM335X2KGX3IHOJAPP5RE34K4KZVN"));
        QCOMPARE(op->getAmount(), QString("100.0000000"));
        QList<Claimant> claimants = op->getClaimants();
        QCOMPARE(claimants.size(), 1);
        QCOMPARE(claimants.at(0).getDestination(), QString("GB56OJGSA6VHEUFZDX6AL2YDVG2TS5JDZYQJHDYHBDH7PCD5NIQKLSDO"));
        Predicate::Or& por = (Predicate::Or&) claimants.at(0).getPredicate();
        QCOMPARE(por.getInner().size(), 2);
        Predicate::AbsBefore& absBefore = (Predicate::AbsBefore&) por.get(0);
        Predicate::RelBefore& relBefore = (Predicate::RelBefore&) por.get(1);
        QCOMPARE(absBefore.getDate().toString(Qt::ISODate), QString("2020-09-28T17:57:04Z"));
        QCOMPARE(relBefore.getSecondsSinceClose(), 12);
    }

    void testDeserializeBeginSponsoringFutureReservesOperation() {
        QByteArray beginSponsoringJSON = "{"
            "  \"_embedded\": { \"records\": [ {\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/4458463816060929\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/02a69bb0dc83d004ae918aab2d10c9dbc2cbf4451ab12927a691b87e5c6c5079\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/4458463816060929/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=4458463816060929\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=4458463816060929\"}},"
            "    \"id\": \"4458463816060929\","
            "    \"paging_token\": \"4458463816060929\","
            "    \"transaction_successful\": true,"
            "    \"source_account\": \"GB6QDNU47MYBR4NDTRP7M3FW27DAFOEADN5KDQI2DAVWW6YVKKG4QJS7\","
            "    \"type\": \"begin_sponsoring_future_reserves\","
            "    \"type_i\": 16,"
            "    \"created_at\": \"2020-10-02T20:35:22Z\","
            "    \"transaction_hash\": \"02a69bb0dc83d004ae918aab2d10c9dbc2cbf4451ab12927a691b87e5c6c5079\","
            "    \"sponsored_id\": \"GB56OJGSA6VHEUFZDX6AL2YDVG2TS5JDZYQJHDYHBDH7PCD5NIQKLSDO\""
            "  } ] }"
            "}";

        OperationPage operationsPage(0);
        operationsPage.loadFromJson(beginSponsoringJSON);

        BeginSponsoringFutureReservesOperationResponse* op =
            (BeginSponsoringFutureReservesOperationResponse*) operationsPage.at(0);

        QVERIFY(op != nullptr);
        QCOMPARE(op->getType(), QString("begin_sponsoring_future_reserves"));
        QCOMPARE(op->sourceAccount(), QString("GB6QDNU47MYBR4NDTRP7M3FW27DAFOEADN5KDQI2DAVWW6YVKKG4QJS7"));
        QCOMPARE(op->getSponsoredID(), QString("GB56OJGSA6VHEUFZDX6AL2YDVG2TS5JDZYQJHDYHBDH7PCD5NIQKLSDO"));
    }

    void testDeserializeEndSponsoringFutureReservesOperation() {
        QByteArray endSponsoringJSON = "{"
            "  \"_embedded\": { \"records\": [ {\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/4458463816060930\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/02a69bb0dc83d004ae918aab2d10c9dbc2cbf4451ab12927a691b87e5c6c5079\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/4458463816060930/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=4458463816060930\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=4458463816060930\"}},"
            "    \"id\": \"4458463816060930\","
            "    \"paging_token\": \"4458463816060930\","
            "    \"transaction_successful\": true,"
            "    \"source_account\": \"GB56OJGSA6VHEUFZDX6AL2YDVG2TS5JDZYQJHDYHBDH7PCD5NIQKLSDO\","
            "    \"type\": \"end_sponsoring_future_reserves\","
            "    \"type_i\": 17,"
            "    \"created_at\": \"2020-10-02T20:35:25Z\","
            "    \"transaction_hash\": \"02a69bb0dc83d004ae918aab2d10c9dbc2cbf4451ab12927a691b87e5c6c5079\","
            "    \"begin_sponsor\": \"GB6QDNU47MYBR4NDTRP7M3FW27DAFOEADN5KDQI2DAVWW6YVKKG4QJS7\""
            "  } ] }"
            "}";

        OperationPage operationsPage(0);
        operationsPage.loadFromJson(endSponsoringJSON);

        EndSponsoringFutureReservesOperationResponse* op =
            (EndSponsoringFutureReservesOperationResponse*) operationsPage.at(0);

        QVERIFY(op != nullptr);
        QCOMPARE(op->getType(), QString("end_sponsoring_future_reserves"));
        QCOMPARE(op->sourceAccount(), QString("GB56OJGSA6VHEUFZDX6AL2YDVG2TS5JDZYQJHDYHBDH7PCD5NIQKLSDO"));
        QCOMPARE(op->getBeginSponsor(), QString("GB6QDNU47MYBR4NDTRP7M3FW27DAFOEADN5KDQI2DAVWW6YVKKG4QJS7"));
    }

    void testDeserializeRevokeAccountSponsorshipOperation() {
        QByteArray revokeAccountJSON = "{"
            "  \"_embedded\": { \"records\": [ {\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/4458463816060931\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/02a69bb0dc83d004ae918aab2d10c9dbc2cbf4451ab12927a691b87e5c6c5079\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/4458463816060931/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=4458463816060931\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=4458463816060931\"}},"
            "    \"id\": \"4458463816060931\","
            "    \"paging_token\": \"4458463816060931\","
            "    \"transaction_successful\": true,"
            "    \"source_account\": \"GB6QDNU47MYBR4NDTRP7M3FW27DAFOEADN5KDQI2DAVWW6YVKKG4QJS7\","
            "    \"type\": \"revoke_sponsorship\","
            "    \"type_i\": 18,"
            "    \"created_at\": \"2020-10-02T20:35:22Z\","
            "    \"transaction_hash\": \"02a69bb0dc83d004ae918aab2d10c9dbc2cbf4451ab12927a691b87e5c6c5079\","
            "    \"account_id\": \"GB56OJGSA6VHEUFZDX6AL2YDVG2TS5JDZYQJHDYHBDH7PCD5NIQKLSDO\""
            "  } ] }"
            "}";

        OperationPage operationsPage(0);
        operationsPage.loadFromJson(revokeAccountJSON);

        RevokeSponsorshipOperationResponse* op =
            (RevokeSponsorshipOperationResponse*) operationsPage.at(0);

        QVERIFY(op != nullptr);
        QCOMPARE(op->getType(), QString("revoke_sponsorship"));
        QCOMPARE(op->getAccountID(), QString("GB56OJGSA6VHEUFZDX6AL2YDVG2TS5JDZYQJHDYHBDH7PCD5NIQKLSDO"));
        QVERIFY(op->getClaimableBalanceID().isNull());
        QVERIFY(op->getDataAccountID().isNull());
        QVERIFY(op->getDataName().isNull());
        QVERIFY(op->getOfferID().isNull());
        QVERIFY(op->getSignerAccountID().isNull());
        QVERIFY(op->getSignerKey().isNull());
        QVERIFY(op->getTrustlineAccountID().isNull());
        QVERIFY(op->getTrustlineAsset().isNull());
    }

    void testDeserializeRevokeDataSponsorshipOperation() {
        QByteArray revokeDataJSON = "{"
            "  \"_embedded\": { \"records\": [ {\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/4458463816060932\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/02a69bb0dc83d004ae918aab2d10c9dbc2cbf4451ab12927a691b87e5c6c5079\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/4458463816060932/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=4458463816060932\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=4458463816060932\"}},"
            "    \"id\": \"4458463816060932\","
            "    \"paging_token\": \"4458463816060932\","
            "    \"transaction_successful\": true,"
            "    \"source_account\": \"GB6QDNU47MYBR4NDTRP7M3FW27DAFOEADN5KDQI2DAVWW6YVKKG4QJS7\","
            "    \"type\": \"revoke_sponsorship\","
            "    \"type_i\": 18,"
            "    \"created_at\": \"2020-10-02T20:35:22Z\","
            "    \"transaction_hash\": \"02a69bb0dc83d004ae918aab2d10c9dbc2cbf4451ab12927a691b87e5c6c5079\","
            "    \"data_account_id\": \"GB56OJGSA6VHEUFZDX6AL2YDVG2TS5JDZYQJHDYHBDH7PCD5NIQKLSDO\","
            "    \"data_name\": \"config.setting\""
            "  } ] }"
            "}";

        OperationPage operationsPage(0);
        operationsPage.loadFromJson(revokeDataJSON);

        RevokeSponsorshipOperationResponse* op =
            (RevokeSponsorshipOperationResponse*) operationsPage.at(0);

        QVERIFY(op != nullptr);
        QCOMPARE(op->getType(), QString("revoke_sponsorship"));
        QCOMPARE(op->getDataAccountID(), QString("GB56OJGSA6VHEUFZDX6AL2YDVG2TS5JDZYQJHDYHBDH7PCD5NIQKLSDO"));
        QCOMPARE(op->getDataName(), QString("config.setting"));
        QVERIFY(op->getAccountID().isNull());
        QVERIFY(op->getClaimableBalanceID().isNull());
        QVERIFY(op->getOfferID().isNull());
        QVERIFY(op->getSignerAccountID().isNull());
        QVERIFY(op->getSignerKey().isNull());
        QVERIFY(op->getTrustlineAccountID().isNull());
        QVERIFY(op->getTrustlineAsset().isNull());
    }

    void testDeserializeRevokeSignerSponsorshipOperation() {
        QByteArray revokeSignerJSON = "{"
            "  \"_embedded\": { \"records\": [ {\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/4458463816060933\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/02a69bb0dc83d004ae918aab2d10c9dbc2cbf4451ab12927a691b87e5c6c5079\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/4458463816060933/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=4458463816060933\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=4458463816060933\"}},"
            "    \"id\": \"4458463816060933\","
            "    \"paging_token\": \"4458463816060933\","
            "    \"transaction_successful\": true,"
            "    \"source_account\": \"GB6QDNU47MYBR4NDTRP7M3FW27DAFOEADN5KDQI2DAVWW6YVKKG4QJS7\","
            "    \"type\": \"revoke_sponsorship\","
            "    \"type_i\": 18,"
            "    \"created_at\": \"2020-10-02T20:35:22Z\","
            "    \"transaction_hash\": \"02a69bb0dc83d004ae918aab2d10c9dbc2cbf4451ab12927a691b87e5c6c5079\","
            "    \"signer_account_id\": \"GB56OJGSA6VHEUFZDX6AL2YDVG2TS5JDZYQJHDYHBDH7PCD5NIQKLSDO\","
            "    \"signer_key\": \"GA5ZSEJYB37JRC5AVCIA5MOP4RHTM335X2KGX3IHOJAPP5RE34K4KZVN\""
            "  } ] }"
            "}";

        OperationPage operationsPage(0);
        operationsPage.loadFromJson(revokeSignerJSON);

        RevokeSponsorshipOperationResponse* op =
            (RevokeSponsorshipOperationResponse*) operationsPage.at(0);

        QVERIFY(op != nullptr);
        QCOMPARE(op->getType(), QString("revoke_sponsorship"));
        QCOMPARE(op->getSignerAccountID(), QString("GB56OJGSA6VHEUFZDX6AL2YDVG2TS5JDZYQJHDYHBDH7PCD5NIQKLSDO"));
        QCOMPARE(op->getSignerKey(), QString("GA5ZSEJYB37JRC5AVCIA5MOP4RHTM335X2KGX3IHOJAPP5RE34K4KZVN"));
        QVERIFY(op->getAccountID().isNull());
        QVERIFY(op->getClaimableBalanceID().isNull());
        QVERIFY(op->getDataAccountID().isNull());
        QVERIFY(op->getDataName().isNull());
        QVERIFY(op->getOfferID().isNull());
        QVERIFY(op->getTrustlineAccountID().isNull());
        QVERIFY(op->getTrustlineAsset().isNull());
    }

    void testDeserializeRevokeTrustlineSponsorshipOperation() {
        QByteArray revokeTrustlineJSON = "{"
            "  \"_embedded\": { \"records\": [ {\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/4458463816060934\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/02a69bb0dc83d004ae918aab2d10c9dbc2cbf4451ab12927a691b87e5c6c5079\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/4458463816060934/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=4458463816060934\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=4458463816060934\"}},"
            "    \"id\": \"4458463816060934\","
            "    \"paging_token\": \"4458463816060934\","
            "    \"transaction_successful\": true,"
            "    \"source_account\": \"GB6QDNU47MYBR4NDTRP7M3FW27DAFOEADN5KDQI2DAVWW6YVKKG4QJS7\","
            "    \"type\": \"revoke_sponsorship\","
            "    \"type_i\": 18,"
            "    \"created_at\": \"2020-10-02T20:35:22Z\","
            "    \"transaction_hash\": \"02a69bb0dc83d004ae918aab2d10c9dbc2cbf4451ab12927a691b87e5c6c5079\","
            "    \"trustline_account_id\": \"GB56OJGSA6VHEUFZDX6AL2YDVG2TS5JDZYQJHDYHBDH7PCD5NIQKLSDO\","
            "    \"trustline_asset\": \"USDC:GA5ZSEJYB37JRC5AVCIA5MOP4RHTM335X2KGX3IHOJAPP5RE34K4KZVN\""
            "  } ] }"
            "}";

        OperationPage operationsPage(0);
        operationsPage.loadFromJson(revokeTrustlineJSON);

        RevokeSponsorshipOperationResponse* op =
            (RevokeSponsorshipOperationResponse*) operationsPage.at(0);

        QVERIFY(op != nullptr);
        QCOMPARE(op->getType(), QString("revoke_sponsorship"));
        QCOMPARE(op->getTrustlineAccountID(), QString("GB56OJGSA6VHEUFZDX6AL2YDVG2TS5JDZYQJHDYHBDH7PCD5NIQKLSDO"));
        QCOMPARE(op->getTrustlineAsset(), QString("USDC:GA5ZSEJYB37JRC5AVCIA5MOP4RHTM335X2KGX3IHOJAPP5RE34K4KZVN"));
        QVERIFY(op->getAccountID().isNull());
        QVERIFY(op->getClaimableBalanceID().isNull());
        QVERIFY(op->getDataAccountID().isNull());
        QVERIFY(op->getDataName().isNull());
        QVERIFY(op->getOfferID().isNull());
        QVERIFY(op->getSignerAccountID().isNull());
        QVERIFY(op->getSignerKey().isNull());
    }

    void testDeserializePaymentWithMuxedAccounts() {
        // CAP-27 muxed source/from/to. muxed_id = 2^64-1 must survive as a string
        // (a qint64 would overflow and corrupt the account id).
        QByteArray json = "{"
            "  \"_embedded\": { \"records\": [ {\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/3704821609664513\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/9f2e106c44bcb5d3cc27746e892d3edcadcead37ce863b274fe698e47865a91a\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/3704821609664513/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=3704821609664513\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=3704821609664513\"}},\"created_at\":\"2026-09-28T15:16:12Z\",\"transaction_hash\":\"9f2e106c44bcb5d3cc27746e892d3edcadcead37ce863b274fe698e47865a91a\","
            "    \"id\": \"3704821609664513\","
            "    \"paging_token\": \"3704821609664513\","
            "    \"transaction_successful\": true,"
            "    \"source_account\": \"GCYK67DDGBOANS6UODJ62QWGLEB2A7JQ3XUV25HCMLT7CI23PMMK3W6R\","
            "    \"source_account_muxed\": \"MA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVAAAAAAAAAAAAAJLK\","
            "    \"source_account_muxed_id\": \"18446744073709551615\","
            "    \"type\": \"payment\","
            "    \"type_i\": 1,"
            "    \"asset_type\": \"native\","
            "    \"from\": \"GCYK67DDGBOANS6UODJ62QWGLEB2A7JQ3XUV25HCMLT7CI23PMMK3W6R\","
            "    \"from_muxed\": \"MA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVAAAAAAAAAAAAAJLK\","
            "    \"from_muxed_id\": \"18446744073709551615\","
            "    \"to\": \"GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H\","
            "    \"to_muxed\": \"MBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2HAAAAAAAAAAAAAPNT2\","
            "    \"to_muxed_id\": \"420\","
            "    \"amount\": \"10.123\""
            "  } ] }"
            "}";

        OperationPage operationsPage(0);
        operationsPage.loadFromJson(json);
        PaymentOperationResponse* op = (PaymentOperationResponse*) operationsPage.at(0);

        QVERIFY(op != nullptr);
        QCOMPARE(op->getType(), QString("payment"));
        // base OperationResponse muxed source (inherited by every operation type)
        QCOMPARE(op->getSourceAccountMuxed(), QString("MA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVAAAAAAAAAAAAAJLK"));
        QCOMPARE(op->getSourceAccountMuxedId(), QString("18446744073709551615"));
        // payment from/to muxed
        QCOMPARE(op->getFromMuxed(), QString("MA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVAAAAAAAAAAAAAJLK"));
        QCOMPARE(op->getFromMuxedId(), QString("18446744073709551615"));
        QCOMPARE(op->getToMuxed(), QString("MBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2HAAAAAAAAAAAAAPNT2"));
        QCOMPARE(op->getToMuxedId(), QString("420"));
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
    /** A page holding `records`, with Horizon's links. */
    static QByteArray pageOf(const QJsonArray& records)
    {
        const QJsonObject self{{"href", "https://horizon.example/operations?cursor=&limit=2&order=asc"}};
        const QJsonObject next{{"href", "https://horizon.example/operations?cursor=2&limit=2&order=asc"}};
        return QJsonDocument(QJsonObject{
            {"_links", QJsonObject{{"self", self}, {"next", next}, {"prev", self}}},
            {"_embedded", QJsonObject{{"records", records}}}}).toJson();
    }
    static QJsonObject firstRealOperation(const char* type)
    {
        const QJsonArray all = QJsonDocument::fromJson(realOperationsOfEveryType()).object()
                .value("_embedded").toObject().value("records").toArray();
        for (const QJsonValue& op : all)
            if (op.toObject().value("type").toString() == QLatin1String(type))
                return op.toObject();
        return QJsonObject();
    }

    /** What an OperationPage loading `json` with `member` set to `value` throws; empty if it loads. */
    static QString pageRefusal(const QByteArray& json, const QString& member, const QJsonValue& value)
    {
        const QJsonValue changed = edited(QJsonDocument::fromJson(json).object(),
                                          member.split(QLatin1Char('.')), value);
        OperationPage page(nullptr);
        try {
            page.loadFromJson(json.startsWith('[') ? json : QJsonDocument(changed.toObject()).toJson());
        } catch (const std::exception& e) {
            return QString::fromUtf8(e.what());
        }
        return QString();
    }

private slots:
    /**
     * A page is read whole or not at all. One whose second record could not
     * be read kept the first, and one without `_embedded.records`, or with a
     * record that is not an object, read as an empty page, or as a record
     * with every member missing.
     */
    void testAPageIsReadWholeOrNotAtAll()
    {
        const QJsonObject payment = firstRealOperation("payment");
        QJsonObject broken = payment;
        broken["amount"] = 5;
        OperationPage page(nullptr);
        QVERIFY_EXCEPTION_THROWN(page.loadFromJson(pageOf(QJsonArray{payment, broken})), std::runtime_error);
        QCOMPARE(page.size(), 0);
        QVERIFY(pageRefusal(pageOf(QJsonArray{payment, broken}), "x", QJsonValue())
                    .startsWith("_embedded.records[1].amount: "));

        QVERIFY(pageRefusal(pageOf(QJsonArray{payment}), "_embedded.records",
                                                    QJsonValue(QJsonValue::Undefined)).startsWith("_embedded.records: "));
        QVERIFY(pageRefusal(pageOf(QJsonArray{payment}), "_embedded",
                                                    QJsonValue(QJsonValue::Undefined)).startsWith("_embedded: "));
        QVERIFY(pageRefusal(pageOf(QJsonArray{payment}), "_links.next.href",
                                                    QJsonValue(QJsonValue::Undefined)).startsWith("_links.next.href: "));
        QVERIFY(pageRefusal(pageOf(QJsonArray{QString("payment")}), "x", QJsonValue())
                    .startsWith("_embedded.records[0]: "));
        QVERIFY(pageRefusal(pageOf(QJsonArray{payment}), "_embedded.records[0].type_i",
                                         QJsonValue(QJsonValue::Undefined)).startsWith("_embedded.records[0].type_i: "));
        QVERIFY(!pageRefusal("[]", "x", QJsonValue()).isEmpty());

        OperationPage good(nullptr);
        good.loadFromJson(pageOf(QJsonArray{payment, payment}));
        QCOMPARE(good.size(), 2);
    }

    /** One real operation of each type Horizon returned, testnet and pubnet, loads as its class. */
    void testRealHorizonOperationsOfEveryType()
    {
        OperationPage operations(nullptr);
        operations.loadFromJson(realOperationsOfEveryType());
        const QJsonArray records = QJsonDocument::fromJson(realOperationsOfEveryType()).object()
                .value("_embedded").toObject().value("records").toArray();
        QCOMPARE(operations.size(), records.size());
        QVERIFY(operations.size() >= 25);
        for (int i = 0; i < operations.size(); ++i) {
            OperationResponse* op = dynamic_cast<OperationResponse*>(operations.getRecords().at(i));
            QVERIFY(op != nullptr);
            QCOMPARE(QString::number(op->getId()), records.at(i).toObject().value("id").toString());
            if (auto* payment = dynamic_cast<PaymentOperationResponse*>(op))
                QCOMPARE(payment->getAmount(), records.at(i).toObject().value("amount").toString());
            if (auto* create = dynamic_cast<CreateAccountOperationResponse*>(op))
                QCOMPARE(create->getStartingBalance(), records.at(i).toObject().value("starting_balance").toString());
            if (auto* deposit = dynamic_cast<LiquidityPoolDepositOperationResponse*>(op))
                QCOMPARE(deposit->getMinPriceR().getNumerator(),
                         records.at(i).toObject().value("min_price_r").toObject().value("n").toInt());
        }
    }

    /**
     * Every member Horizon always sends in an operation is required, for
     * every operation type Horizon returned. Missing or null, a path
     * payment's source amount, most or least read "", a pool deposit's
     * reserves and shares an empty list and "", an offer's price "", the
     * transaction hash and time empty, and transaction_successful false.
     * A pool deposit's or withdrawal's assets are required because its
     * transaction succeeded (these all did): Horizon names them only then,
     * and the same operation with its transaction failed reads without them.
     */
    void testMembersHorizonAlwaysSendsAreRequired()
    {
        const QStringList offer = QStringList() << "amount" << "price" << "price_r" << "price_r.n" << "price_r.d"
                                                << "buying_asset_type" << "selling_asset_type";
        QHash<int, QStringList> byType;
        byType.insert(0, QStringList() << "starting_balance" << "account" << "funder");
        byType.insert(1, QStringList() << "amount" << "asset_type" << "from" << "to");
        byType.insert(2, QStringList() << "amount" << "asset_type" << "source_asset_type" << "from" << "to"
                                       << "path" << "source_amount" << "source_max");
        byType.insert(3, QStringList() << "offer_id" << offer);
        byType.insert(4, offer);
        byType.insert(6, QStringList() << "limit" << "asset_type" << "trustor");
        byType.insert(7, QStringList() << "asset_type" << "trustee" << "trustor" << "authorize"
                                       << "authorize_to_maintain_liabilities");
        byType.insert(8, QStringList() << "account" << "into");
        byType.insert(10, QStringList() << "name" << "value");
        byType.insert(11, QStringList() << "bump_to");
        byType.insert(12, QStringList() << "offer_id" << offer);
        byType.insert(13, QStringList() << "amount" << "asset_type" << "source_asset_type" << "from" << "to"
                                        << "path" << "source_amount" << "destination_min");
        byType.insert(14, QStringList() << "asset" << "amount" << "claimants");
        byType.insert(15, QStringList() << "balance_id" << "claimant");
        byType.insert(16, QStringList() << "sponsored_id");
        byType.insert(19, QStringList() << "amount" << "asset_type" << "from");
        byType.insert(20, QStringList() << "balance_id");
        byType.insert(21, QStringList() << "asset_type" << "trustor");
        // The assets: the transaction succeeded.
        byType.insert(22, QStringList() << "liquidity_pool_id" << "reserves_max" << "reserves_max[0].amount"
                                        << "reserves_max[1].asset" << "min_price" << "min_price_r" << "max_price"
                                        << "max_price_r" << "reserves_deposited" << "reserves_deposited[1].amount"
                                        << "reserves_deposited[0].asset" << "shares_received");
        byType.insert(23, QStringList() << "liquidity_pool_id" << "reserves_min" << "reserves_min[1].asset"
                                        << "reserves_min[0].amount" << "shares" << "reserves_received"
                                        << "reserves_received[0].asset" << "reserves_received[1].amount");
        byType.insert(24, QStringList() << "function" << "address" << "salt");
        byType.insert(25, QStringList() << "extend_to");
        const QStringList everyOperation = QStringList() << "id" << "paging_token" << "type_i" << "type"
                                                         << "source_account" << "created_at" << "transaction_hash"
                                                         << "transaction_successful" << "_links" << "_links.self"
                                                         << "_links.transaction" << "_links.effects"
                                                         << "_links.succeeds" << "_links.precedes"
                                                         << "_links.self.href";
        const QJsonArray records = QJsonDocument::fromJson(realOperationsOfEveryType()).object()
                .value("_embedded").toObject().value("records").toArray();
        int checked = 0;
        for (const QJsonValue& record : records) {
            const int type = record.toObject().value("type_i").toInt();
            QStringList members = everyOperation + byType.value(type);
            // An invocation's parameters and balance changes may be null
            // (Go renders an empty slice so); their entries are whole.
            if (type == 24) {
                if (!record.toObject().value("parameters").toArray().isEmpty())
                    members << "parameters[0].type" << "parameters[0].value";
                if (!record.toObject().value("asset_balance_changes").toArray().isEmpty())
                    members << "asset_balance_changes[0].type" << "asset_balance_changes[0].amount"
                            << "asset_balance_changes[0].asset_type";
            }
            const QByteArray page = pageOf(QJsonArray{record});
            QVERIFY(pageRefusal(page, "_embedded.records[0].id", record.toObject().value("id")).isEmpty());
            for (const QString& member : members) {
                const QString field = QStringLiteral("_embedded.records[0].") + member;
                const QString missing = pageRefusal(page, field, QJsonValue(QJsonValue::Undefined));
                QVERIFY2(missing.startsWith(field + ": "), qPrintable(QString::number(type) + " " + field + " -> " + missing));
                const QString null = pageRefusal(page, field, QJsonValue());
                QVERIFY2(null.startsWith(field + ": "), qPrintable(QString::number(type) + " " + field + " -> " + null));
                ++checked;
            }
        }
        QVERIFY(checked > 300);
        // Go renders an invocation with no parameters or balance changes
        // with null ones: that is read, as empty lists.
        for (const QJsonValue& record : records) {
            if (record.toObject().value("type_i").toInt() != 24)
                continue;
            QJsonObject bare = record.toObject();
            bare.insert("parameters", QJsonValue());
            bare.insert("asset_balance_changes", QJsonValue());
            OperationPage read(nullptr);
            read.loadFromJson(pageOf(QJsonArray{bare}));
            QCOMPARE(read.size(), 1);
        }
        // The same pool deposit and withdrawal with their transaction failed:
        // Horizon leaves every asset out, and each one missing is read (they
        // were refused). Missing where the transaction succeeded, refused.
        int assets = 0;
        for (const char* type : { "liquidity_pool_deposit", "liquidity_pool_withdraw" }) {
            const QJsonObject succeeded = firstRealOperation(type);
            QJsonObject failed = succeeded;
            failed.insert("transaction_successful", false);
            for (const char* list : { "reserves_max", "reserves_deposited", "reserves_min", "reserves_received" }) {
                if (!succeeded.contains(QLatin1String(list)))
                    continue;
                for (int i = 0; i < 2; ++i) {
                    const QString field = QStringLiteral("_embedded.records[0].%1[%2].asset").arg(QLatin1String(list)).arg(i);
                    const QString missing = pageRefusal(pageOf(QJsonArray{failed}), field, QJsonValue(QJsonValue::Undefined));
                    QVERIFY2(missing.isEmpty(), qPrintable(field + " -> " + missing));
                    QVERIFY2(pageRefusal(pageOf(QJsonArray{succeeded}), field, QJsonValue(QJsonValue::Undefined))
                                 .startsWith(field + ": missing, but Horizon sends it when the transaction succeeded"),
                             qPrintable(field));
                    QVERIFY2(pageRefusal(pageOf(QJsonArray{succeeded}), field, QJsonValue()).startsWith(field + ": "),
                             qPrintable(field));
                    ++assets;
                }
            }
            // An amount is required either way.
            const QString amount = QStringLiteral("_embedded.records[0].%1[1].amount")
                                       .arg(QLatin1String(failed.contains("reserves_max") ? "reserves_max" : "reserves_min"));
            QVERIFY(pageRefusal(pageOf(QJsonArray{failed}), amount, QJsonValue(QJsonValue::Undefined))
                        .startsWith(amount + ": "));
        }
        QCOMPARE(assets, 8);
    }

    /**
     * A pool deposit or withdrawal whose transaction failed, as Horizon
     * (29.0.0, testnet) serves it: no asset in any reserve (go-stellar-sdk
     * base.AssetAmount: asset omitempty, "may be empty when unknown"; the
     * ingester names the pool's assets only for a successful transaction),
     * and 0 for what was deposited, received or issued. The operation was
     * refused ("reserves_deposited[0].asset: missing"), and with it an
     * account's operations with include_failed=true and a failed
     * transaction's operations: every page holding one.
     */
    void testFailedPoolOperationsAreRead()
    {
        // Horizon served /operations/{id} of each failed pool operation, and
        // each failed transaction's operations, as exactly these records.
        const QJsonArray records = QJsonDocument::fromJson(failedPoolAccountOperations()).object()
                .value("_embedded").toObject().value("records").toArray();
        QCOMPARE(records.size(), 7);
        GenericOperation deposit;
        deposit.loadFromJson(QJsonDocument(records.at(3).toObject()).toJson());
        auto* d = dynamic_cast<LiquidityPoolDepositOperationResponse*>(deposit.operation());
        QVERIFY(d != nullptr);
        QCOMPARE(d->isTransactionSuccessful(), Boolean(false));
        QCOMPARE(d->getLiquidityPoolId(), QString("a468d41d8e9b8f3c7209651608b74b7db7ac9952dcae0cdf24871d1d9c7b0088"));
        QCOMPARE(d->getReservesMax().size(), 2);
        QCOMPARE(d->getReservesMax().at(0).getAsset(), QString());
        QCOMPARE(d->getReservesMax().at(0).getAmount(), QString("1.0000000"));
        QCOMPARE(d->getReservesMax().at(1).getAmount(), QString("1.0000000"));
        QCOMPARE(d->getReservesDeposited().size(), 2);
        QCOMPARE(d->getReservesDeposited().at(1).getAsset(), QString());
        QCOMPARE(d->getReservesDeposited().at(1).getAmount(), QString("0.0000000"));
        QCOMPARE(d->getSharesReceived(), QString("0.0000000"));
        QCOMPARE(d->getMinPriceR().getNumerator(), 1);
        QCOMPARE(d->getMinPriceR().getDenominator(), 2);

        GenericOperation withdraw;
        withdraw.loadFromJson(QJsonDocument(records.at(4).toObject()).toJson());
        auto* w = dynamic_cast<LiquidityPoolWithdrawOperationResponse*>(withdraw.operation());
        QVERIFY(w != nullptr);
        QCOMPARE(w->isTransactionSuccessful(), Boolean(false));
        QCOMPARE(w->getShares(), QString("1.0000000"));
        QCOMPARE(w->getReservesMin().size(), 2);
        QCOMPARE(w->getReservesMin().at(1).getAsset(), QString());
        QCOMPARE(w->getReservesMin().at(1).getAmount(), QString("0.0000000"));
        QCOMPARE(w->getReservesReceived().size(), 2);
        QCOMPARE(w->getReservesReceived().at(0).getAsset(), QString());
        QCOMPARE(w->getReservesReceived().at(0).getAmount(), QString("0.0000000"));

        // The pages: the account's operations with include_failed=true, and
        // each failed transaction's.
        struct Expected { QByteArray json; int size; int deposits; int withdrawals; };
        const Expected pages[] = {
            { failedPoolAccountOperations(), 7, 1, 1 },
            { pageOf(QJsonArray{records.at(1), records.at(2), records.at(3)}), 3, 1, 0 },
            { pageOf(QJsonArray{records.at(4)}), 1, 0, 1 },
        };
        for (const Expected& expected : pages) {
            OperationPage page(nullptr);
            page.loadFromJson(expected.json);
            QCOMPARE(page.size(), expected.size);
            int deposits = 0, withdrawals = 0;
            for (Response* record : page.getRecords()) {
                if (auto* pool = dynamic_cast<LiquidityPoolDepositOperationResponse*>(record)) {
                    ++deposits;
                    QVERIFY(pool->getReservesMax() == d->getReservesMax());
                    QVERIFY(pool->getReservesDeposited() == d->getReservesDeposited());
                }
                if (auto* pool = dynamic_cast<LiquidityPoolWithdrawOperationResponse*>(record)) {
                    ++withdrawals;
                    QVERIFY(pool->getReservesMin() == w->getReservesMin());
                    QVERIFY(pool->getReservesReceived() == w->getReservesReceived());
                }
            }
            QCOMPARE(deposits, expected.deposits);
            QCOMPARE(withdrawals, expected.withdrawals);
        }
        // What stays required in them: each amount, as an amount.
        const QString amount = QStringLiteral("_embedded.records[3].reserves_deposited[0].amount");
        QVERIFY(pageRefusal(failedPoolAccountOperations(), amount, QJsonValue(QJsonValue::Undefined))
                    .startsWith(amount + ": "));
        QVERIFY(pageRefusal(failedPoolAccountOperations(), amount, 0).startsWith(amount + ": "));
        const QString received = QStringLiteral("_embedded.records[4].reserves_received[1].amount");
        QVERIFY(pageRefusal(failedPoolAccountOperations(), received, QString("-1")).startsWith(received + ": "));
    }

    /**
     * The same operations as an account's operations stream sends them
     * (include_failed=true, Horizon 29.0.0 on testnet): the failed pool
     * deposit and withdrawal were reported as errors, "JSON parse failed:
     * reserves_deposited[0].asset: missing", and the caller never saw them.
     */
    void testFailedPoolOperationsAreStreamed()
    {
        const QString account = QStringLiteral("GDGPL4QJIJHMW52UBFBIPYCW3HZ7HDRROTS4ZMRMWZSQFPENOHVFOBE7");
        FakeServer* fakeServer = new FakeServer();
        fakeServer->addStream(QStringLiteral("/accounts/%1/operations").arg(account),
                              QStringList() << QString::fromLatin1(failedPoolOperationsStream()));
        Server* server = new Server(fakeServer->baseUrl());
        QScopedPointer<KeyPair> keyPair(KeyPair::fromAccountId(account));
        OperationPage* stream = server->operations().forAccount(keyPair.data()).includeFailed(true).stream().execute();

        int ready = 0, errors = 0, deposits = 0, withdrawals = 0;
        QStringList why;
        QObject::connect(stream, &Response::ready, [&]() {
            ++ready;
            if (auto* pool = dynamic_cast<LiquidityPoolDepositOperationResponse*>(stream->streamedElement())) {
                ++deposits;
                if (pool->getReservesDeposited().size() == 2 && pool->getReservesDeposited().at(0).getAsset().isEmpty())
                    why << QStringLiteral("deposit read");
            }
            if (auto* pool = dynamic_cast<LiquidityPoolWithdrawOperationResponse*>(stream->streamedElement())) {
                ++withdrawals;
                if (pool->getShares() == QLatin1String("1.0000000"))
                    why << QStringLiteral("withdrawal read");
            }
        });
        QObject::connect(stream, &Response::error, [&]() {
            ++errors;
            why << (stream->lastException() ? QString::fromUtf8(stream->lastException()->what()) : QString());
        });
        WAIT_FOR(ready + errors < 7)

        QVERIFY2(errors == 0, qPrintable(why.join(QStringLiteral(" | "))));
        QCOMPARE(ready, 7);
        QCOMPARE(deposits, 1);
        QCOMPARE(withdrawals, 1);
        QCOMPARE(why, QStringList() << QStringLiteral("deposit read") << QStringLiteral("withdrawal read"));

        delete server;                                  // tears down the stream and its reconnect timer
        fakeServer->deleteLater();
    }

    /**
     * A pool deposit's least and most prices are prices, as decimal text:
     * anything else was kept as it came, for the caller to read as 0.
     */
    void testDepositPriceTextIsReadExactly()
    {
        const QByteArray page = pageOf(QJsonArray{firstRealOperation("liquidity_pool_deposit")});
        QVERIFY(pageRefusal(page, "_embedded.records[0].min_price", QString("0.5x"))
                    .startsWith("_embedded.records[0].min_price: "));
        QVERIFY(pageRefusal(page, "_embedded.records[0].max_price", QString("-2.0000000"))
                    .startsWith("_embedded.records[0].max_price: "));
        QVERIFY(pageRefusal(page, "_embedded.records[0].max_price", QString("2.00000001"))
                    .startsWith("_embedded.records[0].max_price: "));
        QVERIFY(pageRefusal(page, "_embedded.records[0].min_price", QString("0.5000000")).isEmpty());
    }

    /**
     * A set_options operation's signer weight is the operation's uint32.
     * Before protocol 10 stellar-core took any weight (it counts one past
     * 255 as 255), and Horizon sends it as it was set: such an operation, and
     * the page holding it, was refused (the weight was held to 0..255). The
     * master key's weight and the thresholds are still one byte: core
     * refused more for them from the start.
     */
    void testSetOptionsSignerWeightIsTheUInt32OfTheOperation()
    {
        const QJsonObject setOptions = firstRealOperation("set_options");
        QVERIFY(!setOptions.isEmpty());
        for (const qint64 weight : { Q_INT64_C(256), Q_INT64_C(1000), Q_INT64_C(2147483648), Q_INT64_C(4294967295) }) {
            QJsonObject heavy = setOptions;
            heavy.insert("signer_weight", static_cast<double>(weight));
            OperationPage page(nullptr);
            page.loadFromJson(pageOf(QJsonArray{heavy}));
            QCOMPARE(page.size(), 1);
            auto* read = dynamic_cast<SetOptionsOperationResponse*>(page.getRecords().at(0));
            QVERIFY(read != nullptr);
            QCOMPARE(static_cast<qint64>(read->getSignerWeight()), weight);
        }
        const QByteArray page = pageOf(QJsonArray{setOptions});
        QVERIFY(pageRefusal(page, "_embedded.records[0].signer_weight", 4294967296.0)
                    .startsWith("_embedded.records[0].signer_weight: "));
        QVERIFY(pageRefusal(page, "_embedded.records[0].signer_weight", -1)
                    .startsWith("_embedded.records[0].signer_weight: "));
        QVERIFY(pageRefusal(page, "_embedded.records[0].signer_weight", 256.5)
                    .startsWith("_embedded.records[0].signer_weight: "));
        for (const char* key : { "master_key_weight", "low_threshold", "med_threshold", "high_threshold" }) {
            const QString field = QStringLiteral("_embedded.records[0].") + QLatin1String(key);
            QVERIFY2(pageRefusal(page, field, 256).startsWith(field + ": "), key);
            QVERIFY2(pageRefusal(page, field, 255).isEmpty(), key);
        }
    }
private:
    /** One real operation of each type Horizon (testnet and pubnet) returned. */
    static QByteArray realOperationsOfEveryType()
    {
        return QByteArray(
            "{\"_embedded\":{\"records\":[{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118341309550595\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/f4ae1f4eb66fe406aa8bd52572c58eaed9d8fe5dfc8b80d83588bc7a3b4cbb61\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118341309550595/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118341309550595\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118341309550595\"}},\"id\":\"21118341309550595\",\"paging_token\":\"21118341309550595\",\"transaction_successful\":true,\"source_account\":\"GDTBRJDRRQHHH4EPXOIBEAXTDJVETYVBOO53UGD4NDTW5P7YKG2DAWBD\",\"type\":\"account_merge\",\"type_i\":8,\"created_at\":\"2026-09-28T15:16:12Z\",\"transaction_hash\":\"f4ae1f4eb66fe406aa8bd52572c58eaed9d8fe5dfc8b80d83588bc7a3b4cbb61\",\"account\":\"GDTBRJDRRQHHH4EPXOIBEAXTDJVETYVBOO53UGD4NDTW5P7YKG2DAWBD\",\"into\":\"GCEJWIJD7VYETN2WPVL7EJUK7JPHBTNOUCQXY2SU72FAVEWMOQYR34SR\"},{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118100791373825\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/3bfc81ee6301f7590d9a7f37ace79a19a8d882214bbfd0544a793a5a7cc9ca77\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118100791373825/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118100791373825\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118100791373825\"}},\"id\":\"21118100791373825\",\"paging_token\":\"21118100791373825\",\"transaction_successful\":true,\"source_account\":\"GANCKFGWXAEMJ53GP6VIEHIHRCFO3CIG5ID54WEO7YDR32Y6OG4N2PTU\",\"type\":\"allow_trust\",\"type_i\":7,\"created_at\":\"2026-09-28T15:11"
            ":32Z\",\"transaction_hash\":\"3bfc81ee6301f7590d9a7f37ace79a19a8d882214bbfd0544a793a5a7cc9ca77\",\"asset_type\":\"credit_alphanum4\",\"asset_code\":\"USD\",\"asset_issuer\":\"GANCKFGWXAEMJ53GP6VIEHIHRCFO3CIG5ID54WEO7YDR32Y6OG4N2PTU\",\"trustee\":\"GANCKFGWXAEMJ53GP6VIEHIHRCFO3CIG5ID54WEO7YDR32Y6OG4N2PTU\",\"trustor\":\"GDRVT4WWI4DYCLXU5UP7YCR322KBPDWVEUQSMDAVNNLPPNVDOVIVRFKU\",\"authorize\":true,\"authorize_to_maintain_liabilities\":true},{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118221050449921\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/b96e053a056f98a02a48b277dec7f6035e242dbe5d3ba378006e4fe817b2eda7\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118221050449921/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118221050449921\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118221050449921\"}},\"id\":\"21118221050449921\",\"paging_token\":\"21118221050449921\",\"transaction_successful\":true,\"source_account\":\"GCBS7GI62E2XVLK2O42KTCRR5RRNFZVKUVDHULPX4TIYGNWAZFML5BJQ\",\"type\":\"begin_sponsoring_future_reserves\",\"type_i\":16,\"created_at\":\"2026-09-28T15:13:52Z\",\"transaction_hash\":\"b96e053a056f98a02a48b277dec7f6035e242dbe5d3ba378006e4fe817b2eda7\",\"sponsored_id\":\"GCRYREIBNPYMSL436Q6VBGWWQUZYYOFFZALPN77ZHVRNXBVYSPFITNDO\"},{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118895360315397\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/a9f8eb8174b8da951dc5c4539d658f4bca35a3cdce762236ae47585d14d631a1\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118895360315397/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118895360"
            "315397\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118895360315397\"}},\"id\":\"21118895360315397\",\"paging_token\":\"21118895360315397\",\"transaction_successful\":true,\"source_account\":\"GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD\",\"type\":\"bump_sequence\",\"type_i\":11,\"created_at\":\"2026-09-28T15:26:57Z\",\"transaction_hash\":\"a9f8eb8174b8da951dc5c4539d658f4bca35a3cdce762236ae47585d14d631a1\",\"bump_to\":\"0\"},{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118341309550594\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/f4ae1f4eb66fe406aa8bd52572c58eaed9d8fe5dfc8b80d83588bc7a3b4cbb61\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118341309550594/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118341309550594\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118341309550594\"}},\"id\":\"21118341309550594\",\"paging_token\":\"21118341309550594\",\"transaction_successful\":true,\"source_account\":\"GDTBRJDRRQHHH4EPXOIBEAXTDJVETYVBOO53UGD4NDTW5P7YKG2DAWBD\",\"type\":\"change_trust\",\"type_i\":6,\"created_at\":\"2026-09-28T15:16:12Z\",\"transaction_hash\":\"f4ae1f4eb66fe406aa8bd52572c58eaed9d8fe5dfc8b80d83588bc7a3b4cbb61\",\"asset_type\":\"credit_alphanum12\",\"asset_code\":\"LWTEST\",\"asset_issuer\":\"GAPIM2WRYS65WEWE2PAJYNGST35ZQXEJJQKWG7EUVMEZ2LWP7SXSNTNO\",\"limit\":\"0.0000000\",\"trustee\":\"GAPIM2WRYS65WEWE2PAJYNGST35ZQXEJJQKWG7EUVMEZ2LWP7SXSNTNO\",\"trustor\":\"GDTBRJDRRQHHH4EPXOIBEAXTDJVETYVBOO53UGD4NDTW5P7YKG2DAWBD\"},{\"_links\":{\"self\":{\"href\":\"https://horizon.stellar.org/operations/277732157525614594\"},\"transaction\":{\"href\":\"https://horizon.stellar.org/transactions/ce643830a613389b0d73bc68"
            "267505da4152912d8ff3faaf9844ad9240c7492d\"},\"effects\":{\"href\":\"https://horizon.stellar.org/operations/277732157525614594/effects\"},\"succeeds\":{\"href\":\"https://horizon.stellar.org/effects?order=desc&cursor=277732157525614594\"},\"precedes\":{\"href\":\"https://horizon.stellar.org/effects?order=asc&cursor=277732157525614594\"}},\"id\":\"277732157525614594\",\"paging_token\":\"277732157525614594\",\"transaction_successful\":true,\"source_account\":\"GALHHYP23D6TNX653MBMYH7MYTAF7XL5TT3MLMBI3FZ4F56OMCJ7LYMC\",\"type\":\"claim_claimable_balance\",\"type_i\":15,\"created_at\":\"2026-09-28T15:16:06Z\",\"transaction_hash\":\"ce643830a613389b0d73bc68267505da4152912d8ff3faaf9844ad9240c7492d\",\"balance_id\":\"00000000ebe4dd445a133fa3ed6d45599f0d811442dfad0f57499013afbbeda61d9738b1\",\"claimant\":\"GALHHYP23D6TNX653MBMYH7MYTAF7XL5TT3MLMBI3FZ4F56OMCJ7LYMC\"},{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118951194890241\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/5968f4ed4e82571d9deca442c0a8e80a654df1ad832a928d3fa4327fdeea4de3\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118951194890241/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118951194890241\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118951194890241\"}},\"id\":\"21118951194890241\",\"paging_token\":\"21118951194890241\",\"transaction_successful\":true,\"source_account\":\"GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD\",\"type\":\"clawback\",\"type_i\":19,\"created_at\":\"2026-09-28T15:28:02Z\",\"transaction_hash\":\"5968f4ed4e82571d9deca442c0a8e80a654df1ad832a928d3fa4327fdeea4de3\",\"asset_type\":\"credit_alphanum4\",\"asset_code\":\"TEST\",\"asset_issuer\":\"GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOW"
            "TX54CSBPDQCXD\",\"from\":\"GBTTX7QVGGB6VTNNAKMXIFR6YL3RYA6UU3YR7FVBLKNOB2SWSA7XMQ7H\",\"amount\":\"7.5000000\"},{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118946899927041\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/5f046b3e2e6e80e4ff04225798d7d581a955162a2289e5c975fbf5a06601ed6c\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118946899927041/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118946899927041\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118946899927041\"}},\"id\":\"21118946899927041\",\"paging_token\":\"21118946899927041\",\"transaction_successful\":true,\"source_account\":\"GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD\",\"type\":\"clawback_claimable_balance\",\"type_i\":20,\"created_at\":\"2026-09-28T15:27:57Z\",\"transaction_hash\":\"5f046b3e2e6e80e4ff04225798d7d581a955162a2289e5c975fbf5a06601ed6c\",\"balance_id\":\"00000000346974acd1e0cb35a34bfdb61f511792cb92af49acacce91f81bf32e10e24f05\"},{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118341309534209\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/6c14bdcb9152961848c848be16cdc4f9437d290145b4488c424c3f799d7d7022\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118341309534209/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118341309534209\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118341309534209\"}},\"id\":\"21118341309534209\",\"paging_token\":\"21118341309534209\",\"transaction_successful\":true,\"source_account\":\"GAIH3ULLFQ4DGSECF2AR555KZ4KNDGEKN4AFI4SU2M7B43MGK3QJZNSR\",\"type\":\"create_account"
            "\",\"type_i\":0,\"created_at\":\"2026-09-28T15:16:12Z\",\"transaction_hash\":\"6c14bdcb9152961848c848be16cdc4f9437d290145b4488c424c3f799d7d7022\",\"starting_balance\":\"10000.0000000\",\"funder\":\"GAIH3ULLFQ4DGSECF2AR555KZ4KNDGEKN4AFI4SU2M7B43MGK3QJZNSR\",\"account\":\"GDQYIZ7FCW7FLARWBZ332K2ZKLXIEY4SX66DLVKYJI3J6G4JBRBC4DHW\"},{\"_links\":{\"self\":{\"href\":\"https://horizon.stellar.org/operations/277732097396117505\"},\"transaction\":{\"href\":\"https://horizon.stellar.org/transactions/ef3cf4d411b78bc1c8d48dc93f3ddd82f3ad0db9929746a7301f8e6a7cbc37a5\"},\"effects\":{\"href\":\"https://horizon.stellar.org/operations/277732097396117505/effects\"},\"succeeds\":{\"href\":\"https://horizon.stellar.org/effects?order=desc&cursor=277732097396117505\"},\"precedes\":{\"href\":\"https://horizon.stellar.org/effects?order=asc&cursor=277732097396117505\"}},\"id\":\"277732097396117505\",\"paging_token\":\"277732097396117505\",\"transaction_successful\":true,\"source_account\":\"GDQD6RR3BVCRNGMZNU2T3BW42B5LU5R3VNMSO63KA2AOQLCPTEJLJPNB\",\"type\":\"create_claimable_balance\",\"type_i\":14,\"created_at\":\"2026-09-28T15:14:56Z\",\"transaction_hash\":\"ef3cf4d411b78bc1c8d48dc93f3ddd82f3ad0db9929746a7301f8e6a7cbc37a5\",\"sponsor\":\"GDQD6RR3BVCRNGMZNU2T3BW42B5LU5R3VNMSO63KA2AOQLCPTEJLJPNB\",\"asset\":\"JPNXRP:GDUNTDNUBPVUDEDS4N3AFFUU5WZWC4AHPAJSMW3EKQJA5HG5CDP2JPNB\",\"amount\":\"1.1984659\",\"claimants\":[{\"destination\":\"GD72YZUIF6DVP5PGTUETK7SI6ESSQTZRL6VQHH32UCVRMJH2I7IYTU6G\",\"predicate\":{\"unconditional\":true}},{\"destination\":\"GDQD6RR3BVCRNGMZNU2T3BW42B5LU5R3VNMSO63KA2AOQLCPTEJLJPNB\",\"predicate\":{\"unconditional\":true}}]},{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118912540196867\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/6650238253f9346715c0e49f9f1f2df529306dd65d24bfd354e3a6d288290f01\"},\""
            "effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118912540196867/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118912540196867\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118912540196867\"}},\"id\":\"21118912540196867\",\"paging_token\":\"21118912540196867\",\"transaction_successful\":true,\"source_account\":\"GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD\",\"type\":\"create_passive_sell_offer\",\"type_i\":4,\"created_at\":\"2026-09-28T15:27:17Z\",\"transaction_hash\":\"6650238253f9346715c0e49f9f1f2df529306dd65d24bfd354e3a6d288290f01\",\"amount\":\"50.0000000\",\"price\":\"2.0000000\",\"price_r\":{\"n\":2,\"d\":1},\"buying_asset_type\":\"credit_alphanum4\",\"buying_asset_code\":\"TEST\",\"buying_asset_issuer\":\"GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD\",\"selling_asset_type\":\"native\"},{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118221050449924\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/b96e053a056f98a02a48b277dec7f6035e242dbe5d3ba378006e4fe817b2eda7\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118221050449924/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118221050449924\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118221050449924\"}},\"id\":\"21118221050449924\",\"paging_token\":\"21118221050449924\",\"transaction_successful\":true,\"source_account\":\"GCRYREIBNPYMSL436Q6VBGWWQUZYYOFFZALPN77ZHVRNXBVYSPFITNDO\",\"type\":\"end_sponsoring_future_reserves\",\"type_i\":17,\"created_at\":\"2026-09-28T15:13:52Z\",\"transaction_hash\":\"b96e053a056f98a02a48b277dec7f6035e242dbe5d3ba378006e4fe817b2eda7\",\"begin_sponsor\":"
            "\"GCBS7GI62E2XVLK2O42KTCRR5RRNFZVKUVDHULPX4TIYGNWAZFML5BJQ\"},{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21116898200580097\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/94c41883c70b057338ae00216f14fe7476ea5a1c5baf0073d128d729d4afb322\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21116898200580097/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21116898200580097\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21116898200580097\"}},\"id\":\"21116898200580097\",\"paging_token\":\"21116898200580097\",\"transaction_successful\":true,\"source_account\":\"GBEKWTEMJ2DV4BDE7RTOXMKUQL2KI447FN5UOJNWHA7EJQCONOBZK2U2\",\"type\":\"extend_footprint_ttl\",\"type_i\":25,\"created_at\":\"2026-09-28T14:48:12Z\",\"transaction_hash\":\"94c41883c70b057338ae00216f14fe7476ea5a1c5baf0073d128d729d4afb322\",\"extend_to\":518400},{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118341309566977\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/461ca222d72891f0f20d13f7432f1780027a4ed86ba56b32f892ae8ce6bb824b\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118341309566977/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118341309566977\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118341309566977\"}},\"id\":\"21118341309566977\",\"paging_token\":\"21118341309566977\",\"transaction_successful\":true,\"source_account\":\"GBQHHOH72M522QBF7SMY57JH6FIN7YKTZUWSO4S5IFBXV3B7FI2UQLIQ\",\"type\":\"invoke_host_function\",\"type_i\":24,\"created_at\":\"2026-09-28T15:16:12Z\",\"transaction_hash\":\"461ca222d72891f0f20d13f7432f1780027a4ed86"
            "ba56b32f892ae8ce6bb824b\",\"function\":\"HostFunctionTypeHostFunctionTypeInvokeContract\",\"parameters\":[{\"value\":\"AAAAEgAAAAEJIX5C6S3X6ftDOw+T3MtGCdZN6Xv2zEfpPmTF42f8og==\",\"type\":\"Address\"},{\"value\":\"AAAADwAAAARwdXNo\",\"type\":\"Sym\"},{\"value\":\"AAAABQAAAAAB8XaG\",\"type\":\"U64\"},{\"value\":\"AAAADQAAADCYcrRACpO1w7Z2C5y/E04ZywC9VhMvt1oJbG+eYAzH8bMasXxEkZrAzDn6OAqqhEU=\",\"type\":\"Bytes\"},{\"value\":\"AAAADQAAAGAYcrRACpO1w7Z2C5y/E04ZywC9VhMvt1oJbG+eYAzH8bMasXxEkZrAzDn6OAqqhEUKTFPJkOcuSF2IdRKbAhvKU1Ed39EaG/7cgGBWpBShzl1PQWF3b76+a0J7ncqeut4=\",\"type\":\"Bytes\"}],\"address\":\"\",\"salt\":\"\",\"asset_balance_changes\":null},{\"_links\":{\"self\":{\"href\":\"https://horizon.stellar.org/operations/277731706554052609\"},\"transaction\":{\"href\":\"https://horizon.stellar.org/transactions/1b55f31a4cd7312a4782da47520b5e46cb405269fc5679505094e403884fdad9\"},\"effects\":{\"href\":\"https://horizon.stellar.org/operations/277731706554052609/effects\"},\"succeeds\":{\"href\":\"https://horizon.stellar.org/effects?order=desc&cursor=277731706554052609\"},\"precedes\":{\"href\":\"https://horizon.stellar.org/effects?order=asc&cursor=277731706554052609\"}},\"id\":\"277731706554052609\",\"paging_token\":\"277731706554052609\",\"transaction_successful\":true,\"source_account\":\"GDQVJIGZ5TPOQLMVUQ6AWX5XZNE3YDEL5UXAOM3574U4HGNKS2LE4RMD\",\"type\":\"liquidity_pool_deposit\",\"type_i\":22,\"created_at\":\"2026-09-28T15:07:21Z\",\"transaction_hash\":\"1b55f31a4cd7312a4782da47520b5e46cb405269fc5679505094e403884fdad9\",\"liquidity_pool_id\":\"ad45b80a3a90ccaa251375e208006651a9f7242adc73e0d6928b0f3b2b0255c2\",\"reserves_max\":[{\"asset\":\"SSLX:GBHFGY3ZNEJWLNO4LBUKLYOCEK4V7ENEBJGPRHHX7JU47GWHBREH37UR\",\"amount\":\"2128.4295531\"},{\"asset\":\"STARDUST:GCPLNANLXTV2G6NPIRDPOLM5NAO5XH2IPYQBZCTTAKPNV4IRTU5X5B4S\",\"amount\":\"1267631.6231215\"}],\"min_price\":\"0.0016622\",\""
            "min_price_r\":{\"n\":8311,\"d\":5000000},\"max_price\":\"0.0016958\",\"max_price_r\":{\"n\":8479,\"d\":5000000},\"reserves_deposited\":[{\"asset\":\"SSLX:GBHFGY3ZNEJWLNO4LBUKLYOCEK4V7ENEBJGPRHHX7JU47GWHBREH37UR\",\"amount\":\"2128.4295531\"},{\"asset\":\"STARDUST:GCPLNANLXTV2G6NPIRDPOLM5NAO5XH2IPYQBZCTTAKPNV4IRTU5X5B4S\",\"amount\":\"1267626.0350791\"}],\"shares_received\":\"21585.6866624\"},{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118934015029249\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/f0010ebd03fb1799b621e3cc2ee4182d56755cb7d3e465d1f0895c4843c5b3c6\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118934015029249/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118934015029249\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118934015029249\"}},\"id\":\"21118934015029249\",\"paging_token\":\"21118934015029249\",\"transaction_successful\":true,\"source_account\":\"GBTTX7QVGGB6VTNNAKMXIFR6YL3RYA6UU3YR7FVBLKNOB2SWSA7XMQ7H\",\"type\":\"liquidity_pool_withdraw\",\"type_i\":23,\"created_at\":\"2026-09-28T15:27:42Z\",\"transaction_hash\":\"f0010ebd03fb1799b621e3cc2ee4182d56755cb7d3e465d1f0895c4843c5b3c6\",\"liquidity_pool_id\":\"3fc7b1d1333b185f1bf872174d7c362b7f2f1fcbe41c396a42ddbeb79b3b2dc4\",\"reserves_min\":[{\"asset\":\"native\",\"amount\":\"0.0000000\"},{\"asset\":\"TEST:GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD\",\"amount\":\"0.0000000\"}],\"shares\":\"10.0000000\",\"reserves_received\":[{\"asset\":\"native\",\"amount\":\"3.3203915\"},{\"asset\":\"TEST:GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD\",\"amount\":\"30.1212331\"}]},{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118010597056513\"},\"transaction\""
            ":{\"href\":\"https://horizon-testnet.stellar.org/transactions/23d62e06435725eec4aa7f3d5b50067b9ef8aa0bca0518f1b2bb85f2e721dfad\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118010597056513/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118010597056513\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118010597056513\"}},\"id\":\"21118010597056513\",\"paging_token\":\"21118010597056513\",\"transaction_successful\":true,\"source_account\":\"GDL75QQUCLJECIOHK2LPRW3YICWDS66QUTWX7FF3ZY4FJGYCKLRHRCJ2\",\"type\":\"manage_buy_offer\",\"type_i\":12,\"created_at\":\"2026-09-28T15:09:47Z\",\"transaction_hash\":\"23d62e06435725eec4aa7f3d5b50067b9ef8aa0bca0518f1b2bb85f2e721dfad\",\"amount\":\"5.0000000\",\"price\":\"100.0000000\",\"price_r\":{\"n\":100,\"d\":1},\"buying_asset_type\":\"credit_alphanum4\",\"buying_asset_code\":\"USDC\",\"buying_asset_issuer\":\"GBBD47IF6LWK7P7MDEVSCWR7DPUWV3NY3DTQEVFL4NAT4AQH3ZLLFLA5\",\"selling_asset_type\":\"native\",\"offer_id\":\"0\"},{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118959784837122\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/8a9225aea7641e152fb3524f1a0dca6cd661428a2011d825d61d3d5e266c6b0e\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118959784837122/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118959784837122\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118959784837122\"}},\"id\":\"21118959784837122\",\"paging_token\":\"21118959784837122\",\"transaction_successful\":true,\"source_account\":\"GBTTX7QVGGB6VTNNAKMXIFR6YL3RYA6UU3YR7FVBLKNOB2SWSA7XMQ7H\",\"type\":\"manage_data\",\"type_i\":10,\"created_at\":\"2026-"
            "09-28T15:28:12Z\",\"transaction_hash\":\"8a9225aea7641e152fb3524f1a0dca6cd661428a2011d825d61d3d5e266c6b0e\",\"sponsor\":\"GCALKXRTML6DHW2CT332CVAOYEWZQ2OBXGHW3YNY4ZLOE2QE3OFH7LLA\",\"name\":\"sp\",\"value\":\"c3BvbnNvcmVk\"},{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118040661839873\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/46dd02c512d947df0a6afbd6bdc97fb05e8413689a753a53691297671090dd2d\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118040661839873/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118040661839873\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118040661839873\"}},\"id\":\"21118040661839873\",\"paging_token\":\"21118040661839873\",\"transaction_successful\":true,\"source_account\":\"GBCVBQDCFPOTNQX4A4ZTIJNCARSRK24C7TGG5QZBKOVQL3FDA4HVJ3MJ\",\"type\":\"manage_sell_offer\",\"type_i\":3,\"created_at\":\"2026-09-28T15:10:22Z\",\"transaction_hash\":\"46dd02c512d947df0a6afbd6bdc97fb05e8413689a753a53691297671090dd2d\",\"amount\":\"3222.8235339\",\"price\":\"0.2177380\",\"price_r\":{\"n\":108869,\"d\":500000},\"buying_asset_type\":\"credit_alphanum12\",\"buying_asset_code\":\"CITEST\",\"buying_asset_issuer\":\"GBMMZMK2DC4FFP4CAI6KCVNCQ7WLO5A7DQU7EC7WGHRDQBZB763X4OQI\",\"selling_asset_type\":\"native\",\"offer_id\":\"828697\"},{\"_links\":{\"self\":{\"href\":\"https://horizon.stellar.org/operations/277732157525495809\"},\"transaction\":{\"href\":\"https://horizon.stellar.org/transactions/82c10cd83528afcbead4cdf56b09adc51e2165e777b3bc89e45a207674671709\"},\"effects\":{\"href\":\"https://horizon.stellar.org/operations/277732157525495809/effects\"},\"succeeds\":{\"href\":\"https://horizon.stellar.org/effects?order=desc&cursor=277732157525495809\"},\"precedes\":{\"hre"
            "f\":\"https://horizon.stellar.org/effects?order=asc&cursor=277732157525495809\"}},\"id\":\"277732157525495809\",\"paging_token\":\"277732157525495809\",\"transaction_successful\":true,\"source_account\":\"GAFB7IYPCYZCODQBB5BR5JO45JC4PPVLARUAXQSFHWTLH2KMHPWJ36GD\",\"type\":\"path_payment_strict_receive\",\"type_i\":2,\"created_at\":\"2026-09-28T15:16:06Z\",\"transaction_hash\":\"82c10cd83528afcbead4cdf56b09adc51e2165e777b3bc89e45a207674671709\",\"asset_type\":\"credit_alphanum4\",\"asset_code\":\"SSLX\",\"asset_issuer\":\"GBHFGY3ZNEJWLNO4LBUKLYOCEK4V7ENEBJGPRHHX7JU47GWHBREH37UR\",\"from\":\"GAFB7IYPCYZCODQBB5BR5JO45JC4PPVLARUAXQSFHWTLH2KMHPWJ36GD\",\"to\":\"GAFB7IYPCYZCODQBB5BR5JO45JC4PPVLARUAXQSFHWTLH2KMHPWJ36GD\",\"amount\":\"65.1227001\",\"path\":[{\"asset_type\":\"credit_alphanum4\",\"asset_code\":\"1x\",\"asset_issuer\":\"GBGRBCUB6L7LH4JQ6EPDP7REH2DDACMCUQI76M3P6DM52QWU2Z5LIEVW\"},{\"asset_type\":\"credit_alphanum4\",\"asset_code\":\"USDC\",\"asset_issuer\":\"GA5ZSEJYB37JRC5AVCIA5MOP4RHTM335X2KGX3IHOJAPP5RE34K4KZVN\"}],\"source_amount\":\"16.5807079\",\"source_max\":\"85.6864705\",\"source_asset_type\":\"credit_alphanum4\",\"source_asset_code\":\"cuT\",\"source_asset_issuer\":\"GBGRBCUB6L7LH4JQ6EPDP7REH2DDACMCUQI76M3P6DM52QWU2Z5LIEVW\"},{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118341309562881\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/4d6a195a7377e04f0b3c1866c5e725c3ec6f48d248a91a2dd474e619f99aa7a5\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118341309562881/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118341309562881\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118341309562881\"}},\"id\":\"21118341309562881\",\"paging_token\":\"21118341309562881\",\"transa"
            "ction_successful\":true,\"source_account\":\"GBMXZ62UDDWDWZU464HDDFGO6LZ4IAFPHZTIU3JPODNZQFAUSCIRMHCM\",\"type\":\"path_payment_strict_send\",\"type_i\":13,\"created_at\":\"2026-09-28T15:16:12Z\",\"transaction_hash\":\"4d6a195a7377e04f0b3c1866c5e725c3ec6f48d248a91a2dd474e619f99aa7a5\",\"asset_type\":\"credit_alphanum4\",\"asset_code\":\"USDC\",\"asset_issuer\":\"GCDXYIOQJXFYG42J6C4K4NN7TGMX4TFF5DZK2ENDMNHONRR5D3KIPDS5\",\"from\":\"GBMXZ62UDDWDWZU464HDDFGO6LZ4IAFPHZTIU3JPODNZQFAUSCIRMHCM\",\"to\":\"GBMXZ62UDDWDWZU464HDDFGO6LZ4IAFPHZTIU3JPODNZQFAUSCIRMHCM\",\"amount\":\"3.7153200\",\"path\":[],\"source_amount\":\"1793.5502249\",\"destination_min\":\"3.5295541\",\"source_asset_type\":\"credit_alphanum12\",\"source_asset_code\":\"SMAJDV\",\"source_asset_issuer\":\"GBHOK32DC5HPBQUQTUPK5Y66A27MSBQGK4KDRLP4BGEQFL5O3AFTHIH5\"},{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118341309550593\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/f4ae1f4eb66fe406aa8bd52572c58eaed9d8fe5dfc8b80d83588bc7a3b4cbb61\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118341309550593/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118341309550593\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118341309550593\"}},\"id\":\"21118341309550593\",\"paging_token\":\"21118341309550593\",\"transaction_successful\":true,\"source_account\":\"GDTBRJDRRQHHH4EPXOIBEAXTDJVETYVBOO53UGD4NDTW5P7YKG2DAWBD\",\"type\":\"payment\",\"type_i\":1,\"created_at\":\"2026-09-28T15:16:12Z\",\"transaction_hash\":\"f4ae1f4eb66fe406aa8bd52572c58eaed9d8fe5dfc8b80d83588bc7a3b4cbb61\",\"asset_type\":\"credit_alphanum12\",\"asset_code\":\"LWTEST\",\"asset_issuer\":\"GAPIM2WRYS65WEWE2PAJYNGST35ZQXEJJQKWG7EUVMEZ2LWP7SXSNTNO\",\"from\":\"GDTBRJDRRQ"
            "HHH4EPXOIBEAXTDJVETYVBOO53UGD4NDTW5P7YKG2DAWBD\",\"to\":\"GAPIM2WRYS65WEWE2PAJYNGST35ZQXEJJQKWG7EUVMEZ2LWP7SXSNTNO\",\"amount\":\"99.9999000\"},{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21119071453978626\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/abaeddec932f6d54b0ec2788c2378036b0cd6666b1c54f36880eca22b08724aa\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21119071453978626/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21119071453978626\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21119071453978626\"}},\"id\":\"21119071453978626\",\"paging_token\":\"21119071453978626\",\"transaction_successful\":true,\"source_account\":\"GCALKXRTML6DHW2CT332CVAOYEWZQ2OBXGHW3YNY4ZLOE2QE3OFH7LLA\",\"type\":\"revoke_sponsorship\",\"type_i\":18,\"created_at\":\"2026-09-28T15:30:22Z\",\"transaction_hash\":\"abaeddec932f6d54b0ec2788c2378036b0cd6666b1c54f36880eca22b08724aa\",\"data_account_id\":\"GBTTX7QVGGB6VTNNAKMXIFR6YL3RYA6UU3YR7FVBLKNOB2SWSA7XMQ7H\",\"data_name\":\"sp\"},{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118221050462209\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/f7f915090261a4e7f4685cd636f4c8c072eda3a4a97d5ecf81a0fbd5d319367e\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118221050462209/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118221050462209\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118221050462209\"}},\"id\":\"21118221050462209\",\"paging_token\":\"21118221050462209\",\"transaction_successful\":true,\"source_account\":\"GDSVLIT2LSWQCQ6ZOT225ETNPKDGJFPKOKER7PXJB54RDX43RG"
            "YOFI2E\",\"type\":\"set_options\",\"type_i\":5,\"created_at\":\"2026-09-28T15:13:52Z\",\"transaction_hash\":\"f7f915090261a4e7f4685cd636f4c8c072eda3a4a97d5ecf81a0fbd5d319367e\",\"signer_key\":\"GCBBMGCVLQL2GODKYKKS2PSF55LAJJYLBCVENMHMWEUO4ZHOV4O4Q5RE\",\"signer_weight\":0},{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21116417164185604\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/60d881a9651adba1580dcd35c1c2358bcae411d0a5794046fb4594c2e098880f\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21116417164185604/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21116417164185604\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21116417164185604\"}},\"id\":\"21116417164185604\",\"paging_token\":\"21116417164185604\",\"transaction_successful\":true,\"source_account\":\"GAZOKSGVZTAHBNRXIA7ZNVPXIBCXBJU65FYBPX3QMJ6B6Z3IWTG4BSNC\",\"type\":\"set_trust_line_flags\",\"type_i\":21,\"created_at\":\"2026-09-28T14:38:52Z\",\"transaction_hash\":\"60d881a9651adba1580dcd35c1c2358bcae411d0a5794046fb4594c2e098880f\",\"asset_type\":\"credit_alphanum4\",\"asset_code\":\"NET2\",\"asset_issuer\":\"GAZOKSGVZTAHBNRXIA7ZNVPXIBCXBJU65FYBPX3QMJ6B6Z3IWTG4BSNC\",\"trustor\":\"GBGQMGNZ7ONGS2YING5XP5IFFTZGUF4KJ7NOOP5NVARGWPIBPU4ONQXL\",\"set_flags\":[1],\"set_flags_s\":[\"authorized\"]}]},\"_links\":{\"self\":{\"href\":\"https://horizon.example/x?cursor=&limit=2&order=asc\"},\"next\":{\"href\":\"https://horizon.example/x?cursor=2&limit=2&order=asc\"},\"prev\":{\"href\":\"https://horizon.example/x?cursor=1&limit=2&order=desc\"}}}");
    }

    /**
     * An account's operations with include_failed=true, as Horizon 29.0.0 on
     * testnet served them: its creation, a failed transaction of two
     * manage_data and a deposit into pool a468d41d..., a failed one of a
     * withdrawal from it, and a successful one of two manage_data.
     */
    static QByteArray failedPoolAccountOperations()
    {
        return QByteArray(
            "{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/GDGPL4QJIJHMW52UBFBIPYCW3HZ7HDRROTS4ZMRMWZSQFPENOHVFOBE7/operations?cursor=&include_failed=true&limit=50&order=asc\"},\"next\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/GDGPL4QJIJHMW52UBFBIPYCW3HZ7HDRROTS4ZMRMWZSQFPENOHVFOBE7/operations?cursor=21184664194531330&include_failed=true&limit=50&order=asc\"},\"prev\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/GDGPL4QJIJHMW52UBFBIPYCW3HZ7HDRROTS4ZMRMWZSQFPENOHVFOBE7/operations?cursor=21184651309621249&include_failed=true&limit=50&order=desc\"}},\"_embedded\":{\"records\":[{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21184651309621249\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/774bff54682a201c63a92f219424a38bf9be1a1d614641cd48ceb81129c695f9\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21184651309621249/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?orde"
            "r=desc&cursor=21184651309621249\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21184651309621249\"}},\"id\":\"21184651309621249\",\"paging_token\":\"21184651309621249\",\"transaction_successful\":true,\"source_account\":\"GAIH3ULLFQ4DGSECF2AR555KZ4KNDGEKN4AFI4SU2M7B43MGK3QJZNSR\",\"type\":\"create_account\",\"type_i\":0,\"created_at\":\"2026-09-29T12:42:47Z\",\"transaction_hash\":\"774bff54682a201c63a92f219424a38bf9be1a1d614641cd48ceb81129c695f9\",\"starting_balance\":\"10000.0000000\",\"funder\":\"GAIH3ULLFQ4DGSECF2AR555KZ4KNDGEKN4AFI4SU2M7B43MGK3QJZNSR\",\"account\":\"GDGPL4QJIJHMW52UBFBIPYCW3HZ7HDRROTS4ZMRMWZSQFPENOHVFOBE7\"},{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21184655604584449\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/5f1005f1aa1b9ab9f5d35c0f087f3bbce64c59f8b7bda40d81a47bfdc889eb45\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21184655604584449/effects\"},\"succeeds\":{\"href\":\"https://horizon-testn"
            "et.stellar.org/effects?order=desc&cursor=21184655604584449\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21184655604584449\"}},\"id\":\"21184655604584449\",\"paging_token\":\"21184655604584449\",\"transaction_successful\":false,\"source_account\":\"GDGPL4QJIJHMW52UBFBIPYCW3HZ7HDRROTS4ZMRMWZSQFPENOHVFOBE7\",\"type\":\"manage_data\",\"type_i\":10,\"created_at\":\"2026-09-29T12:42:52Z\",\"transaction_hash\":\"5f1005f1aa1b9ab9f5d35c0f087f3bbce64c59f8b7bda40d81a47bfdc889eb45\",\"name\":\"k\",\"value\":\"dg==\"},{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21184655604584450\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/5f1005f1aa1b9ab9f5d35c0f087f3bbce64c59f8b7bda40d81a47bfdc889eb45\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21184655604584450/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21184655604584450\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/eff"
            "ects?order=asc&cursor=21184655604584450\"}},\"id\":\"21184655604584450\",\"paging_token\":\"21184655604584450\",\"transaction_successful\":false,\"source_account\":\"GDGPL4QJIJHMW52UBFBIPYCW3HZ7HDRROTS4ZMRMWZSQFPENOHVFOBE7\",\"type\":\"manage_data\",\"type_i\":10,\"created_at\":\"2026-09-29T12:42:52Z\",\"transaction_hash\":\"5f1005f1aa1b9ab9f5d35c0f087f3bbce64c59f8b7bda40d81a47bfdc889eb45\",\"name\":\"k\",\"value\":\"\"},{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21184655604584451\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/5f1005f1aa1b9ab9f5d35c0f087f3bbce64c59f8b7bda40d81a47bfdc889eb45\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21184655604584451/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21184655604584451\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21184655604584451\"}},\"id\":\"21184655604584451\",\"paging_token\":\"21184655604584451\",\"transaction_successf"
            "ul\":false,\"source_account\":\"GDGPL4QJIJHMW52UBFBIPYCW3HZ7HDRROTS4ZMRMWZSQFPENOHVFOBE7\",\"type\":\"liquidity_pool_deposit\",\"type_i\":22,\"created_at\":\"2026-09-29T12:42:52Z\",\"transaction_hash\":\"5f1005f1aa1b9ab9f5d35c0f087f3bbce64c59f8b7bda40d81a47bfdc889eb45\",\"liquidity_pool_id\":\"a468d41d8e9b8f3c7209651608b74b7db7ac9952dcae0cdf24871d1d9c7b0088\",\"reserves_max\":[{\"amount\":\"1.0000000\"},{\"amount\":\"1.0000000\"}],\"min_price\":\"0.5000000\",\"min_price_r\":{\"n\":1,\"d\":2},\"max_price\":\"2.0000000\",\"max_price_r\":{\"n\":2,\"d\":1},\"reserves_deposited\":[{\"amount\":\"0.0000000\"},{\"amount\":\"0.0000000\"}],\"shares_received\":\"0.0000000\"},{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21184659899551745\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/410dc4fcb7149b5fc30a11742c425d6cbeaecfc55da659424baf4542586b0a94\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21184659899551745/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?ord"
            "er=desc&cursor=21184659899551745\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21184659899551745\"}},\"id\":\"21184659899551745\",\"paging_token\":\"21184659899551745\",\"transaction_successful\":false,\"source_account\":\"GDGPL4QJIJHMW52UBFBIPYCW3HZ7HDRROTS4ZMRMWZSQFPENOHVFOBE7\",\"type\":\"liquidity_pool_withdraw\",\"type_i\":23,\"created_at\":\"2026-09-29T12:42:57Z\",\"transaction_hash\":\"410dc4fcb7149b5fc30a11742c425d6cbeaecfc55da659424baf4542586b0a94\",\"liquidity_pool_id\":\"a468d41d8e9b8f3c7209651608b74b7db7ac9952dcae0cdf24871d1d9c7b0088\",\"reserves_min\":[{\"amount\":\"0.0000000\"},{\"amount\":\"0.0000000\"}],\"shares\":\"1.0000000\",\"reserves_received\":[{\"amount\":\"0.0000000\"},{\"amount\":\"0.0000000\"}]},{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21184664194531329\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/aa98f219711638ff427deeeea6d00e0ed57208691d38ea15e7f35ffebd15d5aa\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/o"
            "perations/21184664194531329/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21184664194531329\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21184664194531329\"}},\"id\":\"21184664194531329\",\"paging_token\":\"21184664194531329\",\"transaction_successful\":true,\"source_account\":\"GDGPL4QJIJHMW52UBFBIPYCW3HZ7HDRROTS4ZMRMWZSQFPENOHVFOBE7\",\"type\":\"manage_data\",\"type_i\":10,\"created_at\":\"2026-09-29T12:43:02Z\",\"transaction_hash\":\"aa98f219711638ff427deeeea6d00e0ed57208691d38ea15e7f35ffebd15d5aa\",\"name\":\"k2\",\"value\":\"dg==\"},{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21184664194531330\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/aa98f219711638ff427deeeea6d00e0ed57208691d38ea15e7f35ffebd15d5aa\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21184664194531330/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor="
            "21184664194531330\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21184664194531330\"}},\"id\":\"21184664194531330\",\"paging_token\":\"21184664194531330\",\"transaction_successful\":true,\"source_account\":\"GDGPL4QJIJHMW52UBFBIPYCW3HZ7HDRROTS4ZMRMWZSQFPENOHVFOBE7\",\"type\":\"manage_data\",\"type_i\":10,\"created_at\":\"2026-09-29T12:43:02Z\",\"transaction_hash\":\"aa98f219711638ff427deeeea6d00e0ed57208691d38ea15e7f35ffebd15d5aa\",\"name\":\"k2\",\"value\":\"\"}]}}");
    }

    /** The same operations as that account's operations stream sent them, byte for byte. */
    static QByteArray failedPoolOperationsStream()
    {
        return QByteArray(
            "retry: 1000\nevent: open\ndata: \"hello\"\n\nid: 21184651309621249\ndata: {\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21184651309621249\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/774bff54682a201c63a92f219424a38bf9be1a1d614641cd48ceb81129c695f9\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21184651309621249/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=21184651309621249\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=21184651309621249\"}},\"id\":\"21184651309621249\",\"paging_token\":\"21184651309621249\",\"transaction_successful\":true,\"source_account\":\"GAIH3ULLFQ4DGSECF2AR555KZ4KNDGEKN4AFI4SU2M7B43MGK3QJZNSR\",\"type\":\"create_account\",\"type_i\":0,\"created_at\":\"2026-09-29T12:42:47Z\",\"transaction_hash\":\"774bff54682a201c63a92f219424a38bf9be1a1d614641cd48ceb81129c695f9\",\"starting_balance\":\"10000.0000000\",\"funder\":\"GAIH3ULLFQ4DGSECF2AR555K"
            "Z4KNDGEKN4AFI4SU2M7B43MGK3QJZNSR\",\"account\":\"GDGPL4QJIJHMW52UBFBIPYCW3HZ7HDRROTS4ZMRMWZSQFPENOHVFOBE7\"}\n\nid: 21184655604584449\ndata: {\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21184655604584449\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/5f1005f1aa1b9ab9f5d35c0f087f3bbce64c59f8b7bda40d81a47bfdc889eb45\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21184655604584449/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=21184655604584449\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=21184655604584449\"}},\"id\":\"21184655604584449\",\"paging_token\":\"21184655604584449\",\"transaction_successful\":false,\"source_account\":\"GDGPL4QJIJHMW52UBFBIPYCW3HZ7HDRROTS4ZMRMWZSQFPENOHVFOBE7\",\"type\":\"manage_data\",\"type_i\":10,\"created_at\":\"2026-09-29T12:42:52Z\",\"transaction_hash\":\"5f1005f1aa1b9ab9f5d35c0f087f3bbce64c59f8b7bda40d81a47bfdc889eb45\",\"nam"
            "e\":\"k\",\"value\":\"dg==\"}\n\nid: 21184655604584450\ndata: {\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21184655604584450\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/5f1005f1aa1b9ab9f5d35c0f087f3bbce64c59f8b7bda40d81a47bfdc889eb45\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21184655604584450/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=21184655604584450\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=21184655604584450\"}},\"id\":\"21184655604584450\",\"paging_token\":\"21184655604584450\",\"transaction_successful\":false,\"source_account\":\"GDGPL4QJIJHMW52UBFBIPYCW3HZ7HDRROTS4ZMRMWZSQFPENOHVFOBE7\",\"type\":\"manage_data\",\"type_i\":10,\"created_at\":\"2026-09-29T12:42:52Z\",\"transaction_hash\":\"5f1005f1aa1b9ab9f5d35c0f087f3bbce64c59f8b7bda40d81a47bfdc889eb45\",\"name\":\"k\",\"value\":\"\"}\n\nid: 21184655604584451\ndata: {\"_links\":{\"self\":{\"href\":\"https:"
            "//horizon-testnet.stellar.org/operations/21184655604584451\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/5f1005f1aa1b9ab9f5d35c0f087f3bbce64c59f8b7bda40d81a47bfdc889eb45\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21184655604584451/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=21184655604584451\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=21184655604584451\"}},\"id\":\"21184655604584451\",\"paging_token\":\"21184655604584451\",\"transaction_successful\":false,\"source_account\":\"GDGPL4QJIJHMW52UBFBIPYCW3HZ7HDRROTS4ZMRMWZSQFPENOHVFOBE7\",\"type\":\"liquidity_pool_deposit\",\"type_i\":22,\"created_at\":\"2026-09-29T12:42:52Z\",\"transaction_hash\":\"5f1005f1aa1b9ab9f5d35c0f087f3bbce64c59f8b7bda40d81a47bfdc889eb45\",\"liquidity_pool_id\":\"a468d41d8e9b8f3c7209651608b74b7db7ac9952dcae0cdf24871d1d9c7b0088\",\"reserves_max\":[{\"amount\":\"1.0000000\"},{\"amount\":\"1.0000000\"}],\"min_pric"
            "e\":\"0.5000000\",\"min_price_r\":{\"n\":1,\"d\":2},\"max_price\":\"2.0000000\",\"max_price_r\":{\"n\":2,\"d\":1},\"reserves_deposited\":[{\"amount\":\"0.0000000\"},{\"amount\":\"0.0000000\"}],\"shares_received\":\"0.0000000\"}\n\nid: 21184659899551745\ndata: {\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21184659899551745\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/410dc4fcb7149b5fc30a11742c425d6cbeaecfc55da659424baf4542586b0a94\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21184659899551745/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=21184659899551745\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=21184659899551745\"}},\"id\":\"21184659899551745\",\"paging_token\":\"21184659899551745\",\"transaction_successful\":false,\"source_account\":\"GDGPL4QJIJHMW52UBFBIPYCW3HZ7HDRROTS4ZMRMWZSQFPENOHVFOBE7\",\"type\":\"liquidity_pool_withdraw\",\"type_i\":23,\"created_at\":\"2026-09-2"
            "9T12:42:57Z\",\"transaction_hash\":\"410dc4fcb7149b5fc30a11742c425d6cbeaecfc55da659424baf4542586b0a94\",\"liquidity_pool_id\":\"a468d41d8e9b8f3c7209651608b74b7db7ac9952dcae0cdf24871d1d9c7b0088\",\"reserves_min\":[{\"amount\":\"0.0000000\"},{\"amount\":\"0.0000000\"}],\"shares\":\"1.0000000\",\"reserves_received\":[{\"amount\":\"0.0000000\"},{\"amount\":\"0.0000000\"}]}\n\nid: 21184664194531329\ndata: {\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21184664194531329\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/aa98f219711638ff427deeeea6d00e0ed57208691d38ea15e7f35ffebd15d5aa\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21184664194531329/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=21184664194531329\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=21184664194531329\"}},\"id\":\"21184664194531329\",\"paging_token\":\"21184664194531329\",\"transaction_successful\":true,\""
            "source_account\":\"GDGPL4QJIJHMW52UBFBIPYCW3HZ7HDRROTS4ZMRMWZSQFPENOHVFOBE7\",\"type\":\"manage_data\",\"type_i\":10,\"created_at\":\"2026-09-29T12:43:02Z\",\"transaction_hash\":\"aa98f219711638ff427deeeea6d00e0ed57208691d38ea15e7f35ffebd15d5aa\",\"name\":\"k2\",\"value\":\"dg==\"}\n\nid: 21184664194531330\ndata: {\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21184664194531330\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/aa98f219711638ff427deeeea6d00e0ed57208691d38ea15e7f35ffebd15d5aa\"},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21184664194531330/effects\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=21184664194531330\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=21184664194531330\"}},\"id\":\"21184664194531330\",\"paging_token\":\"21184664194531330\",\"transaction_successful\":true,\"source_account\":\"GDGPL4QJIJHMW52UBFBIPYCW3HZ7HDRROTS4ZMRMWZSQFPENOHVFOBE7\",\"type\""
            ":\"manage_data\",\"type_i\":10,\"created_at\":\"2026-09-29T12:43:02Z\",\"transaction_hash\":\"aa98f219711638ff427deeeea6d00e0ed57208691d38ea15e7f35ffebd15d5aa\",\"name\":\"k2\",\"value\":\"\"}\n\n");
    }

private slots:

    /**
     * An operation type this SDK does not know (a later protocol's) is kept
     * as a plain OperationResponse: its common members read, getType() the
     * name Horizon sent, json() all of it, as js 17.1.0 and py 14.1.0 keep
     * it. The whole page, operations().operation(id) and each stream event
     * of that type were refused, "unknown operation type 999". It reads no
     * asset, so its asset_type is not checked; the members every operation
     * has are still required.
     */
    void testAnUnknownOperationTypeIsKeptAsABaseRecord()
    {
        const QJsonObject payment = firstRealOperation("payment");
        QJsonObject unknown = payment;
        unknown["type"] = QStringLiteral("future_operation");
        unknown["type_i"] = 999;
        unknown["asset_type"] = QStringLiteral("liquidity_pool");
        unknown["future_member"] = QStringLiteral("x");
        OperationPage page(nullptr);
        page.loadFromJson(pageOf(QJsonArray{payment, unknown}));
        QCOMPARE(page.size(), 2);
        QVERIFY(dynamic_cast<PaymentOperationResponse*>(page.at(0)) != nullptr);
        OperationResponse* kept = page.at(1);
        QVERIFY(kept->metaObject() == &OperationResponse::staticMetaObject);
        QCOMPARE(kept->getType(), QString("future_operation"));
        QCOMPARE(kept->getId(), page.at(0)->getId());
        QCOMPARE(kept->getTransactionHash(), payment.value("transaction_hash").toString());
        QCOMPARE(kept->json().value("type_i").toInt(), 999);
        QCOMPARE(kept->json().value("future_member").toString(), QString("x"));
        QVERIFY(pageRefusal(pageOf(QJsonArray{unknown}), "_embedded.records[0].transaction_hash",
                            QJsonValue(QJsonValue::Undefined)).startsWith("_embedded.records[0].transaction_hash: "));

        GenericOperation one;
        one.loadFromJson(QJsonDocument(unknown).toJson());
        QVERIFY(one.operation()->metaObject() == &OperationResponse::staticMetaObject);
        QCOMPARE(one.operation()->getType(), QString("future_operation"));

        // Streamed: the event is delivered and the stream goes on.
        FakeServer* fakeServer = new FakeServer();
        const QString events = QStringLiteral("id: 1\ndata: %1\n\nid: 2\ndata: %2\n\n")
                .arg(QString::fromUtf8(QJsonDocument(unknown).toJson(QJsonDocument::Compact)),
                     QString::fromUtf8(QJsonDocument(payment).toJson(QJsonDocument::Compact)));
        fakeServer->addStream(QStringLiteral("/operations"), QStringList() << events);
        Server* server = new Server(fakeServer->baseUrl());
        OperationPage* stream = server->operations().stream().execute();
        QStringList types;
        int errors = 0;
        QObject::connect(stream, &Response::ready, [&]() { types << stream->streamedElement()->getType(); });
        QObject::connect(stream, &Response::error, [&]() { ++errors; });
        WAIT_FOR(types.size() + errors < 2)
        QCOMPARE(errors, 0);
        QCOMPARE(types, QStringList() << "future_operation" << "payment");

        delete server;                                  // tears down the stream and its reconnect timer
        fakeServer->deleteLater();
    }

    /**
     * getNextPage() and getPrevPage() request the page _links.next and
     * _links.prev name into the same object, as js 17.1.0's next() / prev()
     * and py 14.1.0's next() / prev() follow those links. There was no
     * getPrevPage(), and getNextPage() on a page with a next link but no
     * reply (read with loadFromJson) crashed; it returns null now, as it does
     * for a page without the link.
     */
    void testNextAndPrevPagesFollowTheLinks()
    {
        FakeServer* fakeServer = new FakeServer();
        const QString base = fakeServer->baseUrl();
        const QJsonObject payment = firstRealOperation("payment");
        auto pageWith = [&](const QString& id, const QString& next, const QString& prev) {
            QJsonObject op = payment;
            op["id"] = id;
            op["paging_token"] = id;
            QJsonObject links{{"self", QJsonObject{{"href", base + "/operations?cursor=&limit=1"}}},
                              {"prev", QJsonObject{{"href", base + prev}}}};
            if (!next.isEmpty())
                links["next"] = QJsonObject{{"href", base + next}};
            return QString::fromUtf8(QJsonDocument(QJsonObject{{"_links", links},
                {"_embedded", QJsonObject{{"records", QJsonArray{op}}}}}).toJson(QJsonDocument::Compact));
        };
        fakeServer->addGet("/operations?limit=1",
                           pageWith("200", "/operations?cursor=200&limit=1", "/operations?cursor=200&limit=1&order=desc"));
        fakeServer->addGet("/operations?cursor=200&limit=1",
                           pageWith("300", QString(), "/operations?cursor=300&limit=1&order=desc"));
        fakeServer->addGet("/operations?cursor=300&limit=1&order=desc",
                           pageWith("200", "/operations?cursor=200&limit=1&order=desc", "/operations?cursor=200&limit=1&order=asc"));
        Server* server = new Server(base);
        OperationPage* page = server->operations().limit(1).execute();
        int ready = 0, errors = 0;
        QObject::connect(page, &Response::ready, [&]() { ++ready; });
        QObject::connect(page, &Response::error, [&]() { ++errors; });
        WAIT_FOR(ready + errors < 1)
        QCOMPARE(errors, 0);
        QCOMPARE(page->at(0)->getId(), Q_INT64_C(200));

        QVERIFY(page->getNextPage() == page);
        WAIT_FOR(ready + errors < 2)
        QCOMPARE(errors, 0);
        QCOMPARE(page->size(), 1);
        QCOMPARE(page->at(0)->getId(), Q_INT64_C(300));
        QVERIFY(page->getLinks().getNext().getHref().isNull());
        QVERIFY(page->getNextPage() == nullptr);        // the last page: no next link

        QVERIFY(page->getPrevPage() == page);
        WAIT_FOR(ready + errors < 3)
        QCOMPARE(errors, 0);
        QCOMPARE(page->at(0)->getId(), Q_INT64_C(200));
        QCOMPARE(page->getLinks().getNext().getHref(), base + "/operations?cursor=200&limit=1&order=desc");

        // Read without a reply: there is nothing to ask through.
        OperationPage offline(nullptr);
        offline.loadFromJson(pageOf(QJsonArray{payment}));
        QVERIFY(!offline.getLinks().getNext().getHref().isNull());
        QVERIFY(offline.getNextPage() == nullptr);
        QVERIFY(offline.getPrevPage() == nullptr);

        delete server;
        fakeServer->deleteLater();
    }
};





ADD_TEST(OperationsPageTest)
#endif // OPERATIONSPAGETEST_H

