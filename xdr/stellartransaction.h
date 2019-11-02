#ifndef STELLARTRANSACTION_H
#define STELLARTRANSACTION_H

#include "stellarledgerentries.h"
#include "xdrhelper.h"
namespace stellar
{

    using namespace xdr;
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
        PATH_PAYMENT_STRICT_SEND = 13
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
        AccountID destination; // recipient of the payment
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

        AccountID destination; // recipient of the payment
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

        AccountID destination; // recipient of the payment
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
    struct ChangeTrustOp
    {
        Asset line;

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
        Asset asset;
        qint32 authorize;//bool, 4 bytes
    };
    // ASSET_TYPE_NATIVE is not allowed
    inline QDataStream &operator<<(QDataStream &out, const  AllowTrustOp &obj) {
        //allow trust op skips issuer field
        out << obj.trustor << obj.asset.type;
        switch(obj.asset.type)
        {
            case stellar::AssetType::ASSET_TYPE_CREDIT_ALPHANUM4:
                out << obj.asset.alphaNum4.assetCode;break;
            case stellar::AssetType::ASSET_TYPE_CREDIT_ALPHANUM12:
                out << obj.asset.alphaNum12.assetCode;break;
            default:
            break;
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
           default:
           break;
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


    /* An operation is the lowest unit of work that a transaction does */
    struct Operation
    {
        // sourceAccount is the account used to run the operation
        // if not set, the runtime defaults to "sourceAccount" specified at
        // the transaction level
        Optional<AccountID> sourceAccount;
        OperationType type;
        union{
        CreateAccountOp operationCreateAccount;
        PaymentOp operationPayment;
        ManageSellOfferOp operationManageSellOffer;
        CreatePassiveSellOfferOp operationCreatePassiveSellOffer;
        ChangeTrustOp operationChangeTrust;
        AllowTrustOp operationAllowTrust;
        AccountID operationAccountMerge;
        AccountID operationInflation;
        BumpSequenceOp operationBumpSequence;
        ManageBuyOfferOp operationManageBuyOffer;

        //non trivials, you MUST call contructor explicity to use them
        PathPaymentStrictReceiveOp operationPathPaymentStrictReceive;
        SetOptionsOp operationSetOptions;
        ManageDataOp operationManageData;
        PathPaymentStrictSendOp  operationPathPaymentStrictSend;
        };

        /**
         * @brief Operation
         * builds an Operation, it will not construct any of the non trivials members of the union
         * as it is used internally, to fill any of the non trivials you have to call the according placement constructor
         */
        Operation();

        Operation(const stellar::Operation& op);
        ~Operation();
        const Operation& operator = (const Operation& op);

    };
    inline QDataStream &operator<<(QDataStream &out, const  Operation &obj) {
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
            out << obj.operationInflation; break;
        case OperationType::MANAGE_DATA:
            out << obj.operationManageData; break;
        case OperationType::BUMP_SEQUENCE:
            out << obj.operationBumpSequence; break;
        case OperationType::MANAGE_BUY_OFFER:
            out << obj.operationManageBuyOffer; break;
        case OperationType::PATH_PAYMENT_STRICT_SEND:
            out << obj.operationPathPaymentStrictSend; break;
        //default: break;
        }

       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  Operation &obj) {
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
            in >> obj.operationChangeTrust; break;
        case OperationType::ALLOW_TRUST:
            in >> obj.operationAllowTrust; break;
        case OperationType::ACCOUNT_MERGE:
            in >> obj.operationAccountMerge; break;
        case OperationType::INFLATION:
            in >> obj.operationInflation; break;
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
        //default: break;
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
        out << obj.type;;
        switch(obj.type){
        case MemoType::MEMO_NONE:
            break;
        case MemoType::MEMO_TEXT:
            out << obj.text; break;
        case MemoType::MEMO_ID:
            out << obj.id; break;
        case MemoType::MEMO_HASH:
            out << obj.hash; break;
        case MemoType::MEMO_RETURN:
            out << obj.retHash; break;
        //default: break;
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
        //default: break;
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
    // maximum number of operations per transaction
    const int MAX_OPS_PER_TX = 100;

    /* a transaction is a container for a set of operations
        - is executed by an account
        - fees are collected from the account
        - operations are executed in order as one ACID transaction
              either all operations are applied or none are
              if any returns a failing code
    */

    struct Transaction
    {
        // account used to run the transaction
        AccountID sourceAccount;

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
    inline QDataStream &operator<<(QDataStream &out, const  Transaction &obj) {
        out << obj.sourceAccount << obj.fee << obj.seqNum << obj.timeBounds << obj.memo<< obj.operations<< obj.ext;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  Transaction &obj) {
       in >> obj.sourceAccount >> obj.fee >> obj.seqNum >> obj.timeBounds >> obj.memo>> obj.operations >> obj.ext;
       return in;
    }

    struct TransactionSignaturePayload {
        Hash networkId;
        EnvelopeType type;
         /* All other values of type are invalid */
        TransactionSignaturePayload():type(EnvelopeType::ENVELOPE_TYPE_TX)
        {
        }
        Transaction tx;
    };
    inline QDataStream &operator<<(QDataStream &out, const  TransactionSignaturePayload &obj) {
        out << obj.networkId << obj.type <<obj.tx;
       return out;
    }

   inline  QDataStream &operator>>(QDataStream &in,  TransactionSignaturePayload &obj) {
       in >> obj.networkId >> obj.type >> obj.tx;
       return in;
    }

    /* A TransactionEnvelope wraps a transaction with signatures. */
    struct TransactionEnvelope
    {
        Transaction tx;
        /* Each decorated signature is a signature over the SHA256 hash of
         * a TransactionSignaturePayload */
        Array<DecoratedSignature,20> signatures; //max <20>;
    };
    inline QDataStream &operator<<(QDataStream &out, const  TransactionEnvelope &obj) {
        out << obj.tx << obj.signatures;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  TransactionEnvelope &obj) {
       in >> obj.tx >> obj.signatures;
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
    struct alignas(4) CreateAccountResult
    {
        CreateAccountResultCode code;
    };
    XDR_SERIALIZER(CreateAccountResult)

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
    XDR_SERIALIZER(PaymentResultCode)

    struct PaymentResult {
        PaymentResultCode code;
        PaymentResult():code(PaymentResultCode::PAYMENT_SUCCESS){

        }
    };
    XDR_SERIALIZER(PaymentResult)

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
    XDR_SERIALIZER(PathPaymentStrictReceiveResultCode)

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
        PATH_PAYMENT_STRICT_SEND_OVER_SENDMAX = -12       // could not satisfy destMin
    };
    XDR_SERIALIZER(PathPaymentStrictSendResultCode)

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
        QVector<ClaimOfferAtom> offers; //<>;
        union{
            SimplePaymentResult last;
            Asset noIssuer; // the asset that caused the error
        };
    };
    inline QDataStream &operator<<(QDataStream &out, const  PathPaymentStrictReceiveResult &obj) {
        out << obj.code;
        switch(obj.code){
        case PathPaymentStrictReceiveResultCode::PATH_PAYMENT_STRICT_RECEIVE_SUCCESS:
            out << obj.offers << obj.last; break;
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
            in >> obj.offers >> obj.last; break;
        case PathPaymentStrictReceiveResultCode::PATH_PAYMENT_STRICT_RECEIVE_NO_ISSUER:
            in >> obj.noIssuer; break;
        default: break;
        }
       return in;
    }

    struct PathPaymentStrictSendResult {
        PathPaymentStrictSendResultCode code;
        QVector<ClaimOfferAtom> offers; //<>;
        union{
            SimplePaymentResult last;
            Asset noIssuer; // the asset that caused the error
        };
    };
    inline QDataStream &operator<<(QDataStream &out, const  PathPaymentStrictSendResult &obj) {
        out << obj.code;
        switch(obj.code){
        case PathPaymentStrictSendResultCode::PATH_PAYMENT_STRICT_SEND_SUCCESS:
            out << obj.offers << obj.last; break;
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
            in >> obj.offers >> obj.last; break;
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
    XDR_SERIALIZER(ManageSellOfferResultCode)


    enum class ManageOfferEffect : qint32
    {
        MANAGE_OFFER_CREATED = 0,
        MANAGE_OFFER_UPDATED = 1,
        MANAGE_OFFER_DELETED = 2
    };
    XDR_SERIALIZER(ManageOfferEffect)


    struct ManageOfferSuccessResult
    {
        // offers that got claimed while creating this offer
        QVector<ClaimOfferAtom> offersClaimed;
        ManageOfferEffect effect;
        OfferEntry offer;
    };
    inline QDataStream &operator<<(QDataStream &out, const  ManageOfferSuccessResult &obj) {
        out << obj.offersClaimed << obj.effect;
        switch(obj.effect){
        case ManageOfferEffect::MANAGE_OFFER_CREATED:
        case ManageOfferEffect::MANAGE_OFFER_UPDATED:
            out << obj.offer; break;
        default: break;
        }
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  ManageOfferSuccessResult &obj) {
        in >> obj.offersClaimed >> obj.effect;
        switch(obj.effect){
        case ManageOfferEffect::MANAGE_OFFER_CREATED:
        case ManageOfferEffect::MANAGE_OFFER_UPDATED:
            in >> obj.offer; break;
        default: break;
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
        SET_OPTIONS_INVALID_HOME_DOMAIN = -9     // malformed home domain
    };
    XDR_SERIALIZER(SetOptionsResultCode)

    struct alignas(4) SetOptionsResult
    {
        SetOptionsResultCode code;
    };
    XDR_SERIALIZER(SetOptionsResult)

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
        CHANGE_TRUST_SELF_NOT_ALLOWED = -5 // trusting self is not allowed
    };
    XDR_SERIALIZER(ChangeTrustResultCode)

    struct alignas(4) ChangeTrustResult
    {
        ChangeTrustResultCode code;
    };
    XDR_SERIALIZER(ChangeTrustResult)

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
        ALLOW_TRUST_SELF_NOT_ALLOWED = -5 // trusting self is not allowed
    };
    XDR_SERIALIZER(AllowTrustResultCode)

    struct alignas(4) AllowTrustResult
    {
        AllowTrustResultCode code;
    };
    XDR_SERIALIZER(AllowTrustResult)

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
        ACCOUNT_MERGE_DEST_FULL = -6 // can't add source balance to destination balance
    };
    XDR_SERIALIZER(AccountMergeResultCode)

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
    XDR_SERIALIZER(InflationResultCode)

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
            out << obj.payouts; break;
        default: break;
        }

       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  InflationResult &obj) {
        in >> obj.code;
        switch(obj.code){
        case InflationResultCode::INFLATION_SUCCESS:
            in >> obj.payouts; break;
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
    XDR_SERIALIZER(ManageDataResultCode)

    struct alignas(4) ManageDataResult
    {
        ManageDataResultCode code;
    };
    XDR_SERIALIZER(ManageDataResult)


    enum class BumpSequenceResultCode : qint32
    {
        // codes considered as "success" for the operation
        BUMP_SEQUENCE_SUCCESS = 0,
        // codes considerer as "failure" for the operation
        BUMP_SEQUENCE_BAD_SEQ = -1  //  'bumpTo' is not within bounds
    };
    XDR_SERIALIZER(BumpSequenceResultCode)

    struct alignas(4) BumpSequenceResult
    {
         BumpSequenceResultCode code;
    };
    XDR_SERIALIZER(BumpSequenceResult)


    /* High level Operation Result */

    enum class OperationResultCode : qint32
    {
         opINNER = 0, // inner object result is valid
         opBAD_AUTH = -1,  // too few valid signatures / wrong network
         opNO_ACCOUNT = -2, // source account was not found
         opNOT_SUPPORTED = -3, // operation not supported at this time
         opTOO_MANY_SUBENTRIES = -4, // max number of subentries already reached
         opEXCEEDED_WORK_LIMIT = -5  // operation did too much work
    };
    XDR_SERIALIZER(OperationResultCode)


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

    };

    inline QDataStream &operator<<(QDataStream &out, const  OperationResult &obj) {
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
            //default:break;
            }
            break;
        }
        default:break;
        }

       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  OperationResult &obj) {
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
            //default: break;
            }
            break;
        }
        default:break;
        }
        return in;
    }

    enum class TransactionResultCode : qint32
    {
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
        txINTERNAL_ERROR = -11       // an unknown error occured
    };
    XDR_SERIALIZER(TransactionResultCode)


    struct TransactionResult
    {
        qint64 feeCharged; // actual fee charged for the transaction
        TransactionResultCode code;
        QVector<OperationResult> results;

        // reserved for future use
        Reserved ext;
    };
    inline QDataStream &operator<<(QDataStream &out, const  TransactionResult &obj) {
        out << obj.feeCharged << obj.code;
        switch(obj.code){
        case TransactionResultCode::txSUCCESS:
        case TransactionResultCode::txFAILED:
            out << obj.results; break;
        default: break;
        }
        out << obj.ext;
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  TransactionResult &obj) {
        in >> obj.feeCharged>> obj.code;
        switch(obj.code){
        case TransactionResultCode::txSUCCESS:
        case TransactionResultCode::txFAILED:
            in >> obj.results; break;
        default: break;
        }
        in >> obj.ext;
       return in;
    }



}
#endif // STELLARTRANSACTION_H
