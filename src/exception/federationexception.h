#ifndef QSTELLAR_EXCEPTION_FEDERATIONEXCEPTION_H
#define QSTELLAR_EXCEPTION_FEDERATIONEXCEPTION_H

#include "sdkexception.h"

namespace qstellar {
namespace exception {

/**
 * @brief SEP-2 — a lookup that failed at the domain's stellar.toml, or whose
 * answer was stopped for its size.
 *
 * The domain's stellar.toml could not be fetched or read, names no
 * FEDERATION_SERVER, or names one that is not https; or the federation
 * server's answer passed 100 KB and was stopped while it arrived. what()
 * says which. Response::lastException() of the FederationResponse carries
 * it. An answer that arrived whole and was refused (not JSON, an account_id
 * or memo that cannot be paid) is an UnknownResponseException with the
 * reason, as any other answer that cannot be read.
 */
class FederationException : public SdkException
{
public:
    using SdkException::SdkException;
};

} // namespace exception
} // namespace qstellar

#endif // QSTELLAR_EXCEPTION_FEDERATIONEXCEPTION_H
