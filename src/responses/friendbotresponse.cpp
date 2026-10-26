#include "friendbotresponse.h"

#include <limits>
#include <stdexcept>

QSTELLAR_BEGIN_NS

namespace {

bool isHexHash(const QString& hash)
{
    if (hash.size() != 64)
        return false;
    for (const QChar c : hash) {
        if (!((c >= QLatin1Char('0') && c <= QLatin1Char('9'))
              || (c >= QLatin1Char('a') && c <= QLatin1Char('f'))
              || (c >= QLatin1Char('A') && c <= QLatin1Char('F'))))
            return false;
    }
    return true;
}

} // namespace

FriendbotResponse::FriendbotResponse(QNetworkReply* reply)
    : Response(reply)
    , m_successful(false)
    , m_ledger(0)
{
}

FriendbotResponse::~FriendbotResponse()
{
}

void FriendbotResponse::loadFromJson(QByteArray data)
{
    // Only a 200 is friendbot's answer (a 201 or a 204 is not one it gives).
    if (getStatus() != 200)
        throw std::runtime_error(QStringLiteral("friendbot answered HTTP %1, not 200")
                                     .arg(getStatus()).toStdString());
    m_successful = false;
    m_ledger = 0;
    Response::loadFromJson(data); // each member of its type, exactly
    const QJsonObject answer = json();
    if (!isHexHash(m_hash))
        throw std::runtime_error("friendbot's answer names no transaction hash");
    if (!answer.value(QStringLiteral("successful")).isBool() || !m_successful)
        throw std::runtime_error(QStringLiteral("friendbot did not report transaction %1 successful")
                                     .arg(m_hash).toStdString());
    if (answer.contains(QStringLiteral("ledger"))
            && (m_ledger < 1 || m_ledger > static_cast<qint64>(std::numeric_limits<quint32>::max())))
        throw std::runtime_error(QStringLiteral("ledger: %1 is not a ledger sequence")
                                     .arg(m_ledger).toStdString());
}

QString FriendbotResponse::getHash() const
{
    return m_hash;
}

qint64 FriendbotResponse::getLedger() const
{
    return m_ledger;
}

QString FriendbotResponse::getEnvelopeXdr() const
{
    return m_envelopeXdr;
}

QString FriendbotResponse::getResultXdr() const
{
    return m_resultXdr;
}

QSTELLAR_END_NS
