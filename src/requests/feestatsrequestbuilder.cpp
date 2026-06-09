#include "feestatsrequestbuilder.h"
#include "../server.h"

QSTELLAR_BEGIN_NS

FeeStatsRequestBuilder::FeeStatsRequestBuilder(Server *server)
    :RequestBuilder(server, "fee_stats")
{

}


FeeStatsResponse *FeeStatsRequestBuilder::execute(QUrl uri){
    this->setRequestUri(uri);
    return server()->get<FeeStatsResponse>(this);
}

FeeStatsResponse *FeeStatsRequestBuilder::execute() {
    return execute(this->buildUri());
}
QSTELLAR_END_NS
