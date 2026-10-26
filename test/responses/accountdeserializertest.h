#ifndef ACCOUNTDESERIALIZERTEST_H
#define ACCOUNTDESERIALIZERTEST_H
#include <QObject>


#include <QtTest>
#include "../testcollector.h"
#include "../../src/responses/accountresponse.h"
#include "../../src/keypair.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <limits>
class AccountDeserializerTest: public QObject
{
    Q_OBJECT
    const QByteArray json = "{\n"
            "  \"_links\": {\n"
            "    \"effects\": {\n"
            "      \"href\": \"/accounts/GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7/effects{?cursor,limit,order}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"offers\": {\n"
            "      \"href\": \"/accounts/GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7/offers{?cursor,limit,order}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"operations\": {\n"
            "      \"href\": \"/accounts/GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7/operations{?cursor,limit,order}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"self\": {\n"
            "      \"href\": \"/accounts/GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7\"\n"
            "    },\n"
            "    \"transactions\": {\n"
            "      \"href\": \"/accounts/GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7/transactions{?cursor,limit,order}\",\n"
            "      \"templated\": true\n"
            "    }\n"
            "  },"
            "  \"id\": \"GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7\",\n"
            "  \"paging_token\": \"1\",\n"
            "  \"account_id\": \"GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7\",\n"
            "  \"sequence\": 2319149195853854,\n"
            "  \"sequence_ledger\": 47274621,\n"
            "  \"sequence_time\": \"1640995200\",\n"
            "  \"subentry_count\": 0,\n"
            "  \"inflation_destination\": \"GAGRSA6QNQJN2OQYCBNQGMFLO4QLZFNEHIFXOMTQVSUTWVTWT66TOFSC\",\n"
            "  \"home_domain\": \"stellar.org\",\n"
            "  \"thresholds\": {\n"
            "    \"low_threshold\": 10,\n"
            "    \"med_threshold\": 20,\n"
            "    \"high_threshold\": 30\n"
            "  },\n"
            "  \"flags\": {\n"
            "    \"auth_required\": false,\n"
            "    \"auth_revocable\": true,\n"
            "    \"auth_immutable\": true,\n"
            "    \"auth_clawback_enabled\": true\n"
            "  },\n"
            "  \"balances\": [\n"
            "    {\n"
            "      \"balance\": \"1001.0000000\",\n"
            "      \"buying_liabilities\": \"0.0000000\",\n"
            "      \"selling_liabilities\": \"0.0000000\",\n"
            "      \"is_clawback_enabled\": true,\n"
            "      \"limit\": \"12000.4775807\",\n"
            "      \"asset_type\": \"credit_alphanum4\",\n"
            "      \"asset_code\": \"ABC\",\n"
            "      \"asset_issuer\": \"GCRA6COW27CY5MTKIA7POQ2326C5ABYCXODBN4TFF5VL4FMBRHOT3YHU\"\n"
            "    },"
            "    {\n"
            "      \"asset_type\": \"native\",\n"
            "      \"balance\": \"20.0000300\",\n"
            "      \"buying_liabilities\": \"0.0000000\",\n"
            "      \"selling_liabilities\": \"0.0000000\"\n"
            "    },\n"
            "    {\n"
            "      \"asset_type\": \"liquidity_pool_shares\",\n"
            "      \"balance\": \"500.0000000\",\n"
            "      \"limit\": \"922337203685.4775807\",\n"
            "      \"liquidity_pool_id\": \"dd7b1ab831c273310ddbec6f97870aa83c2fbcf57320c0bd000242331b46f1d2\"\n"
            "    }\n"
            "  ],\n"
            "  \"signers\": [\n"
            "    {\n"
            "      \"public_key\": \"GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7\",\n"
            "      \"key\": \"GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7\",\n"
            "      \"weight\": 0,\n"
            "      \"type\": \"ed25519_public_key\"\n"
            "    },\n"
            "    {\n"
            "      \"public_key\": \"GCR2KBCIU6KQXSQY5F5GZYC4WLNHCHCKW4NEGXNEZRYWLTNZIRJJY7D2\",\n"
            "      \"key\": \"GCR2KBCIU6KQXSQY5F5GZYC4WLNHCHCKW4NEGXNEZRYWLTNZIRJJY7D2\",\n"
            "      \"weight\": 1,\n"
            "      \"type\": \"ed25519_public_key\"\n"
            "    }\n"
            "  ],\n"
            "  \"data\": {\n"
            "    \"entry1\": \"dGVzdA==\",\n"
            "    \"entry2\": \"dGVzdDI=\"\n"
            "  },\n"
            "  \"num_sponsoring\": 0,\n"
            "  \"num_sponsored\": 0\n"
            "}";

