#ifndef ROOTREQUESTBUILDER_H
#define ROOTREQUESTBUILDER_H
#include "requestbuilder.h"
#include "../responses/rootresponse.h"

#include <QUrl>


/**
 * Builds request for root network state.
 */

class RootRequestBuilder : protected RequestBuilder
{
public:
    RootRequestBuilder(Server *server);
    RootResponse* execute();
    QUrl buildUri();
};

#endif // ROOTREQUESTBUILDER_H
