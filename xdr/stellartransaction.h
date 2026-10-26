#ifndef STELLARTRANSACTION_H
#define STELLARTRANSACTION_H

#include "stellarledgerentries.h"
#include "sorobanops.h"
#include "xdrhelper.h"

namespace stellar
{
    using namespace xdr;
    // Source or destination of a payment operation
    struct MuxedAccount
    {
        struct med25519_t{
            quint64 id;
            uint256 ed25519;
            bool operator ==(const med25519_t& other) const
            {
                return other.id==id && (memcmp(other.ed25519, ed25519, sizeof(ed25519))==0);
            }
        };
        CryptoKeyType type;
        union{
            uint256 ed25519;
            med25519_t med25519;
        };
        bool operator ==(const MuxedAccount& other) const
        {
            if(isMuxed(other.type))
            {
                return other.type == type && other.med25519==med25519;
            }
            else
            {
                return other.type == type && (memcmp(other.ed25519, ed25519, sizeof(ed25519))==0);
            }

        }
    };
    inline QDataStream &operator<<(QDataStream &out, const  MuxedAccount::med25519_t &obj) {
        out << obj.id<< obj.ed25519;
        return out;
    }
    inline QDataStream &operator>>(QDataStream &in,  MuxedAccount::med25519_t &obj) {
        in >> obj.id>> obj.ed25519;
        return in;
    }

    inline QDataStream &operator<<(QDataStream &out, const  MuxedAccount &obj) {
        switch(obj.type){
        case CryptoKeyType::KEY_TYPE_ED25519:
            out << obj.type << obj.ed25519; break;
        case CryptoKeyType::KEY_TYPE_MUXED_ED25519:
            out << obj.type << obj.med25519; break;
        // Two arms only, as in the reader. Any other key type used to be
        // written as one of them (the muxed bit picked which), under its own
        // discriminant: bytes that no one can decode, hashed and signed.
        default: out.setStatus(QDataStream::WriteFailed); break;
        }
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  MuxedAccount &obj) {
        in >> obj.type;
        // Two arms only. Any other key type (a pre-auth or hash-x type, or
        // one of those with the muxed bit) used to be read as one of them,
        // though js and py refuse it and Core cannot decode it.
        switch(obj.type){
        case CryptoKeyType::KEY_TYPE_ED25519:
            in >> obj.ed25519; break;
        case CryptoKeyType::KEY_TYPE_MUXED_ED25519:
            in >> obj.med25519; break;
        default: in.setStatus(QDataStream::ReadCorruptData); break;
        }
       return in;
    }

    struct DecoratedSignature
    {
        SignatureHint hint;  // last 4 bytes of the public key, used as a hint
        Signature signature; // actual signature
    };
    inline QDataStream &operator<<(QDataStream &out, const  DecoratedSignature &obj) {
        out << obj.hint << obj.signature;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  DecoratedSignature &obj) {
       in >> obj.hint >> obj.signature;
       return in;
    }

    enum class OperationType : qint32
    {
        CREATE_ACCOUNT = 0,
        PAYMENT = 1,
        PATH_PAYMENT_STRICT_RECEIVE = 2,
        MANAGE_SELL_OFFER = 3,
        CREATE_PASSIVE_SELL_OFFER = 4,
        SET_OPTIONS = 5,
        CHANGE_TRUST = 6,
        ALLOW_TRUST = 7,
        ACCOUNT_MERGE = 8,
        INFLATION = 9,
        MANAGE_DATA = 10,
        BUMP_SEQUENCE = 11,
        MANAGE_BUY_OFFER = 12,
        PATH_PAYMENT_STRICT_SEND = 13,
        CREATE_CLAIMABLE_BALANCE = 14,
        CLAIM_CLAIMABLE_BALANCE = 15,
        BEGIN_SPONSORING_FUTURE_RESERVES = 16,
        END_SPONSORING_FUTURE_RESERVES = 17,
        REVOKE_SPONSORSHIP = 18,
        CLAWBACK = 19,
        CLAWBACK_CLAIMABLE_BALANCE = 20,
        SET_TRUST_LINE_FLAGS = 21,
        LIQUIDITY_POOL_DEPOSIT = 22,
        LIQUIDITY_POOL_WITHDRAW = 23,
        INVOKE_HOST_FUNCTION = 24,
        EXTEND_FOOTPRINT_TTL = 25,
        RESTORE_FOOTPRINT = 26
    };

    /* CreateAccount
    Creates and funds a new account with the specified starting balance.
    Threshold: med
    Result: CreateAccountResult
    */

    struct CreateAccountOp
    {
        AccountID destination; // account to create
        qint64 startingBalance; // amount they end up with
    };
    inline QDataStream &operator<<(QDataStream &out, const  CreateAccountOp &obj) {
        out << obj.destination << obj.startingBalance;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  CreateAccountOp &obj) {
       in >> obj.destination >> obj.startingBalance;
       return in;
    }


    /* Payment
        Send an amount in specified asset to a destination account.
        Threshold: med
        Result: PaymentResult
    */
    struct PaymentOp
    {
        MuxedAccount destination; // recipient of the payment
        Asset asset;           // what they end up with
        qint64 amount;          // amount they end up with
    };
    inline QDataStream &operator<<(QDataStream &out, const  PaymentOp &obj) {
        out << obj.destination << obj.asset << obj.amount;
       return out;
    }

   inline  QDataStream &operator>>(QDataStream &in,  PaymentOp &obj) {
       in >> obj.destination >> obj.asset >> obj.amount;
       return in;
    }

    /* PathPaymentStrictReceive
    send an amount to a destination account through a path.
    (up to sendMax, sendAsset)
    (X0, Path[0]) .. (Xn, Path[n])
    (destAmount, destAsset)
    Threshold: med
    Result: PathPaymentStrictReceiveResult
    */
    struct PathPaymentStrictReceiveOp
    {
        Asset sendAsset; // asset we pay with
        qint64 sendMax;   // the maximum amount of sendAsset to
                         // send (excluding fees).
                         // The operation will fail if can't be met

        MuxedAccount destination; // recipient of the payment
        Asset destAsset;       // what they end up with
        qint64 destAmount;      // amount they end up with

        Array<Asset,5> path; //max <5>; // additional hops it must go through to get there
    };

    inline QDataStream &operator<<(QDataStream &out, const  PathPaymentStrictReceiveOp &obj) {
        out << obj.sendAsset << obj.sendMax << obj.destination << obj.destAsset << obj.destAmount<< obj.path;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  PathPaymentStrictReceiveOp &obj) {
       in >> obj.sendAsset >> obj.sendMax >> obj.destination >> obj.destAsset >> obj.destAmount >> obj.path;
       return in;
    }

    /* PathPaymentStrictSend
    send an amount to a destination account through a path.
    (sendMax, sendAsset)
    (X0, Path[0]) .. (Xn, Path[n])
    (at least destAmount, destAsset)
    Threshold: med
    Result: PathPaymentStrictSendResult
    */
    struct PathPaymentStrictSendOp
    {
        Asset sendAsset; // asset we pay with
        qint64 sendAmount;   // amount of sendAsset to send (excluding fees)

        MuxedAccount destination; // recipient of the payment
        Asset destAsset;       // what they end up with
        qint64 destMin;         // the minimum amount of dest asset to
                                // be received
                                // The operation will fail if it can't be met

        Array<Asset,5> path; //max <5>; // additional hops it must go through to get there
    };

    inline QDataStream &operator<<(QDataStream &out, const  PathPaymentStrictSendOp &obj) {
        out << obj.sendAsset << obj.sendAmount << obj.destination << obj.destAsset << obj.destMin<< obj.path;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  PathPaymentStrictSendOp &obj) {
       in >> obj.sendAsset >> obj.sendAmount >> obj.destination >> obj.destAsset >> obj.destMin >> obj.path;
       return in;
    }


    /* Creates, updates or deletes an offer
    Threshold: med
    Result: ManageSellOfferResult
    */
    struct ManageSellOfferOp
    {
        Asset selling;
        Asset buying;
        qint64 amount; // amount being sold. if set to 0, delete the offer
        Price price;  // price of thing being sold in terms of what you are buying

        // 0=create a new offer, otherwise edit an existing offer
        qint64 offerID;
    };
    inline QDataStream &operator<<(QDataStream &out, const  ManageSellOfferOp &obj) {
        out << obj.selling << obj.buying << obj.amount << obj.price << obj.offerID;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  ManageSellOfferOp &obj) {
       in >> obj.selling >> obj.buying >> obj.amount >> obj.price >> obj.offerID;
       return in;
    }


    /* Creates, updates or deletes an offer with amount in terms of buying asset
    Threshold: med
    Result: ManageBuyOfferResult
    */
    struct ManageBuyOfferOp
    {
        Asset selling;
        Asset buying;
        qint64 buyAmount; // amount being bought. if set to 0, delete the offer
        Price price;     // price of thing being bought in terms of what you are
                         // selling

        // 0=create a new offer, otherwise edit an existing offer
        qint64 offerID;
    };

    inline QDataStream &operator<<(QDataStream &out, const  ManageBuyOfferOp &obj) {
        out << obj.selling << obj.buying << obj.buyAmount << obj.price << obj.offerID;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  ManageBuyOfferOp &obj) {
       in >> obj.selling >> obj.buying >> obj.buyAmount >> obj.price >> obj.offerID;
       return in;
    }



    /* Creates an offer that doesn't take offers of the same price
    Threshold: med
    Result: CreatePassiveSellOfferResult
    */
    struct CreatePassiveSellOfferOp
    {
        Asset selling; // A
        Asset buying;  // B
        qint64 amount;  // amount taker gets. if set to 0, delete the offer
        Price price;   // cost of A in terms of B
    };
    inline QDataStream &operator<<(QDataStream &out, const  CreatePassiveSellOfferOp &obj) {
        out << obj.selling << obj.buying << obj.amount << obj.price;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  CreatePassiveSellOfferOp &obj) {
       in >> obj.selling >> obj.buying >> obj.amount >> obj.price;
       return in;
    }


    /* Set Account Options
        updates "AccountEntry" fields.
        note: updating thresholds or signers requires high threshold
        Threshold: med or high
        Result: SetOptionsResult
    */


    struct SetOptionsOp
    {
        Optional<AccountID> inflationDest; // sets the inflation destination

        Optional<quint32> clearFlags; // which flags to clear
        Optional<quint32>setFlags;   // which flags to set

        // account threshold manipulation
        Optional<quint32> masterWeight; // weight of the master account
        Optional<quint32> lowThreshold;
        Optional<quint32> medThreshold;
        Optional<quint32> highThreshold;

        Optional<string32> homeDomain; // sets the home domain

        // Add, update or remove a signer for the account
        // signer is deleted if the weight is 0
        Optional<Signer> signer;
    };
    inline QDataStream &operator<<(QDataStream &out, const  SetOptionsOp &obj) {
        out << obj.inflationDest << obj.clearFlags << obj.setFlags << obj.masterWeight << obj.lowThreshold << obj.medThreshold<< obj.highThreshold<< obj.homeDomain << obj.signer;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  SetOptionsOp &obj) {
       in >> obj.inflationDest >> obj.clearFlags >> obj.setFlags >> obj.masterWeight >> obj.lowThreshold >> obj.medThreshold >> obj.highThreshold >> obj.homeDomain >> obj.signer;
       return in;
    }


    /* Creates, updates or deletes a trust line
        Threshold: med
        Result: ChangeTrustResult
    */
    /**
     * CAP-38: line is a ChangeTrustAsset union (adds POOL_SHARE variant).
     * Wire format is identical to the legacy Asset union for the 3 pre-existing
     * variants (NATIVE/ALPHANUM4/ALPHANUM12).
     */
    struct ChangeTrustOp
    {
        ChangeTrustAsset line;

        // if limit is set to 0, deletes the trust line
        qint64 limit;
    };
    inline QDataStream &operator<<(QDataStream &out, const  ChangeTrustOp &obj) {
        out << obj.line << obj.limit;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  ChangeTrustOp &obj) {
       in >> obj.line >> obj.limit;
       return in;
    }

    /* Updates the "authorized" flag of an existing trust line
       this is called by the issuer of the related asset.
       note that authorize can only be set (and not cleared) if
       the issuer account does not have the AUTH_REVOCABLE_FLAG set
       Threshold: low
       Result: AllowTrustResult
    */


    struct AllowTrustOp
    {
        AccountID trustor;
        AssetCode asset;
        quint32 authorize;// 0, or any bitwise combination of the AUTHORIZED_* flags of TrustLineFlags
    };
    // ASSET_TYPE_NATIVE is not allowed
    inline QDataStream &operator<<(QDataStream &out, const  AllowTrustOp &obj) {
        //allow trust op skips issuer field
        switch(obj.asset.type)
        {
            case stellar::AssetType::ASSET_TYPE_CREDIT_ALPHANUM4:
                out << obj.trustor << obj.asset.type << obj.asset.alphaNum4.assetCode;break;
            case stellar::AssetType::ASSET_TYPE_CREDIT_ALPHANUM12:
                out << obj.trustor << obj.asset.type << obj.asset.alphaNum12.assetCode;break;
            default:            // AssetCode has only the two credit arms: nothing to write
                out.setStatus(QDataStream::WriteFailed);
                return out;
        }
        out << obj.authorize;
       return out;
    }
    inline QDataStream &operator>>(QDataStream &in,  AllowTrustOp &obj) {
        //allow trust op skips issuer field
       in >> obj.trustor >> obj.asset.type;
       switch(obj.asset.type)
       {
           case stellar::AssetType::ASSET_TYPE_CREDIT_ALPHANUM4:
               in >> obj.asset.alphaNum4.assetCode;break;
           case stellar::AssetType::ASSET_TYPE_CREDIT_ALPHANUM12:
               in >> obj.asset.alphaNum12.assetCode;break;
           default:     // AssetCode has only the two credit arms
               in.setStatus(QDataStream::ReadCorruptData); break;
       }
       in >> obj.authorize;

       return in;
    }





    /* Inflation
        Runs inflation
    Threshold: low
    Result: InflationResult
    */

    /* AccountMerge
        Transfers native balance to destination account.
        Threshold: high
        Result : AccountMergeResult
    */

    /* ManageData
        Adds, Updates, or Deletes a key value pair associated with a particular
        account.
        Threshold: low
        Result: ManageDataResult
    */

    struct ManageDataOp
    {
        string64 dataName;
        Optional<DataValue> dataValue;   // set to null to clear
    };
    inline QDataStream &operator<<(QDataStream &out, const  ManageDataOp &obj) {
        out << obj.dataName << obj.dataValue;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  ManageDataOp &obj) {
       in >> obj.dataName >> obj.dataValue;
       return in;
    }

    struct BumpSequenceOp
    {
        SequenceNumber bumpTo;
    };

    inline QDataStream &operator<<(QDataStream &out, const  BumpSequenceOp &obj) {
        out << obj.bumpTo;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  BumpSequenceOp &obj) {
       in >> obj.bumpTo;
       return in;
    }

    /* Creates a claimable balance entry
        Threshold: med
        Result: CreateClaimableBalanceResult
    */
    struct CreateClaimableBalanceOp
    {
        Asset asset;
        qint64 amount;
        Array<Claimant,10> claimants;
    };

    inline QDataStream &operator<<(QDataStream &out, const  CreateClaimableBalanceOp &obj) {
        out << obj.asset << obj.amount << obj.claimants;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  CreateClaimableBalanceOp &obj) {
       in >> obj.asset >> obj.amount >> obj.claimants;
       return in;
    }

