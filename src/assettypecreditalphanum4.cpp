#include "assettypecreditalphanum4.h"
#include "util.h"
#include "keypair.h"

AssetTypeCreditAlphaNum4::AssetTypeCreditAlphaNum4(QString code, KeyPair *issuer)
    :AssetTypeCreditAlphaNum(code,issuer){
    if (code.length() < 1 || code.length() > 4) {
        throw new AssetCodeLengthInvalidException();
    }
}
AssetTypeCreditAlphaNum4::AssetTypeCreditAlphaNum4(QString code,const KeyPair &issuer)
    :AssetTypeCreditAlphaNum(code,issuer){
    if (code.length() < 1 || code.length() > 4) {
        throw new AssetCodeLengthInvalidException();
    }
}
QString AssetTypeCreditAlphaNum4::getType() {
    return QString("credit_alphanum4");
}

stellar::Asset AssetTypeCreditAlphaNum4::toXdr() {
    stellar::Asset xdr;
    xdr.type = stellar::AssetType::ASSET_TYPE_CREDIT_ALPHANUM4;
    QByteArray padded = Util::paddedByteArray(this->m_code,4);
    memcpy(&xdr.alphaNum4.assetCode,padded.data(),sizeof(xdr.alphaNum4.assetCode));

    xdr.alphaNum4.issuer = m_issuer->getXdrPublicKey();

    return xdr;
}
