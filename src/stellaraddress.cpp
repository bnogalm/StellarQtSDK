#include "stellaraddress.h"

#include <stdexcept>

#include "strkey.h"

QSTELLAR_BEGIN_NS

StellarAddress StellarAddress::fromString(const QString& strkey)
{
    if (strkey.isEmpty()) {
        throw std::runtime_error("strkey cannot be empty");
    }
    const QChar prefix = strkey.at(0);
    if (prefix == 'G') {
        return StellarAddress(Type::ACCOUNT, StrKey::decodeStellarAccountId(strkey));
    } else if (prefix == 'M') {
        // For muxed accounts the decoded body is 40 bytes (32 ed25519 + 8 id).
        return StellarAddress(Type::MUXED_ACCOUNT,
                              StrKey::decodeStellarMuxedAccount(strkey));
    } else if (prefix == 'C') {
        return StellarAddress(Type::CONTRACT, StrKey::decodeContract(strkey));
    } else if (prefix == 'B') {
        return StellarAddress(Type::CLAIMABLE_BALANCE, StrKey::decodeClaimableBalance(strkey));
    } else if (prefix == 'L') {
        return StellarAddress(Type::LIQUIDITY_POOL, StrKey::decodeLiquidityPool(strkey));
    }
    throw std::runtime_error("unsupported strkey prefix");
}

QString StellarAddress::toString() const
{
    switch (m_type) {
    case Type::ACCOUNT:
        return StrKey::encodeStellarAccountId(m_bytes);
    case Type::MUXED_ACCOUNT:
        return QString::fromLatin1(
            StrKey::encodeCheck(StrKey::VersionByte::MUXED_ACCOUNT, m_bytes));
    case Type::CONTRACT:
        return StrKey::encodeContract(m_bytes);
    case Type::CLAIMABLE_BALANCE:
        return StrKey::encodeClaimableBalance(m_bytes);
    case Type::LIQUIDITY_POOL:
        return StrKey::encodeLiquidityPool(m_bytes);
    }
    throw std::runtime_error("invalid StellarAddress type");
}

QSTELLAR_END_NS
