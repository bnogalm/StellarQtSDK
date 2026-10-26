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
    /**
     * Sends `destination_account`, which is not a parameter of
     * /paths/strict-receive as Horizon documents it (source_account or
     * source_assets, and the destination asset and amount), so no filter
     * should be expected from it. java's legacy /paths endpoint took it;
     * js's and py's strict-receive builders have no such parameter. Kept
     * so 1.0.x code still builds.
     * @deprecated Not a strict-receive parameter; leave it out. (On
     * strict-send, StrictSendPathsRequestBuilder::destinationAccount is a
     * real filter.)
     */
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
