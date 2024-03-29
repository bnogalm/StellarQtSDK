#ifndef ASSETSPAGEDESERIALIZERTEST_H
#define ASSETSPAGEDESERIALIZERTEST_H
#include <QObject>

#include <QtTest>
#include "../testcollector.h"
#include "../../src/responses/assetresponse.h"
#include "../../src/responses/page.h"
class AssetsPageDeserializerTest: public QObject
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
        Page<AssetResponse> page;
        page.loadFromJson(json);

        QCOMPARE(page.getLinks().getSelf().getHref(), QString("https://horizon.stellar.org/assets?order=asc&limit=10&cursor="));
        QCOMPARE(page.getLinks().getNext().getHref(), QString("https://horizon.stellar.org/assets?order=asc&limit=10&cursor=AsrtoDollar_GDJWXY5XUASXNL4ABCONR6T5MOXJ2S4HD6WDNAJDSDKQ4VS3TVUQJEDJ_credit_alphanum12"));

        QCOMPARE(page.get(0).getAssetType(), QString("credit_alphanum12"));
        QCOMPARE(page.get(0).getAssetCode(), QString("6497847"));
        QCOMPARE(page.get(0).getAssetIssuer(), QString("GCGNWKCJ3KHRLPM3TM6N7D3W5YKDJFL6A2YCXFXNMRTZ4Q66MEMZ6FI2"));
        QCOMPARE(page.get(0).getPagingToken(), QString("6497847_GCGNWKCJ3KHRLPM3TM6N7D3W5YKDJFL6A2YCXFXNMRTZ4Q66MEMZ6FI2_credit_alphanum12"));
        QCOMPARE(page.get(0).getAccounts().authorized(), 1);
        QCOMPARE(page.get(0).getAccounts().authorizedToMaintainLiabilities(), 0);
        QCOMPARE(page.get(0).getAccounts().unauthorized(), 0);
        QCOMPARE(page.get(0).getBalances().authorized(), QString("0.0000000"));
        QCOMPARE(page.get(0).getBalances().authorizedToMaintainLiabilities(), QString("0.0000000"));
        QCOMPARE(page.get(0).getBalances().unauthorized(), QString("0.0000000"));
        QCOMPARE(page.get(0).getNumClaimableBalances(), 0);
        QCOMPARE(page.get(0).getClaimableBalancesAmount(), QString("0.0000000"));
        QCOMPARE(page.get(0).getAmount(), QString("0.0000000"));
        QCOMPARE(page.get(0).getNumAccounts(), 1);
        QCOMPARE(page.get(0).getLinks().getToml().getHref(), QString("https://www.stellar.org/.well-known/stellar.toml"));
        QCOMPARE(page.get(0).getFlags().isAuthRequired(), true);
        QCOMPARE(page.get(0).getFlags().isAuthRevocable(), false);
      }
