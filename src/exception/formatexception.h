#ifndef QSTELLAR_EXCEPTION_FORMATEXCEPTION_H
#define QSTELLAR_EXCEPTION_FORMATEXCEPTION_H

#include "sdkexception.h"

namespace qstellar {
namespace exception {

/**
 * @brief Raised when decoding a strkey-encoded string fails.
 *
 * Replaces the legacy `::FormatException` (global namespace) — a typedef is
 * kept in strkey.h for source compatibility through the entire 0.x series.
 */
class FormatException : public SdkException
{
public:
    FormatException() : SdkException(QStringLiteral("Invalid strkey format")) {}
    explicit FormatException(const QString& message) : SdkException(message) {}
};

} // namespace exception
} // namespace qstellar

#endif // QSTELLAR_EXCEPTION_FORMATEXCEPTION_H
