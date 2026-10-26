#ifndef GETHEALTHRESPONSE_H
#define GETHEALTHRESPONSE_H

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QString>
#include <QStringList>
#include <QtGlobal>
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * Result of `getHealth` (Soroban RPC).
 *
 *   { "status": "healthy",
 *     "latestLedger": ...,
 *     "oldestLedger": ...,
 *     "ledgerRetentionWindow": ... }
 */
class GetHealthResponse
{
    QString m_status;
    quint32 m_latestLedger = 0;
    quint32 m_oldestLedger = 0;
    quint32 m_ledgerRetentionWindow = 0;
public:
    GetHealthResponse() = default;

    /**
     * Builds the response from the JSON-RPC `result` object. Throws
     * std::runtime_error when a field is missing or not what the RPC sends:
     * a status other than "healthy" (an unhealthy RPC answers with an error
     * instead), or a ledger number that is not a JSON integer from 0 to
     * 4294967295.
     */
    static GetHealthResponse fromJson(const QJsonObject& result);

    QString getStatus() const { return m_status; }
    quint32 getLatestLedger() const { return m_latestLedger; }
    quint32 getOldestLedger() const { return m_oldestLedger; }
    quint32 getLedgerRetentionWindow() const { return m_ledgerRetentionWindow; }
};

/**
 * Internal: the field readers every Soroban RPC response reader uses. Declared
 * here, with the first of those readers; not meant to be used on its own.
 */
namespace sorobanrpcdetail {

/**
 * Reads one JSON object of a Soroban RPC answer, field by field, in exactly
 * the form Stellar RPC sends each field, or throws std::runtime_error naming
 * the method and the field.
 *
 * A number is a JSON integer, or a decimal string where the RPC sends a
 * 64-bit value as one; other text, a fraction, a negative number where none
 * can be, a value out of range, and a field the RPC always sends that is
 * missing or null are all refused. A hash is 64 hex digits, and XDR is strict
 * base64 of whole 4-byte words, written the one way the RPC writes it. A list
 * the RPC leaves out when it is empty reads as empty when it is missing. A
 * field the RPC has deprecated, and will stop sending, may be missing: its
 * absence is recorded, not refused. Each refusal shows the value refused, with
 * integers written exactly.
 */
class RpcFields
{
public:
    /** `method` names the RPC method in errors, and `path` prefixes each
     *  field's name there (e.g. "events[2]."). */
    RpcFields(const QJsonObject& object, const QString& method, const QString& path = QString());

    const QJsonObject& json() const { return m_object; }
    const QString& method() const { return m_method; }
    /** The field's name as errors give it. */
    QString name(const QString& key) const { return m_path + key; }
    /** Whether the field is there at all (null included). */
    bool has(const QString& key) const { return m_object.contains(key); }

    /** Throws: the field is there, but it `problem`s (e.g. "is not ..."). */
    [[noreturn]] void refuse(const QString& key, const QString& problem) const;

    /** A JSON integer from `min` to `max`. */
    qint64 integer(const QString& key, qint64 min, qint64 max) const;
    /** A JSON integer from 0 to 4294967295. */
    quint32 uint32(const QString& key) const;
    /** The same, or false when the field is not there. */
    bool optionalUint32(const QString& key, quint32* value) const;
    /** A JSON integer from 0 to 2147483647: a field the SDK keeps as qint32. */
    qint32 int32(const QString& key) const;
    /** A JSON integer from 0 to 9223372036854775807, as its decimal text. */
    QString int64Text(const QString& key) const;
    /** A decimal string (digits only) from 0 to `max`, as its canonical text. */
    QString decimal(const QString& key, quint64 max) const;
    /** The same, or an empty string when the field is not there. */
    QString optionalDecimal(const QString& key, quint64 max) const;

    /** A JSON string, which may be empty. */
    QString string(const QString& key) const;
    /** The same, or an empty string when the field is not there. */
    QString optionalString(const QString& key) const;
    /** A JSON string that is one of `values`. */
    QString oneOf(const QString& key, const QStringList& values) const;
    bool boolean(const QString& key) const;
    /** Whether the field is there. When it is, *value is set to it (null is
     *  refused); when it is not, *value is left as it was, the caller's
     *  default. */
    bool optionalBoolean(const QString& key, bool* value) const;
    /** 64 hex digits (a transaction or ledger hash). */
    QString hash(const QString& key) const;
    /** Base64 XDR. */
    QString xdr(const QString& key) const;
    /** The same, or an empty string when the field is not there. */
    QString optionalXdr(const QString& key) const;
    /** A JSON array of base64 XDR, or an empty list when the field is not there. */
    QStringList xdrList(const QString& key) const;
    /** `value`, named `name` in errors, as a JSON array of base64 XDR. */
    QStringList xdrListValue(const QString& name, const QJsonValue& value) const;
    /** A JSON array. */
    QJsonArray array(const QString& key) const;
    /** A JSON object, read the same way. */
    RpcFields object(const QString& key) const;
    /** Element `index` of `array` (the field `key`), which must be an object. */
    RpcFields element(const QString& key, const QJsonArray& array, int index) const;

private:
    /** The field, or throws when it is missing or null. */
    QJsonValue value(const QString& key) const;

    QJsonObject m_object;
    QString m_method;
    QString m_path;
};

/**
 * Whether `text` is strict, canonical base64 of one or more whole XDR (4-byte)
 * words, as the RPC writes it: nothing but the base64 alphabet, padded with
 * '=' to a multiple of 4 characters, and with the unused bits of the last
 * character zero, so that each value has exactly one spelling.
 */
bool isBase64Xdr(const QString& text);
/** Whether `text` is 64 hex digits. */
bool isHexHash(const QString& text);

} // namespace sorobanrpcdetail

QSTELLAR_END_NS

QSTELLAR_ALIAS(GetHealthResponse)




#endif // GETHEALTHRESPONSE_H
