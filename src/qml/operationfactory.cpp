#include "operationfactory.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QScopedPointer>
#include <QSet>
#include <QHash>
#include <QLocale>
#include <QVariantList>
#include <cstring>
#include <stdexcept>

#include "assetregistry.h"
#include "network.h"
#include "operationtable.h"

#include "accountmergeoperation.h"
#include "allowtrustoperation.h"
#include "asset.h"
#include "beginsponsoringfuturereservesoperation.h"
#include "bumpsequenceoperation.h"
#include "changetrustasset.h"
#include "liquiditypool.h"
#include "changetrustoperation.h"
#include "claimant.h"
#include "claimclaimablebalanceoperation.h"
#include "clawbackclaimablebalanceoperation.h"
#include "clawbackoperation.h"
#include "createaccountoperation.h"
#include "createclaimablebalanceoperation.h"
#include "createpassivesellofferoperation.h"
#include "endsponsoringfuturereservesoperation.h"
#include "extendfootprintttloperation.h"
#include "inflationoperation.h"
#include "invokehostfunctionoperation.h"
#include "keypair.h"
#include "liquiditypooldepositoperation.h"
#include "liquiditypoolwithdrawoperation.h"
#include "managebuyofferoperation.h"
#include "managedataoperation.h"
#include "managesellofferoperation.h"
#include "operation.h"
#include "pathpaymentstrictreceiveoperation.h"
#include "pathpaymentstrictsendoperation.h"
#include "paymentoperation.h"
#include "predicate.h"
#include "price.h"
#include "restorefootprintoperation.h"
#include "revokeaccountsponsorshipoperation.h"
#include "revokeclaimablebalancesponsorshipoperation.h"
#include "revokedatasponsorshipoperation.h"
#include "revokeliquiditypoolsponsorshipoperation.h"
#include "revokeoffersponsorshipoperation.h"
#include "revokesignersponsorshipoperation.h"
#include "revoketrustlinesponsorshipoperation.h"
#include "settrustlineflagsoperation.h"
#include "setoptionsoperation.h"
#include "scval/scv.h"
#include "signerkey.h"
#include "stellaraddress.h"
#include "xdr/scval.h"

#include <algorithm>
#include <cmath>
#include <limits>

QSTELLAR_BEGIN_NS

namespace {

[[noreturn]] void refuseNumber(const QString& what, const QString& why)
{
    throw std::runtime_error(QStringLiteral("%1 %2").arg(what, why).toStdString());
}

/** Sign and magnitude of an exact integer, or an exception. See exactSigned. */
void exactParts(const QVariant& value, const QString& what, bool& negative, quint64& magnitude)
{
    switch (value.userType()) {
    case QMetaType::Int:
    case QMetaType::Long:
    case QMetaType::LongLong:
    case QMetaType::Short:
    case QMetaType::SChar: {
        const qint64 v = value.toLongLong();
        negative = v < 0;
        magnitude = negative ? quint64(0) - static_cast<quint64>(v) : static_cast<quint64>(v);
        return;
    }
    case QMetaType::UInt:
    case QMetaType::ULong:
    case QMetaType::ULongLong:
    case QMetaType::UShort:
    case QMetaType::UChar:
        negative = false;
        magnitude = value.toULongLong();
        return;
    case QMetaType::Double:
    case QMetaType::Float: {
        // What a JavaScript number arrives as, whole or not. It is exact only
        // up to 2^53; past that it is already a different integer from the
        // one in the source, and printing it back gives a third.
        const double d = value.toDouble();
        if (!std::isfinite(d) || std::trunc(d) != d)
            refuseNumber(what, QStringLiteral("must be a whole number"));
        if (d == 0 && std::signbit(d))
            refuseNumber(what, QStringLiteral("must be a whole number (not -0)"));
        // 2^53 itself is out: 2^53 + 1 rounds to it, so a double holding
        // 2^53 may stand for either. Number.MAX_SAFE_INTEGER is 2^53 - 1.
        if (std::fabs(d) >= 9007199254740992.0)
            refuseNumber(what, QStringLiteral("is too large to be exact as a JavaScript "
                                              "number: pass it as text"));
        negative = d < 0;
        magnitude = static_cast<quint64>(std::fabs(d));
        return;
    }
    case QMetaType::QString: {
        const QString text = value.toString().trimmed();
        int i = 0;
        negative = false;
        if (!text.isEmpty() && text.at(0) == QLatin1Char('-')) {
            negative = true;
            i = 1;
        }
        const int digits = static_cast<int>(text.size()) - i;
        bool ok = digits > 0 && digits <= 20;
        for (int k = i; ok && k < text.size(); ++k)
            ok = text.at(k) >= QLatin1Char('0') && text.at(k) <= QLatin1Char('9');
        if (!ok)
            refuseNumber(what, QStringLiteral("must be a whole number"));
        magnitude = text.mid(i).toULongLong(&ok);
        if (!ok)
            refuseNumber(what, QStringLiteral("is out of range"));
        if (negative && magnitude == 0)
            refuseNumber(what, QStringLiteral("must be a whole number (not -0)"));
        return;
    }
    default:
        // bool, lists, maps, QJSValue...: none of them is a number anybody typed.
        refuseNumber(what, QStringLiteral("must be a whole number"));
    }
}

} // namespace

void refuseUnknownArgs(const QVariantMap& args, std::initializer_list<const char*> allowed,
                       const QString& call)
{
    for (auto it = args.constBegin(); it != args.constEnd(); ++it) {
        bool known = false;
        for (const char* key : allowed)
            known = known || it.key() == QLatin1String(key);
        if (!known)
            throw std::runtime_error(QStringLiteral("%1 has no argument \"%2\"")
                                         .arg(call, it.key()).toStdString());
    }
}

bool isMissing(const QVariant& value)
{
    return !value.isValid() || value.userType() == QMetaType::Nullptr;
}

qint64 exactSigned(const QVariant& value, const QString& what, qint64 min, qint64 max)
{
    bool negative = false;
    quint64 magnitude = 0;
    exactParts(value, what, negative, magnitude);
    const QString range = QStringLiteral("must be from %1 to %2").arg(min).arg(max);
    qint64 v = 0;
    if (!negative) {
        if (magnitude > static_cast<quint64>(std::numeric_limits<qint64>::max()))
            refuseNumber(what, range);
        v = static_cast<qint64>(magnitude);
    } else {
        const quint64 limit = static_cast<quint64>(std::numeric_limits<qint64>::max()) + 1;
        if (magnitude > limit)
            refuseNumber(what, range);
        v = magnitude == limit ? std::numeric_limits<qint64>::min()
                               : -static_cast<qint64>(magnitude);
    }
    if (v < min || v > max)
        refuseNumber(what, range);
    return v;
}

QString exactText(const QVariant& value, const QString& what)
{
    if (isMissing(value))
        return QString();
    switch (value.userType()) {
    case QMetaType::QString:
        return value.toString();
    case QMetaType::Int:
    case QMetaType::Long:
    case QMetaType::LongLong:
    case QMetaType::Short:
    case QMetaType::SChar:
    case QMetaType::UInt:
    case QMetaType::ULong:
    case QMetaType::ULongLong:
    case QMetaType::UShort:
    case QMetaType::UChar:
        if (value.userType() == QMetaType::ULongLong || value.userType() == QMetaType::ULong
                || value.userType() == QMetaType::UInt)
            return QString::number(value.toULongLong());
        return QString::number(value.toLongLong());
    case QMetaType::Double:
    case QMetaType::Float: {
        const double d = value.toDouble();
        if (!std::isfinite(d) || std::trunc(d) != d)
            refuseNumber(what, QStringLiteral("must be text (a number is written out only "
                                              "when it is whole)"));
        return QString::number(exactSigned(value, what, std::numeric_limits<qint64>::min(),
                                           std::numeric_limits<qint64>::max()));
    }
    default:
        refuseNumber(what, QStringLiteral("must be text"));
    }
}

namespace {

/**
 * A key that is there but holds nothing readable.
 *
 * On Qt 5 a JS function arrives exactly like undefined: an invalid QVariant
 * under a key that is present. Where "left out" would silently mean something
 * — unconditional, delete the entry, no arguments — the key being there is
 * the tell that the caller meant to give something, and it is refused.
 */
bool presentButUnreadable(const QVariantMap& m, const QString& key)
{
    return m.contains(key) && !m.value(key).isValid();
}

} // namespace

quint64 exactUnsigned(const QVariant& value, const QString& what, quint64 max)
{
    bool negative = false;
    quint64 magnitude = 0;
    exactParts(value, what, negative, magnitude);
    if (negative || magnitude > max)
        refuseNumber(what, QStringLiteral("must be from 0 to %1").arg(max));
    return magnitude;
}

