#ifndef ASSETSREQUESTBUILDER_H
#define ASSETSREQUESTBUILDER_H

#include "requestbuilder.h"
#include "../responses/effectspage.h"
#include "../responses/page.h"
#include "../responses/assetresponse.h"
/**
 * Builds requests connected to assets.
 */

class AssetsRequestBuilder : public RequestBuilder
{
public:
    AssetsRequestBuilder(Server *server);
    AssetsRequestBuilder& assetCode(QString assetCode);

    AssetsRequestBuilder& assetIssuer(QString assetIssuer);
    Page<AssetResponse>* execute(QUrl uri);
    Page<AssetResponse>* execute();
    AssetsRequestBuilder& cursor(QString token);
    AssetsRequestBuilder& limit(qint32 number);
    AssetsRequestBuilder& order(RequestBuilder::Order direction);
};

#endif // ASSETSREQUESTBUILDER_H