    /* Claims a claimable balance entry
        Threshold: low
        Result: ClaimClaimableBalanceResult
    */
    struct ClaimClaimableBalanceOp
    {
        ClaimableBalanceID balanceID;
    };

    inline QDataStream &operator<<(QDataStream &out, const  ClaimClaimableBalanceOp &obj) {
        out << obj.balanceID;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  ClaimClaimableBalanceOp &obj) {
       in >> obj.balanceID;
       return in;
    }

    /* BeginSponsoringFutureReserves
        Establishes the is-sponsoring-future-reserves-for relationship between
        the source account and sponsoredID
        Threshold: med
        Result: BeginSponsoringFutureReservesResult
    */
    struct BeginSponsoringFutureReservesOp
    {
        AccountID sponsoredID;
    };

    inline QDataStream &operator<<(QDataStream &out, const  BeginSponsoringFutureReservesOp &obj) {
        out << obj.sponsoredID;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  BeginSponsoringFutureReservesOp &obj) {
       in >> obj.sponsoredID;
       return in;
    }

    /* EndSponsoringFutureReserves
        Terminates the current is-sponsoring-future-reserves-for relationship in
        which source account is sponsored
        Threshold: med
        Result: EndSponsoringFutureReservesResult
    */
    // EndSponsoringFutureReserves is empty

    /* RevokeSponsorship
        If source account is not sponsored or is sponsored by the owner of the
        specified entry or sub-entry, then attempt to revoke the sponsorship.
        If source account is sponsored, then attempt to transfer the sponsorship
        to the sponsor of source account.
        Threshold: med
        Result: RevokeSponsorshipResult
    */
    enum class RevokeSponsorshipType: qint32
    {
        REVOKE_SPONSORSHIP_LEDGER_ENTRY = 0,
        REVOKE_SPONSORSHIP_SIGNER = 1
    };

    struct RevokeSponsorshipOp
    {
        RevokeSponsorshipType type;
        struct RevokeSponsorshipSigner
        {
            AccountID accountID;
            SignerKey signerKey;
        };
        union
        {
            LedgerKey ledgerKey;//case REVOKE_SPONSORSHIP_LEDGER_ENTRY
            RevokeSponsorshipSigner signer;//case REVOKE_SPONSORSHIP_SIGNER

        };
        RevokeSponsorshipOp():type(RevokeSponsorshipType::REVOKE_SPONSORSHIP_LEDGER_ENTRY)
        {
            new (&ledgerKey) LedgerKey();
        }
        RevokeSponsorshipOp(const RevokeSponsorshipOp &obj)
        {
            type= obj.type;
            switch(obj.type)
            {
            case RevokeSponsorshipType::REVOKE_SPONSORSHIP_LEDGER_ENTRY:
                new (&ledgerKey) LedgerKey();
                ledgerKey = obj.ledgerKey;
                break;
            case RevokeSponsorshipType::REVOKE_SPONSORSHIP_SIGNER:
                new (&signer) RevokeSponsorshipSigner();
                signer.accountID = obj.signer.accountID;
                signer.signerKey = obj.signer.signerKey;
                break;
            }
        }

        ~RevokeSponsorshipOp()
        {
            switch(type)
            {
            case RevokeSponsorshipType::REVOKE_SPONSORSHIP_LEDGER_ENTRY:
                (ledgerKey).~LedgerKey();
                break;
            case RevokeSponsorshipType::REVOKE_SPONSORSHIP_SIGNER:
                (signer).~RevokeSponsorshipSigner();
                break;
            default: break;
            }
        }
    private:
        friend inline QDataStream &operator>>(QDataStream &in,  RevokeSponsorshipOp &obj);
        void clear()
        {
            switch(type)
            {
            case RevokeSponsorshipType::REVOKE_SPONSORSHIP_LEDGER_ENTRY:
                (ledgerKey).~LedgerKey();
                break;
            case RevokeSponsorshipType::REVOKE_SPONSORSHIP_SIGNER:
                (signer).~RevokeSponsorshipSigner();
                break;
            default: break;
            }
        }
    public:
        LedgerKey& fillRevokeSponsorshipLedgerEntry()
        {
            if(type!=RevokeSponsorshipType::REVOKE_SPONSORSHIP_LEDGER_ENTRY)
            {
                clear();
                type=RevokeSponsorshipType::REVOKE_SPONSORSHIP_LEDGER_ENTRY;
                new (&ledgerKey) LedgerKey();
            }
            return ledgerKey;
        }
        RevokeSponsorshipSigner& fillRevokeSponsorshipSigner()
        {
            if(type!=RevokeSponsorshipType::REVOKE_SPONSORSHIP_SIGNER)
            {
                clear();
                type=RevokeSponsorshipType::REVOKE_SPONSORSHIP_SIGNER;
                new (&signer) RevokeSponsorshipSigner();
            }
            return signer;
        }
        const RevokeSponsorshipOp& operator = (const RevokeSponsorshipOp& obj)
        {
            if (this == &obj)
                return *this;   // clear() would destroy what it copies from
            clear();
            type= obj.type;
            switch(obj.type)
            {
            case RevokeSponsorshipType::REVOKE_SPONSORSHIP_LEDGER_ENTRY:
                new (&ledgerKey) LedgerKey();
                ledgerKey = obj.ledgerKey;
                break;
            case RevokeSponsorshipType::REVOKE_SPONSORSHIP_SIGNER:
                new (&signer) RevokeSponsorshipSigner();
                signer.accountID = obj.signer.accountID;
                signer.signerKey = obj.signer.signerKey;
                break;
            }
            return *this;
        }
    };

    inline QDataStream &operator<<(QDataStream &out, const  RevokeSponsorshipOp &obj) {
        switch(obj.type)
        {
        case RevokeSponsorshipType::REVOKE_SPONSORSHIP_LEDGER_ENTRY:
            out << obj.type << obj.ledgerKey;
            break;
        case RevokeSponsorshipType::REVOKE_SPONSORSHIP_SIGNER:
            out << obj.type << obj.signer.accountID << obj.signer.signerKey;
            break;
        default: out.setStatus(QDataStream::WriteFailed); break;   // no arm: nothing to write
        }

       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  RevokeSponsorshipOp &obj) {
        obj.clear();
       in >> obj.type;
       switch(obj.type)
       {
       case RevokeSponsorshipType::REVOKE_SPONSORSHIP_LEDGER_ENTRY:
           new (&obj.ledgerKey) LedgerKey();
           in >> obj.ledgerKey;
           break;
       case RevokeSponsorshipType::REVOKE_SPONSORSHIP_SIGNER:
           new (&obj.signer) RevokeSponsorshipOp::RevokeSponsorshipSigner();
           in >> obj.signer.accountID >> obj.signer.signerKey;;
           break;
       default: in.setStatus(QDataStream::ReadCorruptData); break;
       }
       return in;
    }

    /* Claws back an amount of an asset from an account
        Threshold: med
        Result: ClawbackResult
    */
    struct ClawbackOp
    {
        Asset asset;
        MuxedAccount from;
        qint64 amount;
    };

    inline QDataStream &operator<<(QDataStream &out, const  ClawbackOp &obj) {
        out << obj.asset << obj.from << obj.amount;
        return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  ClawbackOp &obj) {

        in >> obj.asset >> obj.from >> obj.amount;
        return in;
    }

    /* Claws back a claimable balance
        Threshold: med
        Result: ClawbackClaimableBalanceResult
    */
    struct ClawbackClaimableBalanceOp
    {
        ClaimableBalanceID balanceID;
    };


    inline QDataStream &operator<<(QDataStream &out, const  ClawbackClaimableBalanceOp &obj) {
        out << obj.balanceID;
        return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  ClawbackClaimableBalanceOp &obj) {

        in >> obj.balanceID;
        return in;
    }

    /* SetTrustLineFlagsOp
       Updates the flags of an existing trust line.
       This is called by the issuer of the related asset.
       Threshold: low
       Result: SetTrustLineFlagsResult
    */
    struct SetTrustLineFlagsOp
    {
        AccountID trustor;
        Asset asset;

        quint32 clearFlags; // which flags to clear
        quint32 setFlags;   // which flags to set
    };

    inline QDataStream &operator<<(QDataStream &out, const  SetTrustLineFlagsOp &obj) {
        out << obj.trustor << obj.asset << obj.clearFlags << obj.setFlags;
        return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  SetTrustLineFlagsOp &obj) {

        in >> obj.trustor >> obj.asset >> obj.clearFlags >> obj.setFlags;
        return in;
    }

    /* CAP-38 Protocol 18 — deposit into a liquidity pool.
       Threshold: med
       Result: LiquidityPoolDepositResult
    */
    struct LiquidityPoolDepositOp
    {
        PoolID liquidityPoolID;
        qint64 maxAmountA;  // maximum amount of first asset to deposit
        qint64 maxAmountB;  // maximum amount of second asset to deposit
        Price minPrice;     // minimum depositA/depositB
        Price maxPrice;     // maximum depositA/depositB
    };
    inline QDataStream &operator<<(QDataStream &out, const LiquidityPoolDepositOp &obj) {
        out << obj.liquidityPoolID << obj.maxAmountA << obj.maxAmountB
            << obj.minPrice << obj.maxPrice;
        return out;
    }
    inline QDataStream &operator>>(QDataStream &in, LiquidityPoolDepositOp &obj) {
        in >> obj.liquidityPoolID >> obj.maxAmountA >> obj.maxAmountB
           >> obj.minPrice >> obj.maxPrice;
        return in;
    }

    /* CAP-38 Protocol 18 — withdraw from a liquidity pool.
       Threshold: med
       Result: LiquidityPoolWithdrawResult
    */
    struct LiquidityPoolWithdrawOp
    {
        PoolID liquidityPoolID;
        qint64 amount;      // amount of pool shares to withdraw
        qint64 minAmountA;  // minimum amount of first asset to withdraw
        qint64 minAmountB;  // minimum amount of second asset to withdraw
    };
    inline QDataStream &operator<<(QDataStream &out, const LiquidityPoolWithdrawOp &obj) {
        out << obj.liquidityPoolID << obj.amount << obj.minAmountA << obj.minAmountB;
        return out;
    }
    inline QDataStream &operator>>(QDataStream &in, LiquidityPoolWithdrawOp &obj) {
        in >> obj.liquidityPoolID >> obj.amount >> obj.minAmountA >> obj.minAmountB;
        return in;
    }

    /* An operation is the lowest unit of work that a transaction does */
    struct Operation
    {
        // sourceAccount is the account used to run the operation
        // if not set, the runtime defaults to "sourceAccount" specified at
        // the transaction level
        Optional<MuxedAccount> sourceAccount;
        OperationType type;
        union{
        CreateAccountOp operationCreateAccount;
        PaymentOp operationPayment;
        ManageSellOfferOp operationManageSellOffer;
        CreatePassiveSellOfferOp operationCreatePassiveSellOffer;
        ChangeTrustOp operationChangeTrust;
        AllowTrustOp operationAllowTrust;
        MuxedAccount operationAccountMerge;
        // No member for INFLATION: the canonical union writes `case INFLATION:
        // void;` and carries nothing at all. There used to be a MuxedAccount
        // here and the writer wrote it, so every inflation operation put an
        // extra account on the wire where the format expects nothing — and
        // read it out of a union member that fillOperationBody never
        // constructs. Horizon answered "Transaction Malformed"; a symmetric
        // round trip through this file never noticed, because the reader
        // consumed exactly the same bytes the writer invented.
        BumpSequenceOp operationBumpSequence;
        ManageBuyOfferOp operationManageBuyOffer;        
        ClawbackOp operationClawback;
        ClawbackClaimableBalanceOp operationClawbackClaimableBalance;
        SetTrustLineFlagsOp operationSetTrustLineFlags;
        LiquidityPoolDepositOp operationLiquidityPoolDeposit;
        LiquidityPoolWithdrawOp operationLiquidityPoolWithdraw;
        // Soroban — CAP-46-8 (0.10.0).
        ExtendFootprintTTLOp operationExtendFootprintTtl;
        RestoreFootprintOp operationRestoreFootprint;

        //non trivials, you MUST call contructor explicity to use them
        PathPaymentStrictReceiveOp operationPathPaymentStrictReceive;
        SetOptionsOp operationSetOptions;
        ManageDataOp operationManageData;
        PathPaymentStrictSendOp  operationPathPaymentStrictSend;

        CreateClaimableBalanceOp operationCreateClaimableBalance;//case CREATE_CLAIMABLE_BALANCE:
        ClaimClaimableBalanceOp operationClaimClaimableBalance;//case CLAIM_CLAIMABLE_BALANCE:
        BeginSponsoringFutureReservesOp operationBeginSponsoringFutureReserves;//case BEGIN_SPONSORING_FUTURE_RESERVES:
        //void;//case END_SPONSORING_FUTURE_RESERVES:
        RevokeSponsorshipOp operationRevokeSponsorship;//case REVOKE_SPONSORSHIP:
        InvokeHostFunctionOp operationInvokeHostFunction;//case INVOKE_HOST_FUNCTION:
        };

        /**
         * @brief Operation
         * builds an Operation, it will not construct any of the non trivials members of the union
         * as it is used internally, to fill any of the non trivials you have to call the according placement constructor
         */
        Operation();

