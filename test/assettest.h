#ifndef ASSETTEST_H
#define ASSETTEST_H
#include <QObject>
#include "src/keypair.h"
#include "src/util.h"

#include <QtTest>
#include "testcollector.h"
#include "../src/asset.h"
#include "../src/assettypenative.h"
#include "../src/assettypecreditalphanum.h"
#include "../src/keypair.h"
#include "../src/assettypecreditalphanum4.h"
#include "../src/assettypecreditalphanum12.h"
#include "../src/responses/page.h"
#include "../src/responses/assetresponse.h"
#include "../src/responses/traderesponse.h"
#include <QScopedPointer>
#include <stdexcept>

class AssetTest: public QObject
{
    Q_OBJECT

private slots:

    void initTestCase()
    {
    }
    void cleanupTestCase()
    {
    }

    void testAssetTypeNative() {
        AssetTypeNative asset;
        auto xdr = asset.toXdr();
        Asset* parsedAsset = Asset::fromXdr(xdr);
        QVERIFY(dynamic_cast<AssetTypeNative*>(parsedAsset));
        QCOMPARE(asset.toString(),QString("native"));
    }

    void testAssetTypeCreditAlphaNum4() {
        QString code = "USDA";
        KeyPair* issuer = KeyPair::random();
        AssetTypeCreditAlphaNum4 asset(code, issuer);
        auto xdr = asset.toXdr();
        AssetTypeCreditAlphaNum4* parsedAsset = (AssetTypeCreditAlphaNum4*) Asset::fromXdr(xdr);
        QCOMPARE(code, asset.getCode());
        QCOMPARE(issuer->getAccountId(), parsedAsset->getIssuer().getAccountId());
        QCOMPARE(asset.toString(),QString("USDA:%1").arg(issuer->getAccountId()));
    }

    void testAssetTypeCreditAlphaNum12() {
        QString code = "TESTTEST";
        KeyPair* issuer = KeyPair::random();
        AssetTypeCreditAlphaNum12 asset(code, issuer);
        auto xdr = asset.toXdr();
        AssetTypeCreditAlphaNum12* parsedAsset = (AssetTypeCreditAlphaNum12*) Asset::fromXdr(xdr);
        QCOMPARE(code, asset.getCode());
        QCOMPARE(issuer->getAccountId(), parsedAsset->getIssuer().getAccountId());
        QCOMPARE(asset.toString(),QString("TESTTEST:%1").arg(issuer->getAccountId()));
    }


    /**
     * Only the length of a code was checked: "US D", "US$" or "\u00e9" made an
     * asset, and a payment of it was signed (core refuses it as malformed; js
     * "Asset code is invalid", py AssetCodeInvalidError). Every way to make a
     * credit asset checks core's rule now, and the refusal says why (it was a
     * bare std::exception, "Unknown exception").
     */
    void testAssetCodesAreLettersAndDigits()
    {
        const QString issuer = QStringLiteral("GAJZR5RMNUNEK7CRXJVEWXZ5XUXWT7FJGILCDDOITF7EC26RPWJ4UVOE");
        auto refused = [&](const QString& code) {
            int refusals = 0;
            try { delete Asset::createNonNativeAsset(code, issuer); } catch (const AssetCodeLengthInvalidException&) { ++refusals; }
            try { delete Asset::create(code + QLatin1Char(':') + issuer); } catch (const std::exception&) { ++refusals; }
            try { delete new AssetTypeCreditAlphaNum4(code, KeyPair::fromAccountId(issuer)); } catch (const std::exception&) { ++refusals; }
            try { delete new AssetTypeCreditAlphaNum12(code, KeyPair::fromAccountId(issuer)); } catch (const std::exception&) { ++refusals; }
            return refusals;
        };
        for (const QString& bad : {QStringLiteral("US D"), QStringLiteral("US$"), QString::fromUtf8("\xc3\xa9"),
                                   QStringLiteral("AB-C"), QStringLiteral("LONG ASSET"), QStringLiteral("A.B"),
                                   QStringLiteral("AB") + QChar(0) + QStringLiteral("C")}) {
            QCOMPARE(refused(bad), 4);
        }
        // Valid codes: each factory makes the kind the length says; the class
        // of the other length refuses.
        QCOMPARE(refused(QStringLiteral("USD")), 1);
        QCOMPARE(refused(QStringLiteral("A1b2")), 1);
        QCOMPARE(refused(QStringLiteral("LONGASSET12")), 1);
        QCOMPARE(refused(QStringLiteral("ABCDEFGHIJKL")), 1);

        // The message names the code (std::runtime_error now).
        try {
            AssetTypeCreditAlphaNum12(QStringLiteral("USD"), KeyPair::fromAccountId(issuer));
            QFAIL("a 3-character credit_alphanum12 code was taken");
        } catch (const std::runtime_error& e) {
            QVERIFY2(QString::fromUtf8(e.what()).contains(QStringLiteral("\"USD\"")), e.what());
        }
    }

