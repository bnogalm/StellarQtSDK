#ifndef PATHRESPONSE_H
#define PATHRESPONSE_H

#include <QObject>
#include "response.h"
#include "link.h"
namespace PathResponseAttach
{
/**
 * Links connected to path.
 */
class Links {
    Q_GADGET
    Q_PROPERTY(Link self MEMBER m_self)
    Link m_self;

public:

    Link& getSelf() {
        return m_self;
    }
    bool operator !=(Links& links)
    {
        Q_UNUSED(links)
        return true;
    }
};


}
class Asset;
/**
 * Represents path response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/path.html" target="_blank">Path documentation</a>
 * @see org.stellar.sdk.requests.PathsRequestBuilder
 * @see org.stellar.sdk.Server#paths()
 */
class PathResponse : public Response
{
    Q_OBJECT

    Q_PROPERTY(QString destination_amount MEMBER m_destinationAmount)
    Q_PROPERTY(QString destination_asset_type READ destinationAssetType WRITE setDestinationAssetType)
    Q_PROPERTY(QString destination_asset_code READ destinationAssetCode WRITE setDestinationAssetCode)
    Q_PROPERTY(QString destination_asset_issuer READ destinationAssetIssuer WRITE setDestinationAssetIssuer)

    Q_PROPERTY(QString source_amount MEMBER m_sourceAmount)
    Q_PROPERTY(QString source_asset_type READ sourceAssetType WRITE setSourceAssetType)
    Q_PROPERTY(QString source_asset_code READ sourceAssetCode WRITE setSourceAssetCode)
    Q_PROPERTY(QString source_asset_issuer READ sourceAssetIssuer WRITE setSourceAssetIssuer)
    Q_PROPERTY(QVariantList path MEMBER m_path WRITE setPath)
    Q_PROPERTY(PathResponseAttach::Links _links MEMBER m_links)


    QString m_destinationAmount;
    QString m_destinationAssetType;
    QString m_destinationAssetCode;
    QString m_destinationAssetIssuer;
    Asset* m_destinationAsset;

    QString m_sourceAmount;
    QString m_sourceAssetType;
    QString m_sourceAssetCode;
    QString m_sourceAssetIssuer;

    Asset* m_sourceAsset;

    //QList<PathResponseAttach::Asset> m_path;
    QVariantList m_path;
    QList<Asset*> m_pathConverted;
    PathResponseAttach::Links m_links;

public:
    Q_INVOKABLE explicit PathResponse(QNetworkReply *reply = nullptr);
    virtual ~PathResponse();

    QString getDestinationAmount();
    QString getSourceAmount();
    const QList<Asset*>& getPath();
    Asset& getDestinationAsset();
    Asset& getSourceAsset();
    PathResponseAttach::Links& getLinks();
    QString destinationAssetType() const;
    QString destinationAssetCode() const;
    QString destinationAssetIssuer() const;

    QString sourceAssetType() const;
    QString sourceAssetCode() const;
    QString sourceAssetIssuer() const;

signals:

public slots:
    void setDestinationAssetType(QString destinationAssetType);
    void setDestinationAssetCode(QString destinationAssetCode);
    void setDestinationAssetIssuer(QString destinationAssetIssuer);
    void setSourceAssetType(QString sourceAssetType);
    void setSourceAssetCode(QString sourceAssetCode);
    void setSourceAssetIssuer(QString sourceAssetIssuer);

private slots:
    void setPath(QVariantList path);
};
Q_DECLARE_METATYPE(PathResponseAttach::Links)

#endif // PATHRESPONSE_H




