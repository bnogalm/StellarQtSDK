#ifndef LEDGERRESPONSE_H
#define LEDGERRESPONSE_H

#include <QObject>
#include "response.h"
#include "link.h"

namespace LedgerResponseAttach
{

/**
 * Links connected to ledger.
 */
class Links {
    Q_GADGET
    Q_PROPERTY(Link effects MEMBER m_effects)
    Q_PROPERTY(Link operations MEMBER m_operations)
    Q_PROPERTY(Link self MEMBER m_self)
    Q_PROPERTY(Link transactions MEMBER m_transactions)

    Link m_effects;
    Link m_operations;
    Link m_self;
    Link m_transactions;
public:
    Link& getEffects(){
        return m_effects;
    }

    Link& getOperations(){
        return m_operations;
    }

    Link& getSelf() {
        return m_self;
    }

    Link& getTransactions(){
        return m_transactions;
    }
    bool operator !=(Links& links);
    bool operator ==(Links& links);
};
}
/**
 * Represents ledger response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/ledger.html" target="_blank">Ledger documentation</a>
 * @see org.stellar.sdk.requests.LedgersRequestBuilder
 * @see org.stellar.sdk.Server#ledgers()
 */
class LedgerResponse : public Response
{
    Q_OBJECT
    Q_PROPERTY(qint64 sequence MEMBER m_sequence)
    Q_PROPERTY(QString hash MEMBER m_hash)
    Q_PROPERTY(QString paging_token MEMBER m_pagingToken)
    Q_PROPERTY(QString prev_hash MEMBER m_prevHash)
    Q_PROPERTY(qint32 transaction_count MEMBER m_transactionCount)
    Q_PROPERTY(qint32 successful_transaction_count MEMBER m_successfulTransactionCount)
    Q_PROPERTY(qint32 failed_transaction_count MEMBER m_failedTransactionCount)
    Q_PROPERTY(qint32 operation_count MEMBER m_operationCount)
    Q_PROPERTY(QString closed_at MEMBER m_closedAt)
    Q_PROPERTY(QString total_coins MEMBER m_totalCoins)
    Q_PROPERTY(QString fee_pool MEMBER m_feePool)
    Q_PROPERTY(qint64 base_fee MEMBER m_baseFee)
    Q_PROPERTY(QString base_reserve MEMBER m_baseReserve)
    //java SDK uses String, but json contains a number, not a string, that can cause problems, we return
    Q_PROPERTY(qint64 base_fee_in_stroops MEMBER m_baseFeeInStroops)
    Q_PROPERTY(qint64 base_reserve_in_stroops MEMBER m_baseReserveInStroops)
    Q_PROPERTY(qint32 max_tx_set_size MEMBER m_maxTxSetSize)
    Q_PROPERTY(qint32 protocol_version MEMBER m_protocolVersion)
    Q_PROPERTY(QString header_xdr MEMBER m_headerXdr)
    Q_PROPERTY(LedgerResponseAttach::Links _links MEMBER m_links)

    qint64 m_sequence;
    QString m_hash;
    QString m_pagingToken;
    QString m_prevHash;
    qint32 m_transactionCount;
    qint32 m_successfulTransactionCount;
    qint32 m_failedTransactionCount;
    qint32 m_operationCount;
    QString m_closedAt;
    QString m_totalCoins;
    QString m_feePool;
    qint64 m_baseFee;
    QString m_baseReserve;
    qint64 m_baseFeeInStroops;
    qint64 m_baseReserveInStroops;
    qint32 m_maxTxSetSize;    
    qint32 m_protocolVersion;
    QString m_headerXdr;
    LedgerResponseAttach::Links m_links;

public:
    Q_INVOKABLE explicit LedgerResponse(QNetworkReply *reply=nullptr);
    qint64 getSequence() const;
    QString getHash() const;
    QString getPagingToken() const;
    QString getPrevHash() const;
    /**
     * @deprecated Will be removed in Horizon 0.17.0
     */
    Q_DECL_DEPRECATED qint32 getTransactionCount() const;
    qint32 getSuccessfulTransactionCount() const;
    qint32 getFailedTransactionCount() const;
    qint32 getOperationCount() const;
    QString getClosedAt() const;
    QString getTotalCoins() const;
    QString getFeePool() const;
    qint64 getBaseFee() const;
    QString getBaseReserve() const;
    QString getBaseFeeInStroops() const;
    QString getBaseReserveInStroops() const;
    qint64 baseFeeInStroops() const;
    qint64 baseReserveInStroops() const;
    qint32 getMaxTxSetSize() const;
    qint32 getProtocolVersion() const;
    QString getHeaderXdr() const;
    LedgerResponseAttach::Links& getLinks();
};
Q_DECLARE_METATYPE(LedgerResponseAttach::Links)

#endif // LEDGERRESPONSE_H
