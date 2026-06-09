#ifndef STRICTSENDPATHSREQUESTBUILDER_H
#define STRICTSENDPATHSREQUESTBUILDER_H
#include "requestbuilder.h"
#include "../responses/pathresponse.h"
#include "../responses/page.h"

#include "../keypair.h"
#include "../qstellar_namespace.h"
QSTELLAR_FWD(Asset)
QSTELLAR_BEGIN_NS

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

QSTELLAR_END_NS

QSTELLAR_ALIAS(StrictSendPathsRequestBuilder)

#endif // STRICTSENDPATHSREQUESTBUILDER_H
