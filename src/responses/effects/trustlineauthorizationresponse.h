#ifndef TRUSTLINEAUTHORIZATIONRESPONSE_H
#define TRUSTLINEAUTHORIZATIONRESPONSE_H
#include "effectresponse.h"


class TrustlineAuthorizationResponse : public EffectResponse
{
    Q_OBJECT
    Q_PROPERTY(QString trustor READ trustor WRITE setTrustor)
    Q_PROPERTY(QString asset_type MEMBER m_assetType)
    Q_PROPERTY(QString asset_code MEMBER m_assetCode)

    QString m_trustor;
    KeyPair *m_trustorKeypair;
    QString m_assetType;
    QString m_assetCode;

public:
    TrustlineAuthorizationResponse(QNetworkReply *reply =nullptr);
    virtual ~TrustlineAuthorizationResponse();
    KeyPair& getTrustor();

    QString getAssetType();

    QString getAssetCode();
    QString trustor() const;
public slots:
    void setTrustor(QString trustor);
};
Q_DECLARE_METATYPE(TrustlineAuthorizationResponse*)
#endif // TRUSTLINEAUTHORIZATIONRESPONSE_H


