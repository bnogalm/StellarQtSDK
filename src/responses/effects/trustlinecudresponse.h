#ifndef TRUSTLINECUDRESPONSE_H
#define TRUSTLINECUDRESPONSE_H

#include "effectresponse.h"
class Asset;
class TrustlineCUDResponse : public EffectResponse
{
    Q_OBJECT
    Q_PROPERTY(QString limit MEMBER m_limit)
    Q_PROPERTY(QString asset_type READ assetType WRITE setAssetType)
    Q_PROPERTY(QString asset_code READ assetCode WRITE setAssetCode)
    Q_PROPERTY(QString asset_issuer READ assetIssuer WRITE setAssetIssuer)

    QString m_limit;
    QString m_assetType;
    QString m_assetCode;
    QString m_assetIssuer;
    Asset* m_asset;

public:
    TrustlineCUDResponse(QNetworkReply *reply= nullptr);
    virtual ~TrustlineCUDResponse();
    QString getLimit() const;
    Asset* getAsset();
    QString assetType() const;
    QString assetCode() const;
    QString assetIssuer() const;

public slots:
    void setAssetType(QString assetType);
    void setAssetCode(QString assetCode);
    void setAssetIssuer(QString assetIssuer);
};
Q_DECLARE_METATYPE(TrustlineCUDResponse*)
#endif // TRUSTLINECUDRESPONSE_H
