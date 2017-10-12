#ifndef PATHSREQUESTBUILDER_H
#define PATHSREQUESTBUILDER_H
#include "requestbuilder.h"
#include "../responses/pathresponse.h"
#include "../responses/page.h"

class KeyPair;
class Asset;
/**
 * Builds requests connected to paths.
 */
class PathsRequestBuilder : public RequestBuilder
{
public:
    PathsRequestBuilder(Server* server);
    PathsRequestBuilder& destinationAccount(KeyPair* account);
    PathsRequestBuilder& sourceAccount(KeyPair* account);
    PathsRequestBuilder& destinationAmount(QString amount);
    PathsRequestBuilder& destinationAsset(Asset* asset);
    Page<PathResponse> *execute(QUrl uri);
    Page<PathResponse> *execute();
};

#endif // PATHSREQUESTBUILDER_H