        Operation(const stellar::Operation& op);
    private:
        friend inline QDataStream &operator>>(QDataStream &in,  Operation &obj);
        void clear();
    public:
        ~Operation();
        const Operation& operator = (const Operation& op);
        PathPaymentStrictReceiveOp& fillPathPaymentStrictReceiveOp();
        SetOptionsOp& fillSetOptionsOp();
        ManageDataOp& fillManageDataOp();
        PathPaymentStrictSendOp& fillPathPaymentStrictSendOp();
        RevokeSponsorshipOp& fillRevokeSponsorshipOp();
        ChangeTrustOp& fillChangeTrustOp();
        InvokeHostFunctionOp& fillInvokeHostFunctionOp();
        CreateClaimableBalanceOp& fillCreateClaimableBalanceOp();

    };
    inline QDataStream &operator<<(QDataStream &out, const  Operation &obj) {
        // A type with no arm is refused before anything is written; it used
        // to go out as a source account and a bare discriminant.
        if (static_cast<qint32>(obj.type) < static_cast<qint32>(OperationType::CREATE_ACCOUNT)
            || static_cast<qint32>(obj.type) > static_cast<qint32>(OperationType::RESTORE_FOOTPRINT)) {
            out.setStatus(QDataStream::WriteFailed);
            return out;
        }
        out << obj.sourceAccount << obj.type;
        switch(obj.type){
        case OperationType::CREATE_ACCOUNT:
            out << obj.operationCreateAccount; break;
        case OperationType::PAYMENT:
            out << obj.operationPayment; break;
        case OperationType::PATH_PAYMENT_STRICT_RECEIVE:
            out << obj.operationPathPaymentStrictReceive; break;
        case OperationType::MANAGE_SELL_OFFER:
            out << obj.operationManageSellOffer; break;
        case OperationType::CREATE_PASSIVE_SELL_OFFER:
            out << obj.operationCreatePassiveSellOffer; break;
        case OperationType::SET_OPTIONS:
            out << obj.operationSetOptions; break;
        case OperationType::CHANGE_TRUST:
            out << obj.operationChangeTrust; break;
        case OperationType::ALLOW_TRUST:
            out << obj.operationAllowTrust; break;
        case OperationType::ACCOUNT_MERGE:
            out << obj.operationAccountMerge; break;
        case OperationType::INFLATION:
            break;                       // void: the arm carries nothing
        case OperationType::MANAGE_DATA:
            out << obj.operationManageData; break;
        case OperationType::BUMP_SEQUENCE:
            out << obj.operationBumpSequence; break;
        case OperationType::MANAGE_BUY_OFFER:
            out << obj.operationManageBuyOffer; break;
        case OperationType::PATH_PAYMENT_STRICT_SEND:
            out << obj.operationPathPaymentStrictSend; break;
        case OperationType::CREATE_CLAIMABLE_BALANCE:
            out <<obj.operationCreateClaimableBalance; break;
        case OperationType::CLAIM_CLAIMABLE_BALANCE:
            out <<obj.operationClaimClaimableBalance; break;
        case OperationType::BEGIN_SPONSORING_FUTURE_RESERVES:
            out <<obj.operationBeginSponsoringFutureReserves; break;
        case OperationType::END_SPONSORING_FUTURE_RESERVES:
            break;
        case OperationType::REVOKE_SPONSORSHIP:
            out <<obj.operationRevokeSponsorship; break;
        case OperationType::CLAWBACK:
            out << obj.operationClawback; break;
        case OperationType::CLAWBACK_CLAIMABLE_BALANCE:
            out << obj.operationClawbackClaimableBalance; break;
        case OperationType::SET_TRUST_LINE_FLAGS:
            out << obj.operationSetTrustLineFlags; break;
        case OperationType::LIQUIDITY_POOL_DEPOSIT:
            out << obj.operationLiquidityPoolDeposit; break;
        case OperationType::LIQUIDITY_POOL_WITHDRAW:
            out << obj.operationLiquidityPoolWithdraw; break;
        case OperationType::INVOKE_HOST_FUNCTION:
            out << obj.operationInvokeHostFunction; break;
        case OperationType::EXTEND_FOOTPRINT_TTL:
            out << obj.operationExtendFootprintTtl; break;
        case OperationType::RESTORE_FOOTPRINT:
            out << obj.operationRestoreFootprint; break;

        //default: break;
        }

       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  Operation &obj) {
        obj.clear();
        in >> obj.sourceAccount >> obj.type;
        switch(obj.type){
        case OperationType::CREATE_ACCOUNT:
            in >> obj.operationCreateAccount; break;
        case OperationType::PAYMENT:
            in >> obj.operationPayment; break;
        case OperationType::PATH_PAYMENT_STRICT_RECEIVE:
            new (&obj.operationPathPaymentStrictReceive) PathPaymentStrictReceiveOp();
            in >> obj.operationPathPaymentStrictReceive; break;
        case OperationType::MANAGE_SELL_OFFER:
            in >> obj.operationManageSellOffer; break;
        case OperationType::CREATE_PASSIVE_SELL_OFFER:
            in >> obj.operationCreatePassiveSellOffer; break;
        case OperationType::SET_OPTIONS:
            new (&obj.operationSetOptions) SetOptionsOp();
            in >> obj.operationSetOptions; break;
        case OperationType::CHANGE_TRUST:
            new (&obj.operationChangeTrust) ChangeTrustOp();
            in >> obj.operationChangeTrust; break;
        case OperationType::ALLOW_TRUST:
            in >> obj.operationAllowTrust; break;
        case OperationType::ACCOUNT_MERGE:
            in >> obj.operationAccountMerge; break;
        case OperationType::INFLATION:
            break;                       // void: there is nothing to read
        case OperationType::MANAGE_DATA:
            new (&obj.operationManageData) ManageDataOp();
            in >> obj.operationManageData; break;
        case OperationType::BUMP_SEQUENCE:
            in >> obj.operationBumpSequence; break;
        case OperationType::MANAGE_BUY_OFFER:
            in >> obj.operationManageBuyOffer; break;
        case OperationType::PATH_PAYMENT_STRICT_SEND:
            new (&obj.operationPathPaymentStrictSend) PathPaymentStrictSendOp();
            in >> obj.operationPathPaymentStrictSend; break;
        case OperationType::CREATE_CLAIMABLE_BALANCE:
            new (&obj.operationCreateClaimableBalance) CreateClaimableBalanceOp();
            in >>obj.operationCreateClaimableBalance; break;
        case OperationType::CLAIM_CLAIMABLE_BALANCE:
            in >>obj.operationClaimClaimableBalance; break;
        case OperationType::BEGIN_SPONSORING_FUTURE_RESERVES:
            in >>obj.operationBeginSponsoringFutureReserves; break;
        case OperationType::END_SPONSORING_FUTURE_RESERVES:
            break;
        case OperationType::REVOKE_SPONSORSHIP:
            new (&obj.operationRevokeSponsorship) RevokeSponsorshipOp();
            in >>obj.operationRevokeSponsorship; break;
        case OperationType::CLAWBACK:
            in >> obj.operationClawback; break;
        case OperationType::CLAWBACK_CLAIMABLE_BALANCE:
            new (&obj.operationClawbackClaimableBalance) ClawbackClaimableBalanceOp();
            in >> obj.operationClawbackClaimableBalance; break;
        case OperationType::SET_TRUST_LINE_FLAGS:
            in >> obj.operationSetTrustLineFlags; break;
        case OperationType::LIQUIDITY_POOL_DEPOSIT:
            in >> obj.operationLiquidityPoolDeposit; break;
        case OperationType::LIQUIDITY_POOL_WITHDRAW:
            in >> obj.operationLiquidityPoolWithdraw; break;
        case OperationType::INVOKE_HOST_FUNCTION:
            new (&obj.operationInvokeHostFunction) InvokeHostFunctionOp();
            in >> obj.operationInvokeHostFunction; break;
        case OperationType::EXTEND_FOOTPRINT_TTL:
            in >> obj.operationExtendFootprintTtl; break;
        case OperationType::RESTORE_FOOTPRINT:
            in >> obj.operationRestoreFootprint; break;
        default: in.setStatus(QDataStream::ReadCorruptData); break;
        }
       return in;
    }

    struct OperationID
    {
        EnvelopeType type;

        struct
        {
            MuxedAccount sourceAccount;
            SequenceNumber seqNum;
            quint32 opNum;
        } id;//case ENVELOPE_TYPE_OP_ID:
    };

    inline QDataStream &operator<<(QDataStream &out, const  OperationID &obj) {
        switch(obj.type){
        case EnvelopeType::ENVELOPE_TYPE_OP_ID:
            out << obj.type << obj.id.sourceAccount << obj.id.seqNum<< obj.id.opNum; break;
        // A claimable balance id is the hash of this: a bare discriminant
        // was hashed into a balance id that names nothing.
        default: out.setStatus(QDataStream::WriteFailed); break;
        }
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  OperationID &obj) {
        in >> obj.type;
        switch(obj.type){
        case EnvelopeType::ENVELOPE_TYPE_OP_ID:
            in >> obj.id.sourceAccount >> obj.id.seqNum>> obj.id.opNum; break;
        default: in.setStatus(QDataStream::ReadCorruptData); break;
        }

       return in;
    }

    enum class MemoType : qint32
    {
        MEMO_NONE = 0,
        MEMO_TEXT = 1,
        MEMO_ID = 2,
        MEMO_HASH = 3,
        MEMO_RETURN = 4
    };

    struct Memo{
        MemoType type;
        Array<char,28> text; //max <28>
        union{
            quint64 id;
            Hash hash;
            Hash retHash;
        };
    };
    inline QDataStream &operator<<(QDataStream &out, const  Memo &obj) {
        switch(obj.type){
        case MemoType::MEMO_NONE:
            out << obj.type; break;
        case MemoType::MEMO_TEXT:
            out << obj.type << obj.text; break;
        case MemoType::MEMO_ID:
            out << obj.type << obj.id; break;
        case MemoType::MEMO_HASH:
            out << obj.type << obj.hash; break;
        case MemoType::MEMO_RETURN:
            out << obj.type << obj.retHash; break;
        default: out.setStatus(QDataStream::WriteFailed); break;   // no arm: nothing to write
        }
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  Memo &obj) {
        in >> obj.type;;
        switch(obj.type){
        case MemoType::MEMO_NONE:
            break;
        case MemoType::MEMO_TEXT:
            in >> obj.text; break;
        case MemoType::MEMO_ID:
            in >> obj.id; break;
        case MemoType::MEMO_HASH:
            in >> obj.hash; break;
        case MemoType::MEMO_RETURN:
            in >> obj.retHash; break;
        default: in.setStatus(QDataStream::ReadCorruptData); break;
        }
       return in;
    }


    struct TimeBounds
    {
        TimePoint minTime;
        TimePoint maxTime; // 0 here means no maxTime
    };
    inline QDataStream &operator<<(QDataStream &out, const  TimeBounds &obj) {
        out << obj.minTime << obj.maxTime;;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  TimeBounds &obj) {
        in >> obj.minTime >> obj.maxTime;;
       return in;
    }

    // CAP-21 — generalized transaction preconditions.
    typedef quint64 Duration;

    struct LedgerBounds
    {
        quint32 minLedger;
        quint32 maxLedger; // 0 here means no maxLedger
    };
    inline QDataStream &operator<<(QDataStream &out, const LedgerBounds &obj) {
        out << obj.minLedger << obj.maxLedger;
        return out;
    }
    inline QDataStream &operator>>(QDataStream &in, LedgerBounds &obj) {
        in >> obj.minLedger >> obj.maxLedger;
        return in;
    }

    enum class PreconditionType : qint32
    {
        PRECOND_NONE = 0,
        PRECOND_TIME = 1,
        PRECOND_V2 = 2
    };

    // CAP-21 PreconditionsV2. Extra signers are stored as a length-prefixed
    // bounded array (max 2 entries per the spec).
    struct PreconditionsV2
    {
        Optional<TimeBounds> timeBounds;
        Optional<LedgerBounds> ledgerBounds;
        Optional<SequenceNumber> minSeqNum;
        Duration minSeqAge = 0;       // 0 means no constraint
        quint32 minSeqLedgerGap = 0;  // 0 means no constraint
        Array<SignerKey, 2> extraSigners;
    };
    inline QDataStream &operator<<(QDataStream &out, const PreconditionsV2 &obj) {
        out << obj.timeBounds << obj.ledgerBounds << obj.minSeqNum
            << obj.minSeqAge << obj.minSeqLedgerGap << obj.extraSigners;
        return out;
    }
    inline QDataStream &operator>>(QDataStream &in, PreconditionsV2 &obj) {
        in >> obj.timeBounds >> obj.ledgerBounds >> obj.minSeqNum
           >> obj.minSeqAge >> obj.minSeqLedgerGap >> obj.extraSigners;
        return in;
    }

    /**
     * Preconditions union — wraps TimeBounds (legacy) or PreconditionsV2.
     * Wire-compatible with the legacy Optional<TimeBounds> field: PRECOND_NONE
     * has the same on-wire encoding as Optional{filled=0}, and PRECOND_TIME has
     * the same encoding as Optional{filled=1, TimeBounds}.
     */
    /**
     * Invariant: storage always holds a live variant matching `type`. For
     * PRECOND_NONE, the `timeBounds` slot is live (trivial, ignored).
     * After `clear()`, no variant is live until the caller placement-news a
     * new one (always done as part of a transition).
     */
    struct Preconditions
    {
        PreconditionType type;
        union {
            TimeBounds timeBounds;
            PreconditionsV2 v2;
        };

        Preconditions() : type(PreconditionType::PRECOND_NONE)
        {
            new (&timeBounds) TimeBounds();
        }
        Preconditions(const Preconditions &other) : type(other.type)
        {
            switch (type) {
            case PreconditionType::PRECOND_NONE:
                new (&timeBounds) TimeBounds(); break;
            case PreconditionType::PRECOND_TIME:
                new (&timeBounds) TimeBounds(other.timeBounds); break;
            case PreconditionType::PRECOND_V2:
                new (&v2) PreconditionsV2(other.v2); break;
            }
        }
        ~Preconditions() { clear(); }
        Preconditions& operator=(const Preconditions &other)
        {
            if (this == &other) return *this;
            clear();
            type = other.type;
            switch (type) {
            case PreconditionType::PRECOND_NONE:
                new (&timeBounds) TimeBounds(); break;
            case PreconditionType::PRECOND_TIME:
                new (&timeBounds) TimeBounds(other.timeBounds); break;
            case PreconditionType::PRECOND_V2:
                new (&v2) PreconditionsV2(other.v2); break;
            }
            return *this;
        }
        TimeBounds& fillTimeBounds()
        {
            clear();
            type = PreconditionType::PRECOND_TIME;
            new (&timeBounds) TimeBounds();
            return timeBounds;
        }
        PreconditionsV2& fillV2()
        {
            clear();
            type = PreconditionType::PRECOND_V2;
            new (&v2) PreconditionsV2();
            return v2;
        }
    private:
        // Destroys the currently-live variant. Caller must placement-new a new
        // one before this object is read or destroyed again.
        void clear()
        {
            switch (type) {
            case PreconditionType::PRECOND_NONE:
            case PreconditionType::PRECOND_TIME:
                timeBounds.~TimeBounds(); break;
            case PreconditionType::PRECOND_V2:
                v2.~PreconditionsV2(); break;
            }
        }
        friend inline QDataStream &operator>>(QDataStream &in, Preconditions &obj);
    };
    inline QDataStream &operator<<(QDataStream &out, const Preconditions &obj) {
        switch (obj.type) {
        case PreconditionType::PRECOND_NONE: out << obj.type; break;
        case PreconditionType::PRECOND_TIME: out << obj.type << obj.timeBounds; break;
        case PreconditionType::PRECOND_V2:   out << obj.type << obj.v2; break;
        default: out.setStatus(QDataStream::WriteFailed); break;   // no arm: nothing to write
        }
        return out;
    }
    inline QDataStream &operator>>(QDataStream &in, Preconditions &obj) {
        obj.clear();
        in >> obj.type;
        switch (obj.type) {
        case PreconditionType::PRECOND_NONE:
            new (&obj.timeBounds) TimeBounds(); break;
        case PreconditionType::PRECOND_TIME:
            new (&obj.timeBounds) TimeBounds();
            in >> obj.timeBounds; break;
        case PreconditionType::PRECOND_V2:
            new (&obj.v2) PreconditionsV2();
            in >> obj.v2; break;
        default: in.setStatus(QDataStream::ReadCorruptData); break;
        }
        return in;
    }
    // maximum number of operations per transaction
    const int MAX_OPS_PER_TX = 100;


    const int MAX_SIGNATURES_PER_TX = 20;

    /* a transaction is a container for a set of operations
        - is executed by an account
        - fees are collected from the account
        - operations are executed in order as one ACID transaction
              either all operations are applied or none are
              if any returns a failing code
    */

    // TransactionV0 is a transaction with the AccountID discriminant stripped off,
    // leaving a raw ed25519 public key to identify the source account. This is used
    // for backwards compatibility starting from the protocol 12/13 boundary. If an
    // "old-style" TransactionEnvelope containing a Transaction is parsed with this
    // XDR definition, it will be parsed as a "new-style" TransactionEnvelope
    // containing a TransactionV0.
    struct TransactionV0
    {
        // account used to run the transaction
        uint256 sourceAccountEd25519;

        // the fee the sourceAccount will pay
        quint32 fee;

        // sequence number to consume in the account
        SequenceNumber seqNum;

        // validity range (inclusive) for the last ledger close time
        Optional<TimeBounds> timeBounds;

        Memo memo;

