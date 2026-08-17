#ifndef SIMULATIONFAILEDEXCEPTION_H
#define SIMULATIONFAILEDEXCEPTION_H

#include <string>
#include <QString>
#include "assembledtransactionexception.h"

namespace qstellar { namespace exception {

/**
 * Thrown by `AssembledTransaction::simulate()` when the Soroban RPC's
 * `simulateTransaction` returns an error (e.g. the contract call would
 * trap, or the footprint cannot be computed). The message carries the
 * RPC's error string.
 */
class SimulationFailedException : public AssembledTransactionException
{
public:
    explicit SimulationFailedException(const char* message) : AssembledTransactionException(message) {}
    explicit SimulationFailedException(const std::string& message) : AssembledTransactionException(message) {}
    explicit SimulationFailedException(const QString& message) : AssembledTransactionException(message) {}
};

}} // namespace qstellar::exception

#endif // SIMULATIONFAILEDEXCEPTION_H
