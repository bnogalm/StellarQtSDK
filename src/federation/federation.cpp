#include "federation.h"
#include "../stellartomlresolver.h"
#include "federationresponse.h"
#include "federationserver.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>
#include <QUrl>
#include <stdexcept>
#include "../strkey.h"

QSTELLAR_BEGIN_NS


Federation::Federation(){}

namespace {

// A G or M address exactly as it encodes: checksum and canonical spelling.
bool isAccountAddress(const QString& value)
{
    try {
        if (value.size() == StrKey::ACCOUNT_ID_ADDRESS_LENGTH) {
            return StrKey::decodeVersionByte(value) == StrKey::VersionByte::ACCOUNT_ID
                   && StrKey::encodeStellarAccountId(StrKey::decodeStellarAccountId(value)) == value;
        }
        if (value.size() == StrKey::MUXED_ACCOUNT_ADDRESS_LENGTH) {
            return StrKey::decodeVersionByte(value) == StrKey::VersionByte::MUXED_ACCOUNT
                   && StrKey::encodeStellarMuxedAccount(StrKey::encodeToXDRMuxedAccount(value)) == value;
        }
    } catch (const std::exception&) {
    }
    return false;
}

} // namespace

void Federation::validate(QString value) {
    const QString trimmedValue = value.trimmed();
    // An empty value used to come back as the account id "", with no reply
    // behind it: whoever waited for an answer waited for ever.
    if (trimmedValue.isEmpty())
        throw std::runtime_error("Malformed address: the value is empty");
    const QStringList tokens = trimmedValue.split("*");
    if (tokens.count() == 1) {
        // Handed back as the answer, so it has to be one. The value is not
        // quoted: a secret seed pasted by mistake would end up in a log.
        if (!isAccountAddress(trimmedValue))
            throw std::runtime_error("Invalid account id: a value without '*' must be an account id (G... or M...)");
    } else if (tokens.count() == 2) {
        if (tokens[0].isEmpty())
            throw std::runtime_error("Malformed address: the name before '*' is empty");
        // A host name, with an optional port as js and py take one, and
        // nothing else: it is pasted into the stellar.toml URL, so
        // "good.example@evil.example" was fetched from evil.example.
        if (!StellarTomlResolver::isHostName(tokens[1]))
            throw std::runtime_error("Malformed address: the domain after '*' is not a host name");
    } else {
        throw std::runtime_error("Malformed address");
    }
}

FederationResponse *Federation::resolve(QString value) {
    validate(value);
    QString trimmedValue = value.trimmed();
    QStringList tokens = trimmedValue.split("*");
    if (tokens.count() == 1) {
        // An account id is its own answer. ready() is emitted as for any
        // other lookup (queued, so the caller can connect first): it never
        // was, and a caller waiting on ready() waited for ever. Its json() is
        // the record js hands back, {account_id}: it was {}.
        FederationResponse* response = new FederationResponse(nullptr);
        response->loadFromJson(QJsonDocument(QJsonObject{{QStringLiteral("account_id"), trimmedValue}})
                                   .toJson(QJsonDocument::Compact));
        QMetaObject::invokeMethod(response, "ready", Qt::QueuedConnection);
        return response;

    } else if (tokens.count() == 2) {
        QString domain = tokens[1];
        FederationServer* server = FederationServer::createForDomain(domain);        
        // resolveAddress() reports the failure of a server still reading its
        // stellar.toml, with its reason, once.
        auto response = server->resolveAddress(trimmedValue);
        // Without the cache the server is this lookup's alone and goes with
        // its response. With it, the cache owns the server, which other
        // lookups share (FederationServer::createForDomain).
        if(!FederationServer::isCacheActive())
            server->setParent(response);
        return response;
    } else {
        throw std::runtime_error("Malformed address");
    }
}
QSTELLAR_END_NS
