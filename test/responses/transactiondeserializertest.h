#ifndef TRANSACTIONDESERIALIZERTEST_H
#define TRANSACTIONDESERIALIZERTEST_H

#include <QObject>


#include <QtTest>
#include "../testcollector.h"
#include "../../src/responses/transactionresponse.h"
#include "../../src/memo.h"
#include "../../src/transaction.h"

class TransactionDeserializerTest: public QObject
{
    Q_OBJECT

    QByteArray json = "{\n"
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


};


ADD_TEST(TransactionDeserializerTest)


#endif // TRANSACTIONDESERIALIZERTEST_H
