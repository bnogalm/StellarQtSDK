#ifndef HORIZON15RESPONSEFIELDSTEST_H
#define HORIZON15RESPONSEFIELDSTEST_H
#include <QObject>
#include <QtTest>
#include "../testcollector.h"

#include "../../src/responses/accountresponse.h"
#include "../../src/responses/ledgerresponse.h"
#include "../../src/responses/operations/operationresponse.h"
#include "../../src/responses/operations/createaccountoperationresponse.h"
#include "../../src/responses/effects/effectresponse.h"
#include "../../src/responses/effects/accountcreditedeffectresponse.h"

/**
 * Smoke tests for the new Horizon-15 fields exposed on existing responses:
 *   - AccountResponse::Balance::liquidity_pool_id
 *   - LedgerResponse::tx_set_operation_count
 *   - OperationResponse::sponsor
 *   - EffectResponse::account_muxed / account_muxed_id
 */
class Horizon15ResponseFieldsTest: public QObject
{
    Q_OBJECT
private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    void testAccountBalanceLiquidityPoolShares()
    {
        QByteArray json = "{"
            "\"id\":\"GA5...\",\"account_id\":\"GA5...\",\"sequence\":\"1\","
            "\"subentry_count\":0,"
            "\"thresholds\":{\"low_threshold\":0,\"med_threshold\":0,\"high_threshold\":0},"
            "\"flags\":{\"auth_required\":false,\"auth_revocable\":false,\"auth_immutable\":false},"
            "\"balances\":[{"
                "\"asset_type\":\"liquidity_pool_shares\","
                "\"liquidity_pool_id\":\"abc1234\","
                "\"balance\":\"100.0000000\","
                "\"limit\":\"100000000000.0000000\","
                "\"is_authorized\":true,"
                "\"is_authorized_to_maintain_liabilities\":true,"
                "\"last_modified_ledger\":12345"
            "}],"
            "\"signers\":[],"
            "\"data\":{},"
            "\"num_sponsoring\":0,"
            "\"num_sponsored\":0,"
            "\"paging_token\":\"GA5...\","
            "\"_links\":{}"
        "}";
        AccountResponse r;
        r.loadFromJson(json);
        QCOMPARE(r.getBalances().size(), 1);
        AccountResponseAttach::Balance b = r.getBalances()[0];
        QCOMPARE(b.getAssetType(), QString("liquidity_pool_shares"));
        QCOMPARE(b.getLiquidityPoolId(), QString("abc1234"));
        QCOMPARE(b.getBalance(), QString("100.0000000"));
    }

    void testLedgerTxSetOperationCount()
    {
        QByteArray json = "{"
            "\"id\":\"1\",\"paging_token\":\"1\",\"hash\":\"h\",\"prev_hash\":\"\","
            "\"sequence\":1,"
            "\"transaction_count\":3,"
            "\"successful_transaction_count\":3,"
            "\"failed_transaction_count\":0,"
            "\"operation_count\":5,"
            "\"tx_set_operation_count\":7,"
            "\"closed_at\":\"2021-11-08T01:46:38Z\","
            "\"total_coins\":\"0\",\"fee_pool\":\"0\","
            "\"base_fee\":100,\"base_reserve\":\"0\","
            "\"base_fee_in_stroops\":100,\"base_reserve_in_stroops\":5000000,"
            "\"max_tx_set_size\":100,\"protocol_version\":19,"
            "\"header_xdr\":\"\""
        "}";
        LedgerResponse r;
        r.loadFromJson(json);
        QCOMPARE(r.getOperationCount(), (qint32)5);
        QCOMPARE(r.getTxSetOperationCount(), (qint32)7);
    }

    void testOperationSponsorField()
    {
        // Use a concrete OperationResponse subclass — the `sponsor` field is
        // declared on the base, so any subclass surfaces it.
        QByteArray json = "{"
            "\"id\":\"100\",\"paging_token\":\"100\","
            "\"transaction_successful\":true,"
            "\"source_account\":\"GA5...\","
            "\"type\":\"create_account\",\"type_i\":0,"
            "\"created_at\":\"2021-11-08T01:46:38Z\","
            "\"transaction_hash\":\"deadbeef\","
            "\"sponsor\":\"GBSPONSOR\","
            "\"starting_balance\":\"100\","
            "\"funder\":\"GA5...\","
            "\"account\":\"GBDEST\""
        "}";
        CreateAccountOperationResponse op;
        op.loadFromJson(json);
        QCOMPARE(op.getSponsor(), QString("GBSPONSOR"));
    }

    void testEffectAccountMuxed()
    {
        QByteArray json = "{"
            "\"id\":\"0001\",\"paging_token\":\"1\","
            "\"account\":\"GA5ZSEJYB37JRC5AVCIA5MOP4RHTM335X2KGX3IHOJAPP5RE34K4KZVN\","
            "\"account_muxed\":\"MA5ZSEJYB37JRC5AVCIA5MOP4RHTM335X2KGX3IHOJAPP5RE34K4KAAAAAAAAAAB6IT4Y\","
            "\"account_muxed_id\":\"123456789\","
            "\"type\":\"account_credited\",\"type_i\":2,"
            "\"created_at\":\"2021-11-08T01:46:38Z\","
            "\"asset_type\":\"native\","
            "\"amount\":\"10.0000000\""
        "}";
        AccountCreditedEffectResponse e;
        e.loadFromJson(json);
        QCOMPARE(e.getAccountMuxed(),
                 QString("MA5ZSEJYB37JRC5AVCIA5MOP4RHTM335X2KGX3IHOJAPP5RE34K4KAAAAAAAAAAB6IT4Y"));
        QCOMPARE(e.getAccountMuxedId(), QString("123456789"));
    }
};

ADD_TEST(Horizon15ResponseFieldsTest)
#endif // HORIZON15RESPONSEFIELDSTEST_H
