#ifndef STELLARLEDGERENTRIES_H
#define STELLARLEDGERENTRIES_H
#include <QtGlobal>
#include "xdrhelper.h"
#include "stellartypes.h"
#include <stdio.h>
#include <exception>
#include <stdexcept>

namespace stellar
{
    using namespace xdr;
    typedef PublicKey AccountID;

    struct Thresholds{
        quint8 thresholds[4];
    };
    XDR_SERIALIZER(Thresholds)

    template <int S>
    struct string: public Array<char,S>{
        inline void operator=(QString text)
        {
            QByteArray utf8 = text.toUtf8();
            if(utf8.size()>Array<char,S>::maxSize()){
                throw std::runtime_error("String length exceed max size");
            }
            Array<char,S>::set(utf8.data(),utf8.size());
        }
        QString toString() const
        {
            return QString::fromUtf8(this->binary());
        }
    };

    typedef string<32> string32;//<32> //max
    typedef string<64> string64; //<64>

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

    struct AssetCode{
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
    inline QDataStream &operator<<(QDataStream &out, const  AssetCode &obj) {
        out << obj.type;
        switch(obj.type){
        case AssetType::ASSET_TYPE_CREDIT_ALPHANUM4:
            out << obj.alphaNum4.assetCode << obj.alphaNum4.issuer; break;
        case AssetType::ASSET_TYPE_CREDIT_ALPHANUM12:
            out << obj.alphaNum12.assetCode << obj.alphaNum12.issuer; break;
        default: throw std::runtime_error("invalid assed code"); break;
        }

       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  AssetCode &obj) {
        in >> obj.type;
        switch(obj.type){
        case AssetType::ASSET_TYPE_CREDIT_ALPHANUM4:
            in >> obj.alphaNum4.assetCode  >> obj.alphaNum4.issuer; break;
        case AssetType::ASSET_TYPE_CREDIT_ALPHANUM12:
            in >> obj.alphaNum12.assetCode >> obj.alphaNum12.issuer; break;
        default: throw std::runtime_error("invalid assed code"); break;//default: break;
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
        DATA = 3,
        CLAIMABLE_BALANCE = 4
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
        AUTH_IMMUTABLE_FLAG = 0x4,

        // Trustlines are created with clawback enabled set to "true",
        // and claimable balances created from those trustlines are created
        // with clawback enabled set to "true"
        AUTH_CLAWBACK_ENABLED_FLAG = 0x8
    };
    //mask for all valid flags
    const qint32 MASK_ACCOUNT_FLAGS = 0x7;
    const qint32 MASK_ACCOUNT_FLAGS_V16 = 0xF;

    // maximum number of signers
    const qint32 MAX_SIGNERS = 20;

    typedef AccountID SponsorshipDescriptor;

    struct AccountEntryExtensionV2
    {
        quint32 numSponsored;
        quint32 numSponsoring;
        Array<SponsorshipDescriptor,MAX_SIGNERS> signerSponsoringIDs;

        // reserved for future use
        Reserved ext;
    };

    inline QDataStream &operator<<(QDataStream &out, const  AccountEntryExtensionV2 &obj) {
        out << obj.numSponsored<< obj.numSponsoring<< obj.signerSponsoringIDs<< obj.ext;

       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  AccountEntryExtensionV2 &obj) {
       in >> obj.numSponsored>> obj.numSponsoring>> obj.signerSponsoringIDs<< obj.ext;

       return in;
    }

    struct AccountEntryExtensionV1
    {
        Liabilities liabilities;
        Reserved ext;
        union{
            AccountEntryExtensionV2 v2;//case 2 (ext)
        };
        AccountEntryExtensionV1()
        {
//            ext.reserved=2;
//            new (&v2) AccountEntryExtensionV2();
        }
        ~AccountEntryExtensionV1()
        {
            switch(ext.reserved){
            case 2:
                v2.~AccountEntryExtensionV2();break;
            default:break;
            }
        }
    };

    inline QDataStream &operator<<(QDataStream &out, const  AccountEntryExtensionV1 &obj) {
        out << obj.liabilities<< obj.ext;

        switch(obj.ext.reserved){
        case 2:
            out << obj.v2; break;
        default: break;
        }


       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  AccountEntryExtensionV1 &obj) {
       in >> obj.liabilities>> obj.ext;

       switch(obj.ext.reserved){
       case 2:
           new (&obj.v2) AccountEntryExtensionV2();
           in >> obj.v2 ; break;
       default: break;
       }

       return in;
    }

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

        Array<Signer,MAX_SIGNERS> signers; //max : <20>; // possible signers for this account

        Reserved ext;      
        union{
            AccountEntryExtensionV1 v1;//case 1 (ext)
        };
        AccountEntry()
        {
            //ext.reserved=1;
            //new (&v1) AccountEntryExtensionV1();
        }
        ~AccountEntry()
        {
            switch(ext.reserved){
            case 1:
                v1.~AccountEntryExtensionV1();break;
            default:break;
            }
        }
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
           new (&obj.v1) AccountEntryExtensionV1();
           in >> obj.v1.liabilities >> obj.v1.ext ; break;
       default: break;
       }

