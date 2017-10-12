#include "assettypecreditalphanum12.h"
#include "util.h"
#include "keypair.h"


AssetTypeCreditAlphaNum12::AssetTypeCreditAlphaNum12(QString code, KeyPair *issuer)
    :AssetTypeCreditAlphaNum(code, issuer) {

    if (code.length() < 5 || code.length() > 12) {
        throw new AssetCodeLengthInvalidException();
    }
}
AssetTypeCreditAlphaNum12::AssetTypeCreditAlphaNum12(QString code,const KeyPair &issuer)
    :AssetTypeCreditAlphaNum(code, issuer) {

    if (code.length() < 5 || code.length() > 12) {
        throw new AssetCodeLengthInvalidException();
    }
}
QString AssetTypeCreditAlphaNum12::getType() {
    return QString("credit_alphanum12");
}

stellar::Asset AssetTypeCreditAlphaNum12::toXdr() {
    stellar::Asset xdr;
    xdr.type = stellar::AssetType::ASSET_TYPE_CREDIT_ALPHANUM12;
    QByteArray padded = Util::paddedByteArray(this->m_code,12);
    memcpy(&xdr.alphaNum12.assetCode,padded.data(),sizeof(xdr.alphaNum12.assetCode));
    xdr.alphaNum12.issuer = m_issuer->getXdrPublicKey();
    return xdr;
}
