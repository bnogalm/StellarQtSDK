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
    QString m_closeTime;
    QString m_headerXdr;
    QString m_metadataXdr;
public:
    /** Throws std::runtime_error when a field is missing or not what the RPC
     *  sends: an id that is not 64 hex digits, a number that is not a JSON
     *  integer in range, a close time that is not a decimal string, or XDR
     *  that is not base64. closeTime, headerXdr and metadataXdr are required,
     *  as py-stellar-sdk requires them and js-stellar-sdk the two XDRs. The
     *  close time is the RPC's decimal string; py also takes a JSON integer,
     *  which the RPC does not send. */
    static GetLatestLedgerResponse fromJson(const QJsonObject& result);
    QString getId() const { return m_id; }
    qint32 getProtocolVersion() const { return m_protocolVersion; }
    quint32 getSequence() const { return m_sequence; }
    /** Unix seconds, as decimal text (as GetLedgersResponse's close times). */
    QString getCloseTime() const { return m_closeTime; }
    /** The ledger's LedgerHeader, base64 XDR. */
    QString getHeaderXdr() const { return m_headerXdr; }
    /** The ledger's LedgerCloseMeta, base64 XDR. */
    QString getMetadataXdr() const { return m_metadataXdr; }
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(GetLatestLedgerResponse)




#endif // GETLATESTLEDGERRESPONSE_H
