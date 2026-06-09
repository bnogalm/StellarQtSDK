#include "muxedaccount.h"

#include <cstring>
#include <stdexcept>

#include "strkey.h"

QSTELLAR_BEGIN_NS

MuxedAccount::MuxedAccount()
{
    m_xdr.type = stellar::CryptoKeyType::KEY_TYPE_ED25519;
    std::memset(m_xdr.ed25519, 0, sizeof(m_xdr.ed25519));
}

MuxedAccount MuxedAccount::fromAddress(const QString& strkey)
{
    if (strkey.isEmpty()) {
        throw std::runtime_error("strkey cannot be empty");
    }
    MuxedAccount out;
    out.m_xdr = StrKey::encodeToXDRMuxedAccount(strkey);
    return out;
}

MuxedAccount MuxedAccount::fromAccountIdAndMuxedId(const QString& accountId, quint64 muxedId)
{
    if (accountId.size() != StrKey::ACCOUNT_ID_ADDRESS_LENGTH) {
        throw std::runtime_error("invalid G-strkey for muxed account base");
    }
    QByteArray decoded = StrKey::decodeStellarAccountId(accountId);

    MuxedAccount out;
    out.m_xdr.type = stellar::CryptoKeyType::KEY_TYPE_MUXED_ED25519;
    out.m_xdr.med25519.id = muxedId;
    std::memcpy(out.m_xdr.med25519.ed25519, decoded.constData(), 32);
    decoded.fill('\0', decoded.size());
    return out;
}

MuxedAccount MuxedAccount::fromAccountId(const QString& accountId)
{
    if (accountId.size() != StrKey::ACCOUNT_ID_ADDRESS_LENGTH) {
        throw std::runtime_error("invalid G-strkey");
    }
    QByteArray decoded = StrKey::decodeStellarAccountId(accountId);
    MuxedAccount out;
    out.m_xdr.type = stellar::CryptoKeyType::KEY_TYPE_ED25519;
    std::memcpy(out.m_xdr.ed25519, decoded.constData(), 32);
    decoded.fill('\0', decoded.size());
    return out;
}

bool MuxedAccount::isMuxed() const
{
    return m_xdr.type == stellar::CryptoKeyType::KEY_TYPE_MUXED_ED25519;
}

QString MuxedAccount::getAccountId() const
{
    stellar::AccountID accId = StrKey::muxedAccountToAccountId(m_xdr);
    return StrKey::encodeStellarAccountId(accId);
}

quint64 MuxedAccount::getMuxedId() const
{
    return isMuxed() ? m_xdr.med25519.id : 0;
}

QString MuxedAccount::toAddress() const
{
    return StrKey::encodeStellarMuxedAccount(m_xdr);
}

QSTELLAR_END_NS
