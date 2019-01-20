#include "operationfeestatsrequestbuilder.h"
#include "../server.h"
OperationFeeStatsRequestBuilder::OperationFeeStatsRequestBuilder(Server *server)
    :RequestBuilder(server, "operation_fee_stats")
{

}


OperationFeeStatsResponse *OperationFeeStatsRequestBuilder::execute(QUrl uri){
    this->setRequestUri(uri);
    return server()->get<OperationFeeStatsResponse>(this);
}

OperationFeeStatsResponse *OperationFeeStatsRequestBuilder::execute() {
    return execute(this->buildUri());
}
