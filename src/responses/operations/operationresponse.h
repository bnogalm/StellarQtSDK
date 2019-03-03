#ifndef OPERATIONRESPONSE_H
#define OPERATIONRESPONSE_H

#include <QObject>
#include "../response.h"
#include "../link.h"
#include "../../util.h"


namespace OperationResponseAttach
{
  /**
   * Represents operation links.
   */
  class Links {
      Q_GADGET
      Q_PROPERTY(Link effects MEMBER m_effects)
      Q_PROPERTY(Link precedes MEMBER m_precedes)
      Q_PROPERTY(Link self MEMBER m_self)
      Q_PROPERTY(Link succeeds MEMBER m_succeeds)
      Q_PROPERTY(Link transaction MEMBER m_transaction)
      Link m_effects;
      Link m_precedes;
      Link m_self;
      Link m_succeeds;
      Link m_transaction;
  public:

    Link& getEffects() {
      return m_effects;
    }

    Link& getPrecedes() {
      return m_precedes;
    }

    Link& getSelf() {
      return m_self;
    }

    Link& getSucceeds() {
      return m_succeeds;
    }

    Link& getTransaction() {
      return m_transaction;
    }
    bool operator !=(Links& links)
    {
        Q_UNUSED(links)
        return true;
    }
  };
}

class KeyPair;
/**
 * Abstract class for operation responses.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/operation.html" target="_blank">Operation documentation</a>
 * @see org.stellar.sdk.requests.OperationsRequestBuilder
 * @see org.stellar.sdk.Server#operations()
 */
class OperationResponse : public Response
{
    Q_OBJECT
    Q_PROPERTY(qint64 id MEMBER m_id)
    Q_PROPERTY(QString source_account READ sourceAccount WRITE setSourceAccount)
    Q_PROPERTY(QString paging_token MEMBER m_pagingToken)
    Q_PROPERTY(QString created_at MEMBER m_createdAt)
    Q_PROPERTY(QString transaction_hash MEMBER m_transactionHash)
    Q_PROPERTY(QVariant transaction_successful MEMBER m_transactionSuccessful WRITE setTransactionSuccessful)//we use a custom set method to filter non Bool types
    Q_PROPERTY(QString type MEMBER m_type)
    Q_PROPERTY(OperationResponseAttach::Links _links MEMBER m_links)
    qint64 m_id;
    QString m_sourceAccount;
    KeyPair *m_sourceAccountKeypair;
    QString m_pagingToken;
    QString m_createdAt;
    QString m_transactionHash;
    QVariant m_transactionSuccessful;
    QString m_type;
    OperationResponseAttach::Links m_links;


public:
    explicit OperationResponse(QNetworkReply* reply=nullptr);
    virtual ~OperationResponse();
      qint64 getId() const;

      KeyPair& getSourceAccount();

      QString getPagingToken() const;
      QString getCreatedAt() const;
      /**
      * Returns transaction hash of transaction this operation belongs to.
      */
      QString getTransactionHash() const;
      /**
       * <p>Returns operation type. Possible types:</p>
       * <ul>
       *   <li>create_account</li>
       *   <li>payment</li>
       *   <li>allow_trust</li>
       *   <li>change_trust</li>
       *   <li>set_options</li>
       *   <li>account_merge</li>
       *   <li>manage_offer</li>
       *   <li>path_payment</li>
       *   <li>create_passive_offer</li>
       *   <li>inflation</li>
       *   <li>manage_data</li>
       * </ul>
       */
      QString getType() const;

      Boolean isTransactionSuccessful() const;

      OperationResponseAttach::Links& getLinks();

      QString sourceAccount() const;
public slots:
      void setSourceAccount(QString sourceAccount);
      void setTransactionSuccessful(QVariant transactionSuccessful);
};
Q_DECLARE_METATYPE(OperationResponseAttach::Links)
Q_DECLARE_METATYPE(OperationResponse*)
#endif // OPERATIONRESPONSE_H

