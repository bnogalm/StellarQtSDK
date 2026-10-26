#ifndef STELLARLEDGERENTRIES_H
#define STELLARLEDGERENTRIES_H
#include <QtGlobal>
#include "xdrhelper.h"
#include "stellartypes.h"
#include <stdio.h>
#include <cstring>
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
        ASSET_TYPE_CREDIT_ALPHANUM12 = 2,
        ASSET_TYPE_POOL_SHARE = 3
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

        // Zeroed, the largest arm covering the whole union: a code or issuer
        // the caller leaves unset is then zeros on the wire, never whatever
        // the stack held (which used to be written, hashed and signed).
        Asset() : type(AssetType::ASSET_TYPE_NATIVE)
        {
            static_assert(sizeof(alphaNum12) >= sizeof(alphaNum4), "alphaNum12 must cover the union");
            std::memset(&alphaNum12, 0, sizeof alphaNum12);
        }
    };
    inline QDataStream &operator<<(QDataStream &out, const  Asset &obj) {
        switch(obj.type){
        case AssetType::ASSET_TYPE_NATIVE:
            out << obj.type; break;
        case AssetType::ASSET_TYPE_CREDIT_ALPHANUM4:
            out << obj.type << obj.alphaNum4.assetCode << obj.alphaNum4.issuer; break;
        case AssetType::ASSET_TYPE_CREDIT_ALPHANUM12:
            out << obj.type << obj.alphaNum12.assetCode << obj.alphaNum12.issuer; break;
        // POOL_SHARE is not an arm of a plain Asset, and neither is anything
        // else: they went out as a bare discriminant that nothing can decode.
        default: out.setStatus(QDataStream::WriteFailed); break;
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
        default:    // POOL_SHARE is not an arm of a plain Asset: its pool id would be left unread
            in.setStatus(QDataStream::ReadCorruptData); break;
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
        case AssetType::ASSET_TYPE_NATIVE:
        case AssetType::ASSET_TYPE_POOL_SHARE:
        default: throw std::runtime_error("invalid asset code"); break;
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
        case AssetType::ASSET_TYPE_NATIVE:
        case AssetType::ASSET_TYPE_POOL_SHARE:
        default: throw std::runtime_error("invalid asset code"); break;
        }

       return in;
    }


    // CAP-38 Protocol 18 — Liquidity Pools
    typedef Hash PoolID;

    enum class LiquidityPoolType : qint32
    {
        LIQUIDITY_POOL_CONSTANT_PRODUCT = 0
    };

    struct LiquidityPoolConstantProductParameters
    {
        Asset assetA;
        Asset assetB;
        qint32 fee; // basis points (LIQUIDITY_POOL_FEE_V18 = 30)
    };
    inline QDataStream &operator<<(QDataStream &out, const LiquidityPoolConstantProductParameters &obj) {
        out << obj.assetA << obj.assetB << obj.fee;
        return out;
    }
    inline QDataStream &operator>>(QDataStream &in, LiquidityPoolConstantProductParameters &obj) {
        in >> obj.assetA >> obj.assetB >> obj.fee;
        return in;
    }

    struct LiquidityPoolParameters
    {
        LiquidityPoolType type;
        union {
            LiquidityPoolConstantProductParameters constantProduct;
        };

        LiquidityPoolParameters() : type(LiquidityPoolType::LIQUIDITY_POOL_CONSTANT_PRODUCT)
        {
            new (&constantProduct) LiquidityPoolConstantProductParameters();
        }
        LiquidityPoolParameters(const LiquidityPoolParameters &other) : type(other.type)
        {
            switch(type) {
            case LiquidityPoolType::LIQUIDITY_POOL_CONSTANT_PRODUCT:
                new (&constantProduct) LiquidityPoolConstantProductParameters(other.constantProduct);
                break;
            }
        }
        ~LiquidityPoolParameters() { /* trivial — Asset has no destructor */ }
        LiquidityPoolParameters& operator=(const LiquidityPoolParameters &other)
        {
            if (this == &other) return *this;
            type = other.type;
            switch(type) {
            case LiquidityPoolType::LIQUIDITY_POOL_CONSTANT_PRODUCT:
                constantProduct = other.constantProduct;
                break;
            }
            return *this;
        }
        LiquidityPoolConstantProductParameters& fillConstantProduct()
        {
            type = LiquidityPoolType::LIQUIDITY_POOL_CONSTANT_PRODUCT;
            new (&constantProduct) LiquidityPoolConstantProductParameters();
            return constantProduct;
        }
    };
    inline QDataStream &operator<<(QDataStream &out, const LiquidityPoolParameters &obj) {
        switch(obj.type) {
        case LiquidityPoolType::LIQUIDITY_POOL_CONSTANT_PRODUCT:
            out << obj.type << obj.constantProduct; break;
        default: out.setStatus(QDataStream::WriteFailed); break;   // no arm: nothing to write
        }
        return out;
    }
    inline QDataStream &operator>>(QDataStream &in, LiquidityPoolParameters &obj) {
        in >> obj.type;
        switch(obj.type) {
        case LiquidityPoolType::LIQUIDITY_POOL_CONSTANT_PRODUCT:
            new (&obj.constantProduct) LiquidityPoolConstantProductParameters();
            in >> obj.constantProduct; break;
        default: in.setStatus(QDataStream::ReadCorruptData); break;
        }
        return in;
    }


    /**
     * ChangeTrustAsset — discriminated union used by ChangeTrustOp (Protocol 18+).
     * Adds POOL_SHARE variant on top of the 3 plain-Asset variants. POOL_SHARE
     * carries full LiquidityPoolParameters so the core can create the pool if
     * it does not yet exist.
     */
    struct ChangeTrustAsset
    {
        AssetType type;
        union {
            struct {
                AssetCode4 assetCode;
                AccountID issuer;
            } alphaNum4;
            struct {
                AssetCode12 assetCode;
                AccountID issuer;
            } alphaNum12;
            // POOL_SHARE branch stored as plain bytes; constructed/destructed
            // via the helper below to handle the non-trivial member.
            quint8 liquidityPoolStorage[sizeof(LiquidityPoolParameters)];
        };

        ChangeTrustAsset() : type(AssetType::ASSET_TYPE_NATIVE)
        {
            // Zeroed like Asset's (the pool storage is the largest arm).
            static_assert(sizeof(liquidityPoolStorage) >= sizeof(alphaNum12)
                          && sizeof(alphaNum12) >= sizeof(alphaNum4), "the pool storage must cover the union");
            std::memset(liquidityPoolStorage, 0, sizeof liquidityPoolStorage);
        }
        ChangeTrustAsset(const ChangeTrustAsset& other) : type(other.type)
        {
            // A copy of a native asset copies no arm: start from zeros here too.
            std::memset(liquidityPoolStorage, 0, sizeof liquidityPoolStorage);
            switch(type) {
            case AssetType::ASSET_TYPE_NATIVE: break;
            case AssetType::ASSET_TYPE_CREDIT_ALPHANUM4:
                alphaNum4 = other.alphaNum4; break;
            case AssetType::ASSET_TYPE_CREDIT_ALPHANUM12:
                alphaNum12 = other.alphaNum12; break;
            case AssetType::ASSET_TYPE_POOL_SHARE:
                new (liquidityPoolStorage) LiquidityPoolParameters(other.liquidityPool());
                break;
            }
        }
        ~ChangeTrustAsset() { clear(); }
        ChangeTrustAsset& operator=(const ChangeTrustAsset& other)
        {
            if (this == &other) return *this;
            clear();
            type = other.type;
            switch(type) {
            case AssetType::ASSET_TYPE_NATIVE: break;
            case AssetType::ASSET_TYPE_CREDIT_ALPHANUM4:
                alphaNum4 = other.alphaNum4; break;
            case AssetType::ASSET_TYPE_CREDIT_ALPHANUM12:
                alphaNum12 = other.alphaNum12; break;
            case AssetType::ASSET_TYPE_POOL_SHARE:
                new (liquidityPoolStorage) LiquidityPoolParameters(other.liquidityPool());
                break;
            }
            return *this;
        }
        LiquidityPoolParameters& liquidityPool()
        {
            return *reinterpret_cast<LiquidityPoolParameters*>(liquidityPoolStorage);
        }
        const LiquidityPoolParameters& liquidityPool() const
        {
            return *reinterpret_cast<const LiquidityPoolParameters*>(liquidityPoolStorage);
        }
        LiquidityPoolParameters& fillLiquidityPool()
        {
            clear();
            type = AssetType::ASSET_TYPE_POOL_SHARE;
            return *new (liquidityPoolStorage) LiquidityPoolParameters();
        }
    private:
        void clear()
        {
            if (type == AssetType::ASSET_TYPE_POOL_SHARE) {
                liquidityPool().~LiquidityPoolParameters();
            }
        }
        friend inline QDataStream &operator>>(QDataStream &in, ChangeTrustAsset &obj);
    };
    inline QDataStream &operator<<(QDataStream &out, const ChangeTrustAsset &obj) {
        switch(obj.type) {
        case AssetType::ASSET_TYPE_NATIVE:
            out << obj.type; break;
        case AssetType::ASSET_TYPE_CREDIT_ALPHANUM4:
            out << obj.type << obj.alphaNum4.assetCode << obj.alphaNum4.issuer; break;
        case AssetType::ASSET_TYPE_CREDIT_ALPHANUM12:
            out << obj.type << obj.alphaNum12.assetCode << obj.alphaNum12.issuer; break;
        case AssetType::ASSET_TYPE_POOL_SHARE:
            out << obj.type << obj.liquidityPool(); break;
        default: out.setStatus(QDataStream::WriteFailed); break;   // no arm: nothing to write
        }
        return out;
    }
    inline QDataStream &operator>>(QDataStream &in, ChangeTrustAsset &obj) {
        obj.clear();
        in >> obj.type;
        switch(obj.type) {
        case AssetType::ASSET_TYPE_NATIVE: break;
        case AssetType::ASSET_TYPE_CREDIT_ALPHANUM4:
            in >> obj.alphaNum4.assetCode >> obj.alphaNum4.issuer; break;
        case AssetType::ASSET_TYPE_CREDIT_ALPHANUM12:
            in >> obj.alphaNum12.assetCode >> obj.alphaNum12.issuer; break;
        case AssetType::ASSET_TYPE_POOL_SHARE:
            new (obj.liquidityPoolStorage) LiquidityPoolParameters();
            in >> obj.liquidityPool(); break;
        default: in.setStatus(QDataStream::ReadCorruptData); break;
        }
        return in;
    }


    /**
     * TrustLineAsset — discriminated union used by TrustLineEntry / LedgerKey::TrustLine.
     * Unlike ChangeTrustAsset, the POOL_SHARE branch carries only the PoolID
     * (the ledger already knows the parameters once the pool exists).
     */
    struct TrustLineAsset
    {
        AssetType type;
        union {
            struct {
                AssetCode4 assetCode;
                AccountID issuer;
            } alphaNum4;
            struct {
                AssetCode12 assetCode;
                AccountID issuer;
            } alphaNum12;
            PoolID liquidityPoolID;
        };

        TrustLineAsset() : type(AssetType::ASSET_TYPE_NATIVE)
        {
            // Zeroed like Asset's (alphaNum12 is the largest arm).
            static_assert(sizeof(alphaNum12) >= sizeof(alphaNum4)
                          && sizeof(alphaNum12) >= sizeof(liquidityPoolID), "alphaNum12 must cover the union");
            std::memset(&alphaNum12, 0, sizeof alphaNum12);
        }
        /**
         * The same asset as a trustline asset: a plain Asset's three arms are
         * the first three here, byte for byte. Implicit, so code that filled a
         * trustline key with an Asset (when the key held one) still compiles.
         * An Asset that names no arm of its own (POOL_SHARE, or garbage) has
         * no pool id to carry and is refused, not turned into pool zero.
         */
        TrustLineAsset(const Asset& asset) : TrustLineAsset()
        {
            switch (asset.type) {
            case AssetType::ASSET_TYPE_NATIVE: break;
            case AssetType::ASSET_TYPE_CREDIT_ALPHANUM4:
                std::memcpy(alphaNum4.assetCode, asset.alphaNum4.assetCode, sizeof alphaNum4.assetCode);
                alphaNum4.issuer = asset.alphaNum4.issuer; break;
            case AssetType::ASSET_TYPE_CREDIT_ALPHANUM12:
                std::memcpy(alphaNum12.assetCode, asset.alphaNum12.assetCode, sizeof alphaNum12.assetCode);
                alphaNum12.issuer = asset.alphaNum12.issuer; break;
            default:
                throw std::runtime_error("this Asset names no arm a trustline asset can hold");
            }
            type = asset.type;
        }
    };
    inline QDataStream &operator<<(QDataStream &out, const TrustLineAsset &obj) {
        switch(obj.type) {
        case AssetType::ASSET_TYPE_NATIVE:
            out << obj.type; break;
        case AssetType::ASSET_TYPE_CREDIT_ALPHANUM4:
            out << obj.type << obj.alphaNum4.assetCode << obj.alphaNum4.issuer; break;
        case AssetType::ASSET_TYPE_CREDIT_ALPHANUM12:
            out << obj.type << obj.alphaNum12.assetCode << obj.alphaNum12.issuer; break;
        case AssetType::ASSET_TYPE_POOL_SHARE:
            out << obj.type << obj.liquidityPoolID; break;
        default: out.setStatus(QDataStream::WriteFailed); break;   // no arm: nothing to write
        }
        return out;
    }
    inline QDataStream &operator>>(QDataStream &in, TrustLineAsset &obj) {
        in >> obj.type;
        switch(obj.type) {
        case AssetType::ASSET_TYPE_NATIVE: break;
        case AssetType::ASSET_TYPE_CREDIT_ALPHANUM4:
            in >> obj.alphaNum4.assetCode >> obj.alphaNum4.issuer; break;
        case AssetType::ASSET_TYPE_CREDIT_ALPHANUM12:
            in >> obj.alphaNum12.assetCode >> obj.alphaNum12.issuer; break;
        case AssetType::ASSET_TYPE_POOL_SHARE:
            in >> obj.liquidityPoolID; break;
        default: in.setStatus(QDataStream::ReadCorruptData); break;
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
        CLAIMABLE_BALANCE = 4,
        LIQUIDITY_POOL = 5,        // Protocol 18 (XDR slot reserved here)
        CONTRACT_DATA = 6,         // CAP-46 Soroban
        CONTRACT_CODE = 7,
        CONFIG_SETTING = 8,
        TTL = 9
    };

    /** CAP-46 — contract storage durability. */
    enum class ContractDataDurability : qint32
    {
        TEMPORARY = 0,
        PERSISTENT = 1
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

    // The canonical `typedef AccountID* SponsorshipDescriptor;` is an XDR
    // optional: a four-byte present flag, then the account when present. It
    // was a bare AccountID, so every sponsorship descriptor on the wire (the
    // signers' sponsors of an AccountEntryExtensionV2, a LedgerEntry's
    // sponsoringID) was read four bytes out of step.
    typedef Optional<AccountID> SponsorshipDescriptor;

    /** AccountEntryExtensionV3 (Protocol 19): when seqNum took its value. */
    struct AccountEntryExtensionV3
    {
        // ExtensionPoint: 0 is its only arm.
        Reserved ext;

        // Ledger number at which `seqNum` took on its present value.
        quint32 seqLedger = 0;

        // Time at which `seqNum` took on its present value.
        TimePoint seqTime = 0;
    };

    inline QDataStream &operator<<(QDataStream &out, const  AccountEntryExtensionV3 &obj) {
        out << obj.ext << obj.seqLedger << obj.seqTime;
        return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  AccountEntryExtensionV3 &obj) {
        in >> obj.ext >> obj.seqLedger >> obj.seqTime;
        return in;
    }

    /*
     * The account extensions nest, each switched on its own `int v`:
     *
     *   AccountEntry.ext             0: void | 1: AccountEntryExtensionV1
     *   AccountEntryExtensionV1.ext  0: void | 2: AccountEntryExtensionV2
     *   AccountEntryExtensionV2.ext  0: void | 3: AccountEntryExtensionV3
     *
     * Each discriminant is kept in its `ext.reserved`, and the arm it names is
     * the member beside it (a plain member, so every one of these copies and
     * destroys itself). Only the arm the discriminant names is read or
     * written; any other discriminant is corrupt data on the way in and a
     * failed write on the way out. The reader used to stop at V1's
     * discriminant, so a real account (Core gives every account that has
     * sent a transaction since Protocol 19 a V3) was read short, with its
     * sponsorship counts and sequence ledger left in the stream.
     */
    struct AccountEntryExtensionV2
    {
        quint32 numSponsored = 0;
        quint32 numSponsoring = 0;
        Array<SponsorshipDescriptor,MAX_SIGNERS> signerSponsoringIDs;

        Reserved ext;                   // 0 or 3
        AccountEntryExtensionV3 v3;     // case 3
    };

    inline QDataStream &operator<<(QDataStream &out, const  AccountEntryExtensionV2 &obj) {
        if (obj.ext.reserved != 0 && obj.ext.reserved != 3) {
            out.setStatus(QDataStream::WriteFailed);
            return out;
        }
        out << obj.numSponsored<< obj.numSponsoring<< obj.signerSponsoringIDs<< obj.ext.reserved;
        if (obj.ext.reserved == 3)
            out << obj.v3;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  AccountEntryExtensionV2 &obj) {
       in >> obj.numSponsored>> obj.numSponsoring>> obj.signerSponsoringIDs>> obj.ext.reserved;
       obj.v3 = AccountEntryExtensionV3();
       switch(obj.ext.reserved){
       case 0: break;
       case 3: in >> obj.v3; break;
       default: in.setStatus(QDataStream::ReadCorruptData); break;
       }
       return in;
    }

    struct AccountEntryExtensionV1
    {
        Liabilities liabilities = {};
        Reserved ext;                   // 0 or 2
        AccountEntryExtensionV2 v2;     // case 2
    };

    inline QDataStream &operator<<(QDataStream &out, const  AccountEntryExtensionV1 &obj) {
        if (obj.ext.reserved != 0 && obj.ext.reserved != 2) {
            out.setStatus(QDataStream::WriteFailed);
            return out;
        }
        out << obj.liabilities << obj.ext.reserved;
        if (obj.ext.reserved == 2)
            out << obj.v2;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  AccountEntryExtensionV1 &obj) {
       in >> obj.liabilities >> obj.ext.reserved;
       obj.v2 = AccountEntryExtensionV2();
       switch(obj.ext.reserved){
       case 0: break;
       case 2: in >> obj.v2; break;
       default: in.setStatus(QDataStream::ReadCorruptData); break;
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
        qint64 balance = 0;            // in stroops
        SequenceNumber seqNum = 0;    // last sequence number used for this account
        quint32 numSubEntries = 0;     // number of sub-entries this account has
                                  // drives the reserve
        Optional<AccountID> inflationDest; // Account to vote for during inflation
        quint32 flags = 0;             // see AccountFlags

        string32 homeDomain; // can be used for reverse federation and memo lookup

        // fields used for signatures
        // thresholds stores unsigned bytes: [weight of master|low|medium|high]
        Thresholds thresholds = {};

        Array<Signer,MAX_SIGNERS> signers; //max : <20>; // possible signers for this account

        Reserved ext;                   // 0 or 1 (see AccountEntryExtensionV2)
        AccountEntryExtensionV1 v1;     // case 1
    };

    inline QDataStream &operator<<(QDataStream &out, const  AccountEntry &obj) {
        if (obj.ext.reserved != 0 && obj.ext.reserved != 1) {
            out.setStatus(QDataStream::WriteFailed);
            return out;
        }
        out << obj.accountID<< obj.balance << obj.seqNum << obj.numSubEntries << obj.inflationDest << obj.flags << obj.homeDomain << obj.thresholds << obj.signers << obj.ext.reserved;
        if (obj.ext.reserved == 1)
            out << obj.v1;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  AccountEntry &obj) {

       in >> obj.accountID>> obj.balance >> obj.seqNum >> obj.numSubEntries >> obj.inflationDest >> obj.flags >> obj.homeDomain >> obj.thresholds >> obj.signers >> obj.ext.reserved;
       obj.v1 = AccountEntryExtensionV1();
       switch(obj.ext.reserved){
       case 0: break;
       case 1: in >> obj.v1; break;
       default: in.setStatus(QDataStream::ReadCorruptData); break;
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
        // absBefore is the trivial arm: zeroed so no time is ever read from, or
        // written as, whatever the memory held.
        ClaimPredicate():type(ClaimPredicateType::CLAIM_PREDICATE_UNCONDITIONAL), absBefore(0)
        {
        }
    private:
        friend inline QDataStream &operator>>(QDataStream &in, ClaimPredicate &obj);
        // Destroys the live arm and falls back to the trivial UNCONDITIONAL.
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
            type = ClaimPredicateType::CLAIM_PREDICATE_UNCONDITIONAL;
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
        if (this == &obj)
            return *this;       // the switch below destroys what it would copy from
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

    /**
     * Core refuses a claimant whose predicate is nested deeper than this
     * (validatePredicate: the claimant's own predicate is depth 1, and depth 5
     * is refused), so every predicate on the ledger fits. The reader had no
     * limit at all: a hostile balance or envelope nested a few thousand NOTs
     * deep, eight bytes a level, and the recursion overflowed the stack, a
     * crash no catch() can intercept. Deeper nesting is now corrupt data on
     * the way in and a failed write on the way out.
     */
    static const int MAX_CLAIM_PREDICATE_DEPTH = 4;

    /** Whether `p`, itself at `depth`, nests no deeper than the limit. It
     *  stops descending there, so its own recursion is bounded too. */
    inline bool claimPredicateFitsDepth(const ClaimPredicate& p, int depth)
    {
        if (depth > MAX_CLAIM_PREDICATE_DEPTH)
            return false;
        const QVector<ClaimPredicate>* operands = nullptr;
        switch (p.type) {
        case ClaimPredicateType::CLAIM_PREDICATE_AND: operands = &p.andPredicates.value; break;
        case ClaimPredicateType::CLAIM_PREDICATE_OR:  operands = &p.orPredicates.value; break;
        case ClaimPredicateType::CLAIM_PREDICATE_NOT: operands = &p.notPredicate.value; break;
        default: return true;
        }
        for (const ClaimPredicate& operand : *operands)
            if (!claimPredicateFitsDepth(operand, depth + 1))
                return false;
        return true;
    }

    /** How deep the predicate being read on this thread is (1 for the outermost). */
    inline int& claimPredicateReadDepth() { static thread_local int depth = 0; return depth; }
    struct ClaimPredicateReadDepthGuard
    {
        ClaimPredicateReadDepthGuard() { ++claimPredicateReadDepth(); }
        ~ClaimPredicateReadDepthGuard() { --claimPredicateReadDepth(); }
        bool tooDeep() const { return claimPredicateReadDepth() > MAX_CLAIM_PREDICATE_DEPTH; }
    };

    inline QDataStream &operator<<(QDataStream &out, const  ClaimPredicate &obj) {
        // The whole tree, before anything is written (see MAX_CLAIM_PREDICATE_DEPTH).
        if (!claimPredicateFitsDepth(obj, 1)) {
            out.setStatus(QDataStream::WriteFailed);
            return out;
        }
        switch(obj.type){
        case ClaimPredicateType::CLAIM_PREDICATE_UNCONDITIONAL:
            out << obj.type; break;
        case ClaimPredicateType::CLAIM_PREDICATE_AND:
            out << obj.type << obj.andPredicates; break;
        case ClaimPredicateType::CLAIM_PREDICATE_OR:
            out << obj.type << obj.orPredicates; break;
        case ClaimPredicateType::CLAIM_PREDICATE_NOT:
            // `ClaimPredicate* notPredicate`: an optional, which is the same
            // four-byte 0 or 1 as a count of at most one.
            if (obj.notPredicate.value.size() > 1) {
                out.setStatus(QDataStream::WriteFailed);
                break;
            }
            out << obj.type << obj.notPredicate; break;
        case ClaimPredicateType::CLAIM_PREDICATE_BEFORE_ABSOLUTE_TIME:
            out << obj.type << obj.absBefore; break;// Predicate will be true if closeTime < absBefore
        case ClaimPredicateType::CLAIM_PREDICATE_BEFORE_RELATIVE_TIME:
            out << obj.type << obj.relBefore; break;// Seconds since closeTime of the ledger in which the
        // A type with no arm went out as a bare discriminant.
        default: out.setStatus(QDataStream::WriteFailed); break;
        }


       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  ClaimPredicate &obj) {
        // The arm is built fresh below. It used to be built over the live
        // one, so a predicate read into twice leaked the first one's operands.
        obj.clear();
        // Nested deeper than Core allows: nothing more is read, whatever the
        // type (see MAX_CLAIM_PREDICATE_DEPTH).
        const ClaimPredicateReadDepthGuard depth;
        if (depth.tooDeep()) {
            in.setStatus(QDataStream::ReadCorruptData);
            return in;
        }
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
        case ClaimPredicateType::CLAIM_PREDICATE_NOT: {
            new (&obj.notPredicate) Array<ClaimPredicate,1>();
            // An optional: its present flag is 0 or 1 (see xdr::isXdrBool).
            qint32 present = 0;
            in >> present;
            if (!isXdrBool(present)) {
                in.setStatus(QDataStream::ReadCorruptData);
                break;
            }
            if (present) {
                ClaimPredicate operand;
                in >> operand;
                if (in.status() == QDataStream::Ok)
                    obj.notPredicate.append(operand);
            }
            break;
        }
        case ClaimPredicateType::CLAIM_PREDICATE_BEFORE_ABSOLUTE_TIME:
            in>> obj.absBefore; break;// Predicate will be true if closeTime < absBefore
        case ClaimPredicateType::CLAIM_PREDICATE_BEFORE_RELATIVE_TIME:
            in>> obj.relBefore; break;// Seconds since closeTime of the ledger in which the
        default: in.setStatus(QDataStream::ReadCorruptData); break;
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
        switch(obj.type){
        case ClaimantType::CLAIMANT_TYPE_V0:
            out << obj.type << obj.v0.destination << obj.v0.predicate; break;
        default: out.setStatus(QDataStream::WriteFailed); break;   // no arm: nothing to write
        }
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  Claimant &obj) {
        in >> obj.type;

        switch(obj.type){
        case ClaimantType::CLAIMANT_TYPE_V0:
            in>> obj.v0.destination >> obj.v0.predicate; break;
        default: in.setStatus(QDataStream::ReadCorruptData); break;
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
        // V0 is the only arm and the one the constructor builds: the type was
        // left unset, so a key filled with just the hash wrote four stray bytes
        // as its discriminant, and no hash after them.
        ClaimableBalanceID() : type(ClaimableBalanceIDType::CLAIMABLE_BALANCE_ID_TYPE_V0)
        {
            new (&v0) Hash();
        }
    };

    inline QDataStream &operator<<(QDataStream &out, const  ClaimableBalanceID &obj) {
        switch(obj.type){
        case ClaimableBalanceIDType::CLAIMABLE_BALANCE_ID_TYPE_V0:
            out << obj.type << obj.v0; break;
        // Any other type used to go out as a four-byte id with no hash, which
        // no other SDK can decode and Core rejects. Nothing is written now.
        default: out.setStatus(QDataStream::WriteFailed); break;
        }
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  ClaimableBalanceID &obj) {
        in >> obj.type;

        switch(obj.type){
        case ClaimableBalanceIDType::CLAIMABLE_BALANCE_ID_TYPE_V0:
            in>> obj.v0 ; break;
        default: in.setStatus(QDataStream::ReadCorruptData); break;
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
        // reserved for future use: 0 is its only arm
        Reserved ext;

        quint32 flags = 0; // see ClaimableBalanceFlags
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
        qint64 amount = 0;

        // `union switch (int v)`: 0 is void, 1 carries v1 (the member below,
        // read and written only when ext.reserved says 1). Any other value
        // is corrupt data; it was read as "no extension".
        Reserved ext;
        ClaimableBalanceEntryExtensionV1 v1;
    };

    inline QDataStream &operator<<(QDataStream &out, const  ClaimableBalanceEntry &obj) {
        if (obj.ext.reserved != 0 && obj.ext.reserved != 1) {
            out.setStatus(QDataStream::WriteFailed);
            return out;
        }
        out << obj.balanceID<< obj.claimants<< obj.asset<< obj.amount<< obj.ext.reserved;
        if (obj.ext.reserved == 1)
            out << obj.v1;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  ClaimableBalanceEntry &obj) {
        in >> obj.balanceID>> obj.claimants>> obj.asset>> obj.amount>> obj.ext.reserved;
        obj.v1 = ClaimableBalanceEntryExtensionV1();
        switch(obj.ext.reserved)
        {
        case 0: break;
        case 1: in >> obj.v1; break;
        default: in.setStatus(QDataStream::ReadCorruptData); break;
        }
       return in;
    }



    struct LedgerEntryExtensionV1
    {
        SponsorshipDescriptor sponsoringID;     // optional: see SponsorshipDescriptor

        // reserved for future use: 0 is its only arm
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

    /** TrustLineEntryExtensionV2 (Protocol 18): how many pools use the line. */
    struct TrustLineEntryExtensionV2
    {
        qint32 liquidityPoolUseCount = 0;

        // 0 is its only arm
        Reserved ext;
    };

    inline QDataStream &operator<<(QDataStream &out, const  TrustLineEntryExtensionV2 &obj) {
        out << obj.liquidityPoolUseCount << obj.ext;
        return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  TrustLineEntryExtensionV2 &obj) {
        in >> obj.liquidityPoolUseCount >> obj.ext;
        return in;
    }

    /*
     * The trustline extensions nest like the account ones:
     *
     *   TrustLineEntry.ext      0: void | 1: V1 { liabilities, ext }
     *   TrustLineEntry.V1.ext   0: void | 2: TrustLineEntryExtensionV2
     *
     * The asset is a TrustLineAsset (a pool share is a trustline too), and
     * V1's own extension used to be read as a void Reserved, so a trustline
     * that a pool used was read four bytes short.
     */
    struct TrustLineEntry
    {
        AccountID accountID; // account this trustline belongs to
        TrustLineAsset asset;    // type of asset (with issuer), or the pool of a pool share
        qint64 balance = 0;       // how much of this asset the user has.
                             // Asset defines the unit for this;

        qint64 limit = 0;  // balance cannot be above this
        quint32 flags = 0; // see TrustLineFlags

        Reserved ext;       // 0 or 1
        struct V1{
            Liabilities liabilities = {};
            Reserved ext;                   // 0 or 2
            TrustLineEntryExtensionV2 v2;   // case 2
        };
        V1 v1;              // case 1
    };
    inline QDataStream &operator<<(QDataStream &out, const  TrustLineEntry::V1 &obj) {
        if (obj.ext.reserved != 0 && obj.ext.reserved != 2) {
            out.setStatus(QDataStream::WriteFailed);
            return out;
        }
        out << obj.liabilities << obj.ext.reserved;
        if (obj.ext.reserved == 2)
            out << obj.v2;
        return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  TrustLineEntry::V1 &obj) {
        in >> obj.liabilities >> obj.ext.reserved;
        obj.v2 = TrustLineEntryExtensionV2();
        switch(obj.ext.reserved){
        case 0: break;
        case 2: in >> obj.v2; break;
        default: in.setStatus(QDataStream::ReadCorruptData); break;
        }
        return in;
    }

    inline QDataStream &operator<<(QDataStream &out, const  TrustLineEntry &obj) {
        if (obj.ext.reserved != 0 && obj.ext.reserved != 1) {
            out.setStatus(QDataStream::WriteFailed);
            return out;
        }
        out << obj.accountID << obj.asset << obj.balance << obj.limit << obj.flags << obj.ext.reserved;
        if (obj.ext.reserved == 1)
            out << obj.v1;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  TrustLineEntry &obj) {
       in >> obj.accountID >> obj.asset >> obj.balance >> obj.limit >> obj.flags >> obj.ext.reserved;
       obj.v1 = TrustLineEntry::V1();
       switch(obj.ext.reserved){
       case 0: break;
       case 1: in >> obj.v1; break;
       default: in.setStatus(QDataStream::ReadCorruptData); break;
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
       if (obj.ext.reserved != 0) in.setStatus(QDataStream::ReadCorruptData);   // ext is void: 0 only
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

        // reserved for future use: 0 is its only arm
        Reserved ext;
    };
    // The ext was neither written nor read: every DataEntry was four bytes
    // short, and the next field of whatever held it was read from its ext.
    inline QDataStream &operator<<(QDataStream &out, const  DataEntry &obj) {
        out << obj.accountID << obj.dataName << obj.dataValue << obj.ext;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  DataEntry &obj) {
       in >> obj.accountID >> obj.dataName >> obj.dataValue >> obj.ext;
       return in;
    }


    /** LiquidityPoolEntry (CAP-38): a pool and its reserves. */
    struct LiquidityPoolEntry
    {
        PoolID liquidityPoolID = {};

        LiquidityPoolType type = LiquidityPoolType::LIQUIDITY_POOL_CONSTANT_PRODUCT;
        struct ConstantProduct
        {
            LiquidityPoolConstantProductParameters params;

            qint64 reserveA = 0;        // amount of A in the pool
            qint64 reserveB = 0;        // amount of B in the pool
            qint64 totalPoolShares = 0; // total number of pool shares issued
            qint64 poolSharesTrustLineCount = 0; // number of trust lines for the
                                                 // associated pool shares
        };
        ConstantProduct constantProduct;    // case LIQUIDITY_POOL_CONSTANT_PRODUCT
    };

    inline QDataStream &operator<<(QDataStream &out, const  LiquidityPoolEntry &obj) {
        switch (obj.type) {
        case LiquidityPoolType::LIQUIDITY_POOL_CONSTANT_PRODUCT:
            out << obj.liquidityPoolID << obj.type << obj.constantProduct.params
                << obj.constantProduct.reserveA << obj.constantProduct.reserveB
                << obj.constantProduct.totalPoolShares << obj.constantProduct.poolSharesTrustLineCount;
            break;
        default: out.setStatus(QDataStream::WriteFailed); break;   // no arm: nothing to write
        }
        return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  LiquidityPoolEntry &obj) {
        in >> obj.liquidityPoolID >> obj.type;
        switch (obj.type) {
        case LiquidityPoolType::LIQUIDITY_POOL_CONSTANT_PRODUCT:
            in >> obj.constantProduct.params
               >> obj.constantProduct.reserveA >> obj.constantProduct.reserveB
               >> obj.constantProduct.totalPoolShares >> obj.constantProduct.poolSharesTrustLineCount;
            break;
        default: in.setStatus(QDataStream::ReadCorruptData); break;
        }
        return in;
    }


    /*
     * A ledger entry: its data (the member `type` names) and its extension
     * (`ext.reserved`: 0 void, 1 the sponsor in v1).
     *
     * The data union used to be a C++ union that no constructor built and no
     * destructor tore down: an account read into it was read into memory that
     * held no QVector, and the extension that follows the data was neither
     * read nor written. Every arm is a plain member now, and only the one
     * `type` names is read or written. The Soroban entries (contract data and
     * code, config settings, TTLs) have no model here and are refused rather
     * than skipped.
     */
    struct LedgerEntry
    {
        quint32 lastModifiedLedgerSeq = 0; // ledger the LedgerEntry was last changed
        LedgerEntryType type = LedgerEntryType::ACCOUNT;
        AccountEntry account;
        TrustLineEntry trustLine;
        OfferEntry offer;
        DataEntry data;
        ClaimableBalanceEntry claimableBalance;
        LiquidityPoolEntry liquidityPool;

        Reserved ext;               // 0 or 1
        LedgerEntryExtensionV1 v1;  // case 1
    };

    inline QDataStream &operator<<(QDataStream &out, const  LedgerEntry &obj) {
        switch (obj.type) {
        case LedgerEntryType::ACCOUNT:
        case LedgerEntryType::TRUSTLINE:
        case LedgerEntryType::OFFER:
        case LedgerEntryType::DATA:
        case LedgerEntryType::CLAIMABLE_BALANCE:
        case LedgerEntryType::LIQUIDITY_POOL:
            break;
        default:                    // no model here: nothing to write
            out.setStatus(QDataStream::WriteFailed);
            return out;
        }
        if (obj.ext.reserved != 0 && obj.ext.reserved != 1) {
            out.setStatus(QDataStream::WriteFailed);
            return out;
        }
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
        case LedgerEntryType::LIQUIDITY_POOL:
            out << obj.liquidityPool; break;
        default: break;
        }
        out << obj.ext.reserved;
        if (obj.ext.reserved == 1)
            out << obj.v1;
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
        case LedgerEntryType::LIQUIDITY_POOL:
            in >> obj.liquidityPool; break;
        default:
            in.setStatus(QDataStream::ReadCorruptData);
            return in;
        }
        in >> obj.ext.reserved;
        obj.v1 = LedgerEntryExtensionV1();
        switch (obj.ext.reserved) {
        case 0: break;
        case 1: in >> obj.v1; break;
        default: in.setStatus(QDataStream::ReadCorruptData); break;
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
        struct TrustLine//case TRUSTLINE:
        {
            AccountID accountID;
            // A TrustLineAsset, as the XDR says: it was a plain Asset, which
            // cannot hold a pool share, so the key of a pool-share trustline
            // (a valid revokeSponsorship target) could not be read or made.
            TrustLineAsset asset;
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
        struct LiquidityPool//case LIQUIDITY_POOL:
        {
            PoolID liquidityPoolID;
        };
        union
        {
            Account account;
            TrustLine trustLine;
            Offer offer;
            Data data;
            ClaimableBalance claimableBalance;
            LiquidityPool liquidityPool;
        };
        LedgerKey():type(LedgerEntryType::ACCOUNT)
        {
            new (&account) Account();
        }
    private:
        friend inline QDataStream &operator>>(QDataStream &in, LedgerKey &obj);
        // Destroys the live arm. The caller builds the next one (the arms
        // other than Data are trivial, but Data holds a QVector).
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
           case LedgerEntryType::LIQUIDITY_POOL:
                (liquidityPool).~LiquidityPool(); break;
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
        LiquidityPool& fillLiquidityPool()
        {
            clear();
            new (&liquidityPool) LiquidityPool();
            type=LedgerEntryType::LIQUIDITY_POOL;
            return liquidityPool;
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
            type = obj.type;
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
           case LedgerEntryType::LIQUIDITY_POOL:
                new (&liquidityPool) LiquidityPool();
                liquidityPool= obj.liquidityPool; break;
            // The Soroban types have no arm in this struct (SorobanLedgerKey
            // carries them). Without a default, GCC's -Wswitch warned about
            // them in every file that includes this one.
            default: break;
            }
        }
        const LedgerKey& operator=(const LedgerKey &obj)
        {
            if (this == &obj)
                return *this;   // clear() would destroy what it copies from
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
            case LedgerEntryType::LIQUIDITY_POOL:
                new (&liquidityPool) LiquidityPool();
                liquidityPool= obj.liquidityPool; break;
            default: break;     // no arm here: see the copy constructor
            }
            return *this;
        }

    };
    inline QDataStream &operator<<(QDataStream &out, const  LedgerKey &obj) {
        switch(obj.type){
        case LedgerEntryType::ACCOUNT:
            out << obj.type << obj.account.accountID; break;
        case LedgerEntryType::TRUSTLINE:
            out << obj.type << obj.trustLine.accountID << obj.trustLine.asset; break;
        case LedgerEntryType::OFFER:
            out << obj.type << obj.offer.sellerID << obj.offer.offerID; break;
        case LedgerEntryType::DATA:
            out << obj.type << obj.data.accountID<<obj.data.dataName; break;
       case LedgerEntryType::CLAIMABLE_BALANCE:
            out << obj.type << obj.claimableBalance.balanceID; break;
       case LedgerEntryType::LIQUIDITY_POOL:
            out << obj.type << obj.liquidityPool.liquidityPoolID; break;
        // The Soroban keys have their own writer (SorobanLedgerKey); any other
        // type went out as a bare discriminant.
        default: out.setStatus(QDataStream::WriteFailed); break;
        }

       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  LedgerKey &obj) {
        // Every arm below is built fresh: it used to be built over the live
        // one, so a key read into twice leaked a Data key's name.
        obj.clear();
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
       case LedgerEntryType::LIQUIDITY_POOL:
            new (&obj.liquidityPool) LedgerKey::LiquidityPool();
            in >> obj.liquidityPool.liquidityPoolID; break;
        default:    // the Soroban keys have their own reader (SorobanLedgerKey)
            in.setStatus(QDataStream::ReadCorruptData); break;
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
        ENVELOPE_TYPE_OP_ID = 6,
        ENVELOPE_TYPE_POOL_REVOKE_OP_ID = 7,
        ENVELOPE_TYPE_CONTRACT_ID = 8,
        ENVELOPE_TYPE_SOROBAN_AUTHORIZATION = 9,
        // CAP-71 (Protocol 27) — same preimage as the above plus the signer's
        // SCAddress, used by SOROBAN_CREDENTIALS_ADDRESS_V2.
        ENVELOPE_TYPE_SOROBAN_AUTHORIZATION_WITH_ADDRESS = 10
    };
}



#endif // STELLARLEDGERENTRIES_H
