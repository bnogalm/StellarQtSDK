#include "accountdataresponse.h"

#include <stdexcept>

QSTELLAR_BEGIN_NS

namespace {

/** stellar::DataValue is opaque<64>. */
const int MAX_DATA_VALUE_BYTES = 64;

} // namespace

AccountDataResponse::AccountDataResponse(QNetworkReply* reply)
    : Response(reply)
{
}

AccountDataResponse::~AccountDataResponse()
{
}

void AccountDataResponse::reset()
{
    Response::reset();
    m_decoded = QByteArray();
}

void AccountDataResponse::loadFromJson(QByteArray data)
{
    m_decoded = QByteArray();
    // "value" is required (Response::fillObject refuses it missing or null).
    Response::loadFromJson(data);
    const QByteArray text = m_value.toLatin1();
    const QByteArray::FromBase64Result decoded =
        QByteArray::fromBase64Encoding(text, QByteArray::AbortOnBase64DecodingErrors);
    // Canonical only: the text must be what encoding the bytes gives back, so
    // that one value has one form (no missing padding, no stray bits, no
    // characters outside the alphabet; QString::toLatin1 makes those '?').
    if (decoded.decodingStatus != QByteArray::Base64DecodingStatus::Ok
            || decoded.decoded.toBase64() != text
            || QString::fromLatin1(text) != m_value)
        throw std::runtime_error("value: not base64 (canonical, with padding)");
    if (decoded.decoded.size() > MAX_DATA_VALUE_BYTES)
        throw std::runtime_error(QStringLiteral("value: %1 bytes, but a data entry holds at most %2")
                                     .arg(decoded.decoded.size()).arg(MAX_DATA_VALUE_BYTES).toStdString());
    m_decoded = decoded.decoded.isNull() ? QByteArray("") : decoded.decoded;
}

QByteArray AccountDataResponse::value() const
{
    return m_decoded;
}

QString AccountDataResponse::valueBase64() const
{
    return m_value;
}

QString AccountDataResponse::sponsor() const
{
    return m_sponsor;
}

QSTELLAR_END_NS
