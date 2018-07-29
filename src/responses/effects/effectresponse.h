#ifndef EFFECTRESPONSE_H
#define EFFECTRESPONSE_H

#include <QObject>
#include "../response.h"
#include "../link.h"

namespace EffectResponseAttach
{

/**
 * Represents effect links.
 */
class Links {
    Q_GADGET
    Q_PROPERTY(Link operation MEMBER m_operation)
    Q_PROPERTY(Link precedes MEMBER m_precedes)
    Q_PROPERTY(Link succeeds MEMBER m_succeeds)
    Link m_operation;
    Link m_precedes;
    Link m_succeeds;
public:
    Link& getOperation() {
        return m_operation;
    }

    Link& getPrecedes() {
        return m_precedes;
    }

    Link& getSucceeds() {
        return m_succeeds;
    }
    bool operator !=(Links& links);
};
}


class KeyPair;
/**
 * Abstract class for effect responses.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
 * @see org.stellar.sdk.requests.EffectsRequestBuilder
 * @see org.stellar.sdk.Server#effects()
 */
class EffectResponse : public Response
{
    Q_OBJECT
    Q_PROPERTY(QString id MEMBER m_id)
    Q_PROPERTY(QString account READ account WRITE setAccount)
    Q_PROPERTY(QString type MEMBER m_type)
    Q_PROPERTY(QString created_at MEMBER m_createdAt)
    Q_PROPERTY(QString paging_token MEMBER m_pagingToken)
    Q_PROPERTY(EffectResponseAttach::Links _links MEMBER m_links)

    QString m_id;
    QString m_account;
    KeyPair *m_accountKeypair;
    QString m_type;
    QString m_createdAt;
    QString m_pagingToken;
    EffectResponseAttach::Links m_links;
public:
    explicit EffectResponse(QNetworkReply *reply = nullptr);
    virtual ~EffectResponse();

    QString getId();

    KeyPair& getAccount();

    /**
     * <p>Returns effect type. Possible types:</p>
     * <ul>
     *   <li>account_created</li>
     *   <li>account_removed</li>
     *   <li>account_credited</li>
     *   <li>account_debited</li>
     *   <li>account_thresholds_updated</li>
     *   <li>account_home_domain_updated</li>
     *   <li>account_flags_updated</li>
     *   <li>account_inflation_destination_updated</li>
     *   <li>signer_created</li>
     *   <li>signer_removed</li>
     *   <li>signer_updated</li>
     *   <li>trustline_created</li>
     *   <li>trustline_removed</li>
     *   <li>trustline_updated</li>
     *   <li>trustline_authorized</li>
     *   <li>trustline_deauthorized</li>
     *   <li>offer_created</li>
     *   <li>offer_removed</li>
     *   <li>offer_updated</li>
     *   <li>trade</li>
     *   <li>data_created</li>
     *   <li>data_removed</li>
     *   <li>data_updated</li>
     * </ul>
     */
    QString getType();

    QString getCreatedAt();

    QString getPagingToken();

    EffectResponseAttach::Links& getLinks();

    QString account() const;
public slots:
    void setAccount(QString account);
};
Q_DECLARE_METATYPE(EffectResponseAttach::Links)
Q_DECLARE_METATYPE(EffectResponse*)
#endif // EFFECTRESPONSE_H
