#ifndef ALLOWTRUSTOPERATIONRESPONSE_H
#define ALLOWTRUSTOPERATIONRESPONSE_H

#include <QObject>
#include "operationresponse.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

class Asset;/**
 * @deprecated As of release 0.24.0, replaced by {@link SetTrustLineFlagsOperationResponse}
 *
 * Represents AllowTrust operation response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/operation.html" target="_blank">Operation documentation</a>
 * @see org.stellar.sdk.requests.OperationsRequestBuilder
 * @see org.stellar.sdk.Server#operations()
 */
class Q_DECL_DEPRECATED AllowTrustOperationResponse : public OperationResponse
{
    Q_OBJECT
    Q_PROPERTY(QString trustor READ trustor WRITE setTrustor)
    Q_PROPERTY(QString trustee READ trustee WRITE setTrustee)
    Q_PROPERTY(QString asset_type READ assetType WRITE setAssetType)
    Q_PROPERTY(QString asset_code READ assetCode WRITE setAssetCode)
    Q_PROPERTY(QString asset_issuer READ assetIssuer WRITE setAssetIssuer)
    Q_PROPERTY(bool authorize MEMBER m_authorize)
    Q_PROPERTY(bool authorize_to_maintain_liabilities MEMBER m_authorizeToMaintainLiabilities)
    QString m_trustor;
    QString m_trustee;
    QString m_assetType;
    QString m_assetCode;
    QString m_assetIssuer;
    KeyPair* m_trustorKeypair;
    KeyPair* m_trusteeKeypair;
    Asset *m_asset;
    bool m_authorize;
    bool m_authorizeToMaintainLiabilities;
public:
    AllowTrustOperationResponse(QNetworkReply* reply=nullptr);
    virtual ~AllowTrustOperationResponse();
    KeyPair& getTrustor();
    KeyPair& getTrustee();
    Asset* getAsset();
    QString trustor() const;
    QString trustee() const;
    QString assetType() const;
    QString assetCode() const;
    QString assetIssuer() const;
    bool isAuthorize() const;
    bool isAuthorizedToMaintainLiabilities() const;
public slots:
    void setTrustor(QString trustor);
    void setTrustee(QString trustee);
    void setAssetType(QString assetType);
    void setAssetCode(QString assetCode);
    void setAssetIssuer(QString assetIssuer);
};

QSTELLAR_END_NS

// AllowTrustOperationResponse is intentionally deprecated; the legacy alias and
// the metatype registration are the SDK's own required uses of the type, so they
// are silenced here without hiding the deprecation from consumers.
#if defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable: 4996)
#elif defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
QSTELLAR_ALIAS(AllowTrustOperationResponse)



Q_DECLARE_METATYPE(qstellar::AllowTrustOperationResponse*)
#if defined(_MSC_VER)
#  pragma warning(pop)
#elif defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic pop
#endif
#endif // ALLOWTRUSTOPERATIONRESPONSE_H
