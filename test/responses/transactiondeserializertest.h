#ifndef TRANSACTIONDESERIALIZERTEST_H
#define TRANSACTIONDESERIALIZERTEST_H

#include <QObject>


#include <QtTest>
#include "../testcollector.h"
#include "../../src/responses/transactionresponse.h"
#include "../../src/memo.h"
#include "../../src/transaction.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "../../src/responses/page.h"

class TransactionDeserializerTest: public QObject
{
    Q_OBJECT

    QByteArray json = "{\n"
                      "  \"fee_account\": \"GCUB7JL4APK7LKJ6MZF7Q2JTLHAGNBIUA7XIXD5SQTG52GQ2DAT6XZMK\",\n"
                      "  \"_links\": {\n"
                      "    \"account\": {\n"
                      "      \"href\": \"/accounts/GCUB7JL4APK7LKJ6MZF7Q2JTLHAGNBIUA7XIXD5SQTG52GQ2DAT6XZMK\"\n"
                      "    },\n"
                      "    \"effects\": {\n"
                      "      \"href\": \"/transactions/5c2e4dad596941ef944d72741c8f8f1a4282f8f2f141e81d827f44bf365d626b/effects{?cursor,limit,order}\",\n"
                      "      \"templated\": true\n"
                      "    },\n"
                      "    \"ledger\": {\n"
                      "      \"href\": \"/ledgers/915744\"\n"
                      "    },\n"
                      "    \"operations\": {\n"
                      "      \"href\": \"/transactions/5c2e4dad596941ef944d72741c8f8f1a4282f8f2f141e81d827f44bf365d626b/operations{?cursor,limit,order}\",\n"
                      "      \"templated\": true\n"
                      "    },\n"
                      "    \"precedes\": {\n"
                      "      \"href\": \"/transactions?cursor=3933090531512320\\u0026order=asc\"\n"
                      "    },\n"
                      "    \"self\": {\n"
                      "      \"href\": \"/transactions/5c2e4dad596941ef944d72741c8f8f1a4282f8f2f141e81d827f44bf365d626b\"\n"
                      "    },\n"
                      "    \"succeeds\": {\n"
                      "      \"href\": \"/transactions?cursor=3933090531512320\\u0026order=desc\"\n"
                      "    }\n"
                      "  },\n"
                      "  \"id\": \"5c2e4dad596941ef944d72741c8f8f1a4282f8f2f141e81d827f44bf365d626b\",\n"
                      "  \"paging_token\": \"3933090531512320\",\n"
                      "  \"successful\": true,\n"
                      "  \"hash\": \"5c2e4dad596941ef944d72741c8f8f1a4282f8f2f141e81d827f44bf365d626b\",\n"
                      "  \"ledger\": 915744,\n"
                      "  \"created_at\": \"2015-11-20T17:01:28Z\",\n"
                      "  \"source_account\": \"GCUB7JL4APK7LKJ6MZF7Q2JTLHAGNBIUA7XIXD5SQTG52GQ2DAT6XZMK\",\n"
                      "  \"source_account_sequence\": 2373051035426646,\n"
                      "  \"max_fee\": 200,\n"
                      "  \"fee_charged\": 100,\n"
                      "  \"operation_count\": 1,\n"
                      "  \"envelope_xdr\": \"AAAAAKgfpXwD1fWpPmZL+GkzWcBmhRQH7ouPsoTN3RoaGCfrAAAAZAAIbkcAAB9WAAAAAAAAAANRBBZE6D1qyGjISUGLY5Ldvp31PwAAAAAAAAAAAAAAAAAAAAEAAAABAAAAAP1qe44j+i4uIT+arbD4QDQBt8ryEeJd7a0jskQ3nwDeAAAAAAAAAADA7RnarSzCwj3OT+M2btCMFpVBdqxJS+Sr00qBjtFv7gAAAABLCs/QAAAAAAAAAAEaGCfrAAAAQG/56Cj2J8W/KCZr+oC4sWND1CTGWfaccHNtuibQH8kZIb+qBSDY94g7hiaAXrlIeg9b7oz/XuP3x9MWYw2jtwM=\",\n"
                      "  \"result_xdr\": \"AAAAAAAAAGQAAAAAAAAAAQAAAAAAAAAAAAAAAAAAAAA=\",\n"
                      "  \"result_meta_xdr\": \"AAAAAAAAAAEAAAACAAAAAAAN+SAAAAAAAAAAAMDtGdqtLMLCPc5P4zZu0IwWlUF2rElL5KvTSoGO0W/uAAAAAEsKz9AADfkgAAAAAAAAAAAAAAAAAAAAAAAAAAABAAAAAAAAAAAAAAAAAAAAAAAAAQAN+SAAAAAAAAAAAP1qe44j+i4uIT+arbD4QDQBt8ryEeJd7a0jskQ3nwDeAAHp6WMr55YACD1BAAAAHgAAAAoAAAAAAAAAAAAAAAABAAAAAAAACgAAAAARC07BokpLTOF+/vVKBwiAlop7hHGJTNeGGlY4MoPykwAAAAEAAAAAK+Lzfd3yDD+Ov0GbYu1g7SaIBrKZeBUxoCunkLuI7aoAAAABAAAAAERmsKL73CyLV/HvjyQCERDXXpWE70Xhyb6MR5qPO3yQAAAAAQAAAABSORGwAdyuanN3sNOHqNSpACyYdkUM3L8VafUu69EvEgAAAAEAAAAAeCzqJNkMM/jLvyuMIfyFHljBlLCtDyj17RMycPuNtRMAAAABAAAAAIEi4R7juq15ymL00DNlAddunyFT4FyUD4muC4t3bobdAAAAAQAAAACaNpLL5YMfjOTdXVEqrAh99LM12sN6He6pHgCRAa1f1QAAAAEAAAAAqB+lfAPV9ak+Zkv4aTNZwGaFFAfui4+yhM3dGhoYJ+sAAAABAAAAAMNJrEvdMg6M+M+n4BDIdzsVSj/ZI9SvAp7mOOsvAD/WAAAAAQAAAADbHA6xiKB1+G79mVqpsHMOleOqKa5mxDpP5KEp/Xdz9wAAAAEAAAAAAAAAAA==\",\n"
                      "  \"memo_type\": \"hash\",\n"
                      "  \"memo\": \"UQQWROg9ashoyElBi2OS3b6d9T8AAAAAAAAAAAAAAAA=\",\n"
                      "  \"signatures\": [\n"
                      "    \"b/noKPYnxb8oJmv6gLixY0PUJMZZ9pxwc226JtAfyRkhv6oFINj3iDuGJoBeuUh6D1vujP9e4/fH0xZjDaO3Aw==\"\n"
                      "  ]\n"
                      "}";