namespace {

QString str(const QVariantMap& m, const char* key)
{
    return m.value(QString::fromLatin1(key)).toString().trimmed();
}

/**
 * A required text that is payload (a data name, a function name): exactly as
 * given, spaces included, where need() trims. Trimmed, " lead" named another
 * data entry and was signed.
 */
QString payloadText(const QVariantMap& m, const char* key)
{
    const QString v = exactText(m.value(QString::fromLatin1(key)), QString::fromLatin1(key));
    if (v.isEmpty())
        throw std::runtime_error(QStringLiteral("missing field: %1")
                                     .arg(QString::fromLatin1(key)).toStdString());
    return v;
}

/** Required field, with a message that names what is missing. */
QString need(const QVariantMap& m, const char* key)
{
    const QString v = str(m, key);
    if (v.isEmpty())
        throw std::runtime_error(QStringLiteral("missing field: %1")
                                     .arg(QString::fromLatin1(key)).toStdString());
    return v;
}

/**
 * Whether a key is present at all, as opposed to present and empty.
 *
 * SetOptions needs the distinction and nothing else does: for most of its
 * fields zero is a real instruction — master weight 0 retires the master key,
 * signer weight 0 deletes a signer, an empty home domain clears it — so "is it
 * set" cannot mean "is it non-empty".
 */
bool isSet(const QVariantMap& m, const char* key)
{
    // Qt 5 hands a JS function over as an invalid QVariant under a key that
    // is there: "not set" would silently drop the field (a master weight, a
    // threshold) the caller meant to give.
    if (presentButUnreadable(m, QString::fromLatin1(key)))
        throw std::runtime_error(QStringLiteral("%1 cannot be read; leave it out, or give a value")
                                     .arg(QString::fromLatin1(key)).toStdString());
    const auto it = m.constFind(QString::fromLatin1(key));
    // Blank means blank however it is spelled: a lone space used to count as
    // "set", and unsignedNumber() then read it as 0 — a master weight of 0,
    // or a signer weight of 0 that deletes the signer.
    return it != m.constEnd() && it.value().isValid() && !it.value().isNull()
           && !(it.value().userType() == QMetaType::QString
                && it.value().toString().trimmed().isEmpty());
}

/**
 * Refuses JSON number literals that QJsonDocument would round before this code
 * ever saw them.
 *
 * QJsonDocument stores a number as a double, and hands a whole one that fits
 * int64 back as an integer. So "216172782113783809.0" or "2.16172782113783809e17"
 * arrive here as the integer 216172782113783808 — indistinguishable from an
 * exact integer, and signed as an argument nobody typed. A plain integer
 * literal is exact (or, past int64, arrives as a double the reader refuses);
 * a decimal or exponent literal is exact only below 2^53.
 */
void refuseInexactJsonNumbers(const QByteArray& json, const char* what)
{
    bool inString = false;
    for (int i = 0; i < json.size(); ++i) {
        const char c = json.at(i);
        if (inString) {
            if (c == '\\')
                ++i;                    // skip the escaped character
            else if (c == '"')
                inString = false;
            continue;
        }
        if (c == '"') {
            inString = true;
            continue;
        }
        // A number starts at '-', a digit, or a '.' followed by a digit —
        // QJsonDocument accepts ".999…" and would round it to 1.
        const bool dotDigit = c == '.' && i + 1 < json.size()
                              && json.at(i + 1) >= '0' && json.at(i + 1) <= '9';
        if (c != '-' && !dotDigit && (c < '0' || c > '9'))
            continue;
        const int start = i;
        bool fractionOrExponent = false;
        while (i < json.size()) {
            const char t = json.at(i);
            if (t == '.' || t == 'e' || t == 'E')
                fractionOrExponent = true;
            else if (t != '+' && t != '-' && (t < '0' || t > '9'))
                break;
            ++i;
        }
        if (!fractionOrExponent) {
            // A plain integer is exact only while it is an int64: past that
            // QJsonDocument holds a double. Below INT64_MIN the double is
            // exactly INT64_MIN, which it then hands back as that integer, so
            // -9223372036854775809 was signed as -9223372036854775808.
            const QByteArray token = json.mid(start, i - start);
            bool allDigits = !token.isEmpty();
            for (int k = token.startsWith('-') ? 1 : 0; k < token.size(); ++k) {
                if (token.at(k) < '0' || token.at(k) > '9')
                    allDigits = false;
            }
            bool fits = false;
            token.toLongLong(&fits);
            if (allDigits && token != "-" && !fits) {
                throw std::runtime_error(
                    QStringLiteral("%1: the number %2 is outside the 64-bit range of JSON "
                                   "integers - write 64-bit values as strings")
                        .arg(QString::fromLatin1(what), QString::fromLatin1(token))
                        .toStdString());
            }
        }
        if (fractionOrExponent) {
            const QByteArray token = json.mid(start, i - start);
            // Exact only if the literal is itself a whole number: every digit
            // right of the decimal point, once the exponent has moved it, is 0.
            // "7.0000000000000001" is not, and QJsonDocument would still round
            // it to 7 and hand it over as an integer.
            QByteArray mantissa = token;
            qint64 exponent = 0;
            const int e = qMax(token.indexOf('e'), token.indexOf('E'));
            if (e >= 0) {
                mantissa = token.left(e);
                bool expOk = false;
                exponent = token.mid(e + 1).toLongLong(&expOk);
                if (!expOk)
                    exponent = 0;          // malformed: QJsonDocument refuses it
                // Clamped so the decimal-point arithmetic below cannot overflow,
                // and only by the literal's own length: past it, every position
                // of the decimal point means the same thing. (A fixed bound let
                // a mantissa padded beyond it pass as whole.)
                const qint64 reach = static_cast<qint64>(token.size()) + 1;
                exponent = std::max<qint64>(-reach, std::min<qint64>(exponent, reach));
            }
            if (mantissa.startsWith('-'))
                mantissa.remove(0, 1);
            const int dot = mantissa.indexOf('.');
            const QByteArray digits = dot >= 0 ? mantissa.left(dot) + mantissa.mid(dot + 1)
                                               : mantissa;
            const qint64 point = (dot >= 0 ? dot : mantissa.size()) + exponent;
            bool whole = true;
            for (int k = 0; k < digits.size(); ++k) {
                if (k >= point && digits.at(k) != '0') {
                    whole = false;
                    break;
                }
            }
            bool ok = false;
            const double v = token.toDouble(&ok);
            if (ok && (!whole || std::fabs(v) >= 9007199254740992.0)) {
                throw std::runtime_error(
                    QStringLiteral("%1: the number %2 is not an exact whole number in JSON - "
                                   "write 64-bit values as strings")
                        .arg(QString::fromLatin1(what), QString::fromLatin1(token))
                        .toStdString());
            }
        }
        --i;
    }
}

/**
 * Whether JSON text nests arrays and objects deeper than `maxDepth`, counted
 * before Qt parses it. Qt takes up to 1024 levels, and parsing, converting
 * and freeing each recurse once per level: 250 nested "vec" arguments, 500
 * levels, overflowed a debug build's stack in Tx.build, freeing the parsed
 * JSON while the refusal of the 129th level unwound. Brackets inside strings
 * do not count.
 */
bool jsonNestsDeeperThan(const QByteArray& json, int maxDepth)
{
    int depth = 0;
    bool inString = false;
    for (int i = 0; i < json.size(); ++i) {
        const char c = json.at(i);
        if (inString) {
            if (c == '\\')
                ++i;                    // skip the escaped character
            else if (c == '"')
                inString = false;
        } else if (c == '"') {
            inString = true;
        } else if (c == '[' || c == '{') {
            if (++depth > maxDepth)
                return true;
        } else if (c == ']' || c == '}') {
            --depth;
        }
    }
    return false;
}

/**
 * An optional integer field of an operation. Left out, or left blank, means
 * `fallback` — the convention isSet() documents for this file — and anything
 * given is read exactly (exactSigned) or refused. Several of these are int64,
 * so they are exact as text at any size and as a JS number only up to 2^53.
 */
qint64 wholeNumber(const QVariantMap& m, const char* key, qint64 fallback)
{
    const QVariant raw = m.value(QString::fromLatin1(key));
    // A present key with nothing readable in it (a JS function on Qt 5) is
    // not "left out": offerId would become 0 — a new offer instead of a change
    // to the existing one.
    if (presentButUnreadable(m, QString::fromLatin1(key)))
        throw std::runtime_error(QStringLiteral("%1 cannot be read; leave it out, or give a value")
                                     .arg(QString::fromLatin1(key)).toStdString());
    if (isMissing(raw) || (raw.userType() == QMetaType::QString
                           && raw.toString().trimmed().isEmpty()))
        return fallback;
    return exactSigned(raw, QString::fromLatin1(key), std::numeric_limits<qint64>::min(),
                       std::numeric_limits<qint64>::max());
}

quint32 unsignedNumber(const QVariantMap& m, const char* key, quint32 fallback,
                       quint32 max = std::numeric_limits<quint32>::max())
{
    const QVariant raw = m.value(QString::fromLatin1(key));
    // A present key with nothing readable in it (a JS function on Qt 5) is
    // not "left out": offerId would become 0 — a new offer instead of a change
    // to the existing one.
    if (presentButUnreadable(m, QString::fromLatin1(key)))
        throw std::runtime_error(QStringLiteral("%1 cannot be read; leave it out, or give a value")
                                     .arg(QString::fromLatin1(key)).toStdString());
    if (isMissing(raw) || (raw.userType() == QMetaType::QString
                           && raw.toString().trimmed().isEmpty()))
        return fallback;
    return static_cast<quint32>(exactUnsigned(raw, QString::fromLatin1(key), max));
}

/**
 * Weights and thresholds are uint32 in the XDR but bytes to the network: 256
 * is signed, and the transaction then fails — its fee spent. js-stellar-base
 * refuses them before signing; so does this.
 */
quint32 weight(const QVariantMap& m, const char* key)
{
    return unsignedNumber(m, key, 0, 255);
}

} // namespace

