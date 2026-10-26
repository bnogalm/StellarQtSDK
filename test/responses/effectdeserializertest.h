#ifndef EFFECTDESERIALIZERTEST_H
#define EFFECTDESERIALIZERTEST_H
#include <QObject>


#include <QtTest>
#include "../testcollector.h"
#include "../../src/responses/effects/effectresponse.h"
#include "../../src/responses/effects/accountcreatedeffectresponse.h"
#include "../../src/responses/effects/accountcreditedeffectresponse.h"
#include "../../src/responses/effects/accountdebitedeffectresponse.h"
#include "../../src/responses/effects/contractcreditedeffectresponse.h"
#include "../../src/responses/effects/contractdebitedeffectresponse.h"
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
#include "../../src/responses/effects/liquiditypoolcreatedeffectresponse.h"
#include "../../src/responses/effects/liquiditypooldepositedeffectresponse.h"
#include "../../src/responses/effects/liquiditypoolwithdreweffectresponse.h"
#include "../../src/responses/effects/liquiditypooltradeeffectresponse.h"
#include "../../src/responses/effects/liquiditypoolremovedeffectresponse.h"
#include "../../src/responses/effects/liquiditypoolrevokedeffectresponse.h"
#include "../../src/responses/effects/accountsponsorshipcreatedeffectresponse.h"
#include "../../src/responses/effects/accountsponsorshipupdatedeffectresponse.h"
#include "../../src/responses/effects/accountsponsorshipremovedeffectresponse.h"
#include "../../src/responses/effects/trustlinesponsorshipcreatedeffectresponse.h"
#include "../../src/responses/effects/trustlinesponsorshipupdatedeffectresponse.h"
#include "../../src/responses/effects/trustlinesponsorshipremovedeffectresponse.h"
#include "../../src/responses/effects/datasponsorshipcreatedeffectresponse.h"
#include "../../src/responses/effects/datasponsorshipupdatedeffectresponse.h"
#include "../../src/responses/effects/datasponsorshipremovedeffectresponse.h"
#include "../../src/responses/effects/claimablebalancesponsorshipcreatedeffectresponse.h"
#include "../../src/responses/effects/claimablebalancesponsorshipupdatedeffectresponse.h"
#include "../../src/responses/effects/claimablebalancesponsorshipremovedeffectresponse.h"
#include "../../src/responses/effects/signersponsorshipcreatedeffectresponse.h"
#include "../../src/responses/effects/signersponsorshipupdatedeffectresponse.h"
#include "../../src/responses/effects/signersponsorshipremovedeffectresponse.h"
#include "../../src/asset.h"
#include "../../src/assettypenative.h"
#include "../../src/keypair.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "../../src/responses/effectspage.h"
#include "../../src/responses/effects/claimablebalanceclaimantcreatedeffectresponse.h"
#include "../../src/predicate.h"

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
              "        \"created_at\": \"2026-09-28T15:16:12Z\",\n"
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
              "        \"created_at\": \"2026-09-28T15:16:12Z\",\n"
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
              "        \"created_at\": \"2026-09-28T15:16:12Z\",\n"
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

    void testDeserializeContractCreditedEffect() {
      QByteArray json = "{\n"
              "        \"created_at\": \"2026-09-28T15:16:12Z\",\n"
              "        \"_links\": {\n"
              "          \"operation\": {\n"
              "            \"href\": \"https://horizon-testnet.stellar.org/operations/13563506724865\"\n"
              "          },\n"
              "          \"succeeds\": {\n"
              "            \"href\": \"https://horizon-testnet.stellar.org/effects?order=desc&cursor=13563506724865-1\"\n"
              "          },\n"
              "          \"precedes\": {\n"
              "            \"href\": \"https://horizon-testnet.stellar.org/effects?order=asc&cursor=13563506724865-1\"\n"
              "          }\n"
              "        },\n"
              "        \"id\": \"0000013563506724865-0000000001\",\n"
              "        \"paging_token\": \"13563506724865-1\",\n"
              "        \"account\": \"GDLGTRIBFH24364GPWPUS45GUFC2GU4ARPGWTXVCPLGTUHX3IOS3ON47\",\n"
              "        \"type\": \"contract_credited\",\n"
              "        \"type_i\": 96,\n"
              "        \"contract\": \"CA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVSGZ\",\n"
              "        \"asset_type\": \"native\",\n"
              "        \"amount\": \"250.0000000\"\n"
              "      }";

      ContractCreditedEffectResponse effect;
      effect.loadFromJson(json);

      QCOMPARE(effect.getType(), QString("contract_credited"));
      QCOMPARE(effect.getContract(), QString("CA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVSGZ"));
      QCOMPARE(effect.getAmount(), QString("250.0000000"));
      QVERIFY(effect.getAsset()->equals(new AssetTypeNative()));
    }

    void testDeserializeContractDebitedEffect() {
      QByteArray json = "{\n"
              "        \"created_at\": \"2026-09-28T15:16:12Z\",\n"
              "        \"_links\": {\n"
              "          \"operation\": {\n"
              "            \"href\": \"https://horizon-testnet.stellar.org/operations/13563506724866\"\n"
              "          },\n"
              "          \"succeeds\": {\n"
              "            \"href\": \"https://horizon-testnet.stellar.org/effects?order=desc&cursor=13563506724866-1\"\n"
              "          },\n"
              "          \"precedes\": {\n"
              "            \"href\": \"https://horizon-testnet.stellar.org/effects?order=asc&cursor=13563506724866-1\"\n"
              "          }\n"
              "        },\n"
              "        \"id\": \"0000013563506724866-0000000001\",\n"
              "        \"paging_token\": \"13563506724866-1\",\n"
              "        \"account\": \"GDLGTRIBFH24364GPWPUS45GUFC2GU4ARPGWTXVCPLGTUHX3IOS3ON47\",\n"
              "        \"type\": \"contract_debited\",\n"
              "        \"type_i\": 97,\n"
              "        \"contract\": \"CA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVSGZ\",\n"
              "        \"asset_type\": \"credit_alphanum4\",\n"
              "        \"asset_code\": \"USDC\",\n"
              "        \"asset_issuer\": \"GA5ZSEJYB37JRC5AVCIA5MOP4RHTM335X2KGX3IHOJAPP5RE34K4KZVN\",\n"
              "        \"amount\": \"42.0000000\"\n"
              "      }";

      ContractDebitedEffectResponse effect;
      effect.loadFromJson(json);

      QCOMPARE(effect.getType(), QString("contract_debited"));
      QCOMPARE(effect.getContract(), QString("CA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVSGZ"));
      QCOMPARE(effect.getAmount(), QString("42.0000000"));
      QVERIFY(effect.getAsset()->equals(Asset::createNonNativeAsset("USDC", KeyPair::fromAccountId("GA5ZSEJYB37JRC5AVCIA5MOP4RHTM335X2KGX3IHOJAPP5RE34K4KZVN"))));
    }


    void testDeserializeAccountDebitedEffect() {
      QByteArray json = "{\n"
              "        \"created_at\": \"2026-09-28T15:16:12Z\",\n"
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
              "        \"created_at\": \"2026-09-28T15:16:12Z\",\n"
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
              "        \"created_at\": \"2026-09-28T15:16:12Z\",\n"
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
              "        \"created_at\": \"2026-09-28T15:16:12Z\",\n"
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
              "        \"created_at\": \"2026-09-28T15:16:12Z\",\n"
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
              "        \"created_at\": \"2026-09-28T15:16:12Z\",\n"
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
              "        \"created_at\": \"2026-09-28T15:16:12Z\",\n"
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
              "        \"created_at\": \"2026-09-28T15:16:12Z\",\n"
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
              "        \"created_at\": \"2026-09-28T15:16:12Z\",\n"
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
              "        \"created_at\": \"2026-09-28T15:16:12Z\",\n"
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
              "        \"created_at\": \"2026-09-28T15:16:12Z\",\n"
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
              "        \"created_at\": \"2026-09-28T15:16:12Z\",\n"
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
              "        \"created_at\": \"2026-09-28T15:16:12Z\",\n"
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
              "        \"created_at\": \"2026-09-28T15:16:12Z\",\n"
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
            "        \"created_at\": \"2026-09-28T15:16:12Z\",\n"
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

       void testDeserializeLiquidityPoolCreatedEffect() {
         QByteArray json = "{"
           "\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/1\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=1\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=1\"}},"
           "\"id\":\"0001\",\"paging_token\":\"1\","
           "\"account\":\"GDPFGP4IPE5DXG6XRXC4ZBUI43PAGRQ5VVNJ3LJTBXDBZ4ITO6HBHNSF\","
           "\"type\":\"liquidity_pool_created\",\"type_i\":93,"
           "\"created_at\":\"2021-11-08T01:46:38Z\","
           "\"liquidity_pool\":{"
             "\"id\":\"abc\",\"fee_bp\":30,\"type\":\"constant_product\","
             "\"total_trustlines\":\"1\",\"total_shares\":\"100\","
             "\"reserves\":[{\"asset\":\"native\",\"amount\":\"50.0000000\"},"
                           "{\"asset\":\"USDC:GA5\",\"amount\":\"50.0000000\"}]"
           "}}";
         LiquidityPoolCreatedEffectResponse effect;
         effect.loadFromJson(json);
         QCOMPARE(effect.getType(), QString("liquidity_pool_created"));
         QCOMPARE(effect.getLiquidityPool().getId(), QString("abc"));
         QCOMPARE(effect.getLiquidityPool().getFeeBp(), (qint32)30);
         QCOMPARE(effect.getLiquidityPool().getType(), QString("constant_product"));
         QCOMPARE(effect.getLiquidityPool().getReserves().size(), 2);
         QCOMPARE(effect.getLiquidityPool().getReserves()[0].getAsset(), QString("native"));
         QCOMPARE(effect.getLiquidityPool().getReserves()[0].getAmount(), QString("50.0000000"));
       }

       void testDeserializeLiquidityPoolDepositedEffect() {
         QByteArray json = "{"
           "\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/1\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=1\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=1\"}},"
           "\"id\":\"0001\",\"paging_token\":\"1\","
           "\"account\":\"GDPFGP4IPE5DXG6XRXC4ZBUI43PAGRQ5VVNJ3LJTBXDBZ4ITO6HBHNSF\","
           "\"type\":\"liquidity_pool_deposited\",\"type_i\":90,"
           "\"created_at\":\"2021-11-08T01:46:38Z\","
           "\"liquidity_pool\":{\"id\":\"pid\",\"fee_bp\":30,\"type\":\"constant_product\","
             "\"total_trustlines\":\"1\",\"total_shares\":\"100\",\"reserves\":[]},"
           "\"reserves_deposited\":[{\"asset\":\"native\",\"amount\":\"10\"},"
                                  "{\"asset\":\"USDC:GA5\",\"amount\":\"20\"}],"
           "\"shares_received\":\"5.0\""
           "}";
         LiquidityPoolDepositedEffectResponse effect;
         effect.loadFromJson(json);
         QCOMPARE(effect.getType(), QString("liquidity_pool_deposited"));
         QCOMPARE(effect.getLiquidityPool().getId(), QString("pid"));
         QCOMPARE(effect.getReservesDeposited().size(), 2);
         QCOMPARE(effect.getReservesDeposited()[0].getAsset(), QString("native"));
         QCOMPARE(effect.getReservesDeposited()[0].getAmount(), QString("10"));
         QCOMPARE(effect.getReservesDeposited()[1].getAsset(), QString("USDC:GA5"));
         QCOMPARE(effect.getSharesReceived(), QString("5.0"));
       }

       void testDeserializeLiquidityPoolWithdrewEffect() {
         QByteArray json = "{"
           "\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/1\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=1\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=1\"}},"
           "\"id\":\"0001\",\"paging_token\":\"1\","
           "\"account\":\"GDPFGP4IPE5DXG6XRXC4ZBUI43PAGRQ5VVNJ3LJTBXDBZ4ITO6HBHNSF\","
           "\"type\":\"liquidity_pool_withdrew\",\"type_i\":91,"
           "\"created_at\":\"2021-11-08T01:46:38Z\","
           "\"liquidity_pool\":{\"id\":\"pid\",\"fee_bp\":30,\"type\":\"constant_product\","
             "\"total_trustlines\":\"1\",\"total_shares\":\"95\",\"reserves\":[]},"
           "\"reserves_received\":[{\"asset\":\"native\",\"amount\":\"5\"}],"
           "\"shares_redeemed\":\"5.0\""
           "}";
         LiquidityPoolWithdrewEffectResponse effect;
         effect.loadFromJson(json);
         QCOMPARE(effect.getType(), QString("liquidity_pool_withdrew"));
         QCOMPARE(effect.getLiquidityPool().getTotalShares(), QString("95"));
         QCOMPARE(effect.getReservesReceived().size(), 1);
         QCOMPARE(effect.getSharesRedeemed(), QString("5.0"));
       }

       void testDeserializeLiquidityPoolTradeEffect() {
         QByteArray json = "{"
           "\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/1\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=1\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=1\"}},"
           "\"id\":\"0001\",\"paging_token\":\"1\","
           "\"account\":\"GDPFGP4IPE5DXG6XRXC4ZBUI43PAGRQ5VVNJ3LJTBXDBZ4ITO6HBHNSF\","
           "\"type\":\"liquidity_pool_trade\",\"type_i\":92,"
           "\"created_at\":\"2021-11-08T01:46:38Z\","
           "\"liquidity_pool\":{\"id\":\"pid\",\"fee_bp\":30,\"type\":\"constant_product\","
             "\"total_trustlines\":\"1\",\"total_shares\":\"100\",\"reserves\":[]},"
           "\"sold\":{\"asset\":\"native\",\"amount\":\"1\"},"
           "\"bought\":{\"asset\":\"USDC:GA5\",\"amount\":\"2\"}"
           "}";
         LiquidityPoolTradeEffectResponse effect;
         effect.loadFromJson(json);
         QCOMPARE(effect.getType(), QString("liquidity_pool_trade"));
         QCOMPARE(effect.getSold().getAsset(), QString("native"));
         QCOMPARE(effect.getSold().getAmount(), QString("1"));
         QCOMPARE(effect.getBought().getAsset(), QString("USDC:GA5"));
         QCOMPARE(effect.getBought().getAmount(), QString("2"));
       }

       void testDeserializeLiquidityPoolRemovedEffect() {
         QByteArray json = "{"
           "\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/1\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=1\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=1\"}},"
           "\"id\":\"0001\",\"paging_token\":\"1\","
           "\"account\":\"GDPFGP4IPE5DXG6XRXC4ZBUI43PAGRQ5VVNJ3LJTBXDBZ4ITO6HBHNSF\","
           "\"type\":\"liquidity_pool_removed\",\"type_i\":94,"
           "\"created_at\":\"2021-11-08T01:46:38Z\","
           "\"liquidity_pool_id\":\"abc1234\""
           "}";
         LiquidityPoolRemovedEffectResponse effect;
         effect.loadFromJson(json);
         QCOMPARE(effect.getType(), QString("liquidity_pool_removed"));
         QCOMPARE(effect.getLiquidityPoolId(), QString("abc1234"));
       }

       void testDeserializeLiquidityPoolRevokedEffect() {
         QByteArray json = "{"
           "\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/1\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=1\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=1\"}},"
           "\"id\":\"0001\",\"paging_token\":\"1\","
           "\"account\":\"GDPFGP4IPE5DXG6XRXC4ZBUI43PAGRQ5VVNJ3LJTBXDBZ4ITO6HBHNSF\","
           "\"type\":\"liquidity_pool_revoked\",\"type_i\":95,"
           "\"created_at\":\"2021-11-08T01:46:38Z\","
           "\"liquidity_pool\":{\"id\":\"pid\",\"fee_bp\":30,\"type\":\"constant_product\","
             "\"total_trustlines\":\"0\",\"total_shares\":\"0\",\"reserves\":[]},"
           "\"reserves_revoked\":[{\"asset\":\"native\",\"amount\":\"5\","
                                  "\"claimable_balance_id\":\"00abcd\"}],"
           "\"shares_revoked\":\"5.0\""
           "}";
         LiquidityPoolRevokedEffectResponse effect;
         effect.loadFromJson(json);
         QCOMPARE(effect.getType(), QString("liquidity_pool_revoked"));
         QCOMPARE(effect.getReservesRevoked().size(), 1);
         QCOMPARE(effect.getReservesRevoked()[0].getAsset(), QString("native"));
         QCOMPARE(effect.getReservesRevoked()[0].getClaimableBalanceId(), QString("00abcd"));
         QCOMPARE(effect.getSharesRevoked(), QString("5.0"));
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

    // ── Sponsorship effects (type_i 60-74) — were entirely untested ───────
    // CAP-33 reserve sponsorship. Each family carries different fields:
    // created → sponsor; updated → former_sponsor + new_sponsor; removed →
    // former_sponsor; plus a resource discriminator (asset / data_name /
    // balance_id / signer). Deserialized by reflection on the concrete class.

    void testDeserializeAccountSponsorshipCreatedEffect() {
        QByteArray json = "{"
            "\"created_at\":\"2026-09-28T15:16:12Z\",\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/65571265847297\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=65571265847297-1\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=65571265847297-1\"}},"
            "\"id\":\"0000065571265847297-0000000001\",\"paging_token\":\"65571265847297-1\","
            "\"account\":\"GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H\","
            "\"type\":\"account_sponsorship_created\",\"type_i\":60,"
            "\"sponsor\":\"GCFKT6BN2FEASCEVDNHEC4LLFT2KLUUPEMKM4OJPEJ65H2AEZ7IH4RV6\"}";
        AccountSponsorshipCreatedEffectResponse effect;
        effect.loadFromJson(json);
        QCOMPARE(effect.getType(), QString("account_sponsorship_created"));
        QCOMPARE(effect.getSponsor(), QString("GCFKT6BN2FEASCEVDNHEC4LLFT2KLUUPEMKM4OJPEJ65H2AEZ7IH4RV6"));
    }

    void testDeserializeAccountSponsorshipUpdatedEffect() {
        QByteArray json = "{"
            "\"created_at\":\"2026-09-28T15:16:12Z\",\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/p\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=p\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=p\"}},"
            "\"id\":\"i\",\"paging_token\":\"p\","
            "\"account\":\"GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H\","
            "\"type\":\"account_sponsorship_updated\",\"type_i\":61,"
            "\"former_sponsor\":\"GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO\","
            "\"new_sponsor\":\"GB24LPGAHYTWRYOXIDKXLI55SBRWW42T3TZKDAAW3BOJX4ADVIATFTLU\"}";
        AccountSponsorshipUpdatedEffectResponse effect;
        effect.loadFromJson(json);
        QCOMPARE(effect.getType(), QString("account_sponsorship_updated"));
        QCOMPARE(effect.getFormerSponsor(), QString("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"));
        QCOMPARE(effect.getNewSponsor(), QString("GB24LPGAHYTWRYOXIDKXLI55SBRWW42T3TZKDAAW3BOJX4ADVIATFTLU"));
    }

    void testDeserializeAccountSponsorshipRemovedEffect() {
        QByteArray json = "{"
            "\"created_at\":\"2026-09-28T15:16:12Z\",\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/p\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=p\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=p\"}},"
            "\"id\":\"i\",\"paging_token\":\"p\","
            "\"account\":\"GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H\","
            "\"type\":\"account_sponsorship_removed\",\"type_i\":62,"
            "\"former_sponsor\":\"GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO\"}";
        AccountSponsorshipRemovedEffectResponse effect;
        effect.loadFromJson(json);
        QCOMPARE(effect.getType(), QString("account_sponsorship_removed"));
        QCOMPARE(effect.getFormerSponsor(), QString("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"));
    }

    void testDeserializeTrustlineSponsorshipCreatedEffect() {
        QByteArray json = "{"
            "\"created_at\":\"2026-09-28T15:16:12Z\",\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/p\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=p\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=p\"}},"
            "\"id\":\"i\",\"paging_token\":\"p\","
            "\"account\":\"GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H\","
            "\"type\":\"trustline_sponsorship_created\",\"type_i\":63,"
            "\"asset\":\"USD:GCWVFBJ24754I5GXG4JOEB72GJCL3MKWC7VAEYWKGQHPVH3ENPNBSKWS\","
            "\"sponsor\":\"GCFKT6BN2FEASCEVDNHEC4LLFT2KLUUPEMKM4OJPEJ65H2AEZ7IH4RV6\"}";
        TrustlineSponsorshipCreatedEffectResponse effect;
        effect.loadFromJson(json);
        QCOMPARE(effect.getType(), QString("trustline_sponsorship_created"));
        QCOMPARE(effect.asset(), QString("USD:GCWVFBJ24754I5GXG4JOEB72GJCL3MKWC7VAEYWKGQHPVH3ENPNBSKWS"));
        QCOMPARE(effect.getSponsor(), QString("GCFKT6BN2FEASCEVDNHEC4LLFT2KLUUPEMKM4OJPEJ65H2AEZ7IH4RV6"));
    }

    void testDeserializeTrustlineSponsorshipUpdatedEffect() {
        // Also a regression guard: the `new_sponsor` field was previously
        // declared as a `newSponsor` Q_PROPERTY (camelCase), so Horizon's
        // snake_case key never bound and getNewSponsor() came back empty.
        QByteArray json = "{"
            "\"created_at\":\"2026-09-28T15:16:12Z\",\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/p\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=p\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=p\"}},"
            "\"id\":\"i\",\"paging_token\":\"p\","
            "\"account\":\"GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H\","
            "\"type\":\"trustline_sponsorship_updated\",\"type_i\":64,"
            "\"asset\":\"USD:GCWVFBJ24754I5GXG4JOEB72GJCL3MKWC7VAEYWKGQHPVH3ENPNBSKWS\","
            "\"former_sponsor\":\"GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO\","
            "\"new_sponsor\":\"GB24LPGAHYTWRYOXIDKXLI55SBRWW42T3TZKDAAW3BOJX4ADVIATFTLU\"}";
        TrustlineSponsorshipUpdatedEffectResponse effect;
        effect.loadFromJson(json);
        QCOMPARE(effect.getType(), QString("trustline_sponsorship_updated"));
        QCOMPARE(effect.getFormerSponsor(), QString("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"));
        QCOMPARE(effect.getNewSponsor(), QString("GB24LPGAHYTWRYOXIDKXLI55SBRWW42T3TZKDAAW3BOJX4ADVIATFTLU"));
    }

    void testDeserializeTrustlineSponsorshipRemovedEffect() {
        QByteArray json = "{"
            "\"created_at\":\"2026-09-28T15:16:12Z\",\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/p\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=p\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=p\"}},"
            "\"id\":\"i\",\"paging_token\":\"p\","
            "\"account\":\"GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H\","
            "\"type\":\"trustline_sponsorship_removed\",\"type_i\":65,"
            "\"asset\":\"USD:GCWVFBJ24754I5GXG4JOEB72GJCL3MKWC7VAEYWKGQHPVH3ENPNBSKWS\","
            "\"former_sponsor\":\"GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO\"}";
        TrustlineSponsorshipRemovedEffectResponse effect;
        effect.loadFromJson(json);
        QCOMPARE(effect.getType(), QString("trustline_sponsorship_removed"));
        QCOMPARE(effect.getFormerSponsor(), QString("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"));
    }

    /**
     * A pool-share trustline's sponsorship effects carry asset_type
     * "liquidity_pool" and liquidity_pool_id instead of "asset". They were
     * refused as an unknown asset type, and with them the whole page (and
     * the event, in a stream): an account's effects could not be read past
     * one. Real Horizon testnet records.
     */
    void testPoolShareTrustlineSponsorshipEffectsLoad()
    {
        EffectsPage page;
        page.loadFromJson(realPoolSponsorshipEffects());
        QCOMPARE(page.size(), 4);
        auto* created = dynamic_cast<TrustlineSponsorshipCreatedEffectResponse*>(page.getRecords().at(0));
        QVERIFY(created != nullptr);
        QCOMPARE(created->getAssetType(), QString("liquidity_pool"));
        QCOMPARE(created->getLiquidityPoolId(), QString("e590400b7fdb37ac6eb79fa6ddf4f7842b1d679a8b9d515f47c724def2bac1b3"));
        QCOMPARE(created->asset(), QString());
        QCOMPARE(created->getSponsor(), QString("GD3N5KQ4TM6N7IQZFBPTQSKCI3D5HDAOPXM33PG5HNP7LQVSWHT4NAIF"));
        auto* credit = dynamic_cast<TrustlineSponsorshipUpdatedEffectResponse*>(page.getRecords().at(1));
        QVERIFY(credit != nullptr);
        QCOMPARE(credit->getAssetType(), QString("credit_alphanum4"));
        QCOMPARE(credit->asset(), QString("UPD:GB2SY6GIMINOPCHWQ6DAMM632Z6FW2VOAHT4ENKYBXAP3NWQQXCB5GQI"));
        QCOMPARE(credit->getLiquidityPoolId(), QString());
        auto* updated = dynamic_cast<TrustlineSponsorshipUpdatedEffectResponse*>(page.getRecords().at(2));
        QVERIFY(updated != nullptr);
        QCOMPARE(updated->getLiquidityPoolId(), QString("0fe05edb0c9cde295fd2e15175c42628a1988f9dc8a1ecb23635cab692a1c6c1"));
        QCOMPARE(updated->getNewSponsor(), QString("GDNF6H2HLUNAKZ3WGCOOOCX6U3Y3TW5OL33P77KSACARDG6G53HC33YG"));
        auto* removed = dynamic_cast<TrustlineSponsorshipRemovedEffectResponse*>(page.getRecords().at(3));
        QVERIFY(removed != nullptr);
        QCOMPARE(removed->getLiquidityPoolId(), QString("e590400b7fdb37ac6eb79fa6ddf4f7842b1d679a8b9d515f47c724def2bac1b3"));
        QCOMPARE(removed->getFormerSponsor(), QString("GD3N5KQ4TM6N7IQZFBPTQSKCI3D5HDAOPXM33PG5HNP7LQVSWHT4NAIF"));
    }

    /**
     * A pool-share trustline's trustline_created/updated/removed effect is
     * named by liquidity_pool_id, which was dropped: nothing said which pool
     * the line was for, and getAsset() threw on the empty issuer. Real
     * Horizon testnet record. Every member is also in json(), as js and py
     * return the record.
     */
    void testPoolShareTrustlineEffectNamesItsPool()
    {
        const QByteArray json(
            "{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21254732290990082\"},\"succeeds\":{\"href\":\"https://"
            "horizon-testnet.stellar.org/effects?order=desc&cursor=21254732290990082-1\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/ef"
            "fects?order=asc&cursor=21254732290990082-1\"}},\"id\":\"0021254732290990082-0000000001\",\"paging_token\":\"21254732290990082-1\",\"account\""
            ":\"GBUXQKGGE3SL7IGDX6ZFMGACBBDDS6JSAPALZEMRVDO7M65YSNKHWICN\",\"type\":\"trustline_created\",\"type_i\":20,\"created_at\":\"2026-09-30T11:22"
            ":32Z\",\"asset_type\":\"liquidity_pool_shares\",\"liquidity_pool_id\":\"e590400b7fdb37ac6eb79fa6ddf4f7842b1d679a8b9d515f47c724def2bac1b3\",\""
            "limit\":\"922337203685.4775807\"}");
        TrustlineCreatedEffectResponse effect;
        effect.loadFromJson(json);
        QCOMPARE(effect.assetType(), QStringLiteral("liquidity_pool_shares"));
        QCOMPARE(effect.getLiquidityPoolId(), QStringLiteral("e590400b7fdb37ac6eb79fa6ddf4f7842b1d679a8b9d515f47c724def2bac1b3"));
        QCOMPARE(effect.getLimit(), QStringLiteral("922337203685.4775807"));
        bool refused = false;
        try { effect.getAsset(); } catch (const std::runtime_error& e) {
            refused = QString::fromUtf8(e.what()).contains(QStringLiteral("getLiquidityPoolId"));
        }
        QVERIFY(refused);
        QCOMPARE(effect.json(), QJsonDocument::fromJson(json).object());
        QCOMPARE(effect.json().value(QStringLiteral("type_i")).toInt(), 20);
    }

    /**
     * Real Horizon testnet effects (2026-09-30). The data effects' name and
     * value were dropped: nothing said which entry changed. Flag effects list
     * only the flags the operation changed, and an absent one read as false
     * ("cleared"): here the account's flags effect has none (Horizon drops
     * revocable and clawback), and the trustline's has only authorized_flag.
     */
    void testDataAndFlagEffectsSayWhatTheRecordSays()
    {
        EffectsPage page;
        page.loadFromJson(QByteArray(
            "{\"_embedded\":{\"records\":[{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21273784765927426\"},\"su"
            "cceeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21273784765927426-1\"},\"precedes\":{\"href\":\"https://h"
            "orizon-testnet.stellar.org/effects?order=asc&cursor=21273784765927426-1\"}},\"id\":\"0021273784765927426-0000000001\",\"paging_token\":\"212"
            "73784765927426-1\",\"account\":\"GDNZGKUHS3HMWMQPBNRP56PGJCG7K7KG72OFSOKB4YKIMTPHZT3FDTFB\",\"type\":\"data_created\",\"type_i\":40,\"create"
            "d_at\":\"2026-09-30T17:32:12Z\",\"name\":\"bin\",\"value\":\"AAECAwQFBgcICQoLDA0ODxAREhMUFRYXGBkaGxwdHh8gISIjJCUmJygpKissLS4vMDEyMzQ1Njc4OTo"
            "7PD0+Pw==\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21276305911713793\"},\"succeeds\":{\"href"
            "\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21276305911713793-1\"},\"precedes\":{\"href\":\"https://horizon-testnet.s"
            "tellar.org/effects?order=asc&cursor=21276305911713793-1\"}},\"id\":\"0021276305911713793-0000000001\",\"paging_token\":\"21276305911713793-1"
            "\",\"account\":\"GD6ZOJLQLECZ2UKQHBKCBFCFOGJDYX466PZW6M75JEAICPNTOEAAJ5JC\",\"type\":\"data_updated\",\"type_i\":42,\"created_at\":\"2026-09"
            "-30T18:21:07Z\",\"name\":\"k\",\"value\":\"djI=\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/212"
            "73789060878337\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21273789060878337-1\"},\"precedes\""
            ":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21273789060878337-1\"}},\"id\":\"0021273789060878337-0000000001\","
            "\"paging_token\":\"21273789060878337-1\",\"account\":\"GDNZGKUHS3HMWMQPBNRP56PGJCG7K7KG72OFSOKB4YKIMTPHZT3FDTFB\",\"type\":\"data_removed\","
            "\"type_i\":41,\"created_at\":\"2026-09-30T17:32:17Z\",\"name\":\"bin\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stell"
            "ar.org/operations/21273720341413889\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21273720341413"
            "889-3\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21273720341413889-3\"}},\"id\":\"002127372034"
            "1413889-0000000003\",\"paging_token\":\"21273720341413889-3\",\"account\":\"GDNZGKUHS3HMWMQPBNRP56PGJCG7K7KG72OFSOKB4YKIMTPHZT3FDTFB\",\"typ"
            "e\":\"account_flags_updated\",\"type_i\":6,\"created_at\":\"2026-09-30T17:30:57Z\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon-t"
            "estnet.stellar.org/operations/21276224307339265\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21"
            "276224307339265-2\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21276224307339265-2\"}},\"id\":\""
            "0021276224307339265-0000000002\",\"paging_token\":\"21276224307339265-2\",\"account\":\"GAGQ5LNMCIXUBKAFXWWXEHGGG33OONJ27NXL7D5EOAQ77X4OCD7R"
            "JJKN\",\"account_muxed\":\"MAGQ5LNMCIXUBKAFXWWXEHGGG33OONJ27NXL7D5EOAQ77X4OCD7RIAAAAAAAAAAAFLZVW\",\"account_muxed_id\":\"42\",\"type\":\"tr"
            "ustline_flags_updated\",\"type_i\":26,\"created_at\":\"2026-09-30T18:19:32Z\",\"asset_type\":\"credit_alphanum4\",\"asset_code\":\"USD\",\"a"
            "sset_issuer\":\"GAGQ5LNMCIXUBKAFXWWXEHGGG33OONJ27NXL7D5EOAQ77X4OCD7RJJKN\",\"trustor\":\"GD6ZOJLQLECZ2UKQHBKCBFCFOGJDYX466PZW6M75JEAICPNTOEA"
            "AJ5JC\",\"authorized_flag\":true}]}}"));
        QCOMPARE(page.size(), 5);
        auto* created = dynamic_cast<DataCreatedEffectResponse*>(page.getRecords().at(0));
        QVERIFY(created);
        QCOMPARE(created->getName(), QStringLiteral("bin"));
        QCOMPARE(created->getValueBytes(), QByteArray::fromHex(
            "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f"
            "202122232425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f"));
        auto* updated = dynamic_cast<DataUpdatedEffectResponse*>(page.getRecords().at(1));
        QVERIFY(updated);
        QCOMPARE(updated->getName(), QStringLiteral("k"));
        QCOMPARE(updated->getValueBytes(), QByteArray("v2"));
        auto* removed = dynamic_cast<DataRemovedEffectResponse*>(page.getRecords().at(2));
        QVERIFY(removed);
        QCOMPARE(removed->getName(), QStringLiteral("bin"));

        auto* account = dynamic_cast<AccountFlagsUpdatedEffectResponse*>(page.getRecords().at(3));
        QVERIFY(account);
        QVERIFY(!account->hasAuthRequiredFlag());
        QVERIFY(!account->hasAuthRevokableFlag());
        auto* trustline = dynamic_cast<TrustlineFlagsUpdatedEffectResponse*>(page.getRecords().at(4));
        QVERIFY(trustline);
        QVERIFY(trustline->hasAuthorized());
        QCOMPARE(trustline->getAuthorized(), true);
        QVERIFY(!trustline->hasAuthorizedToMaintainLiabilities());
        QVERIFY(!trustline->hasClawbackEnabled());

        // Horizon's spelling of the maintain-liabilities flag is read.
        QJsonObject record = QJsonDocument::fromJson(QByteArray(
            "{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21276224307339265\"},\"succeeds\":{\"href\":\"https://"
            "horizon-testnet.stellar.org/effects?order=desc&cursor=21276224307339265-2\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/ef"
            "fects?order=asc&cursor=21276224307339265-2\"}},\"id\":\"0021276224307339265-0000000002\",\"paging_token\":\"21276224307339265-2\",\"account\""
            ":\"GAGQ5LNMCIXUBKAFXWWXEHGGG33OONJ27NXL7D5EOAQ77X4OCD7RJJKN\",\"account_muxed\":\"MAGQ5LNMCIXUBKAFXWWXEHGGG33OONJ27NXL7D5EOAQ77X4OCD7RIAAAAA"
            "AAAAAAFLZVW\",\"account_muxed_id\":\"42\",\"type\":\"trustline_flags_updated\",\"type_i\":26,\"created_at\":\"2026-09-30T18:19:32Z\",\"asset"
            "_type\":\"credit_alphanum4\",\"asset_code\":\"USD\",\"asset_issuer\":\"GAGQ5LNMCIXUBKAFXWWXEHGGG33OONJ27NXL7D5EOAQ77X4OCD7RJJKN\",\"trustor\""
            ":\"GD6ZOJLQLECZ2UKQHBKCBFCFOGJDYX466PZW6M75JEAICPNTOEAAJ5JC\",\"authorized_flag\":true}")).object();
        record.insert(QStringLiteral("authorized_to_maintain_liabilites_flag"), true);
        TrustlineFlagsUpdatedEffectResponse spelled;
        spelled.loadFromJson(QJsonDocument(record).toJson());
        QVERIFY(spelled.hasAuthorizedToMaintainLiabilities());
        QCOMPARE(spelled.getAuthorizedToMaintainLiabilities(), true);
    }

    /** What a pool-share sponsorship must hold is still checked. */
    void testPoolShareTrustlineSponsorshipIsChecked()
    {
        const QByteArray page = realPoolSponsorshipEffects();
        QVERIFY(refusalOf<EffectsPage>(page, "_embedded.records[0].liquidity_pool_id", QJsonValue(QJsonValue::Undefined))
                    .startsWith("_embedded.records[0].liquidity_pool_id: "));
        QVERIFY(refusalOf<EffectsPage>(page, "_embedded.records[0].liquidity_pool_id",
                                       QString("E590400B7FDB37AC6EB79FA6DDF4F7842B1D679A8B9D515F47C724DEF2BAC1B3"))
                    .startsWith("_embedded.records[0].liquidity_pool_id: "));
        QVERIFY(refusalOf<EffectsPage>(page, "_embedded.records[0].liquidity_pool_id", QString("e590"))
                    .startsWith("_embedded.records[0].liquidity_pool_id: "));
        QVERIFY(refusalOf<EffectsPage>(page, "_embedded.records[0].asset",
                                       QString("UPD:GB2SY6GIMINOPCHWQ6DAMM632Z6FW2VOAHT4ENKYBXAP3NWQQXCB5GQI"))
                    .startsWith("_embedded.records[0].asset: "));
        // A credit sponsorship's asset_type is still checked as an asset type.
        QVERIFY(refusalOf<EffectsPage>(page, "_embedded.records[1].asset_type", QString("pool"))
                    .startsWith("_embedded.records[1].asset_type: "));
        // "liquidity_pool" is a sponsorship's word only: anywhere else it stays unknown.
        QJsonObject credited{{"_embedded", QJsonObject{{"records", QJsonArray{
            QJsonDocument::fromJson(realAccountCredited()).object()}}}}};
        QVERIFY(refusalOf<EffectsPage>(QJsonDocument(credited).toJson(), "_embedded.records[0].asset_type",
                                       QString("liquidity_pool"))
                    .startsWith("_embedded.records[0].asset_type: "));
    }

    void testDeserializeDataSponsorshipCreatedEffect() {
        QByteArray json = "{"
            "\"created_at\":\"2026-09-28T15:16:12Z\",\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/p\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=p\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=p\"}},"
            "\"id\":\"i\",\"paging_token\":\"p\","
            "\"account\":\"GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H\","
            "\"type\":\"data_sponsorship_created\",\"type_i\":66,"
            "\"data_name\":\"config.memo\","
            "\"sponsor\":\"GCFKT6BN2FEASCEVDNHEC4LLFT2KLUUPEMKM4OJPEJ65H2AEZ7IH4RV6\"}";
        DataSponsorshipCreatedEffectResponse effect;
        effect.loadFromJson(json);
        QCOMPARE(effect.getType(), QString("data_sponsorship_created"));
        QCOMPARE(effect.getDataName(), QString("config.memo"));
        QCOMPARE(effect.getSponsor(), QString("GCFKT6BN2FEASCEVDNHEC4LLFT2KLUUPEMKM4OJPEJ65H2AEZ7IH4RV6"));
    }

    void testDeserializeDataSponsorshipUpdatedEffect() {
        QByteArray json = "{"
            "\"created_at\":\"2026-09-28T15:16:12Z\",\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/p\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=p\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=p\"}},"
            "\"id\":\"i\",\"paging_token\":\"p\","
            "\"account\":\"GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H\","
            "\"type\":\"data_sponsorship_updated\",\"type_i\":67,"
            "\"data_name\":\"config.memo\","
            "\"former_sponsor\":\"GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO\","
            "\"new_sponsor\":\"GB24LPGAHYTWRYOXIDKXLI55SBRWW42T3TZKDAAW3BOJX4ADVIATFTLU\"}";
        DataSponsorshipUpdatedEffectResponse effect;
        effect.loadFromJson(json);
        QCOMPARE(effect.getType(), QString("data_sponsorship_updated"));
        QCOMPARE(effect.getDataName(), QString("config.memo"));
        QCOMPARE(effect.getFormerSponsor(), QString("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"));
        QCOMPARE(effect.getNewSponsor(), QString("GB24LPGAHYTWRYOXIDKXLI55SBRWW42T3TZKDAAW3BOJX4ADVIATFTLU"));
    }

    void testDeserializeDataSponsorshipRemovedEffect() {
        QByteArray json = "{"
            "\"created_at\":\"2026-09-28T15:16:12Z\",\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/p\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=p\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=p\"}},"
            "\"id\":\"i\",\"paging_token\":\"p\","
            "\"account\":\"GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H\","
            "\"type\":\"data_sponsorship_removed\",\"type_i\":68,"
            "\"data_name\":\"config.memo\","
            "\"former_sponsor\":\"GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO\"}";
        DataSponsorshipRemovedEffectResponse effect;
        effect.loadFromJson(json);
        QCOMPARE(effect.getType(), QString("data_sponsorship_removed"));
        QCOMPARE(effect.getDataName(), QString("config.memo"));
        QCOMPARE(effect.getFormerSponsor(), QString("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"));
    }

    void testDeserializeClaimableBalanceSponsorshipCreatedEffect() {
        QByteArray json = "{"
            "\"created_at\":\"2026-09-28T15:16:12Z\",\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/p\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=p\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=p\"}},"
            "\"id\":\"i\",\"paging_token\":\"p\","
            "\"account\":\"GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H\","
            "\"type\":\"claimable_balance_sponsorship_created\",\"type_i\":69,"
            "\"balance_id\":\"00000000aaaa\","
            "\"sponsor\":\"GCFKT6BN2FEASCEVDNHEC4LLFT2KLUUPEMKM4OJPEJ65H2AEZ7IH4RV6\"}";
        ClaimableBalanceSponsorshipCreatedEffectResponse effect;
        effect.loadFromJson(json);
        QCOMPARE(effect.getType(), QString("claimable_balance_sponsorship_created"));
        QCOMPARE(effect.getBalanceID(), QString("00000000aaaa"));
        QCOMPARE(effect.getSponsor(), QString("GCFKT6BN2FEASCEVDNHEC4LLFT2KLUUPEMKM4OJPEJ65H2AEZ7IH4RV6"));
    }

    void testDeserializeClaimableBalanceSponsorshipUpdatedEffect() {
        QByteArray json = "{"
            "\"created_at\":\"2026-09-28T15:16:12Z\",\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/p\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=p\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=p\"}},"
            "\"id\":\"i\",\"paging_token\":\"p\","
            "\"account\":\"GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H\","
            "\"type\":\"claimable_balance_sponsorship_updated\",\"type_i\":70,"
            "\"balance_id\":\"00000000aaaa\","
            "\"former_sponsor\":\"GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO\","
            "\"new_sponsor\":\"GB24LPGAHYTWRYOXIDKXLI55SBRWW42T3TZKDAAW3BOJX4ADVIATFTLU\"}";
        ClaimableBalanceSponsorshipUpdatedEffectResponse effect;
        effect.loadFromJson(json);
        QCOMPARE(effect.getType(), QString("claimable_balance_sponsorship_updated"));
        QCOMPARE(effect.getBalanceID(), QString("00000000aaaa"));
        QCOMPARE(effect.getFormerSponsor(), QString("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"));
        QCOMPARE(effect.getNewSponsor(), QString("GB24LPGAHYTWRYOXIDKXLI55SBRWW42T3TZKDAAW3BOJX4ADVIATFTLU"));
    }

    void testDeserializeClaimableBalanceSponsorshipRemovedEffect() {
        QByteArray json = "{"
            "\"created_at\":\"2026-09-28T15:16:12Z\",\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/p\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=p\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=p\"}},"
            "\"id\":\"i\",\"paging_token\":\"p\","
            "\"account\":\"GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H\","
            "\"type\":\"claimable_balance_sponsorship_removed\",\"type_i\":71,"
            "\"balance_id\":\"00000000aaaa\","
            "\"former_sponsor\":\"GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO\"}";
        ClaimableBalanceSponsorshipRemovedEffectResponse effect;
        effect.loadFromJson(json);
        QCOMPARE(effect.getType(), QString("claimable_balance_sponsorship_removed"));
        QCOMPARE(effect.getBalanceID(), QString("00000000aaaa"));
        QCOMPARE(effect.getFormerSponsor(), QString("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"));
    }

    void testDeserializeSignerSponsorshipCreatedEffect() {
        QByteArray json = "{"
            "\"created_at\":\"2026-09-28T15:16:12Z\",\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/p\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=p\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=p\"}},"
            "\"id\":\"i\",\"paging_token\":\"p\","
            "\"account\":\"GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H\","
            "\"type\":\"signer_sponsorship_created\",\"type_i\":72,"
            "\"signer\":\"GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO\","
            "\"sponsor\":\"GCFKT6BN2FEASCEVDNHEC4LLFT2KLUUPEMKM4OJPEJ65H2AEZ7IH4RV6\"}";
        SignerSponsorshipCreatedEffectResponse effect;
        effect.loadFromJson(json);
        QCOMPARE(effect.getType(), QString("signer_sponsorship_created"));
        QCOMPARE(effect.getSigner(), QString("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"));
        QCOMPARE(effect.getSponsor(), QString("GCFKT6BN2FEASCEVDNHEC4LLFT2KLUUPEMKM4OJPEJ65H2AEZ7IH4RV6"));
    }

    void testDeserializeSignerSponsorshipUpdatedEffect() {
        QByteArray json = "{"
            "\"created_at\":\"2026-09-28T15:16:12Z\",\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/p\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=p\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=p\"}},"
            "\"id\":\"i\",\"paging_token\":\"p\","
            "\"account\":\"GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H\","
            "\"type\":\"signer_sponsorship_updated\",\"type_i\":73,"
            "\"signer\":\"GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO\","
            "\"former_sponsor\":\"GCFKT6BN2FEASCEVDNHEC4LLFT2KLUUPEMKM4OJPEJ65H2AEZ7IH4RV6\","
            "\"new_sponsor\":\"GB24LPGAHYTWRYOXIDKXLI55SBRWW42T3TZKDAAW3BOJX4ADVIATFTLU\"}";
        SignerSponsorshipUpdatedEffectResponse effect;
        effect.loadFromJson(json);
        QCOMPARE(effect.getType(), QString("signer_sponsorship_updated"));
        QCOMPARE(effect.getSigner(), QString("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"));
        QCOMPARE(effect.getFormerSponsor(), QString("GCFKT6BN2FEASCEVDNHEC4LLFT2KLUUPEMKM4OJPEJ65H2AEZ7IH4RV6"));
        QCOMPARE(effect.getNewSponsor(), QString("GB24LPGAHYTWRYOXIDKXLI55SBRWW42T3TZKDAAW3BOJX4ADVIATFTLU"));
    }

    void testDeserializeSignerSponsorshipRemovedEffect() {
        QByteArray json = "{"
            "\"created_at\":\"2026-09-28T15:16:12Z\",\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/p\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=p\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=p\"}},"
            "\"id\":\"i\",\"paging_token\":\"p\","
            "\"account\":\"GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H\","
            "\"type\":\"signer_sponsorship_removed\",\"type_i\":74,"
            "\"signer\":\"GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO\","
            "\"former_sponsor\":\"GCFKT6BN2FEASCEVDNHEC4LLFT2KLUUPEMKM4OJPEJ65H2AEZ7IH4RV6\"}";
        SignerSponsorshipRemovedEffectResponse effect;
        effect.loadFromJson(json);
        QCOMPARE(effect.getType(), QString("signer_sponsorship_removed"));
        QCOMPARE(effect.getSigner(), QString("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"));
        QCOMPARE(effect.getFormerSponsor(), QString("GCFKT6BN2FEASCEVDNHEC4LLFT2KLUUPEMKM4OJPEJ65H2AEZ7IH4RV6"));
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
    /** Real trustline sponsorship effects from Horizon testnet 29.0.0 (2026-09-30):
     *  created, updated (credit and pool-share) and removed. A pool-share
     *  trustline comes as asset_type "liquidity_pool" + liquidity_pool_id. */
    static QByteArray realPoolSponsorshipEffects()
    {
        return QByteArray(
            "{\"_embedded\":{\"records\":[{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21254732290990082\"},\"succeeds\":{"
            "\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21254732290990082-2\"},\"precedes\":{\"href\":\"https://horizon-testnet.stell"
            "ar.org/effects?order=asc&cursor=21254732290990082-2\"}},\"id\":\"0021254732290990082-0000000002\",\"paging_token\":\"21254732290990082-2\",\"account\""
            ":\"GBUXQKGGE3SL7IGDX6ZFMGACBBDDS6JSAPALZEMRVDO7M65YSNKHWICN\",\"type\":\"trustline_sponsorship_created\",\"type_i\":63,\"created_at\":\"2026-09-30T11:"
            "22:32Z\",\"asset_type\":\"liquidity_pool\",\"liquidity_pool_id\":\"e590400b7fdb37ac6eb79fa6ddf4f7842b1d679a8b9d515f47c724def2bac1b3\",\"sponsor\":\"GD"
            "3N5KQ4TM6N7IQZFBPTQSKCI3D5HDAOPXM33PG5HNP7LQVSWHT4NAIF\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/212573"
            "65105954819\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21257365105954819-1\"},\"precedes\":{\"href\":\""
            "https://horizon-testnet.stellar.org/effects?order=asc&cursor=21257365105954819-1\"}},\"id\":\"0021257365105954819-0000000001\",\"paging_token\":\"2125"
            "7365105954819-1\",\"account\":\"GBVO5STN7FUCPH3QNIQZXVBNUISKC3Z2YWTTOQZ3AI5XIKYGKQVG5X42\",\"type\":\"trustline_sponsorship_updated\",\"type_i\":64,\""
            "created_at\":\"2026-09-30T12:13:37Z\",\"asset_type\":\"credit_alphanum4\",\"asset\":\"UPD:GB2SY6GIMINOPCHWQ6DAMM632Z6FW2VOAHT4ENKYBXAP3NWQQXCB5GQI\",\""
            "former_sponsor\":\"GA24PJEU4I5RHAMQ2KKKCNLH2D5SWMZ3KK3EJONCWHLF4JQVD3S76HCK\",\"new_sponsor\":\"GDNF6H2HLUNAKZ3WGCOOOCX6U3Y3TW5OL33P77KSACARDG6G53HC33"
            "YG\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21257365105954820\"},\"succeeds\":{\"href\":\"https://hori"
            "zon-testnet.stellar.org/effects?order=desc&cursor=21257365105954820-1\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=as"
            "c&cursor=21257365105954820-1\"}},\"id\":\"0021257365105954820-0000000001\",\"paging_token\":\"21257365105954820-1\",\"account\":\"GBVO5STN7FUCPH3QNIQZ"
            "XVBNUISKC3Z2YWTTOQZ3AI5XIKYGKQVG5X42\",\"type\":\"trustline_sponsorship_updated\",\"type_i\":64,\"created_at\":\"2026-09-30T12:13:37Z\",\"asset_type\""
            ":\"liquidity_pool\",\"liquidity_pool_id\":\"0fe05edb0c9cde295fd2e15175c42628a1988f9dc8a1ecb23635cab692a1c6c1\",\"former_sponsor\":\"GA24PJEU4I5RHAMQ2K"
            "KKCNLH2D5SWMZ3KK3EJONCWHLF4JQVD3S76HCK\",\"new_sponsor\":\"GDNF6H2HLUNAKZ3WGCOOOCX6U3Y3TW5OL33P77KSACARDG6G53HC33YG\"},{\"_links\":{\"operation\":{\"h"
            "ref\":\"https://horizon-testnet.stellar.org/operations/21254796715499521\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order"
            "=desc&cursor=21254796715499521-1\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21254796715499521-1\"}},\"id"
            "\":\"0021254796715499521-0000000001\",\"paging_token\":\"21254796715499521-1\",\"account\":\"GBUXQKGGE3SL7IGDX6ZFMGACBBDDS6JSAPALZEMRVDO7M65YSNKHWICN\""
            ",\"type\":\"trustline_sponsorship_removed\",\"type_i\":65,\"created_at\":\"2026-09-30T11:23:47Z\",\"asset_type\":\"liquidity_pool\",\"liquidity_pool_i"
            "d\":\"e590400b7fdb37ac6eb79fa6ddf4f7842b1d679a8b9d515f47c724def2bac1b3\",\"former_sponsor\":\"GD3N5KQ4TM6N7IQZFBPTQSKCI3D5HDAOPXM33PG5HNP7LQVSWHT4NAIF"
            "\"}]}}");
    }
    static QByteArray realAccountCredited()
    {
        return QByteArray(
            "{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118341309550593\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118341309550593-1\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118341309550593-1\"}},\"id\":\"0021118341309550593-0000000001\",\"paging_token\":\"21118341309550593-1\",\"account\":\"GAPIM2WRYS65WEWE2PAJYNGST35ZQXEJJQKWG7EUVMEZ2LWP7SXSNTNO\",\"type\":\"account_credited\",\"type_i\":2,\"created_at\":\"2026-09-28T15:16:12Z\",\"asset_type\":\"credit_alphanum12\",\"asset_code\":\"LWTEST\",\"asset_issuer\":\"GAPIM2WRYS65WEWE2PAJYNGST35ZQXEJJQKWG7EUVMEZ2LWP7SXSNTNO\",\"amount\":\"99.9999000\"}");
    }
    static QByteArray realTrade()
    {
        return QByteArray(
            "{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118010597056513\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118010597056513-1\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118010597056513-1\"}},\"id\":\"0021118010597056513-0000000001\",\"paging_token\":\"21118010597056513-1\",\"account\":\"GDL75QQUCLJECIOHK2LPRW3YICWDS66QUTWX7FF3ZY4FJGYCKLRHRCJ2\",\"type\":\"trade\",\"type_i\":33,\"created_at\":\"2026-09-28T15:09:47Z\",\"seller\":\"GA6JV3FMUYNA7VHEN7DKHVLYHONNC4V7EWOJGQSJ337KMCRVKBXTZJBA\",\"offer_id\":\"828663\",\"sold_amount\":\"46.2962963\",\"sold_asset_type\":\"native\",\"bought_amount\":\"5.0000000\",\"bought_asset_type\":\"credit_alphanum4\",\"bought_asset_code\":\"USDC\",\"bought_asset_issuer\":\"GBBD47IF6LWK7P7MDEVSCWR7DPUWV3NY3DTQEVFL4NAT4AQH3ZLLFLA5\"}");
    }
    static QByteArray realThresholdsUpdated()
    {
        return QByteArray(
            "{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118221050462211\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118221050462211-1\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118221050462211-1\"}},\"id\":\"0021118221050462211-0000000001\",\"paging_token\":\"21118221050462211-1\",\"account\":\"GDSVLIT2LSWQCQ6ZOT225ETNPKDGJFPKOKER7PXJB54RDX43RGYOFI2E\",\"type\":\"account_thresholds_updated\",\"type_i\":4,\"created_at\":\"2026-09-28T15:13:52Z\",\"low_threshold\":0,\"med_threshold\":1,\"high_threshold\":1}");
    }
    /** A real testnet claimable_balance_claimant_created effect whose time is past the year 9999. */
    static QByteArray realClaimantCreated()
    {
        return QByteArray(
            "{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118938309996547\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118938309996547-2\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118938309996547-2\"}},\"id\":\"0021118938309996547-0000000002\",\"paging_token\":\"21118938309996547-2\",\"account\":\"GBTTX7QVGGB6VTNNAKMXIFR6YL3RYA6UU3YR7FVBLKNOB2SWSA7XMQ7H\",\"type\":\"claimable_balance_claimant_created\",\"type_i\":51,\"created_at\":\"2026-09-28T15:27:47Z\",\"asset\":\"native\",\"balance_id\":\"0000000046506400a6be6d45df1ffe333d0191a59a47331866537ac0e8cced3f0547a062\",\"amount\":\"3.5000000\",\"predicate\":{\"abs_before\":\"+146138514283-06-19T07:45:04Z\",\"abs_before_epoch\":\"4611686018427387904\"}}");
    }

private slots:
    /**
     * A claimant_created effect's predicate was never read: the property is
     * a Predicate*, which nothing could convert a JSON object into, and
     * predicate() was always null.
     */
    void testClaimantCreatedEffectReadsItsPredicate()
    {
        ClaimableBalanceClaimantCreatedEffectResponse effect;
        effect.loadFromJson(realClaimantCreated());
        QVERIFY(effect.predicate() != nullptr);
        auto* absBefore = dynamic_cast<Predicate::AbsBefore*>(effect.predicate());
        QVERIFY(absBefore != nullptr);
        QCOMPARE(absBefore->getTimestampSeconds(), Q_INT64_C(4611686018427387904));
        QCOMPARE(effect.getAmount(), QString("3.5000000"));
        // abs_before alone reads exactly past the year 9999 too (it used to
        // be refused because Qt could not read it); beside abs_before_epoch it
        // must name the same second.
        QVERIFY(refusalOf<ClaimableBalanceClaimantCreatedEffectResponse>(realClaimantCreated(), "predicate",
                    QJsonObject{{"abs_before", "+146138514283-06-19T07:45:04Z"}}).isEmpty());
        QVERIFY(refusalOf<ClaimableBalanceClaimantCreatedEffectResponse>(realClaimantCreated(), "predicate",
                    QJsonObject{{"abs_before", "+146138514283-06-19T07:45:05Z"},
                                {"abs_before_epoch", "4611686018427387904"}}).startsWith("predicate.abs_before: "));
        QVERIFY(refusalOf<ClaimableBalanceClaimantCreatedEffectResponse>(realClaimantCreated(), "predicate",
                    QJsonValue(QJsonValue::Undefined)).startsWith("predicate: "));
    }

    /**
     * What an effect moved, exactly: an amount given as a number (it was left
     * empty), a threshold or weight past a byte, a trade's asset whose code
     * does not fit its type.
     */
    void testEffectsAreReadExactly()
    {
        QVERIFY(refusalOf<AccountCreditedEffectResponse>(realAccountCredited(), "amount", 99.9999).startsWith("amount: "));
        QVERIFY(refusalOf<AccountCreditedEffectResponse>(realAccountCredited(), "amount", QJsonValue(QJsonValue::Undefined)).startsWith("amount: "));
        QVERIFY(refusalOf<AccountCreditedEffectResponse>(realAccountCredited(), "asset_code", QString("LW")).startsWith("asset_code: "));
        QVERIFY(refusalOf<AccountCreditedEffectResponse>(realAccountCredited(), "paging_token", QJsonValue()).startsWith("paging_token: "));
        QVERIFY(refusalOf<TradeEffectResponse>(realTrade(), "sold_amount", QString("46.29629630")).startsWith("sold_amount: "));
        QVERIFY(refusalOf<TradeEffectResponse>(realTrade(), "bought_asset_type", QString("credit_alphanum12")).startsWith("bought_asset_code: "));
        QVERIFY(refusalOf<TradeEffectResponse>(realTrade(), "offer_id", QString("828663.0")).startsWith("offer_id: "));
        QVERIFY(refusalOf<AccountThresholdsUpdatedEffectResponse>(realThresholdsUpdated(), "high_threshold", 300).startsWith("high_threshold: "));

        TradeEffectResponse trade;
        trade.loadFromJson(realTrade());
        QCOMPARE(trade.getSoldAmount(), QString("46.2962963"));
        QCOMPARE(trade.getOfferId(), Q_INT64_C(828663));
    }

    /**
     * A record's type_i picks its class. It was read with toInt(): missing,
     * or given as text, it read 0, and the record became an account_created
     * effect.
     */
    void testEffectTypeIsReadExactly()
    {
        QJsonObject page{{"_embedded", QJsonObject{{"records", QJsonArray{
            QJsonDocument::fromJson(realAccountCredited()).object()}}}}};
        EffectsPage read;
        read.loadFromJson(QJsonDocument(page).toJson());
        QVERIFY(dynamic_cast<AccountCreditedEffectResponse*>(read.getRecords().at(0)) != nullptr);
        QVERIFY(refusalOf<EffectsPage>(QJsonDocument(page).toJson(), "_embedded.records[0].type_i",
                                       QJsonValue(QJsonValue::Undefined)).startsWith("_embedded.records[0].type_i: "));
        QVERIFY(refusalOf<EffectsPage>(QJsonDocument(page).toJson(), "_embedded.records[0].type_i",
                                       QString("account_credited")).startsWith("_embedded.records[0].type_i: "));
    }

    /** One real effect of each type Horizon returned, testnet and pubnet, loads. */
    void testRealHorizonEffectsOfEveryType()
    {
        EffectsPage effects;
        effects.loadFromJson(realEffectsOfEveryType());
        const QJsonArray records = QJsonDocument::fromJson(realEffectsOfEveryType()).object()
                .value("_embedded").toObject().value("records").toArray();
        QCOMPARE(effects.size(), records.size());
        QVERIFY(effects.size() >= 40);
        for (int i = 0; i < effects.size(); ++i) {
            EffectResponse* effect = dynamic_cast<EffectResponse*>(effects.getRecords().at(i));
            QVERIFY(effect != nullptr);
            QCOMPARE(effect->getPagingToken(), records.at(i).toObject().value("paging_token").toString());
            if (auto* credited = dynamic_cast<AccountCreditedEffectResponse*>(effect))
                QCOMPARE(credited->getAmount(), records.at(i).toObject().value("amount").toString());
            if (auto* claimant = dynamic_cast<ClaimableBalanceClaimantCreatedEffectResponse*>(effect))
                QVERIFY(claimant->predicate() != nullptr);
        }
    }
private:
    /** One real effect of each type Horizon (testnet and pubnet) returned. */
    static QByteArray realEffectsOfEveryType()
    {
        return QByteArray(
            "{\"_embedded\":{\"records\":[{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118341309534209\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118341309534209-1\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118341309534209-1\"}},\"id\":\"0021118341309534209-0000000001\",\"paging_token\":\"21118341309534209-1\",\"account\":\"GDQYIZ7FCW7FLARWBZ332K2ZKLXIEY4SX66DLVKYJI3J6G4JBRBC4DHW\",\"type\":\"account_created\",\"type_i\":0,\"created_at\":\"2026-09-28T15:16:12Z\",\"starting_balance\":\"10000.0000000\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118341309550593\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118341309550593-1\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118341309550593-1\"}},\"id\":\"0021118341309550593-0000000001\",\"paging_token\":\"21118341309550593-1\",\"account\":\"GAPIM2WRYS65WEWE2PAJYNGST35ZQXEJJQKWG7EUVMEZ2LWP7SXSNTNO\",\"type\":\"account_credited\",\"type_i\":2,\"created_at\":\"2026-09-28T15:16:12Z\",\"asset_type\":\"credit_alphanum12\",\"asset_code\":\"LWTEST\",\"asset_issuer\":\"GAPIM2WRYS65WEWE2PAJYNGST35ZQXEJJQKWG7EUVMEZ2LWP7SXSNTNO\",\"amount\":\"99.9999000\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118341309534209\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118341309534209-2\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118341309534209-2\"}},\"id\":\"0021118341309534209-0000000002\",\"paging_token\":\"21118341309534209-2\",\"account\":\"GAIH3ULLFQ4DGSECF2AR555KZ4KNDGEKN4AFI4SU2M7B43MGK3QJZNSR\",\"type\":\"account_debited\",\"type_i\":3,\"creat"
            "ed_at\":\"2026-09-28T15:16:12Z\",\"asset_type\":\"native\",\"amount\":\"10000.0000000\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118895360315393\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118895360315393-3\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118895360315393-3\"}},\"id\":\"0021118895360315393-0000000003\",\"paging_token\":\"21118895360315393-3\",\"account\":\"GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD\",\"type\":\"account_flags_updated\",\"type_i\":6,\"created_at\":\"2026-09-28T15:26:57Z\",\"auth_required_flag\":true},{\"_links\":{\"operation\":{\"href\":\"https://horizon.stellar.org/operations/277730503963000834\"},\"succeeds\":{\"href\":\"https://horizon.stellar.org/effects?order=desc&cursor=277730503963000834-1\"},\"precedes\":{\"href\":\"https://horizon.stellar.org/effects?order=asc&cursor=277730503963000834-1\"}},\"id\":\"0277730503963000834-0000000001\",\"paging_token\":\"277730503963000834-1\",\"account\":\"GA3BGFZMXRGSOFSMWG4TJRNP3BTTP5DZWUYM3LENPJ6TYB3ETGYB6NVY\",\"type\":\"account_home_domain_updated\",\"type_i\":5,\"created_at\":\"2026-09-28T14:44:01Z\",\"home_domain\":\"lobstr.co\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon.stellar.org/operations/277730503963000834\"},\"succeeds\":{\"href\":\"https://horizon.stellar.org/effects?order=desc&cursor=277730503963000834-2\"},\"precedes\":{\"href\":\"https://horizon.stellar.org/effects?order=asc&cursor=277730503963000834-2\"}},\"id\":\"0277730503963000834-0000000002\",\"paging_token\":\"277730503963000834-2\",\"account\":\"GA3BGFZMXRGSOFSMWG4TJRNP3BTTP5DZWUYM3LENPJ6TYB3ETGYB6NVY\",\"type\":\"account_inflation_destination_updated\",\"type_i\":7,\"created_at\":\"2026-09-28T14:44:01Z\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon-"
            "testnet.stellar.org/operations/21118341309550595\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118341309550595-3\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118341309550595-3\"}},\"id\":\"0021118341309550595-0000000003\",\"paging_token\":\"21118341309550595-3\",\"account\":\"GDTBRJDRRQHHH4EPXOIBEAXTDJVETYVBOO53UGD4NDTW5P7YKG2DAWBD\",\"type\":\"account_removed\",\"type_i\":1,\"created_at\":\"2026-09-28T15:16:12Z\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118221050449922\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118221050449922-4\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118221050449922-4\"}},\"id\":\"0021118221050449922-0000000004\",\"paging_token\":\"21118221050449922-4\",\"account\":\"GCRYREIBNPYMSL436Q6VBGWWQUZYYOFFZALPN77ZHVRNXBVYSPFITNDO\",\"type\":\"account_sponsorship_created\",\"type_i\":60,\"created_at\":\"2026-09-28T15:13:52Z\",\"sponsor\":\"GCBS7GI62E2XVLK2O42KTCRR5RRNFZVKUVDHULPX4TIYGNWAZFML5BJQ\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21116387099414531\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21116387099414531-4\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21116387099414531-4\"}},\"id\":\"0021116387099414531-0000000004\",\"paging_token\":\"21116387099414531-4\",\"account\":\"GAALG4JHVZERTOLKMAZOOJPFXTO4QODLWRXXEE645YZOJFIXGBYJ73LG\",\"type\":\"account_sponsorship_removed\",\"type_i\":62,\"created_at\":\"2026-09-28T14:38:17Z\",\"former_sponsor\":\"GAKCMMDBUAUPOX4WJXIH7VQDBR4JI46PLGMU6AI2LEZQJAPYHMH3MKPQ\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/ope"
            "rations/21118221050462211\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118221050462211-1\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118221050462211-1\"}},\"id\":\"0021118221050462211-0000000001\",\"paging_token\":\"21118221050462211-1\",\"account\":\"GDSVLIT2LSWQCQ6ZOT225ETNPKDGJFPKOKER7PXJB54RDX43RGYOFI2E\",\"type\":\"account_thresholds_updated\",\"type_i\":4,\"created_at\":\"2026-09-28T15:13:52Z\",\"low_threshold\":0,\"med_threshold\":1,\"high_threshold\":1},{\"_links\":{\"operation\":{\"href\":\"https://horizon.stellar.org/operations/277732157525843969\"},\"succeeds\":{\"href\":\"https://horizon.stellar.org/effects?order=desc&cursor=277732157525843969-2\"},\"precedes\":{\"href\":\"https://horizon.stellar.org/effects?order=asc&cursor=277732157525843969-2\"}},\"id\":\"0277732157525843969-0000000002\",\"paging_token\":\"277732157525843969-2\",\"account\":\"GAMIZAC6BZRUXGXUFWRVT4IAZQMB2E52MP6FOHTTTQ24USWZPVUF5WS6\",\"type\":\"claimable_balance_claimant_created\",\"type_i\":51,\"created_at\":\"2026-09-28T15:16:06Z\",\"asset\":\"PALLADIUM:GAJALNL2LHOKXQYQ7GGGS22WE7TFNYUDCKDSRIY32AYMCW7QIUZGEFU4\",\"balance_id\":\"000000006b3d53f282d357d017ac5ff48278766401f9e5668c5a5b7c9b44d06882b1b8ca\",\"amount\":\"535454111111.0000000\",\"predicate\":{\"not\":{\"abs_before\":\"2026-09-29T15:15:52Z\",\"abs_before_epoch\":\"1790694952\"}}},{\"_links\":{\"operation\":{\"href\":\"https://horizon.stellar.org/operations/277732157525614594\"},\"succeeds\":{\"href\":\"https://horizon.stellar.org/effects?order=desc&cursor=277732157525614594-1\"},\"precedes\":{\"href\":\"https://horizon.stellar.org/effects?order=asc&cursor=277732157525614594-1\"}},\"id\":\"0277732157525614594-0000000001\",\"paging_token\":\"277732157525614594-1\",\"account\":\"GALHHYP23D6TNX653MBMYH7MYTAF7XL5TT3MLMBI3FZ4F56OMCJ7LYMC\","
            "\"type\":\"claimable_balance_claimed\",\"type_i\":52,\"created_at\":\"2026-09-28T15:16:06Z\",\"asset\":\"JPNXRP:GDUNTDNUBPVUDEDS4N3AFFUU5WZWC4AHPAJSMW3EKQJA5HG5CDP2JPNB\",\"balance_id\":\"00000000ebe4dd445a133fa3ed6d45599f0d811442dfad0f57499013afbbeda61d9738b1\",\"amount\":\"1.1984659\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118946899927041\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118946899927041-1\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118946899927041-1\"}},\"id\":\"0021118946899927041-0000000001\",\"paging_token\":\"21118946899927041-1\",\"account\":\"GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD\",\"type\":\"claimable_balance_clawed_back\",\"type_i\":80,\"created_at\":\"2026-09-28T15:27:57Z\",\"balance_id\":\"00000000346974acd1e0cb35a34bfdb61f511792cb92af49acacce91f81bf32e10e24f05\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon.stellar.org/operations/277732157525843969\"},\"succeeds\":{\"href\":\"https://horizon.stellar.org/effects?order=desc&cursor=277732157525843969-1\"},\"precedes\":{\"href\":\"https://horizon.stellar.org/effects?order=asc&cursor=277732157525843969-1\"}},\"id\":\"0277732157525843969-0000000001\",\"paging_token\":\"277732157525843969-1\",\"account\":\"GAMIZAC6BZRUXGXUFWRVT4IAZQMB2E52MP6FOHTTTQ24USWZPVUF5WS6\",\"type\":\"claimable_balance_created\",\"type_i\":50,\"created_at\":\"2026-09-28T15:16:06Z\",\"asset\":\"PALLADIUM:GAJALNL2LHOKXQYQ7GGGS22WE7TFNYUDCKDSRIY32AYMCW7QIUZGEFU4\",\"balance_id\":\"000000006b3d53f282d357d017ac5ff48278766401f9e5668c5a5b7c9b44d06882b1b8ca\",\"amount\":\"535454111111.0000000\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon.stellar.org/operations/277732157525843969\"},\"succeeds\":{\"href\":\"https://horizon.stellar.org/effects?order=desc&"
            "cursor=277732157525843969-5\"},\"precedes\":{\"href\":\"https://horizon.stellar.org/effects?order=asc&cursor=277732157525843969-5\"}},\"id\":\"0277732157525843969-0000000005\",\"paging_token\":\"277732157525843969-5\",\"account\":\"GAMIZAC6BZRUXGXUFWRVT4IAZQMB2E52MP6FOHTTTQ24USWZPVUF5WS6\",\"type\":\"claimable_balance_sponsorship_created\",\"type_i\":69,\"created_at\":\"2026-09-28T15:16:06Z\",\"balance_id\":\"000000006b3d53f282d357d017ac5ff48278766401f9e5668c5a5b7c9b44d06882b1b8ca\",\"sponsor\":\"GAMIZAC6BZRUXGXUFWRVT4IAZQMB2E52MP6FOHTTTQ24USWZPVUF5WS6\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon.stellar.org/operations/277732157525614594\"},\"succeeds\":{\"href\":\"https://horizon.stellar.org/effects?order=desc&cursor=277732157525614594-3\"},\"precedes\":{\"href\":\"https://horizon.stellar.org/effects?order=asc&cursor=277732157525614594-3\"}},\"id\":\"0277732157525614594-0000000003\",\"paging_token\":\"277732157525614594-3\",\"account\":\"GALHHYP23D6TNX653MBMYH7MYTAF7XL5TT3MLMBI3FZ4F56OMCJ7LYMC\",\"type\":\"claimable_balance_sponsorship_removed\",\"type_i\":71,\"created_at\":\"2026-09-28T15:16:06Z\",\"balance_id\":\"00000000ebe4dd445a133fa3ed6d45599f0d811442dfad0f57499013afbbeda61d9738b1\",\"former_sponsor\":\"GDQD6RR3BVCRNGMZNU2T3BW42B5LU5R3VNMSO63KA2AOQLCPTEJLJPNB\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118968374763522\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118968374763522-1\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118968374763522-1\"}},\"id\":\"0021118968374763522-0000000001\",\"paging_token\":\"21118968374763522-1\",\"account\":\"GCALKXRTML6DHW2CT332CVAOYEWZQ2OBXGHW3YNY4ZLOE2QE3OFH7LLA\",\"type\":\"claimable_balance_sponsorship_updated\",\"type_i\":70,\"created_at\":\"2026-09-28T15:28:22Z\",\"balance"
            "_id\":\"00000000b55cca12a4ac2a146685c99be06782b4380cb0d79524cc22e1fec16106a02322\",\"former_sponsor\":\"GCALKXRTML6DHW2CT332CVAOYEWZQ2OBXGHW3YNY4ZLOE2QE3OFH7LLA\",\"new_sponsor\":\"GDEZMR6VMLXNPBJL4UJHJQXJ3NXVQ3PMC2FTQU6F5QAKEBRJNIAXY3BQ\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118341309595649\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118341309595649-4\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118341309595649-4\"}},\"id\":\"0021118341309595649-0000000004\",\"paging_token\":\"21118341309595649-4\",\"account\":\"GDBXA45UBW2O3UH2RJOCOBXRGEMIP5745RQRINZZ2WHKECHHKKUWDOBH\",\"type\":\"contract_credited\",\"type_i\":96,\"created_at\":\"2026-09-28T15:16:12Z\",\"asset_type\":\"native\",\"contract\":\"CAYPAQDKNWMHRATKU5DQ327VDHVRSIVK7UGVWT2A5SUZCUFTLUHXH2JA\",\"amount\":\"490.0000000\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118341309595649\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118341309595649-1\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118341309595649-1\"}},\"id\":\"0021118341309595649-0000000001\",\"paging_token\":\"21118341309595649-1\",\"account\":\"GDBXA45UBW2O3UH2RJOCOBXRGEMIP5745RQRINZZ2WHKECHHKKUWDOBH\",\"type\":\"contract_debited\",\"type_i\":97,\"created_at\":\"2026-09-28T15:16:12Z\",\"asset_type\":\"native\",\"contract\":\"CAYPAQDKNWMHRATKU5DQ327VDHVRSIVK7UGVWT2A5SUZCUFTLUHXH2JA\",\"amount\":\"490.0000000\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118959784837122\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118959784837122-1\"},\"precedes\":{\"href\":\"https://horizon-testnet.st"
            "ellar.org/effects?order=asc&cursor=21118959784837122-1\"}},\"id\":\"0021118959784837122-0000000001\",\"paging_token\":\"21118959784837122-1\",\"account\":\"GBTTX7QVGGB6VTNNAKMXIFR6YL3RYA6UU3YR7FVBLKNOB2SWSA7XMQ7H\",\"type\":\"data_created\",\"type_i\":40,\"created_at\":\"2026-09-28T15:28:12Z\",\"name\":\"sp\",\"value\":\"c3BvbnNvcmVk\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118903950249985\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118903950249985-1\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118903950249985-1\"}},\"id\":\"0021118903950249985-0000000001\",\"paging_token\":\"21118903950249985-1\",\"account\":\"GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD\",\"type\":\"data_removed\",\"type_i\":41,\"created_at\":\"2026-09-28T15:27:07Z\",\"name\":\"k2\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118959784837122\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118959784837122-2\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118959784837122-2\"}},\"id\":\"0021118959784837122-0000000002\",\"paging_token\":\"21118959784837122-2\",\"account\":\"GBTTX7QVGGB6VTNNAKMXIFR6YL3RYA6UU3YR7FVBLKNOB2SWSA7XMQ7H\",\"type\":\"data_sponsorship_created\",\"type_i\":66,\"created_at\":\"2026-09-28T15:28:12Z\",\"data_name\":\"sp\",\"sponsor\":\"GCALKXRTML6DHW2CT332CVAOYEWZQ2OBXGHW3YNY4ZLOE2QE3OFH7LLA\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21119071453978626\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21119071453978626-1\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21119071"
            "453978626-1\"}},\"id\":\"0021119071453978626-0000000001\",\"paging_token\":\"21119071453978626-1\",\"account\":\"GCALKXRTML6DHW2CT332CVAOYEWZQ2OBXGHW3YNY4ZLOE2QE3OFH7LLA\",\"type\":\"data_sponsorship_removed\",\"type_i\":68,\"created_at\":\"2026-09-28T15:30:22Z\",\"data_name\":\"sp\",\"former_sponsor\":\"GCALKXRTML6DHW2CT332CVAOYEWZQ2OBXGHW3YNY4ZLOE2QE3OFH7LLA\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118899655299073\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118899655299073-1\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118899655299073-1\"}},\"id\":\"0021118899655299073-0000000001\",\"paging_token\":\"21118899655299073-1\",\"account\":\"GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD\",\"type\":\"data_updated\",\"type_i\":42,\"created_at\":\"2026-09-28T15:27:02Z\",\"name\":\"k2\",\"value\":\"dw==\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118921130135553\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118921130135553-2\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118921130135553-2\"}},\"id\":\"0021118921130135553-0000000002\",\"paging_token\":\"21118921130135553-2\",\"account\":\"GBTTX7QVGGB6VTNNAKMXIFR6YL3RYA6UU3YR7FVBLKNOB2SWSA7XMQ7H\",\"type\":\"liquidity_pool_created\",\"type_i\":93,\"created_at\":\"2026-09-28T15:27:27Z\",\"liquidity_pool\":{\"id\":\"3fc7b1d1333b185f1bf872174d7c362b7f2f1fcbe41c396a42ddbeb79b3b2dc4\",\"fee_bp\":30,\"type\":\"constant_product\",\"total_trustlines\":\"1\",\"total_shares\":\"0.0000000\",\"reserves\":[{\"asset\":\"native\",\"amount\":\"0.0000000\"},{\"asset\":\"TEST:GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD\",\"amount\":\"0.0000000\"}"
            "]}},{\"_links\":{\"operation\":{\"href\":\"https://horizon.stellar.org/operations/277731706554052609\"},\"succeeds\":{\"href\":\"https://horizon.stellar.org/effects?order=desc&cursor=277731706554052609-1\"},\"precedes\":{\"href\":\"https://horizon.stellar.org/effects?order=asc&cursor=277731706554052609-1\"}},\"id\":\"0277731706554052609-0000000001\",\"paging_token\":\"277731706554052609-1\",\"account\":\"GDQVJIGZ5TPOQLMVUQ6AWX5XZNE3YDEL5UXAOM3574U4HGNKS2LE4RMD\",\"type\":\"liquidity_pool_deposited\",\"type_i\":90,\"created_at\":\"2026-09-28T15:07:21Z\",\"liquidity_pool\":{\"id\":\"ad45b80a3a90ccaa251375e208006651a9f7242adc73e0d6928b0f3b2b0255c2\",\"fee_bp\":30,\"type\":\"constant_product\",\"total_trustlines\":\"212\",\"total_shares\":\"362955168.5792683\",\"reserves\":[{\"asset\":\"SSLX:GBHFGY3ZNEJWLNO4LBUKLYOCEK4V7ENEBJGPRHHX7JU47GWHBREH37UR\",\"amount\":\"35788739.0536156\"},{\"asset\":\"STARDUST:GCPLNANLXTV2G6NPIRDPOLM5NAO5XH2IPYQBZCTTAKPNV4IRTU5X5B4S\",\"amount\":\"21314653013.0547397\"}]},\"reserves_deposited\":[{\"asset\":\"SSLX:GBHFGY3ZNEJWLNO4LBUKLYOCEK4V7ENEBJGPRHHX7JU47GWHBREH37UR\",\"amount\":\"2128.4295531\"},{\"asset\":\"STARDUST:GCPLNANLXTV2G6NPIRDPOLM5NAO5XH2IPYQBZCTTAKPNV4IRTU5X5B4S\",\"amount\":\"1267626.0350791\"}],\"shares_received\":\"21585.6866624\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118981259677697\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118981259677697-9\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118981259677697-9\"}},\"id\":\"0021118981259677697-0000000009\",\"paging_token\":\"21118981259677697-9\",\"account\":\"GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD\",\"type\":\"liquidity_pool_removed\",\"type_i\":94,\"created_at\":\"2026-09-28T15:28:37Z\",\"liquidity_pool_id\":\"3fc7b1d133"
            "3b185f1bf872174d7c362b7f2f1fcbe41c396a42ddbeb79b3b2dc4\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118981259677697\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118981259677697-6\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118981259677697-6\"}},\"id\":\"0021118981259677697-0000000006\",\"paging_token\":\"21118981259677697-6\",\"account\":\"GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD\",\"type\":\"liquidity_pool_revoked\",\"type_i\":95,\"created_at\":\"2026-09-28T15:28:37Z\",\"liquidity_pool\":{\"id\":\"3fc7b1d1333b185f1bf872174d7c362b7f2f1fcbe41c396a42ddbeb79b3b2dc4\",\"fee_bp\":30,\"type\":\"constant_product\",\"total_trustlines\":\"1\",\"total_shares\":\"306.2277660\",\"reserves\":[{\"asset\":\"native\",\"amount\":\"101.6796085\"},{\"asset\":\"TEST:GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD\",\"amount\":\"922.3957932\"}]},\"reserves_revoked\":[{\"asset\":\"native\",\"amount\":\"101.6796085\",\"claimable_balance_id\":\"0000000086e9d33c7a852303b8fda90e4286f9c49f6cf16f9c53643ad3d22168aa22f0a7\"},{\"asset\":\"TEST:GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD\",\"amount\":\"922.3957932\",\"claimable_balance_id\":\"0000000007cef03fa253725b9b3e5a499ea7f10587cc4cbaa40495a3867a9dd3eeff5c16\"}],\"shares_revoked\":\"306.2277660\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118341309562881\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118341309562881-3\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118341309562881-3\"}},\"id\":\"0021118341309562881-0000000003\",\"paging_token\":\"21118341309562881-3\",\"account\":\"GBMXZ62UDDWDWZU464HDDFGO6LZ4IAFPHZTIU3JPODNZQFAUSCIRMHCM"
            "\",\"type\":\"liquidity_pool_trade\",\"type_i\":92,\"created_at\":\"2026-09-28T15:16:12Z\",\"liquidity_pool\":{\"id\":\"7e097aed26baacc53176bb3ae4722386135104eb044be64d21c2a4233840a64f\",\"fee_bp\":30,\"type\":\"constant_product\",\"total_trustlines\":\"1\",\"total_shares\":\"31622.7766016\",\"reserves\":[{\"asset\":\"USDC:GCDXYIOQJXFYG42J6C4K4NN7TGMX4TFF5DZK2ENDMNHONRR5D3KIPDS5\",\"amount\":\"1490.8080916\"},{\"asset\":\"SMAJDV:GBHOK32DC5HPBQUQTUPK5Y66A27MSBQGK4KDRLP4BGEQFL5O3AFTHIH5\",\"amount\":\"719313.8755264\"}]},\"sold\":{\"asset\":\"USDC:GCDXYIOQJXFYG42J6C4K4NN7TGMX4TFF5DZK2ENDMNHONRR5D3KIPDS5\",\"amount\":\"3.7153200\"},\"bought\":{\"asset\":\"SMAJDV:GBHOK32DC5HPBQUQTUPK5Y66A27MSBQGK4KDRLP4BGEQFL5O3AFTHIH5\",\"amount\":\"1793.5502249\"}},{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118934015029249\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118934015029249-1\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118934015029249-1\"}},\"id\":\"0021118934015029249-0000000001\",\"paging_token\":\"21118934015029249-1\",\"account\":\"GBTTX7QVGGB6VTNNAKMXIFR6YL3RYA6UU3YR7FVBLKNOB2SWSA7XMQ7H\",\"type\":\"liquidity_pool_withdrew\",\"type_i\":91,\"created_at\":\"2026-09-28T15:27:42Z\",\"liquidity_pool\":{\"id\":\"3fc7b1d1333b185f1bf872174d7c362b7f2f1fcbe41c396a42ddbeb79b3b2dc4\",\"fee_bp\":30,\"type\":\"constant_product\",\"total_trustlines\":\"1\",\"total_shares\":\"306.2277660\",\"reserves\":[{\"asset\":\"native\",\"amount\":\"101.6796085\"},{\"asset\":\"TEST:GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD\",\"amount\":\"922.3957932\"}]},\"reserves_received\":[{\"asset\":\"native\",\"amount\":\"3.3203915\"},{\"asset\":\"TEST:GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD\",\"amount\":\"30.1212331\"}],\"shares_redeemed\":\""
            "10.0000000\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118899655299075\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118899655299075-1\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118899655299075-1\"}},\"id\":\"0021118899655299075-0000000001\",\"paging_token\":\"21118899655299075-1\",\"account\":\"GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD\",\"type\":\"sequence_bumped\",\"type_i\":43,\"created_at\":\"2026-09-28T15:27:02Z\",\"new_seq\":\"21118873885474917\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118341309534209\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118341309534209-3\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118341309534209-3\"}},\"id\":\"0021118341309534209-0000000003\",\"paging_token\":\"21118341309534209-3\",\"account\":\"GDQYIZ7FCW7FLARWBZ332K2ZKLXIEY4SX66DLVKYJI3J6G4JBRBC4DHW\",\"type\":\"signer_created\",\"type_i\":10,\"created_at\":\"2026-09-28T15:16:12Z\",\"weight\":1,\"public_key\":\"GDQYIZ7FCW7FLARWBZ332K2ZKLXIEY4SX66DLVKYJI3J6G4JBRBC4DHW\",\"key\":\"\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118221050462209\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118221050462209-1\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118221050462209-1\"}},\"id\":\"0021118221050462209-0000000001\",\"paging_token\":\"21118221050462209-1\",\"account\":\"GDSVLIT2LSWQCQ6ZOT225ETNPKDGJFPKOKER7PXJB54RDX43RGYOFI2E\",\"type\":\"signer_removed\",\"type_i\":11,\"created_at\":\"2026-09-28T15:13:52Z\",\"weight\":0,\"public_key\":\"GCBBMGCVLQL2GODKYKKS2P"
            "SF55LAJJYLBCVENMHMWEUO4ZHOV4O4Q5RE\",\"key\":\"\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21116417164185605\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21116417164185605-2\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21116417164185605-2\"}},\"id\":\"0021116417164185605-0000000002\",\"paging_token\":\"21116417164185605-2\",\"account\":\"GBGQMGNZ7ONGS2YING5XP5IFFTZGUF4KJ7NOOP5NVARGWPIBPU4ONQXL\",\"type\":\"signer_sponsorship_created\",\"type_i\":72,\"created_at\":\"2026-09-28T14:38:52Z\",\"signer\":\"GDYWSXWCYXKTU6LD7EEPZWPAS2PRDUDHTFLRU5ICVRY2OUVVUJBIWIXZ\",\"sponsor\":\"GAKCMMDBUAUPOX4WJXIH7VQDBR4JI46PLGMU6AI2LEZQJAPYHMH3MKPQ\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21116387099414531\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21116387099414531-5\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21116387099414531-5\"}},\"id\":\"0021116387099414531-0000000005\",\"paging_token\":\"21116387099414531-5\",\"account\":\"GAALG4JHVZERTOLKMAZOOJPFXTO4QODLWRXXEE645YZOJFIXGBYJ73LG\",\"type\":\"signer_sponsorship_removed\",\"type_i\":74,\"created_at\":\"2026-09-28T14:38:17Z\",\"signer\":\"GAHXE3KYDP2QV6Y4HELGQ7RPFAMTJB6CKZYICDG6QEJIZP35TDIIOCOH\",\"former_sponsor\":\"GAKCMMDBUAUPOX4WJXIH7VQDBR4JI46PLGMU6AI2LEZQJAPYHMH3MKPQ\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118899655299074\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118899655299074-1\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118899655299074-1\"}},\"id\":\"0021118899655299074-0000000001\",\"paging_token\":\"21118899"
            "655299074-1\",\"account\":\"GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD\",\"type\":\"signer_updated\",\"type_i\":12,\"created_at\":\"2026-09-28T15:27:02Z\",\"weight\":2,\"public_key\":\"GCALKXRTML6DHW2CT332CVAOYEWZQ2OBXGHW3YNY4ZLOE2QE3OFH7LLA\",\"key\":\"\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118010597056513\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118010597056513-1\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118010597056513-1\"}},\"id\":\"0021118010597056513-0000000001\",\"paging_token\":\"21118010597056513-1\",\"account\":\"GDL75QQUCLJECIOHK2LPRW3YICWDS66QUTWX7FF3ZY4FJGYCKLRHRCJ2\",\"type\":\"trade\",\"type_i\":33,\"created_at\":\"2026-09-28T15:09:47Z\",\"seller\":\"GA6JV3FMUYNA7VHEN7DKHVLYHONNC4V7EWOJGQSJ337KMCRVKBXTZJBA\",\"offer_id\":\"828663\",\"sold_amount\":\"46.2962963\",\"sold_asset_type\":\"native\",\"bought_amount\":\"5.0000000\",\"bought_asset_type\":\"credit_alphanum4\",\"bought_asset_code\":\"USDC\",\"bought_asset_issuer\":\"GBBD47IF6LWK7P7MDEVSCWR7DPUWV3NY3DTQEVFL4NAT4AQH3ZLLFLA5\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118100791373825\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118100791373825-1\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118100791373825-1\"}},\"id\":\"0021118100791373825-0000000001\",\"paging_token\":\"21118100791373825-1\",\"account\":\"GANCKFGWXAEMJ53GP6VIEHIHRCFO3CIG5ID54WEO7YDR32Y6OG4N2PTU\",\"type\":\"trustline_authorized\",\"type_i\":23,\"created_at\":\"2026-09-28T15:11:32Z\",\"trustor\":\"GDRVT4WWI4DYCLXU5UP7YCR322KBPDWVEUQSMDAVNNLPPNVDOVIVRFKU\",\"asset_type\":\"credit_alphanum4\",\"asset_code\":\"USD\"},{\"_links\":{\"ope"
            "ration\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118311244800001\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118311244800001-1\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118311244800001-1\"}},\"id\":\"0021118311244800001-0000000001\",\"paging_token\":\"21118311244800001-1\",\"account\":\"GDTBRJDRRQHHH4EPXOIBEAXTDJVETYVBOO53UGD4NDTW5P7YKG2DAWBD\",\"type\":\"trustline_created\",\"type_i\":20,\"created_at\":\"2026-09-28T15:15:37Z\",\"asset_type\":\"credit_alphanum12\",\"asset_code\":\"LWTEST\",\"asset_issuer\":\"GAPIM2WRYS65WEWE2PAJYNGST35ZQXEJJQKWG7EUVMEZ2LWP7SXSNTNO\",\"limit\":\"1000.0000000\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118981259677699\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118981259677699-1\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118981259677699-1\"}},\"id\":\"0021118981259677699-0000000001\",\"paging_token\":\"21118981259677699-1\",\"account\":\"GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD\",\"type\":\"trustline_deauthorized\",\"type_i\":24,\"created_at\":\"2026-09-28T15:28:37Z\",\"trustor\":\"GBTTX7QVGGB6VTNNAKMXIFR6YL3RYA6UU3YR7FVBLKNOB2SWSA7XMQ7H\",\"asset_type\":\"credit_alphanum4\",\"asset_code\":\"TWO\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118100791373825\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118100791373825-2\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118100791373825-2\"}},\"id\":\"0021118100791373825-0000000002\",\"paging_token\":\"21118100791373825-2\",\"account\":\"GANCKFGWXAEMJ53GP6VIEHIHRCFO3CIG5ID54WEO7YDR32Y6OG4"
            "N2PTU\",\"type\":\"trustline_flags_updated\",\"type_i\":26,\"created_at\":\"2026-09-28T15:11:32Z\",\"asset_type\":\"credit_alphanum4\",\"asset_code\":\"USD\",\"asset_issuer\":\"GANCKFGWXAEMJ53GP6VIEHIHRCFO3CIG5ID54WEO7YDR32Y6OG4N2PTU\",\"trustor\":\"GDRVT4WWI4DYCLXU5UP7YCR322KBPDWVEUQSMDAVNNLPPNVDOVIVRFKU\",\"authorized_flag\":true},{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118341309550594\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118341309550594-1\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118341309550594-1\"}},\"id\":\"0021118341309550594-0000000001\",\"paging_token\":\"21118341309550594-1\",\"account\":\"GDTBRJDRRQHHH4EPXOIBEAXTDJVETYVBOO53UGD4NDTW5P7YKG2DAWBD\",\"type\":\"trustline_removed\",\"type_i\":21,\"created_at\":\"2026-09-28T15:16:12Z\",\"asset_type\":\"credit_alphanum12\",\"asset_code\":\"LWTEST\",\"asset_issuer\":\"GAPIM2WRYS65WEWE2PAJYNGST35ZQXEJJQKWG7EUVMEZ2LWP7SXSNTNO\",\"limit\":\"0.0000000\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21118221050449923\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21118221050449923-2\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21118221050449923-2\"}},\"id\":\"0021118221050449923-0000000002\",\"paging_token\":\"21118221050449923-2\",\"account\":\"GCRYREIBNPYMSL436Q6VBGWWQUZYYOFFZALPN77ZHVRNXBVYSPFITNDO\",\"type\":\"trustline_sponsorship_created\",\"type_i\":63,\"created_at\":\"2026-09-28T15:13:52Z\",\"asset_type\":\"credit_alphanum4\",\"asset\":\"USDC:GBBD47IF6LWK7P7MDEVSCWR7DPUWV3NY3DTQEVFL4NAT4AQH3ZLLFLA5\",\"sponsor\":\"GCBS7GI62E2XVLK2O42KTCRR5RRNFZVKUVDHULPX4TIYGNWAZFML5BJQ\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.st"
            "ellar.org/operations/21116387099414530\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21116387099414530-2\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21116387099414530-2\"}},\"id\":\"0021116387099414530-0000000002\",\"paging_token\":\"21116387099414530-2\",\"account\":\"GAALG4JHVZERTOLKMAZOOJPFXTO4QODLWRXXEE645YZOJFIXGBYJ73LG\",\"type\":\"trustline_sponsorship_removed\",\"type_i\":65,\"created_at\":\"2026-09-28T14:38:17Z\",\"asset_type\":\"credit_alphanum4\",\"asset\":\"NET1:GBXXHGBP7B3F6WA6CK7NYRJNF6GREBONZKGEDYGNW2DJYU2ED2CFI3DP\",\"former_sponsor\":\"GAKCMMDBUAUPOX4WJXIH7VQDBR4JI46PLGMU6AI2LEZQJAPYHMH3MKPQ\"},{\"_links\":{\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/21116567488040961\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=desc&cursor=21116567488040961-1\"},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/effects?order=asc&cursor=21116567488040961-1\"}},\"id\":\"0021116567488040961-0000000001\",\"paging_token\":\"21116567488040961-1\",\"account\":\"GC3G3T7XGT6ICDU2TCDF4U5EMFWWIUP4OAU7MFDVNEUWXQE5ZXMRNKLD\",\"type\":\"trustline_updated\",\"type_i\":22,\"created_at\":\"2026-09-28T14:41:47Z\",\"asset_type\":\"credit_alphanum4\",\"asset_code\":\"USDC\",\"asset_issuer\":\"GA5ZSEJYB37JRC5AVCIA5MOP4RHTM335X2KGX3IHOJAPP5RE34K4KZVN\",\"limit\":\"922337203685.4775807\"}]},\"_links\":{\"self\":{\"href\":\"https://horizon.example/x?cursor=&limit=2&order=asc\"},\"next\":{\"href\":\"https://horizon.example/x?cursor=2&limit=2&order=asc\"},\"prev\":{\"href\":\"https://horizon.example/x?cursor=1&limit=2&order=desc\"}}}");
    }

    /** The real effect of type `type` from realEffectsOfEveryType(), as a document. */
    static QByteArray realEffectOfType(int type)
    {
        const QJsonArray records = QJsonDocument::fromJson(realEffectsOfEveryType()).object()
                .value("_embedded").toObject().value("records").toArray();
        for (const QJsonValue& record : records) {
            if (record.toObject().value("type_i").toInt() == type)
                return QJsonDocument(record.toObject()).toJson(QJsonDocument::Compact);
        }
        return QByteArray();
    }

    /** The members Horizon always sends in an effect of each type, beyond those of every effect. */
    static QHash<int, QStringList> requiredByType()
    {
        const QStringList sponsorship = QStringList() << "former_sponsor" << "new_sponsor";
        const QStringList pool = QStringList() << "liquidity_pool" << "liquidity_pool.id" << "liquidity_pool.fee_bp"
                                               << "liquidity_pool.type" << "liquidity_pool.total_trustlines"
                                               << "liquidity_pool.total_shares" << "liquidity_pool.reserves"
                                               << "liquidity_pool.reserves[0].asset" << "liquidity_pool.reserves[0].amount";
        QHash<int, QStringList> required;
        required.insert(0, QStringList() << "starting_balance");
        required.insert(2, QStringList() << "amount" << "asset_type");
        required.insert(3, QStringList() << "amount" << "asset_type");
        required.insert(4, QStringList() << "low_threshold" << "med_threshold" << "high_threshold");
        required.insert(5, QStringList() << "home_domain");
        for (int type : { 10, 11, 12 })
            required.insert(type, QStringList() << "weight" << "public_key");
        for (int type : { 20, 21, 22 })
            required.insert(type, QStringList() << "limit" << "asset_type");
        for (int type : { 23, 24, 25, 26 })
            required.insert(type, QStringList() << "trustor" << "asset_type");
        required.insert(33, QStringList() << "seller" << "offer_id" << "sold_amount" << "bought_amount"
                                          << "sold_asset_type" << "bought_asset_type");
        required.insert(43, QStringList() << "new_seq");
        required.insert(50, QStringList() << "balance_id" << "asset" << "amount");
        required.insert(51, QStringList() << "balance_id" << "asset" << "amount" << "predicate");
        required.insert(52, QStringList() << "balance_id" << "asset" << "amount");
        required.insert(60, QStringList() << "sponsor");
        required.insert(61, sponsorship);
        required.insert(62, QStringList() << "former_sponsor");
        required.insert(63, QStringList() << "sponsor");
        required.insert(64, sponsorship);
        required.insert(65, QStringList() << "former_sponsor");
        required.insert(66, QStringList() << "data_name" << "sponsor");
        required.insert(67, QStringList() << "data_name" << sponsorship);
        required.insert(68, QStringList() << "data_name" << "former_sponsor");
        required.insert(69, QStringList() << "balance_id" << "sponsor");
        required.insert(70, QStringList() << "balance_id" << sponsorship);
        required.insert(71, QStringList() << "balance_id" << "former_sponsor");
        required.insert(72, QStringList() << "signer" << "sponsor");
        required.insert(73, QStringList() << "signer" << sponsorship);
        required.insert(74, QStringList() << "signer" << "former_sponsor");
        required.insert(80, QStringList() << "balance_id");
        required.insert(90, pool + (QStringList() << "reserves_deposited" << "reserves_deposited[0].asset"
                                    << "reserves_deposited[0].amount" << "shares_received"));
        required.insert(91, pool + (QStringList() << "reserves_received" << "reserves_received[1].asset" << "shares_redeemed"));
        required.insert(92, pool + (QStringList() << "sold" << "sold.asset" << "sold.amount" << "bought" << "bought.asset"
                                    << "bought.amount"));
        required.insert(93, pool);
        required.insert(94, QStringList() << "liquidity_pool_id");
        required.insert(95, pool + (QStringList() << "reserves_revoked" << "reserves_revoked[0].asset"
                                    << "reserves_revoked[0].amount" << "reserves_revoked[0].claimable_balance_id"
                                    << "shares_revoked"));
        required.insert(96, QStringList() << "amount" << "asset_type" << "contract");
        required.insert(97, QStringList() << "amount" << "asset_type" << "contract");
        return required;
    }

private slots:
    /**
     * Every member Horizon always sends in an effect is required, for every
     * effect type Horizon returned: missing or null, an amount or a pool's
     * shares read "", a sponsor, a balance id or a signer empty, a weight
     * or a threshold 0, a trade's seller and offer as nobody's.
     */
    void testMembersHorizonAlwaysSendsAreRequired()
    {
        const QHash<int, QStringList> byType = requiredByType();
        const QStringList everyEffect = QStringList() << "id" << "paging_token" << "type_i" << "type" << "account"
                                                      << "created_at" << "_links" << "_links.operation"
                                                      << "_links.precedes" << "_links.succeeds"
                                                      << "_links.operation.href";
        const QJsonArray records = QJsonDocument::fromJson(realEffectsOfEveryType()).object()
                .value("_embedded").toObject().value("records").toArray();
        int checked = 0;
        for (const QJsonValue& record : records) {
            const int type = record.toObject().value("type_i").toInt();
            QVERIFY2(byType.contains(type) || QList<int>({ 1, 6, 7, 30, 31, 32, 40, 41, 42 }).contains(type),
                     qPrintable(QString::number(type)));
            const QByteArray page = QJsonDocument(QJsonObject{{"_embedded", QJsonObject{{"records",
                                                   QJsonArray{record}}}}}).toJson();
            QVERIFY(refusalOf<EffectsPage>(page, "_embedded.records[0].id", record.toObject().value("id")).isEmpty());
            for (const QString& member : everyEffect + byType.value(type)) {
                const QString field = QStringLiteral("_embedded.records[0].") + member;
                const QString missing = refusalOf<EffectsPage>(page, field, QJsonValue(QJsonValue::Undefined));
                QVERIFY2(missing.startsWith(field + ": "), qPrintable(QString::number(type) + " " + field + " -> " + missing));
                const QString null = refusalOf<EffectsPage>(page, field, QJsonValue());
                QVERIFY2(null.startsWith(field + ": "), qPrintable(QString::number(type) + " " + field + " -> " + null));
                ++checked;
            }
        }
        QVERIFY(checked > 200);
    }

    /**
     * A signer effect's weight is the int32 Horizon sends. Signers added
     * before protocol 10 may weigh more than 255 (stellar-core counts them
     * as 255), and Horizon casts the ledger's uint32 to an int32: 2^31 and
     * past arrive negative. The weight was held to 0..255, and an effects
     * page holding such an effect was refused whole.
     */
    void testSignerEffectWeightIsTheInt32HorizonSends()
    {
        for (int type : { 10, 11, 12 }) {
            const QByteArray real = realEffectOfType(type);
            QVERIFY(!real.isEmpty());
            for (const qint64 weight : { Q_INT64_C(256), Q_INT64_C(1000), Q_INT64_C(2147483647),
                                         Q_INT64_C(-1294967296), Q_INT64_C(-2147483648) }) {
                QJsonObject changed = QJsonDocument::fromJson(real).object();
                changed.insert("weight", static_cast<double>(weight));
                const QByteArray page = QJsonDocument(QJsonObject{{"_embedded", QJsonObject{{"records",
                                                       QJsonArray{changed}}}}}).toJson();
                EffectsPage effects;
                effects.loadFromJson(page);
                auto* signer = dynamic_cast<SignerEffectResponse*>(effects.getRecords().at(0));
                QVERIFY(signer != nullptr);
                QCOMPARE(static_cast<qint64>(signer->getWeight()), weight);
            }
            QVERIFY(refusalOf<SignerCreatedEffectResponse>(real, "weight", 2147483648.0).startsWith("weight: "));
            QVERIFY(refusalOf<SignerCreatedEffectResponse>(real, "weight", -2147483649.0).startsWith("weight: "));
            QVERIFY(refusalOf<SignerCreatedEffectResponse>(real, "weight", 1.5).startsWith("weight: "));
        }
        // A threshold is still one byte.
        QVERIFY(refusalOf<AccountThresholdsUpdatedEffectResponse>(realThresholdsUpdated(), "low_threshold", 256)
                    .startsWith("low_threshold: "));
    }

    /**
     * A pool's shares redeemed and revoked are amounts, and its trustline
     * count a whole number: text that was neither ("12x", 8 decimals) was
     * kept as it came, for the caller to read as 0.
     */
    void testPoolEffectAmountsAreReadExactly()
    {
        const QByteArray withdrew = realEffectOfType(91);
        const QByteArray revoked = realEffectOfType(95);
        const QByteArray created = realEffectOfType(93);
        QVERIFY(refusalOf<LiquidityPoolWithdrewEffectResponse>(withdrew, "shares_redeemed", QString("12x")).startsWith("shares_redeemed: "));
        QVERIFY(refusalOf<LiquidityPoolWithdrewEffectResponse>(withdrew, "shares_redeemed", QString("10.00000001")).startsWith("shares_redeemed: "));
        QVERIFY(refusalOf<LiquidityPoolWithdrewEffectResponse>(withdrew, "shares_redeemed", QString("10.0000001")).isEmpty());
        QVERIFY(refusalOf<LiquidityPoolRevokedEffectResponse>(revoked, "shares_revoked", QString("306.2277660 shares")).startsWith("shares_revoked: "));
        QVERIFY(refusalOf<LiquidityPoolRevokedEffectResponse>(revoked, "shares_revoked", QString("-306.2277660")).startsWith("shares_revoked: "));
        QVERIFY(refusalOf<LiquidityPoolCreatedEffectResponse>(created, "liquidity_pool.total_trustlines", QString("1x"))
                    .startsWith("liquidity_pool.total_trustlines: "));
        QVERIFY(refusalOf<LiquidityPoolCreatedEffectResponse>(created, "liquidity_pool.total_trustlines", QString("1.5"))
                    .startsWith("liquidity_pool.total_trustlines: "));
        LiquidityPoolWithdrewEffectResponse read;
        read.loadFromJson(withdrew);
        QCOMPARE(read.getSharesRedeemed(), QString("10.0000000"));
    }

    /**
     * An effect type this SDK does not know (Horizon adds them without a
     * major release, as 96 and 97 came) is kept as a plain EffectResponse:
     * its common members read, getType() the name Horizon sent, json() all
     * of it, as js 17.1.0 (CallBuilder hands records back as they come) and
     * py 14.1.0 (plain dicts) keep it. The whole page was refused, "unknown
     * effect type 999". It reads no asset, so its asset_type is not checked;
     * the members every effect has are still required.
     */
    void testAnUnknownEffectTypeIsKeptAsABaseRecord()
    {
        const QJsonObject credited = QJsonDocument::fromJson(realAccountCredited()).object();
        QJsonObject unknown = credited;
        unknown["type"] = QStringLiteral("future_effect");
        unknown["type_i"] = 999;
        unknown["asset_type"] = QStringLiteral("liquidity_pool");
        unknown["future_member"] = QStringLiteral("x");
        const QByteArray page = QJsonDocument(QJsonObject{{"_embedded", QJsonObject{{"records",
            QJsonArray{credited, unknown, credited}}}}}).toJson();
        EffectsPage read;
        read.loadFromJson(page);
        QCOMPARE(read.size(), 3);
        QVERIFY(dynamic_cast<AccountCreditedEffectResponse*>(read.getRecords().at(0)) != nullptr);
        QVERIFY(dynamic_cast<AccountCreditedEffectResponse*>(read.getRecords().at(2)) != nullptr);
        EffectResponse* kept = read.at(1);
        QVERIFY(kept->metaObject() == &EffectResponse::staticMetaObject);
        QCOMPARE(kept->getType(), QString("future_effect"));
        QCOMPARE(kept->getId(), credited.value("id").toString());
        QCOMPARE(kept->getPagingToken(), credited.value("paging_token").toString());
        QCOMPARE(kept->getAccount().getAccountId(), credited.value("account").toString());
        QCOMPARE(kept->json().value("type_i").toInt(), 999);
        QCOMPARE(kept->json().value("future_member").toString(), QString("x"));
        QVERIFY(kept->json().value("amount") == credited.value("amount"));
        QVERIFY(refusalOf<EffectsPage>(page, "_embedded.records[1].paging_token", QJsonValue(QJsonValue::Undefined))
                    .startsWith("_embedded.records[1].paging_token: "));
        QVERIFY(refusalOf<EffectsPage>(page, "_embedded.records[1].type_i", -1)
                    .startsWith("_embedded.records[1].type_i: "));
    }
};

ADD_TEST(EffectDeserializerTest)
#endif // EFFECTDESERIALIZERTEST_H
