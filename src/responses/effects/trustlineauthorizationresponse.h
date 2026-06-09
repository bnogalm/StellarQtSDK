#ifndef TRUSTLINEAUTHORIZATIONRESPONSE_H
#define TRUSTLINEAUTHORIZATIONRESPONSE_H
#include "effectresponse.h"
#include "qstellar_namespace.h"


QSTELLAR_BEGIN_NS

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

QSTELLAR_END_NS

QSTELLAR_ALIAS(TrustlineAuthorizationResponse)



Q_DECLARE_METATYPE(qstellar::TrustlineAuthorizationResponse*)
#endif // TRUSTLINEAUTHORIZATIONRESPONSE_H


