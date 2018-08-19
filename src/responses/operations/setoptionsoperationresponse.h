#ifndef SETOPTIONSOPERATIONRESPONSE_H
#define SETOPTIONSOPERATIONRESPONSE_H

#include <QObject>
#include "operationresponse.h"
/**
 * Represents SetOptions operation response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/operation.html" target="_blank">Operation documentation</a>
 * @see org.stellar.sdk.requests.OperationsRequestBuilder
 * @see org.stellar.sdk.Server#operations()
 */
class SetOptionsOperationResponse : public OperationResponse
{
    Q_OBJECT
    Q_PROPERTY(quint32 low_threshold MEMBER m_lowThreshold)
    Q_PROPERTY(quint32 med_threshold MEMBER m_medThreshold)
    Q_PROPERTY(quint32 high_threshold MEMBER m_highThreshold)
    Q_PROPERTY(QString inflation_dest READ inflationDest WRITE setInflationDest)

    Q_PROPERTY(QString home_domain MEMBER m_homeDomain)
    Q_PROPERTY(QString signer_key READ signerKey WRITE setSignerKey)
    Q_PROPERTY(quint32 signer_weight MEMBER m_signerWeight)
    Q_PROPERTY(quint32 master_key_weight MEMBER m_masterKeyWeight)
    Q_PROPERTY(QStringList clear_flags_s MEMBER m_clearFlagsS)
    Q_PROPERTY(QStringList set_flags_s MEMBER m_setFlagsS)

    quint32 m_lowThreshold;
    quint32 m_medThreshold;
    quint32 m_highThreshold;

    QString m_inflationDest;
    KeyPair *m_inflationDestKeypair;

    QString m_homeDomain;
    QString m_signerKey;
    KeyPair *m_signerKeypair;

    quint32 m_signerWeight;
    quint32 m_masterKeyWeight;

    QStringList m_clearFlagsS;
    QStringList m_setFlagsS;


public:
    SetOptionsOperationResponse(QNetworkReply* reply=nullptr);

    virtual ~SetOptionsOperationResponse();

    quint32 getLowThreshold() const;
    quint32 getMedThreshold() const;
    quint32 getHighThreshold() const;
    KeyPair* getInflationDestination();
    QString getHomeDomain() const;
    //deprecated, use getSignerKey
    Q_DECL_DEPRECATED KeyPair* getSigner();
    QString getSignerKey() const;
    quint32 getSignerWeight() const;
    quint32 getMasterKeyWeight() const;
    QStringList getClearFlags() const;
    QStringList getSetFlags() const;
    QString inflationDest() const;
    QString signerKey() const;

public slots:
    void setInflationDest(QString inflationDest);
    void setSignerKey(QString signerKey);
};
Q_DECLARE_METATYPE(SetOptionsOperationResponse*)
#endif // SETOPTIONSOPERATIONRESPONSE_H