Network* qmlNetwork(const QString& name, const QString& passphrase)
{
    const QString n = name.trimmed().toLower();
    QString fromName;
    if (n == QLatin1String("testnet"))
        fromName = Network::TESTNET_NETWORK_PASSPHRASE();
    else if (n == QLatin1String("public") || n == QLatin1String("mainnet"))
        fromName = Network::PUBLIC_NETWORK_PASSPHRASE();
    else if (n == QLatin1String("futurenet"))
        fromName = Network::FUTURENET_NETWORK_PASSPHRASE();
    else if (!n.isEmpty())
        throw std::runtime_error(QStringLiteral("unknown network \"%1\": use testnet, public "
                                                "(or mainnet), futurenet, or give networkPassphrase")
                                     .arg(n).toStdString());

    const QString given = passphrase.trimmed().isEmpty() ? QString() : passphrase;
    if (!given.isNull() && !fromName.isNull() && given != fromName)
        throw std::runtime_error(QStringLiteral("network \"%1\" and networkPassphrase disagree")
                                     .arg(n).toStdString());
    const QString chosen = !given.isNull() ? given
                           : !fromName.isNull() ? fromName
                           : Network::TESTNET_NETWORK_PASSPHRASE();
    if (chosen == Network::PUBLIC_NETWORK_PASSPHRASE())
        return Network::publicNetwork();
    if (chosen == Network::TESTNET_NETWORK_PASSPHRASE())
        return Network::testnetNetwork();
    // Cached: Network has no value semantics here, and building one per call
    // would leak one per keystroke.
    static QHash<QString, Network*> custom;
    auto it = custom.constFind(chosen);
    if (it != custom.constEnd())
        return it.value();
    Network* net = new Network(chosen);
    custom.insert(chosen, net);
    return net;
}

/**
 * Hex, strictly: an even number of hex digits and nothing else. The bytes are
 * what gets signed, and QByteArray::fromHex skips what it does not understand:
 * "0xdeadbeef" became 00 de ad be ef, "abc" became 0a bc, and a salt one digit
 * short became a different salt — a different contract address.
 */
QByteArray strictHex(const QString& text, const QString& what)
{
    const QByteArray latin = text.toLatin1();
    bool ok = latin.size() % 2 == 0 && QString::fromLatin1(latin) == text;
    for (const char c : latin) {
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')))
            ok = false;
    }
    if (!ok)
        throw std::runtime_error(QStringLiteral("%1 must be hex: an even number of the "
                                                "digits 0-9 and a-f, nothing else")
                                     .arg(what).toStdString());
    return QByteArray::fromHex(latin);
}

namespace {

/**
 * Refuses keys other than `allowed` in an object the caller wrote: a key
 * nothing reads used to be ignored, and a misspelt "predicat" made a one-year
 * lock an unconditional claimable balance.
 */
void refuseKeysOtherThan(const QVariantMap& m, std::initializer_list<const char*> allowed,
                         const QString& what)
{
    for (auto it = m.constBegin(); it != m.constEnd(); ++it) {
        bool known = false;
        for (const char* key : allowed)
            known = known || it.key() == QLatin1String(key);
        if (!known)
            throw std::runtime_error(QStringLiteral("%1 has no field \"%2\"")
                                         .arg(what, it.key()).toStdString());
    }
}

/** An offer id: 0 (a new offer) or the id of an existing one, never negative. */
qint64 offerIdFrom(const QVariantMap& m)
{
    const qint64 id = wholeNumber(m, "offerId", 0);
    if (id < 0)
        throw std::runtime_error("offerId cannot be negative");
    return id;
}

/** A 32-byte id given as 64 hex characters. */
QByteArray hexId(const QVariantMap& m, const char* key, int bytes)
{
    const QString raw = need(m, key);
    const QByteArray decoded = strictHex(raw, QString::fromLatin1(key));
    if (decoded.size() != bytes)
        throw std::runtime_error(QStringLiteral("%1 must be %2 hex characters (%3 bytes)")
                                     .arg(QString::fromLatin1(key))
                                     .arg(bytes * 2).arg(bytes).toStdString());
    return decoded;
}

/**
 * A list of assets, accepted either as a JSON array or as a comma-separated
 * line, because a form field that demands JSON for two hops is hostile.
 */
QStringList assetList(const QVariantMap& m, const char* key)
{
    const QVariant raw = m.value(QString::fromLatin1(key));
    if (!raw.isValid() || raw.isNull())
        return {};

    if (raw.userType() == QMetaType::QVariantList) {
        QStringList out;
        for (const QVariant& entry : raw.toList())
            out.append(entry.toString().trimmed());
        return out;
    }

    QString text = raw.toString().trimmed();
    if (text.isEmpty() || text == QLatin1String("[]"))
        return {};

    if (text.startsWith(QLatin1Char('['))) {
        // A list of asset names: one level.
        if (jsonNestsDeeperThan(text.toUtf8(), 2))
            throw std::runtime_error(QStringLiteral("%1 is a list of assets, not nested lists")
                                         .arg(QString::fromLatin1(key)).toStdString());
        QJsonParseError error{};
        const QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8(), &error);
        if (error.error != QJsonParseError::NoError || !doc.isArray())
            throw std::runtime_error(QStringLiteral("%1 is not a valid JSON array: %2")
                                         .arg(QString::fromLatin1(key), error.errorString())
                                         .toStdString());
        QStringList out;
        for (const QJsonValue& entry : doc.array())
            out.append(entry.toString().trimmed());
        return out;
    }

    QStringList out;
    for (const QString& piece : text.split(QLatin1Char(',')))
        if (!piece.trimmed().isEmpty())
            out.append(piece.trimmed());
    return out;
}

/**
 * One term of an exact price: a whole number from 1 to 2^31 - 1.
 *
 * QVariant::toInt() would have rounded 1.9 to 2, turned "x" into 0 and
 * wrapped 3e9 to a negative number — each of them a valid-looking price the
 * caller never asked for, signed without a word.
 */
qint32 priceTerm(const QVariantMap& exact, const char* key, const char* field)
{
    return static_cast<qint32>(exactSigned(
        exact.value(QString::fromLatin1(key)),
        QStringLiteral("%1R.%2").arg(QString::fromLatin1(field), QString::fromLatin1(key)),
        1, std::numeric_limits<qint32>::max()));
}

/**
 * The price, exactly.
 *
 * A Stellar price IS a fraction, and the decimal a user types may only
 * approximate one. When the caller knows the fraction it wants — because it
 * read one back off the ledger — it passes priceR and gets it verbatim;
 * otherwise Price(QString) reads the decimal exactly and approximates only
 * when the decimal is not an int32 fraction itself.
 */
Price priceFrom(const QVariantMap& m, const char* key)
{
    const QString name = QString::fromLatin1(key);
    const QString exactName = name + QStringLiteral("R");
    // There but unreadable (a JS function on Qt 5) is not "not given": it let
    // the other form through when the caller had given both.
    for (const QString& k : {name, exactName}) {
        if (presentButUnreadable(m, k))
            throw std::runtime_error(QStringLiteral("%1 cannot be read; leave it out, or give a value")
                                         .arg(k).toStdString());
    }
    auto given = [&](const QString& k) {
        const QVariant v = m.value(k);
        return !isMissing(v) && !(v.userType() == QMetaType::QString
                                  && v.toString().trimmed().isEmpty());
    };
    const QVariant raw = m.value(exactName);
    const QVariant decimal = m.value(name);
    // The decimal is text or a number. An object or an array there (the
    // {n, d} shape js-stellar-base also takes) used to read as no price at
    // all, so a stale priceR beside it was signed.
    if (given(name) && decimal.userType() != QMetaType::QString
            && (decimal.userType() == QMetaType::QVariantMap
                || decimal.userType() == QMetaType::QVariantList
                || !decimal.canConvert<double>()))
        throw std::runtime_error(QStringLiteral("%1 must be a decimal, as text or a number; "
                                                "an exact fraction goes in %1R as { n, d }")
                                     .arg(name).toStdString());
    if (given(exactName)) {
        // One price, not two. A form that kept a priceR from an offer it
        // loaded earlier while the user typed a new decimal would otherwise
        // sign the stale fraction.
        if (given(name))
            throw std::runtime_error(QStringLiteral("give %1 or %1R, not both")
                                         .arg(name).toStdString());
        const QVariantMap exact = raw.toMap();
        if (exact.isEmpty())
            throw std::runtime_error(QStringLiteral("%1R must be { n, d }")
                                         .arg(QString::fromLatin1(key)).toStdString());
        refuseKeysOtherThan(exact, {"n", "d"}, exactName);
        return Price(priceTerm(exact, "n", key), priceTerm(exact, "d", key));
    }
    return Price(need(m, key));
}

/**
 * One node of a claim predicate tree.
 *
 * The tree is the spec, mirrored:
 *
 *   { type: "unconditional" }
 *   { type: "absBefore", value: "1798761600" }   Unix seconds
 *   { type: "relBefore", value: "86400" }        seconds after the ledger closes
 *   { type: "not",  predicate:  { ... } }
 *   { type: "and",  predicates: [ { ... }, { ... } ] }
 *   { type: "or",   predicates: [ { ... }, { ... } ] }
 *
 * A returned Predicate owns its children, so deleting the root frees the tree.
 */
