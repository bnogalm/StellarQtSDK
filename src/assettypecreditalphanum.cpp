#include "assettypecreditalphanum.h"

#include "keypair.h"
#include "util.h"

AssetTypeCreditAlphaNum::AssetTypeCreditAlphaNum(QString code, KeyPair *issuer) {
    m_code =checkNotNull(code, "code cannot be null");
    m_issuer = checkNotNull(issuer, "issuer cannot be null");
}
AssetTypeCreditAlphaNum::AssetTypeCreditAlphaNum(QString code,const KeyPair& issuer) {
    m_code =checkNotNull(code, "code cannot be null");
    QString account = checkNotNull(issuer.getAccountId(), "issuer cannot be null");
    m_issuer = KeyPair::fromAccountId(account);
}

AssetTypeCreditAlphaNum::~AssetTypeCreditAlphaNum()
{
    delete m_issuer;
}

QString AssetTypeCreditAlphaNum::getCode() const{
    return m_code;
}

const KeyPair &AssetTypeCreditAlphaNum::getIssuer() const{
    return *m_issuer;
}

uint AssetTypeCreditAlphaNum::hashCode() const{
    return qHash(m_issuer->getAccountId() + this->getCode());
}

bool AssetTypeCreditAlphaNum::equals(Asset *object) {
    if(AssetTypeCreditAlphaNum* asset = dynamic_cast<AssetTypeCreditAlphaNum*>(object))
    {
        return (this->m_code== asset->m_code) && (this->m_issuer->getAccountId() == asset->m_issuer->getAccountId());
    }
    return false;
}

QString AssetTypeCreditAlphaNum::toString() const
{
    return getCode()+":"+this->getIssuer().getAccountId();
}

AssetTypeCreditAlphaNum *checkNotNull(AssetTypeCreditAlphaNum *asset, const char *error)
{
    if(!asset){
        throw std::runtime_error(error);
    }
    return asset;
}
