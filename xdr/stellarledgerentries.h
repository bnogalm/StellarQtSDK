#ifndef STELLARLEDGERENTRIES_H
#define STELLARLEDGERENTRIES_H
#include <QtGlobal>
#include "xdrhelper.h"
#include "stellartypes.h"
#include <stdio.h>
namespace stellar
{
    using namespace xdr;
    typedef PublicKey AccountID;

    struct Thresholds{
        quint8 thresholds[4];
    };
    XDR_SERIALIZER(Thresholds)
    typedef Array<char,32> string32; //<32> //max
    typedef Array<char,64> string64; //<64>
    typedef qint64 SequenceNumber;
    typedef quint64 TimePoint;

    typedef Array<quint8,64> DataValue; //<64>

    enum class AssetType : qint32
    {
        ASSET_TYPE_NATIVE = 0,
        ASSET_TYPE_CREDIT_ALPHANUM4 = 1,
        ASSET_TYPE_CREDIT_ALPHANUM12 = 2
    };



    typedef qint8 AssetCode4[4];
    typedef qint8 AssetCode12[12];
    XDR_SERIALIZER(AssetCode4)
    XDR_SERIALIZER(AssetCode12)

    struct Asset{
        AssetType type;
        union{
            struct{
                AssetCode4 assetCode;// 1 to 4 characters
                AccountID issuer;
            }alphaNum4;
            struct{
                AssetCode12 assetCode;// 5 to 12 characters
                AccountID issuer;
            }alphaNum12;
        };
    };
    inline QDataStream &operator<<(QDataStream &out, const  Asset &obj) {
        out << obj.type;
        switch(obj.type){
        case AssetType::ASSET_TYPE_NATIVE: break;
        case AssetType::ASSET_TYPE_CREDIT_ALPHANUM4:
            out << obj.alphaNum4.assetCode << obj.alphaNum4.issuer; break;
        case AssetType::ASSET_TYPE_CREDIT_ALPHANUM12:
            out << obj.alphaNum12.assetCode << obj.alphaNum12.issuer; break;
        //default: break;
        }

       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  Asset &obj) {
        in >> obj.type;
        switch(obj.type){
        case AssetType::ASSET_TYPE_NATIVE: break;
        case AssetType::ASSET_TYPE_CREDIT_ALPHANUM4:
            in >> obj.alphaNum4.assetCode  >> obj.alphaNum4.issuer; break;
        case AssetType::ASSET_TYPE_CREDIT_ALPHANUM12:
            in >> obj.alphaNum12.assetCode >> obj.alphaNum12.issuer; break;
        //default: break;
        }

       return in;
    }


    // price in fractional representation
    struct alignas(4) Price
    {
        qint32 n; //numerator
        qint32 d; //denominator
    };
    inline QDataStream &operator<<(QDataStream &out, const  Price &obj) {
        out << obj.n<< obj.d;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  Price &obj) {
        in >> obj.n>>obj.d;
       return in;
    }


    struct Liabilities
    {
        qint64 buying;
        qint64 selling;
    };
    inline QDataStream &operator<<(QDataStream &out, const  Liabilities &obj) {
        out << obj.buying<< obj.selling;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  Liabilities &obj) {
        in >> obj.buying>>obj.selling;
       return in;
    }

    // the 'Thresholds' type is packed uint8_t values
    // defined by these indexes
    enum class ThresholdIndexes : qint32
    {
        THRESHOLD_MASTER_WEIGHT = 0,
        THRESHOLD_LOW = 1,
        THRESHOLD_MED = 2,
        THRESHOLD_HIGH = 3
    };


    enum class LedgerEntryType : qint32
    {
        ACCOUNT = 0,
        TRUSTLINE = 1,
        OFFER = 2,
        DATA = 3
    };