       return in;
    }

    enum class ClaimPredicateType : qint32
    {
        CLAIM_PREDICATE_UNCONDITIONAL = 0,
        CLAIM_PREDICATE_AND = 1,
        CLAIM_PREDICATE_OR = 2,
        CLAIM_PREDICATE_NOT = 3,
        CLAIM_PREDICATE_BEFORE_ABSOLUTE_TIME = 4,
        CLAIM_PREDICATE_BEFORE_RELATIVE_TIME = 5
    };

    struct ClaimPredicate
    {
        ClaimPredicateType type;
        union
        {
            //case CLAIM_PREDICATE_UNCONDITIONAL:
              //  void;
            //case CLAIM_PREDICATE_AND:
                Array<ClaimPredicate,2> andPredicates;
            //case CLAIM_PREDICATE_OR:
                Array<ClaimPredicate,2> orPredicates;
            //case CLAIM_PREDICATE_NOT:
                Array<ClaimPredicate,1> notPredicate;//this should be optional but it is serialized the same way than an array of 1, i should improve Optional so it works with a pointer or subclass Array<T,1>
            //case CLAIM_PREDICATE_BEFORE_ABSOLUTE_TIME:
                qint64 absBefore; // Predicate will be true if closeTime < absBefore
            //case CLAIM_PREDICATE_BEFORE_RELATIVE_TIME:
                qint64 relBefore; // Seconds since closeTime of the ledger in which the
                                 // ClaimableBalanceEntry was created
        };
        ClaimPredicate():type(ClaimPredicateType::CLAIM_PREDICATE_UNCONDITIONAL)
        {
        }
    private:
        void clear()
        {
            switch(type){
            case ClaimPredicateType::CLAIM_PREDICATE_AND:
                andPredicates.~Array<ClaimPredicate,2>(); break;
            case ClaimPredicateType::CLAIM_PREDICATE_OR:
                orPredicates.~Array<ClaimPredicate,2>(); break;
            case ClaimPredicateType::CLAIM_PREDICATE_NOT:
                notPredicate.~Array<ClaimPredicate,1>(); break;
            default: break;
            }
        }
    public:
        void setUnconditional()
        {
            clear();
            type = ClaimPredicateType::CLAIM_PREDICATE_UNCONDITIONAL;
        }
        Array<ClaimPredicate,2>& fillAndPredicates()
        {
            clear();
            type = ClaimPredicateType::CLAIM_PREDICATE_AND;
            new (&andPredicates) Array<ClaimPredicate,2>();
            return andPredicates;
        }
        Array<ClaimPredicate,2>& fillOrPredicates()
        {
            clear();
            type = ClaimPredicateType::CLAIM_PREDICATE_OR;
            new (&orPredicates) Array<ClaimPredicate,2>();
            return orPredicates;
        }
        Array<ClaimPredicate,1>& fillNotPredicate()
        {
            clear();
            type = ClaimPredicateType::CLAIM_PREDICATE_NOT;
            new (&notPredicate) Array<ClaimPredicate,1>();
            return notPredicate;
        }
        void setAbsBefore(qint64 ab)
        {
            clear();
            type = ClaimPredicateType::CLAIM_PREDICATE_BEFORE_ABSOLUTE_TIME;
            absBefore= ab;
        }
        void setRelBefore(qint64 rb)
        {
            clear();
            type = ClaimPredicateType::CLAIM_PREDICATE_BEFORE_RELATIVE_TIME;
            relBefore= rb;
        }
        ClaimPredicate(const stellar::ClaimPredicate &obj){
            type = obj.type;
            switch(obj.type){
            case ClaimPredicateType::CLAIM_PREDICATE_UNCONDITIONAL:
                break;
            case ClaimPredicateType::CLAIM_PREDICATE_AND:
                new (&andPredicates) Array<ClaimPredicate,2>();
                andPredicates= obj.andPredicates; break;
            case ClaimPredicateType::CLAIM_PREDICATE_OR:
                new (&orPredicates) Array<ClaimPredicate,2>();
                orPredicates= obj.orPredicates; break;
            case ClaimPredicateType::CLAIM_PREDICATE_NOT:
                new (&notPredicate) Array<ClaimPredicate,1>();
                notPredicate= obj.notPredicate; break;
            case ClaimPredicateType::CLAIM_PREDICATE_BEFORE_ABSOLUTE_TIME:
                absBefore= obj.absBefore; break;// Predicate will be true if closeTime < absBefore
            case ClaimPredicateType::CLAIM_PREDICATE_BEFORE_RELATIVE_TIME:
                relBefore= obj.relBefore; break;// Seconds since closeTime of the ledger in which the
            default: break;
            }
        }
        ~ClaimPredicate()
        {
            clear();
        }
        const ClaimPredicate& operator = (const ClaimPredicate& op);

    };

