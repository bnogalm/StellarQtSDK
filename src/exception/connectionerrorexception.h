#ifndef QSTELLAR_EXCEPTION_CONNECTIONERROREXCEPTION_H
#define QSTELLAR_EXCEPTION_CONNECTIONERROREXCEPTION_H

#include "networkexception.h"

namespace qstellar {
namespace exception {

/** @brief Connection-level failure (DNS, refused, TLS handshake, …). */
class ConnectionErrorException : public NetworkException
{
public:
    explicit ConnectionErrorException(const QString& details = QString())
        : NetworkException(QStringLiteral("Connection error: %1").arg(details), -1, QString()) {}
};

/**
 * @brief The SDK gave up waiting: the request did not finish within
 * Response::replyTimeout() (STELLAR_QT_REPLY_TIMEOUT unless set), or the
 * network stack timed it out.
 *
 * Unlike a connection that failed, the request may have reached Horizon: a
 * submitted transaction may still be applied, as after Horizon's own 504
 * (RequestTimeoutException). Resubmit the SAME signed envelope, or look the
 * transaction up by hash; do not rebuild it with a new sequence number.
 *
 * Before 1.0.4 this was reported as a plain ConnectionErrorException. It
 * still is one, so a `catch (ConnectionErrorException&)` or a dynamic_cast
 * to it keeps catching it; test for this class first to tell it apart.
 */
class ClientTimeoutException : public ConnectionErrorException
{
public:
    explicit ClientTimeoutException(int milliseconds, const QString& details = QString())
        : ConnectionErrorException(QStringLiteral("no answer within %1 ms, the request was abandoned "
                                                  "(it may still have been received)%2")
                                       .arg(milliseconds)
                                       .arg(details.isEmpty() ? QString() : QStringLiteral(": ") + details)),
          m_milliseconds(milliseconds) {}

    /** The time the SDK waited, in milliseconds. */
    int getTimeout() const { return m_milliseconds; }

private:
    int m_milliseconds;
};

} // namespace exception
} // namespace qstellar

#endif // QSTELLAR_EXCEPTION_CONNECTIONERROREXCEPTION_H
