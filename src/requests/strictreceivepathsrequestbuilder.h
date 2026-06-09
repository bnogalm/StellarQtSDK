#ifndef STRICTRECEIVEPATHSREQUESTBUILDER_H
#define STRICTRECEIVEPATHSREQUESTBUILDER_H
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

QSTELLAR_END_NS

QSTELLAR_ALIAS(StrictReceivePathsRequestBuilder)

#endif // STRICTRECEIVEPATHSREQUESTBUILDER_H
