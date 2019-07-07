#ifndef OPERATIONFEESTATSREQUESTBUILDER_H
#define OPERATIONFEESTATSREQUESTBUILDER_H

#include "requestbuilder.h"
#include "../responses/feestatsresponse.h"

class FeeStatsRequestBuilder : public RequestBuilder
{
public:
    FeeStatsRequestBuilder(Server* server);

    /**
     * Requests <code>GET /operation_fee_stats</code>
     * @see <a href="https://www.stellar.org/developers/horizon/reference/operation-fee-stats.html">Operation Fee Stats</a>
     */
    FeeStatsResponse* execute(QUrl uri);
    FeeStatsResponse* execute();


};

#endif // OPERATIONFEESTATSREQUESTBUILDER_H


