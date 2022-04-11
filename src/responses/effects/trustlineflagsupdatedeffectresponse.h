#ifndef TRUSTLINEFLAGSUPDATEDEFFECTRESPONSE_H
#define TRUSTLINEFLAGSUPDATEDEFFECTRESPONSE_H

#include "effectresponse.h"


class Asset;
/**
 * Represents trustline_flags_updated effect response.
 *
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
 * @see org.stellar.sdk.requests.EffectsRequestBuilder
 * @see org.stellar.sdk.Server#effects()
 */
class TrustlineFlagsUpdatedEffectResponse : public EffectResponse
{
    Q_OBJECT

    Q_PROPERTY(QString trustor READ getTrustor WRITE setTrustor NOTIFY trustorChanged)
    Q_PROPERTY(QString asset_type READ getAssetType WRITE setAssetType NOTIFY assetTypeChanged)
    Q_PROPERTY(QString asset_code READ getAssetCode WRITE setAssetCode NOTIFY assetCodeChanged)
    Q_PROPERTY(QString asset_issuer READ getAssetIssuer WRITE setAssetIssuer NOTIFY assetIssuerChanged)

    Q_PROPERTY(bool authorized_flag READ getAuthorized WRITE setAuthorized NOTIFY authorizedChanged)
    Q_PROPERTY(bool authorized_to_maintain_liabilities_flag READ getAuthorizedToMaintainLiabilities WRITE setAuthorizedToMaintainLiabilities NOTIFY authorizedToMaintainLiabilitiesChanged)
    Q_PROPERTY(bool clawback_enabled_flag READ getClawbackEnabled WRITE setClawbackEnabled NOTIFY clawbackEnabledChanged)


    QString m_trustor;

    QString m_assetType;

    QString m_assetCode;

    QString m_assetIssuer;

    bool m_authorizedFlag;

    bool m_authorizedToMaintainLiabilitiesFlag;

    bool m_clawbackEnabledFlag;


    Asset* m_asset;
public:
    TrustlineFlagsUpdatedEffectResponse(QNetworkReply *reply=nullptr);
    virtual ~TrustlineFlagsUpdatedEffectResponse();


    Asset* getAsset();
    const QString getTrustor() const;
    void setTrustor(const QString newTrustor);
    const QString getAssetType() const;
    void setAssetType(const QString newAsset_type);

    const QString getAssetCode() const;
    void setAssetCode(const QString newAsset_code);

    const QString getAssetIssuer() const;
    void setAssetIssuer(const QString newAsset_issuer);

    bool getAuthorized() const;
    void setAuthorized(bool newAuthorized_flag);

    bool getAuthorizedToMaintainLiabilities() const;
    void setAuthorizedToMaintainLiabilities(bool newAuthorized_to_maintain_liabilities_flag);

    bool getClawbackEnabled() const;
    void setClawbackEnabled(bool newClawback_enabled_flag);

signals:
    void trustorChanged();
    void assetTypeChanged();
    void assetCodeChanged();
    void assetIssuerChanged();
    void authorizedChanged();
    void authorizedToMaintainLiabilitiesChanged();
    void clawbackEnabledChanged();
};

#endif // TRUSTLINEFLAGSUPDATEDEFFECTRESPONSE_H
