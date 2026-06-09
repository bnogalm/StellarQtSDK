#ifndef ASSETSREQUESTBUILDER_H
#define ASSETSREQUESTBUILDER_H

#include "requestbuilder.h"
#include "../responses/effectspage.h"
#include "../responses/page.h"
#include "../responses/assetresponse.h"
#include "../qstellar_namespace.h"
QSTELLAR_FWD(Asset)
QSTELLAR_BEGIN_NS


/**
 * Builds requests connected to assets.
 */

class AssetsRequestBuilder : public RequestBuilder
{
public:
    AssetsRequestBuilder(Server *server);
    AssetsRequestBuilder& assetCode(QString assetCode);
    AssetsRequestBuilder& assetIssuer(QString assetIssuer);
    /**
     * Shortcut for `assetCode(asset.code).assetIssuer(asset.issuer)`.
     * Native asset is not a valid filter here (Horizon would 400).
     */
    AssetsRequestBuilder& forAsset(Asset* asset);

    Page<AssetResponse>* execute(QUrl uri);
    Page<AssetResponse>* execute();
    AssetsRequestBuilder& cursor(QString token);
    AssetsRequestBuilder& limit(qint32 number);
    AssetsRequestBuilder& order(RequestBuilder::Order direction);
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(AssetsRequestBuilder)

#endif // ASSETSREQUESTBUILDER_H