    inline const stellar::ClaimPredicate &stellar::ClaimPredicate::operator =(const stellar::ClaimPredicate &obj) {
        switch(type){
        case ClaimPredicateType::CLAIM_PREDICATE_AND:
            andPredicates.~Array<ClaimPredicate,2>(); break;
        case ClaimPredicateType::CLAIM_PREDICATE_OR:
            orPredicates.~Array<ClaimPredicate,2>(); break;
        case ClaimPredicateType::CLAIM_PREDICATE_NOT:
            notPredicate.~Array<ClaimPredicate,1>(); break;
        default: break;
        }
        type = obj.type;
        switch(obj.type){
        case ClaimPredicateType::CLAIM_PREDICATE_UNCONDITIONAL:
            break;
        case ClaimPredicateType::CLAIM_PREDICATE_AND:
            new (&andPredicates) Array<ClaimPredicate,2>();
            andPredicates= obj.andPredicates; break;
        case ClaimPredicateType::CLAIM_PREDICATE_OR:
            new (&orPredicates) Array<ClaimPredicate,2>();
            orPredicates= obj.orPredicates; break;
        case ClaimPredicateType::CLAIM_PREDICATE_NOT:
            new (&notPredicate) Array<ClaimPredicate,1>();
            notPredicate= obj.notPredicate; break;
        case ClaimPredicateType::CLAIM_PREDICATE_BEFORE_ABSOLUTE_TIME:
            absBefore= obj.absBefore; break;// Predicate will be true if closeTime < absBefore
        case ClaimPredicateType::CLAIM_PREDICATE_BEFORE_RELATIVE_TIME:
            relBefore= obj.relBefore; break;// Seconds since closeTime of the ledger in which the
        default: break;
        }
        return *this;
    }


    inline QDataStream &operator<<(QDataStream &out, const  ClaimPredicate &obj) {
        out << obj.type;

        switch(obj.type){
        case ClaimPredicateType::CLAIM_PREDICATE_UNCONDITIONAL:
            break;
        case ClaimPredicateType::CLAIM_PREDICATE_AND:
            out<< obj.andPredicates; break;
        case ClaimPredicateType::CLAIM_PREDICATE_OR:
            out<< obj.orPredicates; break;
        case ClaimPredicateType::CLAIM_PREDICATE_NOT:
            out<< obj.notPredicate; break;
        case ClaimPredicateType::CLAIM_PREDICATE_BEFORE_ABSOLUTE_TIME:
            out<< obj.absBefore; break;// Predicate will be true if closeTime < absBefore
        case ClaimPredicateType::CLAIM_PREDICATE_BEFORE_RELATIVE_TIME:
            out<< obj.relBefore; break;// Seconds since closeTime of the ledger in which the
        default: break;
        }


       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  ClaimPredicate &obj) {
        in >> obj.type;

        switch(obj.type){
        case ClaimPredicateType::CLAIM_PREDICATE_UNCONDITIONAL:
            break;
        case ClaimPredicateType::CLAIM_PREDICATE_AND:
            new (&obj.andPredicates) Array<ClaimPredicate,2>();
            in>> obj.andPredicates; break;
        case ClaimPredicateType::CLAIM_PREDICATE_OR:
            new (&obj.orPredicates) Array<ClaimPredicate,2>();
            in>> obj.orPredicates; break;
        case ClaimPredicateType::CLAIM_PREDICATE_NOT:
            new (&obj.notPredicate) Array<ClaimPredicate,1>();
            in>> obj.notPredicate; break;
        case ClaimPredicateType::CLAIM_PREDICATE_BEFORE_ABSOLUTE_TIME:
            in>> obj.absBefore; break;// Predicate will be true if closeTime < absBefore
        case ClaimPredicateType::CLAIM_PREDICATE_BEFORE_RELATIVE_TIME:
            in>> obj.relBefore; break;// Seconds since closeTime of the ledger in which the
        default: break;
        }


       return in;
    }

