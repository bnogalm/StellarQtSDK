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
        QCOMPARE(transaction.getSourceAccount()->getAccountId(), QString("GCUB7JL4APK7LKJ6MZF7Q2JTLHAGNBIUA7XIXD5SQTG52GQ2DAT6XZMK"));
        QCOMPARE(transaction.getSourceAccountSequence(), 2373051035426646L);
        QCOMPARE(transaction.getMaxFee(), 200);
        QCOMPARE(transaction.getFeeCharged(), 100);
        QCOMPARE(transaction.getOperationCount(), 1);
        QCOMPARE(transaction.getEnvelopeXdr(), QString("AAAAAKgfpXwD1fWpPmZL+GkzWcBmhRQH7ouPsoTN3RoaGCfrAAAAZAAIbkcAAB9WAAAAAAAAAANRBBZE6D1qyGjISUGLY5Ldvp31PwAAAAAAAAAAAAAAAAAAAAEAAAABAAAAAP1qe44j+i4uIT+arbD4QDQBt8ryEeJd7a0jskQ3nwDeAAAAAAAAAADA7RnarSzCwj3OT+M2btCMFpVBdqxJS+Sr00qBjtFv7gAAAABLCs/QAAAAAAAAAAEaGCfrAAAAQG/56Cj2J8W/KCZr+oC4sWND1CTGWfaccHNtuibQH8kZIb+qBSDY94g7hiaAXrlIeg9b7oz/XuP3x9MWYw2jtwM="));
        QCOMPARE(transaction.getResultXdr(), QString("AAAAAAAAAGQAAAAAAAAAAQAAAAAAAAAAAAAAAAAAAAA="));
        QCOMPARE(transaction.getResultMetaXdr(), QString("AAAAAAAAAAEAAAACAAAAAAAN+SAAAAAAAAAAAMDtGdqtLMLCPc5P4zZu0IwWlUF2rElL5KvTSoGO0W/uAAAAAEsKz9AADfkgAAAAAAAAAAAAAAAAAAAAAAAAAAABAAAAAAAAAAAAAAAAAAAAAAAAAQAN+SAAAAAAAAAAAP1qe44j+i4uIT+arbD4QDQBt8ryEeJd7a0jskQ3nwDeAAHp6WMr55YACD1BAAAAHgAAAAoAAAAAAAAAAAAAAAABAAAAAAAACgAAAAARC07BokpLTOF+/vVKBwiAlop7hHGJTNeGGlY4MoPykwAAAAEAAAAAK+Lzfd3yDD+Ov0GbYu1g7SaIBrKZeBUxoCunkLuI7aoAAAABAAAAAERmsKL73CyLV/HvjyQCERDXXpWE70Xhyb6MR5qPO3yQAAAAAQAAAABSORGwAdyuanN3sNOHqNSpACyYdkUM3L8VafUu69EvEgAAAAEAAAAAeCzqJNkMM/jLvyuMIfyFHljBlLCtDyj17RMycPuNtRMAAAABAAAAAIEi4R7juq15ymL00DNlAddunyFT4FyUD4muC4t3bobdAAAAAQAAAACaNpLL5YMfjOTdXVEqrAh99LM12sN6He6pHgCRAa1f1QAAAAEAAAAAqB+lfAPV9ak+Zkv4aTNZwGaFFAfui4+yhM3dGhoYJ+sAAAABAAAAAMNJrEvdMg6M+M+n4BDIdzsVSj/ZI9SvAp7mOOsvAD/WAAAAAQAAAADbHA6xiKB1+G79mVqpsHMOleOqKa5mxDpP5KEp/Xdz9wAAAAEAAAAAAAAAAA=="));

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

};


ADD_TEST(TransactionDeserializerTest)


#endif // TRANSACTIONDESERIALIZERTEST_H
