#ifndef GETVERSIONINFORESPONSE_H
#define GETVERSIONINFORESPONSE_H

#include <QJsonObject>
#include <QString>
#include <QtGlobal>

/** Soroban RPC `getVersionInfo` result. */
class GetVersionInfoResponse
{
    QString m_version;
    QString m_commitHash;
    QString m_buildTimestamp;
    QString m_captiveCoreVersion;
    qint32 m_protocolVersion = 0;
public:
    static GetVersionInfoResponse fromJson(const QJsonObject& result);
    QString getVersion() const { return m_version; }
    QString getCommitHash() const { return m_commitHash; }
    QString getBuildTimestamp() const { return m_buildTimestamp; }
    QString getCaptiveCoreVersion() const { return m_captiveCoreVersion; }
    qint32 getProtocolVersion() const { return m_protocolVersion; }
};

#endif // GETVERSIONINFORESPONSE_H
