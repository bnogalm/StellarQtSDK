#ifndef CHANGETRUSTASSETTEST_H
#define CHANGETRUSTASSETTEST_H

#include <QObject>
#include <QtTest>
#include <stdexcept>
#include "testcollector.h"

#include "../src/changetrustasset.h"
#include "../src/trustlineasset.h"
#include "../src/asset.h"
#include "../src/assettypenative.h"
#include "../src/assettypecreditalphanum4.h"
#include "../src/liquiditypool.h"
#include "../src/changetrustoperation.h"
#include "../src/operation.h"

/** Tests for the ChangeTrustAsset / TrustLineAsset XDR wrappers (CAP-38). */
class ChangeTrustAssetTest: public QObject
{
    Q_OBJECT
private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    // ─── ChangeTrustAsset ────────────────────────────────────────────────

    void testWrapNativeRoundTrip()
    {
        QScopedPointer<ChangeTrustAsset> wrapped(ChangeTrustAsset::wrap(new AssetTypeNative()));
        QCOMPARE(wrapped->getType(), QString("native"));

        stellar::ChangeTrustAsset xdr = wrapped->toXdr();
        QCOMPARE(static_cast<int>(xdr.type),
                 static_cast<int>(stellar::AssetType::ASSET_TYPE_NATIVE));

        QScopedPointer<ChangeTrustAsset> back(ChangeTrustAsset::fromXdr(xdr));
        QVERIFY(wrapped->equals(back.data()));
    }

    void testWrapAlphaNum4RoundTrip()
    {
        QScopedPointer<ChangeTrustAsset> wrapped(ChangeTrustAsset::wrap(
            Asset::create("USD", "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG")));
        QCOMPARE(wrapped->getType(), QString("credit_alphanum4"));

        QByteArray bytes;
        { QDataStream s(&bytes, QIODevice::WriteOnly); s << wrapped->toXdr(); }
        stellar::ChangeTrustAsset xdr;
        { QDataStream s(&bytes, QIODevice::ReadOnly); s >> xdr; }

        QScopedPointer<ChangeTrustAsset> back(ChangeTrustAsset::fromXdr(xdr));
        QVERIFY(wrapped->equals(back.data()));
    }

    void testLiquidityPoolShareRoundTrip()
    {
        Asset* a = new AssetTypeNative();
        Asset* b = Asset::create("USD", "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");

        QScopedPointer<ChangeTrustAsset> lp(
            new LiquidityPoolShareChangeTrustAsset(a, b, LiquidityPool::FEE));
        QCOMPARE(lp->getType(), QString("liquidity_pool_shares"));

        QByteArray bytes;
        { QDataStream s(&bytes, QIODevice::WriteOnly); s << lp->toXdr(); }
        stellar::ChangeTrustAsset xdr;
        { QDataStream s(&bytes, QIODevice::ReadOnly); s >> xdr; }
        QCOMPARE(static_cast<int>(xdr.type),
                 static_cast<int>(stellar::AssetType::ASSET_TYPE_POOL_SHARE));

        QScopedPointer<ChangeTrustAsset> back(ChangeTrustAsset::fromXdr(xdr));
        QVERIFY(lp->equals(back.data()));

        // Pool ID computed by the wrapper matches the helper.
        auto* lpCast = static_cast<LiquidityPoolShareChangeTrustAsset*>(lp.data());
        QScopedPointer<Asset> nat(new AssetTypeNative());
        QScopedPointer<Asset> usd(
            Asset::create("USD", "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"));
        QByteArray expected = LiquidityPool::getLiquidityPoolID(
            stellar::LiquidityPoolType::LIQUIDITY_POOL_CONSTANT_PRODUCT,
            nat.data(), usd.data(), LiquidityPool::FEE);
        QCOMPARE(lpCast->getLiquidityPoolID().toHex(), expected.toHex());
    }