Predicate* predicateFrom(const QVariantMap& node, int depth = 1)
{
    // Core refuses a predicate nested deeper than 4 (validatePredicate): it was
    // signed and then refused, fee spent; and a hostile tree with no bound at
    // all ran the stack out.
    if (depth > 4)
        throw std::runtime_error("claim predicates nest at most 4 deep");
    const QString type = str(node, "type").toLower();
    if (type.isEmpty())
        throw std::runtime_error("a claim predicate needs a type");

    if (type == QLatin1String("unconditional")) {
        refuseKeysOtherThan(node, {"type"}, QStringLiteral("an unconditional predicate"));
        return new Predicate::Unconditional();
    }

    if (type == QLatin1String("absbefore") || type == QLatin1String("relbefore")) {
        refuseKeysOtherThan(node, {"type", "value"}, QStringLiteral("a %1 predicate").arg(type));
        const qint64 seconds = exactSigned(node.value(QStringLiteral("value")),
                                           QStringLiteral("a claim predicate value"), 0,
                                           std::numeric_limits<qint64>::max());
        if (type == QLatin1String("absbefore"))
            return new Predicate::AbsBefore(seconds);
        return new Predicate::RelBefore(seconds);
    }

    if (type == QLatin1String("not")) {
        refuseKeysOtherThan(node, {"type", "predicate"}, QStringLiteral("a not predicate"));
        // Not takes ownership of what it is handed.
        return new Predicate::Not(
            predicateFrom(node.value(QStringLiteral("predicate")).toMap(), depth + 1));
    }

    if (type == QLatin1String("and") || type == QLatin1String("or")) {
        refuseKeysOtherThan(node, {"type", "predicates"}, QStringLiteral("an %1 predicate").arg(type));
        const QVariantList kids = node.value(QStringLiteral("predicates")).toList();
        // The XDR holds exactly two operands per node; nest for more.
        if (kids.size() != 2)
            throw std::runtime_error(
                "and/or claim predicates take exactly two operands; nest them for more");
        // Held one at a time: until the node exists nothing owns the first
        // child, so the second one throwing would leak it.
        QScopedPointer<Predicate> first(predicateFrom(kids.at(0).toMap(), depth + 1));
        QScopedPointer<Predicate> second(predicateFrom(kids.at(1).toMap(), depth + 1));
        QList<Predicate*> inner;
        inner.append(first.take());
        inner.append(second.take());
        if (type == QLatin1String("and"))
            return new Predicate::And(inner);    // takes ownership of both
        return new Predicate::Or(inner);         // takes ownership of both
    }

    throw std::runtime_error(
        QStringLiteral("unknown claim predicate type: %1").arg(type).toStdString());
}

/** The claimants of a claimable balance, given as JSON or as a QML array. */
QList<Claimant> claimantsFrom(const QVariantMap& op, const char* key)
{
    QVariantList entries;
    const QVariant raw = op.value(QString::fromLatin1(key));
    if (raw.userType() == QMetaType::QVariantList) {
        entries = raw.toList();
    } else {
        const QString text = raw.toString().trimmed();
        if (!text.isEmpty()) {
            QJsonParseError error{};
            // Ten claimants with predicates four deep nest a dozen levels.
            if (jsonNestsDeeperThan(text.toUtf8(), 32))
                throw std::runtime_error("claimants nest too deep");
            refuseInexactJsonNumbers(text.toUtf8(), "claimants");
            const QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8(), &error);
            if (error.error != QJsonParseError::NoError || !doc.isArray())
                throw std::runtime_error(
                    QStringLiteral("claimants is not a valid JSON array: %1")
                        .arg(error.errorString()).toStdString());
            entries = doc.array().toVariantList();
        }
    }

    if (entries.isEmpty())
        throw std::runtime_error("a claimable balance needs at least one claimant");
    if (entries.size() > 10)
        throw std::runtime_error("a claimable balance takes at most 10 claimants");

    QList<Claimant> claimants;
    claimants.reserve(entries.size());
    for (const QVariant& entry : entries) {
        const QVariantMap c = entry.toMap();
        // Only these two: "predicat", "Predicate" or "condition" used to be
        // ignored, and the balance then had no condition at all.
        refuseKeysOtherThan(c, {"destination", "predicate"}, QStringLiteral("a claimant"));
        // Destination first, so a missing one fails before a tree is allocated
        // and the message names the field.
        const QString destination = need(c, "destination");
        // Unconditional only when the predicate is left out. A predicate in
        // the wrong shape — a string, a list, {} — used to become an empty map
        // here and so "unconditional": a time-locked balance, claimable now.
        const QVariant given = c.value(QStringLiteral("predicate"));
        QVariantMap predicate;
        if (presentButUnreadable(c, QStringLiteral("predicate")))
            throw std::runtime_error("a claimant's predicate must be a { type, ... } object");
        if (isMissing(given)) {
            predicate.insert(QStringLiteral("type"), QStringLiteral("unconditional"));
        } else {
            if (given.userType() != QMetaType::QVariantMap || given.toMap().isEmpty())
                throw std::runtime_error("a claimant's predicate must be a { type, ... } object");
            predicate = given.toMap();
        }
        // Claimant takes ownership of the predicate.
        QScopedPointer<Predicate> tree(predicateFrom(predicate));
        claimants.append(Claimant(destination, tree.take()));
    }
    return claimants;
}

/**
 * One typed contract argument, from { type, value }.
 *
 * Soroban arguments are TYPED, and nothing guesses the type for you: the SDK
 * has no QVariant-to-SCVal bridge, and the host will refuse a u32 where the
 * contract declared an i128 rather than quietly converting it. So the form
 * carries the type alongside the value and this turns the pair into an SCVal.
 *
 * Recursive, because vectors and maps hold more of the same:
 *
 *   { "type": "u32",     "value": "42" }
 *   { "type": "symbol",  "value": "hello" }
 *   { "type": "address", "value": "C..." }        (a G... account works too)
 *   { "type": "bytes",   "value": "deadbeef" }    (hex)
 *   { "type": "vec",     "value": [ {...}, {...} ] }
 *   { "type": "map",     "value": [ { "key": {...}, "value": {...} } ] }
 *
 * `depth` is how deep this argument sits, 1 for a top-level one. Nesting is
 * refused past the limit the XDR reader enforces (stellar::MAX_SCVAL_DEPTH):
 * 250 nested vectors, 7 KB of JSON, overflowed the stack in a debug build.
 */
stellar::SCVal scValFrom(const QVariantMap& node, int depth);

std::string argumentsNestTooDeep()
{
    return QStringLiteral("contract arguments nest at most %1 deep")
        .arg(stellar::MAX_SCVAL_DEPTH).toStdString();
}

stellar::SCVal scValFromVariant(const QVariant& entry, int depth)
{
    if (entry.userType() != QMetaType::QVariantMap)
        throw std::runtime_error("each contract argument must be a { type, value } object");
    if (depth > stellar::MAX_SCVAL_DEPTH)
        throw std::runtime_error(argumentsNestTooDeep());
    return scValFrom(entry.toMap(), depth);
}

/**
 * A 128- or 256-bit contract integer, `bytes` wide, as the big-endian two's
 * complement the Scv factories take. Read exactly: decimal text (an optional
 * '-' and digits, which is how these values should travel), or an integer
 * QVariant, or a JS number while it is exact (below 2^53). Nothing is
 * rounded, and a value that does not fit the type is refused, not wrapped.
 */
QByteArray bigIntegerFrom(const QVariant& raw, const QString& type, int bytes, bool isSigned)
{
    const QString what = QStringLiteral("%1 argument").arg(type);
    bool negative = false;
    QString digits;
    if (raw.userType() == QMetaType::QString) {
        const QString text = raw.toString().trimmed();
        negative = text.startsWith(QLatin1Char('-'));
        digits = negative ? text.mid(1) : text;
        bool ok = !digits.isEmpty() && digits.size() <= 80;
        for (const QChar c : digits)
            ok = ok && c >= QLatin1Char('0') && c <= QLatin1Char('9');
        if (!ok)
            refuseNumber(what, QStringLiteral("must be a whole number, written in decimal"));
    } else {
        quint64 magnitude = 0;
        exactParts(raw, what, negative, magnitude);
        digits = QString::number(magnitude);
    }
    // The magnitude, big-endian, one byte wider than the type to see overflow.
    QByteArray mag(bytes + 1, '\0');
    for (const QChar c : digits) {
        unsigned carry = unsigned(c.unicode() - '0');
        for (int i = mag.size() - 1; i >= 0; --i) {
            const unsigned v = unsigned(static_cast<unsigned char>(mag[i])) * 10u + carry;
            mag[i] = char(v & 0xff);
            carry = v >> 8;
        }
        if (carry)
            refuseNumber(what, QStringLiteral("is out of range"));
    }
    bool zero = true;
    for (const char b : mag)
        zero = zero && b == 0;
    if (negative && zero)
        refuseNumber(what, QStringLiteral("must be a whole number (not -0)"));
    if (mag.at(0) != 0)
        refuseNumber(what, QStringLiteral("is out of range"));
    QByteArray value = mag.mid(1);
    const unsigned char top = static_cast<unsigned char>(value.at(0));
    if (!isSigned) {
        if (negative)
            refuseNumber(what, QStringLiteral("cannot be negative"));
        return value;
    }
    // Signed: up to 2^(8*bytes-1) - 1, and down to -2^(8*bytes-1).
    if (top & 0x80) {
        bool minimum = negative && top == 0x80;
        for (int i = 1; minimum && i < value.size(); ++i)
            minimum = value.at(i) == 0;
        if (!minimum)
            refuseNumber(what, QStringLiteral("is out of range"));
    }
    if (negative) {
        // Two's complement: invert and add one.
        unsigned carry = 1;
        for (int i = value.size() - 1; i >= 0; --i) {
            const unsigned v = unsigned(static_cast<unsigned char>(~static_cast<unsigned char>(value[i]))) + carry;
            value[i] = char(v & 0xff);
            carry = v >> 8;
        }
    }
    return value;
}