    const QByteArray jsonAuthorizedToMaintainLiabilities = "{\n"
          "  \"_links\": {\n"
          "    \"effects\": {\n"
          "      \"href\": \"/accounts/GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7/effects{?cursor,limit,order}\",\n"
          "      \"templated\": true\n"
          "    },\n"
          "    \"offers\": {\n"
          "      \"href\": \"/accounts/GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7/offers{?cursor,limit,order}\",\n"
          "      \"templated\": true\n"
          "    },\n"
          "    \"operations\": {\n"
          "      \"href\": \"/accounts/GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7/operations{?cursor,limit,order}\",\n"
          "      \"templated\": true\n"
          "    },\n"
          "    \"self\": {\n"
          "      \"href\": \"/accounts/GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7\"\n"
          "    },\n"
          "    \"transactions\": {\n"
          "      \"href\": \"/accounts/GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7/transactions{?cursor,limit,order}\",\n"
          "      \"templated\": true\n"
          "    }\n"
          "  },"
          "  \"id\": \"GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7\",\n"
          "  \"paging_token\": \"1\",\n"
          "  \"account_id\": \"GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7\",\n"
          "  \"sequence\": 2319149195853854,\n"
          "  \"subentry_count\": 0,\n"
          "  \"inflation_destination\": \"GAGRSA6QNQJN2OQYCBNQGMFLO4QLZFNEHIFXOMTQVSUTWVTWT66TOFSC\",\n"
          "  \"home_domain\": \"stellar.org\",\n"
          "  \"thresholds\": {\n"
          "    \"low_threshold\": 10,\n"
          "    \"med_threshold\": 20,\n"
          "    \"high_threshold\": 30\n"
          "  },\n"
          "  \"flags\": {\n"
          "    \"auth_required\": false,\n"
          "    \"auth_revocable\": true,\n"
          "    \"auth_immutable\": true,\n"
          "    \"auth_clawback_enabled\": false\n"
          "  },\n"
          "  \"balances\": [\n"
          "    {\n"
          "      \"balance\": \"1001.0000000\",\n"
          "      \"buying_liabilities\": \"100.1234567\",\n"
          "      \"selling_liabilities\": \"100.7654321\",\n"
          "      \"limit\": \"12000.4775807\",\n"
          "      \"asset_type\": \"credit_alphanum4\",\n"
          "      \"asset_code\": \"ABC\",\n"
          "      \"asset_issuer\": \"GCRA6COW27CY5MTKIA7POQ2326C5ABYCXODBN4TFF5VL4FMBRHOT3YHU\",\n"
          "      \"is_authorized\": false,\n"
          "      \"is_authorized_to_maintain_liabilities\": true\n"
          "    },"
          "    {\n"
          "      \"asset_type\": \"native\",\n"
          "      \"balance\": \"20.0000300\",\n"
          "      \"buying_liabilities\": \"5.1234567\",\n"
          "      \"selling_liabilities\": \"1.7654321\"\n"
          "    }\n"
          "  ],\n"
          "  \"signers\": [\n"
          "    {\n"
          "      \"public_key\": \"GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7\",\n"
          "      \"key\": \"GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7\",\n"
          "      \"weight\": 0,\n"
          "      \"type\": \"ed25519_public_key\"\n"
          "    },\n"
          "    {\n"
          "      \"public_key\": \"GCR2KBCIU6KQXSQY5F5GZYC4WLNHCHCKW4NEGXNEZRYWLTNZIRJJY7D2\",\n"
          "      \"key\": \"GCR2KBCIU6KQXSQY5F5GZYC4WLNHCHCKW4NEGXNEZRYWLTNZIRJJY7D2\",\n"
          "      \"weight\": 1,\n"
          "      \"type\": \"ed25519_public_key\"\n"
          "    }\n"
          "  ],\n"
          "  \"data\": {\n"
          "    \"entry1\": \"dGVzdA==\",\n"
          "    \"entry2\": \"dGVzdDI=\"\n"
          "  },\n"
          "  \"num_sponsoring\": 0,\n"
          "  \"num_sponsored\": 0\n"
          "}";
    const QByteArray withSponsor = "{\n"
          "  \"_links\": {\n"
          "    \"self\": {\n"
          "      \"href\": \"https://horizon-protocol14.stellar.org/accounts/GB56OJGSA6VHEUFZDX6AL2YDVG2TS5JDZYQJHDYHBDH7PCD5NIQKLSDO\"\n"
          "    },\n"
          "    \"transactions\": {\n"
          "      \"href\": \"https://horizon-protocol14.stellar.org/accounts/GB56OJGSA6VHEUFZDX6AL2YDVG2TS5JDZYQJHDYHBDH7PCD5NIQKLSDO/transactions{?cursor,limit,order}\",\n"
          "      \"templated\": true\n"
          "    },\n"
          "    \"operations\": {\n"
          "      \"href\": \"https://horizon-protocol14.stellar.org/accounts/GB56OJGSA6VHEUFZDX6AL2YDVG2TS5JDZYQJHDYHBDH7PCD5NIQKLSDO/operations{?cursor,limit,order}\",\n"
          "      \"templated\": true\n"
          "    },\n"
          "    \"payments\": {\n"
          "      \"href\": \"https://horizon-protocol14.stellar.org/accounts/GB56OJGSA6VHEUFZDX6AL2YDVG2TS5JDZYQJHDYHBDH7PCD5NIQKLSDO/payments{?cursor,limit,order}\",\n"
          "      \"templated\": true\n"
          "    },\n"
          "    \"effects\": {\n"
          "      \"href\": \"https://horizon-protocol14.stellar.org/accounts/GB56OJGSA6VHEUFZDX6AL2YDVG2TS5JDZYQJHDYHBDH7PCD5NIQKLSDO/effects{?cursor,limit,order}\",\n"
          "      \"templated\": true\n"
          "    },\n"
          "    \"offers\": {\n"
          "      \"href\": \"https://horizon-protocol14.stellar.org/accounts/GB56OJGSA6VHEUFZDX6AL2YDVG2TS5JDZYQJHDYHBDH7PCD5NIQKLSDO/offers{?cursor,limit,order}\",\n"
          "      \"templated\": true\n"
          "    },\n"
          "    \"trades\": {\n"
          "      \"href\": \"https://horizon-protocol14.stellar.org/accounts/GB56OJGSA6VHEUFZDX6AL2YDVG2TS5JDZYQJHDYHBDH7PCD5NIQKLSDO/trades{?cursor,limit,order}\",\n"
          "      \"templated\": true\n"
          "    },\n"
          "    \"data\": {\n"
          "      \"href\": \"https://horizon-protocol14.stellar.org/accounts/GB56OJGSA6VHEUFZDX6AL2YDVG2TS5JDZYQJHDYHBDH7PCD5NIQKLSDO/data/{key}\",\n"
          "      \"templated\": true\n"
          "    }\n"
          "  },\n"
          "  \"id\": \"GB56OJGSA6VHEUFZDX6AL2YDVG2TS5JDZYQJHDYHBDH7PCD5NIQKLSDO\",\n"
          "  \"account_id\": \"GB56OJGSA6VHEUFZDX6AL2YDVG2TS5JDZYQJHDYHBDH7PCD5NIQKLSDO\",\n"
          "  \"sequence\": \"504211980681218\",\n"
          "  \"subentry_count\": 1,\n"
          "  \"last_modified_ledger\": 117663,\n"
          "  \"last_modified_time\": \"2020-09-28T17:56:04Z\",\n"
          "  \"thresholds\": {\n"
          "    \"low_threshold\": 0,\n"
          "    \"med_threshold\": 0,\n"
          "    \"high_threshold\": 0\n"
          "  },\n"
          "  \"flags\": {\n"
          "    \"auth_required\": false,\n"
          "    \"auth_revocable\": false,\n"
          "    \"auth_immutable\": false,\n"
          "    \"auth_clawback_enabled\": false\n"
          "  },\n"
          "  \"balances\": [\n"
          "    {\n"
          "      \"sponsor\": \"GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7\",\n"
          "      \"balance\": \"9999.9999800\",\n"
          "      \"buying_liabilities\": \"0.0000000\",\n"
          "      \"selling_liabilities\": \"0.0000000\",\n"
          "      \"asset_type\": \"native\"\n"
          "    }\n"
          "  ],\n"
          "  \"signers\": [\n"
          "    {\n"
          "      \"sponsor\": \"GCR2KBCIU6KQXSQY5F5GZYC4WLNHCHCKW4NEGXNEZRYWLTNZIRJJY7D2\",\n"
          "      \"weight\": 1,\n"
          "      \"key\": \"GB56OJGSA6VHEUFZDX6AL2YDVG2TS5JDZYQJHDYHBDH7PCD5NIQKLSDO\",\n"
          "      \"type\": \"ed25519_public_key\"\n"
          "    }\n"
          "  ],\n"
          "  \"data\": {\n"
          "    \"welcome-friend\": \"aHR0cHM6Ly93d3cueW91dHViZS5jb20vd2F0Y2g/dj1kUXc0dzlXZ1hjUQ==\"\n"
          "  },\n"
          "  \"num_sponsoring\": 2,\n"
          "  \"num_sponsored\": 3,\n"
          "  \"sponsor\": \"GCA7RXNKN7FGBLJVETJCUUXGXTCR6L2SJQFXDGMQCDET5YUE6KFNHQHO\",\n"
          "  \"paging_token\": \"GB56OJGSA6VHEUFZDX6AL2YDVG2TS5JDZYQJHDYHBDH7PCD5NIQKLSDO\"\n"
          "}";
public:


private slots:

    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }
    void testDeserialize() {
      AccountResponse account(0);
      account.loadFromJson(json);
      QCOMPARE(account.getKeypair()->getAccountId(), QString("GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7"));
      QCOMPARE(account.getPagingToken(), QString("1"));
      QCOMPARE(account.getSequenceNumber(), 2319149195853854L);
      QCOMPARE(account.getSequenceLedger(), 47274621L);
      QCOMPARE(account.getSequenceTime(), QString("1640995200"));
      QCOMPARE(account.getSubentryCount(), 0);
      QCOMPARE(account.getInflationDestination(), QString("GAGRSA6QNQJN2OQYCBNQGMFLO4QLZFNEHIFXOMTQVSUTWVTWT66TOFSC"));
      QCOMPARE(account.getHomeDomain(), QString("stellar.org"));
      QVERIFY(account.getSponsor().isNull());

      QCOMPARE(account.getThresholds().getLowThreshold(), 10);
      QCOMPARE(account.getThresholds().getMedThreshold(), 20);
      QCOMPARE(account.getThresholds().getHighThreshold(), 30);

      QCOMPARE(account.getFlags().getAuthRequired(), false);
      QCOMPARE(account.getFlags().getAuthRevocable(), true);
      QCOMPARE(account.getFlags().getAuthImmutable(), true);
      QCOMPARE(account.getFlags().getAuthClawbackEnabled(), true);

      QCOMPARE(account.getBalances()[0].getAssetType(), QString("credit_alphanum4"));
      QCOMPARE(account.getBalances()[0].getAssetCode(), QString("ABC"));
      QCOMPARE(account.getBalances()[0].getAssetIssuer().getAccountId(), QString("GCRA6COW27CY5MTKIA7POQ2326C5ABYCXODBN4TFF5VL4FMBRHOT3YHU"));
      QCOMPARE(account.getBalances()[0].getBalance(), QString("1001.0000000"));
      QCOMPARE(account.getBalances()[0].getLimit(), QString("12000.4775807"));
      QVERIFY(account.getBalances()[0].getSponsor().isNull());
      QCOMPARE(account.getBalances()[0].getClawbackEnabled(), true);

      QCOMPARE(account.getBalances()[1].getAssetType(), QString("native"));
      QCOMPARE(account.getBalances()[1].getBalance(), QString("20.0000300"));
      QCOMPARE(account.getBalances()[1].getLimit(), QString());
      QVERIFY(account.getBalances()[1].getSponsor().isNull());

      // P1.8 — liquidity_pool_shares balance must not throw on getAsset()
      QCOMPARE(account.getBalances()[2].getAssetType(), QString("liquidity_pool_shares"));
      QCOMPARE(account.getBalances()[2].getLiquidityPoolId(), QString("dd7b1ab831c273310ddbec6f97870aa83c2fbcf57320c0bd000242331b46f1d2"));
      QVERIFY(account.getBalances()[2].getAsset() == nullptr);

      QCOMPARE(account.getSigners()[0].getType(), QString("ed25519_public_key"));
      QCOMPARE(account.getSigners()[0].getKey(), QString("GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7"));
      QCOMPARE(account.getSigners()[0].getWeight(), 0);
      QVERIFY(account.getSigners()[0].getSponsor().isNull());
      QCOMPARE(account.getSigners()[1].getType(), QString("ed25519_public_key"));
      QCOMPARE(account.getSigners()[1].getKey(), QString("GCR2KBCIU6KQXSQY5F5GZYC4WLNHCHCKW4NEGXNEZRYWLTNZIRJJY7D2"));
      QCOMPARE(account.getSigners()[1].getWeight(), 1);
      QVERIFY(account.getSigners()[1].getSponsor().isNull());


      QCOMPARE(account.getData().size(), 2);
      QCOMPARE(account.getData().get("entry1"), "dGVzdA==");
      QCOMPARE(account.getData().getDecoded("entry1"), QByteArray("test"));
      QCOMPARE(account.getData().get("entry2"), "dGVzdDI=");
      QCOMPARE(account.getData().getDecoded("entry2"), QByteArray("test2"));


      QCOMPARE(account.getLinks().getEffects().getHref(),QString( "/accounts/GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7/effects{?cursor,limit,order}"));
      QCOMPARE(account.getLinks().getOffers().getHref(), QString("/accounts/GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7/offers{?cursor,limit,order}"));
      QCOMPARE(account.getLinks().getOperations().getHref(), QString("/accounts/GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7/operations{?cursor,limit,order}"));
      QCOMPARE(account.getLinks().getSelf().getHref(), QString("/accounts/GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7"));
      QCOMPARE(account.getLinks().getTransactions().getHref(), QString("/accounts/GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7/transactions{?cursor,limit,order}"));
    }
    void testDeserializeBalanceAuth() {
        AccountResponse account(0);
        account.loadFromJson(jsonAuthorizedToMaintainLiabilities);

        QCOMPARE(account.getBalances()[0].getAssetType(), "credit_alphanum4");
        QCOMPARE(account.getBalances()[0].getAssetCode(), "ABC");
        QCOMPARE(account.getBalances()[0].getAssetIssuer().getAccountId(), "GCRA6COW27CY5MTKIA7POQ2326C5ABYCXODBN4TFF5VL4FMBRHOT3YHU");
        QCOMPARE(account.getBalances()[0].getBalance(), "1001.0000000");
        QCOMPARE(account.getBalances()[0].getLimit(), "12000.4775807");
        QCOMPARE(account.getBalances()[0].getBuyingLiabilities(), "100.1234567");
        QCOMPARE(account.getBalances()[0].getSellingLiabilities(), "100.7654321");
        QCOMPARE(account.getBalances()[0].getAuthorized(), false);
        QCOMPARE(account.getBalances()[0].getAuthorizedToMaintainLiabilities(), true);

        QCOMPARE(account.getBalances()[1].getAssetType(), "native");
        QCOMPARE(account.getBalances()[1].getBalance(), "20.0000300");
        QCOMPARE(account.getBalances()[1].getBuyingLiabilities(), "5.1234567");
        QCOMPARE(account.getBalances()[1].getSellingLiabilities(), "1.7654321");
        QCOMPARE(account.getBalances()[1].getLimit(), QString());
    }



    void testDeserializeSponsor() {
      AccountResponse account(0);
      account.loadFromJson(withSponsor);
      QCOMPARE(account.getSponsor(), "GCA7RXNKN7FGBLJVETJCUUXGXTCR6L2SJQFXDGMQCDET5YUE6KFNHQHO");
      QCOMPARE(account.getBalances()[0].getSponsor(), "GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7");
      QCOMPARE(account.getSigners()[0].getSponsor(), "GCR2KBCIU6KQXSQY5F5GZYC4WLNHCHCKW4NEGXNEZRYWLTNZIRJJY7D2");
      QCOMPARE(account.getNumSponsored(), 3);
      QCOMPARE(account.getNumSponsoring(), 2);
    }

    // All four signer key types — only ed25519_public_key was exercised
    // before. The non-G strkey types (X = sha256 hash, T = pre-auth tx,
    // P = ed25519 signed payload, CAP-40) must deserialize their `key`,
    // `type` and `weight` like any other signer.
    void testDeserializeSignerTypes() {
        QByteArray json = "{"
            "\"_links\":{\"self\":{\"href\":\"/accounts/GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7\"},"
            "\"transactions\":{\"href\":\"/accounts/GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7/transactions\"},"
            "\"operations\":{\"href\":\"/accounts/GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7/operations\"},"
            "\"effects\":{\"href\":\"/accounts/GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7/effects\"},"
            "\"offers\":{\"href\":\"/accounts/GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7/offers\"}},"
            "\"id\":\"GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7\","
            "\"account_id\":\"GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7\","
            "\"paging_token\":\"GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7\","
            "\"sequence\":\"1\",\"subentry_count\":3,\"num_sponsoring\":0,\"num_sponsored\":0,"
            "\"thresholds\":{\"low_threshold\":0,\"med_threshold\":0,\"high_threshold\":0},"
            "\"flags\":{\"auth_required\":false,\"auth_revocable\":false,\"auth_immutable\":false,\"auth_clawback_enabled\":false},"
            "\"balances\":[{\"balance\":\"1.0000000\",\"buying_liabilities\":\"0.0000000\","
            "\"selling_liabilities\":\"0.0000000\",\"asset_type\":\"native\"}],\"data\":{},"
            "\"signers\":["
                "{\"key\":\"GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7\",\"weight\":3,\"type\":\"ed25519_public_key\"},"
                "{\"key\":\"XDRPF6NZRR7EEVO7ESIWUDXHAOMM2QSKIQQBJK6I2FB7YKDZES5UCLWD\",\"weight\":2,\"type\":\"sha256_hash\"},"
                "{\"key\":\"TDRPF6NZRR7EEVO7ESIWUDXHAOMM2QSKIQQBJK6I2FB7YKDZES5UCLWD\",\"weight\":1,\"type\":\"preauth_tx\"},"
                "{\"key\":\"PA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVAI\",\"weight\":4,\"type\":\"ed25519_signed_payload\"}"
            "]}";
        AccountResponse account(0);
        account.loadFromJson(json);

        auto signers = account.getSigners();
        QCOMPARE(signers.size(), 4);

        QCOMPARE(signers[0].getType(), QString("ed25519_public_key"));
        QCOMPARE(signers[0].getKey(), QString("GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7"));
        QCOMPARE(signers[0].getWeight(), 3);

        QCOMPARE(signers[1].getType(), QString("sha256_hash"));
        QCOMPARE(signers[1].getKey(), QString("XDRPF6NZRR7EEVO7ESIWUDXHAOMM2QSKIQQBJK6I2FB7YKDZES5UCLWD"));
        QCOMPARE(signers[1].getWeight(), 2);

        QCOMPARE(signers[2].getType(), QString("preauth_tx"));
        QCOMPARE(signers[2].getKey(), QString("TDRPF6NZRR7EEVO7ESIWUDXHAOMM2QSKIQQBJK6I2FB7YKDZES5UCLWD"));
        QCOMPARE(signers[2].getWeight(), 1);

        QCOMPARE(signers[3].getType(), QString("ed25519_signed_payload"));
        QCOMPARE(signers[3].getKey(), QString("PA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVAI"));
        QCOMPARE(signers[3].getWeight(), 4);
    }

    // ---- Exact reading (1.0.4) -------------------------------------------
