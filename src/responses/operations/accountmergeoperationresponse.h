#ifndef ACCOUNTMERGEOPERATIONRESPONSE_H
#define ACCOUNTMERGEOPERATIONRESPONSE_H

#include <QObject>
#include "operationresponse.h"
class KeyPair;
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
    Q_PROPERTY(QString into READ into WRITE setInto)

public:
    AccountMergeOperationResponse(QNetworkReply *reply= nullptr);
    virtual ~AccountMergeOperationResponse();
    QString account() const;
    QString into() const;
    KeyPair &getAccount();
    KeyPair &getInto();


public slots:
    void setAccount(QString account);
    void setInto(QString into);

private:
    QString m_account;
    QString m_into;
    KeyPair *m_accountKeypair;
    KeyPair *m_intoKeypair;

};
Q_DECLARE_METATYPE(AccountMergeOperationResponse*)
#endif // ACCOUNTMERGEOPERATIONRESPONSE_Hs
