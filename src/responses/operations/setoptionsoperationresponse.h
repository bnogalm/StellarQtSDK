#ifndef SETOPTIONSOPERATIONRESPONSE_H
#define SETOPTIONSOPERATIONRESPONSE_H

#include <QObject>
#include "operationresponse.h"
#include "qstellar_namespace.h"
QSTELLAR_BEGIN_NS

/**
 * Represents SetOptions operation response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/operation.html" target="_blank">Operation documentation</a>
 * @see org.stellar.sdk.requests.OperationsRequestBuilder
 * @see org.stellar.sdk.Server#operations()
 */
class SetOptionsOperationResponse : public OperationResponse
{
    Q_OBJECT
    // Horizon leaves out every field the operation does not change: each of
    // these says whether it was there (hasLowThreshold()...). They were never
    // initialised, and an absent one read as whatever was in memory.
    Q_PROPERTY(quint32 low_threshold READ getLowThreshold WRITE setLowThreshold)
    Q_PROPERTY(quint32 med_threshold READ getMedThreshold WRITE setMedThreshold)
    Q_PROPERTY(quint32 high_threshold READ getHighThreshold WRITE setHighThreshold)
    Q_PROPERTY(QString inflation_dest READ inflationDest WRITE setInflationDest)

    Q_PROPERTY(QString home_domain MEMBER m_homeDomain)
    Q_PROPERTY(QString signer_key READ signerKey WRITE setSignerKey)
    Q_PROPERTY(quint32 signer_weight READ getSignerWeight WRITE setSignerWeight)
    Q_PROPERTY(quint32 master_key_weight READ getMasterKeyWeight WRITE setMasterKeyWeight)
    Q_PROPERTY(QStringList clear_flags_s MEMBER m_clearFlagsS)
    Q_PROPERTY(QStringList set_flags_s MEMBER m_setFlagsS)
    Q_PROPERTY(QList<qint32> clear_flags MEMBER m_clearFlags)
    Q_PROPERTY(QList<qint32> set_flags MEMBER m_setFlags)

    quint32 m_lowThreshold = 0;
    quint32 m_medThreshold = 0;
    quint32 m_highThreshold = 0;
    enum Present { LowThreshold = 1, MedThreshold = 2, HighThreshold = 4, SignerWeight = 8, MasterKeyWeight = 16 };
    int m_present = 0;

    QString m_inflationDest;
    KeyPair *m_inflationDestKeypair;

    QString m_homeDomain;
    QString m_signerKey;
    KeyPair *m_signerKeypair;

    quint32 m_signerWeight = 0;
    quint32 m_masterKeyWeight = 0;

    QStringList m_clearFlagsS;
    QStringList m_setFlagsS;
    QList<qint32> m_clearFlags;
    QList<qint32> m_setFlags;


public:
    SetOptionsOperationResponse(QNetworkReply* reply=nullptr);

    virtual ~SetOptionsOperationResponse();

    /** 0 when the operation did not set it: see hasLowThreshold(). */
    quint32 getLowThreshold() const;
    quint32 getMedThreshold() const;
    quint32 getHighThreshold() const;
    /**
     * Whether the operation sets the field. Horizon leaves unchanged fields
     * out, and 0 is a real value (a master weight of 0 retires the master
     * key, a signer weight of 0 removes the signer), so "not set" is told
     * apart this way, as js (undefined) and py (no key) do.
     */
    bool hasLowThreshold() const { return m_present & LowThreshold; }
    bool hasMedThreshold() const { return m_present & MedThreshold; }
    bool hasHighThreshold() const { return m_present & HighThreshold; }
    bool hasSignerWeight() const { return m_present & SignerWeight; }
    bool hasMasterKeyWeight() const { return m_present & MasterKeyWeight; }
    KeyPair* getInflationDestination();
    QString getHomeDomain() const;
    //deprecated, use getSignerKey
    Q_DECL_DEPRECATED KeyPair* getSigner();
    QString getSignerKey() const;
    quint32 getSignerWeight() const;
    quint32 getMasterKeyWeight() const;
    QStringList getClearFlags() const;
    QStringList getSetFlags() const;
    /** Horizon's clear_flags: the flag values (1, 2, 4, 8) the operation clears; js record.clear_flags. */
    QList<qint32> getClearFlagValues() const { return m_clearFlags; }
    /** Horizon's set_flags: the flag values the operation sets; js record.set_flags. */
    QList<qint32> getSetFlagValues() const { return m_setFlags; }
    QString inflationDest() const;
    QString signerKey() const;

public slots:
    void setInflationDest(QString inflationDest);
    void setSignerKey(QString signerKey);
    void setLowThreshold(quint32 v) { m_lowThreshold = v; m_present |= LowThreshold; }
    void setMedThreshold(quint32 v) { m_medThreshold = v; m_present |= MedThreshold; }
    void setHighThreshold(quint32 v) { m_highThreshold = v; m_present |= HighThreshold; }
    void setSignerWeight(quint32 v) { m_signerWeight = v; m_present |= SignerWeight; }
    void setMasterKeyWeight(quint32 v) { m_masterKeyWeight = v; m_present |= MasterKeyWeight; }
protected:
    // Response::reset() (between stream events) writes a default through
    // every setter: none of them was sent.
    void reset() override { OperationResponse::reset(); m_present = 0; }
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(SetOptionsOperationResponse)



Q_DECLARE_METATYPE(qstellar::SetOptionsOperationResponse*)
#endif // SETOPTIONSOPERATIONRESPONSE_H





