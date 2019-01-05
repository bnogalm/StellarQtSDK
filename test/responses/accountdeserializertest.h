#ifndef ACCOUNTDESERIALIZERTEST_H
#define ACCOUNTDESERIALIZERTEST_H
#include <QObject>


#include <QtTest>
#include "../testcollector.h"
#include "../../src/responses/accountresponse.h"
#include "../../src/keypair.h"
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
            "    \"auth_immutable\": true\n"
            "  },\n"
            "  \"balances\": [\n"
            "    {\n"
            "      \"balance\": \"1001.0000000\",\n"
            "      \"limit\": \"12000.4775807\",\n"
            "      \"asset_type\": \"credit_alphanum4\",\n"
            "      \"asset_code\": \"ABC\",\n"
            "      \"asset_issuer\": \"GCRA6COW27CY5MTKIA7POQ2326C5ABYCXODBN4TFF5VL4FMBRHOT3YHU\"\n"
            "    },"
            "    {\n"
            "      \"asset_type\": \"native\",\n"
            "      \"balance\": \"20.0000300\"\n"
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
            "  }"
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
      QCOMPARE(account.getSubentryCount(), 0);
      QCOMPARE(account.getInflationDestination(), QString("GAGRSA6QNQJN2OQYCBNQGMFLO4QLZFNEHIFXOMTQVSUTWVTWT66TOFSC"));
      QCOMPARE(account.getHomeDomain(), QString("stellar.org"));

      QCOMPARE(account.getThresholds().getLowThreshold(), 10);
      QCOMPARE(account.getThresholds().getMedThreshold(), 20);
      QCOMPARE(account.getThresholds().getHighThreshold(), 30);

      QCOMPARE(account.getFlags().getAuthRequired(), false);
      QCOMPARE(account.getFlags().getAuthRevocable(), true);
      QCOMPARE(account.getFlags().getAuthImmutable(), true);

      QCOMPARE(account.getBalances()[0].getAssetType(), QString("credit_alphanum4"));
      QCOMPARE(account.getBalances()[0].getAssetCode(), QString("ABC"));
      QCOMPARE(account.getBalances()[0].getAssetIssuer().getAccountId(), QString("GCRA6COW27CY5MTKIA7POQ2326C5ABYCXODBN4TFF5VL4FMBRHOT3YHU"));
      QCOMPARE(account.getBalances()[0].getBalance(), QString("1001.0000000"));
      QCOMPARE(account.getBalances()[0].getLimit(), QString("12000.4775807"));

      QCOMPARE(account.getBalances()[1].getAssetType(), QString("native"));
      QCOMPARE(account.getBalances()[1].getBalance(), QString("20.0000300"));
      QCOMPARE(account.getBalances()[1].getLimit(), QString());

      QCOMPARE(account.getSigners()[0].getType(), QString("ed25519_public_key"));
      QCOMPARE(account.getSigners()[0].getKey(), QString("GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7"));
      QCOMPARE(account.getSigners()[0].getWeight(), 0);
      QCOMPARE(account.getSigners()[1].getType(), QString("ed25519_public_key"));
      QCOMPARE(account.getSigners()[1].getKey(), QString("GCR2KBCIU6KQXSQY5F5GZYC4WLNHCHCKW4NEGXNEZRYWLTNZIRJJY7D2"));
      QCOMPARE(account.getSigners()[1].getWeight(), 1);


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



};

ADD_TEST(AccountDeserializerTest)
#endif // ACCOUNTDESERIALIZERTEST_H
