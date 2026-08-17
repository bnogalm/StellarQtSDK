#ifndef SENDFAILEDEXCEPTION_H
#define SENDFAILEDEXCEPTION_H

#include <string>
#include <QString>
#include "assembledtransactionexception.h"

namespace qstellar { namespace exception {

/**
 * Thrown by `AssembledTransaction::signAndSend()` / `restoreFootprint()`
 * when `sendTransaction` is rejected by the Soroban RPC with status
 * ERROR (the transaction was not accepted into the mempool). The message
 * carries the error-result XDR returned by the RPC.
 */
class SendFailedException : public AssembledTransactionException
{
public:
    explicit SendFailedException(const char* message) : AssembledTransactionException(message) {}
    explicit SendFailedException(const std::string& message) : AssembledTransactionException(message) {}
    explicit SendFailedException(const QString& message) : AssembledTransactionException(message) {}
};

}} // namespace qstellar::exception

#endif // SENDFAILEDEXCEPTION_H
