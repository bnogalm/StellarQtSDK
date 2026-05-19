#ifndef LIQUIDITYPOOLTEST_H
#define LIQUIDITYPOOLTEST_H

#include <QObject>
#include <QtTest>
#include "testcollector.h"

#include "../src/liquiditypool.h"
#include "../src/asset.h"
#include "../src/assettypenative.h"
#include "../src/assettypecreditalphanum.h"
#include "../src/assettypecreditalphanum4.h"
#include "../src/assettypecreditalphanum12.h"
#include "../src/keypair.h"

/**
 * Tests for CAP-38 Liquidity Pool helpers (XDR types + pool ID).
 */
class LiquidityPoolTest: public QObject
{
    Q_OBJECT
private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    /** Asset ordering: NATIVE < ALPHANUM4 < ALPHANUM12. */
    void testCompareAssetsByType()
    {
        AssetTypeNative native;
        QScopedPointer<Asset> alpha4(Asset::create("USDA", "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"));
        QScopedPointer<Asset> alpha12(Asset::create("LONGCODE", "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"));

        QVERIFY(LiquidityPool::compareAssets(&native, alpha4.data()) < 0);
        QVERIFY(LiquidityPool::compareAssets(alpha4.data(), alpha12.data()) < 0);
        QVERIFY(LiquidityPool::compareAssets(&native, alpha12.data()) < 0);
        QVERIFY(LiquidityPool::compareAssets(alpha4.data(), &native) > 0);
    }

    /** Same type+code, ordering driven by issuer bytes. */
    void testCompareAssetsByIssuer()
    {
        QScopedPointer<Asset> a(Asset::create("USDX", "GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7"));
        QScopedPointer<Asset> b(Asset::create("USDX", "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"));

        const int cmp = LiquidityPool::compareAssets(a.data(), b.data());
        QVERIFY(cmp != 0);
        QCOMPARE(LiquidityPool::compareAssets(b.data(), a.data()), -cmp);
    }

    /** getLiquidityPoolID refuses inverted asset order. */
    void testInvertedOrderThrows()
    {
        AssetTypeNative native;
        QScopedPointer<Asset> usd(Asset::create("USD", "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"));

        bool threw = false;
        try {
            LiquidityPool::getLiquidityPoolID(
                stellar::LiquidityPoolType::LIQUIDITY_POOL_CONSTANT_PRODUCT,
                usd.data(), &native, LiquidityPool::FEE);
        } catch (const std::exception&) {
            threw = true;
        }
        QVERIFY(threw);
    }

    /** Returned hash is 32 bytes (SHA-256). */
    void testPoolIdLength()
    {
        AssetTypeNative native;
        QScopedPointer<Asset> usd(Asset::create("USD", "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"));

        QByteArray id = LiquidityPool::getLiquidityPoolID(
            stellar::LiquidityPoolType::LIQUIDITY_POOL_CONSTANT_PRODUCT,
            &native, usd.data(), LiquidityPool::FEE);
        QCOMPARE(id.size(), 32);
    }

    /** Deterministic: same inputs → same pool ID. */
    void testPoolIdDeterministic()
    {
        AssetTypeNative native;
        QScopedPointer<Asset> usd(Asset::create("USD", "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"));

        QByteArray id1 = LiquidityPool::getLiquidityPoolID(
            stellar::LiquidityPoolType::LIQUIDITY_POOL_CONSTANT_PRODUCT,
            &native, usd.data(), LiquidityPool::FEE);
        QByteArray id2 = LiquidityPool::getLiquidityPoolID(
            stellar::LiquidityPoolType::LIQUIDITY_POOL_CONSTANT_PRODUCT,
            &native, usd.data(), LiquidityPool::FEE);
        QCOMPARE(id1, id2);
    }

