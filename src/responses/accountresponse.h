#ifndef ACCOUNTRESPONSE_H
#define ACCOUNTRESPONSE_H
#include "response.h"
#include "../asset.h"
#include "../assettypenative.h"
#include "link.h"
#include <QMetaProperty>
#include <QVariantMap>
namespace AccountResponseAttach
{
/**
 * Represents account thresholds.
 */
class Thresholds {
    Q_GADGET
    Q_PROPERTY(int low_threshold MEMBER m_lowThreshold)
    Q_PROPERTY(int med_threshold MEMBER m_medThreshold)
    Q_PROPERTY(int high_threshold MEMBER m_highThreshold)

    int m_lowThreshold;
    int m_medThreshold;
    int m_highThreshold;
public:
    Thresholds();

    int getLowThreshold() const;
    int getMedThreshold() const;
    int getHighThreshold() const;
    bool operator !=(Thresholds& t);
    bool operator ==(Thresholds& t);
};

/**
 * Represents account flags.
 */
class Flags {
    Q_GADGET
    Q_PROPERTY(bool auth_required MEMBER m_authRequired)
    Q_PROPERTY(bool auth_revocable MEMBER m_authRevocable)
    Q_PROPERTY(bool auth_immutable MEMBER m_authImmutable)
    bool m_authRequired;
    bool m_authRevocable;
    bool m_authImmutable;
public:
    Flags();

    bool getAuthRequired() const;
    bool getAuthRevocable() const;
    bool getAuthImmutable() const;
    bool operator !=(Flags& f);
    bool operator ==(Flags& f);
};

/**
 * Represents account balance.
 */
class Balance {
    Q_GADGET
    Q_PROPERTY(QString asset_type MEMBER m_assetType)
    Q_PROPERTY(QString asset_code MEMBER m_assetCode)
    Q_PROPERTY(QString asset_issuer MEMBER m_assetIssuer)
    Q_PROPERTY(QString limit MEMBER m_limit)
    Q_PROPERTY(QString balance MEMBER m_balance)

    Q_PROPERTY(QString buying_liabilities MEMBER m_buyingLiabilities)
    Q_PROPERTY(QString selling_liabilities MEMBER m_sellingLiabilities)
    Q_PROPERTY(bool is_authorized MEMBER m_isAuthorized)
    Q_PROPERTY(bool is_authorized_to_maintain_liabilities MEMBER m_isAuthorizedToMaintainLiabilities)
    Q_PROPERTY(int last_modified_ledger MEMBER m_lastModifiedLedger)
    Q_PROPERTY(QString sponsor READ getSponsor MEMBER m_sponsor)



    QString m_assetType;
    QString m_assetCode;
    QString m_assetIssuer;
    QString m_limit;
    QString m_balance;

    QString m_buyingLiabilities;
    QString m_sellingLiabilities;
    bool m_isAuthorized;
    bool m_isAuthorizedToMaintainLiabilities;
    int m_lastModifiedLedger;

    KeyPair *m_assetIssuerKeypair;
    Asset *m_asset;
    QString m_sponsor;

public:
    Balance();
    ~Balance();
    Asset* getAsset();
    QString getAssetType() const;
    QString getAssetCode() const;
    QString assetIssuer() const;
    KeyPair& getAssetIssuer();
    QString getBalance() const;
    QString getLimit() const;

    QString getBuyingLiabilities() const;
    QString getSellingLiabilities() const;
    bool getAuthorized() const;
    bool getAuthorizedToMaintainLiabilities() const;
    int getLastModifiedLedger() const;

    bool operator !=(Balance& b);
    bool operator ==(Balance& b);

    QString getSponsor() const;
};

/**
 * Represents account signers.
 */
class Signer {
    Q_GADGET
    Q_PROPERTY(QString public_key MEMBER m_accountId )
    Q_PROPERTY(QString key MEMBER m_accountId )
    Q_PROPERTY(QString type MEMBER m_type )
    Q_PROPERTY(int weight MEMBER m_weight)
    Q_PROPERTY(QString sponsor READ getSponsor MEMBER m_sponsor)

    QString m_accountId;
    QString m_type;
    int m_weight;
    QString m_sponsor;

public:
    Signer();

    //use getKey() instead
    Q_DECL_DEPRECATED QString getAccountId() const;
    QString getKey() const;
    QString getType() const;

    int getWeight() const;

    bool operator !=(Signer& s);
    bool operator ==(Signer& s);
    QString getSponsor() const;
};

/**
 * Links connected to account.
 */
class Links {
    Q_GADGET
    Q_PROPERTY(Link effects MEMBER m_effects)
    Q_PROPERTY(Link offers MEMBER m_offers)
    Q_PROPERTY(Link operations MEMBER m_operations)
    Q_PROPERTY(Link self MEMBER m_self)
    Q_PROPERTY(Link transactions MEMBER m_transactions)

    Link m_effects;
    Link m_offers;
    Link m_operations;
    Link m_self;
    Link m_transactions;
public:

