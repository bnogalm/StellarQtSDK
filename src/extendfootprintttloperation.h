#ifndef EXTENDFOOTPRINTTTLOPERATION_H
#define EXTENDFOOTPRINTTTLOPERATION_H

#include "operation.h"

/**
 * CAP-46 — extends the TTL of the ledger entries listed in the transaction's
 * Soroban footprint (read-only) to at least `extendTo` ledgers in the future.
 */
class ExtendFootprintTTLOperation : public Operation
{
    quint32 m_extendTo;
public:
    explicit ExtendFootprintTTLOperation(quint32 extendTo);
    explicit ExtendFootprintTTLOperation(const stellar::ExtendFootprintTTLOp& op);
    ~ExtendFootprintTTLOperation() override = default;

    quint32 getExtendTo() const { return m_extendTo; }

    void fillOperationBody(AccountConverter& accountConverter, stellar::Operation& operation) override;

    static ExtendFootprintTTLOperation* build(const stellar::ExtendFootprintTTLOp& op);
    static ExtendFootprintTTLOperation* create(quint32 extendTo);

    ExtendFootprintTTLOperation* setSourceAccount(QString sourceAccount);
};

#endif // EXTENDFOOTPRINTTTLOPERATION_H
