#ifndef SETTRUSTLINEFLAGSOPERATIONRESPONSE_H
#define SETTRUSTLINEFLAGSOPERATIONRESPONSE_H

#include "operationresponse.h"


class Asset;
/**
 * Represents a Set Trustine Flags operation response.
 *
 * @see org.stellar.sdk.requests.OperationsRequestBuilder
 * @see org.stellar.sdk.Server#operations()
 */
class SetTrustLineFlagsOperationResponse : public OperationResponse
{
    Q_OBJECT

    Q_PROPERTY(QString asset_type READ getAssetType WRITE setAssetType NOTIFY assetTypeChanged)
    Q_PROPERTY(QString asset_code READ getAssetCode WRITE setAssetCode NOTIFY assetCodeChanged)
    Q_PROPERTY(QString asset_issuer READ getAssetIssuer WRITE setAssetIssuer NOTIFY assetIssuerChanged)


    Q_PROPERTY(QList<qint32> clear_flags READ getClearFlags WRITE setClearFlags NOTIFY clearFlagsChanged)
    Q_PROPERTY(QStringList clear_flags_s READ getClearFlagStrings WRITE setClearFlagStrings NOTIFY clearFlagStringsChanged)
    Q_PROPERTY(QList<qint32> set_flags READ getSetFlags WRITE setSetFlags NOTIFY setFlagsChanged)
    Q_PROPERTY(QStringList set_flags_s READ getSetFlagStrings WRITE setSetFlagStrings NOTIFY setFlagStringsChanged)

    Q_PROPERTY(QString trustor READ getTrustor WRITE setTrustor NOTIFY trustorChanged)


    QString m_assetType;

    QString m_assetCode;

    QString m_assetIssuer;

    QList<qint32> m_clearFlags;

    QList<QString> m_clearFlagsS;

    QList<qint32> m_setFlags;

    QList<QString> m_setFlagsS;

    QString m_trustor;

    Asset* m_asset;
public:
    SetTrustLineFlagsOperationResponse(QNetworkReply* reply=nullptr);
    virtual ~SetTrustLineFlagsOperationResponse();

    const QString getAssetType() const;
    void setAssetType(const QString newAsset_type);

    const QString getAssetCode() const;
    void setAssetCode(const QString newAsset_code);

    const QString getAssetIssuer() const;
    void setAssetIssuer(const QString newAsset_issuer);

    const QList<qint32> getClearFlags() const;
    void setClearFlags(const QList<qint32> newClear_flags);

    const QStringList getClearFlagStrings() const;
    void setClearFlagStrings(const QStringList newClear_flags_s);

    const QList<qint32> getSetFlags() const;
    void setSetFlags(const QList<qint32> newSet_flags);

    const QStringList getSetFlagStrings() const;
    void setSetFlagStrings(const QStringList newSet_flags_s);

    const QString getTrustor() const;
    void setTrustor(const QString newTrustor);

    Asset* getAsset();
signals:
    void assetTypeChanged();
    void assetCodeChanged();
    void assetIssuerChanged();
    void clearFlagsChanged();
    void clearFlagStringsChanged();
    void setFlagsChanged();
    void setFlagStringsChanged();
    void trustorChanged();
};

#endif // SETTRUSTLINEFLAGSOPERATIONRESPONSE_H
