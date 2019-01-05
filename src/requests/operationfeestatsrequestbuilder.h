#ifndef OPERATIONFEESTATSREQUESTBUILDER_H
#define OPERATIONFEESTATSREQUESTBUILDER_H

#include "requestbuilder.h"
#include "../responses/operationfeestatsresponse.h"

class OperationFeeStatsRequestBuilder : public RequestBuilder
{
public:
    OperationFeeStatsRequestBuilder(Server* server);

    /**
     * Requests <code>GET /operation_fee_stats</code>
     * @see <a href="https://www.stellar.org/developers/horizon/reference/operation-fee-stats.html">Operation Fee Stats</a>
     */
    OperationFeeStatsResponse* execute(QUrl uri);
    OperationFeeStatsResponse* execute();


};

#endif // OPERATIONFEESTATSREQUESTBUILDER_H


