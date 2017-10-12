#ifndef FEDERATIONRESPONSE_H
#define FEDERATIONRESPONSE_H

#include <QObject>
#include "../responses/response.h"

/**
 * Object to hold a response from a federation server.
 * @see <a href="https://www.stellar.org/developers/learn/concepts/federation.html" target="_blank">Federation docs</a>
 */
class FederationResponse : public Response
{
    Q_OBJECT

    Q_PROPERTY(QString stellar_address READ getStellarAddress MEMBER m_stellarAddress)
    Q_PROPERTY(QString account_id READ getAccountId MEMBER m_accountId)
    Q_PROPERTY(QString memo_type READ getMemoType MEMBER m_memoType)//text id hash return?
    Q_PROPERTY(QString memo READ getMemo MEMBER m_memo)

    QString m_stellarAddress;
    QString m_accountId;
    QString m_memoType;
    QString m_memo;

public:

    FederationResponse(QNetworkReply* reply=0);
    FederationResponse(QString stellarAddress, QString accountId, QString memoType, QString memo);
    QString getStellarAddress();

    QString getAccountId();

    /**
     * @return Memo type or null when no memo attached
     */
    QString getMemoType();

    /**
     * @return Memo value or null when no memo attached
     */
    QString getMemo();
};

#endif // FEDERATIONRESPONSE_H