    void testLiquidityPoolShareRejectsInvertedOrder()
    {
        Asset* a = new AssetTypeNative();
        Asset* b = Asset::create("USD", "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        bool threw = false;
        try {
            // Inverted order — b > a so passing (b, a) must throw.
            new LiquidityPoolShareChangeTrustAsset(b, a, LiquidityPool::FEE);
        } catch (const std::exception&) {
            threw = true;
            delete a; delete b; // wrapper didn't take ownership before throw
        }
        QVERIFY(threw);
    }

    // ─── TrustLineAsset ──────────────────────────────────────────────────

    void testTrustLineWrapNativeRoundTrip()
    {
        QScopedPointer<TrustLineAsset> wrapped(TrustLineAsset::wrap(new AssetTypeNative()));
        QCOMPARE(wrapped->getType(), QString("native"));

        QByteArray bytes;
        { QDataStream s(&bytes, QIODevice::WriteOnly); s << wrapped->toXdr(); }
        stellar::TrustLineAsset xdr;
        { QDataStream s(&bytes, QIODevice::ReadOnly); s >> xdr; }
        QCOMPARE(static_cast<int>(xdr.type),
                 static_cast<int>(stellar::AssetType::ASSET_TYPE_NATIVE));

        QScopedPointer<TrustLineAsset> back(TrustLineAsset::fromXdr(xdr));
        QVERIFY(wrapped->equals(back.data()));
    }

    void testTrustLinePoolShareRoundTrip()
    {
        // Build a deterministic 32-byte poolId via the helper.
        QScopedPointer<Asset> nat(new AssetTypeNative());
        QScopedPointer<Asset> usd(
            Asset::create("USD", "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"));
        QByteArray poolId = LiquidityPool::getLiquidityPoolID(
            stellar::LiquidityPoolType::LIQUIDITY_POOL_CONSTANT_PRODUCT,
            nat.data(), usd.data(), LiquidityPool::FEE);

        QScopedPointer<TrustLineAsset> lp(new LiquidityPoolShareTrustLineAsset(poolId));
        QCOMPARE(lp->getType(), QString("liquidity_pool_shares"));

        QByteArray bytes;
        { QDataStream s(&bytes, QIODevice::WriteOnly); s << lp->toXdr(); }
        QCOMPARE(bytes.size(), 4 + 32); // type discriminant + poolID

        stellar::TrustLineAsset xdr;
        { QDataStream s(&bytes, QIODevice::ReadOnly); s >> xdr; }
        QScopedPointer<TrustLineAsset> back(TrustLineAsset::fromXdr(xdr));
        QVERIFY(lp->equals(back.data()));

        auto* lpCast = static_cast<LiquidityPoolShareTrustLineAsset*>(back.data());
        QCOMPARE(lpCast->getLiquidityPoolID(), poolId);
    }

    void testTrustLinePoolIdRejectsWrongSize()
    {
        bool threw = false;
        try {
            new LiquidityPoolShareTrustLineAsset(QByteArray(31, '\0'));
        } catch (const std::exception&) {
            threw = true;
        }
        QVERIFY(threw);
    }

    // ─── ChangeTrustOperation integration ────────────────────────────────

    /** Legacy ctor with Asset* still works (wire-compatible). */
    void testChangeTrustOperationLegacyCtor()
    {
        QScopedPointer<Asset> asset(
            Asset::create("USD", "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"));
        QScopedPointer<ChangeTrustOperation> op(
            ChangeTrustOperation::create(asset.data(), "100"));

        auto xdr = op->toXdr();
        QScopedPointer<ChangeTrustOperation> parsed(
            static_cast<ChangeTrustOperation*>(Operation::fromXdr(xdr)));

        QVERIFY(parsed->getAsset() != nullptr);
        QCOMPARE(parsed->getAsset()->getType(), QString("credit_alphanum4"));
        QCOMPARE(parsed->getLimit(), QString("100"));
    }

    /** New ctor with ChangeTrustAsset* — pool share variant. */
    void testChangeTrustOperationLiquidityPoolShare()
    {
        Asset* a = new AssetTypeNative();
        Asset* b = Asset::create("USD", "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        QScopedPointer<LiquidityPoolShareChangeTrustAsset> line(
            new LiquidityPoolShareChangeTrustAsset(a, b, LiquidityPool::FEE));

        QScopedPointer<ChangeTrustOperation> op(
            ChangeTrustOperation::create(line.data(), "100"));

        // Round-trip via Operation factory.
        auto xdr = op->toXdr();
        QCOMPARE(static_cast<int>(xdr.operationChangeTrust.line.type),
                 static_cast<int>(stellar::AssetType::ASSET_TYPE_POOL_SHARE));

        QScopedPointer<ChangeTrustOperation> parsed(
            static_cast<ChangeTrustOperation*>(Operation::fromXdr(xdr)));

        // getAsset() returns nullptr for LP variant.
        QVERIFY(parsed->getAsset() == nullptr);

        // getLine() returns the LP wrapper with the same pool ID.
        ChangeTrustAsset* parsedLine = parsed->getLine();
        QVERIFY(parsedLine != nullptr);
        auto* parsedLp = dynamic_cast<LiquidityPoolShareChangeTrustAsset*>(parsedLine);
        QVERIFY(parsedLp != nullptr);
        QCOMPARE(parsedLp->getLiquidityPoolID().toHex(),
                 line->getLiquidityPoolID().toHex());
        QCOMPARE(parsed->getLimit(), QString("100"));
    }

    /** Wire format for non-LP variants is identical to legacy ChangeTrustOp. */
    void testChangeTrustOperationWireCompatNative()
    {
        QScopedPointer<Asset> nat(new AssetTypeNative());
        QScopedPointer<ChangeTrustOperation> op(
            ChangeTrustOperation::create(nat.data(), "100"));
        op->setSourceAccount("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");

        // Pre-existing base64 fixture from operationtest.h-style invariants:
        // non-LP encoding hasn't changed at the byte level.
        QByteArray bytes;
        {
            QDataStream s(&bytes, QIODevice::WriteOnly);
            s << op->toXdr().operationChangeTrust;
        }
        // ChangeTrustAsset for native = 4 bytes type (0) + 0 payload, plus 8 bytes limit.
        QCOMPARE(bytes.size(), 4 + 8);
    }

    /**
     * A wrapped Asset whose type names no arm is refused, not passed on. It
     * became a ChangeTrustAsset of that type with no arm at all, which was
     * written as a bare discriminant.
     */
    void testAWrappedAssetOfNoKnownTypeIsRefused()
    {
        struct UnknownTypeAsset : public Asset
        {
            QString getType() override { return QStringLiteral("unknown"); }
            stellar::Asset toXdr() override
            {
                stellar::Asset xdr;
                xdr.type = stellar::AssetType(7);
                return xdr;
            }
            QString toString() const override { return QStringLiteral("unknown"); }
        };
        QScopedPointer<ChangeTrustAsset> wrapped(ChangeTrustAsset::wrap(new UnknownTypeAsset));
        bool threw = false;
        try { wrapped->toXdr(); } catch (const std::runtime_error&) { threw = true; }
        QVERIFY(threw);
    }

    /**
     * A pool share of a pool type that does not exist is refused. Its
     * parameters were read as a constant-product pool's regardless.
     */
    void testAPoolShareOfAnUnknownPoolTypeIsRefused()
    {
        stellar::ChangeTrustAsset xdr;
        xdr.fillLiquidityPool().type = stellar::LiquidityPoolType(1);
        bool threw = false;
        try { delete ChangeTrustAsset::fromXdr(xdr); } catch (const std::runtime_error&) { threw = true; }
        QVERIFY(threw);
    }

    /**
     * A decoded pool share whose assets are out of order or whose fee is not
     * 30 is refused, as js, py and core refuse it, and the two assets
     * decoded for it are freed: they leaked on every such decode (a leak
     * checker shows it; the refusal is what this can see). A caller of the
     * constructor still owns what it refused, as before.
     */
    void testARefusedPoolShareFromXdrIsRefusedWithoutLeaking()
    {
        Asset* a = new AssetTypeNative();
        Asset* b = Asset::create("USD", "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        QScopedPointer<ChangeTrustAsset> pool(new LiquidityPoolShareChangeTrustAsset(a, b, LiquidityPool::FEE));
        const stellar::ChangeTrustAsset good = pool->toXdr();
        QScopedPointer<ChangeTrustAsset> back(ChangeTrustAsset::fromXdr(good));
        QVERIFY(back->equals(pool.data()));

        stellar::ChangeTrustAsset otherFee = good;
        otherFee.liquidityPool().constantProduct.fee = 31;
        stellar::ChangeTrustAsset swapped = good;
        auto& cp = swapped.liquidityPool().constantProduct;
        const stellar::Asset first = cp.assetA;
        cp.assetA = cp.assetB;
        cp.assetB = first;
        for (const stellar::ChangeTrustAsset& bad : {otherFee, swapped}) {
            for (int i = 0; i < 3; ++i)
                QVERIFY_EXCEPTION_THROWN(delete ChangeTrustAsset::fromXdr(bad), std::runtime_error);
        }

        Asset* c = new AssetTypeNative();
        Asset* d = Asset::create("USD", "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        QVERIFY_EXCEPTION_THROWN(LiquidityPoolShareChangeTrustAsset(c, d, 31), std::runtime_error);
        delete c;   // still the caller's
        delete d;
    }

    /**
     * A plain Asset becomes the same TrustLineAsset, byte for byte after the
     * discriminant, which is how a trustline key filled with an Asset still
     * builds now that the key holds a TrustLineAsset. An Asset that names
     * POOL_SHARE carries no pool id and is refused, not turned into pool zero.
     */
    void testATrustLineAssetFromAnAsset()
    {
        QScopedPointer<Asset> usd(new AssetTypeCreditAlphaNum4(
            QStringLiteral("USD"), QStringLiteral("GD6ROJBYLKQMOW3E7N4M2YBPUHMZD7PL65VRHRMO24BOVSBV5H3BQRSL")));
        const stellar::Asset plain = usd->toXdr();
        const stellar::TrustLineAsset line = plain;
        auto bytes = [](const auto& value) {
            QByteArray out;
            QDataStream s(&out, QIODevice::WriteOnly);
            s << value;
            return out;
        };
        QCOMPARE(bytes(line), bytes(plain));
        const stellar::TrustLineAsset native = stellar::Asset();
        QCOMPARE(bytes(native), bytes(stellar::Asset()));

        stellar::Asset pool;
        pool.type = stellar::AssetType::ASSET_TYPE_POOL_SHARE;
        bool threw = false;
        try { stellar::TrustLineAsset converted = pool; Q_UNUSED(converted) } catch (const std::runtime_error&) { threw = true; }
        QVERIFY(threw);
    }
};

ADD_TEST(ChangeTrustAssetTest)
#endif // CHANGETRUSTASSETTEST_H