    enum class ClaimantType : qint32
    {
        CLAIMANT_TYPE_V0 = 0
    };

    struct Claimant
    {
        ClaimantType type;
        struct V0
        {
            AccountID destination;    // The account that can use this condition
            ClaimPredicate predicate; // Claimable if predicate is true
        };

        //union{
            //case CLAIMANT_TYPE_V0
            V0 v0;
        //};
    };

    inline QDataStream &operator<<(QDataStream &out, const  Claimant &obj) {
        out << obj.type;

        switch(obj.type){
        case ClaimantType::CLAIMANT_TYPE_V0:
            out<< obj.v0.destination << obj.v0.predicate; break;
        default: break;
        }
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  Claimant &obj) {
        in >> obj.type;

        switch(obj.type){
        case ClaimantType::CLAIMANT_TYPE_V0:
            in>> obj.v0.destination >> obj.v0.predicate; break;
        default: break;
        }


       return in;
    }

    enum class ClaimableBalanceIDType : qint32
    {
        CLAIMABLE_BALANCE_ID_TYPE_V0 = 0
    };

    struct ClaimableBalanceID
    {
        ClaimableBalanceIDType type;
        union
        {
            //case CLAIMABLE_BALANCE_ID_TYPE_V0:
            Hash v0;
        };
        ClaimableBalanceID()
        {
            new (&v0) Hash();
        }
    };

    inline QDataStream &operator<<(QDataStream &out, const  ClaimableBalanceID &obj) {
        out << obj.type;

        switch(obj.type){
        case ClaimableBalanceIDType::CLAIMABLE_BALANCE_ID_TYPE_V0:
            out<< obj.v0; break;
        default: break;
        }
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  ClaimableBalanceID &obj) {
        in >> obj.type;

        switch(obj.type){
        case ClaimableBalanceIDType::CLAIMABLE_BALANCE_ID_TYPE_V0:
            in>> obj.v0 ; break;
        default: break;
        }


       return in;
    }

    enum class ClaimableBalanceFlags : qint32
    {
        // If set, the issuer account of the asset held by the claimable balance may
        // clawback the claimable balance
        CLAIMABLE_BALANCE_CLAWBACK_ENABLED_FLAG = 0x1
    };

    const qint32 MASK_CLAIMABLE_BALANCE_FLAGS = 0x1;

    struct ClaimableBalanceEntryExtensionV1
    {
        // reserved for future use
        Reserved ext;

        quint32 flags; // see ClaimableBalanceFlags
    };

    inline QDataStream &operator<<(QDataStream &out, const  ClaimableBalanceEntryExtensionV1 &obj) {
        out << obj.ext<< obj.flags;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  ClaimableBalanceEntryExtensionV1 &obj) {
        in >> obj.ext>> obj.flags;
       return in;
    }

    struct ClaimableBalanceEntry
    {
        // Unique identifier for this ClaimableBalanceEntry
        ClaimableBalanceID balanceID;

        // List of claimants with associated predicate
        Array<Claimant,10> claimants;

        // Any asset including native
        Asset asset;

        // Amount of asset
        qint64 amount;

        // reserved for future use
        Reserved ext;
        union{
            ClaimableBalanceEntryExtensionV1 v1;
        };
    };