    QByteArray jsonMemoNone = "{\n"
                              "  \"fee_account\": \"GCUB7JL4APK7LKJ6MZF7Q2JTLHAGNBIUA7XIXD5SQTG52GQ2DAT6XZMK\",\n"
                              "  \"_links\": {\n"
                              "    \"account\": {\n"
                              "      \"href\": \"/accounts/GCUB7JL4APK7LKJ6MZF7Q2JTLHAGNBIUA7XIXD5SQTG52GQ2DAT6XZMK\"\n"
                              "    },\n"
                              "    \"effects\": {\n"
                              "      \"href\": \"/transactions/5c2e4dad596941ef944d72741c8f8f1a4282f8f2f141e81d827f44bf365d626b/effects{?cursor,limit,order}\",\n"
                              "      \"templated\": true\n"
                              "    },\n"
                              "    \"ledger\": {\n"
                              "      \"href\": \"/ledgers/915744\"\n"
                              "    },\n"
                              "    \"operations\": {\n"
                              "      \"href\": \"/transactions/5c2e4dad596941ef944d72741c8f8f1a4282f8f2f141e81d827f44bf365d626b/operations{?cursor,limit,order}\",\n"
                              "      \"templated\": true\n"
                              "    },\n"
                              "    \"precedes\": {\n"
                              "      \"href\": \"/transactions?cursor=3933090531512320\\u0026order=asc\"\n"
                              "    },\n"
                              "    \"self\": {\n"
                              "      \"href\": \"/transactions/5c2e4dad596941ef944d72741c8f8f1a4282f8f2f141e81d827f44bf365d626b\"\n"
                              "    },\n"
                              "    \"succeeds\": {\n"
                              "      \"href\": \"/transactions?cursor=3933090531512320\\u0026order=desc\"\n"
                              "    }\n"
                              "  },\n"
                              "  \"id\": \"5c2e4dad596941ef944d72741c8f8f1a4282f8f2f141e81d827f44bf365d626b\",\n"
                              "  \"paging_token\": \"3933090531512320\",\n"
                              "  \"successful\": false,\n"
                              "  \"hash\": \"5c2e4dad596941ef944d72741c8f8f1a4282f8f2f141e81d827f44bf365d626b\",\n"
                              "  \"ledger\": 915744,\n"
                              "  \"created_at\": \"2015-11-20T17:01:28Z\",\n"
                              "  \"source_account\": \"GCUB7JL4APK7LKJ6MZF7Q2JTLHAGNBIUA7XIXD5SQTG52GQ2DAT6XZMK\",\n"
                              "  \"source_account_sequence\": 2373051035426646,\n"
                              "  \"max_fee\": 200,\n"
                              "  \"fee_charged\": 100,\n"
                              "  \"operation_count\": 1,\n"
                              "  \"envelope_xdr\": \"AAAAAKgfpXwD1fWpPmZL+GkzWcBmhRQH7ouPsoTN3RoaGCfrAAAAZAAIbkcAAB9WAAAAAAAAAANRBBZE6D1qyGjISUGLY5Ldvp31PwAAAAAAAAAAAAAAAAAAAAEAAAABAAAAAP1qe44j+i4uIT+arbD4QDQBt8ryEeJd7a0jskQ3nwDeAAAAAAAAAADA7RnarSzCwj3OT+M2btCMFpVBdqxJS+Sr00qBjtFv7gAAAABLCs/QAAAAAAAAAAEaGCfrAAAAQG/56Cj2J8W/KCZr+oC4sWND1CTGWfaccHNtuibQH8kZIb+qBSDY94g7hiaAXrlIeg9b7oz/XuP3x9MWYw2jtwM=\",\n"
                              "  \"result_xdr\": \"AAAAAAAAAGQAAAAAAAAAAQAAAAAAAAAAAAAAAAAAAAA=\",\n"
                              "  \"result_meta_xdr\": \"AAAAAAAAAAEAAAACAAAAAAAN+SAAAAAAAAAAAMDtGdqtLMLCPc5P4zZu0IwWlUF2rElL5KvTSoGO0W/uAAAAAEsKz9AADfkgAAAAAAAAAAAAAAAAAAAAAAAAAAABAAAAAAAAAAAAAAAAAAAAAAAAAQAN+SAAAAAAAAAAAP1qe44j+i4uIT+arbD4QDQBt8ryEeJd7a0jskQ3nwDeAAHp6WMr55YACD1BAAAAHgAAAAoAAAAAAAAAAAAAAAABAAAAAAAACgAAAAARC07BokpLTOF+/vVKBwiAlop7hHGJTNeGGlY4MoPykwAAAAEAAAAAK+Lzfd3yDD+Ov0GbYu1g7SaIBrKZeBUxoCunkLuI7aoAAAABAAAAAERmsKL73CyLV/HvjyQCERDXXpWE70Xhyb6MR5qPO3yQAAAAAQAAAABSORGwAdyuanN3sNOHqNSpACyYdkUM3L8VafUu69EvEgAAAAEAAAAAeCzqJNkMM/jLvyuMIfyFHljBlLCtDyj17RMycPuNtRMAAAABAAAAAIEi4R7juq15ymL00DNlAddunyFT4FyUD4muC4t3bobdAAAAAQAAAACaNpLL5YMfjOTdXVEqrAh99LM12sN6He6pHgCRAa1f1QAAAAEAAAAAqB+lfAPV9ak+Zkv4aTNZwGaFFAfui4+yhM3dGhoYJ+sAAAABAAAAAMNJrEvdMg6M+M+n4BDIdzsVSj/ZI9SvAp7mOOsvAD/WAAAAAQAAAADbHA6xiKB1+G79mVqpsHMOleOqKa5mxDpP5KEp/Xdz9wAAAAEAAAAAAAAAAA==\",\n"
                              "  \"memo_type\": \"none\",\n"
                              "  \"signatures\": [\n"
                              "    \"b/noKPYnxb8oJmv6gLixY0PUJMZZ9pxwc226JtAfyRkhv6oFINj3iDuGJoBeuUh6D1vujP9e4/fH0xZjDaO3Aw==\"\n"
                              "  ]\n"
                              "}";

