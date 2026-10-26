#ifndef ASSETSPAGEDESERIALIZERTEST_H
#define ASSETSPAGEDESERIALIZERTEST_H
#include <QObject>

#include <QtTest>
#include "../testcollector.h"
#include "../../src/responses/assetresponse.h"
#include "../../src/responses/page.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <limits>
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
        QCOMPARE(page.get(0).getNumLiquidityPools(), 4);
        QCOMPARE(page.get(0).getLiquidityPoolsAmount(), QString("123.4560000"));
        QCOMPARE(page.get(0).getNumContracts(), 7);
        QCOMPARE(page.get(0).getContractsAmount(), QString("789.0000000"));
        QCOMPARE(page.get(0).getNumArchivedContracts(), 2);
        QCOMPARE(page.get(0).getArchivedContractsAmount(), QString("10.0000000"));
        QCOMPARE(page.get(0).getContractId(), QString("CAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAH7B"));
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
              "        \"num_liquidity_pools\": 4,\n"
              "        \"liquidity_pools_amount\": \"123.4560000\",\n"
              "        \"num_contracts\": 7,\n"
              "        \"contracts_amount\": \"789.0000000\",\n"
              "        \"num_archived_contracts\": 2,\n"
              "        \"archived_contracts_amount\": \"10.0000000\",\n"
              "        \"contract_id\": \"CAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAH7B\",\n"
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
              "        },\n"
              "        \"num_liquidity_pools\": 0,\n"
              "        \"num_contracts\": 0,\n"
              "        \"liquidity_pools_amount\": \"0.0000000\",\n"
              "        \"contracts_amount\": \"0.0000000\"\n"
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
              "        },\n"
              "        \"num_liquidity_pools\": 0,\n"
              "        \"num_contracts\": 0,\n"
              "        \"liquidity_pools_amount\": \"0.0000000\",\n"
              "        \"contracts_amount\": \"0.0000000\"\n"
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
              "        },\n"
              "        \"num_liquidity_pools\": 0,\n"
              "        \"num_contracts\": 0,\n"
              "        \"liquidity_pools_amount\": \"0.0000000\",\n"
              "        \"contracts_amount\": \"0.0000000\"\n"
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
              "        },\n"
              "        \"num_liquidity_pools\": 0,\n"
              "        \"num_contracts\": 0,\n"
              "        \"liquidity_pools_amount\": \"0.0000000\",\n"
              "        \"contracts_amount\": \"0.0000000\"\n"
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
              "        },\n"
              "        \"num_liquidity_pools\": 0,\n"
              "        \"num_contracts\": 0,\n"
              "        \"liquidity_pools_amount\": \"0.0000000\",\n"
              "        \"contracts_amount\": \"0.0000000\"\n"
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
              "        },\n"
              "        \"num_liquidity_pools\": 0,\n"
              "        \"num_contracts\": 0,\n"
              "        \"liquidity_pools_amount\": \"0.0000000\",\n"
              "        \"contracts_amount\": \"0.0000000\"\n"
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
              "        },\n"
              "        \"num_liquidity_pools\": 0,\n"
              "        \"num_contracts\": 0,\n"
              "        \"liquidity_pools_amount\": \"0.0000000\",\n"
              "        \"contracts_amount\": \"0.0000000\"\n"
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
              "        },\n"
              "        \"num_liquidity_pools\": 0,\n"
              "        \"num_contracts\": 0,\n"
              "        \"liquidity_pools_amount\": \"0.0000000\",\n"
              "        \"contracts_amount\": \"0.0000000\"\n"
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
              "        },\n"
              "        \"num_liquidity_pools\": 0,\n"
              "        \"num_contracts\": 0,\n"
              "        \"liquidity_pools_amount\": \"0.0000000\",\n"
              "        \"contracts_amount\": \"0.0000000\"\n"
              "      }\n"
              "    ]\n"
              "  }\n"
              "}";


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

