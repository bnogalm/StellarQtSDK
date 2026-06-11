#ifndef CREATEACCOUNTOPERATIONRESPONSE_H
#define CREATEACCOUNTOPERATIONRESPONSE_H

#include <QObject>
#include "operationresponse.h"
#include "qstellar_namespace.h"
QSTELLAR_BEGIN_NS

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
    Q_PROPERTY(QString funder_muxed MEMBER m_funderMuxed)
    Q_PROPERTY(QString funder_muxed_id MEMBER m_funderMuxedId)
    Q_PROPERTY(QString starting_balance MEMBER m_startingBalance)
    QString m_account;
    QString m_funder;
    QString m_startingBalance;
    KeyPair * m_accountKeypair;
    KeyPair * m_funderKeypair;
    QString m_funderMuxed;
    QString m_funderMuxedId;
public:
    CreateAccountOperationResponse(QNetworkReply* reply=nullptr);
    virtual ~CreateAccountOperationResponse();
    KeyPair& getAccount();

    QString getStartingBalance() const;

    KeyPair& getFunder();
    QString account() const;
    QString funder() const;
    QString getFunderMuxed() const { return m_funderMuxed; }
    QString getFunderMuxedId() const { return m_funderMuxedId; }

public slots:
    void setAccount(QString account);
    void setFunder(QString funder);
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(CreateAccountOperationResponse)



Q_DECLARE_METATYPE(qstellar::CreateAccountOperationResponse*)
#endif // CREATEACCOUNTOPERATIONRESPONSE_H



