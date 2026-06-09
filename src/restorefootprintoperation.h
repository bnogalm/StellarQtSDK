#ifndef RESTOREFOOTPRINTOPERATION_H
#define RESTOREFOOTPRINTOPERATION_H

#include "operation.h"
#include "qstellar_namespace.h"

/**
 * CAP-46 — restores expired (archived) ledger entries listed in the
 * transaction's Soroban footprint (read-write). The operation body itself
 * carries no parameters beyond the standard ext field.
 */
QSTELLAR_BEGIN_NS

class RestoreFootprintOperation : public Operation
{
public:
    RestoreFootprintOperation() = default;
    explicit RestoreFootprintOperation(const stellar::RestoreFootprintOp& op);
    ~RestoreFootprintOperation() override = default;

    void fillOperationBody(AccountConverter& accountConverter, stellar::Operation& operation) override;

    static RestoreFootprintOperation* build(const stellar::RestoreFootprintOp& op);
    static RestoreFootprintOperation* create();

    RestoreFootprintOperation* setSourceAccount(QString sourceAccount);
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(RestoreFootprintOperation)

#endif // RESTOREFOOTPRINTOPERATION_H