    void testHashCode() {
        KeyPair* issuer1 = KeyPair::random();
        KeyPair* issuer2 = KeyPair::random();

        // Equal
        QCOMPARE(AssetTypeNative().hashCode(), AssetTypeNative().hashCode());
        QCOMPARE(AssetTypeCreditAlphaNum4("USD", *issuer1).hashCode(), AssetTypeCreditAlphaNum4("USD", *issuer1).hashCode());
        QCOMPARE(AssetTypeCreditAlphaNum12("ABCDE", *issuer1).hashCode(), AssetTypeCreditAlphaNum12("ABCDE", *issuer1).hashCode());

        // Not equal

        QVERIFY(AssetTypeNative().hashCode()!= AssetTypeCreditAlphaNum4("USD", *issuer1).hashCode());
        QVERIFY(AssetTypeNative().hashCode()!= AssetTypeCreditAlphaNum12("ABCDE", *issuer1).hashCode());
        QVERIFY(AssetTypeCreditAlphaNum4("EUR", *issuer1).hashCode()!= AssetTypeCreditAlphaNum4("USD", *issuer1).hashCode());
        QVERIFY(AssetTypeCreditAlphaNum4("EUR", *issuer1).hashCode()!= AssetTypeCreditAlphaNum4("EUR", *issuer2).hashCode());
        QVERIFY(AssetTypeCreditAlphaNum4("EUR", *issuer1).hashCode()!= AssetTypeCreditAlphaNum12("EUROPE", *issuer1).hashCode());
        QVERIFY(AssetTypeCreditAlphaNum4("EUR", *issuer1).hashCode()!= AssetTypeCreditAlphaNum12("EUROPE", *issuer2).hashCode());
        QVERIFY(AssetTypeCreditAlphaNum12("ABCDE", *issuer1).hashCode()!= AssetTypeCreditAlphaNum12("EDCBA", *issuer1).hashCode());
        QVERIFY(AssetTypeCreditAlphaNum12("ABCDE", *issuer1).hashCode()!= AssetTypeCreditAlphaNum12("ABCDE", *issuer2).hashCode());
    }

    void testAssetEquals() {
        KeyPair* issuer1 = KeyPair::random();
        KeyPair* issuer2 = KeyPair::random();
        qDebug()<< "KEYS? ";
        qDebug() << issuer1->getAccountId();
        qDebug() << issuer2->getAccountId();
        QVERIFY(AssetTypeNative().equals(new AssetTypeNative()));
        QVERIFY(AssetTypeCreditAlphaNum4("USD", *issuer1).equals(new AssetTypeCreditAlphaNum4("USD", *issuer1)));
        QVERIFY(AssetTypeCreditAlphaNum12("ABCDE", *issuer1).equals(new AssetTypeCreditAlphaNum12("ABCDE", *issuer1)));

        QVERIFY(!AssetTypeNative().equals(new AssetTypeCreditAlphaNum4("USD", *issuer1)));
        QVERIFY(!AssetTypeNative().equals(new AssetTypeCreditAlphaNum12("ABCDE", *issuer1)));
        QVERIFY(!AssetTypeCreditAlphaNum4("EUR", *issuer1).equals(new AssetTypeCreditAlphaNum4("USD", *issuer1)));
        QVERIFY(!AssetTypeCreditAlphaNum4("EUR", *issuer1).equals(new AssetTypeCreditAlphaNum4("EUR", *issuer2)));
        QVERIFY(!AssetTypeCreditAlphaNum12("ABCDE", *issuer1).equals(new AssetTypeCreditAlphaNum12("EDCBA", *issuer1)));
        QVERIFY(!AssetTypeCreditAlphaNum12("ABCDE", *issuer1).equals(new AssetTypeCreditAlphaNum12("ABCDE", *issuer2)));
    }