private slots:

    /**
     * An asset's statistics, exactly: an account count that is not a whole
     * number read as 0, an amount given as a number as empty, and a flag
     * given as text as false.
     */
    void testAssetIsReadExactly()
    {
        typedef Page<AssetResponse> Assets;
        const QByteArray real = realPubnetAssets();
        const QString r = QStringLiteral("_embedded.records[0].");
        QVERIFY(refusalOf<Assets>(real, r + "accounts.authorized", QString("12 accounts")).startsWith(r + "accounts.authorized: "));
        QVERIFY(refusalOf<Assets>(real, r + "balances.authorized", 5).startsWith(r + "balances.authorized: "));
        QVERIFY(refusalOf<Assets>(real, r + "flags.auth_required", QString("yes")).startsWith(r + "flags.auth_required: "));
        QVERIFY(refusalOf<Assets>(real, r + "num_claimable_balances", 1.5).startsWith(r + "num_claimable_balances: "));
        QVERIFY(refusalOf<Assets>(real, r + "asset_code", QString("TOOLONGFORATYPE")).startsWith(r + "asset_code: "));
        QVERIFY(refusalOf<Assets>(real, r + "paging_token", 7).startsWith(r + "paging_token: "));
    }

    /** Real pubnet assets load. */
    /**
     * An asset's flags are four, as js HorizonApi.Flags has them; only two
     * were read, so an immutable issuer or one that can claw back read as
     * neither.
     */
    void testAllFourAssetFlagsAreRead()
    {
        QJsonObject page = QJsonDocument::fromJson(realPubnetAssets()).object();
        QJsonObject embedded = page.value("_embedded").toObject();
        QJsonArray records = embedded.value("records").toArray();
        QJsonObject record = records.at(0).toObject();
        QJsonObject flags = record.value("flags").toObject();
        flags.insert("auth_immutable", true);
        flags.insert("auth_clawback_enabled", true);
        record.insert("flags", flags);
        records.replace(0, record);
        embedded.insert("records", records);
        page.insert("_embedded", embedded);
        Page<AssetResponse> assets;
        assets.loadFromJson(QJsonDocument(page).toJson());
        QCOMPARE(assets.get(0).getFlags().isAuthImmutable(), true);
        QCOMPARE(assets.get(0).getFlags().isAuthClawbackEnabled(), true);
        Page<AssetResponse> real;
        real.loadFromJson(realPubnetAssets());
        QCOMPARE(real.get(0).getFlags().isAuthImmutable(), false);
        QCOMPARE(real.get(0).getFlags().isAuthClawbackEnabled(), false);
    }

    void testRealHorizonAssets()
    {
        Page<AssetResponse> assets;
        assets.loadFromJson(realPubnetAssets());
        QCOMPARE(assets.size(), 2);
        const QJsonObject first = QJsonDocument::fromJson(realPubnetAssets()).object()
                .value("_embedded").toObject().value("records").toArray().at(0).toObject();
        QCOMPARE(assets.get(0).getAssetCode(), first.value("asset_code").toString());
        QCOMPARE(assets.get(0).getBalances().authorized(), first.value("balances").toObject().value("authorized").toString());
        QCOMPARE(assets.get(0).getAccounts().authorized(), first.value("accounts").toObject().value("authorized").toInt());
    }

    /**
     * Every member Horizon always sends in an asset is required. Missing or
     * null, the amounts held in balances, claimable balances, pools and
     * contracts read "", the counts 0 and the flags false. What an older
     * Horizon sent and today's no longer does (amount, num_accounts, the
     * archived contracts) stays optional.
     */
    void testMembersHorizonAlwaysSendsAreRequired()
    {
        typedef Page<AssetResponse> Assets;
        const QByteArray real = realPubnetAssets();
        const QString r = QStringLiteral("_embedded.records[0].");
        QVERIFY(refusalOf<Assets>(real, r + "asset_code", QString("0")).isEmpty());
        for (const char* key : { "asset_type", "asset_code", "asset_issuer", "paging_token", "accounts", "balances",
                                 "claimable_balances_amount", "num_claimable_balances", "liquidity_pools_amount",
                                 "num_liquidity_pools", "contracts_amount", "num_contracts", "flags", "_links",
                                 "accounts.authorized", "accounts.authorized_to_maintain_liabilities",
                                 "accounts.unauthorized", "balances.authorized",
                                 "balances.authorized_to_maintain_liabilities", "balances.unauthorized",
                                 "flags.auth_required", "flags.auth_revocable", "_links.toml", "_links.toml.href" }) {
            QVERIFY2(refusalOf<Assets>(real, r + key, QJsonValue(QJsonValue::Undefined)).startsWith(r + key + ": "), key);
            QVERIFY2(refusalOf<Assets>(real, r + key, QJsonValue()).startsWith(r + key + ": "), key);
        }
        for (const char* key : { "amount", "num_accounts", "archived_contracts_amount", "num_archived_contracts",
                                 "contract_id" })
            QVERIFY2(refusalOf<Assets>(real, r + key, QJsonValue(QJsonValue::Undefined)).isEmpty(), key);
    }

    /**
     * The archived contracts' amount is an amount: anything else was kept
     * as it came, for the caller to read as 0.
     */
    void testArchivedContractsAmountIsReadExactly()
    {
        typedef Page<AssetResponse> Assets;
        const QByteArray real = realPubnetAssets();
        const QString r = QStringLiteral("_embedded.records[0].");
        QVERIFY(refusalOf<Assets>(real, r + "archived_contracts_amount", QString("10 XLM"))
                    .startsWith(r + "archived_contracts_amount: "));
        QVERIFY(refusalOf<Assets>(real, r + "archived_contracts_amount", QString("10.00000001"))
                    .startsWith(r + "archived_contracts_amount: "));
        QVERIFY(refusalOf<Assets>(real, r + "archived_contracts_amount", QString("10.0000000")).isEmpty());
    }
