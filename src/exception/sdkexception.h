#ifndef QSTELLAR_EXCEPTION_SDKEXCEPTION_H
#define QSTELLAR_EXCEPTION_SDKEXCEPTION_H

#include <stdexcept>
#include <string>
#include <QString>

namespace qstellar {
namespace exception {

/**
 * @brief Base class for all exceptions thrown by StellarQtSDK.
 *
 * Inherits from std::runtime_error so existing `catch (std::runtime_error&)`
 * clauses keep working during the 0.4.0 transition.
 */
class SdkException : public std::runtime_error
{
public:
    SdkException() : std::runtime_error("") {}
    // Disambiguator for `""` literals: without this overload a C-string
    // literal would be ambiguous between std::string and QString.
    explicit SdkException(const char* message) : std::runtime_error(message ? message : "") {}
    explicit SdkException(const std::string& message) : std::runtime_error(message) {}
    explicit SdkException(const QString& message) : std::runtime_error(message.toStdString()) {}
};

} // namespace exception
} // namespace qstellar

#endif // QSTELLAR_EXCEPTION_SDKEXCEPTION_H
