#ifndef EFFECTDESERIALIZERTEST_H
#define EFFECTDESERIALIZERTEST_H
#include <QObject>


#include <QtTest>
#include "../testcollector.h"
#include "../../src/responses/effects/effectresponse.h"
#include "../../src/responses/effects/accountcreatedeffectresponse.h"
#include "../../src/responses/effects/accountcreditedeffectresponse.h"
#include "../../src/responses/effects/accountdebitedeffectresponse.h"
#include "../../src/responses/effects/accountflagsupdatedeffectresponse.h"
#include "../../src/responses/effects/accounthomedomainupdatedeffectresponse.h"
#include "../../src/responses/effects/accountremovedeffectresponse.h"
#include "../../src/responses/effects/accountthresholdsupdatedeffectresponse.h"
#include "../../src/responses/effects/accountinflationdestinationupdatedeffectresponse.h"
#include "../../src/responses/effects/offercreatedeffectresponse.h"
#include "../../src/responses/effects/offerremovedeffectresponse.h"
#include "../../src/responses/effects/offerupdatedeffectresponse.h"
#include "../../src/responses/effects/signercreatedeffectresponse.h"
#include "../../src/responses/effects/signereffectresponse.h"
#include "../../src/responses/effects/signerremovedeffectresponse.h"
#include "../../src/responses/effects/signerupdatedeffectresponse.h"
#include "../../src/responses/effects/trustlineauthorizationresponse.h"
#include "../../src/responses/effects/trustlineauthorizedeffectresponse.h"
#include "../../src/responses/effects/trustlinecreatedeffectresponse.h"
#include "../../src/responses/effects/trustlinecudresponse.h"
#include "../../src/responses/effects/trustlinedeauthorizedeffectresponse.h"
#include "../../src/responses/effects/trustlineremovedeffectresponse.h"
#include "../../src/responses/effects/trustlineupdatedeffectresponse.h"
#include "../../src/responses/effects/trustlineauthorizedtomaintainliabilitieseffectresponse.h"
#include "../../src/responses/effects/datacreatedeffectresponse.h"
#include "../../src/responses/effects/dataremovedeffectresponse.h"
#include "../../src/responses/effects/dataupdatedeffectresponse.h"
#include "../../src/responses/effects/tradeeffectresponse.h"
#include "../../src/responses/effects/sequencebumpedeffectresponse.h"
#include "../../src/responses/effects/claimablebalanceclawedbackeffectresponse.h"
#include "../../src/responses/effects/trustlineflagsupdatedeffectresponse.h"
#include "../../src/asset.h"
#include "../../src/assettypenative.h"
#include "../../src/keypair.h"

