#include "gettransactionsresponse.h"
#include "gethealthresponse.h"

#include <QJsonArray>

QSTELLAR_BEGIN_NS


GetTransactionsResponse GetTransactionsResponse::fromJson(const QJsonObject& r)
{
    // Each field as the RPC sends it, or the answer is refused. The page's
    // close times come as latestLedgerCloseTimestamp and
    // oldestLedgerCloseTimestamp (JSON integers): they used to be looked for
    // as latestLedgerCloseTime and oldestLedgerCloseTime, so every real answer
    // gave them empty. A missing or non-numeric ledger number read as 0.
    const sorobanrpcdetail::RpcFields fields(r, QStringLiteral("getTransactions"));
    GetTransactionsResponse out;
    const QString transactionsKey = QStringLiteral("transactions");
    const QJsonArray transactions = fields.array(transactionsKey);
    for (int i = 0; i < transactions.size(); ++i) {
        out.m_transactions.append(GetTransactionResponse::read(
            fields.element(transactionsKey, transactions, i), false));
    }
    out.m_latestLedger          = fields.uint32(QStringLiteral("latestLedger"));
    out.m_latestLedgerCloseTime = fields.int64Text(QStringLiteral("latestLedgerCloseTimestamp"));
    out.m_oldestLedger          = fields.uint32(QStringLiteral("oldestLedger"));
    out.m_oldestLedgerCloseTime = fields.int64Text(QStringLiteral("oldestLedgerCloseTimestamp"));
    out.m_cursor                = fields.string(QStringLiteral("cursor"));
    return out;
}
QSTELLAR_END_NS
