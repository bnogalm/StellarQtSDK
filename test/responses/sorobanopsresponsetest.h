#ifndef SOROBANOPSRESPONSETEST_H
#define SOROBANOPSRESPONSETEST_H

#include <QObject>
#include <QtTest>
#include "../testcollector.h"

#include "../../src/responses/operations/invokehostfunctionoperationresponse.h"
#include "../../src/responses/operations/extendfootprintttloperationresponse.h"
#include "../../src/responses/operations/restorefootprintoperationresponse.h"

/** JSON deserialization for the 3 Soroban operation responses (CAP-46). */
class SorobanOpsResponseTest: public QObject
{
    Q_OBJECT
private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    void testInvokeHostFunctionDeserialize()
    {
        QByteArray json = "{"
            "\"id\":\"1\",\"paging_token\":\"1\","
            "\"transaction_successful\":true,"
            "\"source_account\":\"GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG\","
            "\"type\":\"invoke_host_function\",\"type_i\":24,"
            "\"created_at\":\"2026-01-01T00:00:00Z\","
            "\"transaction_hash\":\"deadbeef\","
            "\"function\":\"HostFunctionTypeHostFunctionTypeInvokeContract\","
            "\"address\":\"CDLZFC3SYJYDZT7K67VZ75HPJVIEUVNIXF47ZG2FB2RMQQVU2HHGCYSC\","
            "\"salt\":\"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\","
            "\"parameters\":["
                "{\"type\":\"Address\",\"value\":\"AAAAEgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\"},"
                "{\"type\":\"Sym\",\"value\":\"AAAADwAAAAhfX2luaXQ_AAAA\"}"
            "],"
            "\"asset_balance_changes\":[]"
        "}";

        InvokeHostFunctionOperationResponse r;
        r.loadFromJson(json);
        QCOMPARE(r.getFunction(),
                 QString("HostFunctionTypeHostFunctionTypeInvokeContract"));
        QCOMPARE(r.getAddress(),
                 QString("CDLZFC3SYJYDZT7K67VZ75HPJVIEUVNIXF47ZG2FB2RMQQVU2HHGCYSC"));
        QCOMPARE(r.getParameters().size(), 2);
        QCOMPARE(r.getParameters().at(0).getType(), QString("Address"));
        QCOMPARE(r.getParameters().at(1).getType(), QString("Sym"));
        QCOMPARE(r.getAssetBalanceChanges().size(), 0);
    }

    void testInvokeHostFunctionWithBalanceChanges()
    {
        QByteArray json = "{"
            "\"id\":\"1\",\"paging_token\":\"1\","
            "\"transaction_successful\":true,"
            "\"source_account\":\"GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG\","
            "\"type\":\"invoke_host_function\",\"type_i\":24,"
            "\"created_at\":\"2026-01-01T00:00:00Z\","
            "\"transaction_hash\":\"deadbeef\","
            "\"function\":\"HostFunctionTypeHostFunctionTypeInvokeContract\","
            "\"parameters\":[],"
            "\"asset_balance_changes\":["
                "{\"type\":\"transfer\","
                 "\"from\":\"GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG\","
                 "\"to\":\"GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7\","
                 "\"amount\":\"100.0000000\","
                 "\"asset_type\":\"native\"}"
            "]"
        "}";

        InvokeHostFunctionOperationResponse r;
        r.loadFromJson(json);
        // getAssetBalanceChanges() returns by value — bind to a local so the
        // QList stays alive across QCOMPAREs. Taking a reference into the
        // returned temporary's element dangles after the full expression.
        const auto changes = r.getAssetBalanceChanges();
        QCOMPARE(changes.size(), 1);
        const auto& bc = changes.first();
        QCOMPARE(bc.getType(), QString("transfer"));
        QCOMPARE(bc.getAmount(), QString("100.0000000"));
        QCOMPARE(bc.getAssetType(), QString("native"));
    }

    void testExtendFootprintTtlDeserialize()
    {
        QByteArray json = "{"
            "\"id\":\"1\",\"paging_token\":\"1\","
            "\"transaction_successful\":true,"
            "\"source_account\":\"GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG\","
            "\"type\":\"extend_footprint_ttl\",\"type_i\":25,"
            "\"created_at\":\"2026-01-01T00:00:00Z\","
            "\"transaction_hash\":\"deadbeef\","
            "\"extend_to\":123456"
        "}";

        ExtendFootprintTTLOperationResponse r;
        r.loadFromJson(json);
        QCOMPARE(r.getExtendTo(), quint32(123456));
    }

    void testRestoreFootprintDeserialize()
    {
        QByteArray json = "{"
            "\"id\":\"1\",\"paging_token\":\"1\","
            "\"transaction_successful\":true,"
            "\"source_account\":\"GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG\","
            "\"type\":\"restore_footprint\",\"type_i\":26,"
            "\"created_at\":\"2026-01-01T00:00:00Z\","
            "\"transaction_hash\":\"deadbeef\""
        "}";

        RestoreFootprintOperationResponse r;
        r.loadFromJson(json);
        // getSourceAccount() returns KeyPair&; use the string accessor instead.
        QCOMPARE(r.sourceAccount(),
                 QString("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"));
    }
};

ADD_TEST(SorobanOpsResponseTest)
#endif // SOROBANOPSRESPONSETEST_H