private:
    /** Two real pubnet assets. */
    static QByteArray realPubnetAssets()
    {
        return QByteArray(
            "{\"_embedded\":{\"records\":[{\"_links\":{\"toml\":{\"href\":\"https://NUMBERLAND/.well-known/stellar.toml\"}},\"asset_type\":\"credit_alphanum4\",\"asset_code\":\"0\",\"asset_issuer\":\"GAAHZ7FRQQI4WUZW6MMVEDOCFVKAIYPXKLJVKKEYTZJ7EQAMKWHIDSIX\",\"paging_token\":\"0_GAAHZ7FRQQI4WUZW6MMVEDOCFVKAIYPXKLJVKKEYTZJ7EQAMKWHIDSIX_credit_alphanum4\",\"num_claimable_balances\":0,\"num_liquidity_pools\":0,\"num_contracts\":0,\"accounts\":{\"authorized\":1,\"authorized_to_maintain_liabilities\":0,\"unauthorized\":0},\"claimable_balances_amount\":\"0.0000000\",\"liquidity_pools_amount\":\"0.0000000\",\"contracts_amount\":\"0.0000000\",\"balances\":{\"authorized\":\"100000000000.0000000\",\"authorized_to_maintain_liabilities\":\"0.0000000\",\"unauthorized\":\"0.0000000\"},\"flags\":{\"auth_required\":false,\"auth_revocable\":false,\"auth_immutable\":false,\"auth_clawback_enabled\":false}},{\"_links\":{\"toml\":{\"href\":\"https://stellarterm.com/.well-known/stellar.toml\"}},\"asset_type\":\"credit_alphanum4\",\"asset_code\":\"0\",\"asset_issuer\":\"GAFEDMD42RJVK4TWOV7QXFTBPJOU7HIKS4DBNKZYUNFUEK24AW26ZJJ5\",\"paging_token\":\"0_GAFEDMD42RJVK4TWOV7QXFTBPJOU7HIKS4DBNKZYUNFUEK24AW26ZJJ5_credit_alphanum4\",\"num_claimable_balances\":0,\"num_liquidity_pools\":22,\"num_contracts\":0,\"accounts\":{\"authorized\":8,\"authorized_to_maintain_liabilities\":0,\"unauthorized\":0},\"claimable_balances_amount\":\"0.0000000\",\"liquidity_pools_amount\":\"1356385.0861926\",\"contracts_amount\":\"0.0000000\",\"balances\":{\"authorized\":\"45817197.5870638\",\"authorized_to_maintain_liabilities\":\"0.0000000\",\"unauthorized\":\"0.0000000\"},\"flags\":{\"auth_required\":false,\"auth_revocable\":false,\"auth_immutable\":false,\"auth_clawback_enabled\":false}}]},\"_links\":{\"self\":{\"href\":\"https://horizon.example/x?cursor=&limit=2&order=asc\"},\"next\":{\"href\":\"https://horizon.example/x?cursor=2"
            "&limit=2&order=asc\"},\"prev\":{\"href\":\"https://horizon.example/x?cursor=1&limit=2&order=desc\"}}}");
    }

private slots:
};

ADD_TEST(AssetsPageDeserializerTest)
#endif // ASSETSPAGEDESERIALIZERTEST_H