        Array<Operation,MAX_OPS_PER_TX> operations; //max <100>;

        // reserved for future use
        Reserved ext;
    };
    inline QDataStream &operator<<(QDataStream &out, const  TransactionV0 &obj) {
        out << obj.sourceAccountEd25519 << obj.fee << obj.seqNum << obj.timeBounds << obj.memo<< obj.operations<< obj.ext;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  TransactionV0 &obj) {
       in >> obj.sourceAccountEd25519 >> obj.fee >> obj.seqNum >> obj.timeBounds >> obj.memo>> obj.operations >> obj.ext;
       if (obj.ext.reserved != 0) in.setStatus(QDataStream::ReadCorruptData);   // ext is void: 0 only
       return in;
    }

    struct TransactionV0Envelope
    {
        TransactionV0 tx;
        /* Each decorated signature is a signature over the SHA256 hash of
         * a TransactionSignaturePayload */
        Array<DecoratedSignature,MAX_SIGNATURES_PER_TX> signatures; //max <20>;
    };
    inline QDataStream &operator<<(QDataStream &out, const  TransactionV0Envelope &obj) {
        out << obj.tx << obj.signatures;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  TransactionV0Envelope &obj) {
       in >> obj.tx >> obj.signatures;
       return in;
    }

    /**
     * Transaction::ext — discriminated union by `v`. v=0 is wire-identical
     * to the legacy `Reserved ext`. v=1 carries CAP-46 SorobanTransactionData.
     */
    struct TransactionExt
    {
        qint32 v = 0;
        SorobanTransactionData sorobanData;
    };
    inline QDataStream &operator<<(QDataStream &out, const TransactionExt &obj) {
        if (obj.v != 0 && obj.v != 1) {             // no arm: nothing to write
            out.setStatus(QDataStream::WriteFailed);
            return out;
        }
        out << obj.v;
        if (obj.v == 1) out << obj.sorobanData;
        return out;
    }
    inline QDataStream &operator>>(QDataStream &in, TransactionExt &obj) {
        in >> obj.v;
        if (obj.v == 1) in >> obj.sorobanData;
        else if (obj.v != 0) in.setStatus(QDataStream::ReadCorruptData);   // an arm it does not know
        return in;
    }

    struct Transaction
    {
        // account used to run the transaction
        MuxedAccount sourceAccount;

        // the fee the sourceAccount will pay
        quint32 fee;

        // sequence number to consume in the account
        SequenceNumber seqNum;

        // CAP-21 — was Optional<TimeBounds>. Wire-compat for the
        // PRECOND_NONE and PRECOND_TIME variants (same byte sequence).
        Preconditions cond;

        Memo memo;

        Array<Operation,MAX_OPS_PER_TX> operations; //max <100>;

        // CAP-46 — discriminated union. v=0 is wire-identical to the legacy
        // Reserved ext (4 zero bytes); v=1 carries SorobanTransactionData.
        TransactionExt ext;
    };
    inline QDataStream &operator<<(QDataStream &out, const  Transaction &obj) {
        out << obj.sourceAccount << obj.fee << obj.seqNum << obj.cond << obj.memo<< obj.operations<< obj.ext;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  Transaction &obj) {
       in >> obj.sourceAccount >> obj.fee >> obj.seqNum >> obj.cond >> obj.memo>> obj.operations >> obj.ext;
       return in;
    }

    /* A TransactionEnvelope wraps a transaction with signatures. */
    struct TransactionV1Envelope
    {
        Transaction tx;
        /* Each decorated signature is a signature over the SHA256 hash of
         * a TransactionSignaturePayload */
        Array<DecoratedSignature,MAX_SIGNATURES_PER_TX> signatures; //max <20>;
    };
    inline QDataStream &operator<<(QDataStream &out, const  TransactionV1Envelope &obj) {
        out << obj.tx << obj.signatures;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  TransactionV1Envelope &obj) {
       obj.tx = Transaction();      // tx is always live: reset it, never build a second one over it
       in >> obj.tx >> obj.signatures;
       return in;
    }


    struct FeeBumpTransaction
    {
        MuxedAccount feeSource;
        qint64 fee;
        EnvelopeType type;
        union{//innerTx
            TransactionV1Envelope v1;   //ENVELOPE_TYPE_TX
        };

        Reserved ext;
        /**
         * @brief FeeBumpTransaction
         * builds an FeeBumpTransaction, it will not construct any of the non trivials members of the union
         * as it is used internally, to fill any of the non trivials you have to call the according placement constructor
         */
        FeeBumpTransaction();

        FeeBumpTransaction(const stellar::FeeBumpTransaction& fbt);
        ~FeeBumpTransaction();
        const FeeBumpTransaction& operator = (const FeeBumpTransaction& fbt);
    };

    inline QDataStream &operator<<(QDataStream &out, const  FeeBumpTransaction &obj) {
        // A fee bump wraps a v1 transaction only: any other inner type is
        // refused before the fee source and fee are written.
        if (obj.type != EnvelopeType::ENVELOPE_TYPE_TX) {
            out.setStatus(QDataStream::WriteFailed);
            return out;
        }
        out << obj.feeSource << obj.fee << obj.type << obj.v1 << obj.ext;
       return out;
    }

   inline  QDataStream &operator>>(QDataStream &in,  FeeBumpTransaction &obj) {
       in >> obj.feeSource >> obj.fee >> obj.type;
       switch(obj.type)
       {
        case EnvelopeType::ENVELOPE_TYPE_TX:
       {
           in >> obj.v1;        // v1 is always live (see the constructor)
           break;
       }
       default: in.setStatus(QDataStream::ReadCorruptData); break;     // a fee bump wraps a v1 transaction only
       }
       in >> obj.ext;
       if (obj.ext.reserved != 0) in.setStatus(QDataStream::ReadCorruptData);   // ext is void: 0 only

       return in;
    }

    struct FeeBumpTransactionEnvelope
    {
        FeeBumpTransaction tx;
        /* Each decorated signature is a signature over the SHA256 hash of
         * a TransactionSignaturePayload */
        Array<DecoratedSignature,MAX_SIGNATURES_PER_TX> signatures;

        FeeBumpTransactionEnvelope(const FeeBumpTransactionEnvelope& te)
            :tx(te.tx),signatures(te.signatures)
        {

        }
        FeeBumpTransactionEnvelope()
        {
        }
        const FeeBumpTransactionEnvelope& operator = (const FeeBumpTransactionEnvelope& te);
    };

    inline QDataStream &operator<<(QDataStream &out, const  FeeBumpTransactionEnvelope &obj) {
        out << obj.tx << obj.signatures;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  FeeBumpTransactionEnvelope &obj) {
       in >> obj.tx >> obj.signatures;
       return in;
    }

    /* A TransactionEnvelope wraps a transaction with signatures. */
    struct TransactionEnvelope
    {
        EnvelopeType type;
        union{
            //non trivial
            TransactionV0Envelope v0;           //ENVELOPE_TYPE_TX_V0
            TransactionV1Envelope v1;           //ENVELOPE_TYPE_TX
            FeeBumpTransactionEnvelope feeBump; //ENVELOPE_TYPE_TX_FEE_BUMP
        };

        /**
         * @brief TransactionEnvelope
         * builds an TransactionEnvelope, it will not construct any of the non trivials members of the union
         * as it is used internally, to fill any of the non trivials you have to call the according placement constructor
         */
        TransactionEnvelope();        
        TransactionEnvelope(const TransactionV0Envelope& t);
        TransactionEnvelope(const TransactionV1Envelope& t);
        TransactionEnvelope(const FeeBumpTransactionEnvelope &t);

        TransactionEnvelope(const stellar::TransactionEnvelope& te);
        ~TransactionEnvelope();
        const TransactionEnvelope& operator = (const TransactionEnvelope& te);
    private:
        friend inline QDataStream &operator>>(QDataStream &in,  TransactionEnvelope &obj);
        // Destroys the live arm, whichever of the three it is. The caller
        // constructs the next one.
        void clear();
    };

    inline QDataStream &operator<<(QDataStream &out, const  TransactionEnvelope &obj) {
        switch(obj.type)
        {
        case EnvelopeType::ENVELOPE_TYPE_TX_V0:
            out << obj.type << obj.v0; break;
        case EnvelopeType::ENVELOPE_TYPE_TX:
            out << obj.type << obj.v1; break;
        case EnvelopeType::ENVELOPE_TYPE_TX_FEE_BUMP:
            out << obj.type << obj.feeBump; break;
        default: out.setStatus(QDataStream::WriteFailed); break;   // no arm: nothing to write
        }
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  TransactionEnvelope &obj) {
        obj.clear();            // an envelope read into twice must not keep (or leak) the first arm
        in >> obj.type;
        switch(obj.type)
        {
        case EnvelopeType::ENVELOPE_TYPE_TX_V0:
        {
            new (&obj.v0) TransactionV0Envelope();
            in >> obj.v0; break;
        }
        case EnvelopeType::ENVELOPE_TYPE_TX:
        {
            new (&obj.v1) TransactionV1Envelope();
            in >> obj.v1; break;
        }
        case EnvelopeType::ENVELOPE_TYPE_TX_FEE_BUMP:
        {
            new (&obj.feeBump) FeeBumpTransactionEnvelope();
            in >> obj.feeBump; break;
        }
        default: in.setStatus(QDataStream::ReadCorruptData); break;
        }
       return in;
    }

    struct TransactionSignaturePayload
    {
        Hash networkId;
        EnvelopeType type;
        // Backwards Compatibility: Use ENVELOPE_TYPE_TX to sign ENVELOPE_TYPE_TX_V0
        union{//taggedTransaction
            Transaction tx;             //ENVELOPE_TYPE_TX
            FeeBumpTransaction feeBump; //ENVELOPE_TYPE_TX_FEE_BUMP
        };
        TransactionSignaturePayload(EnvelopeType t):type(t)
        {
            switch(type)
            {
                case EnvelopeType::ENVELOPE_TYPE_TX:
                {                    
                    new (&tx) Transaction();
                    break;
                }
                case EnvelopeType::ENVELOPE_TYPE_TX_FEE_BUMP:
                {
                    new (&feeBump) FeeBumpTransaction();
                    break;
                }
                default:break;
            }
        }
        TransactionSignaturePayload(FeeBumpTransaction t, QByteArray networkHash)
        {
            type = EnvelopeType::ENVELOPE_TYPE_TX_FEE_BUMP;            
            memcpy(networkId,networkHash.data(),sizeof(networkId));
            new (&feeBump) FeeBumpTransaction();
            feeBump=t;
        }
        TransactionSignaturePayload(Transaction t, QByteArray networkHash)
        {
            type = EnvelopeType::ENVELOPE_TYPE_TX;
            memcpy(networkId,networkHash.data(),sizeof(networkId));
            new (&tx) Transaction();
            tx=t;
        }
        ~TransactionSignaturePayload()
        {
            clear();
        }
    private:
        friend inline QDataStream &operator>>(QDataStream &in,  TransactionSignaturePayload &obj);
        // Destroys the live arm, if any; the caller builds the next one.
        void clear()
        {
            switch(type)
            {
                case EnvelopeType::ENVELOPE_TYPE_TX:
                {
                    tx.~Transaction();
                    break;
                }
                case EnvelopeType::ENVELOPE_TYPE_TX_FEE_BUMP:
                {
                    feeBump.~FeeBumpTransaction();
                    break;
                }
                default:break;
            }
        }

    };

    inline QDataStream &operator<<(QDataStream &out, const  TransactionSignaturePayload &obj) {
        switch(obj.type)
        {
            case EnvelopeType::ENVELOPE_TYPE_TX:
            {
                out << obj.networkId << obj.type << obj.tx;
                break;
            }
            case EnvelopeType::ENVELOPE_TYPE_TX_FEE_BUMP:
            {
                out << obj.networkId << obj.type << obj.feeBump;
                break;
            }
            // What is signed is the hash of this: a network id and a bare
            // discriminant, hashed and signed as if it were a transaction.
            default: out.setStatus(QDataStream::WriteFailed); break;
        }

       return out;
    }

   inline  QDataStream &operator>>(QDataStream &in,  TransactionSignaturePayload &obj) {
       // The arm is built fresh below, never over the live one (which leaked
       // it), and a type with no arm is refused: it was read as nothing.
       obj.clear();
       obj.type = EnvelopeType::ENVELOPE_TYPE_TX_V0;     // no arm is live now
       in >> obj.networkId >> obj.type;
       switch(obj.type)
       {
           case EnvelopeType::ENVELOPE_TYPE_TX:
           {
               new (&obj.tx) Transaction();
               in >>obj.tx;
               break;
           }
           case EnvelopeType::ENVELOPE_TYPE_TX_FEE_BUMP:
           {
               new (&obj.feeBump) FeeBumpTransaction();
               in >>obj.feeBump;
               break;
           }
           default:     // no arm was built, and clear() destroys none for this type
               in.setStatus(QDataStream::ReadCorruptData);
               break;
       }
       return in;
    }


    /* Operation Results section */

    /* This result is used when offers are taken during an operation */
    struct ClaimOfferAtom
    {
        // emitted to identify the offer
        AccountID sellerID; // Account that owns the offer
        qint64 offerID;

        // amount and asset taken from the owner
        Asset assetSold;
        qint64 amountSold;

        // amount and asset sent to the owner
        Asset assetBought;
        qint64 amountBought;
    };
    inline QDataStream &operator<<(QDataStream &out, const  ClaimOfferAtom &obj) {
        out << obj.sellerID << obj.offerID << obj.assetSold << obj.amountSold << obj.assetBought << obj.amountBought;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  ClaimOfferAtom &obj) {
       in >> obj.sellerID >> obj.offerID >> obj.assetSold >> obj.amountSold >> obj.assetBought >> obj.amountBought;
       return in;
    }

    enum class ClaimAtomType : qint32
    {
        CLAIM_ATOM_TYPE_V0 = 0,
        CLAIM_ATOM_TYPE_ORDER_BOOK = 1,
        CLAIM_ATOM_TYPE_LIQUIDITY_POOL = 2
    };
    XDR_ENUM_SERIALIZER(ClaimAtomType)

    /* The pre-protocol-18 form of ClaimOfferAtom: the seller as a raw key. */
    struct ClaimOfferAtomV0
    {
        uint256 sellerEd25519 = {};
        qint64 offerID = 0;
        Asset assetSold;
        qint64 amountSold = 0;
        Asset assetBought;
        qint64 amountBought = 0;
    };
    inline QDataStream &operator<<(QDataStream &out, const ClaimOfferAtomV0 &obj) {
        out << obj.sellerEd25519 << obj.offerID << obj.assetSold << obj.amountSold
            << obj.assetBought << obj.amountBought;
        return out;
    }
    inline QDataStream &operator>>(QDataStream &in, ClaimOfferAtomV0 &obj) {
        in >> obj.sellerEd25519 >> obj.offerID >> obj.assetSold >> obj.amountSold
           >> obj.assetBought >> obj.amountBought;
        return in;
    }

    /* A trade against a liquidity pool rather than an offer. */
    struct ClaimLiquidityAtom
    {
        PoolID liquidityPoolID = {};
        Asset assetSold;
        qint64 amountSold = 0;
        Asset assetBought;
        qint64 amountBought = 0;
    };
    inline QDataStream &operator<<(QDataStream &out, const ClaimLiquidityAtom &obj) {
        out << obj.liquidityPoolID << obj.assetSold << obj.amountSold
            << obj.assetBought << obj.amountBought;
        return out;
    }
    inline QDataStream &operator>>(QDataStream &in, ClaimLiquidityAtom &obj) {
        in >> obj.liquidityPoolID >> obj.assetSold >> obj.amountSold
           >> obj.assetBought >> obj.amountBought;
        return in;
    }

