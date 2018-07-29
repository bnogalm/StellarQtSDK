#include "rootrequestbuilder.h"
#include "server.h"

RootRequestBuilder::RootRequestBuilder(Server *server):RequestBuilder(server)
{

}

RootResponse *RootRequestBuilder::execute()
{
    QUrl url=this->buildUri();
    this->setRequestUri(url);
    return server()->get<RootResponse>(this);
}

QUrl RootRequestBuilder::buildUri()
{
    return RequestBuilder::buildUri();
}
