#include "federationresponse.h"
#include "../exception/federationexception.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QNetworkReply>
#include <QPointer>
#include <stdexcept>
#include <string>
#include "../strkey.h"

QSTELLAR_BEGIN_NS


namespace {

// A G or M address exactly as it encodes: checksum and canonical spelling.
bool isAccountAddress(const QString& value)
{
    try {
        if (value.size() == StrKey::ACCOUNT_ID_ADDRESS_LENGTH) {
            return StrKey::decodeVersionByte(value) == StrKey::VersionByte::ACCOUNT_ID
                   && StrKey::encodeStellarAccountId(StrKey::decodeStellarAccountId(value)) == value;
        }
        if (value.size() == StrKey::MUXED_ACCOUNT_ADDRESS_LENGTH) {
            return StrKey::decodeVersionByte(value) == StrKey::VersionByte::MUXED_ACCOUNT
                   && StrKey::encodeStellarMuxedAccount(StrKey::encodeToXDRMuxedAccount(value)) == value;
        }
    } catch (const std::exception&) {
    }
    return false;
}

// A member that is a string, or empty when absent or null. A number was
// converted on its way into the QString property, so an id memo past 2^53
// arrived rounded: a different customer's.
QString stringMember(const QJsonObject& answer, const char* key)
{
    const QJsonValue value = answer.value(QLatin1String(key));
    if (value.isUndefined() || value.isNull())
        return QString();
    if (!value.isString())
        throw std::runtime_error(std::string("the federation answer's ") + key + " is not a string");
    return value.toString();
}

// What a wallet pays: the account and the memo that routes the payment, each
// valid for what it says it is. The values are not quoted back.
void requirePayableAnswer(const QJsonObject& answer)
{
    if (!isAccountAddress(stringMember(answer, "account_id")))
        throw std::runtime_error("the federation answer has no valid account_id (G... or M...)");
    const QString memoType = stringMember(answer, "memo_type");
    const QString memo = stringMember(answer, "memo");
    if (memoType.isEmpty()) {
        if (!memo.isEmpty())
            throw std::runtime_error("the federation answer has a memo without its memo_type");
        return;
    }
    // Absent or null only: an empty text memo is a memo (core takes
    // MEMO_TEXT ""), and js and py hand it back. An empty id or hash memo
    // is refused below, by its own format.
    const QJsonValue given = answer.value(QLatin1String("memo"));
    if (given.isUndefined() || given.isNull())
        throw std::runtime_error("the federation answer has a memo_type without a memo");
    if (memoType == QLatin1String("text")) {
        if (memo.toUtf8().size() > 28)
            throw std::runtime_error("the federation answer's text memo is longer than 28 bytes");
    } else if (memoType == QLatin1String("id")) {
        bool ok = false;
        const quint64 id = memo.toULongLong(&ok, 10);
        if (!ok || QString::number(id) != memo)
            throw std::runtime_error("the federation answer's id memo is not a uint64 in decimal");
    } else if (memoType == QLatin1String("hash")) {
        const auto hash = QByteArray::fromBase64Encoding(
            memo.toLatin1(), QByteArray::Base64Encoding | QByteArray::AbortOnBase64DecodingErrors);
        if (!hash || (*hash).size() != 32)
            throw std::runtime_error("the federation answer's hash memo is not 32 bytes of base64");
    } else {
        throw std::runtime_error("the federation answer's memo_type is not text, id or hash");
    }
}

} // namespace

FederationResponse::FederationResponse(QNetworkReply *reply):Response(reply) {
    if (reply)
        limitSize(reply);
}

void FederationResponse::limitSize(QNetworkReply* reply)
{
    QPointer<QNetworkReply> watched(reply);
    connect(reply, &QNetworkReply::downloadProgress, this, [this, watched](qint64 received, qint64 total) {
        if (!watched || (received <= MaxSize && total <= MaxSize))
            return;
        // Not read on: the answer is no longer this response's, it is
        // stopped, and its end (an aborted request) says nothing more.
        QNetworkReply* reply = watched;
        disconnect(reply, &QNetworkReply::finished, this, nullptr);
        disconnect(reply, &QNetworkReply::downloadProgress, this, nullptr);
        reply->abort();
        reply->deleteLater();   // its destroyed() lets go of it, timeout included
        fail(QStringLiteral("the federation answer is larger than 100 KB"));
    });
}

void FederationResponse::loadFromJson(QByteArray data)
{
    // Checked before anything is stored, so a refused answer leaves no
    // account id behind for somebody to read anyway.
    if (data.size() > MaxSize)
        throw std::runtime_error("the federation answer is larger than 100 KB");
    QJsonParseError check;
    const QJsonDocument document = QJsonDocument::fromJson(data, &check);
    if (check.error != QJsonParseError::NoError)
        throw std::runtime_error(check.errorString().toStdString());
    if (!document.isObject())
        throw std::runtime_error("a federation answer is a JSON object");
    QJsonObject answer = document.object();
    if (!m_reverseOf.isEmpty()) {
        // A reverse lookup's answer is the record of the account asked for,
        // read as py reads it: its stellar_address is what was asked (js hands
        // back an answer without one), and an answer that leaves account_id
        // out, null or "" is about that account (py: `account_id or
        // data.get("account_id")`; "" was refused as naming another account).
        // One naming another account is refused: py puts the asked one in its
        // place, js hands it back as sent.
        if (stringMember(answer, "stellar_address").isEmpty())
            throw std::runtime_error("the federation answer to a reverse lookup has no stellar_address");
        const QJsonValue account = answer.value(QLatin1String("account_id"));
        if (account.isUndefined() || account.isNull()
            || (account.isString() && account.toString().isEmpty())) {
            answer.insert(QStringLiteral("account_id"), m_reverseOf);
            data = QJsonDocument(answer).toJson(QJsonDocument::Compact);
        } else if (account.isString() && account.toString() != m_reverseOf) {
            throw std::runtime_error("the federation answer to a reverse lookup is about another account");
        }
    }
    requirePayableAnswer(answer);
    Response::loadFromJson(data);
}

FederationResponse::FederationResponse(QString stellarAddress, QString accountId, QString memoType, QString memo) {
    m_stellarAddress = stellarAddress;
    m_accountId = accountId;
    m_memoType = memoType;
    m_memo = memo;
}

QString FederationResponse::getStellarAddress() {
    return m_stellarAddress;
}

QString FederationResponse::getAccountId() {
    return m_accountId;
}

QString FederationResponse::getMemoType() {
    return m_memoType;
}

QString FederationResponse::getMemo() {
    return m_memo;
}
void FederationResponse::fail(const QString& reason)
{
    emitError(new qstellar::exception::FederationException(
        QStringLiteral("federation: %1").arg(reason)));
}

QSTELLAR_END_NS
