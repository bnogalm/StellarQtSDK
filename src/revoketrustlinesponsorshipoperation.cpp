#include "revoketrustlinesponsorshipoperation.h"
#include <cstring>
#include <stdexcept>

QSTELLAR_BEGIN_NS




RevokeTrustlineSponsorshipOperation::RevokeTrustlineSponsorshipOperation(QString accountId, Asset *asset)
    : m_asset(nullptr)   // FIX: was uninitialized → getAsset()/dtor read/deleted garbage
{
    auto& revokeSponsorshipLedgerEntry =m_op.fillRevokeSponsorshipLedgerEntry();
    auto& trustLine = revokeSponsorshipLedgerEntry.fillTrustLine();
    trustLine.accountID = StrKey::encodeToXDRAccountId(accountId);
    trustLine.asset = asset->toXdr();       // the same three arms, as a TrustLineAsset
}

RevokeTrustlineSponsorshipOperation::RevokeTrustlineSponsorshipOperation(QString accountId, const QByteArray &liquidityPoolId)
    : m_asset(nullptr)
{
    auto& trustLine = m_op.fillRevokeSponsorshipLedgerEntry().fillTrustLine();
    trustLine.accountID = StrKey::encodeToXDRAccountId(accountId);
    // A PoolID is a SHA-256: 32 bytes, never padded or cut to fit.
    if (liquidityPoolId.size() != int(sizeof(trustLine.asset.liquidityPoolID)))
        throw std::runtime_error("a liquidity pool id is 32 bytes");
    trustLine.asset.type = stellar::AssetType::ASSET_TYPE_POOL_SHARE;
    std::memcpy(trustLine.asset.liquidityPoolID, liquidityPoolId.constData(),
                sizeof(trustLine.asset.liquidityPoolID));
}

RevokeTrustlineSponsorshipOperation::RevokeTrustlineSponsorshipOperation(stellar::RevokeSponsorshipOp &op):m_asset(nullptr),m_op(op)
{

}

RevokeTrustlineSponsorshipOperation::~RevokeTrustlineSponsorshipOperation()
{
    if(m_asset)
        delete m_asset;
}

QString RevokeTrustlineSponsorshipOperation::getAccountId() const
{
    return StrKey::encodeStellarAccountId(m_op.ledgerKey.trustLine.accountID);
}

Asset *RevokeTrustlineSponsorshipOperation::getAsset()
{
    if(!m_asset)
    {
        const stellar::TrustLineAsset& line = m_op.ledgerKey.trustLine.asset;
        stellar::Asset asset;
        switch (line.type) {
        case stellar::AssetType::ASSET_TYPE_NATIVE: break;
        case stellar::AssetType::ASSET_TYPE_CREDIT_ALPHANUM4:
            std::memcpy(asset.alphaNum4.assetCode, line.alphaNum4.assetCode, sizeof(asset.alphaNum4.assetCode));
            asset.alphaNum4.issuer = line.alphaNum4.issuer; break;
        case stellar::AssetType::ASSET_TYPE_CREDIT_ALPHANUM12:
            std::memcpy(asset.alphaNum12.assetCode, line.alphaNum12.assetCode, sizeof(asset.alphaNum12.assetCode));
            asset.alphaNum12.issuer = line.alphaNum12.issuer; break;
        case stellar::AssetType::ASSET_TYPE_POOL_SHARE:
            return nullptr;             // pool shares are no asset: see getLiquidityPoolID()
        default:
            throw std::runtime_error("unknown trustline asset type");
        }
        asset.type = line.type;
        m_asset = Asset::fromXdr(asset);
    }
    return m_asset;
}

bool RevokeTrustlineSponsorshipOperation::isLiquidityPoolShare() const
{
    return m_op.ledgerKey.trustLine.asset.type == stellar::AssetType::ASSET_TYPE_POOL_SHARE;
}

QByteArray RevokeTrustlineSponsorshipOperation::getLiquidityPoolID() const
{
    if (!isLiquidityPoolShare())
        return QByteArray();
    const stellar::TrustLineAsset& line = m_op.ledgerKey.trustLine.asset;
    return QByteArray(reinterpret_cast<const char*>(line.liquidityPoolID), sizeof(line.liquidityPoolID));
}

void RevokeTrustlineSponsorshipOperation::fillOperationBody(AccountConverter &accountConverter, stellar::Operation &op)
{
    Q_UNUSED(accountConverter)
    auto& o = op.fillRevokeSponsorshipOp();
    o = m_op;
}

RevokeTrustlineSponsorshipOperation *RevokeTrustlineSponsorshipOperation::build(stellar::RevokeSponsorshipOp &op)
{
    return new RevokeTrustlineSponsorshipOperation(op);
}

RevokeTrustlineSponsorshipOperation *RevokeTrustlineSponsorshipOperation::create(QString accountId, Asset *asset)
{
    checkNotNull(accountId, "accountId cannot be null");
    checkNotNull(asset, "asset cannot be null");
    return new RevokeTrustlineSponsorshipOperation(accountId,asset);
}

RevokeTrustlineSponsorshipOperation *RevokeTrustlineSponsorshipOperation::create(QString accountId, const QByteArray &liquidityPoolId)
{
    checkNotNull(accountId, "accountId cannot be null");
    return new RevokeTrustlineSponsorshipOperation(accountId, liquidityPoolId);
}

RevokeTrustlineSponsorshipOperation *RevokeTrustlineSponsorshipOperation::setSourceAccount(QString sourceAccount)
{
    checkNotNull(sourceAccount, "sourceAccount cannot be null");
    Operation::setSourceAccount(sourceAccount);
    return this;
}
QSTELLAR_END_NS
