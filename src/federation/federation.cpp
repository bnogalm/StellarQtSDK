#include "federation.h"
#include "federationresponse.h"
#include "federationserver.h"
#include <QStringList>
#include <stdexcept>

Federation::Federation(){}

FederationResponse *Federation::resolve(QString value) {
    QString trimmedValue = value.trimmed();
    QStringList tokens = trimmedValue.split("*");
    if (tokens.count() == 1) {
        // accountId
        return new FederationResponse(0, trimmedValue, 0, 0);

    } else if (tokens.count() == 2) {
        QString domain = tokens[1];
        FederationServer* server = FederationServer::createForDomain(domain);        
        auto response = server->resolveAddress(trimmedValue);
        QObject::connect(server,&FederationServer::error,response,&FederationResponse::error);
        if(!FederationServer::isCacheActive())
            server->setParent(response);
        return response;
    } else {
        throw std::runtime_error("Malformed address");
    }
}