private:
    /** The member of `json` at `path` ("a.b[1].c") replaced by `value`, or removed when undefined. */
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

    /** What loading `json` with `member` set to `value` throws; empty if it loads. */
    static QString refusal(const QByteArray& json, const QString& member, const QJsonValue& value)
    {
        const QJsonValue changed = edited(QJsonDocument::fromJson(json).object(),
                                          member.split(QLatin1Char('.')), value);
        AccountResponse account(nullptr);
        try {
            account.loadFromJson(QJsonDocument(changed.toObject()).toJson());
        } catch (const std::exception& e) {
            return QString::fromUtf8(e.what());
        }
        return QString();
    }

private slots:
    /**
     * The sequence a transaction is signed with, exactly. Text that is not a
     * number, a fraction, a number past int64 and a JSON number past 2^53
     * (already rounded by the parser) all used to leave it at 0, or at a
     * rounded value; missing or null, it was 0 too.
     */
    void testSequenceIsReadExactly()
    {
        const QByteArray base = json;
        QVERIFY(refusal(base, "sequence", QString("12x")).startsWith("sequence: "));
        QVERIFY(refusal(base, "sequence", 1.5).startsWith("sequence: "));
        QVERIFY(refusal(base, "sequence", 9007199254740993.0).startsWith("sequence: "));
        QVERIFY(refusal(base, "sequence", QString("9223372036854775808")).startsWith("sequence: "));
        QVERIFY(refusal(base, "sequence", QString(" 12")).startsWith("sequence: "));
        QVERIFY(refusal(base, "sequence", QJsonValue()).startsWith("sequence: "));
        QVERIFY(refusal(base, "sequence", QJsonValue(QJsonValue::Undefined)).startsWith("sequence: "));

        AccountResponse account(nullptr);
        QJsonObject max = QJsonDocument::fromJson(json).object();
        max["sequence"] = QString("9223372036854775807");
        account.loadFromJson(QJsonDocument(max).toJson());
        QCOMPARE(account.getSequenceNumber(), std::numeric_limits<qint64>::max());
        max["sequence"] = 9007199254740991.0; // 2^53 - 1, exact as a JSON number
        account.loadFromJson(QJsonDocument(max).toJson());
        QCOMPARE(account.getSequenceNumber(), Q_INT64_C(9007199254740991));
    }

    /**
     * A balance's amounts, limit and asset, exactly: a limit given as a
     * number used to become empty, an amount with 8 decimals or text that is
     * not an amount was passed on as it came, and a credit_alphanum4 type
     * with a 5-letter code became an alphanum12 asset.
     */
    void testBalancesAreReadExactly()
    {
        const QByteArray base = json;
        QVERIFY(refusal(base, "balances[0].limit", 12000).startsWith("balances[0].limit: "));
        QVERIFY(refusal(base, "balances[0].balance", QString("1001.00000001")).startsWith("balances[0].balance: "));
        QVERIFY(refusal(base, "balances[0].balance", QString("1,001")).startsWith("balances[0].balance: "));
        QVERIFY(refusal(base, "balances[0].balance", QString("-1.0")).startsWith("balances[0].balance: "));
        QVERIFY(refusal(base, "balances[0].buying_liabilities", 0).startsWith("balances[0].buying_liabilities: "));
        QVERIFY(refusal(base, "balances[0].limit", QJsonValue(QJsonValue::Undefined)).startsWith("balances[0].limit: "));
        QVERIFY(refusal(base, "balances[0].asset_code", QString("ABCDE")).startsWith("balances[0].asset_code: "));
        QVERIFY(refusal(base, "balances[0].asset_issuer", QString("GNOTANACCOUNT")).startsWith("balances[0].asset_issuer: "));
        QVERIFY(refusal(base, "balances[0].asset_type", QString("credit_alphanum5")).startsWith("balances[0].asset_type: "));
        QVERIFY(refusal(base, "balances[0].is_authorized", QString("true")).startsWith("balances[0].is_authorized: "));
        QVERIFY(refusal(base, "balances[2].liquidity_pool_id", QJsonValue(QJsonValue::Undefined)).startsWith("balances[2].liquidity_pool_id: "));
        QVERIFY(refusal(base, "balances[1]", QString("native")).startsWith("balances[1]: "));
        QVERIFY(refusal(base, "balances", QJsonObject()).startsWith("balances: "));
        QVERIFY(refusal(base, "balances", QJsonValue(QJsonValue::Undefined)).startsWith("balances: "));
    }

    /**
     * Thresholds are one byte on the ledger, signer weights whole numbers,
     * and flags booleans: 300, a weight of 1.5 or "true" used to be kept as
     * 300, or read as 0 and false, and a transaction built on them signed
     * with the wrong expectations.
     */
    void testThresholdsWeightsAndFlagsAreReadExactly()
    {
        const QByteArray base = json;
        QVERIFY(refusal(base, "thresholds.low_threshold", 256).startsWith("thresholds.low_threshold: "));
        QVERIFY(refusal(base, "thresholds.med_threshold", -1).startsWith("thresholds.med_threshold: "));
        QVERIFY(refusal(base, "thresholds.high_threshold", QString("30x")).startsWith("thresholds.high_threshold: "));
        QVERIFY(refusal(base, "thresholds.high_threshold", QJsonValue(QJsonValue::Undefined)).startsWith("thresholds.high_threshold: "));
        QVERIFY(refusal(base, "signers[1].weight", 1.5).startsWith("signers[1].weight: "));
        // A weight past a byte is a real signer's (see
        // testSignerWeightsAreTheInt32HorizonSends); this expected a refusal.
        QVERIFY(refusal(base, "signers[1].weight", 256).isEmpty());
        QVERIFY(refusal(base, "signers[1].weight", QJsonValue()).startsWith("signers[1].weight: "));
        QVERIFY(refusal(base, "signers[1]", QJsonObject{{"weight", 1}, {"type", "ed25519_public_key"}})
                    .startsWith("signers[1].key: "));
        QVERIFY(refusal(base, "flags.auth_required", QString("true")).startsWith("flags.auth_required: "));
        QVERIFY(refusal(base, "flags.auth_revocable", 1).startsWith("flags.auth_revocable: "));
        QVERIFY(refusal(base, "flags", QJsonValue()).startsWith("flags: "));
        QVERIFY(refusal(base, "subentry_count", QString("zero")).startsWith("subentry_count: "));
        QVERIFY(refusal(base, "num_sponsoring", 2.5).startsWith("num_sponsoring: "));
        QVERIFY(refusal(base, "sequence_time", QString("yesterday")).startsWith("sequence_time: "));
        // A weight given as decimal text is the same number, exactly.
        QVERIFY(refusal(base, "signers[1].weight", QString("1")).isEmpty());
    }

    /**
     * A data entry's value is the base64 of at most 64 bytes, and SEP-29
     * compares it with "MQ==". Anything else was kept as it came, and
     * getDecoded() turned it into other bytes.
     */
    void testDataValuesAreBase64()
    {
        const QByteArray base = json;
        QVERIFY(refusal(base, "data.entry1", QString("not base64!")).startsWith("data.entry1: "));
        QVERIFY(refusal(base, "data.entry1", QString("dGVzdA")).startsWith("data.entry1: "));
        QVERIFY(refusal(base, "data.entry1", 1).startsWith("data.entry1: "));
        QVERIFY(refusal(base, "data.entry1", QString(QByteArray(65, 'x').toBase64())).startsWith("data.entry1: "));
        QVERIFY(refusal(base, "data", QJsonValue(QJsonValue::Undefined)).startsWith("data: "));
    }

    /**
     * A native balance has no is_authorized and no last_modified_ledger, and
     * Balance left both uninitialised: getAuthorized() read whatever the
     * memory held (true in a debug build).
     */
    void testBalanceMembersStartDefined()
    {
        AccountResponseAttach::Balance balance;
        QCOMPARE(balance.getAuthorized(), false);
        QCOMPARE(balance.getAuthorizedToMaintainLiabilities(), false);
        QCOMPARE(balance.getLastModifiedLedger(), 0);
        AccountResponse account(nullptr);
        account.loadFromJson(json);
        QCOMPARE(account.getBalances()[1].getAssetType(), QString("native"));
        QCOMPARE(account.getBalances()[1].getAuthorized(), false);
        QCOMPARE(account.getBalances()[1].getLastModifiedLedger(), 0);
    }

    /** An answer that is not an object used to load as an empty account. */
    void testAnAccountMustBeAnObject()
    {
        AccountResponse account(nullptr);
        QVERIFY_EXCEPTION_THROWN(account.loadFromJson("[1,2]"), std::runtime_error);
        QVERIFY_EXCEPTION_THROWN(account.loadFromJson("\"GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7\""), std::runtime_error);
    }

    /**
     * Before protocol 10 stellar-core took any 32-bit signer weight (it
     * counts one past 255 as 255), and those signers are still on the
     * ledger. Horizon sends the weight as an int32: 256 as it is, 2^31 and
     * past as a negative number. Such an account was refused (weights were
     * held to 0..255): it could not be loaded, nor paid through SEP-29's
     * lookup. Its weights are read exactly; a weight outside int32, or not
     * a whole number, is still refused.
     */
    void testSignerWeightsAreTheInt32HorizonSends()
    {
        const QByteArray base = json;
        for (const qint64 weight : { Q_INT64_C(256), Q_INT64_C(300), Q_INT64_C(2147483647),
                                     Q_INT64_C(-1294967296), Q_INT64_C(-2147483648) }) {
            const QJsonValue changed = edited(QJsonDocument::fromJson(base).object(),
                                              QStringList() << "signers[1]" << "weight",
                                              QJsonValue(static_cast<double>(weight)));
            AccountResponse account(nullptr);
            account.loadFromJson(QJsonDocument(changed.toObject()).toJson());
            QCOMPARE(static_cast<qint64>(account.getSigners()[1].getWeight()), weight);
            QCOMPARE(account.getSigners()[1].getKey(), QString("GCR2KBCIU6KQXSQY5F5GZYC4WLNHCHCKW4NEGXNEZRYWLTNZIRJJY7D2"));
        }
        QVERIFY(refusal(base, "signers[1].weight", QString("256")).isEmpty());
        QVERIFY(refusal(base, "signers[1].weight", 2147483648.0).startsWith("signers[1].weight: "));
        QVERIFY(refusal(base, "signers[1].weight", -2147483649.0).startsWith("signers[1].weight: "));
        QVERIFY(refusal(base, "signers[1].weight", 256.5).startsWith("signers[1].weight: "));
        // Thresholds and flags are unchanged: one byte, booleans.
        QVERIFY(refusal(base, "thresholds.high_threshold", 256).startsWith("thresholds.high_threshold: "));
    }

    /**
     * Every member Horizon always sends in an account is required. Missing
     * or null, the sponsoring counts read 0, the clawback flag false (as if
     * the issuer could not claw back), the paging token and links empty.
     */
    void testMembersHorizonAlwaysSendsAreRequired()
    {
        const QByteArray base = realTestnetAccountRequiringMemo();
        QVERIFY(refusal(base, "sequence", QString("21118873885474923")).isEmpty());
        const char* const members[] = {
            "account_id", "sequence", "subentry_count", "thresholds", "flags", "balances", "signers", "data",
            "num_sponsoring", "num_sponsored", "paging_token", "_links",
            "thresholds.low_threshold", "thresholds.med_threshold", "thresholds.high_threshold",
            "flags.auth_required", "flags.auth_revocable", "flags.auth_immutable", "flags.auth_clawback_enabled",
            "balances[0].asset_type", "balances[0].balance", "signers[0].weight", "signers[0].type",
            "_links.self", "_links.transactions", "_links.operations", "_links.effects", "_links.offers",
            "_links.self.href"
        };
        for (const char* member : members) {
            QVERIFY2(refusal(base, member, QJsonValue(QJsonValue::Undefined)).startsWith(QString(member) + ": "), member);
            QVERIFY2(refusal(base, member, QJsonValue()).startsWith(QString(member) + ": "), member);
        }
        // What Horizon leaves out when it has nothing to say stays optional.
        for (const char* member : { "sequence_ledger", "sequence_time", "inflation_destination", "home_domain",
                                    "sponsor", "last_modified_time", "_links.trades", "_links.data" })
            QVERIFY2(refusal(base, member, QJsonValue(QJsonValue::Undefined)).isEmpty(), member);
    }

    // ---- Real Horizon answers --------------------------------------------

    /**
     * Real accounts, from pubnet and testnet, load and read as Horizon wrote
     * them: liquidity-pool shares, credit lines of both lengths, a vault
     * co-signer, SEP-29's data entry, a sponsored reserve.
     */
    void testRealHorizonAccounts()
    {
        AccountResponse lp(nullptr);
        lp.loadFromJson(realPubnetAccountWithPoolShares());
        QCOMPARE(lp.getSequenceNumber(), Q_INT64_C(89360273516790010));
        QCOMPARE(lp.getSubentryCount(), 13);
        QCOMPARE(lp.getBalances().size(), 9);
        QCOMPARE(lp.getBalances()[0].getAssetType(), QString("liquidity_pool_shares"));
        QCOMPARE(lp.getBalances()[0].getLiquidityPoolId(), QString("59fa1dc57433dcfbd2db7319d26cb3da1f28f2d8095a3ec36ad4ef9cadb0013e"));
        QCOMPARE(lp.getBalances()[0].getBalance(), QString("3933.6685577"));
        QCOMPARE(lp.getBalances()[0].getLimit(), QString("922337203685.4775807"));
        QCOMPARE(lp.getBalances()[6].getAssetCode(), QString("yUSDC"));
        QCOMPARE(lp.getBalances()[6].getAsset()->getType(), QString("credit_alphanum12"));
        QCOMPARE(lp.getBalances()[8].getAssetType(), QString("native"));
        QCOMPARE(lp.getBalances()[8].getBalance(), QString("1007.9106218"));
        // What Horizon left out is told apart from a false it sent.
        QVERIFY(lp.getBalances()[0].hasAuthorized());
        QVERIFY(!lp.getBalances()[0].getAuthorized());
        QVERIFY(!lp.getBalances()[0].hasClawbackEnabled());
        QVERIFY(lp.getBalances()[3].hasAuthorizedToMaintainLiabilities());
        QVERIFY(!lp.getBalances()[8].hasAuthorized());
        QVERIFY(!lp.getBalances()[8].hasAuthorizedToMaintainLiabilities());
        QVERIFY(!lp.getBalances()[8].hasClawbackEnabled());
        QCOMPARE(lp.getThresholds().getHighThreshold(), 20);
        QCOMPARE(lp.getSigners().size(), 3);
        QCOMPARE(lp.getSigners()[0].getWeight(), 10);
        QCOMPARE(lp.getSigners()[1].getKey(), QString("GA2T6GR7VXXXBETTERSAFETHANSORRYXXXPROTECTEDBYLOBSTRVAULT"));
        // Dropped before 1.0.4; js and py hand both through.
        QCOMPARE(lp.getLastModifiedLedger(), Q_INT64_C(64665748));
        QCOMPARE(lp.getLastModifiedTime(), QString("2026-09-28T16:55:21Z"));

        AccountResponse memo(nullptr);
        memo.loadFromJson(realTestnetAccountRequiringMemo());
        QCOMPARE(memo.getData().get("config.memo_required"), QString("MQ=="));
        QCOMPARE(memo.getData().keys(), QStringList() << "config.memo_required");
        QCOMPARE(memo.getFlags().getAuthClawbackEnabled(), true);
        QCOMPARE(memo.getSequenceNumber(), Q_INT64_C(21118873885474923));

        AccountResponse sponsored(nullptr);
        sponsored.loadFromJson(realTestnetSponsoredAccount());
        QCOMPARE(sponsored.getSponsor(), QString("GCALKXRTML6DHW2CT332CVAOYEWZQ2OBXGHW3YNY4ZLOE2QE3OFH7LLA"));
        QCOMPARE(sponsored.getNumSponsored(), 2);

        AccountResponse trust(nullptr);
        trust.loadFromJson(realTestnetAccountWithTrustlines());
        QCOMPARE(trust.getSequenceNumber(), Q_INT64_C(21118878180442119));
        QCOMPARE(trust.getData().getDecoded("sp"), QByteArray("sponsored"));
        QCOMPARE(trust.getBalances()[1].getBalance(), QString("98997.6212331"));
        QCOMPARE(trust.getBalances()[1].getClawbackEnabled(), true);
        QCOMPARE(trust.getSigners()[1].getKey(), QString("GDEZMR6VMLXNPBJL4UJHJQXJ3NXVQ3PMC2FTQU6F5QAKEBRJNIAXY3BQ"));
    }

