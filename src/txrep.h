#ifndef TXREP_H
#define TXREP_H

#include <QString>

#include "network.h"

#include "qstellar_namespace.h"
QSTELLAR_FWD(Transaction)
QSTELLAR_BEGIN_NS


/**
 * SEP-11 — Txrep, the human-readable text representation of Stellar
 * transactions.
 *
 * This release ships a minimal subset focused on the SEP-10 challenge
 * shape (Memo::NONE, preconditions NONE / TIME, ManageData and Payment
 * operations). Other operation types throw an `unsupported operation
 * type` runtime_error so callers can detect the gap explicitly. Surface
 * is intentionally extensible — future releases will add operations
 * without changing the public API.
 *
 * The format follows the dotted-path notation of the SEP-11 spec:
 *   type: ENVELOPE_TYPE_TX
 *   tx.sourceAccount: GC...
 *   tx.fee: 100
 *   tx.operations.len: 1
 *   ...
 */
class Txrep
{
public:
    /** Renders a Transaction's V1 envelope as a SEP-11 Txrep string. */
    static QString toTxrep(Transaction* transaction);

    /**
     * Parses a SEP-11 Txrep string back into a Transaction. The caller
     * takes ownership of the returned pointer. The `network` parameter is
     * required because SEP-11 doesn't encode the network passphrase.
     */
    static Transaction* fromTxrep(const QString& txrep, Network* network);
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(Txrep)

#endif // TXREP_H
