#ifndef ROOTRESPONSE_H
#define ROOTRESPONSE_H
#include "response.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
* Represents root endpoint response.
* @see org.stellar.sdk.Server#root()
*/
class RootResponse : public Response
{
    Q_OBJECT
    Q_PROPERTY(QString horizon_version MEMBER m_horizonVersion)
    Q_PROPERTY(QString core_version MEMBER m_coreVersion)
    Q_PROPERTY(qint32 history_latest_ledger MEMBER m_historyLatestLedger)
    Q_PROPERTY(qint32 history_elder_ledger MEMBER m_historyElderLedger)
    Q_PROPERTY(qint32 core_latest_ledger MEMBER m_coreLatestLedger)
    Q_PROPERTY(QString network_passphrase MEMBER m_networkPassphrase)
    Q_PROPERTY(qint32 protocol_version MEMBER m_protocolVersion)
    Q_PROPERTY(qint32 current_protocol_version MEMBER m_currentProtocolVersion)
    Q_PROPERTY(qint32 core_supported_protocol_version MEMBER m_coreSupportedProtocolVersion)
    Q_PROPERTY(qint64 ingest_latest_ledger MEMBER m_ingestLatestLedger)
    Q_PROPERTY(QString history_latest_ledger_closed_at MEMBER m_historyLatestLedgerClosedAt)
    Q_PROPERTY(qint32 supported_protocol_version MEMBER m_supportedProtocolVersion)

    QString m_horizonVersion;
    QString m_coreVersion;
    qint32 m_historyLatestLedger;
    qint32 m_historyElderLedger;
    qint32 m_coreLatestLedger;
    QString m_networkPassphrase;
    qint32 m_protocolVersion;
    qint32 m_currentProtocolVersion;
    qint32 m_coreSupportedProtocolVersion;
    qint64 m_ingestLatestLedger = 0;
    QString m_historyLatestLedgerClosedAt;
    qint32 m_supportedProtocolVersion = 0;
public:
    RootResponse(QNetworkReply *reply=nullptr);
    virtual ~RootResponse();
    QString getHorizonVersion() const;

    QString getStellarCoreVersion() const;

    qint32 getHistoryLatestLedger() const;

    qint32 getHistoryElderLedger() const;

    qint32 getCoreLatestLedger() const;

    QString getNetworkPassphrase() const;

    /**
     * @deprecated Will be removed in Horizon 0.17.0
     */
    Q_DECL_DEPRECATED qint32 getProtocolVersion() const;

    qint32 getCurrentProtocolVersion() const;

    qint32 getCoreSupportedProtocolVersion() const;

    /** The last ledger Horizon ingested. 0 if absent. */
    qint64 getIngestLatestLedger() const { return m_ingestLatestLedger; }

    /** When history_latest_ledger closed, as Horizon writes it (ISO 8601). Empty if absent. */
    QString getHistoryLatestLedgerClosedAt() const { return m_historyLatestLedgerClosedAt; }

    /**
     * The newest protocol this Horizon supports. During an upgrade window it
     * is ahead of getCurrentProtocolVersion(). 0 if absent.
     */
    qint32 getSupportedProtocolVersion() const { return m_supportedProtocolVersion; }
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(RootResponse)



Q_DECLARE_METATYPE(qstellar::RootResponse*)
#endif // ROOTRESPONSE_H

