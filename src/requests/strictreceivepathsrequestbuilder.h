#ifndef STRICTRECEIVEPATHSREQUESTBUILDER_H
#define STRICTRECEIVEPATHSREQUESTBUILDER_H
#include "requestbuilder.h"
#include "../responses/pathresponse.h"
#include "../responses/page.h"

class KeyPair;
class Asset;
/**
 * Builds requests connected to paths.
 */
class StrictReceivePathsRequestBuilder : public RequestBuilder
{
public:
    StrictReceivePathsRequestBuilder(Server* server);
    StrictReceivePathsRequestBuilder& destinationAccount(const KeyPair& account);
    StrictReceivePathsRequestBuilder& sourceAccount(const KeyPair& account);
    StrictReceivePathsRequestBuilder &sourceAssets(const QList<Asset *> assets);
    StrictReceivePathsRequestBuilder& destinationAmount(QString amount);
    StrictReceivePathsRequestBuilder& destinationAsset(Asset* asset);
    Page<PathResponse> *execute(QUrl uri);
    Page<PathResponse> *execute();

};

#endif // STRICTRECEIVEPATHSREQUESTBUILDER_H