    const QByteArray jsonFeeBump = "{\n"
          "  \"_links\": {\n"
          "    \"self\": {\n"
          "      \"href\": \"http://localhost/transactions/3dfef7d7226995b504f2827cc63d45ad41e9687bb0a8abcf08ba755fedca0352\"\n"
          "    },\n"
          "    \"account\": {\n"
          "      \"href\": \"http://localhost/accounts/GABQGAYAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAB2MX\"\n"
          "    },\n"
          "    \"ledger\": {\n"
          "      \"href\": \"http://localhost/ledgers/123\"\n"
          "    },\n"
          "    \"operations\": {\n"
          "      \"href\": \"http://localhost/transactions/3dfef7d7226995b504f2827cc63d45ad41e9687bb0a8abcf08ba755fedca0352/operations{?cursor,limit,order}\",\n"
          "      \"templated\": true\n"
          "    },\n"
          "    \"effects\": {\n"
          "      \"href\": \"http://localhost/transactions/3dfef7d7226995b504f2827cc63d45ad41e9687bb0a8abcf08ba755fedca0352/effects{?cursor,limit,order}\",\n"
          "      \"templated\": true\n"
          "    },\n"
          "    \"precedes\": {\n"
          "      \"href\": \"http://localhost/transactions?order=asc\\u0026cursor=528280981504\"\n"
          "    },\n"
          "    \"succeeds\": {\n"
          "      \"href\": \"http://localhost/transactions?order=desc\\u0026cursor=528280981504\"\n"
          "    },\n"
          "    \"transaction\": {\n"
          "      \"href\": \"http://localhost/transactions/3dfef7d7226995b504f2827cc63d45ad41e9687bb0a8abcf08ba755fedca0352\"\n"
          "    }\n"
          "  },\n"
          "  \"id\": \"3dfef7d7226995b504f2827cc63d45ad41e9687bb0a8abcf08ba755fedca0352\",\n"
          "  \"paging_token\": \"528280981504\",\n"
          "  \"successful\": true,\n"
          "  \"hash\": \"3dfef7d7226995b504f2827cc63d45ad41e9687bb0a8abcf08ba755fedca0352\",\n"
          "  \"ledger\": 123,\n"
          "  \"created_at\": \"2020-04-21T10:21:26Z\",\n"
          "  \"source_account\": \"GABQGAYAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAB2MX\",\n"
          "  \"source_account_sequence\": \"97\",\n"
          "  \"fee_account\": \"GABAEAQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABGKJ\",\n"
          "  \"fee_charged\": 123,\n"
          "  \"max_fee\": 776,\n"
          "  \"operation_count\": 1,\n"
          "  \"envelope_xdr\": \"AAAABQAAAAACAgIAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAMIAAAAAgAAAAADAwMAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAGMAAAAAAAAAYQAAAAEAAAAAAAAAAgAAAAAAAAAEAAAAAAAAAAEAAAAAAAAACwAAAAAAAABiAAAAAAAAAAECAgICAAAAAxQUFAAAAAAAAAAAAQMDAwMAAAADHh4eAA==\",\n"
          "  \"result_xdr\": \"AAAAAAAAAHsAAAAB6Yhpu6i84IwQt4QGICEn84iMJUVM03sCYAhiRSdR9SYAAAAAAAAAAAAAAAAAAAABAAAAAAAAAAsAAAAAAAAAAAAAAAA=\",\n"
          "  \"result_meta_xdr\": \"AAAAAQAAAAAAAAAA\",\n"
          "  \"fee_meta_xdr\": \"AAAAAA==\",\n"
          "  \"memo_type\": \"none\",\n"
          "  \"signatures\": [\n"
          "    \"Hh4e\"\n"
          "  ],\n"
          "  \"valid_after\": \"1970-01-01T00:00:02Z\",\n"
          "  \"valid_before\": \"1970-01-01T00:00:04Z\",\n"
          "  \"fee_bump_transaction\": {\n"
          "    \"hash\": \"3dfef7d7226995b504f2827cc63d45ad41e9687bb0a8abcf08ba755fedca0352\",\n"
          "    \"signatures\": [\n"
          "      \"Hh4e\"\n"
          "    ]\n"
          "  },\n"
          "  \"inner_transaction\": {\n"
          "    \"hash\": \"e98869bba8bce08c10b78406202127f3888c25454cd37b02600862452751f526\",\n"
          "    \"signatures\": [\n"
          "      \"FBQU\"\n"
          "    ],\n"
          "    \"max_fee\": \"99\"\n"
          "  }\n"
          "}";
public:


private slots:

    void testDeserialize() {
        TransactionResponse transaction;
        transaction.loadFromJson(json);
        QCOMPARE(transaction.getHash(), QString("5c2e4dad596941ef944d72741c8f8f1a4282f8f2f141e81d827f44bf365d626b"));
        QCOMPARE(transaction.getLedger(), 915744);
        QCOMPARE(transaction.getCreatedAt(), QString("2015-11-20T17:01:28Z"));
        QCOMPARE(transaction.getPagingToken(), QString("3933090531512320"));
        QCOMPARE(transaction.isSuccessful(), Boolean(true));
        QCOMPARE(transaction.getSourceAccount(), QString("GCUB7JL4APK7LKJ6MZF7Q2JTLHAGNBIUA7XIXD5SQTG52GQ2DAT6XZMK"));
        QCOMPARE(transaction.getSourceAccountSequence(), 2373051035426646L);
        QCOMPARE(transaction.getMaxFee(), 200);
        QCOMPARE(transaction.getFeeCharged(), 100);
        QCOMPARE(transaction.getOperationCount(), 1);
        QCOMPARE(transaction.getEnvelopeXdr(), QString("AAAAAKgfpXwD1fWpPmZL+GkzWcBmhRQH7ouPsoTN3RoaGCfrAAAAZAAIbkcAAB9WAAAAAAAAAANRBBZE6D1qyGjISUGLY5Ldvp31PwAAAAAAAAAAAAAAAAAAAAEAAAABAAAAAP1qe44j+i4uIT+arbD4QDQBt8ryEeJd7a0jskQ3nwDeAAAAAAAAAADA7RnarSzCwj3OT+M2btCMFpVBdqxJS+Sr00qBjtFv7gAAAABLCs/QAAAAAAAAAAEaGCfrAAAAQG/56Cj2J8W/KCZr+oC4sWND1CTGWfaccHNtuibQH8kZIb+qBSDY94g7hiaAXrlIeg9b7oz/XuP3x9MWYw2jtwM="));
        QCOMPARE(transaction.getResultXdr(), QString("AAAAAAAAAGQAAAAAAAAAAQAAAAAAAAAAAAAAAAAAAAA="));
        QCOMPARE(transaction.getResultMetaXdr(), QString("AAAAAAAAAAEAAAACAAAAAAAN+SAAAAAAAAAAAMDtGdqtLMLCPc5P4zZu0IwWlUF2rElL5KvTSoGO0W/uAAAAAEsKz9AADfkgAAAAAAAAAAAAAAAAAAAAAAAAAAABAAAAAAAAAAAAAAAAAAAAAAAAAQAN+SAAAAAAAAAAAP1qe44j+i4uIT+arbD4QDQBt8ryEeJd7a0jskQ3nwDeAAHp6WMr55YACD1BAAAAHgAAAAoAAAAAAAAAAAAAAAABAAAAAAAACgAAAAARC07BokpLTOF+/vVKBwiAlop7hHGJTNeGGlY4MoPykwAAAAEAAAAAK+Lzfd3yDD+Ov0GbYu1g7SaIBrKZeBUxoCunkLuI7aoAAAABAAAAAERmsKL73CyLV/HvjyQCERDXXpWE70Xhyb6MR5qPO3yQAAAAAQAAAABSORGwAdyuanN3sNOHqNSpACyYdkUM3L8VafUu69EvEgAAAAEAAAAAeCzqJNkMM/jLvyuMIfyFHljBlLCtDyj17RMycPuNtRMAAAABAAAAAIEi4R7juq15ymL00DNlAddunyFT4FyUD4muC4t3bobdAAAAAQAAAACaNpLL5YMfjOTdXVEqrAh99LM12sN6He6pHgCRAa1f1QAAAAEAAAAAqB+lfAPV9ak+Zkv4aTNZwGaFFAfui4+yhM3dGhoYJ+sAAAABAAAAAMNJrEvdMg6M+M+n4BDIdzsVSj/ZI9SvAp7mOOsvAD/WAAAAAQAAAADbHA6xiKB1+G79mVqpsHMOleOqKa5mxDpP5KEp/Xdz9wAAAAEAAAAAAAAAAA=="));

        QCOMPARE(transaction.getSignatures(), QStringList() << ("b/noKPYnxb8oJmv6gLixY0PUJMZZ9pxwc226JtAfyRkhv6oFINj3iDuGJoBeuUh6D1vujP9e4/fH0xZjDaO3Aw=="));
        QCOMPARE(transaction.getFeeBump().getHash(), QString());
        QCOMPARE(transaction.getInner().getHash(), QString());

        QVERIFY(dynamic_cast<MemoHash*>(transaction.getMemo()));
        MemoHash* memo = (MemoHash*) transaction.getMemo();
        QCOMPARE(memo->getHexValue(), QString("51041644e83d6ac868c849418b6392ddbe9df53f000000000000000000000000"));

        QCOMPARE(transaction.getLinks().getAccount().getHref(), QString("/accounts/GCUB7JL4APK7LKJ6MZF7Q2JTLHAGNBIUA7XIXD5SQTG52GQ2DAT6XZMK"));
        QCOMPARE(transaction.getLinks().getEffects().getHref(), QString("/transactions/5c2e4dad596941ef944d72741c8f8f1a4282f8f2f141e81d827f44bf365d626b/effects{?cursor,limit,order}"));
        QCOMPARE(transaction.getLinks().getLedger().getHref(), QString("/ledgers/915744"));
        QCOMPARE(transaction.getLinks().getOperations().getHref(), QString("/transactions/5c2e4dad596941ef944d72741c8f8f1a4282f8f2f141e81d827f44bf365d626b/operations{?cursor,limit,order}"));
        QCOMPARE(transaction.getLinks().getPrecedes().getHref(), QString("/transactions?cursor=3933090531512320&order=asc"));
        QCOMPARE(transaction.getLinks().getSelf().getHref(), QString("/transactions/5c2e4dad596941ef944d72741c8f8f1a4282f8f2f141e81d827f44bf365d626b"));
        QCOMPARE(transaction.getLinks().getSucceeds().getHref(), QString("/transactions?cursor=3933090531512320&order=desc"));
    }

