#ifndef CREATEACCOUNTOPERATIONRESPONSE_H
#define CREATEACCOUNTOPERATIONRESPONSE_H

#include <QObject>
#include "operationresponse.h"
/**
 * Represents CreateAccount operation response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/operation.html" target="_blank">Operation documentation</a>
 * @see org.stellar.sdk.requests.OperationsRequestBuilder
 * @see org.stellar.sdk.Server#operations()
 */
class CreateAccountOperationResponse : public OperationResponse
{
    Q_OBJECT
    Q_PROPERTY(QString account READ account WRITE setAccount)
    Q_PROPERTY(QString funder READ funder WRITE setFunder)
    Q_PROPERTY(QString starting_balance MEMBER m_startingBalance)
    QString m_account;
    QString m_funder;
    QString m_startingBalance;
    KeyPair * m_accountKeypair;
    KeyPair * m_funderKeypair;
public:
    CreateAccountOperationResponse(QNetworkReply* reply=nullptr);
    virtual ~CreateAccountOperationResponse();
    KeyPair& getAccount();

    QString getStartingBalance() const;

    KeyPair& getFunder();
    QString account() const;
    QString funder() const;

public slots:
    void setAccount(QString account);
    void setFunder(QString funder);
};
Q_DECLARE_METATYPE(CreateAccountOperationResponse*)
#endif // CREATEACCOUNTOPERATIONRESPONSE_H