    /**
     * create(type, code, issuer) made every type but "native" the asset the
     * code's length made: "credit_alphanum12" with "USD" came back an
     * alphanum4 USD, "credit_alphanum4" with "USDCX" an alphanum12 asset, and
     * a type nobody knows (a typo, "liquidity_pool_shares") an issued asset,
     * or whatever the code made; "native" with a code or an issuer was XLM.
     * Each is refused now, with a std::runtime_error naming type and code.
     */
    void testCreateByTypeRefusesWhatTheTypeDoesNotName()
    {
        const QString issuer = QStringLiteral("GAYSHLG75RPSMXWJ5KX7O7STE6RSZTD6NE4CTWAXFZYYVYIFRUVJIBJH");
        struct Case { const char* type; QString code; QString issuer; };
        const QList<Case> cases = {
            { "credit_alphanum12", QStringLiteral("USD"), issuer },          // was an alphanum4 USD
            { "credit_alphanum12", QStringLiteral("ABCDEFGHIJKLM"), issuer },
            { "credit_alphanum4", QStringLiteral("USDCX"), issuer },         // was an alphanum12 asset
            { "credit_alphanum4", QString(), issuer },
            { "credit_alphanum4", QStringLiteral("US$"), issuer },           // a code Core refuses
            { "credit_alphanum12", QStringLiteral("EURO ZONE"), issuer },
            { "credit_alphanum12", QString::fromUtf8("EUR\xC3\x91OS"), issuer }, // not ASCII
            { "credit_alphanum", QStringLiteral("USD"), issuer },            // was an alphanum4 USD
            { "CREDIT_ALPHANUM4", QStringLiteral("USD"), issuer },
            { "", QStringLiteral("USD"), issuer },
            { "liquidity_pool_shares", QString(), QString() },
            { "native", QStringLiteral("XLM"), QString() },                  // was XLM
            { "native", QString(), issuer },                                 // was XLM
            { "Native", QString(), QString() },
        };
        for (const Case& c : cases) {
            const QString type = QString::fromLatin1(c.type);
            QString why;
            try {
                delete Asset::create(type, c.code, c.issuer);
                why = QStringLiteral("read as an asset");
            } catch (const std::runtime_error& e) {
                why = QString::fromUtf8(e.what());
                if (!why.contains(QLatin1Char('"') + type + QLatin1Char('"'))
                        || !why.contains(QLatin1Char('"') + c.code + QLatin1Char('"')))
                    why = QStringLiteral("does not name the type and the code: ") + why;
                else
                    why.clear();
            } catch (const std::exception& e) {
                why = QStringLiteral("not a std::runtime_error: ") + QString::fromUtf8(e.what());
            }
            QVERIFY2(why.isEmpty(), qPrintable(type + QStringLiteral(" / ") + c.code + QStringLiteral(": ") + why));
        }
        // The issuer is still checked: one that is not an account id throws.
        bool threw = false;
        try {
            delete Asset::create(QStringLiteral("credit_alphanum4"), QStringLiteral("USD"),
                                 QStringLiteral("GNOTANACCOUNT"));
        } catch (const std::exception&) {
            threw = true;
        }
        QVERIFY(threw);
    }