    struct alignas(4) Signer
    {
        SignerKey key;
        quint32 weight; // really only need 1byte
    };
    inline QDataStream &operator<<(QDataStream &out, const  Signer &obj) {
        out << obj.key<< obj.weight;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  Signer &obj) {
        in >> obj.key>>obj.weight;
       return in;
    }


    enum class AccountFlags : qint32
    { // masks for each flag

        // Flags set on issuer accounts
        // TrustLines are created with authorized set to "false" requiring
        // the issuer to set it for each TrustLine
        AUTH_REQUIRED_FLAG = 0x1,
        // If set, the authorized flag in TrustLines can be cleared
        // otherwise, authorization cannot be revoked
        AUTH_REVOCABLE_FLAG = 0x2,
        // Once set, causes all AUTH_* flags to be read-only
        AUTH_IMMUTABLE_FLAG = 0x4
    };
    //mask for all valid flags
    const qint32 MASK_ACCOUNT_FLAGS = 0x7;

    /* AccountEntry
        Main entry representing a user in Stellar. All transactions are
        performed using an account.
        Other ledger entries created require an account.
    */

    struct AccountEntry
    {
        AccountID accountID;      // master public key for this account
        qint64 balance;            // in stroops
        SequenceNumber seqNum;    // last sequence number used for this account
        quint32 numSubEntries;     // number of sub-entries this account has
                                  // drives the reserve
        Optional<AccountID> inflationDest; // Account to vote for during inflation
        quint32 flags;             // see AccountFlags

        string32 homeDomain; // can be used for reverse federation and memo lookup

        // fields used for signatures
        // thresholds stores unsigned bytes: [weight of master|low|medium|high]
        Thresholds thresholds;

        Array<Signer,20> signers; //max : <20>; // possible signers for this account
        // reserved for future use
        Reserved ext;
        struct V1{
            Liabilities liabilities;
            Reserved ext;
        };
        union{
            V1 v1;
        };

    };

    inline QDataStream &operator<<(QDataStream &out, const  AccountEntry &obj) {
        out << obj.accountID<< obj.balance << obj.seqNum << obj.numSubEntries << obj.inflationDest << obj.flags << obj.homeDomain << obj.thresholds << obj.signers << obj.ext;

        switch(obj.ext.reserved){
        case 1:
            out << obj.v1.liabilities << obj.v1.ext ; break;
        default: break;
        }


       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  AccountEntry &obj) {
       in >> obj.accountID>> obj.balance >> obj.seqNum >> obj.numSubEntries >> obj.inflationDest >> obj.flags >> obj.homeDomain >> obj.thresholds >> obj.signers >> obj.ext;

       switch(obj.ext.reserved){
       case 1:
           in >> obj.v1.liabilities >> obj.v1.ext ; break;
       default: break;
       }

       return in;
    }

    enum class TrustLineFlags : qint32
    {
        // issuer has authorized account to perform transactions with its credit
        AUTHORIZED_FLAG = 1
    };
    //mask for all trustline flags
    const qint32 MASK_TRUSTLINE_FLAGS = 1;

    struct TrustLineEntry
    {
        AccountID accountID; // account this trustline belongs to
        Asset asset;         // type of asset (with issuer)
        qint64 balance;       // how much of this asset the user has.
                             // Asset defines the unit for this;

        qint64 limit;  // balance cannot be above this
        quint32 flags; // see TrustLineFlags

        // reserved for future use
        Reserved ext;
        struct V1{
            Liabilities liabilities;
            Reserved ext;
        };
        union{
            V1 v1;
        };
    };
    inline QDataStream &operator<<(QDataStream &out, const  TrustLineEntry &obj) {
        out << obj.accountID << obj.asset << obj.balance << obj.limit << obj.flags << obj.ext;
        switch(obj.ext.reserved){
        case 1:
            out << obj.v1.liabilities << obj.v1.ext ; break;
        default: break;
        }
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  TrustLineEntry &obj) {
       in >> obj.accountID >> obj.asset >> obj.balance >> obj.limit >> obj.flags >> obj.ext;
       switch(obj.ext.reserved){
       case 1:
           in >> obj.v1.liabilities >> obj.v1.ext ; break;
       default: break;
       }
       return in;
    }