    /**
     * One offer or pool an offer or a path payment traded against:
     *
     *   union ClaimAtom switch (ClaimAtomType type) {
     *   case CLAIM_ATOM_TYPE_V0:             ClaimOfferAtomV0 v0;
     *   case CLAIM_ATOM_TYPE_ORDER_BOOK:     ClaimOfferAtom orderBook;
     *   case CLAIM_ATOM_TYPE_LIQUIDITY_POOL: ClaimLiquidityAtom liquidityPool;
     *   };
     *
     * The results used to hold bare ClaimOfferAtoms, with no discriminant: an
     * offer that crossed anything was misread from there on — the offer id
     * of the caller's own offer came back as 0, or as another seller's.
     * Only the member `type` names is read or written.
     */
    struct ClaimAtom
    {
        ClaimAtomType type = ClaimAtomType::CLAIM_ATOM_TYPE_ORDER_BOOK;
        ClaimOfferAtomV0 v0;
        ClaimOfferAtom orderBook;
        ClaimLiquidityAtom liquidityPool;
    };
    inline QDataStream &operator<<(QDataStream &out, const ClaimAtom &obj) {
        switch (obj.type) {
        case ClaimAtomType::CLAIM_ATOM_TYPE_V0:             out << obj.type << obj.v0; break;
        case ClaimAtomType::CLAIM_ATOM_TYPE_ORDER_BOOK:     out << obj.type << obj.orderBook; break;
        case ClaimAtomType::CLAIM_ATOM_TYPE_LIQUIDITY_POOL: out << obj.type << obj.liquidityPool; break;
        default: out.setStatus(QDataStream::WriteFailed); break;   // no arm: nothing to write
        }
        return out;
    }
    inline QDataStream &operator>>(QDataStream &in, ClaimAtom &obj) {
        in >> obj.type;
        switch (obj.type) {
        case ClaimAtomType::CLAIM_ATOM_TYPE_V0:             in >> obj.v0; break;
        case ClaimAtomType::CLAIM_ATOM_TYPE_ORDER_BOOK:     in >> obj.orderBook; break;
        case ClaimAtomType::CLAIM_ATOM_TYPE_LIQUIDITY_POOL: in >> obj.liquidityPool; break;
        default: in.setStatus(QDataStream::ReadCorruptData); break;
        }
        return in;
    }

    /******* CreateAccount Result ********/

    enum class CreateAccountResultCode : qint32
    {
        // codes considered as "success" for the operation
        CREATE_ACCOUNT_SUCCESS = 0, // account was created

        // codes considered as "failure" for the operation
        CREATE_ACCOUNT_MALFORMED = -1,   // invalid destination
        CREATE_ACCOUNT_UNDERFUNDED = -2, // not enough funds in source account
        CREATE_ACCOUNT_LOW_RESERVE =
            -3, // would create an account below the min reserve
        CREATE_ACCOUNT_ALREADY_EXIST = -4 // account already exists
    };
    XDR_RESULT_CODE_SERIALIZER(CreateAccountResultCode, CREATE_ACCOUNT_ALREADY_EXIST, CREATE_ACCOUNT_SUCCESS)
    struct alignas(4) CreateAccountResult
    {
        CreateAccountResultCode code;
    };
    XDR_CODE_STRUCT_SERIALIZER(CreateAccountResult)

    /******* Payment Result ********/

    enum class PaymentResultCode : qint32
    {
        // codes considered as "success" for the operation
        PAYMENT_SUCCESS = 0, // payment successfuly completed

        // codes considered as "failure" for the operation
        PAYMENT_MALFORMED = -1,          // bad input
        PAYMENT_UNDERFUNDED = -2,        // not enough funds in source account
        PAYMENT_SRC_NO_TRUST = -3,       // no trust line on source account
        PAYMENT_SRC_NOT_AUTHORIZED = -4, // source not authorized to transfer
        PAYMENT_NO_DESTINATION = -5,     // destination account does not exist
        PAYMENT_NO_TRUST = -6,       // destination missing a trust line for asset
        PAYMENT_NOT_AUTHORIZED = -7, // destination not authorized to hold asset
        PAYMENT_LINE_FULL = -8,      // destination would go above their limit
        PAYMENT_NO_ISSUER = -9       // missing issuer on asset
    };
    XDR_RESULT_CODE_SERIALIZER(PaymentResultCode, PAYMENT_NO_ISSUER, PAYMENT_SUCCESS)

    struct PaymentResult {
        PaymentResultCode code;
        PaymentResult():code(PaymentResultCode::PAYMENT_SUCCESS){

        }
    };
    XDR_CODE_STRUCT_SERIALIZER(PaymentResult)

    /******* PathPaymentStrictReceive Result ********/

    enum class PathPaymentStrictReceiveResultCode: qint32
    {
        // codes considered as "success" for the operation
        PATH_PAYMENT_STRICT_RECEIVE_SUCCESS = 0, // success

        // codes considered as "failure" for the operation
        PATH_PAYMENT_STRICT_RECEIVE_MALFORMED = -1,          // bad input
        PATH_PAYMENT_STRICT_RECEIVE_UNDERFUNDED = -2,        // not enough funds in source account
        PATH_PAYMENT_STRICT_RECEIVE_SRC_NO_TRUST = -3,       // no trust line on source account
        PATH_PAYMENT_STRICT_RECEIVE_SRC_NOT_AUTHORIZED = -4, // source not authorized to transfer
        PATH_PAYMENT_STRICT_RECEIVE_NO_DESTINATION = -5,     // destination account does not exist
        PATH_PAYMENT_STRICT_RECEIVE_NO_TRUST = -6,           // dest missing a trust line for asset
        PATH_PAYMENT_STRICT_RECEIVE_NOT_AUTHORIZED = -7,     // dest not authorized to hold asset
        PATH_PAYMENT_STRICT_RECEIVE_LINE_FULL = -8,          // dest would go above their limit
        PATH_PAYMENT_STRICT_RECEIVE_NO_ISSUER = -9,          // missing issuer on one asset
        PATH_PAYMENT_STRICT_RECEIVE_TOO_FEW_OFFERS = -10,    // not enough offers to satisfy path
        PATH_PAYMENT_STRICT_RECEIVE_OFFER_CROSS_SELF = -11,  // would cross one of its own offers
        PATH_PAYMENT_STRICT_RECEIVE_OVER_SENDMAX = -12       // could not satisfy sendmax
    };
    XDR_RESULT_CODE_SERIALIZER(PathPaymentStrictReceiveResultCode,
                               PATH_PAYMENT_STRICT_RECEIVE_OVER_SENDMAX, PATH_PAYMENT_STRICT_RECEIVE_SUCCESS)

    /******* PathPaymentStrictSend Result ********/

    enum class PathPaymentStrictSendResultCode: qint32
    {
        // codes considered as "success" for the operation
        PATH_PAYMENT_STRICT_SEND_SUCCESS = 0, // success

        // codes considered as "failure" for the operation
        PATH_PAYMENT_STRICT_SEND_MALFORMED = -1,          // bad input
        PATH_PAYMENT_STRICT_SEND_UNDERFUNDED = -2,        // not enough funds in source account
        PATH_PAYMENT_STRICT_SEND_SRC_NO_TRUST = -3,       // no trust line on source account
        PATH_PAYMENT_STRICT_SEND_SRC_NOT_AUTHORIZED = -4, // source not authorized to transfer
        PATH_PAYMENT_STRICT_SEND_NO_DESTINATION = -5,     // destination account does not exist
        PATH_PAYMENT_STRICT_SEND_NO_TRUST = -6,           // dest missing a trust line for asset
        PATH_PAYMENT_STRICT_SEND_NOT_AUTHORIZED = -7,     // dest not authorized to hold asset
        PATH_PAYMENT_STRICT_SEND_LINE_FULL = -8,          // dest would go above their limit
        PATH_PAYMENT_STRICT_SEND_NO_ISSUER = -9,          // missing issuer on one asset
        PATH_PAYMENT_STRICT_SEND_TOO_FEW_OFFERS = -10,    // not enough offers to satisfy path
        PATH_PAYMENT_STRICT_SEND_OFFER_CROSS_SELF = -11,  // would cross one of its own offers
        PATH_PAYMENT_STRICT_SEND_OVER_SENDMAX = -12,      // could not satisfy destMin (the name this SDK used)
        PATH_PAYMENT_STRICT_SEND_UNDER_DESTMIN = -12      // stellar-xdr's name for the same code
    };
    XDR_RESULT_CODE_SERIALIZER(PathPaymentStrictSendResultCode,
                               PATH_PAYMENT_STRICT_SEND_UNDER_DESTMIN, PATH_PAYMENT_STRICT_SEND_SUCCESS)

    struct SimplePaymentResult
    {
        AccountID destination;
        Asset asset;
        qint64 amount;
    };
    inline QDataStream &operator<<(QDataStream &out, const  SimplePaymentResult &obj) {
        out << obj.destination << obj.asset << obj.amount;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  SimplePaymentResult &obj) {
        in >> obj.destination >> obj.asset >> obj.amount;
       return in;
    }

    struct PathPaymentStrictReceiveResult {
        PathPaymentStrictReceiveResultCode code;
        QVector<ClaimAtom> offers; //<>;
        union{
            SimplePaymentResult last;
            Asset noIssuer; // the asset that caused the error
        };
        // Asset has a constructor, so the union needs one spelled out.
        PathPaymentStrictReceiveResult()
            : code(PathPaymentStrictReceiveResultCode::PATH_PAYMENT_STRICT_RECEIVE_SUCCESS)
        {
            new (&last) SimplePaymentResult();
        }
    };
    inline QDataStream &operator<<(QDataStream &out, const  PathPaymentStrictReceiveResult &obj) {
        out << obj.code;
        switch(obj.code){
        case PathPaymentStrictReceiveResultCode::PATH_PAYMENT_STRICT_RECEIVE_SUCCESS:
            writeXdrList(out, obj.offers) << obj.last; break;
        case PathPaymentStrictReceiveResultCode::PATH_PAYMENT_STRICT_RECEIVE_NO_ISSUER:
            out << obj.noIssuer; break;
        default: break;
        }
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  PathPaymentStrictReceiveResult &obj) {
        in >> obj.code;
        switch(obj.code){
        case PathPaymentStrictReceiveResultCode::PATH_PAYMENT_STRICT_RECEIVE_SUCCESS:
            readXdrList(in, obj.offers) >> obj.last; break;
        case PathPaymentStrictReceiveResultCode::PATH_PAYMENT_STRICT_RECEIVE_NO_ISSUER:
            in >> obj.noIssuer; break;
        default: break;
        }
       return in;
    }

    struct PathPaymentStrictSendResult {
        PathPaymentStrictSendResultCode code;
        QVector<ClaimAtom> offers; //<>;
        union{
            SimplePaymentResult last;
            Asset noIssuer; // the asset that caused the error
        };
        PathPaymentStrictSendResult()
            : code(PathPaymentStrictSendResultCode::PATH_PAYMENT_STRICT_SEND_SUCCESS)
        {
            new (&last) SimplePaymentResult();
        }
    };
    inline QDataStream &operator<<(QDataStream &out, const  PathPaymentStrictSendResult &obj) {
        out << obj.code;
        switch(obj.code){
        case PathPaymentStrictSendResultCode::PATH_PAYMENT_STRICT_SEND_SUCCESS:
            writeXdrList(out, obj.offers) << obj.last; break;
        case PathPaymentStrictSendResultCode::PATH_PAYMENT_STRICT_SEND_NO_ISSUER:
            out << obj.noIssuer; break;
        default: break;
        }
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  PathPaymentStrictSendResult &obj) {
        in >> obj.code;
        switch(obj.code){
        case PathPaymentStrictSendResultCode::PATH_PAYMENT_STRICT_SEND_SUCCESS:
            readXdrList(in, obj.offers) >> obj.last; break;
        case PathPaymentStrictSendResultCode::PATH_PAYMENT_STRICT_SEND_NO_ISSUER:
            in >> obj.noIssuer; break;
        default: break;
        }
       return in;
    }

    /******* ManageSellOffer Result ********/

    enum class ManageSellOfferResultCode :qint32
    {
        // codes considered as "success" for the operation
        MANAGE_SELL_OFFER_SUCCESS  = 0,

        // codes considered as "failure" for the operation
        MANAGE_SELL_OFFER_MALFORMED  = -1,     // generated offer would be invalid
        MANAGE_SELL_OFFER_SELL_NO_TRUST = -2, // no trust line for what we're selling
        MANAGE_SELL_OFFER_BUY_NO_TRUST = -3,  // no trust line for what we're buying
        MANAGE_SELL_OFFER_SELL_NOT_AUTHORIZED = -4, // not authorized to sell
        MANAGE_SELL_OFFER_BUY_NOT_AUTHORIZED = -5,  // not authorized to buy
        MANAGE_SELL_OFFER_LINE_FULL = -6,      // can't receive more of what it's buying
        MANAGE_SELL_OFFER_UNDERFUNDED = -7,    // doesn't hold what it's trying to sell
        MANAGE_SELL_OFFER_CROSS_SELF = -8,     // would cross an offer from the same user
        MANAGE_SELL_OFFER_SELL_NO_ISSUER = -9, // no issuer for what we're selling
        MANAGE_SELL_OFFER_BUY_NO_ISSUER = -10, // no issuer for what we're buying

        // update errors
        MANAGE_SELL_OFFER_NOT_FOUND = -11, // offerID does not match an existing offer

        MANAGE_SELL_OFFER_LOW_RESERVE = -12 // not enough funds to create a new Offer
    };
    XDR_RESULT_CODE_SERIALIZER(ManageSellOfferResultCode, MANAGE_SELL_OFFER_LOW_RESERVE, MANAGE_SELL_OFFER_SUCCESS)


    enum class ManageOfferEffect : qint32
    {
        MANAGE_OFFER_CREATED = 0,
        MANAGE_OFFER_UPDATED = 1,
        MANAGE_OFFER_DELETED = 2
    };
    XDR_ENUM_SERIALIZER(ManageOfferEffect)


    struct ManageOfferSuccessResult
    {
        // offers that got claimed while creating this offer
        QVector<ClaimAtom> offersClaimed;
        ManageOfferEffect effect;
        OfferEntry offer;
    };
    inline QDataStream &operator<<(QDataStream &out, const  ManageOfferSuccessResult &obj) {
        switch(obj.effect){
        case ManageOfferEffect::MANAGE_OFFER_CREATED:
        case ManageOfferEffect::MANAGE_OFFER_UPDATED:
            writeXdrList(out, obj.offersClaimed) << obj.effect << obj.offer; break;
        case ManageOfferEffect::MANAGE_OFFER_DELETED:
            writeXdrList(out, obj.offersClaimed) << obj.effect; break;
        default: out.setStatus(QDataStream::WriteFailed); break;   // no arm: nothing to write
        }
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  ManageOfferSuccessResult &obj) {
        readXdrList(in, obj.offersClaimed) >> obj.effect;
        switch(obj.effect){
        case ManageOfferEffect::MANAGE_OFFER_CREATED:
        case ManageOfferEffect::MANAGE_OFFER_UPDATED:
            in >> obj.offer; break;
        case ManageOfferEffect::MANAGE_OFFER_DELETED: break;
        default: in.setStatus(QDataStream::ReadCorruptData); break;
        }
       return in;
    }