    /** What the type names, at both ends of each code length. */
    void testCreateByTypeBuildsTheAssetTheTypeNames()
    {
        const QString issuer = QStringLiteral("GAYSHLG75RPSMXWJ5KX7O7STE6RSZTD6NE4CTWAXFZYYVYIFRUVJIBJH");
        QScopedPointer<Asset> native(Asset::create(QStringLiteral("native"), QString(), QString()));
        QVERIFY(dynamic_cast<AssetTypeNative*>(native.data()));
        QScopedPointer<Asset> nativeEmpty(Asset::create(QStringLiteral("native"), QStringLiteral(""), QStringLiteral("")));
        QVERIFY(dynamic_cast<AssetTypeNative*>(nativeEmpty.data()));
        const QList<QPair<QString, QString> > credits = {
            { QStringLiteral("credit_alphanum4"), QStringLiteral("A") },
            { QStringLiteral("credit_alphanum4"), QStringLiteral("USDC") },
            { QStringLiteral("credit_alphanum12"), QStringLiteral("ZZYZX") },
            { QStringLiteral("credit_alphanum12"), QStringLiteral("ZyEpRQXogjjO") },
        };
        for (const auto& credit : credits) {
            QScopedPointer<Asset> asset(Asset::create(credit.first, credit.second, issuer));
            QCOMPARE(asset->getType(), credit.first);
            QCOMPARE(asset->toString(), credit.second + QLatin1Char(':') + issuer);
        }
    }

