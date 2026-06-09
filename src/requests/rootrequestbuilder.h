#ifndef ROOTREQUESTBUILDER_H
#define ROOTREQUESTBUILDER_H
#include "requestbuilder.h"
#include "../responses/rootresponse.h"

#include <QUrl>
#include "../qstellar_namespace.h"
QSTELLAR_BEGIN_NS



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

QSTELLAR_END_NS

QSTELLAR_ALIAS(RootRequestBuilder)

#endif // ROOTREQUESTBUILDER_H