    inline QDataStream &operator<<(QDataStream &out, const  ClaimableBalanceEntry &obj) {
        out << obj.balanceID<< obj.claimants<< obj.asset<< obj.amount<< obj.ext;
        switch(obj.ext.reserved)
        {
        case 1:
        {
            out << obj.v1;
            break;
        }
        default:
        break;
        }

       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  ClaimableBalanceEntry &obj) {
        in >> obj.balanceID>> obj.claimants>> obj.asset>> obj.amount>> obj.ext;
        switch(obj.ext.reserved)
        {
        case 1:
        {
            in >> obj.v1;
            break;
        }
        default:
        break;
        }
       return in;
    }



    struct LedgerEntryExtensionV1
    {
        SponsorshipDescriptor sponsoringID;

        // reserved for future use
        Reserved ext;
    };

    inline QDataStream &operator<<(QDataStream &out, const  LedgerEntryExtensionV1 &obj) {
        out << obj.sponsoringID<<obj.ext;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  LedgerEntryExtensionV1 &obj) {
        in >> obj.sponsoringID>> obj.ext;
       return in;
    }

    enum class TrustLineFlags : qint32
    {
        // issuer has authorized account to perform transactions with its credit
        AUTHORIZED_FLAG = 1,
        // issuer has authorized account to maintain and reduce liabilities for its
        // credit
        AUTHORIZED_TO_MAINTAIN_LIABILITIES_FLAG = 2,
        // issuer has specified that it may clawback its credit, and that claimable
        // balances created with its credit may also be clawed back
        TRUSTLINE_CLAWBACK_ENABLED_FLAG = 4
    };
    //mask for all trustline flags
    const qint32 MASK_TRUSTLINE_FLAGS = 1;
    const qint32 MASK_TRUSTLINE_FLAGS_V13 = 3;
    const qint32 MASK_TRUSTLINE_FLAGS_V16 = 7;

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
            ClaimableBalanceEntry claimableBalance;
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
       case LedgerEntryType::CLAIMABLE_BALANCE:
            out << obj.claimableBalance; break;
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
        case LedgerEntryType::CLAIMABLE_BALANCE:
             in >> obj.claimableBalance; break;
        //default: break;
        }
       return in;
    }


    struct LedgerKey
    {
        LedgerEntryType type;
        struct Account//case ACCOUNT:
        {
            AccountID accountID;
        };
        struct TrustLine//case ACCOUNT:
        {
            AccountID accountID;
            Asset asset;
        };
        struct Offer //case OFFER:
        {
            AccountID sellerID;
            qint64 offerID;
        };
        struct Data//case DATA:
        {
            AccountID accountID;
            string64 dataName;
        };
        struct ClaimableBalance//case CLAIMABLE_BALANCE:
        {
            ClaimableBalanceID balanceID;
        };
        union
        {
            Account account;
            TrustLine trustLine;
            Offer offer;
            Data data;
            ClaimableBalance claimableBalance;
        };
        LedgerKey():type(LedgerEntryType::ACCOUNT)
        {
            new (&account) Account();
        }
    private:
        void clear()
        {
            switch(type){
            case LedgerEntryType::ACCOUNT:
                (account).~Account(); break;
            case LedgerEntryType::TRUSTLINE:
                (trustLine).~TrustLine(); break;
            case LedgerEntryType::OFFER:
                (offer).~Offer(); break;
            case LedgerEntryType::DATA:
                (data).~Data(); break;
           case LedgerEntryType::CLAIMABLE_BALANCE:
                (claimableBalance).~ClaimableBalance(); break;
            default: break;
            }
        }
    public:
        Account& fillAccount()
        {
            clear();
            new (&account) Account();
            type=LedgerEntryType::ACCOUNT;
            return account;
        }
        ClaimableBalance& fillClaimableBalance()
        {
            clear();
            new (&claimableBalance) ClaimableBalance();
            type=LedgerEntryType::CLAIMABLE_BALANCE;
            return claimableBalance;
        }
        Data& fillData()
        {
            clear();
            new (&data) Data();
            type=LedgerEntryType::DATA;
            return data;
        }
        Offer& fillOffer()
        {
            clear();
            new (&offer) Offer();
            type=LedgerEntryType::OFFER;
            return offer;
        }
        TrustLine& fillTrustLine()
        {
            clear();
            new (&trustLine) TrustLine();
            type=LedgerEntryType::TRUSTLINE;
            return trustLine;
        }

        ~LedgerKey()
        {
            clear();
        }
        LedgerKey(const LedgerKey &obj)
        {
            switch(obj.type){
            case LedgerEntryType::ACCOUNT:
                new (&account) Account();
                account.accountID= obj.account.accountID; break;
            case LedgerEntryType::TRUSTLINE:
                new (&trustLine) TrustLine();
                trustLine.accountID= obj.trustLine.accountID;
                trustLine.asset= obj.trustLine.asset; break;
            case LedgerEntryType::OFFER:
                new (&offer) Offer();
                offer.sellerID= obj.offer.sellerID;
                offer.offerID=obj.offer.offerID; break;
            case LedgerEntryType::DATA:
                new (&data) Data();
                data.accountID= obj.data.accountID;
                data.dataName=obj.data.dataName; break;
           case LedgerEntryType::CLAIMABLE_BALANCE:
                new (&claimableBalance) ClaimableBalance();
                claimableBalance.balanceID= obj.claimableBalance.balanceID; break;
            //default: break;
            }
        }
        const LedgerKey& operator=(const LedgerKey &obj)
        {
            clear();
            type = obj.type;
            switch(type){
            case LedgerEntryType::ACCOUNT:
                new (&account) Account();
                account.accountID= obj.account.accountID; break;
            case LedgerEntryType::TRUSTLINE:
                new (&trustLine) TrustLine();
                trustLine.accountID= obj.trustLine.accountID;
                trustLine.asset= obj.trustLine.asset; break;
            case LedgerEntryType::OFFER:
                new (&offer) Offer();
                offer.sellerID= obj.offer.sellerID;
                offer.offerID= obj.offer.offerID; break;
            case LedgerEntryType::DATA:
                new (&data) Data();
                data.accountID= obj.data.accountID;                
                data.dataName= obj.data.dataName; break;
            case LedgerEntryType::CLAIMABLE_BALANCE:
                new (&claimableBalance) ClaimableBalance();
                claimableBalance.balanceID= obj.claimableBalance.balanceID; break;
            }
            return *this;
        }

    };
    inline QDataStream &operator<<(QDataStream &out, const  LedgerKey &obj) {
        out  << obj.type;
        switch(obj.type){
        case LedgerEntryType::ACCOUNT:
            out << obj.account.accountID; break;
        case LedgerEntryType::TRUSTLINE:
            out << obj.trustLine.accountID << obj.trustLine.asset; break;
        case LedgerEntryType::OFFER:
            out << obj.offer.sellerID << obj.offer.offerID; break;
        case LedgerEntryType::DATA:             
            out << obj.data.accountID<<obj.data.dataName; break;
       case LedgerEntryType::CLAIMABLE_BALANCE:
            out << obj.claimableBalance.balanceID; break;
        //default: break;
        }

       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  LedgerKey &obj) {
        in >> obj.type;
        switch(obj.type){
        case LedgerEntryType::ACCOUNT:
            new (&obj.account) LedgerKey::Account();
            in >> obj.account.accountID; break;
        case LedgerEntryType::TRUSTLINE:
            new (&obj.trustLine) LedgerKey::TrustLine();
            in >> obj.trustLine.accountID >> obj.trustLine.asset; break;
        case LedgerEntryType::OFFER:
            new (&obj.offer) LedgerKey::Offer();
            in >> obj.offer.sellerID >> obj.offer.offerID; break;
        case LedgerEntryType::DATA:
            new (&obj.data) LedgerKey::Data();
            in >> obj.data.accountID  >> obj.data.dataName; break;
       case LedgerEntryType::CLAIMABLE_BALANCE:
            new (&obj.claimableBalance) LedgerKey::ClaimableBalance();
            in >> obj.claimableBalance.balanceID; break;
        //default: break;
        }
       return in;
    }

    // list of all envelope types used in the application
    // those are prefixes used when building signatures for
    // the respective envelopes
    enum class EnvelopeType : qint32
    {
        ENVELOPE_TYPE_TX_V0 = 0,
        ENVELOPE_TYPE_SCP = 1,
        ENVELOPE_TYPE_TX = 2,
        ENVELOPE_TYPE_AUTH = 3,
        ENVELOPE_TYPE_SCPVALUE = 4,
        ENVELOPE_TYPE_TX_FEE_BUMP = 5,
        ENVELOPE_TYPE_OP_ID = 6
    };
}



#endif // STELLARLEDGERENTRIES_H
