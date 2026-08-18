#ifndef INFLATIONOPERATION_H
#define INFLATIONOPERATION_H
#include "operation.h"
#include "qstellar_namespace.h"

/**
 * Represents <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html#inflation" target="_blank">Inflation</a> operation.
 * @see <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html" target="_blank">List of Operations</a>
 */
QSTELLAR_BEGIN_NS

class InflationOperation : public Operation
{
public:
    InflationOperation();
    void fillOperationBody(AccountConverter& accountConverter, stellar::Operation &operation);
    static InflationOperation* create();
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(InflationOperation)

#endif // INFLATIONOPERATION_H