    struct ManageSellOfferResult
    {
        ManageSellOfferResultCode code;
        ManageOfferSuccessResult success;
    };

    inline QDataStream &operator<<(QDataStream &out, const  ManageSellOfferResult &obj) {
        out << obj.code;
        switch(obj.code){
        case ManageSellOfferResultCode::MANAGE_SELL_OFFER_SUCCESS :
            out << obj.success; break;
        default: break;
        }

       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  ManageSellOfferResult &obj) {
        in >> obj.code;
        switch(obj.code){
        case ManageSellOfferResultCode::MANAGE_SELL_OFFER_SUCCESS :
            in >> obj.success; break;
        default: break;
        }
       return in;
    }

    /******* ManageBuyOffer Result ********/

    enum class ManageBuyOfferResultCode : qint32
    {
        // codes considered as "success" for the operation
        MANAGE_BUY_OFFER_SUCCESS = 0,

        // codes considered as "failure" for the operation
        MANAGE_BUY_OFFER_MALFORMED = -1,     // generated offer would be invalid
        MANAGE_BUY_OFFER_SELL_NO_TRUST = -2, // no trust line for what we're selling
        MANAGE_BUY_OFFER_BUY_NO_TRUST = -3,  // no trust line for what we're buying
        MANAGE_BUY_OFFER_SELL_NOT_AUTHORIZED = -4, // not authorized to sell
        MANAGE_BUY_OFFER_BUY_NOT_AUTHORIZED = -5,  // not authorized to buy
        MANAGE_BUY_OFFER_LINE_FULL = -6,      // can't receive more of what it's buying
        MANAGE_BUY_OFFER_UNDERFUNDED = -7,    // doesn't hold what it's trying to sell
        MANAGE_BUY_OFFER_CROSS_SELF = -8,     // would cross an offer from the same user
        MANAGE_BUY_OFFER_SELL_NO_ISSUER = -9, // no issuer for what we're selling
        MANAGE_BUY_OFFER_BUY_NO_ISSUER = -10, // no issuer for what we're buying

        // update errors
        MANAGE_BUY_OFFER_NOT_FOUND = -11, // offerID does not match an existing offer

        MANAGE_BUY_OFFER_LOW_RESERVE = -12 // not enough funds to create a new Offer
    };
    XDR_RESULT_CODE_SERIALIZER(ManageBuyOfferResultCode, MANAGE_BUY_OFFER_LOW_RESERVE, MANAGE_BUY_OFFER_SUCCESS)

    struct ManageBuyOfferResult
    {
        ManageBuyOfferResultCode code;
        ManageOfferSuccessResult success;
    };

    inline QDataStream &operator<<(QDataStream &out, const  ManageBuyOfferResult &obj) {
        out << obj.code;
        switch(obj.code){
        case ManageBuyOfferResultCode::MANAGE_BUY_OFFER_SUCCESS :
            out << obj.success; break;
        default: break;
        }

       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  ManageBuyOfferResult &obj) {
        in >> obj.code;
        switch(obj.code){
        case ManageBuyOfferResultCode::MANAGE_BUY_OFFER_SUCCESS :
            in >> obj.success; break;
        default: break;
        }
       return in;
    }

    /******* SetOptions Result ********/

    enum class SetOptionsResultCode :qint32
    {
        // codes considered as "success" for the operation
        SET_OPTIONS_SUCCESS = 0,
        // codes considered as "failure" for the operation
        SET_OPTIONS_LOW_RESERVE = -1,      // not enough funds to add a signer
        SET_OPTIONS_TOO_MANY_SIGNERS = -2, // max number of signers already reached
        SET_OPTIONS_BAD_FLAGS = -3,        // invalid combination of clear/set flags
        SET_OPTIONS_INVALID_INFLATION = -4,      // inflation account does not exist
        SET_OPTIONS_CANT_CHANGE = -5,            // can no longer change this option
        SET_OPTIONS_UNKNOWN_FLAG = -6,           // can't set an unknown flag
        SET_OPTIONS_THRESHOLD_OUT_OF_RANGE = -7, // bad value for weight/threshold
        SET_OPTIONS_BAD_SIGNER = -8,             // signer cannot be masterkey
        SET_OPTIONS_INVALID_HOME_DOMAIN = -9,     // malformed home domain
        SET_OPTIONS_AUTH_REVOCABLE_REQUIRED = -10 // auth revocable is required for clawback
    };
    XDR_RESULT_CODE_SERIALIZER(SetOptionsResultCode, SET_OPTIONS_AUTH_REVOCABLE_REQUIRED, SET_OPTIONS_SUCCESS)

    struct alignas(4) SetOptionsResult
    {
        SetOptionsResultCode code;
    };
    XDR_CODE_STRUCT_SERIALIZER(SetOptionsResult)

    /******* ChangeTrust Result ********/

    enum class ChangeTrustResultCode : qint32
    {
        // codes considered as "success" for the operation
        CHANGE_TRUST_SUCCESS = 0,
        // codes considered as "failure" for the operation
        CHANGE_TRUST_MALFORMED = -1,     // bad input
        CHANGE_TRUST_NO_ISSUER = -2,     // could not find issuer
        CHANGE_TRUST_INVALID_LIMIT = -3, // cannot drop limit below balance
                                         // cannot create with a limit of 0
        CHANGE_TRUST_LOW_RESERVE = -4, // not enough funds to create a new trust line,
        CHANGE_TRUST_SELF_NOT_ALLOWED = -5, // trusting self is not allowed
        CHANGE_TRUST_TRUST_LINE_MISSING = -6, // Asset trustline is missing for pool
        CHANGE_TRUST_CANNOT_DELETE = -7, // Asset trustline is still referenced in a pool
        CHANGE_TRUST_NOT_AUTH_MAINTAIN_LIABILITIES = -8 // Asset trustline is deauthorized
    };
    XDR_RESULT_CODE_SERIALIZER(ChangeTrustResultCode,
                               CHANGE_TRUST_NOT_AUTH_MAINTAIN_LIABILITIES, CHANGE_TRUST_SUCCESS)

    struct alignas(4) ChangeTrustResult
    {
        ChangeTrustResultCode code;
    };
    XDR_CODE_STRUCT_SERIALIZER(ChangeTrustResult)

    /******* AllowTrust Result ********/

    enum class AllowTrustResultCode :qint32
    {
        // codes considered as "success" for the operation
        ALLOW_TRUST_SUCCESS = 0,
        // codes considered as "failure" for the operation
        ALLOW_TRUST_MALFORMED = -1,     // asset is not ASSET_TYPE_ALPHANUM
        ALLOW_TRUST_NO_TRUST_LINE = -2, // trustor does not have a trustline
                                        // source account does not require trust
        ALLOW_TRUST_TRUST_NOT_REQUIRED = -3,
        ALLOW_TRUST_CANT_REVOKE = -4, // source account can't revoke trust,
        ALLOW_TRUST_SELF_NOT_ALLOWED = -5, // trusting self is not allowed
        ALLOW_TRUST_LOW_RESERVE = -6 // claimable balances can't be created
                                     // on revoke due to low reserves
    };
    XDR_RESULT_CODE_SERIALIZER(AllowTrustResultCode, ALLOW_TRUST_LOW_RESERVE, ALLOW_TRUST_SUCCESS)

    struct alignas(4) AllowTrustResult
    {
        AllowTrustResultCode code;
    };
    XDR_CODE_STRUCT_SERIALIZER(AllowTrustResult)

    /******* AccountMerge Result ********/

    enum class AccountMergeResultCode : qint32
    {
        // codes considered as "success" for the operation
        ACCOUNT_MERGE_SUCCESS = 0,
        // codes considered as "failure" for the operation
        ACCOUNT_MERGE_MALFORMED = -1,      // can't merge onto itself
        ACCOUNT_MERGE_NO_ACCOUNT = -2,     // destination does not exist
        ACCOUNT_MERGE_IMMUTABLE_SET = -3,  // source account has AUTH_IMMUTABLE set
        ACCOUNT_MERGE_HAS_SUB_ENTRIES = -4, // account has trust lines/offers
        ACCOUNT_MERGE_SEQNUM_TOO_FAR = -5, // sequence number is over max allowed
        ACCOUNT_MERGE_DEST_FULL = -6, // can't add source balance to destination balance
        ACCOUNT_MERGE_IS_SPONSOR = -7       // can't merge account that is a sponsor
    };
    XDR_RESULT_CODE_SERIALIZER(AccountMergeResultCode, ACCOUNT_MERGE_IS_SPONSOR, ACCOUNT_MERGE_SUCCESS)

    struct AccountMergeResult
    {
        AccountMergeResultCode code;
        qint64 sourceAccountBalance; // how much got transfered from source account
    };

    inline QDataStream &operator<<(QDataStream &out, const  AccountMergeResult &obj) {
        out << obj.code;
        switch(obj.code){
        case AccountMergeResultCode::ACCOUNT_MERGE_SUCCESS:
            out << obj.sourceAccountBalance; break;
        default: break;
        }

       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  AccountMergeResult &obj) {
        in >> obj.code;
        switch(obj.code){
        case AccountMergeResultCode::ACCOUNT_MERGE_SUCCESS:
            in >> obj.sourceAccountBalance; break;
        default: break;
        }
       return in;
    }

    /******* Inflation Result ********/

    enum class InflationResultCode : qint32
    {
        // codes considered as "success" for the operation
        INFLATION_SUCCESS = 0,
        // codes considered as "failure" for the operation
        INFLATION_NOT_TIME = -1
    };
    XDR_RESULT_CODE_SERIALIZER(InflationResultCode, INFLATION_NOT_TIME, INFLATION_SUCCESS)

    struct InflationPayout // or use PaymentResultAtom to limit types?
    {
        AccountID destination;
        qint64 amount;
    };
    inline QDataStream &operator<<(QDataStream &out, const  InflationPayout &obj) {
        out << obj.destination << obj.amount;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  InflationPayout &obj) {
        in >> obj.destination >> obj.amount;
       return in;
    }

    struct InflationResult
    {
        InflationResultCode code;
        QVector<InflationPayout> payouts;
    };
    inline QDataStream &operator<<(QDataStream &out, const  InflationResult &obj) {
        out << obj.code;
        switch(obj.code){
        case InflationResultCode::INFLATION_SUCCESS:
            writeXdrList(out, obj.payouts); break;
        default: break;
        }

       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  InflationResult &obj) {
        in >> obj.code;
        switch(obj.code){
        case InflationResultCode::INFLATION_SUCCESS:
            readXdrList(in, obj.payouts); break;
        default: break;
        }
       return in;
    }

    /******* ManageData Result ********/

    enum class ManageDataResultCode : qint32
    {
        // codes considered as "success" for the operation
        MANAGE_DATA_SUCCESS = 0,
        // codes considered as "failure" for the operation
        MANAGE_DATA_NOT_SUPPORTED_YET = -1, // The network hasn't moved to this protocol change yet
        MANAGE_DATA_NAME_NOT_FOUND = -2,    // Trying to remove a Data Entry that isn't there
        MANAGE_DATA_LOW_RESERVE = -3,       // not enough funds to create a new Data Entry
        MANAGE_DATA_INVALID_NAME = -4       // Name not a valid string
    };
    XDR_RESULT_CODE_SERIALIZER(ManageDataResultCode, MANAGE_DATA_INVALID_NAME, MANAGE_DATA_SUCCESS)

    struct alignas(4) ManageDataResult
    {
        ManageDataResultCode code;
    };
    XDR_CODE_STRUCT_SERIALIZER(ManageDataResult)


    enum class BumpSequenceResultCode : qint32
    {
        // codes considered as "success" for the operation
        BUMP_SEQUENCE_SUCCESS = 0,
        // codes considerer as "failure" for the operation
        BUMP_SEQUENCE_BAD_SEQ = -1  //  'bumpTo' is not within bounds
    };
    XDR_RESULT_CODE_SERIALIZER(BumpSequenceResultCode, BUMP_SEQUENCE_BAD_SEQ, BUMP_SEQUENCE_SUCCESS)

    struct alignas(4) BumpSequenceResult
    {
         BumpSequenceResultCode code;
    };
    XDR_CODE_STRUCT_SERIALIZER(BumpSequenceResult)

    /******* CreateClaimableBalance Result ********/

    enum class CreateClaimableBalanceResultCode: qint32
    {
        CREATE_CLAIMABLE_BALANCE_SUCCESS = 0,
        CREATE_CLAIMABLE_BALANCE_MALFORMED = -1,
        CREATE_CLAIMABLE_BALANCE_LOW_RESERVE = -2,
        CREATE_CLAIMABLE_BALANCE_NO_TRUST = -3,
        CREATE_CLAIMABLE_BALANCE_NOT_AUTHORIZED = -4,
        CREATE_CLAIMABLE_BALANCE_UNDERFUNDED = -5
    };
    XDR_RESULT_CODE_SERIALIZER(CreateClaimableBalanceResultCode,
                               CREATE_CLAIMABLE_BALANCE_UNDERFUNDED, CREATE_CLAIMABLE_BALANCE_SUCCESS)

    // A struct: as a union, `code` and the balance id's type shared storage,
    // so setting a balance id rewrote the code (both V0 and SUCCESS are 0,
    // which is why decoding worked).
    struct CreateClaimableBalanceResult
    {
        CreateClaimableBalanceResultCode code = CreateClaimableBalanceResultCode::CREATE_CLAIMABLE_BALANCE_SUCCESS;
        ClaimableBalanceID balanceID; // case CREATE_CLAIMABLE_BALANCE_SUCCESS:
                //default void
    };

    inline QDataStream &operator<<(QDataStream &out, const  CreateClaimableBalanceResult &obj) {
        out << obj.code;
        switch(obj.code){
        case CreateClaimableBalanceResultCode::CREATE_CLAIMABLE_BALANCE_SUCCESS:
            out << obj.balanceID; break;
        default: break;
        }

       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  CreateClaimableBalanceResult &obj) {
        in >> obj.code;
        switch(obj.code){
        case CreateClaimableBalanceResultCode::CREATE_CLAIMABLE_BALANCE_SUCCESS:
            in >> obj.balanceID; break;
        default: break;
        }
       return in;
    }

    /******* ClaimClaimableBalance Result ********/

    enum class ClaimClaimableBalanceResultCode : qint32
    {
        CLAIM_CLAIMABLE_BALANCE_SUCCESS = 0,
        CLAIM_CLAIMABLE_BALANCE_DOES_NOT_EXIST = -1,
        CLAIM_CLAIMABLE_BALANCE_CANNOT_CLAIM = -2,
        CLAIM_CLAIMABLE_BALANCE_LINE_FULL = -3,
        CLAIM_CLAIMABLE_BALANCE_NO_TRUST = -4,
        CLAIM_CLAIMABLE_BALANCE_NOT_AUTHORIZED = -5,
        CLAIM_CLAIMABLE_BALANCE_TRUSTLINE_FROZEN = -6
    };
    XDR_RESULT_CODE_SERIALIZER(ClaimClaimableBalanceResultCode,
                               CLAIM_CLAIMABLE_BALANCE_TRUSTLINE_FROZEN, CLAIM_CLAIMABLE_BALANCE_SUCCESS)

    union ClaimClaimableBalanceResult
    {
        ClaimClaimableBalanceResultCode code;
//        union
//        {
//            //void case CLAIM_CLAIMABLE_BALANCE_SUCCESS:
//            //default void
//        };
    };
    inline QDataStream &operator<<(QDataStream &out, const  ClaimClaimableBalanceResult &obj) {
        out << obj.code;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  ClaimClaimableBalanceResult &obj) {
        in >> obj.code;
       return in;
    }
    /******* BeginSponsoringFutureReserves Result ********/