/**
 * A contract argument that holds no others: everything but vec and map. Kept
 * out of scValFrom, which recurses, so that each level of a nested argument
 * costs one small frame. With all of this inline a level took ~4.7 KB of a
 * debug build's stack, and refusing an argument at the depth limit then
 * overflowed it while freeing the JSON it had been parsed from.
 */
stellar::SCVal scalarScValFrom(const QString& type, const QVariant& raw)
{
    // Read exactly, and only by the types that are text: a JS number given
    // for a string used to reach the contract as "1e+06".
    auto textOf = [&](bool trim) {
        const QString t = exactText(raw, QStringLiteral("%1 argument").arg(type));
        return trim ? t.trimmed() : t;
    };

    // Integers are read from the value itself, not from its text: a 64-bit
    // argument given as a JSON or JS number above 2^53 has already been
    // rounded to a double, and its text is a third number again.
    auto whole = [&](const char* what, qint64 min, qint64 max) {
        return exactSigned(raw, QStringLiteral("%1 argument").arg(QString::fromLatin1(what)),
                           min, max);
    };
    auto unsignedWhole = [&](const char* what, quint64 max) {
        return exactUnsigned(raw, QStringLiteral("%1 argument").arg(QString::fromLatin1(what)),
                             max);
    };

    if (type == QLatin1String("void")) {
        // A value given with void was dropped in silence: the caller meant
        // something, and it was not "nothing".
        if (!isMissing(raw))
            throw std::runtime_error("a void argument takes no value");
        return Scv::toVoid();
    }
    // 32-bit arguments are range-checked, never cast: a cast narrowed in
    // silence, "4294967296" to u32 0 and "2147483648" to i32 -2147483648 — a
    // different argument from the one typed, in a transaction whose TxRep
    // cannot show it.
    if (type == QLatin1String("bool")) {
        // Only the two words. Anything else — "yes", "1", a typo of "true" —
        // used to become false and be signed as such.
        if (raw.userType() == QMetaType::Bool)
            return Scv::toBoolean(raw.toBool());
        const QString text = textOf(true);
        if (text.compare(QLatin1String("true"), Qt::CaseInsensitive) == 0)
            return Scv::toBoolean(true);
        if (text.compare(QLatin1String("false"), Qt::CaseInsensitive) == 0)
            return Scv::toBoolean(false);
        throw std::runtime_error("bool argument must be true or false");
    }
    const quint64 u64Max = std::numeric_limits<quint64>::max();
    const qint64 i64Min = std::numeric_limits<qint64>::min();
    const qint64 i64Max = std::numeric_limits<qint64>::max();
    if (type == QLatin1String("u32"))
        return Scv::toUint32(static_cast<quint32>(
            unsignedWhole("u32", std::numeric_limits<quint32>::max())));
    if (type == QLatin1String("i32"))
        return Scv::toInt32(static_cast<qint32>(
            whole("i32", std::numeric_limits<qint32>::min(), std::numeric_limits<qint32>::max())));
    // 128/256-bit integers (token amounts are i128): from exact decimal text.
    if (type == QLatin1String("u128")) return Scv::toUint128(bigIntegerFrom(raw, type, 16, false));
    if (type == QLatin1String("i128")) return Scv::toInt128(bigIntegerFrom(raw, type, 16, true));
    if (type == QLatin1String("u256")) return Scv::toUint256(bigIntegerFrom(raw, type, 32, false));
    if (type == QLatin1String("i256")) return Scv::toInt256(bigIntegerFrom(raw, type, 32, true));
    if (type == QLatin1String("u64"))       return Scv::toUint64(unsignedWhole("u64", u64Max));
    if (type == QLatin1String("i64"))       return Scv::toInt64(whole("i64", i64Min, i64Max));
    if (type == QLatin1String("timepoint")) return Scv::toTimePoint(unsignedWhole("timepoint", u64Max));
    if (type == QLatin1String("duration"))  return Scv::toDuration(unsignedWhole("duration", u64Max));
    // Not trimmed: " transfer" is not the symbol "transfer" (and not a
    // symbol at all: Scv::toSymbol refuses it, as js does).
    if (type == QLatin1String("symbol"))    return Scv::toSymbol(textOf(false));
    // A string argument is payload: signed exactly as given, spaces included.
    if (type == QLatin1String("string"))    return Scv::toString(textOf(false));
    if (type == QLatin1String("address"))   return Scv::toAddress(textOf(true));
    if (type == QLatin1String("bytes")) {
        // Text only: a JS number is not hex, and 255 used to sign 02 55.
        if (raw.userType() != QMetaType::QString)
            throw std::runtime_error("a bytes argument is written as hex text");
        return Scv::toBytes(strictHex(textOf(true), QStringLiteral("a bytes argument")));
    }
    throw std::runtime_error(
        QStringLiteral("unknown contract argument type: %1").arg(type).toStdString());
}

stellar::SCVal scValFrom(const QVariantMap& node, int depth)
{
    refuseKeysOtherThan(node, {"type", "value"}, QStringLiteral("a contract argument"));
    const QString type = str(node, "type").toLower();
    const QVariant raw = node.value(QStringLiteral("value"));
    if (type.isEmpty())
        throw std::runtime_error("a contract argument needs a type");
    // Every argument but void carries a value. Left out, or unreadable (a JS
    // function on Qt 5), it used to become "" or an empty container.
    if (type != QLatin1String("void") && isMissing(raw))
        throw std::runtime_error(QStringLiteral("a %1 argument needs a value")
                                     .arg(type).toStdString());
    const bool isVec = type == QLatin1String("vec");
    if (!isVec && type != QLatin1String("map"))
        return scalarScValFrom(type, raw);
    // A container must be given as one. raw.toList() of anything else is an
    // empty list, and an empty vec or map is a valid argument the host will
    // happily take — signed as nothing at all.
    if (raw.userType() != QMetaType::QVariantList)
        throw std::runtime_error(QStringLiteral("a %1 argument's value must be an array")
                                     .arg(type).toStdString());
    if (isVec) {
        QList<stellar::SCVal> items;
        for (const QVariant& entry : raw.toList())
            items.append(scValFromVariant(entry, depth + 1));
        return Scv::toVec(items);
    }
    // A list of pairs rather than an object, because an SCVal map key is
    // itself typed — it is not always, or even usually, a string. Scv::toMap
    // sorts the keys into the order the host requires and rejects
    // duplicates, so the canonical form comes for free.
    QList<stellar::SCVal> keys;
    QList<stellar::SCVal> values;
    for (const QVariant& entry : raw.toList()) {
        const QVariantMap pair = entry.toMap();
        refuseKeysOtherThan(pair, {"key", "value"}, QStringLiteral("a map entry"));
        keys.append(scValFromVariant(pair.value(QStringLiteral("key")), depth + 1));
        values.append(scValFromVariant(pair.value(QStringLiteral("value")), depth + 1));
    }
    return Scv::toMap(keys, values);
}

/** The argument list, given as a JSON array or as a QML array. */
QList<stellar::SCVal> scValListFrom(const QVariantMap& op, const char* key)
{
    QVariantList entries;
    const QVariant raw = op.value(QString::fromLatin1(key));
    if (presentButUnreadable(op, QString::fromLatin1(key)))
        throw std::runtime_error("the contract arguments must be an array");
    if (raw.userType() == QMetaType::QVariantList) {
        entries = raw.toList();
    } else if (!isMissing(raw) && raw.userType() != QMetaType::QString) {
        // An object here used to print as "" and sign an invocation with no
        // arguments at all.
        throw std::runtime_error("the contract arguments must be an array");
    } else {
        const QString text = raw.toString().trimmed();
        if (text.isEmpty() || text == QLatin1String("[]"))
            return {};
        // No argument list nests deeper than the list itself, three levels
        // per map (the argument, its entries, an entry) and, at the bottom, an
        // argument holding an empty container: 3 * MAX_SCVAL_DEPTH in all.
        if (jsonNestsDeeperThan(text.toUtf8(), 3 * stellar::MAX_SCVAL_DEPTH))
            throw std::runtime_error(argumentsNestTooDeep());
        QJsonParseError error{};
        refuseInexactJsonNumbers(text.toUtf8(), key);
        const QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8(), &error);
        if (error.error != QJsonParseError::NoError || !doc.isArray())
            throw std::runtime_error(
                QStringLiteral("the arguments are not a valid JSON array: %1")
                    .arg(error.errorString()).toStdString());
        entries = doc.array().toVariantList();
    }

    QList<stellar::SCVal> args;
    args.reserve(entries.size());
    for (const QVariant& entry : entries)
        args.append(scValFromVariant(entry, 1));
    return args;
}

