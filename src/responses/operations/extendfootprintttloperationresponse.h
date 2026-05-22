#ifndef EXTENDFOOTPRINTTTLOPERATIONRESPONSE_H
#define EXTENDFOOTPRINTTTLOPERATIONRESPONSE_H

#include <QObject>
#include "operationresponse.h"

/**
 * Represents ExtendFootprintTTL operation response (CAP-46).
 *
 * Horizon exposes a single Soroban-specific field for this operation:
 * `extend_to` — the ledger height that the targeted entries' TTL is
 * extended up to.
 */
class ExtendFootprintTTLOperationResponse : public OperationResponse
{
    Q_OBJECT
    Q_PROPERTY(quint32 extend_to MEMBER m_extendTo)
    quint32 m_extendTo = 0;
public:
    explicit ExtendFootprintTTLOperationResponse(QNetworkReply* reply = nullptr);
    ~ExtendFootprintTTLOperationResponse() override = default;
    quint32 getExtendTo() const { return m_extendTo; }
};
Q_DECLARE_METATYPE(ExtendFootprintTTLOperationResponse*)
#endif // EXTENDFOOTPRINTTTLOPERATIONRESPONSE_H
