#ifndef CHANGETRUSTOPERATIONRESPONSE_H
#define CHANGETRUSTOPERATIONRESPONSE_H

#include <QObject>
#include "operationresponse.h"


class Asset;


/**
 * Represents ChangeTrust operation response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/operation.html" target="_blank">Operation documentation</a>
 * @see org.stellar.sdk.requests.OperationsRequestBuilder
 * @see org.stellar.sdk.Server#operations()
 */
class ChangeTrustOperationResponse : public OperationResponse
{
    Q_OBJECT
    Q_PROPERTY(QString trustor READ trustor WRITE setTrustor)
    Q_PROPERTY(QString trustee READ trustee WRITE setTrustee)
    Q_PROPERTY(QString asset_type READ assetType WRITE setAssetType)
    Q_PROPERTY(QString asset_code READ assetCode WRITE setAssetCode)
    Q_PROPERTY(QString asset_issuer READ assetIssuer WRITE setAssetIssuer)
    Q_PROPERTY(QString limit MEMBER m_limit)

    QString m_trustor;
    QString m_trustee;
    QString m_assetType;
    QString m_assetCode;
    QString m_assetIssuer;
    KeyPair* m_trustorKeypair;
    KeyPair* m_trusteeKeypair;
    Asset *m_asset;
    QString m_limit;

public:
    ChangeTrustOperationResponse(QNetworkReply* reply=nullptr);
    virtual ~ChangeTrustOperationResponse();
    KeyPair& getTrustor();
    KeyPair& getTrustee();
    Asset* getAsset();
    QString trustor() const;
    QString trustee() const;
    QString assetType() const;
    QString assetCode() const;
    QString assetIssuer() const;
    QString getLimit() const;
public slots:
    void setTrustor(QString trustor);
    void setTrustee(QString trustee);
    void setAssetType(QString assetType);
    void setAssetCode(QString assetCode);
    void setAssetIssuer(QString assetIssuer);
};
Q_DECLARE_METATYPE(ChangeTrustOperationResponse*)
#endif // CHANGETRUSTOPERATIONRESPONSE_H


