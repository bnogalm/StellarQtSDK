#ifndef STRICTSENDPATHSREQUESTBUILDER_H
#define STRICTSENDPATHSREQUESTBUILDER_H
#include "requestbuilder.h"
#include "../responses/pathresponse.h"
#include "../responses/page.h"

class KeyPair;
class Asset;
/**
 * Builds requests connected to paths.
 */
class StrictSendPathsRequestBuilder : public RequestBuilder
{
public:
    StrictSendPathsRequestBuilder(Server* server);
    StrictSendPathsRequestBuilder& destinationAccount(const KeyPair& account);

    StrictSendPathsRequestBuilder& destinationAssets(QList<Asset *> assets);
    StrictSendPathsRequestBuilder& sourceAmount(QString amount);
    StrictSendPathsRequestBuilder& sourceAsset(Asset* asset);
    Page<PathResponse> *execute(QUrl uri);
    Page<PathResponse> *execute();
};

#endif // STRICTSENDPATHSREQUESTBUILDER_H
