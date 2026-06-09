#ifndef GETLATESTLEDGERRESPONSE_H
#define GETLATESTLEDGERRESPONSE_H

#include <QJsonObject>
#include <QString>
#include <QtGlobal>
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/** Soroban RPC `getLatestLedger` result. */
class GetLatestLedgerResponse
{
    QString m_id;
    qint32 m_protocolVersion = 0;
    quint32 m_sequence = 0;
public:
    static GetLatestLedgerResponse fromJson(const QJsonObject& result);
    QString getId() const { return m_id; }
    qint32 getProtocolVersion() const { return m_protocolVersion; }
    quint32 getSequence() const { return m_sequence; }
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(GetLatestLedgerResponse)




#endif // GETLATESTLEDGERRESPONSE_H