    /**
     * Real pubnet assets and trades (links dropped from the records) build
     * the asset Horizon names: codes of 1, 4, 5 and 12 characters, and the
     * native legs of trades.
     */
    void testCreateByTypeReadsRealHorizonAssetsAndTrades()
    {
        Page<AssetResponse> assets;
        assets.loadFromJson(realPubnetAssets());
        QCOMPARE(assets.size(), 4);
        for (int i = 0; i < assets.size(); ++i) {
            AssetResponse& record = assets.get(i);
            QCOMPARE(record.getAsset()->getType(), record.getAssetType());
            QCOMPARE(record.getAsset()->toString(), record.getAssetCode() + QLatin1Char(':') + record.getAssetIssuer());
        }
        QCOMPARE(assets.get(2).getAsset()->getType(), QString("credit_alphanum12"));
        QCOMPARE(assets.get(3).getAsset()->toString(),
                 QString("A:GA5O7NTUIKYDPZSV3MNMIRFO2LCV5RJBNO7TNN7Y3MDO2IXRBSWDEO3T"));

        Page<TradeResponse> trades;
        trades.loadFromJson(realPubnetTrades());
        QCOMPARE(trades.size(), 3);
        QCOMPARE(trades.get(0).getBaseAsset()->getType(), QString("native"));
        QCOMPARE(trades.get(0).getCounterAsset()->toString(),
                 QString("Libre100:GBVYKIM4QGW7NK3Q5S76GWMZMLAWYB6FB43JQ4J7ZOXHAA3LEZZGYHSD"));
        QCOMPARE(trades.get(0).getCounterAsset()->getType(), QString("credit_alphanum12"));
        QCOMPARE(trades.get(1).getBaseAsset()->toString(),
                 QString("VELO:GDM4RQUQQUVSKQA7S6EM7XBZP3FCGH4Q7CL6TABQ7B2BEJ5ERARM2M5M"));
        QCOMPARE(trades.get(1).getBaseAsset()->getType(), QString("credit_alphanum4"));
        QCOMPARE(trades.get(2).getBaseAsset()->getType(), QString("credit_alphanum4"));
        QCOMPARE(trades.get(2).getCounterAsset()->getType(), QString("native"));
    }

private:
    /** Four real pubnet assets, codes of 4, 5, 12 and 1 characters (links dropped). */
    static QByteArray realPubnetAssets()
    {
        return QByteArray(
            "{\"_embedded\":{\"records\":[{\"_links\":{\"toml\":{\"href\":\"https://lobstr.co/.well-known/stellar.toml\"}},\"asset_type\":\"credit_alphanum4\",\"asset_code\":\"ZZZZ\",\"asset_issuer\":\"GA5BUT4SND34VR"
            "UJGFEVLG6LMEYOU5HSSYZLX673I2IJVRLLPATMH4RN\",\"paging_token\":\"ZZZZ_GA5BUT4SND34VRUJGFEVLG6LMEYOU5HSSYZLX673I2I"
            "JVRLLPATMH4RN_credit_alphanum4\",\"num_claimable_balances\":0,\"num_liquidity_pools\":0,\"num_contracts\":0,\"accoun"
            "ts\":{\"authorized\":1,\"authorized_to_maintain_liabilities\":0,\"unauthorized\":0},\"claimable_balances_amount\":\"0."
            "0000000\",\"liquidity_pools_amount\":\"0.0000000\",\"contracts_amount\":\"0.0000000\",\"balances\":{\"authorized\":\"0.010"
            "0000\",\"authorized_to_maintain_liabilities\":\"0.0000000\",\"unauthorized\":\"0.0000000\"},\"flags\":{\"auth_required\":"
            "false,\"auth_revocable\":false,\"auth_immutable\":false,\"auth_clawback_enabled\":false}},{\"_links\":{\"toml\":{\"href\":\"https://kiss.stellarskull.io/.well-known/stellar.toml\"}},\"asset_type\":\"credit_al"
            "phanum12\",\"asset_code\":\"ZZYZX\",\"asset_issuer\":\"GA4DIX2AQJFDZRALZLRRWERDHOFLQZ25GXPXBEA6UULMFE6ZYJEPZMQX\",\"pa"
            "ging_token\":\"ZZYZX_GA4DIX2AQJFDZRALZLRRWERDHOFLQZ25GXPXBEA6UULMFE6ZYJEPZMQX_credit_alphanum12\",\"num_claimabl"
            "e_balances\":0,\"num_liquidity_pools\":0,\"num_contracts\":0,\"accounts\":{\"authorized\":135,\"authorized_to_maintain"
            "_liabilities\":0,\"unauthorized\":0},\"claimable_balances_amount\":\"0.0000000\",\"liquidity_pools_amount\":\"0.000000"
            "0\",\"contracts_amount\":\"0.0000000\",\"balances\":{\"authorized\":\"98997924518.6090910\",\"authorized_to_maintain_lia"
            "bilities\":\"0.0000000\",\"unauthorized\":\"0.0000000\"},\"flags\":{\"auth_required\":false,\"auth_revocable\":false,\"aut"
            "h_immutable\":false,\"auth_clawback_enabled\":false}},{\"_links\":{\"toml\":{\"href\":\"\"}},\"asset_type\":\"credit_alphanum12\",\"asset_code\":\"ZyEpRQXog"
            "jjO\",\"asset_issuer\":\"GDS6O3DEADYMB56UXRN2NTD6SRUKUNXVTZZXC35LB4FCAZWFNUJRHK6N\",\"paging_token\":\"ZyEpRQXogjjO_"
            "GDS6O3DEADYMB56UXRN2NTD6SRUKUNXVTZZXC35LB4FCAZWFNUJRHK6N_credit_alphanum12\",\"num_claimable_balances\":0,\"num_"
            "liquidity_pools\":0,\"num_contracts\":0,\"accounts\":{\"authorized\":1,\"authorized_to_maintain_liabilities\":0,\"unau"
            "thorized\":0},\"claimable_balances_amount\":\"0.0000000\",\"liquidity_pools_amount\":\"0.0000000\",\"contracts_amount\""
            ":\"0.0000000\",\"balances\":{\"authorized\":\"99.0000000\",\"authorized_to_maintain_liabilities\":\"0.0000000\",\"unautho"
            "rized\":\"0.0000000\"},\"flags\":{\"auth_required\":false,\"auth_revocable\":false,\"auth_immutable\":false,\"auth_clawb"
            "ack_enabled\":false}},{\"_links\":{\"toml\":{\"href\":\"https://lobstr.co/.well-known/stellar.toml\"}},\"asset_type\":\"credit_alphanum4\",\"asset_code\":\"A\",\"asset_issuer\":\"GA5O7NTUIKYDPZSV3MNMI"
            "RFO2LCV5RJBNO7TNN7Y3MDO2IXRBSWDEO3T\",\"paging_token\":\"A_GA5O7NTUIKYDPZSV3MNMIRFO2LCV5RJBNO7TNN7Y3MDO2IXRBSWDE"
            "O3T_credit_alphanum4\",\"num_claimable_balances\":0,\"num_liquidity_pools\":1,\"num_contracts\":0,\"accounts\":{\"auth"
            "orized\":13,\"authorized_to_maintain_liabilities\":0,\"unauthorized\":0},\"claimable_balances_amount\":\"0.0000000\","
            "\"liquidity_pools_amount\":\"22038.1490879\",\"contracts_amount\":\"0.0000000\",\"balances\":{\"authorized\":\"9223371906"
            "74.3284436\",\"authorized_to_maintain_liabilities\":\"0.0000000\",\"unauthorized\":\"0.0000000\"},\"flags\":{\"auth_requ"
            "ired\":false,\"auth_revocable\":true,\"auth_immutable\":false,\"auth_clawback_enabled\":true}}]},\"_links\":{\"self\":{"
            "\"href\":\"https://horizon.stellar.org/assets?cursor=&limit=200&order=desc\"},\"next\":{\"href\":\"https://horizon.st"
            "ellar.org/assets?cursor=zUSD_GAQLH5PIARNOCWLKWPDMO3DB7QU6EE23MILWBCDRHXJD2YZ344TYG5NE_credit_alphanum4&limit"
            "=200&order=desc\"},\"prev\":{\"href\":\"https://horizon.stellar.org/assets?cursor=ZZZZ_GA5BUT4SND34VRUJGFEVLG6LMEY"
            "OU5HSSYZLX673I2IJVRLLPATMH4RN_credit_alphanum4&limit=200&order=asc\"}}}");
    }