    /** Different fee → different ID. */
    void testPoolIdDependsOnFee()
    {
        AssetTypeNative native;
        QScopedPointer<Asset> usd(Asset::create("USD", "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"));

        QByteArray id30 = LiquidityPool::getLiquidityPoolID(
            stellar::LiquidityPoolType::LIQUIDITY_POOL_CONSTANT_PRODUCT,
            &native, usd.data(), 30);
        QByteArray id100 = LiquidityPool::getLiquidityPoolID(
            stellar::LiquidityPoolType::LIQUIDITY_POOL_CONSTANT_PRODUCT,
            &native, usd.data(), 100);
        QVERIFY(id30 != id100);
    }

    /** XDR round-trip for LiquidityPoolParameters. */
    void testParametersXdrRoundTrip()
    {
        AssetTypeNative native;
        QScopedPointer<Asset> usd(Asset::create("USD", "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"));

        stellar::LiquidityPoolParameters params;
        auto& cp = params.fillConstantProduct();
        cp.assetA = native.toXdr();
        cp.assetB = usd->toXdr();
        cp.fee = LiquidityPool::FEE;

        QByteArray bytes;
        {
            QDataStream out(&bytes, QIODevice::WriteOnly);
            out << params;
        }

        stellar::LiquidityPoolParameters parsed;
        {
            QDataStream in(&bytes, QIODevice::ReadOnly);
            in >> parsed;
        }

        QCOMPARE(static_cast<int>(parsed.type),
                 static_cast<int>(stellar::LiquidityPoolType::LIQUIDITY_POOL_CONSTANT_PRODUCT));
        QCOMPARE(parsed.constantProduct.fee, LiquidityPool::FEE);
        QCOMPARE(static_cast<int>(parsed.constantProduct.assetA.type),
                 static_cast<int>(stellar::AssetType::ASSET_TYPE_NATIVE));
        QCOMPARE(static_cast<int>(parsed.constantProduct.assetB.type),
                 static_cast<int>(stellar::AssetType::ASSET_TYPE_CREDIT_ALPHANUM4));
    }

    /**
     * Pool ID = SHA-256(LiquidityPoolParameters XDR).
     * Builds the canonical XDR bytes by hand from issuer raw pubkey and asserts
     * the helper matches.
     */
    void testPoolIdMatchesManualXdr()
    {
        const QString issuerStr = "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG";
        AssetTypeNative native;
        QScopedPointer<Asset> usdAsset(Asset::create("USD", issuerStr));
        auto* usd = static_cast<AssetTypeCreditAlphaNum*>(usdAsset.data());

        // Canonical XDR layout (big-endian, all fields 4-byte aligned):
        //   pool type (uint32) = 0
        //   assetA: type=0 (NATIVE)
        //   assetB: type=1 (ALPHANUM4) | code "USD\0" | issuer (PublicKey: type=0 + 32 bytes ed25519)
        //   fee (int32) = 30
        QByteArray expected;
        QDataStream s(&expected, QIODevice::WriteOnly);
        s << qint32(0);                                // LIQUIDITY_POOL_CONSTANT_PRODUCT
        s << qint32(0);                                // assetA: NATIVE
        s << qint32(1);                                // assetB: ALPHANUM4
        s.writeRawData("USD\0", 4);                    // code padded to 4
        s << qint32(0);                                // PublicKeyType::ED25519
        QByteArray pub = usd->getIssuer().getPublicKey();
        QCOMPARE(pub.size(), 32);
        s.writeRawData(pub.constData(), 32);
        s << qint32(LiquidityPool::FEE);               // fee

        const QByteArray expectedHash =
            QCryptographicHash::hash(expected, QCryptographicHash::Sha256);

        const QByteArray actual = LiquidityPool::getLiquidityPoolID(
            stellar::LiquidityPoolType::LIQUIDITY_POOL_CONSTANT_PRODUCT,
            &native, usd, LiquidityPool::FEE);

        QCOMPARE(actual.size(), 32);
        QCOMPARE(actual.toHex(), expectedHash.toHex());
    }
};

ADD_TEST(LiquidityPoolTest)
#endif // LIQUIDITYPOOLTEST_H