    enum class BeginSponsoringFutureReservesResultCode: qint32
    {
        // codes considered as "success" for the operation
        BEGIN_SPONSORING_FUTURE_RESERVES_SUCCESS = 0,

        // codes considered as "failure" for the operation
        BEGIN_SPONSORING_FUTURE_RESERVES_MALFORMED = -1,
        BEGIN_SPONSORING_FUTURE_RESERVES_ALREADY_SPONSORED = -2,
        BEGIN_SPONSORING_FUTURE_RESERVES_RECURSIVE = -3
    };
    XDR_RESULT_CODE_SERIALIZER(BeginSponsoringFutureReservesResultCode,
                               BEGIN_SPONSORING_FUTURE_RESERVES_RECURSIVE, BEGIN_SPONSORING_FUTURE_RESERVES_SUCCESS)

    union BeginSponsoringFutureReservesResult
    {
        BeginSponsoringFutureReservesResultCode code;
        //        union
        //        {
        //            //void case BEGIN_SPONSORING_FUTURE_RESERVES_SUCCESS:
        //            //default void
        //        };
    };
    inline QDataStream &operator<<(QDataStream &out, const  BeginSponsoringFutureReservesResult &obj) {
        out << obj.code;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  BeginSponsoringFutureReservesResult &obj) {
        in >> obj.code;
       return in;
    }
    /******* EndSponsoringFutureReserves Result ********/

    enum class EndSponsoringFutureReservesResultCode: qint32
    {
        // codes considered as "success" for the operation
        END_SPONSORING_FUTURE_RESERVES_SUCCESS = 0,

        // codes considered as "failure" for the operation
        END_SPONSORING_FUTURE_RESERVES_NOT_SPONSORED = -1
    };
    XDR_RESULT_CODE_SERIALIZER(EndSponsoringFutureReservesResultCode,
                               END_SPONSORING_FUTURE_RESERVES_NOT_SPONSORED, END_SPONSORING_FUTURE_RESERVES_SUCCESS)

    union EndSponsoringFutureReservesResult
    {
        EndSponsoringFutureReservesResultCode code;
        //        union
        //        {
        //            //void case END_SPONSORING_FUTURE_RESERVES_SUCCESS:
        //            //default void
        //        };
    };
    inline QDataStream &operator<<(QDataStream &out, const  EndSponsoringFutureReservesResult &obj) {
        out << obj.code;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  EndSponsoringFutureReservesResult &obj) {
        in >> obj.code;
       return in;
    }
    /******* RevokeSponsorship Result ********/

    enum class RevokeSponsorshipResultCode: qint32
    {
        // codes considered as "success" for the operation
        REVOKE_SPONSORSHIP_SUCCESS = 0,

        // codes considered as "failure" for the operation
        REVOKE_SPONSORSHIP_DOES_NOT_EXIST = -1,
        REVOKE_SPONSORSHIP_NOT_SPONSOR = -2,
        REVOKE_SPONSORSHIP_LOW_RESERVE = -3,
        REVOKE_SPONSORSHIP_ONLY_TRANSFERABLE = -4,
        REVOKE_SPONSORSHIP_MALFORMED = -5
    };
    XDR_RESULT_CODE_SERIALIZER(RevokeSponsorshipResultCode,
                               REVOKE_SPONSORSHIP_MALFORMED, REVOKE_SPONSORSHIP_SUCCESS)

    union RevokeSponsorshipResult
    {
        RevokeSponsorshipResultCode code;
        //        union
        //        {
        //            //void case REVOKE_SPONSORSHIP_SUCCESS:
        //            //default void
        //        };
    };
    inline QDataStream &operator<<(QDataStream &out, const  RevokeSponsorshipResult &obj) {
        out << obj.code;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  RevokeSponsorshipResult &obj) {
        in >> obj.code;
       return in;
    }

    /******* Clawback Result ********/

    enum class ClawbackResultCode : qint32
    {
        // codes considered as "success" for the operation
        CLAWBACK_SUCCESS = 0,

        // codes considered as "failure" for the operation
        CLAWBACK_MALFORMED = -1,
        CLAWBACK_NOT_CLAWBACK_ENABLED = -2,
        CLAWBACK_NO_TRUST = -3,
        CLAWBACK_UNDERFUNDED = -4
    };
    XDR_RESULT_CODE_SERIALIZER(ClawbackResultCode, CLAWBACK_UNDERFUNDED, CLAWBACK_SUCCESS)

    struct ClawbackResult
    {
        ClawbackResultCode code;
//        union{
//            case : CLAWBACK_SUCCESS void;
//            default: void
//        };
    };
    inline QDataStream &operator<<(QDataStream &out, const  ClawbackResult &obj) {
        out << obj.code;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  ClawbackResult &obj) {
        in >> obj.code;
       return in;
    }

    /******* ClawbackClaimableBalance Result ********/

    enum class ClawbackClaimableBalanceResultCode : qint32
    {
        // codes considered as "success" for the operation
        CLAWBACK_CLAIMABLE_BALANCE_SUCCESS = 0,

        // codes considered as "failure" for the operation
        CLAWBACK_CLAIMABLE_BALANCE_DOES_NOT_EXIST = -1,
        CLAWBACK_CLAIMABLE_BALANCE_NOT_ISSUER = -2,
        CLAWBACK_CLAIMABLE_BALANCE_NOT_CLAWBACK_ENABLED = -3
    };
    XDR_RESULT_CODE_SERIALIZER(ClawbackClaimableBalanceResultCode,
                               CLAWBACK_CLAIMABLE_BALANCE_NOT_CLAWBACK_ENABLED, CLAWBACK_CLAIMABLE_BALANCE_SUCCESS)

    struct ClawbackClaimableBalanceResult
    {
        ClawbackClaimableBalanceResultCode code;
//        union{
//            case CLAWBACK_CLAIMABLE_BALANCE_SUCCESS:
//                void;
//            default:
//                void;
//        };

    };
    inline QDataStream &operator<<(QDataStream &out, const  ClawbackClaimableBalanceResult &obj) {
        out << obj.code;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  ClawbackClaimableBalanceResult &obj) {
        in >> obj.code;
       return in;
    }

    /******* SetTrustLineFlags Result ********/

    enum class SetTrustLineFlagsResultCode : qint32
    {
        // codes considered as "success" for the operation
        SET_TRUST_LINE_FLAGS_SUCCESS = 0,

        // codes considered as "failure" for the operation
        SET_TRUST_LINE_FLAGS_MALFORMED = -1,
        SET_TRUST_LINE_FLAGS_NO_TRUST_LINE = -2,
        SET_TRUST_LINE_FLAGS_CANT_REVOKE = -3,
        SET_TRUST_LINE_FLAGS_INVALID_STATE = -4,
        SET_TRUST_LINE_FLAGS_LOW_RESERVE = -5 // claimable balances can't be created
                                              // on revoke due to low reserves
    };
    XDR_RESULT_CODE_SERIALIZER(SetTrustLineFlagsResultCode,
                               SET_TRUST_LINE_FLAGS_LOW_RESERVE, SET_TRUST_LINE_FLAGS_SUCCESS)

    union SetTrustLineFlagsResult
    {
        SetTrustLineFlagsResultCode code;
//        union{
//            case SET_TRUST_LINE_FLAGS_SUCCESS:
//                void;
//            default:
//                void;
//        };
    };
    inline QDataStream &operator<<(QDataStream &out, const  SetTrustLineFlagsResult &obj) {
        out << obj.code;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  SetTrustLineFlagsResult &obj) {
        in >> obj.code;
       return in;
    }


    /******* LiquidityPoolDeposit / LiquidityPoolWithdraw Result (CAP-38) ********/

    enum class LiquidityPoolDepositResultCode : qint32
    {
        LIQUIDITY_POOL_DEPOSIT_SUCCESS = 0,
        LIQUIDITY_POOL_DEPOSIT_MALFORMED = -1,
        LIQUIDITY_POOL_DEPOSIT_NO_TRUST = -2,
        LIQUIDITY_POOL_DEPOSIT_NOT_AUTHORIZED = -3,
        LIQUIDITY_POOL_DEPOSIT_UNDERFUNDED = -4,
        LIQUIDITY_POOL_DEPOSIT_LINE_FULL = -5,
        LIQUIDITY_POOL_DEPOSIT_BAD_PRICE = -6,
        LIQUIDITY_POOL_DEPOSIT_POOL_FULL = -7,
        LIQUIDITY_POOL_DEPOSIT_TRUSTLINE_FROZEN = -8
    };
    XDR_RESULT_CODE_SERIALIZER(LiquidityPoolDepositResultCode,
                               LIQUIDITY_POOL_DEPOSIT_TRUSTLINE_FROZEN, LIQUIDITY_POOL_DEPOSIT_SUCCESS)

    struct alignas(4) LiquidityPoolDepositResult
    {
        LiquidityPoolDepositResultCode code;
    };
    XDR_CODE_STRUCT_SERIALIZER(LiquidityPoolDepositResult)

    enum class LiquidityPoolWithdrawResultCode : qint32
    {
        LIQUIDITY_POOL_WITHDRAW_SUCCESS = 0,
        LIQUIDITY_POOL_WITHDRAW_MALFORMED = -1,
        LIQUIDITY_POOL_WITHDRAW_NO_TRUST = -2,
        LIQUIDITY_POOL_WITHDRAW_UNDERFUNDED = -3,
        LIQUIDITY_POOL_WITHDRAW_LINE_FULL = -4,
        LIQUIDITY_POOL_WITHDRAW_UNDER_MINIMUM = -5,
        LIQUIDITY_POOL_WITHDRAW_TRUSTLINE_FROZEN = -6
    };
    XDR_RESULT_CODE_SERIALIZER(LiquidityPoolWithdrawResultCode,
                               LIQUIDITY_POOL_WITHDRAW_TRUSTLINE_FROZEN, LIQUIDITY_POOL_WITHDRAW_SUCCESS)

    struct alignas(4) LiquidityPoolWithdrawResult
    {
        LiquidityPoolWithdrawResultCode code;
    };
    XDR_CODE_STRUCT_SERIALIZER(LiquidityPoolWithdrawResult)


    /* High level Operation Result */

    enum class OperationResultCode : qint32
    {
         opINNER = 0, // inner object result is valid
         opBAD_AUTH = -1,  // too few valid signatures / wrong network
         opNO_ACCOUNT = -2, // source account was not found
         opNOT_SUPPORTED = -3, // operation not supported at this time
         opTOO_MANY_SUBENTRIES = -4, // max number of subentries already reached
         opEXCEEDED_WORK_LIMIT = -5, // operation did too much work
         opTOO_MANY_SPONSORING = -6  // account is sponsoring too many entries
    };
    XDR_RESULT_CODE_SERIALIZER(OperationResultCode, opTOO_MANY_SPONSORING, opINNER)


    /******* Soroban operation results ********/

    enum class InvokeHostFunctionResultCode : qint32
    {
        INVOKE_HOST_FUNCTION_SUCCESS = 0,
        INVOKE_HOST_FUNCTION_MALFORMED = -1,
        INVOKE_HOST_FUNCTION_TRAPPED = -2,
        INVOKE_HOST_FUNCTION_RESOURCE_LIMIT_EXCEEDED = -3,
        INVOKE_HOST_FUNCTION_ENTRY_ARCHIVED = -4,
        INVOKE_HOST_FUNCTION_INSUFFICIENT_REFUNDABLE_FEE = -5
    };
    XDR_RESULT_CODE_SERIALIZER(InvokeHostFunctionResultCode,
                               INVOKE_HOST_FUNCTION_INSUFFICIENT_REFUNDABLE_FEE, INVOKE_HOST_FUNCTION_SUCCESS)

    /* success: the hash of the InvokeHostFunctionSuccessPreImage; void otherwise. */
    struct InvokeHostFunctionResult
    {
        InvokeHostFunctionResultCode code;
        Hash success;
    };
    inline QDataStream &operator<<(QDataStream &out, const InvokeHostFunctionResult &obj) {
        out << obj.code;
        if (obj.code == InvokeHostFunctionResultCode::INVOKE_HOST_FUNCTION_SUCCESS)
            out << obj.success;
        return out;
    }
    inline QDataStream &operator>>(QDataStream &in, InvokeHostFunctionResult &obj) {
        in >> obj.code;
        if (obj.code == InvokeHostFunctionResultCode::INVOKE_HOST_FUNCTION_SUCCESS)
            in >> obj.success;
        return in;
    }

    enum class ExtendFootprintTTLResultCode : qint32
    {
        EXTEND_FOOTPRINT_TTL_SUCCESS = 0,
        EXTEND_FOOTPRINT_TTL_MALFORMED = -1,
        EXTEND_FOOTPRINT_TTL_RESOURCE_LIMIT_EXCEEDED = -2,
        EXTEND_FOOTPRINT_TTL_INSUFFICIENT_REFUNDABLE_FEE = -3
    };
    XDR_RESULT_CODE_SERIALIZER(ExtendFootprintTTLResultCode,
                               EXTEND_FOOTPRINT_TTL_INSUFFICIENT_REFUNDABLE_FEE, EXTEND_FOOTPRINT_TTL_SUCCESS)
    struct alignas(4) ExtendFootprintTTLResult
    {
        ExtendFootprintTTLResultCode code;
    };
    XDR_CODE_STRUCT_SERIALIZER(ExtendFootprintTTLResult)

    enum class RestoreFootprintResultCode : qint32
    {
        RESTORE_FOOTPRINT_SUCCESS = 0,
        RESTORE_FOOTPRINT_MALFORMED = -1,
        RESTORE_FOOTPRINT_RESOURCE_LIMIT_EXCEEDED = -2,
        RESTORE_FOOTPRINT_INSUFFICIENT_REFUNDABLE_FEE = -3
    };
    XDR_RESULT_CODE_SERIALIZER(RestoreFootprintResultCode,
                               RESTORE_FOOTPRINT_INSUFFICIENT_REFUNDABLE_FEE, RESTORE_FOOTPRINT_SUCCESS)
    struct alignas(4) RestoreFootprintResult
    {
        RestoreFootprintResultCode code;
    };
    XDR_CODE_STRUCT_SERIALIZER(RestoreFootprintResult)

    struct OperationResult
    {
        OperationResultCode code;
        OperationType type;
        union{
        CreateAccountResult createAccountResult;
        SetOptionsResult setOptionsResult;
        ChangeTrustResult changeTrustResult;
        AllowTrustResult allowTrustResult;
        AccountMergeResult accountMergeResult;
        ManageDataResult manageDataResult;
        BumpSequenceResult bumpSequenceResult;
        CreateClaimableBalanceResult createClaimableBalanceResult;
        ClawbackResult clawbackResult;
        ClawbackClaimableBalanceResult clawbackClaimableBalanceResult;
        SetTrustLineFlagsResult setTrustLineFlagsResult;
        ClaimClaimableBalanceResult claimClaimableBalanceResult;
        BeginSponsoringFutureReservesResult beginSponsoringFutureReservesResult;
        EndSponsoringFutureReservesResult endSponsoringFutureReservesResult;
        RevokeSponsorshipResult revokeSponsorshipResult;
        LiquidityPoolDepositResult liquidityPoolDepositResult;
        LiquidityPoolWithdrawResult liquidityPoolWithdrawResult;
        InvokeHostFunctionResult invokeHostFunctionResult;
        ExtendFootprintTTLResult extendFootprintTTLResult;
        RestoreFootprintResult restoreFootprintResult;


