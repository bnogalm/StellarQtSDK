#ifndef OFFERRESPONSE_H
#define OFFERRESPONSE_H

#include <QObject>
#include "response.h"
#include "link.h"


namespace OfferResponseAttach {
/**
     * Links connected to ledger.
     */
class Links {
    Q_GADGET
    Q_PROPERTY(Link self MEMBER m_self)
    Q_PROPERTY(Link offer_maker MEMBER m_offerMaker)
    Link m_self;
    Link m_offerMaker;
public:
    Link& getSelf(){
        return m_self;
    }

    Link& getOfferMager(){
        return m_offerMaker;
    }
    bool operator !=(Links& links);
};
}
class KeyPair;
class Asset;
/**
 * Represents offer response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/offer.html" target="_blank">Offer documentation</a>
 * @see org.stellar.sdk.requests.OffersRequestBuilder
 * @see org.stellar.sdk.Server#offers()
 */
class OfferResponse : public Response
{
    Q_OBJECT
    Q_PROPERTY(qint64 id MEMBER m_id)
    Q_PROPERTY(QString paging_token MEMBER m_pagingToken)
    Q_PROPERTY(QString seller READ seller WRITE setSeller)
    Q_PROPERTY(QVariantMap selling READ selling WRITE setSelling)
    Q_PROPERTY(QVariantMap buying READ buying WRITE setBuying)
    Q_PROPERTY(QString amount MEMBER m_amount)
    Q_PROPERTY(QString price MEMBER m_price)
    Q_PROPERTY(qint32 last_modified_ledger MEMBER m_lastModifiedLedger)
    Q_PROPERTY(QString last_modified_time MEMBER m_lastModifiedTime)
    Q_PROPERTY(OfferResponseAttach::Links _links MEMBER m_links)
    Q_PROPERTY(QString sponsor MEMBER m_sponsor READ getSponsor)

    qint64 m_id;
    QString m_pagingToken;
    QString m_seller;
    KeyPair *m_sellerKeypair;
    QVariantMap m_selling;
    Asset* m_sellerAsset;
    QVariantMap m_buying;
    Asset* m_buyingAsset;
    QString m_amount;
    QString m_price;
    qint32 m_lastModifiedLedger;
    QString m_lastModifiedTime;
    OfferResponseAttach::Links m_links;
    QString m_sponsor;

public:
    Q_INVOKABLE explicit OfferResponse(QNetworkReply* reply=nullptr);
    virtual ~OfferResponse();
    qint64 getId() const;
    QString getPagingToken() const;
    KeyPair* getSeller();
    Asset* getSelling();
    Asset* getBuying();
    QString getAmount();
    QString getPrice() const;
    qint32 getLastModifiedLedger() const;
    // Can be null if ledger adding an offer has not been ingested yet.
    QString getLastModifiedTime() const;
    OfferResponseAttach::Links getLinks() const;
    QString seller() const;
    QVariantMap buying() const;
    QVariantMap selling() const;
    QString getSponsor() const;

public slots:
    void setSeller(QString seller);
    void setBuying(QVariantMap buying);
    void setSelling(QVariantMap selling);
};
Q_DECLARE_METATYPE(OfferResponseAttach::Links)
#endif // OFFERRESPONSE_H
