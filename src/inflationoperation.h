#ifndef INFLATIONOPERATION_H
#define INFLATIONOPERATION_H
#include "operation.h"

/**
 * Represents <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html#inflation" target="_blank">Inflation</a> operation.
 * @see <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html" target="_blank">List of Operations</a>
 */
class InflationOperation : public Operation
{
public:
    InflationOperation();
    void fillOperationBody(AccountConverter& accountConverter, stellar::Operation &operation);
    InflationOperation* create();
};

#endif // INFLATIONOPERATION_H