/** A ContractExecutable naming already-uploaded WASM by its hash. */
stellar::ContractExecutable executableFromWasmHash(const QByteArray& hash)
{
    // wasmHash is a raw 32-byte array, so a short one would be a silent buffer
    // overrun rather than an error.
    if (hash.size() != 32)
        throw std::runtime_error("a WASM hash is 64 hex characters (32 bytes)");
    stellar::ContractExecutable executable;
    executable.type = stellar::ContractExecutableType::CONTRACT_EXECUTABLE_WASM;
    // uint256 is a raw quint8[32], not a container.
    std::memcpy(executable.wasmHash, hash.constData(), 32);
    return executable;
}

/** Trustline flags as a bitfield. */
QFlags<stellar::TrustLineFlags> trustLineFlags(const QVariantMap& m, const char* key)
{
    return QFlags<stellar::TrustLineFlags>(QFlag(int(unsignedNumber(m, key, 0))));
}

} // namespace

namespace {

/**
 * Refuses a key the operation does not read. It used to be ignored: Horizon's
 * own price_r beside a decimal price signed the decimal, and a misspelt field
 * was simply left out of what was signed. The keys an operation reads are the
 * fields of its row in the operation table, plus type and sourceAccount, plus
 * the exact form (priceR, minPriceR, maxPriceR) of each price field.
 */
void refuseUnknownKeys(const QVariantMap& op, const QString& type)
{
    const OpSpec* spec = operationSpec(type);
    if (!spec)
        return;                 // an unknown type is refused on its own
    QSet<QString> known{QStringLiteral("type"), QStringLiteral("sourceAccount")};
    for (const FieldSpec& field : spec->fields) {
        known.insert(QString::fromLatin1(field.name));
        if (qstrcmp(field.kind, "price") == 0)
            known.insert(QString::fromLatin1(field.name) + QStringLiteral("R"));
    }
    if (type == QLatin1String("createContract"))
        known.insert(QStringLiteral("constructorArgs"));
    for (auto it = op.constBegin(); it != op.constEnd(); ++it) {
        if (!known.contains(it.key()))
            throw std::runtime_error(QStringLiteral("a %1 operation has no field \"%2\"")
                                         .arg(type, it.key()).toStdString());
    }
}

/**
 * The bytes a manageData value stands for, as its valueFormat says: "text"
 * (the default, also when left blank) is the UTF-8 of the text; "hex" and
 * "base64" are decoded strictly. Anything else is refused.
 *
 * The table has always typed this field dataValue, "up to 64 bytes", but the
 * value was signed as the UTF-8 of whatever was typed: "deadbeef" meant as
 * four bytes went out as eight characters, and a value that is not UTF-8 —
 * most binary ones — could not be given at all.
 */
QByteArray dataValueBytes(const QVariantMap& op, const QString& value)
{
    if (presentButUnreadable(op, QStringLiteral("valueFormat")))
        throw std::runtime_error("valueFormat cannot be read; leave it out, or give "
                                 "text, hex or base64");
    const QVariant raw = op.value(QStringLiteral("valueFormat"));
    QString format = QStringLiteral("text");
    if (!isMissing(raw)) {
        if (raw.userType() != QMetaType::QString)
            throw std::runtime_error("valueFormat must be text, hex or base64");
        if (!raw.toString().trimmed().isEmpty())
            format = raw.toString().trimmed();
    }

    if (format == QLatin1String("text")) {
        // toUtf8() writes an unpaired surrogate as U+FFFD: a value nobody
        // typed would be signed.
        if (!value.isValidUtf16())
            throw std::runtime_error("value is not valid text (an unpaired surrogate)");
        return value.toUtf8();
    }
    if (format == QLatin1String("hex"))
        return strictHex(value, QStringLiteral("value"));
    if (format == QLatin1String("base64")) {
        // QByteArray::fromBase64 skips characters it does not know and takes
        // missing padding or stray low bits: "AP8", "AP9=" and "AP8=\n" all
        // decode to the same bytes as "AP8=". Only the canonical spelling of
        // the bytes is accepted, which is what re-encoding them gives back.
        const QByteArray latin = value.toLatin1();
        const QByteArray decoded = QByteArray::fromBase64(latin);
        if (QString::fromLatin1(latin) != value || decoded.toBase64() != latin)
            throw std::runtime_error("value must be base64: the standard alphabet with its "
                                     "= padding, nothing else");
        return decoded;
    }
    throw std::runtime_error(QStringLiteral("valueFormat must be text, hex or base64, not \"%1\"")
                                 .arg(format).toStdString());
}

/**
 * An amount that must be above zero (what js-stellar-base requires for these
 * fields). Zero or less is a malformed operation the network refuses: -5 was
 * signed as -50000000 stroops, and 0 as a payment of nothing.
 */
/**
 * An amount as the caller meant it, as decimal text ("" when left out). Text
 * is taken as it is (Operation::toXdrAmount reads it strictly). A JS number
 * arrives as a double: below 2^29 units every seven-decimal amount has a
 * double of its own, and its shortest decimal form is the amount the source
 * code wrote; from 2^29 on (the gap between doubles passes one stroop)
 * several amounts share one double, and only a whole number is still exact.
 * It was read with QVariant::toString(): 536870912.1234005 was signed as
 * 536870912.1234004, 922337203685.4775807 as 922337203685.4775, and 1e11 or
 * 1e-7 were refused as "1e+11" / "1e-07". js-stellar-base refuses any number;
 * py reads a float as its shortest decimal. This reads the ones that are
 * exact and asks for text otherwise.
 */
QString amountText(const QVariantMap& m, const char* key)
{
    const QVariant value = m.value(QString::fromLatin1(key));
    const int type = value.userType();
    if (type != QMetaType::Double && type != QMetaType::Float)
        return str(m, key);
    const double d = value.toDouble();
    const QString what = QString::fromLatin1(key);
    if (!std::isfinite(d))
        throw std::runtime_error((what + QStringLiteral(" is not a number")).toStdString());
    if (std::fabs(d) >= 536870912.0 && std::trunc(d) != d)
        throw std::runtime_error((what + QStringLiteral(" has more digits than a JavaScript number keeps "
                                                        "exactly: pass it as text")).toStdString());
    if (std::fabs(d) >= 9007199254740992.0)
        throw std::runtime_error((what + QStringLiteral(" is too large to be exact as a JavaScript "
                                                        "number: pass it as text")).toStdString());
    // Fixed notation, shortest digits that read back as this double.
    return QString::number(d, 'f', QLocale::FloatingPointShortest);
}

/** A required amount (see amountText). */
QString needAmount(const QVariantMap& m, const char* key)
{
    const QString text = amountText(m, key);
    if (text.isEmpty())
        throw std::runtime_error(QStringLiteral("missing field: %1")
                                     .arg(QString::fromLatin1(key)).toStdString());
    return text;
}

QString positiveAmount(const QVariantMap& m, const char* key)
{
    const QString text = needAmount(m, key);
    if (Operation::toXdrAmount(text) <= 0)
        throw std::runtime_error(QStringLiteral("%1 must be more than zero")
                                     .arg(QString::fromLatin1(key)).toStdString());
    return text;
}

} // namespace

