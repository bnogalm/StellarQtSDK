#include "accountconverter.h"


AccountConverter::AccountConverter(bool enabled) : m_enableMuxed(enabled) {
}

AccountConverter& AccountConverter::enableMuxed() {
    m_enableMuxed=true;
    return *this;
}

AccountConverter& AccountConverter::disableMuxed() {
    m_enableMuxed=false;
    return *this;
}

stellar::MuxedAccount AccountConverter::encode(QString account) {
    stellar::MuxedAccount muxed = StrKey::encodeToXDRMuxedAccount(account);

    if (m_enableMuxed || muxed.type == stellar::CryptoKeyType::KEY_TYPE_ED25519) {
        return muxed;
    }

    stellar::MuxedAccount med25519;

    med25519.type = stellar::CryptoKeyType::KEY_TYPE_ED25519;
    memcpy(med25519.ed25519,muxed.med25519.ed25519,KeyPair::keyLength);
    return med25519;
}

QString AccountConverter::decode(stellar::MuxedAccount account) {
    if (m_enableMuxed || account.type == stellar::CryptoKeyType::KEY_TYPE_ED25519) {
        return StrKey::encodeStellarMuxedAccount(account);
    }


    return StrKey::encodeStellarAccountId(StrKey::muxedAccountToAccountId(account));
}

stellar::MuxedAccount AccountConverter::filter(stellar::MuxedAccount account)
{
    if (m_enableMuxed || account.type == stellar::CryptoKeyType::KEY_TYPE_ED25519) {
        return account;
    }

    stellar::MuxedAccount med25519;

    med25519.type = stellar::CryptoKeyType::KEY_TYPE_ED25519;
    memcpy(med25519.ed25519,account.med25519.ed25519,KeyPair::keyLength);
    return med25519;
}

QString AccountConverter::filter(QString account)
{
    stellar::MuxedAccount muxed = StrKey::encodeToXDRMuxedAccount(account);
    if(m_enableMuxed || muxed.type == stellar::CryptoKeyType::KEY_TYPE_ED25519)
    {
        return account;
    }
    return StrKey::encodeStellarAccountId(StrKey::muxedAccountToAccountId(muxed));
}