    Link getEffects();
    Link getOffers();
    Link getOperations();
    Link getSelf();
    Link getTransactions();
    bool operator !=(Links& links);
    bool operator ==(Links& links);
};

/**
 * Data connected to account.
 */
class Data{
    Q_GADGET
    QHash<QString,QByteArray> m_data;
public:
    Data();
    Data(const QVariantMap& data);
    int size() const;
     /**
     * Gets base64-encoded value for a given key or utf8 representation.
     * @param key Data entry name
     * @return base64-encoded value/utf8 representation
     */
    QString get(QString key) const;
     /**
     * Gets decoded base64 value for a given key.
     * @param key Data entry name
     * @return decoded value
     */
    QByteArray getDecoded(QString key) const;
    /**
    * Gets raw value for a given key.
    * @param key Data entry name
    * @return raw value
    */
    QByteArray getRaw(QString key) const;
    bool operator !=(Data& data);
    bool operator ==(Data& data);
};
}
namespace ResponseConverters{
namespace Account{
inline AccountResponseAttach::Data convertData(const QVariantMap& source);
}
}


class AccountResponse : public Response
{
    Q_OBJECT
public:

private:
    Q_PROPERTY(QString account_id READ accountID WRITE setAccountID)
    Q_PROPERTY(KeyPair* keypair READ getKeypair)//we dont allow to overwrite this
    Q_PROPERTY(qint64 sequence MEMBER m_sequence NOTIFY sequenceNumberChanged)
    Q_PROPERTY(QString paging_token MEMBER m_pagingToken NOTIFY pagingTokenChanged)
    Q_PROPERTY(int subentry_count MEMBER m_subentryCount NOTIFY subentryCountChanged)
    Q_PROPERTY(QString inflation_destination MEMBER m_inflationDestination NOTIFY inflationDestinationChanged)
    Q_PROPERTY(QString home_domain MEMBER m_homeDomain NOTIFY homeDomainChanged)
    Q_PROPERTY(AccountResponseAttach::Thresholds thresholds MEMBER m_thresholds NOTIFY thresholdsChanged)
    Q_PROPERTY(AccountResponseAttach::Flags flags MEMBER m_flags NOTIFY flagsChanged)
    Q_PROPERTY(QList<AccountResponseAttach::Balance> balances MEMBER m_balances NOTIFY balancesChanged)
    Q_PROPERTY(QList<AccountResponseAttach::Signer> signers MEMBER m_signers NOTIFY signersChanged)
    Q_PROPERTY(AccountResponseAttach::Data data MEMBER m_data NOTIFY dataChanged)
    Q_PROPERTY(AccountResponseAttach::Links _links MEMBER m_links NOTIFY linksChanged)
    Q_PROPERTY(qint64 num_sponsoring READ getNumSponsoring MEMBER m_numSponsoring NOTIFY numSponsoringChanged)
    Q_PROPERTY(qint64 num_sponsored READ getNumSponsored MEMBER m_numSponsored NOTIFY numSponsoredChanged)
    Q_PROPERTY(QString sponsor READ getSponsor MEMBER m_sponsor NOTIFY sponsorChanged)

    QString m_account_id;
    KeyPair * m_keypair;//generated with m_account_id

    qint64 m_sequence;
    QString m_pagingToken;
    int m_subentryCount;
    QString m_inflationDestination;
    QString m_homeDomain;
    AccountResponseAttach::Thresholds m_thresholds;
    AccountResponseAttach::Flags m_flags;
    QList<AccountResponseAttach::Balance> m_balances;
    QList<AccountResponseAttach::Signer> m_signers;
    AccountResponseAttach::Data m_data;
    AccountResponseAttach::Links m_links;

    qint64 m_numSponsoring;
    qint64 m_numSponsored;
    QString m_sponsor;

public:
    Q_INVOKABLE explicit AccountResponse(QNetworkReply* reply=nullptr);
    virtual ~AccountResponse();

    // TransactionBuilderAccount interface
public:
    /**
     * @brief getKeypair
     * @return KeyPair* , it will get deleted when AccountResponse get deleted.
     */
    KeyPair *getKeypair();
    qint64 getSequenceNumber() const;
    qint64 getIncrementedSequenceNumber() const;
    void incrementSequenceNumber();
    QString accountID() const;
    AccountResponseAttach::Links getLinks() const;
    QString getPagingToken() const;
    qint64 getSubentryCount() const;
    QString getInflationDestination() const;
    QString getHomeDomain() const;
    AccountResponseAttach::Data getData() const;
    AccountResponseAttach::Thresholds getThresholds() const;
    AccountResponseAttach::Flags getFlags() const;
    QList<AccountResponseAttach::Balance> getBalances() const;
    QList<AccountResponseAttach::Signer> getSigners() const;

    qint64 getNumSponsoring() const;
    qint64 getNumSponsored() const;
    QString getSponsor() const;

public slots:
    void setAccountID(QString account_id);
signals:
    void sequenceNumberChanged();
    void pagingTokenChanged();
    void subentryCountChanged();
    void inflationDestinationChanged();
    void homeDomainChanged();
    void thresholdsChanged();
    void flagsChanged();
    void balancesChanged();
    void signersChanged();
    void dataChanged();
    void linksChanged();
    void numSponsoringChanged();
    void numSponsoredChanged();
    void sponsorChanged();
};

Q_DECLARE_METATYPE(AccountResponseAttach::Thresholds)
Q_DECLARE_METATYPE(AccountResponseAttach::Flags)
Q_DECLARE_METATYPE(AccountResponseAttach::Balance)
Q_DECLARE_METATYPE(AccountResponseAttach::Signer)
Q_DECLARE_METATYPE(AccountResponseAttach::Links)
Q_DECLARE_METATYPE(AccountResponseAttach::Data)

#endif // ACCOUNTRESPONSE_H