private:
    /** A real pubnet account: liquidity-pool shares, credit lines, three signers. */
    static QByteArray realPubnetAccountWithPoolShares()
    {
        return QByteArray(
            "{\"_links\":{\"self\":{\"href\":\"https://horizon.stellar.org/accounts/GA2OWF453G57QILHC5BYRXO3CRSTEDQNAAJPYDZVDH3E2X2NCUNVYIOO\"},\"transactions\":{\"href\":\"https://horizon.stellar.org/accounts/GA2OWF453G57QILHC5BYRXO3CRSTEDQNAAJPYDZVDH3E2X2NCUNVYIOO/transactions{?cursor,limit,order}\",\"templated\":true},\"operations\":{\"href\":\"https://horizon.stellar.org/accounts/GA2OWF453G57QILHC5BYRXO3CRSTEDQNAAJPYDZVDH3E2X2NCUNVYIOO/operations{?cursor,limit,order}\",\"templated\":true},\"payments\":{\"href\":\"https://horizon.stellar.org/accounts/GA2OWF453G57QILHC5BYRXO3CRSTEDQNAAJPYDZVDH3E2X2NCUNVYIOO/payments{?cursor,limit,order}\",\"templated\":true},\"effects\":{\"href\":\"https://horizon.stellar.org/accounts/GA2OWF453G57QILHC5BYRXO3CRSTEDQNAAJPYDZVDH3E2X2NCUNVYIOO/effects{?cursor,limit,order}\",\"templated\":true},\"offers\":{\"href\":\"https://horizon.stellar.org/accounts/GA2OWF453G57QILHC5BYRXO3CRSTEDQNAAJPYDZVDH3E2X2NCUNVYIOO/offers{?cursor,limit,order}\",\"templated\":true},\"trades\":{\"href\":\"https://horizon.stellar.org/accounts/GA2OWF453G57QILHC5BYRXO3CRSTEDQNAAJPYDZVDH3E2X2NCUNVYIOO/trades{?cursor,limit,order}\",\"templated\":true},\"data\":{\"href\":\"https://horizon.stellar.org/accounts/GA2OWF453G57QILHC5BYRXO3CRSTEDQNAAJPYDZVDH3E2X2NCUNVYIOO/data/{key}\",\"templated\":true}},\"id\":\"GA2OWF453G57QILHC5BYRXO3CRSTEDQNAAJPYDZVDH3E2X2NCUNVYIOO\",\"account_id\":\"GA2OWF453G57QILHC5BYRXO3CRSTEDQNAAJPYDZVDH3E2X2NCUNVYIOO\",\"sequence\":\"89360273516790010\",\"sequence_ledger\":64282661,\"sequence_time\":\"1788597283\",\"subentry_count\":13,\"inflation_destination\":\"GCCD6AJOYZCUAQLX32ZJF2MKFFAUJ53PVCFQI3RHWKL3V47QYE2BNAUT\",\"last_modified_ledger\":64665748,\"last_modified_time\":\"2026-09-28T16:55:21Z\",\"thresholds\":{\"low_threshold\":20,\"med_threshold\":20,\"high_threshold\":20},\"flags\":{\"auth_required\":false,\"auth_revocable\":false,\"auth_immutable\":"
            "false,\"auth_clawback_enabled\":false},\"balances\":[{\"balance\":\"3933.6685577\",\"liquidity_pool_id\":\"59fa1dc57433dcfbd2db7319d26cb3da1f28f2d8095a3ec36ad4ef9cadb0013e\",\"limit\":\"922337203685.4775807\",\"last_modified_ledger\":40868653,\"is_authorized\":false,\"is_authorized_to_maintain_liabilities\":false,\"asset_type\":\"liquidity_pool_shares\"},{\"balance\":\"0.6088223\",\"liquidity_pool_id\":\"960569e4e2456c7c9da41e3fcd9063a0edfdb4debd9c7eda620a6e9527eb68d2\",\"limit\":\"922337203685.4775807\",\"last_modified_ledger\":63238724,\"is_authorized\":false,\"is_authorized_to_maintain_liabilities\":false,\"asset_type\":\"liquidity_pool_shares\"},{\"balance\":\"183.7942593\",\"liquidity_pool_id\":\"a468d41d8e9b8f3c7209651608b74b7db7ac9952dcae0cdf24871d1d9c7b0088\",\"limit\":\"922337203685.4775807\",\"last_modified_ledger\":38117227,\"is_authorized\":false,\"is_authorized_to_maintain_liabilities\":false,\"asset_type\":\"liquidity_pool_shares\"},{\"balance\":\"138333.2165828\",\"limit\":\"922337203685.4775807\",\"buying_liabilities\":\"0.0000000\",\"selling_liabilities\":\"0.0000000\",\"last_modified_ledger\":64282661,\"is_authorized\":true,\"is_authorized_to_maintain_liabilities\":true,\"asset_type\":\"credit_alphanum4\",\"asset_code\":\"AQUA\",\"asset_issuer\":\"GBNZILSTVQZ4R7IKQDGHYGY2QXL5QOFJYQMXPKWRRM5PAV7Y4M67AQUA\"},{\"balance\":\"0.0021009\",\"limit\":\"922337203685.4775807\",\"buying_liabilities\":\"0.0000000\",\"selling_liabilities\":\"0.0000000\",\"last_modified_ledger\":63238724,\"is_authorized\":true,\"is_authorized_to_maintain_liabilities\":true,\"asset_type\":\"credit_alphanum4\",\"asset_code\":\"BTC\",\"asset_issuer\":\"GDPJALI4AZKUU2W426U5WKMAT6CN3AJRPIIRYR2YM54TL2GDWO5O2MZM\"},{\"balance\":\"4.2444631\",\"limit\":\"922337203685.4775807\",\"buying_liabilities\":\"0.0000000\",\"selling_liabilities\":\"0.0000000\",\"last_modified_ledger\":63238950,\"is"
            "_authorized\":true,\"is_authorized_to_maintain_liabilities\":true,\"asset_type\":\"credit_alphanum4\",\"asset_code\":\"USDC\",\"asset_issuer\":\"GA5ZSEJYB37JRC5AVCIA5MOP4RHTM335X2KGX3IHOJAPP5RE34K4KZVN\"},{\"balance\":\"235.2390944\",\"limit\":\"922337203685.4775807\",\"buying_liabilities\":\"0.0000000\",\"selling_liabilities\":\"0.0000000\",\"last_modified_ledger\":64661486,\"is_authorized\":true,\"is_authorized_to_maintain_liabilities\":true,\"asset_type\":\"credit_alphanum12\",\"asset_code\":\"yUSDC\",\"asset_issuer\":\"GDGTVWSM4MGS4T7Z6W4RPWOCHE2I6RDFCIFZGS3DOA63LWQTRNZNTTFF\"},{\"balance\":\"800.9500667\",\"limit\":\"922337203685.4775807\",\"buying_liabilities\":\"0.0000000\",\"selling_liabilities\":\"0.0000000\",\"last_modified_ledger\":64657168,\"is_authorized\":true,\"is_authorized_to_maintain_liabilities\":true,\"asset_type\":\"credit_alphanum4\",\"asset_code\":\"yXLM\",\"asset_issuer\":\"GARDNV3Q7YGT4AKSDF25LT32YSCCW4EV22Y2TV3I2PU2MMXJTEDL5T55\"},{\"balance\":\"1007.9106218\",\"buying_liabilities\":\"0.0000000\",\"selling_liabilities\":\"0.0000000\",\"asset_type\":\"native\"}],\"signers\":[{\"weight\":10,\"key\":\"GA2OWF453G57QILHC5BYRXO3CRSTEDQNAAJPYDZVDH3E2X2NCUNVYIOO\",\"type\":\"ed25519_public_key\"},{\"weight\":1,\"key\":\"GA2T6GR7VXXXBETTERSAFETHANSORRYXXXPROTECTEDBYLOBSTRVAULT\",\"type\":\"ed25519_public_key\"},{\"weight\":10,\"key\":\"GAY5MAFHYFYTKGEZK32JXYREQC3DO6GUEVHFTITGYEN2RRZUUGXK56RI\",\"type\":\"ed25519_public_key\"}],\"data\":{},\"num_sponsoring\":0,\"num_sponsored\":0,\"paging_token\":\"GA2OWF453G57QILHC5BYRXO3CRSTEDQNAAJPYDZVDH3E2X2NCUNVYIOO\"}");
    }

    /** A real testnet account that sets SEP-29's config.memo_required. */
    static QByteArray realTestnetAccountRequiringMemo()
    {
        return QByteArray(
            "{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD\"},\"transactions\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD/transactions{?cursor,limit,order}\",\"templated\":true},\"operations\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD/operations{?cursor,limit,order}\",\"templated\":true},\"payments\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD/payments{?cursor,limit,order}\",\"templated\":true},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD/effects{?cursor,limit,order}\",\"templated\":true},\"offers\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD/offers{?cursor,limit,order}\",\"templated\":true},\"trades\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD/trades{?cursor,limit,order}\",\"templated\":true},\"data\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD/data/{key}\",\"templated\":true}},\"id\":\"GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD\",\"account_id\":\"GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD\",\"sequence\":\"21118873885474923\",\"sequence_ledger\":4917146,\"sequence_time\":\"1790609317\",\"subentry_count\":2,\"inflation_destination\":\"GBTTX7QVGGB6VTNNAKMXIFR6YL3RYA6UU3YR7FVBLKNOB2SWSA7XMQ7H\",\"home_domain\":\"example.org\",\"last_modified_ledger\":4917147,\"last_modified_time\":\"2026-09-28T15:28:42Z\",\"thresholds\":{\"low_threshold\":1,\"med_threshold\":1,\"high_thre"
            "shold\":1},\"flags\":{\"auth_required\":false,\"auth_revocable\":true,\"auth_immutable\":false,\"auth_clawback_enabled\":true},\"balances\":[{\"balance\":\"19995.4998000\",\"buying_liabilities\":\"0.0000000\",\"selling_liabilities\":\"50.0000000\",\"asset_type\":\"native\"}],\"signers\":[{\"weight\":1,\"key\":\"GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD\",\"type\":\"ed25519_public_key\"}],\"data\":{\"config.memo_required\":\"MQ==\"},\"num_sponsoring\":2,\"num_sponsored\":0,\"paging_token\":\"GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD\"}");
    }

    /** A real testnet account whose reserve another account sponsors. */
    static QByteArray realTestnetSponsoredAccount()
    {
        return QByteArray(
            "{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/GBXD5XDNL5FKTDG7PCGFLKENVM3HZCM32JWN3FKFFVGQISFS2C6Q334E\"},\"transactions\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/GBXD5XDNL5FKTDG7PCGFLKENVM3HZCM32JWN3FKFFVGQISFS2C6Q334E/transactions{?cursor,limit,order}\",\"templated\":true},\"operations\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/GBXD5XDNL5FKTDG7PCGFLKENVM3HZCM32JWN3FKFFVGQISFS2C6Q334E/operations{?cursor,limit,order}\",\"templated\":true},\"payments\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/GBXD5XDNL5FKTDG7PCGFLKENVM3HZCM32JWN3FKFFVGQISFS2C6Q334E/payments{?cursor,limit,order}\",\"templated\":true},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/GBXD5XDNL5FKTDG7PCGFLKENVM3HZCM32JWN3FKFFVGQISFS2C6Q334E/effects{?cursor,limit,order}\",\"templated\":true},\"offers\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/GBXD5XDNL5FKTDG7PCGFLKENVM3HZCM32JWN3FKFFVGQISFS2C6Q334E/offers{?cursor,limit,order}\",\"templated\":true},\"trades\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/GBXD5XDNL5FKTDG7PCGFLKENVM3HZCM32JWN3FKFFVGQISFS2C6Q334E/trades{?cursor,limit,order}\",\"templated\":true},\"data\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/GBXD5XDNL5FKTDG7PCGFLKENVM3HZCM32JWN3FKFFVGQISFS2C6Q334E/data/{key}\",\"templated\":true}},\"id\":\"GBXD5XDNL5FKTDG7PCGFLKENVM3HZCM32JWN3FKFFVGQISFS2C6Q334E\",\"account_id\":\"GBXD5XDNL5FKTDG7PCGFLKENVM3HZCM32JWN3FKFFVGQISFS2C6Q334E\",\"sequence\":\"21118955489853440\",\"subentry_count\":0,\"last_modified_ledger\":4917140,\"last_modified_time\":\"2026-09-28T15:28:07Z\",\"thresholds\":{\"low_threshold\":0,\"med_threshold\":0,\"high_threshold\":0},\"flags\":{\"auth_required\":false,\"auth_revocable\":false,\"auth_immutable\":false,\"auth_clawback_enabled\":false},\"balances\":[{\"balance\":\"0.0000000\",\"buying_l"
            "iabilities\":\"0.0000000\",\"selling_liabilities\":\"0.0000000\",\"asset_type\":\"native\"}],\"signers\":[{\"weight\":1,\"key\":\"GBXD5XDNL5FKTDG7PCGFLKENVM3HZCM32JWN3FKFFVGQISFS2C6Q334E\",\"type\":\"ed25519_public_key\"}],\"data\":{},\"num_sponsoring\":0,\"num_sponsored\":2,\"sponsor\":\"GCALKXRTML6DHW2CT332CVAOYEWZQ2OBXGHW3YNY4ZLOE2QE3OFH7LLA\",\"paging_token\":\"GBXD5XDNL5FKTDG7PCGFLKENVM3HZCM32JWN3FKFFVGQISFS2C6Q334E\"}");
    }

    /** A real testnet account with two credit lines, two signers and a data entry. */
    static QByteArray realTestnetAccountWithTrustlines()
    {
        return QByteArray(
            "{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/GBTTX7QVGGB6VTNNAKMXIFR6YL3RYA6UU3YR7FVBLKNOB2SWSA7XMQ7H\"},\"transactions\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/GBTTX7QVGGB6VTNNAKMXIFR6YL3RYA6UU3YR7FVBLKNOB2SWSA7XMQ7H/transactions{?cursor,limit,order}\",\"templated\":true},\"operations\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/GBTTX7QVGGB6VTNNAKMXIFR6YL3RYA6UU3YR7FVBLKNOB2SWSA7XMQ7H/operations{?cursor,limit,order}\",\"templated\":true},\"payments\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/GBTTX7QVGGB6VTNNAKMXIFR6YL3RYA6UU3YR7FVBLKNOB2SWSA7XMQ7H/payments{?cursor,limit,order}\",\"templated\":true},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/GBTTX7QVGGB6VTNNAKMXIFR6YL3RYA6UU3YR7FVBLKNOB2SWSA7XMQ7H/effects{?cursor,limit,order}\",\"templated\":true},\"offers\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/GBTTX7QVGGB6VTNNAKMXIFR6YL3RYA6UU3YR7FVBLKNOB2SWSA7XMQ7H/offers{?cursor,limit,order}\",\"templated\":true},\"trades\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/GBTTX7QVGGB6VTNNAKMXIFR6YL3RYA6UU3YR7FVBLKNOB2SWSA7XMQ7H/trades{?cursor,limit,order}\",\"templated\":true},\"data\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/GBTTX7QVGGB6VTNNAKMXIFR6YL3RYA6UU3YR7FVBLKNOB2SWSA7XMQ7H/data/{key}\",\"templated\":true}},\"id\":\"GBTTX7QVGGB6VTNNAKMXIFR6YL3RYA6UU3YR7FVBLKNOB2SWSA7XMQ7H\",\"account_id\":\"GBTTX7QVGGB6VTNNAKMXIFR6YL3RYA6UU3YR7FVBLKNOB2SWSA7XMQ7H\",\"sequence\":\"21118878180442119\",\"sequence_ledger\":4917148,\"sequence_time\":\"1790609327\",\"subentry_count\":4,\"last_modified_ledger\":4917167,\"last_modified_time\":\"2026-09-28T15:30:22Z\",\"thresholds\":{\"low_threshold\":0,\"med_threshold\":0,\"high_threshold\":0},\"flags\":{\"auth_required\":false,\"auth_revocable\":false,\"auth_immutable\":false,\"auth_clawback_enabled"
            "\":false},\"balances\":[{\"balance\":\"0.0000000\",\"limit\":\"922337203685.4775807\",\"buying_liabilities\":\"0.0000000\",\"selling_liabilities\":\"0.0000000\",\"last_modified_ledger\":4917167,\"is_authorized\":false,\"is_authorized_to_maintain_liabilities\":false,\"is_clawback_enabled\":true,\"asset_type\":\"credit_alphanum4\",\"asset_code\":\"SPN\",\"asset_issuer\":\"GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD\"},{\"balance\":\"98997.6212331\",\"limit\":\"922337203685.4775807\",\"buying_liabilities\":\"0.0000000\",\"selling_liabilities\":\"0.0000000\",\"last_modified_ledger\":4917146,\"is_authorized\":false,\"is_authorized_to_maintain_liabilities\":false,\"is_clawback_enabled\":true,\"asset_type\":\"credit_alphanum4\",\"asset_code\":\"TEST\",\"asset_issuer\":\"GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD\"},{\"balance\":\"9921.3202216\",\"buying_liabilities\":\"0.0000000\",\"selling_liabilities\":\"0.0000000\",\"asset_type\":\"native\"}],\"signers\":[{\"weight\":1,\"key\":\"GBTTX7QVGGB6VTNNAKMXIFR6YL3RYA6UU3YR7FVBLKNOB2SWSA7XMQ7H\",\"type\":\"ed25519_public_key\"},{\"weight\":1,\"key\":\"GDEZMR6VMLXNPBJL4UJHJQXJ3NXVQ3PMC2FTQU6F5QAKEBRJNIAXY3BQ\",\"type\":\"ed25519_public_key\"}],\"data\":{\"sp\":\"c3BvbnNvcmVk\"},\"num_sponsoring\":2,\"num_sponsored\":0,\"paging_token\":\"GBTTX7QVGGB6VTNNAKMXIFR6YL3RYA6UU3YR7FVBLKNOB2SWSA7XMQ7H\"}");
    }

private slots:
};

ADD_TEST(AccountDeserializerTest)
#endif // ACCOUNTDESERIALIZERTEST_H
