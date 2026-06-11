#ifndef ACCOUNTMERGEOPERATIONRESPONSE_H
#define ACCOUNTMERGEOPERATIONRESPONSE_H

#include <QObject>
#include "operationresponse.h"
#include "../../keypair.h"
#include "qstellar_namespace.h"
QSTELLAR_BEGIN_NS

/**
 * Represents AccountMerge operation response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/operation.html" target="_blank">Operation documentation</a>
 * @see org.stellar.sdk.requests.OperationsRequestBuilder
 * @see org.stellar.sdk.Server#operations()
 */
class AccountMergeOperationResponse : public OperationResponse
{
    Q_OBJECT
    Q_PROPERTY(QString account READ account WRITE setAccount)
    Q_PROPERTY(QString account_muxed MEMBER m_accountMuxed)
    Q_PROPERTY(QString account_muxed_id MEMBER m_accountMuxedId)
    Q_PROPERTY(QString into READ into WRITE setInto)
    Q_PROPERTY(QString into_muxed MEMBER m_intoMuxed)
    Q_PROPERTY(QString into_muxed_id MEMBER m_intoMuxedId)

public:
    AccountMergeOperationResponse(QNetworkReply *reply= nullptr);
    virtual ~AccountMergeOperationResponse();
    QString account() const;
    QString into() const;
    KeyPair &getAccount();
    KeyPair &getInto();
    QString getAccountMuxed() const { return m_accountMuxed; }
    QString getAccountMuxedId() const { return m_accountMuxedId; }
    QString getIntoMuxed() const { return m_intoMuxed; }
    QString getIntoMuxedId() const { return m_intoMuxedId; }


public slots:
    void setAccount(QString account);
    void setInto(QString into);

private:
    QString m_account;
    QString m_into;
    QString m_accountMuxed;
    QString m_accountMuxedId;
    QString m_intoMuxed;
    QString m_intoMuxedId;
    KeyPair *m_accountKeypair;
    KeyPair *m_intoKeypair;

};

QSTELLAR_END_NS

QSTELLAR_ALIAS(AccountMergeOperationResponse)



Q_DECLARE_METATYPE(qstellar::AccountMergeOperationResponse*)
#endif // ACCOUNTMERGEOPERATIONRESPONSE_Hs
