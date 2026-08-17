#ifndef NOTYETSIMULATEDEXCEPTION_H
#define NOTYETSIMULATEDEXCEPTION_H

#include <string>
#include <QString>
#include "assembledtransactionexception.h"

namespace qstellar { namespace exception {

/**
 * Thrown when an `AssembledTransaction` operation that depends on a prior
 * successful `simulate()` — `result()`, `needsNonInvokerSigningBy()`,
 * `signAuthEntries()`, `restoreFootprint()` — is called before simulation
 * has run. Call `simulate()` first.
 */
class NotYetSimulatedException : public AssembledTransactionException
{
public:
    explicit NotYetSimulatedException(const char* message) : AssembledTransactionException(message) {}
    explicit NotYetSimulatedException(const std::string& message) : AssembledTransactionException(message) {}
    explicit NotYetSimulatedException(const QString& message) : AssembledTransactionException(message) {}
};

}} // namespace qstellar::exception

#endif // NOTYETSIMULATEDEXCEPTION_H
