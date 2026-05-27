#ifndef GETNETWORKRESPONSE_H
#define GETNETWORKRESPONSE_H

#include <QJsonObject>
#include <QString>
#include <QtGlobal>

/** Soroban RPC `getNetwork` result. */
class GetNetworkResponse
{
    QString m_friendbotUrl;
    QString m_passphrase;
    qint32 m_protocolVersion = 0;
public:
    static GetNetworkResponse fromJson(const QJsonObject& result);
    QString getFriendbotUrl() const { return m_friendbotUrl; }
    QString getPassphrase() const { return m_passphrase; }
    qint32 getProtocolVersion() const { return m_protocolVersion; }
};

#endif // GETNETWORKRESPONSE_H