Operation* makeOperation(const QVariantMap& op, AssetRegistry* assets)
{
    const QString type = str(op, "type");
    if (type.isEmpty())
        throw std::runtime_error("an operation needs a type");
    refuseUnknownKeys(op, type);

    // The per-operation source is an account id or nothing. An account OBJECT
    // printed as "" and was dropped, and the operation then debited the
    // transaction's source instead of the account named.
    const QVariant sourceRaw = op.value(QStringLiteral("sourceAccount"));
    if (presentButUnreadable(op, QStringLiteral("sourceAccount"))
            || (!isMissing(sourceRaw) && sourceRaw.userType() != QMetaType::QString))
        throw std::runtime_error("sourceAccount must be an account id (text)");

    Operation* built = nullptr;

    // ─── Payments and accounts ───────────────────────────────────────

    if (type == QLatin1String("payment")) {
        built = PaymentOperation::create(need(op, "destination"),
                                         assets->assetFor(need(op, "asset")),
                                         positiveAmount(op, "amount"));

    } else if (type == QLatin1String("createAccount")) {
        built = CreateAccountOperation::create(need(op, "destination"),
                                               needAmount(op, "startingBalance"));

    } else if (type == QLatin1String("accountMerge")) {
        built = AccountMergeOperation::create(need(op, "destination"));

    } else if (type == QLatin1String("pathPaymentStrictSend")) {
        // The second argument is named sendMax in the header; positionally it
        // is the exact amount sent, which is what makes this the strict-SEND
        // form. The header name is wrong, not the position.
        // Owned from the moment it exists. Three things below can throw —
        // malformed JSON in the path, more than five hops, an unparseable hop
        // — and ownership does not transfer until addOperation() much later,
        // so a bare pointer here abandoned the whole operation on every one of
        // them. The Builder rebuilds on every keystroke, so that was one leak
        // per character typed while an asset was half-written.
        QScopedPointer<PathPaymentStrictSendOperation> path(
            PathPaymentStrictSendOperation::create(
                assets->assetFor(need(op, "sendAsset")),
                positiveAmount(op, "sendAmount"),
                need(op, "destination"),
                assets->assetFor(need(op, "destAsset")),
                positiveAmount(op, "destMin")));
        const QStringList hops = assetList(op, "path");
        if (hops.size() > 5)
            throw std::runtime_error("a path takes at most 5 intermediate assets");
        if (!hops.isEmpty()) {
            QList<Asset*> resolved;
            for (const QString& hop : hops)
                resolved.append(assets->assetFor(hop));
            path->setPath(resolved);
        }
        built = path.take();

    } else if (type == QLatin1String("pathPaymentStrictReceive")) {
        QScopedPointer<PathPaymentStrictReceiveOperation> path(
            PathPaymentStrictReceiveOperation::create(
                assets->assetFor(need(op, "sendAsset")),
                positiveAmount(op, "sendMax"),
                need(op, "destination"),
                assets->assetFor(need(op, "destAsset")),
                positiveAmount(op, "destAmount")));
        const QStringList hops = assetList(op, "path");
        if (hops.size() > 5)
            throw std::runtime_error("a path takes at most 5 intermediate assets");
        if (!hops.isEmpty()) {
            QList<Asset*> resolved;
            for (const QString& hop : hops)
                resolved.append(assets->assetFor(hop));
            path->setPath(resolved);
        }
        built = path.take();

    // ─── Trust and authorisation ─────────────────────────────────────

    } else if (type == QLatin1String("changeTrust")) {
        const QString poolA = str(op, "poolShareAssetA");
        // One line: an asset, or a pool of two. Given both, the pool used to be
        // built and the asset silently dropped.
        if (!poolA.isEmpty() && !str(op, "asset").isEmpty())
            throw std::runtime_error("give asset, or poolShareAssetA and poolShareAssetB, not both");
        if (poolA.isEmpty() && (!str(op, "poolShareAssetB").isEmpty() || isSet(op, "poolShareFee")))
            throw std::runtime_error("a pool share trustline needs poolShareAssetA");
        // No limit (or a blank one) is the maximum, as js-stellar-base does;
        // "0" removes the trustline.
        const QString limit = amountText(op, "limit").isEmpty()
                              ? QStringLiteral("922337203685.4775807") : amountText(op, "limit");
        if (!poolA.isEmpty()) {
            // A trustline to a pool share rather than to an issued asset.
            // LiquidityPoolShareChangeTrustAsset TAKES OWNERSHIP of both
            // assets, so these are copies — handing it the registry's would be
            // a double free the moment the operation is destroyed.
            QScopedPointer<Asset> a(assets->copyOf(poolA));
            QScopedPointer<Asset> b(assets->copyOf(need(op, "poolShareAssetB")));
            if (!a || !b)
                throw std::runtime_error("both pool assets must be valid");

            // Check the ordering BEFORE letting go of either asset.
            // LiquidityPoolShareChangeTrustAsset's constructor throws from its
            // body when the assets are out of canonical order, and a
            // constructor that throws does not run its own destructor — so the
            // two assets, already released by take(), were abandoned along with
            // the KeyPair each of them owns. The fee is parsed first for the
            // same reason: it is a sibling argument, and argument evaluation
            // order is unspecified.
            if (LiquidityPool::compareAssets(a.data(), b.data()) >= 0)
                throw std::runtime_error("AssetA must be < AssetB in canonical "
                                         "XDR order; \"native\" always sorts first");
            const quint32 poolFeeRaw = unsignedNumber(op, "poolShareFee", 30);
            // 30 basis points is the only fee a pool has; core refuses any other.
            if (poolFeeRaw != 30)
                throw std::runtime_error("poolShareFee is 30 (the only pool fee there is)");
            const qint32 poolFee = static_cast<qint32>(poolFeeRaw);
            QScopedPointer<ChangeTrustAsset> line(new LiquidityPoolShareChangeTrustAsset(
                a.take(), b.take(), poolFee));
            built = ChangeTrustOperation::create(line.data(), limit);
        } else {
            built = ChangeTrustOperation::create(assets->assetFor(need(op, "asset")), limit);
        }

    } else if (type == QLatin1String("setTrustlineFlags")) {
        built = SetTrustlineFlagsOperation::create(need(op, "trustor"),
                                                   assets->assetFor(need(op, "asset")),
                                                   trustLineFlags(op, "clearFlags"),
                                                   trustLineFlags(op, "setFlags"));

    } else if (type == QLatin1String("allowTrust")) {
        // Superseded by setTrustlineFlags, kept because transactions in the
        // wild still carry it.
        // Each flag is a bool or the words true/false/1/0, and nothing else:
        // anything else used to read as false, and both false is a full
        // revocation — a trustline frozen by a typo, or by the XDR value 2.
        const auto flag = [&](const char* key) {
            const QVariant raw = op.value(QString::fromLatin1(key));
            if (presentButUnreadable(op, QString::fromLatin1(key)))
                throw std::runtime_error(QStringLiteral("%1 cannot be read")
                                             .arg(QString::fromLatin1(key)).toStdString());
            if (isMissing(raw))
                return false;
            if (raw.userType() == QMetaType::Bool)
                return raw.toBool();
            const QString text = exactText(raw, QString::fromLatin1(key)).trimmed().toLower();
            if (text == QLatin1String("true") || text == QLatin1String("1"))
                return true;
            if (text.isEmpty() || text == QLatin1String("false") || text == QLatin1String("0"))
                return false;
            throw std::runtime_error(QStringLiteral("%1 must be true or false")
                                         .arg(QString::fromLatin1(key)).toStdString());
        };
        // authorize is required, as js requires it: left out it read as
        // false, and with the box below unset that is a full revocation.
        if (!isSet(op, "authorize"))
            throw std::runtime_error("missing field: authorize (false revokes the trustline's "
                                     "authorization)");
        const bool authorize = flag("authorize");
        const bool maintain = flag("authorizeToMaintainLiabilities");
        QScopedPointer<KeyPair> trustor(KeyPair::fromAccountId(need(op, "trustor")));
        built = AllowTrustOperation::create(trustor.data(), need(op, "assetCode"),
                                            authorize, maintain);

    } else if (type == QLatin1String("clawback")) {
        built = ClawbackOperation::create(need(op, "from"),
                                          assets->assetFor(need(op, "asset")),
                                          positiveAmount(op, "amount"));

    } else if (type == QLatin1String("clawbackClaimableBalance")) {
        built = ClawbackClaimableBalanceOperation::create(need(op, "balanceId"));

    // ─── Offers and pools ────────────────────────────────────────────

    } else if (type == QLatin1String("manageSellOffer")) {
        built = new ManageSellOfferOperation(assets->assetFor(need(op, "selling")),
                                             assets->assetFor(need(op, "buying")),
                                             needAmount(op, "amount"),
                                             priceFrom(op, "price"),
                                             offerIdFrom(op));

    } else if (type == QLatin1String("manageBuyOffer")) {
        built = new ManageBuyOfferOperation(assets->assetFor(need(op, "selling")),
                                            assets->assetFor(need(op, "buying")),
                                            needAmount(op, "amount"),
                                            priceFrom(op, "price"),
                                            offerIdFrom(op));

    } else if (type == QLatin1String("createPassiveSellOffer")) {
        built = CreatePassiveSellOfferOperation::create(assets->assetFor(need(op, "selling")),
                                                        assets->assetFor(need(op, "buying")),
                                                        needAmount(op, "amount"),
                                                        priceFrom(op, "price"));

    } else if (type == QLatin1String("liquidityPoolDeposit")) {
        built = LiquidityPoolDepositOperation::create(hexId(op, "liquidityPoolId", 32),
                                                       positiveAmount(op, "maxAmountA"),
                                                       positiveAmount(op, "maxAmountB"),
                                                       priceFrom(op, "minPrice"),
                                                       priceFrom(op, "maxPrice"));

    } else if (type == QLatin1String("liquidityPoolWithdraw")) {
        built = LiquidityPoolWithdrawOperation::create(hexId(op, "liquidityPoolId", 32),
                                                        positiveAmount(op, "amount"),
                                                        needAmount(op, "minAmountA"),
                                                        needAmount(op, "minAmountB"));

    // ─── Claimable balances ──────────────────────────────────────────

    } else if (type == QLatin1String("createClaimableBalance")) {
        // The list owns its predicate trees and frees them when it goes out of
        // scope; create() copies each claimant into XDR.
        const QList<Claimant> claimants = claimantsFrom(op, "claimants");
        built = CreateClaimableBalanceOperation::create(positiveAmount(op, "amount"),
                                                         assets->assetFor(need(op, "asset")),
                                                         claimants);

    } else if (type == QLatin1String("claimClaimableBalance")) {
        built = ClaimClaimableBalanceOperation::create(need(op, "balanceId"));

    // ─── Account settings ────────────────────────────────────────────

    } else if (type == QLatin1String("setOptions")) {
        // Held in a scoped pointer while it is assembled: several setters throw
        // — a bad account id, a home domain over 32 bytes, a signer weight over
        // 255 — and nothing owns the operation until it is complete.
        QScopedPointer<SetOptionsOperation> options(SetOptionsOperation::create());

        if (isSet(op, "inflationDest")) {
            // The setter copies the XDR public key; the KeyPair stays ours.
            QScopedPointer<KeyPair> dest(KeyPair::fromAccountId(need(op, "inflationDest")));
            options->setInflationDestination(dest.data());
        }
        if (isSet(op, "clearFlags"))
            options->setClearFlags(unsignedNumber(op, "clearFlags", 0));
        if (isSet(op, "setFlags"))
            options->setSetFlags(unsignedNumber(op, "setFlags", 0));
        if (isSet(op, "masterWeight"))
            options->setMasterKeyWeight(weight(op, "masterWeight"));
        if (isSet(op, "lowThreshold"))
            options->setLowThreshold(weight(op, "lowThreshold"));
        if (isSet(op, "medThreshold"))
            options->setMediumThreshold(weight(op, "medThreshold"));
        if (isSet(op, "highThreshold"))
            options->setHighThreshold(weight(op, "highThreshold"));
        // Left out, undefined or null leaves it alone, like every other field
        // here; an explicit "" clears it. Undefined used to clear it.
        const QVariant homeDomain = op.value(QStringLiteral("homeDomain"));
        if (!isMissing(homeDomain))
            // As given: spaces are part of a home domain (core takes them),
            // and "  " trimmed to "" cleared the account's.
            options->setHomeDomain(exactText(homeDomain, QStringLiteral("homeDomain")));

        // signerKey and signerWeight are one optional signer in the XDR, so
        // they travel together. Weight 0 deletes the signer, which is why a
        // missing weight is refused rather than defaulted.
        if (!isSet(op, "signerKey") && isSet(op, "signerWeight"))
            throw std::runtime_error("signerKey is required when signerWeight is set");
        if (isSet(op, "signerKey")) {
            if (!isSet(op, "signerWeight"))
                throw std::runtime_error("signerWeight is required when signerKey is set");
            options->setSigner(SignerKey::fromString(need(op, "signerKey")).toXdr(),
                               weight(op, "signerWeight"));
        }
        built = options.take();

    } else if (type == QLatin1String("manageData")) {
        // As given, spaces included (core takes them, js and py sign them):
        // trimmed, " lead" set or deleted another entry, "lead".
        const QString name = exactText(op.value(QStringLiteral("name")),
                                       QStringLiteral("name"));
        if (name.isEmpty())
            throw std::runtime_error("missing field: name");
        if (!name.isValidUtf16())
            throw std::runtime_error("name is not valid text (an unpaired surrogate)");
        // The value is payload, signed as given — not trimmed, not reprinted.
        // No value deletes the entry, so an unreadable one must not count as
        // none.
        if (presentButUnreadable(op, QStringLiteral("value")))
            throw std::runtime_error("value must be text; leave it out to delete the entry");
        const QString value = exactText(op.value(QStringLiteral("value")),
                                        QStringLiteral("value"));
        // Decoded (and a bad valueFormat refused) even for a delete: a
        // format nobody reads would be one more thing ignored in silence.
        QByteArray bytes = dataValueBytes(op, value);
        // Only a value left out (or undefined, null) deletes the entry. An
        // empty one, in any format, is an empty value, as js and py sign it
        // and core keeps it; it used to delete the entry too.
        if (isMissing(op.value(QStringLiteral("value")))) {
            built = ManageDataOperation::create(name);
        } else {
            if (bytes.isNull())
                bytes = QByteArray("");
            built = ManageDataOperation::create(name, bytes);
        }

    } else if (type == QLatin1String("bumpSequence")) {
        // Required: left out, it used to bump to 0, a no-op nobody asked for.
        if (!isSet(op, "bumpTo"))
            throw std::runtime_error("missing field: bumpTo");
        // Not negative, as the field says: core refuses a negative bumpTo
        // (BUMP_SEQUENCE_BAD_SEQ). It was signed.
        const qint64 bumpTo = wholeNumber(op, "bumpTo", 0);
        if (bumpTo < 0)
            throw std::runtime_error("bumpTo must be from 0 to 9223372036854775807");
        built = BumpSequenceOperation::create(bumpTo);

    // ─── Sponsorship ─────────────────────────────────────────────────

    } else if (type == QLatin1String("beginSponsoringFutureReserves")) {
        built = BeginSponsoringFutureReservesOperation::create(need(op, "sponsoredId"));

    } else if (type == QLatin1String("endSponsoringFutureReserves")) {
        built = EndSponsoringFutureReservesOperation::create();

    } else if (type == QLatin1String("revokeAccountSponsorship")) {
        built = RevokeAccountSponsorshipOperation::create(need(op, "accountId"));

    } else if (type == QLatin1String("revokeTrustlineSponsorship")) {
        // A credit trustline by its asset, a pool-share one by its pool (as
        // js's LiquidityPoolId asset): exactly one. The pool could not be
        // named here.
        const bool byPool = isSet(op, "liquidityPoolId");
        if (byPool == isSet(op, "asset"))
            throw std::runtime_error("give exactly one of asset (a credit trustline) and "
                                     "liquidityPoolId (a pool-share trustline)");
        built = byPool ? RevokeTrustlineSponsorshipOperation::create(
                             need(op, "accountId"), hexId(op, "liquidityPoolId", 32))
                       : RevokeTrustlineSponsorshipOperation::create(
                             need(op, "accountId"), assets->assetFor(need(op, "asset")));

    } else if (type == QLatin1String("revokeOfferSponsorship")) {
        built = RevokeOfferSponsorshipOperation::create(need(op, "seller"),
                                                         offerIdFrom(op));

    } else if (type == QLatin1String("revokeDataSponsorship")) {
        built = RevokeDataSponsorshipOperation::create(need(op, "accountId"),
                                                        payloadText(op, "dataName"));

    } else if (type == QLatin1String("revokeClaimableBalanceSponsorship")) {
        built = RevokeClaimableBalanceSponsorshipOperation::create(need(op, "balanceId"));

    } else if (type == QLatin1String("revokeLiquidityPoolSponsorship")) {
        built = RevokeLiquidityPoolSponsorshipOperation::create(need(op, "liquidityPoolId"));

    } else if (type == QLatin1String("revokeSignerSponsorship")) {
        built = RevokeSignerSponsorshipOperation::create(
            need(op, "accountId"), SignerKey::fromString(need(op, "signerKey")).toXdr());

    // ─── Soroban state, and the one nobody uses ──────────────────────

    } else if (type == QLatin1String("invokeContract")) {
        built = InvokeHostFunctionOperation::invokeContractFunctionOperationBuilder(
            StellarAddress::fromString(need(op, "contractId")),
            payloadText(op, "functionName"),
            scValListFrom(op, "args"));

    } else if (type == QLatin1String("uploadContractWasm")) {
        const QByteArray wasm = strictHex(need(op, "wasmHex"), QStringLiteral("wasmHex"));
        if (wasm.isEmpty())
            throw std::runtime_error("the WASM bytes are written as hex");
        built = InvokeHostFunctionOperation::uploadContractWasmOperationBuilder(wasm);

    } else if (type == QLatin1String("createContract")) {
        // Always a v2 create (Protocol 22), with no constructor arguments
        // when none are given, as js (createCustomContract) and py
        // (append_create_contract_op) sign it. Without arguments it was a v1
        // create: another host function, another hash, and another auth tree
        // than the one a js or py deploy authorizes. Constructor arguments
        // used to be dropped without being read.
        built = InvokeHostFunctionOperation::createContractV2OperationBuilder(
            StellarAddress::fromString(need(op, "fromAddress")),
            hexId(op, "salt", 32),
            executableFromWasmHash(hexId(op, "wasmHash", 32)),
            scValListFrom(op, "constructorArgs"));

    } else if (type == QLatin1String("createContractFromAsset")) {
        // The Stellar Asset Contract: the bridge that lets a contract hold an
        // asset that existed before contracts did.
        Asset* asset = assets->assetFor(need(op, "asset"));
        if (!asset)
            throw std::runtime_error("that is not a valid asset");
        built = InvokeHostFunctionOperation::createContractFromAssetOperationBuilder(
            asset->toXdr());

    } else if (type == QLatin1String("extendFootprintTtl")) {
        // Required: a missing or blank ledger count used to build an extension
        // to ledger 0.
        if (!isSet(op, "extendTo"))
            throw std::runtime_error("missing field: extendTo");
        built = ExtendFootprintTTLOperation::create(unsignedNumber(op, "extendTo", 0));

    } else if (type == QLatin1String("restoreFootprint")) {
        built = RestoreFootprintOperation::create();

    } else if (type == QLatin1String("inflation")) {
        // Disabled on the network since 2019. Buildable because the ledger
        // still contains transactions that carry it.
        built = InflationOperation::create();

    } else {
        throw std::runtime_error(
            QStringLiteral("unsupported operation type: %1").arg(type).toStdString());
    }

    if (!built)
        throw std::runtime_error(
            QStringLiteral("could not build a %1 operation").arg(type).toStdString());

    // Every operation may name its own source account. Empty means the
    // transaction's own, which is what almost every operation wants.
    const QString source = str(op, "sourceAccount");
    if (!source.isEmpty())
        built->setSourceAccount(source);

    return built;
}

QSTELLAR_END_NS