class EffectDeserializerTest: public QObject
{
    Q_OBJECT

public:


private slots:

    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }
    void testDeserializeAccountCreatedEffect() {
      QByteArray json = "{\n"
              "        \"_links\": {\n"
              "          \"operation\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/operations/65571265847297\"\n"
              "          },\n"
              "          \"succeeds\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=65571265847297-1\"\n"
              "          },\n"
              "          \"precedes\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=65571265847297-1\"\n"
              "          }\n"
              "        },\n"
              "        \"id\": \"0000065571265847297-0000000001\",\n"
              "        \"paging_token\": \"65571265847297-1\",\n"
              "        \"account\": \"GCBQ6JRBPF3SXQBQ6SO5MRBE7WVV4UCHYOSHQGXSZNPZLFRYVYOWBZRQ\",\n"
              "        \"type\": \"account_created\",\n"
              "        \"type_i\": 0,\n"
              "        \"starting_balance\": \"30.0\"\n"
              "      }";

      AccountCreatedEffectResponse effect;
      effect.loadFromJson(json);

      QCOMPARE(effect.getAccount().getAccountId(), QString("GCBQ6JRBPF3SXQBQ6SO5MRBE7WVV4UCHYOSHQGXSZNPZLFRYVYOWBZRQ"));
      QCOMPARE(effect.getStartingBalance(), QString("30.0"));
      QCOMPARE(effect.getPagingToken(), QString("65571265847297-1"));

      QCOMPARE(effect.getLinks().getOperation().getHref(), QString("http://horizon-testnet.stellar.org/operations/65571265847297"));
      QCOMPARE(effect.getLinks().getSucceeds().getHref(), QString("http://horizon-testnet.stellar.org/effects?order=desc&cursor=65571265847297-1"));
      QCOMPARE(effect.getLinks().getPrecedes().getHref(), QString("http://horizon-testnet.stellar.org/effects?order=asc&cursor=65571265847297-1"));
    }


    void testDeserializeAccountRemovedEffect() {
      QByteArray json = "{\n"
              "        \"_links\": {\n"
              "          \"operation\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/operations/65571265847297\"\n"
              "          },\n"
              "          \"succeeds\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=65571265847297-1\"\n"
              "          },\n"
              "          \"precedes\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=65571265847297-1\"\n"
              "          }\n"
              "        },\n"
              "        \"id\": \"0000065571265847297-0000000001\",\n"
              "        \"paging_token\": \"65571265847297-1\",\n"
              "        \"account\": \"GCBQ6JRBPF3SXQBQ6SO5MRBE7WVV4UCHYOSHQGXSZNPZLFRYVYOWBZRQ\",\n"
              "        \"type\": \"account_removed\",\n"
              "        \"type_i\": 1\n"
              "      }";

      AccountRemovedEffectResponse effect;
      effect.loadFromJson(json);

      QCOMPARE(effect.getAccount().getAccountId(), QString("GCBQ6JRBPF3SXQBQ6SO5MRBE7WVV4UCHYOSHQGXSZNPZLFRYVYOWBZRQ"));

      QCOMPARE(effect.getLinks().getOperation().getHref(), QString("http://horizon-testnet.stellar.org/operations/65571265847297"));
      QCOMPARE(effect.getLinks().getSucceeds().getHref(), QString("http://horizon-testnet.stellar.org/effects?order=desc&cursor=65571265847297-1"));
      QCOMPARE(effect.getLinks().getPrecedes().getHref(), QString("http://horizon-testnet.stellar.org/effects?order=asc&cursor=65571265847297-1"));
    }


    void testDeserializeAccountCreditedEffect() {
      QByteArray json = "{\n"
              "        \"_links\": {\n"
              "          \"operation\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/operations/13563506724865\"\n"
              "          },\n"
              "          \"succeeds\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=13563506724865-1\"\n"
              "          },\n"
              "          \"precedes\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=13563506724865-1\"\n"
              "          }\n"
              "        },\n"
              "        \"id\": \"0000013563506724865-0000000001\",\n"
              "        \"paging_token\": \"13563506724865-1\",\n"
              "        \"account\": \"GDLGTRIBFH24364GPWPUS45GUFC2GU4ARPGWTXVCPLGTUHX3IOS3ON47\",\n"
              "        \"type\": \"account_credited\",\n"
              "        \"type_i\": 2,\n"
              "        \"asset_type\": \"native\",\n"
              "        \"amount\": \"1000.0\"\n"
              "      }";

      AccountCreditedEffectResponse effect;
      effect.loadFromJson(json);

      QCOMPARE(effect.getAccount().getAccountId(), QString("GDLGTRIBFH24364GPWPUS45GUFC2GU4ARPGWTXVCPLGTUHX3IOS3ON47"));
      QVERIFY(effect.getAsset()->equals(new AssetTypeNative()));
      QCOMPARE(effect.getAmount(), QString("1000.0"));

      QCOMPARE(effect.getLinks().getOperation().getHref(), QString("http://horizon-testnet.stellar.org/operations/13563506724865"));
      QCOMPARE(effect.getLinks().getSucceeds().getHref(), QString("http://horizon-testnet.stellar.org/effects?order=desc&cursor=13563506724865-1"));
      QCOMPARE(effect.getLinks().getPrecedes().getHref(), QString("http://horizon-testnet.stellar.org/effects?order=asc&cursor=13563506724865-1"));
    }


    void testDeserializeAccountDebitedEffect() {
      QByteArray json = "{\n"
              "        \"_links\": {\n"
              "          \"operation\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/operations/65571265843201\"\n"
              "          },\n"
              "          \"succeeds\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=65571265843201-2\"\n"
              "          },\n"
              "          \"precedes\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=65571265843201-2\"\n"
              "          }\n"
              "        },\n"
              "        \"id\": \"0000065571265843201-0000000002\",\n"
              "        \"paging_token\": \"65571265843201-2\",\n"
              "        \"account\": \"GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H\",\n"
              "        \"type\": \"account_debited\",\n"
              "        \"type_i\": 3,\n"
              "        \"asset_type\": \"native\",\n"
              "        \"amount\": \"30.0\"\n"
              "      }";

      AccountDebitedEffectResponse effect;
      effect.loadFromJson(json);

      QCOMPARE(effect.getAccount().getAccountId(), QString("GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H"));
      QVERIFY(effect.getAsset()->equals(new AssetTypeNative()));
      QCOMPARE(effect.getAmount(), QString("30.0"));

      QCOMPARE(effect.getLinks().getOperation().getHref(), QString("http://horizon-testnet.stellar.org/operations/65571265843201"));
      QCOMPARE(effect.getLinks().getSucceeds().getHref(), QString("http://horizon-testnet.stellar.org/effects?order=desc&cursor=65571265843201-2"));
      QCOMPARE(effect.getLinks().getPrecedes().getHref(), QString("http://horizon-testnet.stellar.org/effects?order=asc&cursor=65571265843201-2"));
    }


    void testDeserializeAccountThresholdsUpdatedEffect() {
      QByteArray json = "{\n"
              "        \"_links\": {\n"
              "          \"operation\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/operations/18970870550529\"\n"
              "          },\n"
              "          \"succeeds\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=18970870550529-1\"\n"
              "          },\n"
              "          \"precedes\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=18970870550529-1\"\n"
              "          }\n"
              "        },\n"
              "        \"id\": \"0000018970870550529-0000000001\",\n"
              "        \"paging_token\": \"18970870550529-1\",\n"
              "        \"account\": \"GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO\",\n"
              "        \"low_threshold\": 2,\n"
              "        \"med_threshold\": 3,\n"
              "        \"high_threshold\": 4,\n"
              "        \"type\": \"account_thresholds_updated\",\n"
              "        \"type_i\": 4\n"
              "      }";

      AccountThresholdsUpdatedEffectResponse effect;
      effect.loadFromJson(json);

      QCOMPARE(effect.getAccount().getAccountId(), QString("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"));
      QCOMPARE(effect.getLowThreshold(), 2);
      QCOMPARE(effect.getMedThreshold(), 3);
      QCOMPARE(effect.getHighThreshold(), 4);

      QCOMPARE(effect.getLinks().getOperation().getHref(), QString("http://horizon-testnet.stellar.org/operations/18970870550529"));
      QCOMPARE(effect.getLinks().getSucceeds().getHref(), QString("http://horizon-testnet.stellar.org/effects?order=desc&cursor=18970870550529-1"));
      QCOMPARE(effect.getLinks().getPrecedes().getHref(), QString("http://horizon-testnet.stellar.org/effects?order=asc&cursor=18970870550529-1"));
    }


    void testDeserializeAccountHomeDomainUpdatedEffect() {
      QByteArray json = "{\n"
              "        \"_links\": {\n"
              "          \"operation\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/operations/18970870550529\"\n"
              "          },\n"
              "          \"succeeds\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=18970870550529-1\"\n"
              "          },\n"
              "          \"precedes\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=18970870550529-1\"\n"
              "          }\n"
              "        },\n"
              "        \"id\": \"0000018970870550529-0000000001\",\n"
              "        \"paging_token\": \"18970870550529-1\",\n"
              "        \"account\": \"GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO\",\n"
              "        \"type\": \"account_home_domain_updated\",\n"
              "        \"type_i\": 5,\n"
              "        \"home_domain\": \"stellar.org\"\n"
              "      }";

      AccountHomeDomainUpdatedEffectResponse effect;
      effect.loadFromJson(json);

      QCOMPARE(effect.getAccount().getAccountId(), QString("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"));
      QCOMPARE(effect.getHomeDomain(),QString("stellar.org"));

      QCOMPARE(effect.getLinks().getOperation().getHref(), QString("http://horizon-testnet.stellar.org/operations/18970870550529"));
      QCOMPARE(effect.getLinks().getSucceeds().getHref(), QString("http://horizon-testnet.stellar.org/effects?order=desc&cursor=18970870550529-1"));
      QCOMPARE(effect.getLinks().getPrecedes().getHref(), QString("http://horizon-testnet.stellar.org/effects?order=asc&cursor=18970870550529-1"));
    }


    void testDeserializeAccountFlagsUpdatedEffect() {
      QByteArray json = "{\n"
              "        \"_links\": {\n"
              "          \"operation\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/operations/18970870550529\"\n"
              "          },\n"
              "          \"succeeds\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=18970870550529-1\"\n"
              "          },\n"
              "          \"precedes\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=18970870550529-1\"\n"
              "          }\n"
              "        },\n"
              "        \"id\": \"0000018970870550529-0000000001\",\n"
              "        \"paging_token\": \"18970870550529-1\",\n"
              "        \"account\": \"GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO\",\n"
              "        \"type\": \"account_flags_updated\",\n"
              "        \"type_i\": 6,\n"
              "        \"auth_required_flag\": false,\n"
              "        \"auth_revokable_flag\": true\n"
              "      }";

      AccountFlagsUpdatedEffectResponse effect;
      effect.loadFromJson(json);

      QCOMPARE(effect.getAccount().getAccountId(), QString("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"));
      QCOMPARE(effect.getAuthRequiredFlag(),false);
      QCOMPARE(effect.getAuthRevokableFlag(), true);

      QCOMPARE(effect.getLinks().getOperation().getHref(), QString("http://horizon-testnet.stellar.org/operations/18970870550529"));
      QCOMPARE(effect.getLinks().getSucceeds().getHref(), QString("http://horizon-testnet.stellar.org/effects?order=desc&cursor=18970870550529-1"));
      QCOMPARE(effect.getLinks().getPrecedes().getHref(), QString("http://horizon-testnet.stellar.org/effects?order=asc&cursor=18970870550529-1"));
    }

    void testDeserializeAccountInflationDestinationUpdatedEffect() {
        QByteArray json = "{\n"
                    "        \"_links\": {\n"
                    "          \"operation\": {\n"
                    "            \"href\": \"https://horizon-testnet.stellar.org/operations/40181321724596225\"\n"
                    "          },\n"
                    "          \"succeeds\": {\n"
                    "            \"href\": \"https://horizon-testnet.stellar.org/effects?order=desc&cursor=40181321724596225-1\"\n"
                    "          },\n"
                    "          \"precedes\": {\n"
                    "            \"href\": \"https://horizon-testnet.stellar.org/effects?order=asc&cursor=40181321724596225-1\"\n"
                    "          }\n"
                    "        },\n"
                    "        \"id\": \"0040181321724596225-0000000001\",\n"
                    "        \"paging_token\": \"40181321724596225-1\",\n"
                    "        \"account\": \"GDPFGP4IPE5DXG6XRXC4ZBUI43PAGRQ5VVNJ3LJTBXDBZ4ITO6HBHNSF\",\n"
                    "        \"type\": \"account_inflation_destination_updated\",\n"
                    "        \"type_i\": 7,\n"
                    "        \"created_at\": \"2018-06-06T10:20:50Z\"\n"
                    "      }";
        AccountInflationDestinationUpdatedEffectResponse effect;
        effect.loadFromJson(json);

        QCOMPARE(effect.getAccount().getAccountId(), QString("GDPFGP4IPE5DXG6XRXC4ZBUI43PAGRQ5VVNJ3LJTBXDBZ4ITO6HBHNSF"));
        QCOMPARE(effect.getCreatedAt(), QString("2018-06-06T10:20:50Z"));
    }

    void testDeserializeSignerCreatedEffect() {
      QByteArray json = "{\n"
              "        \"_links\": {\n"
              "          \"operation\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/operations/65571265859585\"\n"
              "          },\n"
              "          \"succeeds\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=65571265859585-3\"\n"
              "          },\n"
              "          \"precedes\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=65571265859585-3\"\n"
              "          }\n"
              "        },\n"
              "        \"id\": \"0000065571265859585-0000000003\",\n"
              "        \"paging_token\": \"65571265859585-3\",\n"
              "        \"account\": \"GB24LPGAHYTWRYOXIDKXLI55SBRWW42T3TZKDAAW3BOJX4ADVIATFTLU\",\n"
              "        \"type\": \"signer_created\",\n"
              "        \"type_i\": 10,\n"
              "        \"weight\": 1,\n"
              "        \"public_key\": \"GB24LPGAHYTWRYOXIDKXLI55SBRWW42T3TZKDAAW3BOJX4ADVIATFTLU\"\n"
              "      }";

      SignerCreatedEffectResponse effect;
      effect.loadFromJson(json);

      QCOMPARE(effect.getAccount().getAccountId(), QString("GB24LPGAHYTWRYOXIDKXLI55SBRWW42T3TZKDAAW3BOJX4ADVIATFTLU"));
      QCOMPARE(effect.getWeight(), 1);
      QCOMPARE(effect.getPublicKey(),QString("GB24LPGAHYTWRYOXIDKXLI55SBRWW42T3TZKDAAW3BOJX4ADVIATFTLU"));

      QCOMPARE(effect.getLinks().getOperation().getHref(), QString("http://horizon-testnet.stellar.org/operations/65571265859585"));
      QCOMPARE(effect.getLinks().getSucceeds().getHref(), QString("http://horizon-testnet.stellar.org/effects?order=desc&cursor=65571265859585-3"));
      QCOMPARE(effect.getLinks().getPrecedes().getHref(), QString("http://horizon-testnet.stellar.org/effects?order=asc&cursor=65571265859585-3"));
    }


    void testDeserializeSignerRemovedEffect() {
      QByteArray json = "{\n"
              "        \"_links\": {\n"
              "          \"operation\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/operations/43658342567940\"\n"
              "          },\n"
              "          \"succeeds\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=43658342567940-2\"\n"
              "          },\n"
              "          \"precedes\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=43658342567940-2\"\n"
              "          }\n"
              "        },\n"
              "        \"id\": \"0000043658342567940-0000000002\",\n"
              "        \"paging_token\": \"43658342567940-2\",\n"
              "        \"account\": \"GCFKT6BN2FEASCEVDNHEC4LLFT2KLUUPEMKM4OJPEJ65H2AEZ7IH4RV6\",\n"
              "        \"type\": \"signer_removed\",\n"
              "        \"type_i\": 11,\n"
              "        \"weight\": 0,\n"
              "        \"public_key\": \"GCFKT6BN2FEASCEVDNHEC4LLFT2KLUUPEMKM4OJPEJ65H2AEZ7IH4RV6\"\n"
              "      }";

      SignerRemovedEffectResponse effect;
      effect.loadFromJson(json);

      QCOMPARE(effect.getAccount().getAccountId(), QString("GCFKT6BN2FEASCEVDNHEC4LLFT2KLUUPEMKM4OJPEJ65H2AEZ7IH4RV6"));
      QCOMPARE(effect.getWeight(), 0);
      QCOMPARE(effect.getPublicKey(), QString("GCFKT6BN2FEASCEVDNHEC4LLFT2KLUUPEMKM4OJPEJ65H2AEZ7IH4RV6"));

      QCOMPARE(effect.getLinks().getOperation().getHref(), QString("http://horizon-testnet.stellar.org/operations/43658342567940"));
      QCOMPARE(effect.getLinks().getSucceeds().getHref(), QString("http://horizon-testnet.stellar.org/effects?order=desc&cursor=43658342567940-2"));
      QCOMPARE(effect.getLinks().getPrecedes().getHref(), QString("http://horizon-testnet.stellar.org/effects?order=asc&cursor=43658342567940-2"));
    }


    void testDeserializeSignerUpdatedEffect() {
      QByteArray json = "{\n"
              "        \"_links\": {\n"
              "          \"operation\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/operations/33788507721730\"\n"
              "          },\n"
              "          \"succeeds\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=33788507721730-2\"\n"
              "          },\n"
              "          \"precedes\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=33788507721730-2\"\n"
              "          }\n"
              "        },\n"
              "        \"id\": \"0000033788507721730-0000000002\",\n"
              "        \"paging_token\": \"33788507721730-2\",\n"
              "        \"account\": \"GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO\",\n"
              "        \"type\": \"signer_updated\",\n"
              "        \"type_i\": 12,\n"
              "        \"weight\": 2,\n"
              "        \"public_key\": \"GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO\"\n"
              "      }";

      SignerUpdatedEffectResponse effect;
      effect.loadFromJson(json);

      QCOMPARE(effect.getAccount().getAccountId(), QString("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"));
      QCOMPARE(effect.getWeight(), 2);
      QCOMPARE(effect.getPublicKey(), QString("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"));

      QCOMPARE(effect.getLinks().getOperation().getHref(), QString("http://horizon-testnet.stellar.org/operations/33788507721730"));
      QCOMPARE(effect.getLinks().getSucceeds().getHref(), QString("http://horizon-testnet.stellar.org/effects?order=desc&cursor=33788507721730-2"));
      QCOMPARE(effect.getLinks().getPrecedes().getHref(), QString("http://horizon-testnet.stellar.org/effects?order=asc&cursor=33788507721730-2"));
    }


    void testDeserializeTrustlineCreatedEffect() {
      QByteArray json = "{\n"
              "        \"_links\": {\n"
              "          \"operation\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/operations/33788507721730\"\n"
              "          },\n"
              "          \"succeeds\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=33788507721730-2\"\n"
              "          },\n"
              "          \"precedes\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=33788507721730-2\"\n"
              "          }\n"
              "        },\n"
              "        \"id\": \"0000033788507721730-0000000002\",\n"
              "        \"paging_token\": \"33788507721730-2\",\n"
              "        \"account\": \"GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO\",\n"
              "        \"type\": \"trustline_created\",\n"
              "        \"type_i\": 20,\n"
              "        \"asset_type\": \"credit_alphanum4\",\n"
              "        \"asset_code\": \"EUR\",\n"
              "        \"asset_issuer\": \"GAZN3PPIDQCSP5JD4ETQQQ2IU2RMFYQTAL4NNQZUGLLO2XJJJ3RDSDGA\",\n"
              "        \"limit\": \"1000.0\"\n"
              "      }";

      TrustlineCreatedEffectResponse effect;
      effect.loadFromJson(json);

      QCOMPARE(effect.getAccount().getAccountId(), QString("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"));
      QVERIFY(effect.getAsset()->equals(Asset::createNonNativeAsset("EUR", KeyPair::fromAccountId(QString("GAZN3PPIDQCSP5JD4ETQQQ2IU2RMFYQTAL4NNQZUGLLO2XJJJ3RDSDGA")))));
      QCOMPARE(effect.getLimit(), QString("1000.0"));

      QCOMPARE(effect.getLinks().getOperation().getHref(), QString("http://horizon-testnet.stellar.org/operations/33788507721730"));
      QCOMPARE(effect.getLinks().getSucceeds().getHref(), QString("http://horizon-testnet.stellar.org/effects?order=desc&cursor=33788507721730-2"));
      QCOMPARE(effect.getLinks().getPrecedes().getHref(), QString("http://horizon-testnet.stellar.org/effects?order=asc&cursor=33788507721730-2"));
    }


    void testDeserializeTrustlineRemovedEffect() {
      QByteArray json = "{\n"
              "        \"_links\": {\n"
              "          \"operation\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/operations/33788507721730\"\n"
              "          },\n"
              "          \"succeeds\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=33788507721730-2\"\n"
              "          },\n"
              "          \"precedes\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=33788507721730-2\"\n"
              "          }\n"
              "        },\n"
              "        \"id\": \"0000033788507721730-0000000002\",\n"
              "        \"paging_token\": \"33788507721730-2\",\n"
              "        \"account\": \"GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO\",\n"
              "        \"type\": \"trustline_removed\",\n"
              "        \"type_i\": 21,\n"
              "        \"asset_type\": \"credit_alphanum4\",\n"
              "        \"asset_code\": \"EUR\",\n"
              "        \"asset_issuer\": \"GAZN3PPIDQCSP5JD4ETQQQ2IU2RMFYQTAL4NNQZUGLLO2XJJJ3RDSDGA\",\n"
              "        \"limit\": \"0.0\"\n"
              "      }";

      TrustlineRemovedEffectResponse effect;
      effect.loadFromJson(json);

      QCOMPARE(effect.getAccount().getAccountId(), QString("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"));
      QVERIFY(effect.getAsset()->equals(Asset::createNonNativeAsset("EUR", KeyPair::fromAccountId(QString("GAZN3PPIDQCSP5JD4ETQQQ2IU2RMFYQTAL4NNQZUGLLO2XJJJ3RDSDGA")))));
      QCOMPARE(effect.getLimit(), QString("0.0"));

      QCOMPARE(effect.getLinks().getOperation().getHref(), QString("http://horizon-testnet.stellar.org/operations/33788507721730"));
      QCOMPARE(effect.getLinks().getSucceeds().getHref(), QString("http://horizon-testnet.stellar.org/effects?order=desc&cursor=33788507721730-2"));
      QCOMPARE(effect.getLinks().getPrecedes().getHref(), QString("http://horizon-testnet.stellar.org/effects?order=asc&cursor=33788507721730-2"));
    }


    void testDeserializeTrustlineUpdatedEffect() {
      QByteArray json = "{\n"
              "        \"_links\": {\n"
              "          \"operation\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/operations/33788507721730\"\n"
              "          },\n"
              "          \"succeeds\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=33788507721730-2\"\n"
              "          },\n"
              "          \"precedes\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=33788507721730-2\"\n"
              "          }\n"
              "        },\n"
              "        \"id\": \"0000033788507721730-0000000002\",\n"
              "        \"paging_token\": \"33788507721730-2\",\n"
              "        \"account\": \"GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO\",\n"
              "        \"type\": \"trustline_updated\",\n"
              "        \"type_i\": 22,\n"
              "        \"asset_type\": \"credit_alphanum12\",\n"
              "        \"asset_code\": \"TESTTEST\",\n"
              "        \"asset_issuer\": \"GAZN3PPIDQCSP5JD4ETQQQ2IU2RMFYQTAL4NNQZUGLLO2XJJJ3RDSDGA\",\n"
              "        \"limit\": \"100.0\"\n"
              "      }";

      TrustlineUpdatedEffectResponse effect;
      effect.loadFromJson(json);

      QCOMPARE(effect.getAccount().getAccountId(), QString("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"));
      QVERIFY(effect.getAsset()->equals(Asset::createNonNativeAsset("TESTTEST", KeyPair::fromAccountId(QString("GAZN3PPIDQCSP5JD4ETQQQ2IU2RMFYQTAL4NNQZUGLLO2XJJJ3RDSDGA")))));
      QCOMPARE(effect.getLimit(), QString("100.0"));

      QCOMPARE(effect.getLinks().getOperation().getHref(), QString("http://horizon-testnet.stellar.org/operations/33788507721730"));
      QCOMPARE(effect.getLinks().getSucceeds().getHref(), QString("http://horizon-testnet.stellar.org/effects?order=desc&cursor=33788507721730-2"));
      QCOMPARE(effect.getLinks().getPrecedes().getHref(), QString("http://horizon-testnet.stellar.org/effects?order=asc&cursor=33788507721730-2"));
    }


    void testDeserializeTrustlineAuthorizedEffect() {
      QByteArray json = "{\n"
              "        \"_links\": {\n"
              "          \"operation\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/operations/33788507721730\"\n"
              "          },\n"
              "          \"succeeds\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=33788507721730-2\"\n"
              "          },\n"
              "          \"precedes\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=33788507721730-2\"\n"
              "          }\n"
              "        },\n"
              "        \"id\": \"0000033788507721730-0000000002\",\n"
              "        \"paging_token\": \"33788507721730-2\",\n"
              "        \"account\": \"GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO\",\n"
              "        \"type\": \"trustline_authorized\",\n"
              "        \"type_i\": 23,\n"
              "        \"asset_type\": \"credit_alphanum12\",\n"
              "        \"asset_code\": \"TESTTEST\",\n"
              "        \"trustor\": \"GB3E4AB4VWXJDUVN4Z3CPBU5HTMWVEQXONZYVDFMHQD6333KHCOL3UBR\"\n"
              "      }";

      TrustlineAuthorizedEffectResponse effect;
      effect.loadFromJson(json);

      QCOMPARE(effect.getAccount().getAccountId(), QString("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"));
      QCOMPARE(effect.getAssetType(), QString("credit_alphanum12"));
      QCOMPARE(effect.getAssetCode(), QString("TESTTEST"));
      QCOMPARE(effect.getTrustor().getAccountId(), QString("GB3E4AB4VWXJDUVN4Z3CPBU5HTMWVEQXONZYVDFMHQD6333KHCOL3UBR"));

      QCOMPARE(effect.getLinks().getOperation().getHref(), QString("http://horizon-testnet.stellar.org/operations/33788507721730"));
      QCOMPARE(effect.getLinks().getSucceeds().getHref(), QString("http://horizon-testnet.stellar.org/effects?order=desc&cursor=33788507721730-2"));
      QCOMPARE(effect.getLinks().getPrecedes().getHref(), QString("http://horizon-testnet.stellar.org/effects?order=asc&cursor=33788507721730-2"));
    }


    void testDeserializeTrustlineDeauthorizedEffect() {
      QByteArray json = "{\n"
              "        \"_links\": {\n"
              "          \"operation\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/operations/33788507721730\"\n"
              "          },\n"
              "          \"succeeds\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=33788507721730-2\"\n"
              "          },\n"
              "          \"precedes\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=33788507721730-2\"\n"
              "          }\n"
              "        },\n"
              "        \"id\": \"0000033788507721730-0000000002\",\n"
              "        \"paging_token\": \"33788507721730-2\",\n"
              "        \"account\": \"GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO\",\n"
              "        \"type\": \"trustline_deauthorized\",\n"
              "        \"type_i\": 24,\n"
              "        \"asset_type\": \"credit_alphanum4\",\n"
              "        \"asset_code\": \"EUR\",\n"
              "        \"trustor\": \"GB3E4AB4VWXJDUVN4Z3CPBU5HTMWVEQXONZYVDFMHQD6333KHCOL3UBR\"\n"
              "      }";

      TrustlineDeauthorizedEffectResponse effect;
      effect.loadFromJson(json);

      QCOMPARE(effect.getAccount().getAccountId(), QString("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"));
      QCOMPARE(effect.getAssetType(), QString("credit_alphanum4"));
      QCOMPARE(effect.getAssetCode(), QString("EUR"));
      QCOMPARE(effect.getTrustor().getAccountId(), QString("GB3E4AB4VWXJDUVN4Z3CPBU5HTMWVEQXONZYVDFMHQD6333KHCOL3UBR"));

      QCOMPARE(effect.getLinks().getOperation().getHref(), QString("http://horizon-testnet.stellar.org/operations/33788507721730"));
      QCOMPARE(effect.getLinks().getSucceeds().getHref(),QString("http://horizon-testnet.stellar.org/effects?order=desc&cursor=33788507721730-2"));
      QCOMPARE(effect.getLinks().getPrecedes().getHref(), QString("http://horizon-testnet.stellar.org/effects?order=asc&cursor=33788507721730-2"));
    }


    void testDeserializeTradeEffect() {
      QByteArray json = "{\n"
              "        \"_links\": {\n"
              "          \"operation\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/operations/33788507721730\"\n"
              "          },\n"
              "          \"succeeds\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=33788507721730-2\"\n"
              "          },\n"
              "          \"precedes\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=33788507721730-2\"\n"
              "          }\n"
              "        },\n"
              "        \"id\": \"0000033788507721730-0000000002\",\n"
              "        \"paging_token\": \"33788507721730-2\",\n"
              "        \"account\": \"GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO\",\n"
              "        \"type\": \"trade\",\n"
              "        \"type_i\": 33,\n"
              "        \"seller\": \"GCVHDLN6EHZBYW2M3BQIY32C23E4GPIRZZDBNF2Q73DAZ5VJDRGSMYRB\",\n"
              "        \"offer_id\": 1,\n"
              "        \"sold_amount\": \"1000.0\",\n"
              "        \"sold_asset_type\": \"credit_alphanum4\",\n"
              "        \"sold_asset_code\": \"EUR\",\n"
              "        \"sold_asset_issuer\": \"GCWVFBJ24754I5GXG4JOEB72GJCL3MKWC7VAEYWKGQHPVH3ENPNBSKWS\",\n"
              "        \"bought_amount\": \"60.0\",\n"
              "        \"bought_asset_type\": \"credit_alphanum12\",\n"
              "        \"bought_asset_code\": \"TESTTEST\",\n"
              "        \"bought_asset_issuer\": \"GAHXPUDP3AK6F2QQM4FIRBGPNGKLRDDSTQCVKEXXKKRHJZUUQ23D5BU7\"\n"
              "      }";

      TradeEffectResponse effect;
      effect.loadFromJson(json);

      QCOMPARE(effect.getAccount().getAccountId(), QString("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"));
      QCOMPARE(effect.getSeller().getAccountId(), QString("GCVHDLN6EHZBYW2M3BQIY32C23E4GPIRZZDBNF2Q73DAZ5VJDRGSMYRB"));
      QCOMPARE(effect.getOfferId(), 1);
      QCOMPARE(effect.getSoldAmount(), QString("1000.0"));
      QVERIFY(effect.getSoldAsset()->equals(Asset::createNonNativeAsset("EUR", KeyPair::fromAccountId(QString("GCWVFBJ24754I5GXG4JOEB72GJCL3MKWC7VAEYWKGQHPVH3ENPNBSKWS")))));
      QCOMPARE(effect.getBoughtAmount(), QString("60.0"));
      QVERIFY(effect.getBoughtAsset()->equals(Asset::createNonNativeAsset("TESTTEST", KeyPair::fromAccountId(QString("GAHXPUDP3AK6F2QQM4FIRBGPNGKLRDDSTQCVKEXXKKRHJZUUQ23D5BU7")))));

      QCOMPARE(effect.getLinks().getOperation().getHref(), QString("http://horizon-testnet.stellar.org/operations/33788507721730"));
      QCOMPARE(effect.getLinks().getSucceeds().getHref(), QString("http://horizon-testnet.stellar.org/effects?order=desc&cursor=33788507721730-2"));
      QCOMPARE(effect.getLinks().getPrecedes().getHref(), QString("http://horizon-testnet.stellar.org/effects?order=asc&cursor=33788507721730-2"));

  }

    void testDeserializeTradeEffectOfferIDString() {
      QByteArray json = "{\n"
              "        \"_links\": {\n"
              "          \"operation\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/operations/33788507721730\"\n"
              "          },\n"
              "          \"succeeds\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=33788507721730-2\"\n"
              "          },\n"
              "          \"precedes\": {\n"
              "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=33788507721730-2\"\n"
              "          }\n"
              "        },\n"
              "        \"id\": \"0000033788507721730-0000000002\",\n"
              "        \"paging_token\": \"33788507721730-2\",\n"
              "        \"account\": \"GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO\",\n"
              "        \"type\": \"trade\",\n"
              "        \"type_i\": 33,\n"
              "        \"seller\": \"GCVHDLN6EHZBYW2M3BQIY32C23E4GPIRZZDBNF2Q73DAZ5VJDRGSMYRB\",\n"
              "        \"offer_id\": \"2\",\n"
              "        \"sold_amount\": \"1000.0\",\n"
              "        \"sold_asset_type\": \"credit_alphanum4\",\n"
              "        \"sold_asset_code\": \"EUR\",\n"
              "        \"sold_asset_issuer\": \"GCWVFBJ24754I5GXG4JOEB72GJCL3MKWC7VAEYWKGQHPVH3ENPNBSKWS\",\n"
              "        \"bought_amount\": \"60.0\",\n"
              "        \"bought_asset_type\": \"credit_alphanum12\",\n"
              "        \"bought_asset_code\": \"TESTTEST\",\n"
              "        \"bought_asset_issuer\": \"GAHXPUDP3AK6F2QQM4FIRBGPNGKLRDDSTQCVKEXXKKRHJZUUQ23D5BU7\"\n"
              "      }";

      TradeEffectResponse effect;
      effect.loadFromJson(json);
      QCOMPARE(effect.getAccount().getAccountId(), QString("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"));
      QCOMPARE(effect.getSeller().getAccountId(), QString("GCVHDLN6EHZBYW2M3BQIY32C23E4GPIRZZDBNF2Q73DAZ5VJDRGSMYRB"));
      QCOMPARE(effect.getOfferId(), 2);
      QCOMPARE(effect.getSoldAmount(), QString("1000.0"));
      QVERIFY(effect.getSoldAsset()->equals(Asset::createNonNativeAsset("EUR", KeyPair::fromAccountId(QString("GCWVFBJ24754I5GXG4JOEB72GJCL3MKWC7VAEYWKGQHPVH3ENPNBSKWS")))));
      QCOMPARE(effect.getBoughtAmount(), QString("60.0"));
      QVERIFY(effect.getBoughtAsset()->equals(Asset::createNonNativeAsset("TESTTEST", KeyPair::fromAccountId(QString("GAHXPUDP3AK6F2QQM4FIRBGPNGKLRDDSTQCVKEXXKKRHJZUUQ23D5BU7")))));

      QCOMPARE(effect.getLinks().getOperation().getHref(), QString("http://horizon-testnet.stellar.org/operations/33788507721730"));
      QCOMPARE(effect.getLinks().getSucceeds().getHref(), QString("http://horizon-testnet.stellar.org/effects?order=desc&cursor=33788507721730-2"));
      QCOMPARE(effect.getLinks().getPrecedes().getHref(), QString("http://horizon-testnet.stellar.org/effects?order=asc&cursor=33788507721730-2"));

  }

      void testDeserializeDataCreatedEffect() {
        QByteArray json = "{\n"
                "        \"_links\": {\n"
                "          \"operation\": {\n"
                "            \"href\": \"https://horizon-testnet.stellar.org/operations/40181480638386177\"\n"
                "          },\n"
                "          \"succeeds\": {\n"
                "            \"href\": \"https://horizon-testnet.stellar.org/effects?order=desc&cursor=40181480638386177-1\"\n"
                "          },\n"
                "          \"precedes\": {\n"
                "            \"href\": \"https://horizon-testnet.stellar.org/effects?order=asc&cursor=40181480638386177-1\"\n"
                "          }\n"
                "        },\n"
                "        \"id\": \"0040181480638386177-0000000001\",\n"
                "        \"paging_token\": \"40181480638386177-1\",\n"
                "        \"account\": \"GDPFGP4IPE5DXG6XRXC4ZBUI43PAGRQ5VVNJ3LJTBXDBZ4ITO6HBHNSF\",\n"
                "        \"type\": \"data_created\",\n"
                "        \"type_i\": 40,\n"
                "        \"created_at\": \"2018-06-06T10:23:57Z\"\n"
                "      }";

        DataCreatedEffectResponse effect;
        effect.loadFromJson(json);
        QCOMPARE(effect.getAccount().getAccountId(), QString("GDPFGP4IPE5DXG6XRXC4ZBUI43PAGRQ5VVNJ3LJTBXDBZ4ITO6HBHNSF"));
        QCOMPARE(effect.getCreatedAt(), QString("2018-06-06T10:23:57Z"));
      }

      void testDeserializeDataRemovedEffect() {
        QByteArray json = "{\n"
                "        \"_links\": {\n"
                "          \"operation\": {\n"
                "            \"href\": \"https://horizon-testnet.stellar.org/operations/40181480638386177\"\n"
                "          },\n"
                "          \"succeeds\": {\n"
                "            \"href\": \"https://horizon-testnet.stellar.org/effects?order=desc&cursor=40181480638386177-1\"\n"
                "          },\n"
                "          \"precedes\": {\n"
                "            \"href\": \"https://horizon-testnet.stellar.org/effects?order=asc&cursor=40181480638386177-1\"\n"
                "          }\n"
                "        },\n"
                "        \"id\": \"0040181480638386177-0000000001\",\n"
                "        \"paging_token\": \"40181480638386177-1\",\n"
                "        \"account\": \"GDPFGP4IPE5DXG6XRXC4ZBUI43PAGRQ5VVNJ3LJTBXDBZ4ITO6HBHNSF\",\n"
                "        \"type\": \"data_removed\",\n"
                "        \"type_i\": 41,\n"
                "        \"created_at\": \"2018-06-06T10:23:57Z\"\n"
                "      }";

        DataRemovedEffectResponse effect;
        effect.loadFromJson(json);

        QCOMPARE(effect.getAccount().getAccountId(), QString("GDPFGP4IPE5DXG6XRXC4ZBUI43PAGRQ5VVNJ3LJTBXDBZ4ITO6HBHNSF"));
        QCOMPARE(effect.getCreatedAt(), QString("2018-06-06T10:23:57Z"));
      }

      void testDeserializeDataUpdatedEffect() {
        QByteArray json = "{\n"
                "        \"_links\": {\n"
                "          \"operation\": {\n"
                "            \"href\": \"https://horizon-testnet.stellar.org/operations/40181480638386177\"\n"
                "          },\n"
                "          \"succeeds\": {\n"
                "            \"href\": \"https://horizon-testnet.stellar.org/effects?order=desc&cursor=40181480638386177-1\"\n"
                "          },\n"
                "          \"precedes\": {\n"
                "            \"href\": \"https://horizon-testnet.stellar.org/effects?order=asc&cursor=40181480638386177-1\"\n"
                "          }\n"
                "        },\n"
                "        \"id\": \"0040181480638386177-0000000001\",\n"
                "        \"paging_token\": \"40181480638386177-1\",\n"
                "        \"account\": \"GDPFGP4IPE5DXG6XRXC4ZBUI43PAGRQ5VVNJ3LJTBXDBZ4ITO6HBHNSF\",\n"
                "        \"type\": \"data_updated\",\n"
                "        \"type_i\": 42,\n"
                "        \"created_at\": \"2018-06-06T10:23:57Z\"\n"
                "      }";

        DataUpdatedEffectResponse effect;
        effect.loadFromJson(json);

        QCOMPARE(effect.getAccount().getAccountId(), QString("GDPFGP4IPE5DXG6XRXC4ZBUI43PAGRQ5VVNJ3LJTBXDBZ4ITO6HBHNSF"));
        QCOMPARE(effect.getCreatedAt(), QString("2018-06-06T10:23:57Z"));
    }

      void testDeserializeTrustlineAuthorizedToMaintainLiabilitiesEffect() {
        QByteArray json = "{\n"
            "        \"_links\": {\n"
            "          \"operation\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/operations/33788507721730\"\n"
            "          },\n"
            "          \"succeeds\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=desc\\u0026cursor=33788507721730-2\"\n"
            "          },\n"
            "          \"precedes\": {\n"
            "            \"href\": \"http://horizon-testnet.stellar.org/effects?order=asc\\u0026cursor=33788507721730-2\"\n"
            "          }\n"
            "        },\n"
            "        \"id\": \"0000033788507721730-0000000002\",\n"
            "        \"paging_token\": \"33788507721730-2\",\n"
            "        \"account\": \"GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO\",\n"
            "        \"type\": \"trustline_authorized_to_maintain_liabilities\",\n"
            "        \"type_i\": 25,\n"
            "        \"asset_type\": \"credit_alphanum12\",\n"
            "        \"asset_code\": \"TESTTEST\",\n"
            "        \"trustor\": \"GB3E4AB4VWXJDUVN4Z3CPBU5HTMWVEQXONZYVDFMHQD6333KHCOL3UBR\"\n"
            "      }";

        TrustlineAuthorizedToMaintainLiabilitiesEffectResponse effect;
        effect.loadFromJson(json);

        QCOMPARE(effect.getAccount().getAccountId(), "GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO");
        QCOMPARE(effect.getAssetType(), "credit_alphanum12");
        QCOMPARE(effect.getAssetCode(), "TESTTEST");
        QCOMPARE(effect.getTrustor().getAccountId(), "GB3E4AB4VWXJDUVN4Z3CPBU5HTMWVEQXONZYVDFMHQD6333KHCOL3UBR");

        QCOMPARE(effect.getLinks().getOperation().getHref(), "http://horizon-testnet.stellar.org/operations/33788507721730");
        QCOMPARE(effect.getLinks().getSucceeds().getHref(), "http://horizon-testnet.stellar.org/effects?order=desc&cursor=33788507721730-2");
        QCOMPARE(effect.getLinks().getPrecedes().getHref(), "http://horizon-testnet.stellar.org/effects?order=asc&cursor=33788507721730-2");
      }


      void testDeserializeSequenceBumpedEffect() {
        QByteArray json = "{\n"
                "        \"_links\": {\n"
                "          \"operation\": {\n"
                "            \"href\": \"https://horizon-testnet.stellar.org/operations/40181480638386177\"\n"
                "          },\n"
                "          \"succeeds\": {\n"
                "            \"href\": \"https://horizon-testnet.stellar.org/effects?order=desc&cursor=40181480638386177-1\"\n"
                "          },\n"
                "          \"precedes\": {\n"
                "            \"href\": \"https://horizon-testnet.stellar.org/effects?order=asc&cursor=40181480638386177-1\"\n"
                "          }\n"
                "        },\n"
                "        \"id\": \"0040181480638386177-0000000001\",\n"
                "        \"paging_token\": \"40181480638386177-1\",\n"
                "        \"account\": \"GDPFGP4IPE5DXG6XRXC4ZBUI43PAGRQ5VVNJ3LJTBXDBZ4ITO6HBHNSF\",\n"
                "        \"type\": \"sequence_bumped\",\n"
                "        \"type_i\": 43,\n"
                "        \"new_seq\": \"79473726952833048\",\n"
                "        \"created_at\": \"2018-06-06T10:23:57Z\"\n"
                "      }";

        SequenceBumpedEffectResponse effect;
        effect.loadFromJson(json);
        QCOMPARE(effect.getAccount().getAccountId(), "GDPFGP4IPE5DXG6XRXC4ZBUI43PAGRQ5VVNJ3LJTBXDBZ4ITO6HBHNSF");
        QCOMPARE(effect.getCreatedAt(), "2018-06-06T10:23:57Z");
        QCOMPARE(effect.getNewSequence(), 79473726952833048L);
      }


      void testDeserializeClaimableBalanceClawedbackEffect() {
         QByteArray json = "{\n"
             "        \"_links\": {\n"
             "          \"operation\": {\n"
             "            \"href\": \"https://horizon-testnet.stellar.org/operations/40181480638386177\"\n"
             "          },\n"
             "          \"succeeds\": {\n"
             "            \"href\": \"https://horizon-testnet.stellar.org/effects?order=desc&cursor=40181480638386177-1\"\n"
             "          },\n"
             "          \"precedes\": {\n"
             "            \"href\": \"https://horizon-testnet.stellar.org/effects?order=asc&cursor=40181480638386177-1\"\n"
             "          }\n"
             "        },\n"
             "        \"id\": \"0040181480638386177-0000000001\",\n"
             "        \"paging_token\": \"40181480638386177-1\",\n"
             "        \"account\": \"GDPFGP4IPE5DXG6XRXC4ZBUI43PAGRQ5VVNJ3LJTBXDBZ4ITO6HBHNSF\",\n"
             "        \"type\": \"claimable_balance_clawed_back\",\n"
             "        \"type_i\": 80,\n"
             "        \"balance_id\": \"00000000178826fbfe339e1f5c53417c6fedfe2c05e8bec14303143ec46b38981b09c3f9\",\n"
             "        \"created_at\": \"2018-06-06T10:23:57Z\"\n"
             "      }";

         ClaimableBalanceClawedBackEffectResponse effect;
         effect.loadFromJson(json);

         QCOMPARE(effect.getAccount(), QString("GDPFGP4IPE5DXG6XRXC4ZBUI43PAGRQ5VVNJ3LJTBXDBZ4ITO6HBHNSF"));
         QCOMPARE(effect.getCreatedAt(), QString("2018-06-06T10:23:57Z"));
         QCOMPARE(effect.getBalanceID(), QString("00000000178826fbfe339e1f5c53417c6fedfe2c05e8bec14303143ec46b38981b09c3f9"));
         QCOMPARE(effect.getType(), QString("claimable_balance_clawed_back"));
       }

       void testDeserializeTrustlineFlagsUpdatedEffect() {
         QByteArray json = "{\n"
             "        \"_links\": {\n"
             "          \"operation\": {\n"
             "            \"href\": \"https://horizon-testnet.stellar.org/operations/40181480638386177\"\n"
             "          },\n"
             "          \"succeeds\": {\n"
             "            \"href\": \"https://horizon-testnet.stellar.org/effects?order=desc&cursor=40181480638386177-1\"\n"
             "          },\n"
             "          \"precedes\": {\n"
             "            \"href\": \"https://horizon-testnet.stellar.org/effects?order=asc&cursor=40181480638386177-1\"\n"
             "          }\n"
             "        },\n"
             "        \"id\": \"0040181480638386177-0000000001\",\n"
             "        \"paging_token\": \"40181480638386177-1\",\n"
             "        \"account\": \"GDPFGP4IPE5DXG6XRXC4ZBUI43PAGRQ5VVNJ3LJTBXDBZ4ITO6HBHNSF\",\n"
             "        \"type\": \"trustline_flags_updated\",\n"
             "        \"type_i\": 26,\n"
             "        \"trustor\": \"GCVHDLN6EHZBYW2M3BQIY32C23E4GPIRZZDBNF2Q73DAZ5VJDRGSMYRB\",\n"
             "        \"asset_type\": \"credit_alphanum4\",\n"
             "        \"asset_code\": \"EUR\",\n"
             "        \"asset_issuer\": \"GCWVFBJ24754I5GXG4JOEB72GJCL3MKWC7VAEYWKGQHPVH3ENPNBSKWS\",\n"
             "        \"authorized_flag\": true,\n"
             "        \"clawback_enabled_flag\": true,\n"
             "        \"created_at\": \"2018-06-06T10:23:57Z\"\n"
             "      }";

         TrustlineFlagsUpdatedEffectResponse effect;
         effect.loadFromJson(json);

         QCOMPARE(effect.getType(), "trustline_flags_updated");

         QCOMPARE(effect.getAccount(), QString("GDPFGP4IPE5DXG6XRXC4ZBUI43PAGRQ5VVNJ3LJTBXDBZ4ITO6HBHNSF"));
         QCOMPARE(effect.getCreatedAt(), QString("2018-06-06T10:23:57Z"));
         QCOMPARE(effect.getTrustor(), QString("GCVHDLN6EHZBYW2M3BQIY32C23E4GPIRZZDBNF2Q73DAZ5VJDRGSMYRB"));
         QVERIFY(effect.getAuthorized());
         QVERIFY(effect.getClawbackEnabled());
         QVERIFY(!effect.getAuthorizedToMaintainLiabilities());

         QVERIFY(effect.getAsset()->equals(Asset::createNonNativeAsset("EUR", QString("GCWVFBJ24754I5GXG4JOEB72GJCL3MKWC7VAEYWKGQHPVH3ENPNBSKWS"))));
         QCOMPARE(effect.getAssetIssuer(), QString("GCWVFBJ24754I5GXG4JOEB72GJCL3MKWC7VAEYWKGQHPVH3ENPNBSKWS"));
         QCOMPARE(effect.getAssetCode(), QString("EUR"));
         QCOMPARE(effect.getAssetType(), QString("credit_alphanum4"));
       }
};

ADD_TEST(EffectDeserializerTest)
#endif // EFFECTDESERIALIZERTEST_H