    /** Three real pubnet trades: native / alphanum12, alphanum4 / alphanum12, alphanum4 / native. */
    static QByteArray realPubnetTrades()
    {
        return QByteArray(
            "{\"_embedded\":{\"records\":[{\"_links\":{\"self\":{\"href\":\"\"},\"base\":{\"href\":\"https://horizon.stellar.org/accounts/GBK66IESUU4ZW5YKNW53BORA4XN4KVC6HXI6Q5A3P7BHDETY5CN22S7A\"},\"counter\":{\"href\":\"https://horizon.stellar.org/liquidity_pools/161ed837446e99e9b6ee6dbbe5b20d428bfd7249f678376081c37afd0a8eb5f1\"},\"operation\":{\"href\":\"https://horizon.stellar.org/operations/277775815868366853\"}},\"id\":\"277775815868366853-0\",\"paging_token\":\"277775815868366853-0\",\"ledger_close_ti"
            "me\":\"2026-09-29T05:23:11Z\",\"trade_type\":\"liquidity_pool\",\"liquidity_pool_fee_bp\":30,\"base_offer_id\":\"4889461"
            "834295754757\",\"base_account\":\"GBK66IESUU4ZW5YKNW53BORA4XN4KVC6HXI6Q5A3P7BHDETY5CN22S7A\",\"base_amount\":\"0.010"
            "7847\",\"base_asset_type\":\"native\",\"counter_liquidity_pool_id\":\"161ed837446e99e9b6ee6dbbe5b20d428bfd7249f67837"
            "6081c37afd0a8eb5f1\",\"counter_amount\":\"0.0081436\",\"counter_asset_type\":\"credit_alphanum12\",\"counter_asset_cod"
            "e\":\"Libre100\",\"counter_asset_issuer\":\"GBVYKIM4QGW7NK3Q5S76GWMZMLAWYB6FB43JQ4J7ZOXHAA3LEZZGYHSD\",\"base_is_sel"
            "ler\":false,\"price\":{\"n\":\"81436\",\"d\":\"107847\"}},{\"_links\":{\"self\":{\"href\":\"\"},\"base\":{\"href\":\"https://horizon.stellar.org/liquidity_pools/065e0575a20bdf319ad06ce187adb24441963016ebdc97e342a664e6a13acf72\"},\"counter\":{\"href\":\"https://horizon.stellar.org/accounts/GBK66IESUU4ZW5YKNW53BORA4XN4KVC6HXI6Q5A3P7BHDETY5CN22S7A\"},\"operation\":{\"href\":\"https://horizon.stellar.org/operations/277775815868366853\"}},\"id\":\"277775815868366853-1\",\"paging_token\":\"2777758158683668"
            "53-1\",\"ledger_close_time\":\"2026-09-29T05:23:11Z\",\"trade_type\":\"liquidity_pool\",\"liquidity_pool_fee_bp\":30,\"b"
            "ase_liquidity_pool_id\":\"065e0575a20bdf319ad06ce187adb24441963016ebdc97e342a664e6a13acf72\",\"base_amount\":\"0.4"
            "981623\",\"base_asset_type\":\"credit_alphanum4\",\"base_asset_code\":\"VELO\",\"base_asset_issuer\":\"GDM4RQUQQUVSKQA7S"
            "6EM7XBZP3FCGH4Q7CL6TABQ7B2BEJ5ERARM2M5M\",\"counter_offer_id\":\"4889461834295754757\",\"counter_account\":\"GBK66IE"
            "SUU4ZW5YKNW53BORA4XN4KVC6HXI6Q5A3P7BHDETY5CN22S7A\",\"counter_amount\":\"0.0081436\",\"counter_asset_type\":\"credit"
            "_alphanum12\",\"counter_asset_code\":\"Libre100\",\"counter_asset_issuer\":\"GBVYKIM4QGW7NK3Q5S76GWMZMLAWYB6FB43JQ4J"
            "7ZOXHAA3LEZZGYHSD\",\"base_is_seller\":true,\"price\":{\"n\":\"81436\",\"d\":\"4981623\"}},{\"_links\":{\"self\":{\"href\":\"\"},\"base\":{\"href\":\"https://horizon.stellar.org/accounts/GD77RS525TLPXBSFTZSHDEVDSCDO3NQBTATLRJQVHBOTKUC2PMGPTQEF\"},\"counter\":{\"href\":\"https://horizon.stellar.org/accounts/GB2XJWJ63M72PI4WW4FKXN5YC66O6MXF5DBNOKDFGIF23T2XKCCC7WHD\"},\"operation\":{\"href\":\"https://horizon.stellar.org/operations/277775815868121089\"}},\"id\":\"277775815868121089-3\",\""
            "paging_token\":\"277775815868121089-3\",\"ledger_close_time\":\"2026-09-29T05:23:11Z\",\"trade_type\":\"orderbook\",\"ba"
            "se_offer_id\":\"4889461834295508993\",\"base_account\":\"GD77RS525TLPXBSFTZSHDEVDSCDO3NQBTATLRJQVHBOTKUC2PMGPTQEF\""
            ",\"base_amount\":\"0.0114981\",\"base_asset_type\":\"credit_alphanum4\",\"base_asset_code\":\"XRP\",\"base_asset_issuer\":"
            "\"GBXRPL45NPHCVMFFAYZVUVFFVKSIZ362ZXFP7I2ETNQ3QKZMFLPRDTD5\",\"counter_offer_id\":\"1859505320\",\"counter_account\""
            ":\"GB2XJWJ63M72PI4WW4FKXN5YC66O6MXF5DBNOKDFGIF23T2XKCCC7WHD\",\"counter_amount\":\"0.0760000\",\"counter_asset_type"
            "\":\"native\",\"base_is_seller\":false,\"price\":{\"n\":\"66097879\",\"d\":\"10000000\"}}]},\"_links\":{\"self\":{\"href\":\"https"
            "://horizon.stellar.org/trades?cursor=&limit=200&order=desc\"},\"next\":{\"href\":\"https://horizon.stellar.org/tra"
            "des?cursor=277775798688043009-0&limit=200&order=desc\"},\"prev\":{\"href\":\"https://horizon.stellar.org/trades?cu"
            "rsor=277775815868366853-2&limit=200&order=asc\"}}}");
    }
};

ADD_TEST(AssetTest)
#endif // ASSETTEST_H