    enum class OfferEntryFlags : qint32
    {
        // issuer has authorized account to perform transactions with its credit
        PASSIVE_FLAG = 1
    };
    //mask for offerentry flags
    const qint32 MASK_OFFERENTRY_FLAGS = 1;


    /* OfferEntry
        An offer is the building block of the offer book, they are automatically
        claimed by payments when the price set by the owner is met.
        For example an Offer is selling 10A where 1A is priced at 1.5B
    */
    struct OfferEntry
    {
        AccountID sellerID;
        qint64 offerID;
        Asset selling; // A
        Asset buying;  // B
        qint64 amount;  // amount of A

        /* price for this offer:
            price of A in terms of B
            price=AmountB/AmountA=priceNumerator/priceDenominator
            price is after fees
        */
        Price price;
        quint32 flags; // see OfferEntryFlags

        // reserved for future use
        Reserved ext;
    };
    inline QDataStream &operator<<(QDataStream &out, const  OfferEntry &obj) {
        out << obj.sellerID << obj.offerID << obj.selling << obj.buying << obj.amount << obj.price << obj.flags << obj.ext;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  OfferEntry &obj) {
       in >> obj.sellerID >> obj.offerID >> obj.selling >> obj.buying >> obj.amount >> obj.price >> obj.flags >> obj.ext;
       return in;
    }

    /* DataEntry
        Data can be attached to accounts.
    */
    struct DataEntry
    {
        AccountID accountID; // account this data belongs to
        string64 dataName;
        DataValue dataValue;

        // reserved for future use
        Reserved ext;
    };
    inline QDataStream &operator<<(QDataStream &out, const  DataEntry &obj) {
        out << obj.accountID << obj.dataName << obj.dataValue;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  DataEntry &obj) {
       in >> obj.accountID >> obj.dataName >> obj.dataValue;
       return in;
    }



    struct LedgerEntry
    {
        quint32 lastModifiedLedgerSeq; // ledger the LedgerEntry was last changed
        LedgerEntryType type;
        union{
            AccountEntry account;
            TrustLineEntry trustLine;
            OfferEntry offer;
            DataEntry data;
        };
        ~LedgerEntry(){}
    };

    inline QDataStream &operator<<(QDataStream &out, const  LedgerEntry &obj) {
        out << obj.lastModifiedLedgerSeq << obj.type;
        switch(obj.type){
        case LedgerEntryType::ACCOUNT:
            out << obj.account; break;
        case LedgerEntryType::TRUSTLINE:
            out << obj.trustLine; break;
        case LedgerEntryType::OFFER:
            out << obj.offer; break;
        case LedgerEntryType::DATA:
            out << obj.data; break;
        //default: break;
        }

       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  LedgerEntry &obj) {
        in >> obj.lastModifiedLedgerSeq >> obj.type;
        switch(obj.type){
        case LedgerEntryType::ACCOUNT:
            in >> obj.account; break;
        case LedgerEntryType::TRUSTLINE:
            in >> obj.trustLine; break;
        case LedgerEntryType::OFFER:
            in >> obj.offer; break;
        case LedgerEntryType::DATA:
            in >> obj.data; break;
        //default: break;
        }
       return in;
    }

    // list of all envelope types used in the application
    // those are prefixes used when building signatures for
    // the respective envelopes
    enum class EnvelopeType : qint32
    {
        ENVELOPE_TYPE_SCP = 1,
        ENVELOPE_TYPE_TX = 2,
        ENVELOPE_TYPE_AUTH = 3,
        ENVELOPE_TYPE_SCPVALUE = 4
    };
}



#endif // STELLARLEDGERENTRIES_H