private:
      const QByteArray json = "{\n"
              "  \"_links\": {\n"
              "    \"self\": {\n"
              "      \"href\": \"https://horizon.stellar.org/assets?order=asc\\u0026limit=10\\u0026cursor=\"\n"
              "    },\n"
              "    \"next\": {\n"
              "      \"href\": \"https://horizon.stellar.org/assets?order=asc\\u0026limit=10\\u0026cursor=AsrtoDollar_GDJWXY5XUASXNL4ABCONR6T5MOXJ2S4HD6WDNAJDSDKQ4VS3TVUQJEDJ_credit_alphanum12\"\n"
              "    },\n"
              "    \"prev\": {\n"
              "      \"href\": \"https://horizon.stellar.org/assets?order=desc\\u0026limit=10\\u0026cursor=6497847_GCGNWKCJ3KHRLPM3TM6N7D3W5YKDJFL6A2YCXFXNMRTZ4Q66MEMZ6FI2_credit_alphanum12\"\n"
              "    }\n"
              "  },\n"
              "  \"_embedded\": {\n"
              "    \"records\": [\n"
              "      {\n"
              "        \"_links\": {\n"
              "          \"toml\": {\n"
              "            \"href\": \"https://www.stellar.org/.well-known/stellar.toml\"\n"
              "          }\n"
              "        },\n"
              "        \"asset_type\": \"credit_alphanum12\",\n"
              "        \"asset_code\": \"6497847\",\n"
              "        \"asset_issuer\": \"GCGNWKCJ3KHRLPM3TM6N7D3W5YKDJFL6A2YCXFXNMRTZ4Q66MEMZ6FI2\",\n"
              "        \"paging_token\": \"6497847_GCGNWKCJ3KHRLPM3TM6N7D3W5YKDJFL6A2YCXFXNMRTZ4Q66MEMZ6FI2_credit_alphanum12\",\n"
              "        \"accounts\": {\n"
              "          \"authorized\": 1,\n"
              "          \"authorized_to_maintain_liabilities\": 0,\n"
              "          \"unauthorized\": 0\n"
              "        },\n"
              "        \"balances\": {\n"
              "          \"authorized\": \"0.0000000\",\n"
              "          \"authorized_to_maintain_liabilities\": \"0.0000000\",\n"
              "          \"unauthorized\": \"0.0000000\"\n"
              "        },\n"
              "        \"claimable_balances_amount\": \"0.0000000\",\n"
              "        \"num_claimable_balances\": 0,\n"
              "        \"amount\": \"0.0000000\",\n"
              "        \"num_accounts\": 1,\n"
              "        \"flags\": {\n"
              "          \"auth_required\": true,\n"
              "          \"auth_revocable\": false\n"
              "        }\n"
              "      },\n"
              "      {\n"
              "        \"_links\": {\n"
              "          \"toml\": {\n"
              "            \"href\": \"\"\n"
              "          }\n"
              "        },\n"
              "        \"asset_type\": \"credit_alphanum12\",\n"
              "        \"asset_code\": \"9HORIZONS\",\n"
              "        \"asset_issuer\": \"GB2HXY7UEDCSHOWZ4553QFGFILNU73OFS2P4HU5IB3UUU66TWPBPVTGW\",\n"
              "        \"paging_token\": \"9HORIZONS_GB2HXY7UEDCSHOWZ4553QFGFILNU73OFS2P4HU5IB3UUU66TWPBPVTGW_credit_alphanum12\",\n"
              "        \"accounts\": {\n"
              "          \"authorized\": 3,\n"
              "          \"authorized_to_maintain_liabilities\": 0,\n"
              "          \"unauthorized\": 0\n"
              "        },\n"
              "        \"balances\": {\n"
              "          \"authorized\": \"1000000.0000000\",\n"
              "          \"authorized_to_maintain_liabilities\": \"0.0000000\",\n"
              "          \"unauthorized\": \"0.0000000\"\n"
              "        },\n"
              "        \"claimable_balances_amount\": \"0.0000000\",\n"
              "        \"num_claimable_balances\": 0,\n"
              "        \"amount\": \"1000000.0000000\",\n"
              "        \"num_accounts\": 3,\n"
              "        \"flags\": {\n"
              "          \"auth_required\": false,\n"
              "          \"auth_revocable\": false\n"
              "        }\n"
              "      },\n"
              "      {\n"
              "        \"_links\": {\n"
              "          \"toml\": {\n"
              "            \"href\": \"\"\n"
              "          }\n"
              "        },\n"
              "        \"asset_type\": \"credit_alphanum4\",\n"
              "        \"asset_code\": \"AIR\",\n"
              "        \"asset_issuer\": \"GB2SQ74JCS6F4MVDU4BF4L4S4Z5Z36ABOTP6DF5JJOFGFE3ETZAUVUQK\",\n"
              "        \"paging_token\": \"AIR_GB2SQ74JCS6F4MVDU4BF4L4S4Z5Z36ABOTP6DF5JJOFGFE3ETZAUVUQK_credit_alphanum4\",\n"
              "        \"accounts\": {\n"
              "          \"authorized\": 2,\n"
              "          \"authorized_to_maintain_liabilities\": 0,\n"
              "          \"unauthorized\": 0\n"
              "        },\n"
              "        \"balances\": {\n"
              "          \"authorized\": \"100000000000.0000000\",\n"
              "          \"authorized_to_maintain_liabilities\": \"0.0000000\",\n"
              "          \"unauthorized\": \"0.0000000\"\n"
              "        },\n"
              "        \"claimable_balances_amount\": \"0.0000000\",\n"
              "        \"num_claimable_balances\": 0,\n"
              "        \"amount\": \"100000000000.0000000\",\n"
              "        \"num_accounts\": 2,\n"
              "        \"flags\": {\n"
              "          \"auth_required\": false,\n"
              "          \"auth_revocable\": false\n"
              "        }\n"
              "      },\n"
              "      {\n"
              "        \"_links\": {\n"
              "          \"toml\": {\n"
              "            \"href\": \"\"\n"
              "          }\n"
              "        },\n"
              "        \"asset_type\": \"credit_alphanum12\",\n"
              "        \"asset_code\": \"AlambLedgerS\",\n"
              "        \"asset_issuer\": \"GCMXATSZBEYTNPFQXHFQXUYXOTHA4HA5L2YZEKKOVGYWTUT24KIHECG3\",\n"
              "        \"paging_token\": \"AlambLedgerS_GCMXATSZBEYTNPFQXHFQXUYXOTHA4HA5L2YZEKKOVGYWTUT24KIHECG3_credit_alphanum12\",\n"
              "        \"accounts\": {\n"
              "          \"authorized\": 0,\n"
              "          \"authorized_to_maintain_liabilities\": 0,\n"
              "          \"unauthorized\": 0\n"
              "        },\n"
              "        \"balances\": {\n"
              "          \"authorized\": \"0.0000000\",\n"
              "          \"authorized_to_maintain_liabilities\": \"0.0000000\",\n"
              "          \"unauthorized\": \"0.0000000\"\n"
              "        },\n"
              "        \"claimable_balances_amount\": \"0.0000000\",\n"
              "        \"num_claimable_balances\": 0,\n"
              "        \"amount\": \"0.0000000\",\n"
              "        \"num_accounts\": 0,\n"
              "        \"flags\": {\n"
              "          \"auth_required\": false,\n"
              "          \"auth_revocable\": false\n"
              "        }\n"
              "      },\n"
              "      {\n"
              "        \"_links\": {\n"
              "          \"toml\": {\n"
              "            \"href\": \"\"\n"
              "          }\n"
              "        },\n"
              "        \"asset_type\": \"credit_alphanum4\",\n"
              "        \"asset_code\": \"AMO\",\n"
              "        \"asset_issuer\": \"GBOMFBZG5PWUXDIIW5ITVRVEL6YCIC6ZDXLNBH33BNPCX3D7AXDCDKHF\",\n"
              "        \"paging_token\": \"AMO_GBOMFBZG5PWUXDIIW5ITVRVEL6YCIC6ZDXLNBH33BNPCX3D7AXDCDKHF_credit_alphanum4\",\n"
              "        \"accounts\": {\n"
              "          \"authorized\": 1,\n"
              "          \"authorized_to_maintain_liabilities\": 0,\n"
              "          \"unauthorized\": 0\n"
              "        },\n"
              "        \"balances\": {\n"
              "          \"authorized\": \"10000000.0000000\",\n"
              "          \"authorized_to_maintain_liabilities\": \"0.0000000\",\n"
              "          \"unauthorized\": \"0.0000000\"\n"
              "        },\n"
              "        \"claimable_balances_amount\": \"0.0000000\",\n"
              "        \"num_claimable_balances\": 0,\n"
              "        \"amount\": \"10000000.0000000\",\n"
              "        \"num_accounts\": 1,\n"
              "        \"flags\": {\n"
              "          \"auth_required\": false,\n"
              "          \"auth_revocable\": false\n"
              "        }\n"
              "      },\n"
              "      {\n"
              "        \"_links\": {\n"
              "          \"toml\": {\n"
              "            \"href\": \"\"\n"
              "          }\n"
              "        },\n"
              "        \"asset_type\": \"credit_alphanum4\",\n"
              "        \"asset_code\": \"AMO\",\n"
              "        \"asset_issuer\": \"GDIAIZ7S7L2OBEQBH62KE7IWXK76XA7ES7XCH7JCPXQGV7VB3V6VETOX\",\n"
              "        \"paging_token\": \"AMO_GDIAIZ7S7L2OBEQBH62KE7IWXK76XA7ES7XCH7JCPXQGV7VB3V6VETOX_credit_alphanum4\",\n"
              "        \"accounts\": {\n"
              "          \"authorized\": 1,\n"
              "          \"authorized_to_maintain_liabilities\": 0,\n"
              "          \"unauthorized\": 0\n"
              "        },\n"
              "        \"balances\": {\n"
              "          \"authorized\": \"0.0000000\",\n"
              "          \"authorized_to_maintain_liabilities\": \"0.0000000\",\n"
              "          \"unauthorized\": \"0.0000000\"\n"
              "        },\n"
              "        \"claimable_balances_amount\": \"0.0000000\",\n"
              "        \"num_claimable_balances\": 0,\n"
              "        \"amount\": \"0.0000000\",\n"
              "        \"num_accounts\": 1,\n"
              "        \"flags\": {\n"
              "          \"auth_required\": false,\n"
              "          \"auth_revocable\": false\n"
              "        }\n"
              "      },\n"
              "      {\n"
              "        \"_links\": {\n"
              "          \"toml\": {\n"
              "            \"href\": \"\"\n"
              "          }\n"
              "        },\n"
              "        \"asset_type\": \"credit_alphanum4\",\n"
              "        \"asset_code\": \"ASD\",\n"
              "        \"asset_issuer\": \"GAOMRMILWSX7UXZMYC4X7B7BVJXORYV36XUK3EURVJF7DA6B77ABFVOJ\",\n"
              "        \"paging_token\": \"ASD_GAOMRMILWSX7UXZMYC4X7B7BVJXORYV36XUK3EURVJF7DA6B77ABFVOJ_credit_alphanum4\",\n"
              "        \"accounts\": {\n"
              "          \"authorized\": 0,\n"
              "          \"authorized_to_maintain_liabilities\": 0,\n"
              "          \"unauthorized\": 0\n"
              "        },\n"
              "        \"balances\": {\n"
              "          \"authorized\": \"0.0000000\",\n"
              "          \"authorized_to_maintain_liabilities\": \"0.0000000\",\n"
              "          \"unauthorized\": \"0.0000000\"\n"
              "        },\n"
              "        \"claimable_balances_amount\": \"0.0000000\",\n"
              "        \"num_claimable_balances\": 0,\n"
              "        \"amount\": \"0.0000000\",\n"
              "        \"num_accounts\": 0,\n"
              "        \"flags\": {\n"
              "          \"auth_required\": false,\n"
              "          \"auth_revocable\": false\n"
              "        }\n"
              "      },\n"
              "      {\n"
              "        \"_links\": {\n"
              "          \"toml\": {\n"
              "            \"href\": \"https://mystellar.tools/.well-known/stellar.toml\"\n"
              "          }\n"
              "        },\n"
              "        \"asset_type\": \"credit_alphanum4\",\n"
              "        \"asset_code\": \"ASD\",\n"
              "        \"asset_issuer\": \"GDP4SJE5Y5ODX627DO2F7ZNBAPVXRFHKKR3W4UJ6I4XMW3S3OH2XRWYD\",\n"
              "        \"paging_token\": \"ASD_GDP4SJE5Y5ODX627DO2F7ZNBAPVXRFHKKR3W4UJ6I4XMW3S3OH2XRWYD_credit_alphanum4\",\n"
              "        \"accounts\": {\n"
              "          \"authorized\": 0,\n"
              "          \"authorized_to_maintain_liabilities\": 0,\n"
              "          \"unauthorized\": 0\n"
              "        },\n"
              "        \"balances\": {\n"
              "          \"authorized\": \"0.0000000\",\n"
              "          \"authorized_to_maintain_liabilities\": \"0.0000000\",\n"
              "          \"unauthorized\": \"0.0000000\"\n"
              "        },\n"
              "        \"claimable_balances_amount\": \"0.0000000\",\n"
              "        \"num_claimable_balances\": 0,\n"
              "        \"amount\": \"0.0000000\",\n"
              "        \"num_accounts\": 0,\n"
              "        \"flags\": {\n"
              "          \"auth_required\": false,\n"
              "          \"auth_revocable\": false\n"
              "        }\n"
              "      },\n"
              "      {\n"
              "        \"_links\": {\n"
              "          \"toml\": {\n"
              "            \"href\": \"\"\n"
              "          }\n"
              "        },\n"
              "        \"asset_type\": \"credit_alphanum12\",\n"
              "        \"asset_code\": \"AsrtoDollar\",\n"
              "        \"asset_issuer\": \"GBPGO557IQWSWOIKHWB7YJ5QIBWVF4QS6SPGWT5YBGDUPE6QKOD7RR7S\",\n"
              "        \"paging_token\": \"AsrtoDollar_GBPGO557IQWSWOIKHWB7YJ5QIBWVF4QS6SPGWT5YBGDUPE6QKOD7RR7S_credit_alphanum12\",\n"
              "        \"accounts\": {\n"
              "          \"authorized\": 0,\n"
              "          \"authorized_to_maintain_liabilities\": 0,\n"
              "          \"unauthorized\": 0\n"
              "        },\n"
              "        \"balances\": {\n"
              "          \"authorized\": \"0.0000000\",\n"
              "          \"authorized_to_maintain_liabilities\": \"0.0000000\",\n"
              "          \"unauthorized\": \"0.0000000\"\n"
              "        },\n"
              "        \"claimable_balances_amount\": \"0.0000000\",\n"
              "        \"num_claimable_balances\": 0,\n"
              "        \"amount\": \"0.0000000\",\n"
              "        \"num_accounts\": 0,\n"
              "        \"flags\": {\n"
              "          \"auth_required\": false,\n"
              "          \"auth_revocable\": false\n"
              "        }\n"
              "      },\n"
              "      {\n"
              "        \"_links\": {\n"
              "          \"toml\": {\n"
              "            \"href\": \"\"\n"
              "          }\n"
              "        },\n"
              "        \"asset_type\": \"credit_alphanum12\",\n"
              "        \"asset_code\": \"AsrtoDollar\",\n"
              "        \"asset_issuer\": \"GDJWXY5XUASXNL4ABCONR6T5MOXJ2S4HD6WDNAJDSDKQ4VS3TVUQJEDJ\",\n"
              "        \"paging_token\": \"AsrtoDollar_GDJWXY5XUASXNL4ABCONR6T5MOXJ2S4HD6WDNAJDSDKQ4VS3TVUQJEDJ_credit_alphanum12\",\n"
              "        \"accounts\": {\n"
              "          \"authorized\": 0,\n"
              "          \"authorized_to_maintain_liabilities\": 0,\n"
              "          \"unauthorized\": 0\n"
              "        },\n"
              "        \"balances\": {\n"
              "          \"authorized\": \"0.0000000\",\n"
              "          \"authorized_to_maintain_liabilities\": \"0.0000000\",\n"
              "          \"unauthorized\": \"0.0000000\"\n"
              "        },\n"
              "        \"claimable_balances_amount\": \"0.0000000\",\n"
              "        \"num_claimable_balances\": 0,\n"
              "        \"amount\": \"0.0000000\",\n"
              "        \"num_accounts\": 0,\n"
              "        \"flags\": {\n"
              "          \"auth_required\": false,\n"
              "          \"auth_revocable\": false\n"
              "        }\n"
              "      }\n"
              "    ]\n"
              "  }\n"
              "}";

};

ADD_TEST(AssetsPageDeserializerTest)
#endif // ASSETSPAGEDESERIALIZERTEST_H