        //no trivial
        ManageBuyOfferResult manageBuyOfferResult;
        PaymentResult paymentResult;
        PathPaymentStrictReceiveResult pathPaymentStrictReceiveResult;
        PathPaymentStrictSendResult pathPaymentStrictSendResult;

        ManageSellOfferResult manageSellOfferResult;
        ManageSellOfferResult createPassiveOfferResult;
        InflationResult inflationResult;
        };
        //no trivial
        /**
         * @brief Operation
         * builds an Operation, it will not construct any of the non trivials members of the union
         * as it is used internally, to fill any of the non trivials you have to call the according placement constructor
         */
        OperationResult();

        OperationResult(const stellar::OperationResult& op);
        ~OperationResult();
        const OperationResult& operator = (const OperationResult& op);
    private:
        friend inline QDataStream &operator>>(QDataStream &in,  OperationResult &obj);
        // Destroys the live arm and falls back to the trivial CREATE_ACCOUNT.
        void clear();
    };

    inline QDataStream &operator<<(QDataStream &out, const  OperationResult &obj) {
        // Refused before anything is written, as Operation's writer does: with
        // opINNER and a type that has no arm, the code and the type went out
        // first and only then was the write marked failed.
        if (!isKnownResultCode(obj.code)
            || (obj.code == OperationResultCode::opINNER
                && (static_cast<qint32>(obj.type) < static_cast<qint32>(OperationType::CREATE_ACCOUNT)
                    || static_cast<qint32>(obj.type) > static_cast<qint32>(OperationType::RESTORE_FOOTPRINT)))) {
            out.setStatus(QDataStream::WriteFailed);
            return out;
        }
        out << obj.code;
        switch(obj.code){
        case OperationResultCode::opINNER:
        {
            out << obj.type;
            switch (obj.type)
            {
            case OperationType::CREATE_ACCOUNT:
                out << obj.createAccountResult; break;
            case OperationType::PAYMENT:
                out << obj.paymentResult; break;
            case OperationType::PATH_PAYMENT_STRICT_RECEIVE:
                out << obj.pathPaymentStrictReceiveResult; break;
            case OperationType::MANAGE_SELL_OFFER:                
                out << obj.manageSellOfferResult; break;
            case OperationType::CREATE_PASSIVE_SELL_OFFER:
                out << obj.createPassiveOfferResult; break;
            case OperationType::SET_OPTIONS:
                out << obj.setOptionsResult; break;
            case OperationType::CHANGE_TRUST:
                out << obj.changeTrustResult; break;
            case OperationType::ALLOW_TRUST:
                out << obj.allowTrustResult; break;
            case OperationType::ACCOUNT_MERGE:
                out << obj.accountMergeResult; break;
            case OperationType::INFLATION:
                out << obj.inflationResult; break;
            case OperationType::MANAGE_DATA:
                out << obj.manageDataResult; break;
            case OperationType::BUMP_SEQUENCE:
                out << obj.bumpSequenceResult; break;
            case OperationType::MANAGE_BUY_OFFER:
                out << obj.manageBuyOfferResult; break;
            case OperationType::PATH_PAYMENT_STRICT_SEND:
                out << obj.pathPaymentStrictSendResult; break;
            case OperationType::CREATE_CLAIMABLE_BALANCE:
                out << obj.createClaimableBalanceResult; break;
            case OperationType::CLAWBACK:
                out << obj.clawbackResult; break;
            case OperationType::CLAWBACK_CLAIMABLE_BALANCE:
                out << obj.clawbackClaimableBalanceResult; break;
            case OperationType::SET_TRUST_LINE_FLAGS:
                out << obj.setTrustLineFlagsResult; break;
            case OperationType::CLAIM_CLAIMABLE_BALANCE:
                out << obj.claimClaimableBalanceResult; break;
            case OperationType::BEGIN_SPONSORING_FUTURE_RESERVES:
                out << obj.beginSponsoringFutureReservesResult; break;
            case OperationType::END_SPONSORING_FUTURE_RESERVES:
                out << obj.endSponsoringFutureReservesResult; break;
            case OperationType::REVOKE_SPONSORSHIP:
                out << obj.revokeSponsorshipResult; break;
            case OperationType::LIQUIDITY_POOL_DEPOSIT:
                out << obj.liquidityPoolDepositResult; break;
            case OperationType::LIQUIDITY_POOL_WITHDRAW:
                out << obj.liquidityPoolWithdrawResult; break;
            case OperationType::INVOKE_HOST_FUNCTION:
                out << obj.invokeHostFunctionResult; break;
            case OperationType::EXTEND_FOOTPRINT_TTL:
                out << obj.extendFootprintTTLResult; break;
            case OperationType::RESTORE_FOOTPRINT:
                out << obj.restoreFootprintResult; break;
            default:
                out.setStatus(QDataStream::WriteFailed); break;
            }
            break;
        }
        default:break;
        }

       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  OperationResult &obj) {
        obj.clear();            // the arms below are built fresh: never over a live one
        in >> obj.code;
        switch(obj.code){
        case OperationResultCode::opINNER:
        {
            in >> obj.type;
            switch (obj.type)
            {
            case OperationType::CREATE_ACCOUNT:
                in >> obj.createAccountResult; break;
            case OperationType::PAYMENT:
                new (&obj.paymentResult) PaymentResult();
                in >> obj.paymentResult; break;
            case OperationType::PATH_PAYMENT_STRICT_RECEIVE:
                new (&obj.pathPaymentStrictReceiveResult) PathPaymentStrictReceiveResult();
                in >> obj.pathPaymentStrictReceiveResult; break;
            case OperationType::MANAGE_SELL_OFFER:
                new (&obj.manageSellOfferResult) ManageSellOfferResult();
                in >> obj.manageSellOfferResult; break;
            case OperationType::CREATE_PASSIVE_SELL_OFFER:
                new (&obj.createPassiveOfferResult) ManageSellOfferResult();
                in >> obj.createPassiveOfferResult; break;
            case OperationType::SET_OPTIONS:
                in >> obj.setOptionsResult; break;
            case OperationType::CHANGE_TRUST:
                in >> obj.changeTrustResult; break;
            case OperationType::ALLOW_TRUST:
                in >> obj.allowTrustResult; break;
            case OperationType::ACCOUNT_MERGE:
                in >> obj.accountMergeResult; break;
            case OperationType::INFLATION:
                new (&obj.inflationResult) InflationResult();
                in >> obj.inflationResult; break;
            case OperationType::MANAGE_DATA:
                in >> obj.manageDataResult; break;
            case OperationType::BUMP_SEQUENCE:
                in >> obj.bumpSequenceResult; break;
            case OperationType::MANAGE_BUY_OFFER:
                new (&obj.manageBuyOfferResult) ManageBuyOfferResult();
                in >> obj.manageBuyOfferResult; break;
            case OperationType::PATH_PAYMENT_STRICT_SEND:
                new (&obj.pathPaymentStrictSendResult) PathPaymentStrictSendResult();
                in >> obj.pathPaymentStrictSendResult; break;
            case OperationType::CREATE_CLAIMABLE_BALANCE:
                in >> obj.createClaimableBalanceResult; break;
            case OperationType::CLAWBACK:
                in >> obj.clawbackResult; break;
            case OperationType::CLAWBACK_CLAIMABLE_BALANCE:
                in >> obj.clawbackClaimableBalanceResult; break;
            case OperationType::SET_TRUST_LINE_FLAGS:
                in >> obj.setTrustLineFlagsResult; break;
            case OperationType::CLAIM_CLAIMABLE_BALANCE:
                in >> obj.claimClaimableBalanceResult; break;
            case OperationType::BEGIN_SPONSORING_FUTURE_RESERVES:
                in >> obj.beginSponsoringFutureReservesResult; break;
            case OperationType::END_SPONSORING_FUTURE_RESERVES:
                in >> obj.endSponsoringFutureReservesResult; break;
            case OperationType::REVOKE_SPONSORSHIP:
                in >> obj.revokeSponsorshipResult; break;
            case OperationType::LIQUIDITY_POOL_DEPOSIT:
                in >> obj.liquidityPoolDepositResult; break;
            case OperationType::LIQUIDITY_POOL_WITHDRAW:
                in >> obj.liquidityPoolWithdrawResult; break;
            // The three Soroban results were missing: their payload stayed in
            // the stream and was read as whatever came next.
            case OperationType::INVOKE_HOST_FUNCTION:
                in >> obj.invokeHostFunctionResult; break;
            case OperationType::EXTEND_FOOTPRINT_TTL:
                in >> obj.extendFootprintTTLResult; break;
            case OperationType::RESTORE_FOOTPRINT:
                in >> obj.restoreFootprintResult; break;
            default:
                in.setStatus(QDataStream::ReadCorruptData); break;
            }
            break;
        }
        default:break;
        }
        return in;
    }

    enum class TransactionResultCode : qint32
    {
        txFEE_BUMP_INNER_SUCCESS = 1, // fee bump inner transaction succeeded
        txSUCCESS = 0, // all operations succeeded

        txFAILED = -1, // one of the operations failed (none were applied)

        txTOO_EARLY = -2,         // ledger closeTime before minTime
        txTOO_LATE = -3,          // ledger closeTime after maxTime
        txMISSING_OPERATION = -4, // no operation was specified
        txBAD_SEQ = -5,           // sequence number does not match source account

        txBAD_AUTH = -6,             // too few valid signatures / wrong network
        txINSUFFICIENT_BALANCE = -7, // fee would bring account below reserve
        txNO_ACCOUNT = -8,           // source account not found
        txINSUFFICIENT_FEE = -9,     // fee is too small
        txBAD_AUTH_EXTRA = -10,      // unused signatures attached to transaction
        txINTERNAL_ERROR = -11,       // an unknown error occured
        txNOT_SUPPORTED = -12,        // transaction type not supported
        txFEE_BUMP_INNER_FAILED = -13, // fee bump inner transaction failed
        txBAD_SPONSORSHIP = -14,       // sponsorship not confirmed
        txBAD_MIN_SEQ_AGE_OR_GAP = -15, // minSeqAge or minSeqLedgerGap conditions not met
        txMALFORMED = -16,             // precondition is invalid
        txSOROBAN_INVALID = -17,       // soroban-specific preconditions were not met
        txFROZEN_KEY_ACCESSED = -18    // a frozen ledger key was accessed
    };
    XDR_RESULT_CODE_SERIALIZER(TransactionResultCode, txFROZEN_KEY_ACCESSED, txFEE_BUMP_INNER_SUCCESS)

    /** InnerTransactionResult's union declares every code but the two fee-bump
     *  ones: an inner transaction is never itself a fee bump. */
    inline bool isInnerTransactionResultCode(TransactionResultCode code)
    {
        return isKnownResultCode(code)
            && code != TransactionResultCode::txFEE_BUMP_INNER_SUCCESS
            && code != TransactionResultCode::txFEE_BUMP_INNER_FAILED;
    }

    // InnerTransactionResult must be binary compatible with TransactionResult
    // because it is be used to represent the result of a Transaction.
    struct InnerTransactionResult
    {
        // Always 0. Here for binary compatibility.
        qint64 feeCharged;
        TransactionResultCode code;

        QVector<OperationResult> results;
        // reserved for future use: the ext union's discriminant, and 0 is its
        // only arm. Initialised because the writer puts it on the wire — left
        // alone it was four bytes of whatever the stack held.
        qint32 v = 0;
    };

    /*
     * Two things were wrong here and both were invisible.
     *
     * The reader said `in << obj.results` — an OUTPUT operator inside an input
     * one. It compiles, because writing to a QDataStream is a perfectly good
     * thing to do; it just writes to a read-only buffer, sets the stream status
     * to WriteFailed, leaves the operation results empty and kills everything
     * after it in the stream. A fee-bump result therefore decoded as a
     * transaction whose inner transaction did nothing.
     *
     * And neither side touched `ext`, which the XDR declares as a four-byte
     * union discriminant. Leaving it out made every inner result four bytes
     * short of canonical — symmetric again, so nothing local noticed, and
     * wrong against Core.
     */
    inline QDataStream &operator<<(QDataStream &out, const  InnerTransactionResult &obj) {
        if (!isInnerTransactionResultCode(obj.code) || obj.v != 0) {   // no arm: nothing to write
            out.setStatus(QDataStream::WriteFailed);
            return out;
        }
        out << obj.feeCharged << obj.code;
        switch(obj.code){
        case TransactionResultCode::txSUCCESS:
        case TransactionResultCode::txFAILED:
            writeXdrList(out, obj.results); break;
        default: break;
        }
        out << obj.v;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  InnerTransactionResult &obj) {
        in >> obj.feeCharged >> obj.code;
        obj.results.clear();          // decoding must replace, never merge
        if (!isInnerTransactionResultCode(obj.code)) {
            in.setStatus(QDataStream::ReadCorruptData);
            return in;
        }
        switch(obj.code){
        case TransactionResultCode::txSUCCESS:
        case TransactionResultCode::txFAILED:
            readXdrList(in, obj.results); break;
        default: break;
        }
        in >> obj.v;
        // Only v = 0 exists; anything else is not a result this can read.
        if (obj.v != 0)
            in.setStatus(QDataStream::ReadCorruptData);
       return in;
    }

    struct InnerTransactionResultPair
    {
        Hash transactionHash;          // hash of the inner transaction
        InnerTransactionResult result; // result for the inner transaction
    };

    inline QDataStream &operator<<(QDataStream &out, const  InnerTransactionResultPair &obj) {
        out << obj.transactionHash << obj.result;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  InnerTransactionResultPair &obj) {
        in >> obj.transactionHash >> obj.result;
       return in;
    }

    struct TransactionResult
    {
        qint64 feeCharged; // actual fee charged for the transaction
        TransactionResultCode code;

        InnerTransactionResultPair innerResultPair;
        QVector<OperationResult> results;

        // reserved for future use
        Reserved ext;


    };
    inline QDataStream &operator<<(QDataStream &out, const  TransactionResult &obj) {
        out << obj.feeCharged << obj.code;
        switch(obj.code){
        case TransactionResultCode::txFEE_BUMP_INNER_SUCCESS:
        case TransactionResultCode::txFEE_BUMP_INNER_FAILED:
               out << obj.innerResultPair;break;
        case TransactionResultCode::txSUCCESS:
        case TransactionResultCode::txFAILED:
            writeXdrList(out, obj.results); break;
        default: break;
        }
        out << obj.ext;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  TransactionResult &obj) {
        // Decoding must replace, never merge: a txBAD_SEQ read into an object
        // that held 48 operation results used to keep all 48.
        obj.results.clear();
        obj.innerResultPair = InnerTransactionResultPair();
        in >> obj.feeCharged>> obj.code;
        switch(obj.code){
        case TransactionResultCode::txFEE_BUMP_INNER_SUCCESS:
        case TransactionResultCode::txFEE_BUMP_INNER_FAILED:
               in >> obj.innerResultPair;break;
        case TransactionResultCode::txSUCCESS:
        case TransactionResultCode::txFAILED:
            readXdrList(in, obj.results); break;
        default: break;
        }
        in >> obj.ext;
        if (obj.ext.reserved != 0) in.setStatus(QDataStream::ReadCorruptData);   // ext is void: 0 only
       return in;
    }



}
#endif // STELLARTRANSACTION_H