    void testDeserializeWithoutMemo() {
        TransactionResponse transaction;
        transaction.loadFromJson(jsonMemoNone);

        QVERIFY(dynamic_cast<MemoNone*>(transaction.getMemo()));
        QCOMPARE(transaction.isSuccessful(), Boolean(false));
    }

    void testDeserializeFeeBump()
    {
      TransactionResponse transaction;
      transaction.loadFromJson(jsonFeeBump);
      QCOMPARE(transaction.getHash(), "3dfef7d7226995b504f2827cc63d45ad41e9687bb0a8abcf08ba755fedca0352");
      QCOMPARE(transaction.getLedger(), 123L);
      QCOMPARE(transaction.isSuccessful(), true);
      QCOMPARE(transaction.getSourceAccount(), "GABQGAYAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAB2MX");
      QCOMPARE(transaction.getFeeAccount(), "GABAEAQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABGKJ");
      QCOMPARE(transaction.getSourceAccountSequence(), 97L);
      QCOMPARE(transaction.getMaxFee(), 776L);
      QCOMPARE(transaction.getFeeCharged(), 123L);
      QCOMPARE(transaction.getOperationCount(), 1);
      QCOMPARE(transaction.getSignatures(), QStringList() << QString("Hh4e"));

      TransactionResponseAttach::FeeBumpTransaction feeBumpTransaction = transaction.getFeeBump();
      QCOMPARE(feeBumpTransaction.getHash(), "3dfef7d7226995b504f2827cc63d45ad41e9687bb0a8abcf08ba755fedca0352");
      QCOMPARE(feeBumpTransaction.getSignatures(), QStringList() << QString("Hh4e"));

      TransactionResponseAttach::InnerTransaction innerTransaction = transaction.getInner();
      QCOMPARE(innerTransaction.getHash(), "e98869bba8bce08c10b78406202127f3888c25454cd37b02600862452751f526");
      QCOMPARE(innerTransaction.getMaxFee(), 99L);
      QCOMPARE(innerTransaction.getSignatures(), QStringList() << QString("FBQU"));
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
    /** Record `index` of the real transactions, as JSON. */
    static QByteArray realTransaction(int index)
    {
        return QJsonDocument(QJsonDocument::fromJson(realTransactions()).object()
                                 .value("_embedded").toObject().value("records").toArray()
                                 .at(index).toObject()).toJson();
    }

private slots:
    /**
     * A memo, exactly. getMemo() read an id memo that is not a number as 0,
     * a hash memo that is not the base64 of 32 bytes as other bytes, and a
     * memo type it did not know threw only when asked.
     */
    void testMemoIsReadExactly()
    {
        const QByteArray id = realTransaction(2);
        const QByteArray hash = realTransaction(3);
        const QByteArray text = realTransaction(1);
        QVERIFY(refusalOf<TransactionResponse>(id, "memo", QString("12abc")).startsWith("memo: "));
        QVERIFY(refusalOf<TransactionResponse>(id, "memo", QString("18446744073709551616")).startsWith("memo: "));
        QVERIFY(refusalOf<TransactionResponse>(id, "memo", 12).startsWith("memo: "));
        QVERIFY(refusalOf<TransactionResponse>(id, "memo", QJsonValue(QJsonValue::Undefined)).startsWith("memo: "));
        QVERIFY(refusalOf<TransactionResponse>(hash, "memo", QString("LXEWQrcmsEQBYnyp")).startsWith("memo: "));
        QVERIFY(refusalOf<TransactionResponse>(hash, "memo", QString("not base64 at all, not 32 bytes!")).startsWith("memo: "));
        QVERIFY(refusalOf<TransactionResponse>(text, "memo_type", QString("txt")).startsWith("memo_type: "));
        QVERIFY(refusalOf<TransactionResponse>(text, "memo_type", QJsonValue(QJsonValue::Undefined)).startsWith("memo_type: "));
        QVERIFY(refusalOf<TransactionResponse>(text, "memo_bytes", QString("aGVsbG8gw6k")).startsWith("memo_bytes: "));
        QVERIFY(refusalOf<TransactionResponse>(text, "memo", 5).startsWith("memo: "));
    }

    /**
     * An id memo the response check lets through is one getMemo() builds.
     * "-0" passed the check (a whole number, not below 0), and getMemo()
     * then threw: Memo::parse() reads an id from decimal digits only, as
     * js 17.1.0's Memo.id, which refuses "-0" and "+5" ("Expects a uint64
     * as a string") and keeps "007". py 14.1.0 hands Horizon's memo over
     * as text, unread.
     */
    void testAnIdMemoTheCheckLetsThroughIsOneGetMemoBuilds()
    {
        const QByteArray id = realTransaction(2);
        QVERIFY(refusalOf<TransactionResponse>(id, "memo", QString("-0")).startsWith("memo: "));
        QVERIFY(refusalOf<TransactionResponse>(id, "memo", QString("-00")).startsWith("memo: "));
        QVERIFY(refusalOf<TransactionResponse>(id, "memo", QString("+5")).startsWith("memo: "));
        const QStringList accepted = { QStringLiteral("0"), QStringLiteral("007"),
                                       QStringLiteral("18446744073709551615") };
        for (const QString& text : accepted) {
            QJsonObject tx = QJsonDocument::fromJson(id).object();
            tx["memo"] = text;
            TransactionResponse response;
            response.loadFromJson(QJsonDocument(tx).toJson());
            auto* memo = dynamic_cast<MemoId*>(response.getMemo());
            QVERIFY(memo != nullptr);
            QCOMPARE(memo->getId(), text.toULongLong());
        }
    }

    /**
     * A text memo is its bytes, which need not be UTF-8. Horizon's "memo"
     * holds them as text, where bytes that are not UTF-8 have become U+FFFD,
     * so two different memos read the same; memo_bytes has them exactly, and
     * getMemo() builds the memo from it now.
     */
    void testTextMemoIsReadFromItsBytes()
    {
        QJsonObject tx = QJsonDocument::fromJson(realTransaction(1)).object();
        tx["memo"] = QStringLiteral("hello \uFFFD");
        tx["memo_bytes"] = QString::fromLatin1(QByteArray("hello \xe9").toBase64());
        TransactionResponse response;
        response.loadFromJson(QJsonDocument(tx).toJson());
        auto* memo = dynamic_cast<MemoText*>(response.getMemo());
        QVERIFY(memo != nullptr);
        QCOMPARE(memo->getBytes(), QByteArray("hello \xe9"));
    }

    /**
     * Whether the transaction succeeded, its fees and its source sequence,
     * exactly: "successful" given as text was dropped (isSuccessful() null),
     * and a fee or sequence that is not a whole number read as 0.
     */
    void testTransactionIsReadExactly()
    {
        const QByteArray bump = realTransaction(0);
        QVERIFY(refusalOf<TransactionResponse>(bump, "successful", QString("true")).startsWith("successful: "));
        QVERIFY(refusalOf<TransactionResponse>(bump, "successful", QJsonValue(QJsonValue::Undefined)).startsWith("successful: "));
        QVERIFY(refusalOf<TransactionResponse>(bump, "fee_charged", QString("77035.0")).startsWith("fee_charged: "));
        QVERIFY(refusalOf<TransactionResponse>(bump, "max_fee", QJsonValue(QJsonValue::Undefined)).startsWith("max_fee: "));
        QVERIFY(refusalOf<TransactionResponse>(bump, "source_account_sequence", 19404606408762425.0).startsWith("source_account_sequence: "));
        QVERIFY(refusalOf<TransactionResponse>(bump, "inner_transaction.max_fee", QString("88386 stroops")).startsWith("inner_transaction.max_fee: "));
        QVERIFY(refusalOf<TransactionResponse>(bump, "ledger", QString("next")).startsWith("ledger: "));
        QVERIFY(refusalOf<TransactionResponse>(bump, "signatures", QJsonArray{1, 2}).startsWith("signatures[0]: "));
        QVERIFY(refusalOf<TransactionResponse>(bump, "preconditions.timebounds", QString("0-1790608598")).startsWith("preconditions.timebounds: "));
        QVERIFY(refusalOf<TransactionResponse>(bump, "preconditions.timebounds.max_time", QString("never"))
                    .startsWith("preconditions.timebounds.max_time: "));
        QVERIFY(refusalOf<TransactionResponse>(bump, "preconditions.min_account_sequence", QString("5x"))
                    .startsWith("preconditions.min_account_sequence: "));
        QVERIFY(refusalOf<TransactionResponse>(realTransaction(4), "preconditions.ledgerbounds.max_ledger", -1)
                    .startsWith("preconditions.ledgerbounds.max_ledger: "));
    }

    /**
     * Real transactions load with their exact memos (an id at 2^64 - 1, a
     * hash, a text), fees, sequence, fee bump and ledger bounds.
     */
    void testRealHorizonTransactions()
    {
        Page<TransactionResponse> page;
        page.loadFromJson(realTransactions());
        QCOMPARE(page.size(), 5);
        QCOMPARE(page.get(0).getFeeCharged(), Q_INT64_C(77035));
        QCOMPARE(page.get(0).getSourceAccountSequence(), Q_INT64_C(19404606408762425));
        QVERIFY(!page.get(0).getFeeBump().getHash().isEmpty());
        QVERIFY(page.get(1).getMemo()->toXdr().type == stellar::MemoType::MEMO_TEXT);
        QCOMPARE(dynamic_cast<MemoText*>(page.get(1).getMemo())->getBytes(), QByteArray("hello \xc3\xa9"));
        QCOMPARE(dynamic_cast<MemoId*>(page.get(2).getMemo())->getId(), Q_UINT64_C(18446744073709551615));
        QCOMPARE(dynamic_cast<MemoHash*>(page.get(3).getMemo())->getBytes(),
                 QByteArray::fromBase64("LXEWQrcmsEQBYnyp+6wy9chTD7GQPMTbAiWHF5IaSIE="));
        QCOMPARE(page.get(4).getPreconditions().getLedgerBounds().getMaxLedger(), 4917019u);
        QCOMPARE(page.get(4).getFeeCharged(), Q_INT64_C(1690276));
    }

    /**
     * Every member Horizon always sends in a transaction is required.
     * Missing or null, the envelope and the result read empty, the
     * operation count 0, the time empty, the fee account nobody's and the
     * signatures none; so did an inner transaction's signatures, a fee
     * bump's, and a ledger bound's least ledger.
     */
    void testMembersHorizonAlwaysSendsAreRequired()
    {
        const QByteArray bump = realTransaction(0);
        QVERIFY(refusalOf<TransactionResponse>(bump, "ledger", 4917000).isEmpty());
        for (const char* key : { "hash", "ledger", "created_at", "successful", "source_account",
                                 "source_account_sequence", "fee_account", "fee_charged", "max_fee",
                                 "operation_count", "envelope_xdr", "result_xdr", "signatures", "paging_token",
                                 "memo_type", "_links", "_links.self", "_links.account", "_links.ledger",
                                 "_links.operations", "_links.effects", "_links.precedes", "_links.succeeds",
                                 "_links.self.href", "inner_transaction.hash", "inner_transaction.signatures",
                                 "inner_transaction.max_fee", "fee_bump_transaction.hash",
                                 "fee_bump_transaction.signatures" }) {
            QVERIFY2(refusalOf<TransactionResponse>(bump, key, QJsonValue(QJsonValue::Undefined))
                         .startsWith(QString(key) + ": "), key);
            QVERIFY2(refusalOf<TransactionResponse>(bump, key, QJsonValue()).startsWith(QString(key) + ": "), key);
        }
        const QByteArray bounded = realTransaction(4);
        QVERIFY(refusalOf<TransactionResponse>(bounded, "preconditions.ledgerbounds.min_ledger",
                                               QJsonValue(QJsonValue::Undefined))
                    .startsWith("preconditions.ledgerbounds.min_ledger: "));
        // What Horizon leaves out when there is nothing to say stays optional.
        for (const char* key : { "result_meta_xdr", "memo", "memo_bytes", "account_muxed", "fee_account_muxed",
                                 "preconditions", "inner_transaction", "fee_bump_transaction" })
            QVERIFY2(refusalOf<TransactionResponse>(bump, key, QJsonValue(QJsonValue::Undefined)).isEmpty(), key);
    }
private:
    /** Real transactions: a fee bump, text / id / hash memos, ledger bounds, a failed one. */
    static QByteArray realTransactions()
    {
        return QByteArray(
            "{\"_embedded\":{\"records\":[{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/6149592948e451b9d3c3da05df3688d423bffdec6f3256eb6fa1c8e7992d184f\"},\"account\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/GABIPZE6RV7MWDLX6FSXOQF2AIKQU7YBGGV77XSH6JNLYFZAYZCEYFHF\"},\"ledger\":{\"href\":\"https://horizon-testnet.stellar.org/ledgers/4917000\"},\"operations\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/6149592948e451b9d3c3da05df3688d423bffdec6f3256eb6fa1c8e7992d184f/operations{?cursor,limit,order}\",\"templated\":true},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/6149592948e451b9d3c3da05df3688d423bffdec6f3256eb6fa1c8e7992d184f/effects{?cursor,limit,order}\",\"templated\":true},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/transactions?order=asc&cursor=21118354194493440\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/transactions?order=desc&cursor=21118354194493440\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/6149592948e451b9d3c3da05df3688d423bffdec6f3256eb6fa1c8e7992d184f\"}},\"id\":\"6149592948e451b9d3c3da05df3688d423bffdec6f3256eb6fa1c8e7992d184f\",\"paging_token\":\"21118354194493440\",\"successful\":true,\"hash\":\"6149592948e451b9d3c3da05df3688d423bffdec6f3256eb6fa1c8e7992d184f\",\"ledger\":4917000,\"created_at\":\"2026-09-28T15:16:27Z\",\"source_account\":\"GABIPZE6RV7MWDLX6FSXOQF2AIKQU7YBGGV77XSH6JNLYFZAYZCEYFHF\",\"source_account_sequence\":\"19404606408762425\",\"fee_account\":\"GDLWWFBT2UOC7DJ3ZDIGP5GUHX42KDDQFAT3LBPQSU2G5NOB2AISDYEM\",\"fee_charged\":\"77035\",\"max_fee\":\"88386\",\"operation_count\":1,\"envelope_xdr\":\"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\",\"result_xdr\":\"AAAAAAABLOsAAAABiFen5l6jwlns9rePT+u8o7tokijtqkQ+MQktFIkMhdcAAAAAAAAAAAAAAAAAAAABAAAAAAAAABgAAAAAdkytK+CIUO1zLWF1DFlpVijUmEUSTP3fGDeBRfXWHjwAAAAAAAAAAA==\",\"fee_meta_xdr\":\"AAAAAgAAAAMASwcBAAAAAAAAAADXaxQz1RwvjTvI0Gf01D35pQxwKCe1hfCVNG61wdARIQAAABXOxRaGAETwWQAAAAEAAAAAAAAAAAAAAAAAAAAAAQAAAAAAAAAAAAABAAAAAAAAAAAAAAAAAAAAAAAAAAIAAAAAAAAAAAAAAAAAAAADAAAAAABE9qsAAAAAapwy+gAAAAAAAAABAEsHCAAAAAAAAAAA12sUM9UcL407yNBn9NQ9+aUMcCgntYXwlTRutcHQESEAAAAVzsO9RwBE8FkAAAABAAAAAAAAAAAAAAAAAAAAAAEAAAAAAAAAAAAAAQAAAAAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAAAAAAAAAAAAAwAAAAAARParAAAAAGqcMvoAAAAA\",\"memo_type\":\"none\",\"signatures\":[\"x+84OEaOalPgw4xU8WEWSrfOnAePfpGG451x98OqxVYzgpz3xSvmvda8rOZHyx6P9ErLGwc3hnjKuhk0Aj/4AA==\"],\"preconditions\":{\"timebounds\":{\"min_time\":\"0\",\"max_time\":\"1790608598\"}},\"fee_bump_transaction\":{\"hash\":\"6149592948e451b9d3c3da05df3688d423bffdec6f3256eb6fa1c8e7992d184f\",\"signatures\":[\"x+84OEaOalPgw4xU8WEWSrfOnAePfpGG451x98OqxVYzgpz3xSvmvda8rOZHyx6P9ErLGwc3hnjKuhk0Aj/4AA==\"]},\"inner_transaction\":{\"hash\":\"8857a7e65ea3c259ecf6b78f4febbca3bb689228edaa443e31092d14890c85d7\",\"signatures\":[\"Jrqkyf43c/mRgIrHoF9W5ZJmZekqIkX1pQZ+Tfg4tZw98qoW10RRHbYqXA9uA2iqM7fKC6hmfz2ZUaeZZhuvAA==\"],\"max_fee\":\"88283\"}},{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/a9f8eb8174b8da951dc5c4539d658f4bca35a3cdce762236ae47585d14d631a1\"},\"account\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD\"},\"ledger\":{\"href\":\"https://horizon-testnet.ste"
            "llar.org/ledgers/4917126\"},\"operations\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/a9f8eb8174b8da951dc5c4539d658f4bca35a3cdce762236ae47585d14d631a1/operations{?cursor,limit,order}\",\"templated\":true},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/a9f8eb8174b8da951dc5c4539d658f4bca35a3cdce762236ae47585d14d631a1/effects{?cursor,limit,order}\",\"templated\":true},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/transactions?order=asc&cursor=21118895360315392\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/transactions?order=desc&cursor=21118895360315392\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/a9f8eb8174b8da951dc5c4539d658f4bca35a3cdce762236ae47585d14d631a1\"}},\"memo\":\"hello \\u00e9\",\"memo_bytes\":\"aGVsbG8gw6k=\",\"id\":\"a9f8eb8174b8da951dc5c4539d658f4bca35a3cdce762236ae47585d14d631a1\",\"paging_token\":\"21118895360315392\",\"successful\":true,\"hash\":\"a9f8eb8174b8da951dc5c4539d658f4bca35a3cdce762236ae47585d14d631a1\",\"ledger\":4917126,\"created_at\":\"2026-09-28T15:26:57Z\",\"source_account\":\"GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD\",\"source_account_sequence\":\"21118873885474817\",\"fee_account\":\"GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD\",\"fee_charged\":\"500\",\"max_fee\":\"5000\",\"operation_count\":5,\"envelope_xdr\":\"AAAAAgAAAACmbZ4QTV66B5LVPNH+/gfyMVA+ehY2u94DrTv3gpBeOAAAE4gASweBAAAAAQAAAAEAAAAAAAAAAAAAAABquohtAAAAAQAAAAhoZWxsbyDDqQAAAAUAAAAAAAAABQAAAAEAAAAAgLVeM2L8M9tCnvehVA7BLZhpwbmPbeG45lbiagTbin8AAAAAAAAAAQAAAAsAAAAAAAAAAQAAAAEAAAABAAAAAQAAAAEAAAABAAAAAQAAAAtleGFtcGxlLmNvbQAAAAAAAAAAAAAAAAUAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEAAAAAgLVeM2L8M9tCnvehVA7BLZhpwbmPbeG45lbiagTbin8AAAABAAAAAAAAAAoAAAAUY29uZmlnLm1lbW9fcmVxdWlyZWQAAAABAAAAATEAAAAAAAAAAAAACgAAAAJrMgAAAAAAAQAAAAF2AAAAAAAAAAAAAAsAA"
            "AAAAAAAAAAAAAAAAAABgpBeOAAAAECZ3wn4ww9x/DksLDwdhLRCxzEIHCy5y/fqK10ih/vLFUjEUTaVCD2/8g2fsKs1z3KgLHs6NvZjLxNnSx0BQYIP\",\"result_xdr\":\"AAAAAAAAAfQAAAAAAAAABQAAAAAAAAAFAAAAAAAAAAAAAAAFAAAAAAAAAAAAAAAKAAAAAAAAAAAAAAAKAAAAAAAAAAAAAAALAAAAAAAAAAA=\",\"fee_meta_xdr\":\"AAAAAgAAAAMASweBAAAAAAAAAACmbZ4QTV66B5LVPNH+/gfyMVA+ehY2u94DrTv3gpBeOAAAABdIdugAAEsHgQAAAAAAAAAAAAAAAAAAAAAAAAAAAQAAAAAAAAAAAAAAAAAAAAAAAAEASweGAAAAAAAAAACmbZ4QTV66B5LVPNH+/gfyMVA+ehY2u94DrTv3gpBeOAAAABdIduYMAEsHgQAAAAAAAAAAAAAAAAAAAAAAAAAAAQAAAAAAAAAAAAAAAAAAAA==\",\"memo_type\":\"text\",\"signatures\":[\"md8J+MMPcfw5LCw8HYS0QscxCBwsucv36itdIof7yxVIxFE2lQg9v/INn7CrNc9yoCx7Ojb2Yy8TZ0sdAUGCDw==\"],\"preconditions\":{\"timebounds\":{\"min_time\":\"0\",\"max_time\":\"1790609517\"}}},{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/2c3826b9d5885b5d8051e086d58d6f6437f4df972e869bc93c1135d9e1a6d6b7\"},\"account\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD\"},\"ledger\":{\"href\":\"https://horizon-testnet.stellar.org/ledgers/4917127\"},\"operations\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/2c3826b9d5885b5d8051e086d58d6f6437f4df972e869bc93c1135d9e1a6d6b7/operations{?cursor,limit,order}\",\"templated\":true},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/2c3826b9d5885b5d8051e086d58d6f6437f4df972e869bc93c1135d9e1a6d6b7/effects{?cursor,limit,order}\",\"templated\":true},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/transactions?order=asc&cursor=21118899655299072\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/transactions?order=desc&cursor=21118899655299072\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/2c3826b9d5885b5d8051e086d58d6f6437f4df972e869bc93c1135d9e1a6d6b7\"}},\"memo\":\"18446744073709551615\",\"i"
            "d\":\"2c3826b9d5885b5d8051e086d58d6f6437f4df972e869bc93c1135d9e1a6d6b7\",\"paging_token\":\"21118899655299072\",\"successful\":true,\"hash\":\"2c3826b9d5885b5d8051e086d58d6f6437f4df972e869bc93c1135d9e1a6d6b7\",\"ledger\":4917127,\"created_at\":\"2026-09-28T15:27:02Z\",\"source_account\":\"GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD\",\"source_account_sequence\":\"21118873885474818\",\"fee_account\":\"GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD\",\"fee_charged\":\"300\",\"max_fee\":\"3000\",\"operation_count\":3,\"envelope_xdr\":\"AAAAAgAAAACmbZ4QTV66B5LVPNH+/gfyMVA+ehY2u94DrTv3gpBeOAAAC7gASweBAAAAAgAAAAEAAAAAAAAAAAAAAABquohwAAAAAv//////////AAAAAwAAAAAAAAAKAAAAAmsyAAAAAAABAAAAAXcAAAAAAAAAAAAABQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAQAAAACAtV4zYvwz20Ke96FUDsEtmGnBuY9t4bjmVuJqBNuKfwAAAAIAAAAAAAAACwBLB4EAAABlAAAAAAAAAAGCkF44AAAAQHujnI+PTiDl2bd68PQ0wYdy+boshhGlXd4DQRmIvPxJAXrB7JsiI2GGox+6aU9ddJmFyd7201+eoeHwP0+5VAU=\",\"result_xdr\":\"AAAAAAAAASwAAAAAAAAAAwAAAAAAAAAKAAAAAAAAAAAAAAAFAAAAAAAAAAAAAAALAAAAAAAAAAA=\",\"fee_meta_xdr\":\"AAAAAgAAAAMASweGAAAAAAAAAACmbZ4QTV66B5LVPNH+/gfyMVA+ehY2u94DrTv3gpBeOAAAABdIduYMAEsHgQAAAAEAAAADAAAAAQAAAACAtV4zYvwz20Ke96FUDsEtmGnBuY9t4bjmVuJqBNuKfwAAAAsAAAALZXhhbXBsZS5jb20AAQEBAQAAAAEAAAAAgLVeM2L8M9tCnvehVA7BLZhpwbmPbeG45lbiagTbin8AAAABAAAAAQAAAAAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAAAAAAABAAAAAAAAAAMAAAAAAEsHhgAAAABquodBAAAAAAAAAAEASweHAAAAAAAAAACmbZ4QTV66B5LVPNH+/gfyMVA+ehY2u94DrTv3gpBeOAAAABdIduTgAEsHgQAAAAEAAAADAAAAAQAAAACAtV4zYvwz20Ke96FUDsEtmGnBuY9t4bjmVuJqBNuKfwAAAAsAAAALZXhhbXBsZS5jb20AAQEBAQAAAAEAAAAAgLVeM2L8M9tCnvehVA7BLZhpwbmPbeG45lbiagTbin8AAAABAAAAAQAAAAAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAAAAAAABAAAAAAAAAAMAAAAAAEsHhgAAAABquodBAAAAAA==\",\"memo_type\":\"id\",\"signatures\":[\"e6Ocj49OIOXZt3rw9DTBh3L5uiyGEaVd3gNBGYi8/EkBesHsmyIjYYajH7ppT110mYXJ3vbTX56h4fA/T7lUBQ==\"],\"preconditions\":{\"timebounds\":{\"min_t"
            "ime\":\"0\",\"max_time\":\"1790609520\"}}},{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/aa4f7470f69fe579a5408ad878f8c3eabf301934ba4bf21a1e759b57bc8572f5\"},\"account\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD\"},\"ledger\":{\"href\":\"https://horizon-testnet.stellar.org/ledgers/4917128\"},\"operations\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/aa4f7470f69fe579a5408ad878f8c3eabf301934ba4bf21a1e759b57bc8572f5/operations{?cursor,limit,order}\",\"templated\":true},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/aa4f7470f69fe579a5408ad878f8c3eabf301934ba4bf21a1e759b57bc8572f5/effects{?cursor,limit,order}\",\"templated\":true},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/transactions?order=asc&cursor=21118903950249984\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/transactions?order=desc&cursor=21118903950249984\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/aa4f7470f69fe579a5408ad878f8c3eabf301934ba4bf21a1e759b57bc8572f5\"}},\"memo\":\"LXEWQrcmsEQBYnyp+6wy9chTD7GQPMTbAiWHF5IaSIE=\",\"id\":\"aa4f7470f69fe579a5408ad878f8c3eabf301934ba4bf21a1e759b57bc8572f5\",\"paging_token\":\"21118903950249984\",\"successful\":true,\"hash\":\"aa4f7470f69fe579a5408ad878f8c3eabf301934ba4bf21a1e759b57bc8572f5\",\"ledger\":4917128,\"created_at\":\"2026-09-28T15:27:07Z\",\"source_account\":\"GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD\",\"source_account_sequence\":\"21118873885474918\",\"fee_account\":\"GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD\",\"fee_charged\":\"200\",\"max_fee\":\"2000\",\"operation_count\":2,\"envelope_xdr\":\"AAAAAgAAAACmbZ4QTV66B5LVPNH+/gfyMVA+ehY2u94DrTv3gpBeOAAAB9AASweBAAAAZgAAAAEAAAAAAAAAAAAAAABquoh1AAAAAy1xFkK3JrBEAWJ8qfusM"
            "vXIUw+xkDzE2wIlhxeSGkiBAAAAAgAAAAAAAAAKAAAAAmsyAAAAAAAAAAAAAAAAAAUAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEAAAAAgLVeM2L8M9tCnvehVA7BLZhpwbmPbeG45lbiagTbin8AAAAAAAAAAAAAAAGCkF44AAAAQGLshlQSfeRfKKxoFB5HSRD23ktjjNqQ24VkGUkw+pWxtg+4c/nStsdGOrSY20MYTx9aLqcA5Xj95sUPxvGnSgo=\",\"result_xdr\":\"AAAAAAAAAMgAAAAAAAAAAgAAAAAAAAAKAAAAAAAAAAAAAAAFAAAAAAAAAAA=\",\"fee_meta_xdr\":\"AAAAAgAAAAMASweHAAAAAAAAAACmbZ4QTV66B5LVPNH+/gfyMVA+ehY2u94DrTv3gpBeOAAAABdIduTgAEsHgQAAAGUAAAADAAAAAQAAAACAtV4zYvwz20Ke96FUDsEtmGnBuY9t4bjmVuJqBNuKfwAAAAsAAAALZXhhbXBsZS5jb20AAQEBAQAAAAEAAAAAgLVeM2L8M9tCnvehVA7BLZhpwbmPbeG45lbiagTbin8AAAACAAAAAQAAAAAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAAAAAAABAAAAAAAAAAMAAAAAAEsHhwAAAABquodGAAAAAAAAAAEASweIAAAAAAAAAACmbZ4QTV66B5LVPNH+/gfyMVA+ehY2u94DrTv3gpBeOAAAABdIduQYAEsHgQAAAGUAAAADAAAAAQAAAACAtV4zYvwz20Ke96FUDsEtmGnBuY9t4bjmVuJqBNuKfwAAAAsAAAALZXhhbXBsZS5jb20AAQEBAQAAAAEAAAAAgLVeM2L8M9tCnvehVA7BLZhpwbmPbeG45lbiagTbin8AAAACAAAAAQAAAAAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAAAAAAABAAAAAAAAAAMAAAAAAEsHhwAAAABquodGAAAAAA==\",\"memo_type\":\"hash\",\"signatures\":[\"YuyGVBJ95F8orGgUHkdJEPbeS2OM2pDbhWQZSTD6lbG2D7hz+dK2x0Y6tJjbQxhPH1oupwDleP3mxQ/G8adKCg==\"],\"preconditions\":{\"timebounds\":{\"min_time\":\"0\",\"max_time\":\"1790609525\"}}},{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/08fc20e112e2b14f40db12ac9e377859b425a054f19a7292cc692c27aaf5a4e5\"},\"account\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/GCCPXPELRJ2X4LJRTZCYEABPW7FGUQDBGPVZTZKSXS374Z5H2AJOTCJK\"},\"ledger\":{\"href\":\"https://horizon-testnet.stellar.org/ledgers/4917000\"},\"operations\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/08fc20e112e2b14f40db12ac9e377859b425a054f19a7292cc692c27aaf5a4e5/operations{?cursor,limit,order}\",\"templated\":true},\"effects\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/08fc20e112e2b14f40db12ac9e37"
            "7859b425a054f19a7292cc692c27aaf5a4e5/effects{?cursor,limit,order}\",\"templated\":true},\"precedes\":{\"href\":\"https://horizon-testnet.stellar.org/transactions?order=asc&cursor=21118354194501632\"},\"succeeds\":{\"href\":\"https://horizon-testnet.stellar.org/transactions?order=desc&cursor=21118354194501632\"},\"transaction\":{\"href\":\"https://horizon-testnet.stellar.org/transactions/08fc20e112e2b14f40db12ac9e377859b425a054f19a7292cc692c27aaf5a4e5\"}},\"id\":\"08fc20e112e2b14f40db12ac9e377859b425a054f19a7292cc692c27aaf5a4e5\",\"paging_token\":\"21118354194501632\",\"successful\":true,\"hash\":\"08fc20e112e2b14f40db12ac9e377859b425a054f19a7292cc692c27aaf5a4e5\",\"ledger\":4917000,\"created_at\":\"2026-09-28T15:16:27Z\",\"source_account\":\"GCCPXPELRJ2X4LJRTZCYEABPW7FGUQDBGPVZTZKSXS374Z5H2AJOTCJK\",\"source_account_sequence\":\"20218081804551533\",\"fee_account\":\"GCCPXPELRJ2X4LJRTZCYEABPW7FGUQDBGPVZTZKSXS374Z5H2AJOTCJK\",\"fee_charged\":\"1690276\",\"max_fee\":\"1963299\",\"operation_count\":1,\"envelope_xdr\":\"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\",\"result_xdr\":\"AAAAAAAZyqQAAAAAAAAAAQAAAAAAAAAYAAAAAIOepz7sH5LhkrAjLuvWN9vbQU+28P6DtWvGolUDwUl+AAAAAA==\",\"fee_meta_xdr\":\"AAAAAgAAAAMASwb+AAAAAAAAAACE+7yLinV+LTGeRYIAL7fKakBhM+uZ5VK8t/5np9AS6QAAABcfPL6eAEfUPQAABWwAAAAAAAAAAAAAAAAAAAAAAQAAAAAAAAAAAAABAAAAAAAAAAAAAAAAAAAAAAAAAAIAAAAAAAAAAAAAAAAAAAADAAAAAABLBv4AAAAAarqEmQAAAAAAAAABAEsHCAAAAAAAAAAAhPu8i4p1fi0xnkWCAC+3ympAYTPrmeVSvLf+Z6fQEukAAAAXHx7JewBH1D0AAAVsAAAAAAAAAAAAAAAAAAAAAAEAAAAAAAAAAAAAAQAAAAAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAAAAAAAAAAAAAwAAAAAASwb+AAAAAGq6hJkAAAAA\",\"memo_type\":\"none\",\"signatures\":[\"3/7yEBhUv23g0D3Zf161JdzIPF4BDNCOZMNc3PRgBxKgmpp+EePOAEGywpcTudFcZWnSrKpvIRpF6faV744xDA==\"],\"preconditions\":{\"timebounds\":{\"min_time\":\"0\",\"max_time\":\"1790608883\"},\"ledgerbounds\":{\"min_ledger\":0,\"max_ledger\":4917019}}}]},\"_links\":{\"self\":{\"href\":\"https://horizon.example/x?cursor=&limit=2&order=asc\"},\"nex"
            "t\":{\"href\":\"https://horizon.example/x?cursor=2&limit=2&order=asc\"},\"prev\":{\"href\":\"https://horizon.example/x?cursor=1&limit=2&order=desc\"}}}");
    }

private slots:
};


ADD_TEST(TransactionDeserializerTest)


#endif // TRANSACTIONDESERIALIZERTEST_H
