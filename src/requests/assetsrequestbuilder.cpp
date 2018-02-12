#include "assetsrequestbuilder.h"
#include "../server.h"
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
