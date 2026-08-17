#ifndef ASSEMBLEDTRANSACTIONEXCEPTION_H
#define ASSEMBLEDTRANSACTIONEXCEPTION_H

#include <string>
#include <QString>
#include "sdkexception.h"

namespace qstellar { namespace exception {

/**
 * Base class for failures raised while building, simulating, signing or
 * sending an `AssembledTransaction` (the Soroban high-level flow).
 *
 * Derives from `SdkException` (→ `std::runtime_error`) so existing
 * `catch (std::runtime_error&)` / `catch (std::exception&)` clauses keep
 * working unchanged — typing only ADDS the ability to react to a specific
 * failure (simulate failed vs. not-yet-simulated vs. send failed).
 */
class AssembledTransactionException : public SdkException
{
public:
    explicit AssembledTransactionException(const char* message) : SdkException(message) {}
    explicit AssembledTransactionException(const std::string& message) : SdkException(message) {}
    explicit AssembledTransactionException(const QString& message) : SdkException(message) {}
};

}} // namespace qstellar::exception

#endif // ASSEMBLEDTRANSACTIONEXCEPTION_H
