#include "assetsrequestbuilder.h"
#include "../server.h"
#include "../asset.h"
#include "../assettypecreditalphanum.h"
#include "../util.h"

QSTELLAR_BEGIN_NS

AssetsRequestBuilder::AssetsRequestBuilder(Server *server):RequestBuilder(server, "assets")
{

}

AssetsRequestBuilder& AssetsRequestBuilder::assetCode(QString assetCode) {
    addParameter("asset_code", assetCode);
    return *this;
}

AssetsRequestBuilder& AssetsRequestBuilder::assetIssuer(QString assetIssuer) {
    addParameter("asset_issuer", assetIssuer);
    return *this;
}

AssetsRequestBuilder& AssetsRequestBuilder::forAsset(Asset* asset)
{
    AssetTypeCreditAlphaNum* credit = Util::assertNonNativeAsset(asset);
    assetCode(credit->getCode());
    assetIssuer(credit->getIssuer().getAccountId());
    return *this;
}

Page<AssetResponse>* AssetsRequestBuilder::execute(QUrl uri){
    this->setRequestUri(uri);//we overwrite the uri
    return server()->get<Page<AssetResponse> >(this);
}

Page<AssetResponse>* AssetsRequestBuilder::execute(){
    return execute(this->buildUri());
}

AssetsRequestBuilder &AssetsRequestBuilder::cursor(QString token)
{
    RequestBuilder::cursor(token);
    return *this;
}

AssetsRequestBuilder &AssetsRequestBuilder::limit(qint32 number)
{
    RequestBuilder::limit(number);
    return *this;
}

AssetsRequestBuilder &AssetsRequestBuilder::order(RequestBuilder::Order direction)
{
    RequestBuilder::order(direction);
    return *this;
}
QSTELLAR_END_NS
