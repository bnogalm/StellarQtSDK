#include "sepsqml.h"
#include "../sep29.h"
#include "operationfactory.h"

#include <QCryptographicHash>
#include <QDataStream>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QScopedPointer>
#include <QSet>
#include <QStringList>
#include <QVariantList>
#include <algorithm>
#include <limits>
#include <memory>
#include <stdexcept>
#include <vector>

#include "accountmergeoperation.h"
#include "createaccountoperation.h"
#include "federation/federation.h"
#include "federation/federationresponse.h"
#include "exception/sdkexception.h"
#include "exception/networkexception.h"
#include "feebumptransaction.h"
#include "keypair.h"
#include "memo.h"
#include "messageauth.h"
#include "muxedaccount.h"
#include "network.h"
#include "operation.h"
#include "pathpaymentoperation.h"
#include "pathpaymentstrictreceiveoperation.h"
#include "pathpaymentstrictsendoperation.h"
#include "paymentoperation.h"
#include "scval/scv.h"
#include "sep10challenge.h"
#include "sep45challenge.h"
#include "sep7.h"
#include "slip10.h"
#include "stellartoml.h"
#include "strkey.h"
#include "timebounds.h"
#include "toid.h"
#include "transaction.h"
#include "txrep.h"
#include "util.h"

QSTELLAR_BEGIN_NS

namespace {

/** Runs `fn` and reports failure the way QML can consume: a map with "error". */
template <typename Fn>
QVariantMap guarded(Fn fn)
{
    try {
        return fn();
    } catch (const std::exception& e) {
        QVariantMap out;
        out.insert(QStringLiteral("error"), QString::fromUtf8(e.what()));
        return out;
    } catch (...) {
        QVariantMap out;
        out.insert(QStringLiteral("error"), QStringLiteral("unknown failure"));
        return out;
    }
}

QString arg(const QVariantMap& m, const char* key)
{
    return m.value(QString::fromLatin1(key)).toString().trimmed();
}

/**
 * The home domains a SEP-10 call accepts: `homeDomains` as a list, or a single
 * `homeDomain`. Blank entries are dropped, so an empty text field in a list
 * cannot match a challenge whose key is just " auth".
 */
QStringList homeDomainsFrom(const QVariantMap& args)
{
    QStringList domains;
    const QVariant given = args.value(QStringLiteral("homeDomains"));
    // A plain string is one domain, as js-stellar-sdk takes it. Converting it
    // as a list split it into characters on Qt 6 (and dropped it on Qt 5), so
    // "example.com" matched a challenge keyed "e auth".
    QStringList listed;
    if (given.userType() == QMetaType::QString)
        listed.append(given.toString());
    else if (given.userType() == QMetaType::QStringList)
        listed = given.toStringList();
    else if (given.userType() == QMetaType::QVariantList)
        listed = QVariant(given.toList()).toStringList();
    for (const QString& d : listed) {
        if (!d.trimmed().isEmpty())
            domains.append(d.trimmed());
    }
    if (domains.isEmpty() && !arg(args, "homeDomain").isEmpty())
        domains.append(arg(args, "homeDomain"));
    return domains;
}

/**
 * The keys `alreadySignedBy` names: a list, or one key as a string. Each must
 * be a G address; blanks are dropped and repeats counted once.
 */
QStringList signerKeysFrom(const QVariantMap& args, const char* key)
{
    const QVariant given = args.value(QString::fromLatin1(key));
    QStringList listed;
    const bool single = given.userType() == QMetaType::QString;
    if (single)
        listed.append(given.toString());
    else if (given.userType() == QMetaType::QStringList)
        listed = given.toStringList();
    else if (given.userType() == QMetaType::QVariantList)
        listed = QVariant(given.toList()).toStringList();
    else if (!isMissing(given))
        throw std::runtime_error(QStringLiteral("%1 is a list of account ids")
                                     .arg(QString::fromLatin1(key)).toStdString());
    QStringList keys;
    for (int i = 0; i < listed.size(); ++i) {
        const QString k = listed.at(i).trimmed();
        if (k.isEmpty() || keys.contains(k))
            continue;
        bool valid = false;
        try {
            valid = StrKey::decodeVersionByte(k) == StrKey::VersionByte::ACCOUNT_ID
                    && StrKey::decodeStellarAccountId(k).size() == 32;
        } catch (const std::exception&) {}
        // Named by its place, never quoted: what is not an account id may be
        // a secret seed pasted into the wrong field, and errors get shown and
        // logged.
        if (!valid)
            throw std::runtime_error((single ? QStringLiteral("%1 is not an account id (G...)")
                                                   .arg(QString::fromLatin1(key))
                                             : QStringLiteral("%1[%2] is not an account id (G...)")
                                                   .arg(QString::fromLatin1(key)).arg(i))
                                         .toStdString());
        keys.append(k);
    }
    return keys;
}

/** Whether `key` has signed `tx` already. */
bool hasSigned(Transaction* tx, KeyPair* key)
{
    const stellar::SignatureHint hint = key->getSignatureHint();
    const QByteArray hash = tx->hash();
    for (const stellar::DecoratedSignature& existing : tx->getSignatures()) {
        if (existing.hint == hint && key->verify(hash, existing.signature.binary()))
            return true;
    }
    return false;
}

/**
 * Refuses a challenge unless it carries exactly one signature by each of
 * `keys`, the server's first, and nothing else. A signature the server cannot
 * attribute makes it refuse the challenge once the client has signed it too,
 * so it is refused here, before a key touches it.
 */
void requireSignaturesOfExactly(Transaction* tx, const QStringList& keys)
{
    const QByteArray hash = tx->hash();
    std::vector<std::unique_ptr<KeyPair>> pairs;
    for (const QString& k : keys)
        pairs.emplace_back(KeyPair::fromAccountId(k));
    std::vector<bool> used(pairs.size(), false);
    for (const stellar::DecoratedSignature& existing : tx->getSignatures()) {
        const QByteArray signature = existing.signature.binary();
        bool matched = false;
        for (size_t i = 0; i < pairs.size() && !matched; ++i) {
            if (!used[i] && existing.hint == pairs[i]->getSignatureHint()
                    && pairs[i]->verify(hash, signature)) {
                used[i] = true;
                matched = true;
            }
        }
        if (!matched)
            throw std::runtime_error(keys.size() > 1
                ? "this challenge carries a signature that is neither the server's nor one "
                  "alreadySignedBy names: the server would refuse it"
                : "this challenge carries a signature that is not the server's: the server "
                  "would refuse it (name the keys that signed it before in alreadySignedBy)");
    }
    for (size_t i = 1; i < pairs.size(); ++i) {
        if (!used[i])
            throw std::runtime_error(QStringLiteral("alreadySignedBy names %1, which has not "
                                                    "signed this challenge").arg(keys.at(int(i)))
                                         .toStdString());
    }
}

/**
 * A client domain is expected with its toml's SIGNING_KEY or not at all:
 * either alone cannot say whose client_domain operation to accept.
 */
void requireClientDomainPair(const QString& clientDomain, const QString& signingKey)
{
    if (!clientDomain.isEmpty() && signingKey.isEmpty())
        throw std::runtime_error("clientDomainSigningKey is required with clientDomain: the "
                                 "SIGNING_KEY of that domain's stellar.toml");
    if (clientDomain.isEmpty() && !signingKey.isEmpty())
        throw std::runtime_error("clientDomain is required with clientDomainSigningKey");
}

/** The client account, and for a muxed one the G whose signers log it in. */
void putClientAccount(QVariantMap& out, const QString& clientAccountId)
{
    out.insert(QStringLiteral("clientAccountId"), clientAccountId);
    if (clientAccountId.startsWith(QLatin1Char('M'))) {
        const MuxedAccount muxed = MuxedAccount::fromAddress(clientAccountId);
        out.insert(QStringLiteral("clientBaseAccountId"), muxed.getAccountId());
        out.insert(QStringLiteral("clientMuxedId"), QString::number(muxed.getMuxedId()));
    }
}

/**
 * The challenge's client_domain operation, shown whether or not it was
 * checked: the key it names is one the client's signature vouches for.
 */
void putClientDomain(QVariantMap& out, const Sep10Challenge::ChallengeTransaction& read,
                     bool checked)
{
    if (read.getClientDomainSigningKey().isEmpty())
        return;
    out.insert(QStringLiteral("clientDomain"), read.getClientDomain());
    out.insert(QStringLiteral("clientDomainSigningKey"), read.getClientDomainSigningKey());
    out.insert(QStringLiteral("clientDomainChecked"), checked);
}

/** Only sets a field when the caller gave one, so empty stays absent. */
void assignIf(QString& target, const QVariantMap& m, const char* key)
{
    const QString value = arg(m, key);
    if (!value.isEmpty())
        target = value;
}

void put(QVariantMap& out, const char* key, const QString& value)
{
    if (!value.isEmpty())
        out.insert(QString::fromLatin1(key), value);
}

/**
 * The network a call should run against.
 *
 * Accepts either a passphrase or the short name the lab uses everywhere else.
 * A custom passphrase is cached because Network has no value semantics here and
 * building a fresh one per call would leak one object per keystroke.
 */
Network* networkFrom(const QVariantMap& spec)
{
    return qmlNetwork(arg(spec, "network"), arg(spec, "networkPassphrase"));
}

/**
 * A message as bytes.
 *
 * SEP-53 hashes BYTES, not text, and the published vectors include messages
 * that are not valid UTF-8 at all. A page that could only sign what somebody
 * typed would be unable to reproduce half of them, so the encoding is part of
 * the input rather than an assumption.
 */
/**
 * A challenge's lifetime in seconds: 1 to 86400, read exactly, 300 when left
 * out. QVariant::toLongLong rounded 300.7 to 301, read true as 1 and, on Qt 6,
 * Infinity or 1e300 as INT64_MAX — a challenge that never expires, built by
 * overflowing now + timeout.
 */
qint64 challengeTimeout(const QVariantMap& args)
{
    if (args.contains(QStringLiteral("timeoutSeconds")) && args.contains(QStringLiteral("timeoutSec")))
        throw std::runtime_error("give timeoutSeconds or timeoutSec, not both");
    const QString key = args.contains(QStringLiteral("timeoutSeconds"))
                        ? QStringLiteral("timeoutSeconds") : QStringLiteral("timeoutSec");
    const QVariant raw = args.value(key);
    if (args.contains(key) && !raw.isValid())
        throw std::runtime_error(QStringLiteral("%1 cannot be read; leave it out, or give a value")
                                     .arg(key).toStdString());
    if (isMissing(raw) || (raw.userType() == QMetaType::QString && raw.toString().trimmed().isEmpty()))
        return 300;
    if (raw.userType() == QMetaType::Bool)
        throw std::runtime_error(QStringLiteral("%1 must be a whole number of seconds")
                                     .arg(key).toStdString());
    return exactSigned(raw, key, 1, 86400);
}

QByteArray messageBytes(const QString& message, const QString& encoding)
{
    const QString how = encoding.trimmed().toLower();
    if (!how.isEmpty() && how != QLatin1String("text") && how != QLatin1String("utf8")
            && how != QLatin1String("utf-8") && how != QLatin1String("hex")
            && how != QLatin1String("base64"))
        throw std::runtime_error("the encoding is text, hex or base64");
    if (how == QLatin1String("hex")) {
        // Strictly: the bytes are what gets signed, and fromHex skipped what it
        // did not understand ("0xab" signed 00 ab). Whitespace between the
        // bytes, as hex is usually pasted, separates them and nothing else.
        QString compact;
        for (const QChar c : message)
            if (!c.isSpace())
                compact.append(c);
        return strictHex(compact, QStringLiteral("the message"));
    }
    if (how == QLatin1String("base64")) {
        const auto decoded = QByteArray::fromBase64Encoding(
            message.toLatin1(), QByteArray::AbortOnBase64DecodingErrors);
        if (!decoded)
            throw std::runtime_error("not valid base64");
        return *decoded;
    }
    return message.toUtf8();
}

/** Flattens the nested map a toml parses into, so a page can render it whole. */
void flatten(const QVariant& value, const QString& path, QVariantList& rows)
{
    const char* kind = "string";

    if (value.userType() == QMetaType::QVariantMap) {
        const QVariantMap table = value.toMap();
        if (table.isEmpty() && !path.isEmpty()) {
            // An empty table is a fact about the file. Emitting only leaves
            // made it vanish, so a reader could not tell a `[SECTION]` with
            // nothing under it from one that was never there.
            QVariantMap row;
            row.insert(QStringLiteral("path"), path);
            row.insert(QStringLiteral("value"), QStringLiteral("(empty table)"));
            row.insert(QStringLiteral("kind"), QStringLiteral("table"));
            rows.append(row);
            return;
        }
        for (auto it = table.constBegin(); it != table.constEnd(); ++it) {
            // A key that contains a dot is QUOTED: `"a.b" = 1` is one key, and
            // written bare its path would read as the nested table `[a]`,
            // key `b` (which StellarToml reads as such since 1.0.4).
            const QString segment = it.key().contains(QLatin1Char('.'))
                                    ? QLatin1Char('"') + it.key() + QLatin1Char('"')
                                    : it.key();
            flatten(it.value(),
                    path.isEmpty() ? segment : path + QLatin1Char('.') + segment,
                    rows);
        }
        return;
    }
    if (value.userType() == QMetaType::QVariantList) {
        const QVariantList items = value.toList();
        if (items.isEmpty() && !path.isEmpty()) {
            QVariantMap row;
            row.insert(QStringLiteral("path"), path);
            row.insert(QStringLiteral("value"), QStringLiteral("(empty array)"));
            row.insert(QStringLiteral("kind"), QStringLiteral("array"));
            rows.append(row);
            return;
        }
        for (int i = 0; i < items.size(); ++i) {
            flatten(items.at(i),
                    QStringLiteral("%1[%2]").arg(path).arg(i),
                    rows);
        }
        return;
    }
    if (value.userType() == QMetaType::Bool)
        kind = "bool";
    else if (value.userType() == QMetaType::LongLong || value.userType() == QMetaType::Int)
        kind = "int";
    else if (value.userType() == QMetaType::Double)
        kind = "float";

    QVariantMap row;
    row.insert(QStringLiteral("path"), path);
    row.insert(QStringLiteral("value"), value.toString());
    row.insert(QStringLiteral("kind"), QString::fromLatin1(kind));
    rows.append(row);
}

/** Digs a dotted path out of the nested map, for the keys that identify somebody. */
QString tomlValue(const QVariantMap& values, const QString& key)
{
    return values.value(key).toString();
}

const char* versionByteName(StrKey::VersionByte byte)
{
    switch (byte) {
    case StrKey::VersionByte::ACCOUNT_ID:        return "ACCOUNT_ID";
    case StrKey::VersionByte::MUXED_ACCOUNT:     return "MUXED_ACCOUNT";
    case StrKey::VersionByte::SEED:              return "SEED";
    case StrKey::VersionByte::PRE_AUTH_TX:       return "PRE_AUTH_TX";
    case StrKey::VersionByte::SHA256_HASH:       return "SHA256_HASH";
    case StrKey::VersionByte::SIGNED_PAYLOAD:    return "SIGNED_PAYLOAD";
    case StrKey::VersionByte::CONTRACT:          return "CONTRACT";
    case StrKey::VersionByte::CLAIMABLE_BALANCE: return "CLAIMABLE_BALANCE";
    case StrKey::VersionByte::LIQUIDITY_POOL:    return "LIQUIDITY_POOL";
    }
    return "unknown";
}

/** One row of the strkey alphabet: what a prefix means and how long it runs. */
QVariantMap kindRow(const char* kind, const char* prefix,
                    StrKey::VersionByte byte, int length, int bodyBytes,
                    const char* label, bool supported = true)
{
    QVariantMap row;
    // Whether StellarQml::strKeyKind() names this one (all nine since 1.0.4).
    row.insert(QStringLiteral("supported"), supported);
    row.insert(QStringLiteral("kind"), QString::fromLatin1(kind));
    row.insert(QStringLiteral("prefix"), QString::fromLatin1(prefix));
    row.insert(QStringLiteral("versionByte"), int(byte));
    row.insert(QStringLiteral("length"), length);
    row.insert(QStringLiteral("bodyBytes"), bodyBytes);
    row.insert(QStringLiteral("label"), QString::fromLatin1(label));
    return row;
}

} // namespace

SepsQml::SepsQml(QObject* parent) : QObject(parent) {}

SepsQml::~SepsQml() = default;

// ─── SEP-1 ───────────────────────────────────────────────────────────

namespace {

/** The value as JavaScript can hold it: an integer past ±2^53 as its text. */
QVariant forJavaScript(const QVariant& value)
{
    if (value.userType() == QMetaType::LongLong) {
        const qlonglong number = value.toLongLong();
        const qlonglong exact = Q_INT64_C(9007199254740992);   // 2^53
        if (number > exact || number < -exact)
            return QString::number(number);
        return value;
    }
    if (value.userType() == QMetaType::QVariantMap) {
        QVariantMap map = value.toMap();
        for (auto it = map.begin(); it != map.end(); ++it)
            it.value() = forJavaScript(it.value());
        return map;
    }
    if (value.userType() == QMetaType::QVariantList) {
        QVariantList list = value.toList();
        for (QVariant& item : list)
            item = forJavaScript(item);
        return list;
    }
    return value;
}

} // namespace

QVariantMap SepsQml::parseToml(const QString& text) const
{
    return guarded([&]() {
        QVariantMap out;
        out.insert(QStringLiteral("values"), forJavaScript(StellarToml::parse(text)));
        return out;
    });
}

QStringList SepsQml::wellKnownTomlKeys() const
{
    // The handful a wallet actually reads. VERSION and NETWORK_PASSPHRASE say
    // what you are looking at; SIGNING_KEY is the one that matters for SEP-10,
    // and URI_REQUEST_SIGNING_KEY for verifying a SEP-7 link.
    return { QStringLiteral("VERSION"),
             QStringLiteral("NETWORK_PASSPHRASE"),
             QStringLiteral("FEDERATION_SERVER"),
             QStringLiteral("WEB_AUTH_ENDPOINT"),
             QStringLiteral("SIGNING_KEY"),
             QStringLiteral("URI_REQUEST_SIGNING_KEY"),
             QStringLiteral("TRANSFER_SERVER"),
             QStringLiteral("HORIZON_URL") };
}

QVariantMap SepsQml::parseTomlRows(const QString& text) const
{
    return guarded([&]() {
        QVariantList rows;
        flatten(StellarToml::parse(text), QString(), rows);
        QVariantMap out;
        out.insert(QStringLiteral("rows"), rows);
        out.insert(QStringLiteral("count"), rows.size());
        return out;
    });
}

QVariantMap SepsQml::tomlSigningKeys(const QString& text) const
{
    return guarded([&]() {
        const QVariantMap values = StellarToml::parse(text);
        // The two keys are checked: they are what a SEP-10 challenge and a
        // SEP-7 request are verified against, and were handed on as whatever
        // text the toml held (a number, a secret seed, a damaged key).
        for (const char* name : {"SIGNING_KEY", "URI_REQUEST_SIGNING_KEY"}) {
            const QVariant value = values.value(QString::fromLatin1(name));
            if (!value.isValid())
                continue;
            const QString key = value.toString();
            bool account = false;
            if (value.userType() == QMetaType::QString) {
                try {
                    account = StrKey::decodeVersionByte(key) == StrKey::VersionByte::ACCOUNT_ID
                              && StrKey::encodeStellarAccountId(StrKey::decodeStellarAccountId(key)) == key;
                } catch (const std::exception&) {
                }
            }
            if (!account) {
                throw std::runtime_error(std::string("this stellar.toml's ") + name
                                         + " is not an account id (G...)");
            }
        }
        QVariantMap out;
        put(out, "signingKey", tomlValue(values, QStringLiteral("SIGNING_KEY")));
        put(out, "uriRequestSigningKey",
            tomlValue(values, QStringLiteral("URI_REQUEST_SIGNING_KEY")));
        put(out, "networkPassphrase",
            tomlValue(values, QStringLiteral("NETWORK_PASSPHRASE")));
        put(out, "webAuthEndpoint",
            tomlValue(values, QStringLiteral("WEB_AUTH_ENDPOINT")));
        put(out, "federationServer",
            tomlValue(values, QStringLiteral("FEDERATION_SERVER")));
        return out;
    });
}

// ─── SEP-7 ───────────────────────────────────────────────────────────

QVariantMap SepsQml::buildPayUri(const QVariantMap& args) const
{
    return guarded([&]() {
        // Only the fields this reads. SEP-7's own names (asset_code, memo_type)
        // used to be ignored: the request then asked for XLM with no memo.
        refuseUnknownArgs(args, {"destination", "amount", "assetCode", "assetIssuer", "memo",
                                 "memoType", "callback", "msg", "networkPassphrase", "originDomain"},
                          QStringLiteral("buildPayUri"));
        const QString destination = arg(args, "destination");
        if (destination.isEmpty())
            throw std::runtime_error("a payment request needs a destination");

        //! [sepsBuildUri.cpp]
        Sep7Pay pay(destination);
        assignIf(pay.amount, args, "amount");
        //! [sepsBuildUri.cpp]
        assignIf(pay.assetCode, args, "assetCode");
        assignIf(pay.assetIssuer, args, "assetIssuer");
        assignIf(pay.memo, args, "memo");
        assignIf(pay.memoType, args, "memoType");
        assignIf(pay.callback, args, "callback");
        assignIf(pay.msg, args, "msg");
        assignIf(pay.networkPassphrase, args, "networkPassphrase");
        assignIf(pay.originDomain, args, "originDomain");

        QVariantMap out;
        out.insert(QStringLiteral("uri"), pay.toUri());
        return out;
    });
}

QVariantMap SepsQml::buildTxUri(const QVariantMap& args) const
{
    return guarded([&]() {
        refuseUnknownArgs(args, {"xdr", "callback", "pubkey", "msg", "networkPassphrase",
                                 "originDomain", "replace"},
                          QStringLiteral("buildTxUri"));
        const QString xdr = arg(args, "xdr");
        if (xdr.isEmpty())
            throw std::runtime_error("a transaction request needs an envelope");

        Sep7Tx tx(xdr);
        assignIf(tx.callback, args, "callback");
        assignIf(tx.pubkey, args, "pubkey");
        assignIf(tx.msg, args, "msg");
        assignIf(tx.networkPassphrase, args, "networkPassphrase");
        assignIf(tx.originDomain, args, "originDomain");

        for (const QVariant& entry : args.value(QStringLiteral("replace")).toList()) {
            const QVariantMap row = entry.toMap();
            Sep7Replacement replacement;
            replacement.field = arg(row, "field");
            replacement.reference = arg(row, "reference");
            replacement.hint = arg(row, "hint");
            if (replacement.field.isEmpty() || replacement.reference.isEmpty())
                throw std::runtime_error("a replacement needs both a field and "
                                         "a reference naming it");
            tx.replace.append(replacement);
        }

        QVariantMap out;
        out.insert(QStringLiteral("uri"), tx.toUri());
        return out;
    });
}

QVariantMap SepsQml::parseUri(const QString& uri) const
{
    return guarded([&]() {
        QVariantMap out;
        // The path decides which of the two shapes it is, and the SDK refuses
        // anything else outright.
        if (uri.contains(QLatin1String("web+stellar:tx"))) {
            const Sep7Tx tx = Sep7Tx::fromUri(uri);
            out.insert(QStringLiteral("kind"), QStringLiteral("tx"));
            put(out, "xdr", tx.xdr);
            put(out, "callback", tx.callback);
            put(out, "pubkey", tx.pubkey);
            put(out, "msg", tx.msg);
            put(out, "networkPassphrase", tx.networkPassphrase);
            put(out, "originDomain", tx.originDomain);
            put(out, "signature", tx.signature);

            QVariantList replacements;
            for (const Sep7Replacement& r : tx.replace) {
                QVariantMap entry;
                entry.insert(QStringLiteral("field"), r.field);
                entry.insert(QStringLiteral("reference"), r.reference);
                entry.insert(QStringLiteral("hint"), r.hint);
                replacements.append(entry);
            }
            if (!replacements.isEmpty())
                out.insert(QStringLiteral("replace"), replacements);
        } else {
            const Sep7Pay pay = Sep7Pay::fromUri(uri);
            out.insert(QStringLiteral("kind"), QStringLiteral("pay"));
            put(out, "destination", pay.destination);
            put(out, "amount", pay.amount);
            put(out, "assetCode", pay.assetCode);
            put(out, "assetIssuer", pay.assetIssuer);
            put(out, "memo", pay.memo);
            put(out, "memoType", pay.memoType);
            put(out, "callback", pay.callback);
            put(out, "msg", pay.msg);
            put(out, "networkPassphrase", pay.networkPassphrase);
            put(out, "originDomain", pay.originDomain);
            put(out, "signature", pay.signature);
        }
        return out;
    });
}

QVariantMap SepsQml::signUri(const QString& uri, const QString& secretSeed) const
{
    return guarded([&]() {
        QScopedPointer<KeyPair> signer(KeyPair::fromSecretSeed(secretSeed.trimmed()));

        QVariantMap out;
        if (uri.contains(QLatin1String("web+stellar:tx"))) {
            Sep7Tx tx = Sep7Tx::fromUri(uri);
            tx.sign(signer.data());
            out.insert(QStringLiteral("uri"), tx.toUri());
            out.insert(QStringLiteral("signature"), tx.signature);
        } else {
            Sep7Pay pay = Sep7Pay::fromUri(uri);
            pay.sign(signer.data());
            out.insert(QStringLiteral("uri"), pay.toUri());
            out.insert(QStringLiteral("signature"), pay.signature);
        }
        // The signer is ours to free: sign() reads the key and takes nothing.
        return out;
    });
}

QVariantMap SepsQml::verifyUri(const QString& uri, const QString& accountId) const
{
    return guarded([&]() {
        QScopedPointer<KeyPair> key(KeyPair::fromAccountId(accountId.trimmed()));

        QVariantMap out;
        // A false is an answer, not a failure: the URI parsed, the signature
        // simply is not the one this account would have made.
        if (uri.contains(QLatin1String("web+stellar:tx"))) {
            const Sep7Tx tx = Sep7Tx::fromUri(uri);
            out.insert(QStringLiteral("valid"), tx.verify(key.data()));
            out.insert(QStringLiteral("signed"), !tx.signature.isEmpty());
        } else {
            const Sep7Pay pay = Sep7Pay::fromUri(uri);
            out.insert(QStringLiteral("valid"), pay.verify(key.data()));
            out.insert(QStringLiteral("signed"), !pay.signature.isEmpty());
        }
        return out;
    });
}

QVariantMap SepsQml::verifyUriAgainstToml(const QString& uri,
                                          const QString& tomlText) const
{
    return guarded([&]() {
        const QVariantMap values = StellarToml::parse(tomlText);
        if (!values.contains(QStringLiteral("URI_REQUEST_SIGNING_KEY"))) {
            QVariantMap missing;
            missing.insert(QStringLiteral("hasUriRequestSigningKey"), false);
            missing.insert(QStringLiteral("valid"), false);
            missing.insert(QStringLiteral("fetched"), false);
            missing.insert(QStringLiteral("error"),
                           QStringLiteral("this stellar.toml publishes no "
                                          "URI_REQUEST_SIGNING_KEY, so there is "
                                          "nothing to check the signature against"));
            return missing;
        }

        // The check Sep7OriginCheck runs on the toml it fetches. The key was
        // trimmed and read as any text here, so a padded one passed that
        // tomlSigningKeys and the C++ check refuse, and so did a URI with no
        // origin_domain. A refusal is the { error } saying which check failed.
        const QString key = Sep7::verifyOrigin(uri, values);

        QVariantMap out;
        out.insert(QStringLiteral("hasUriRequestSigningKey"), true);
        out.insert(QStringLiteral("valid"), true);
        out.insert(QStringLiteral("signed"), true);
        out.insert(QStringLiteral("keyUsed"), key);
        // A wallet reads the toml from the URI's own origin_domain, as
        // Sep7OriginCheck does; here it was pasted, so nothing ties the key
        // to that domain.
        out.insert(QStringLiteral("fetched"), false);
        out.insert(QStringLiteral("originDomain"), Sep7::originDomain(uri));
        return out;
    });
}

// ─── SEP-5 ───────────────────────────────────────────────────────────

QVariantMap SepsQml::inspectMnemonic(const QString& mnemonic) const
{
    return guarded([&]() {
        const QString phrase = mnemonic.simplified();
        const QStringList words = phrase.isEmpty()
                                  ? QStringList()
                                  : phrase.split(QLatin1Char(' '));

        QVariantMap out;
        out.insert(QStringLiteral("wordCount"), words.size());

        // A bare true/false cannot tell a typo from a wrong length from a
        // correct-looking phrase whose checksum fails, and those have entirely
        // different fixes. Work out which one it is.
        static const QSet<QString> vocabulary = []() {
            QSet<QString> set;
            const QList<QString> list = Util::mnemonicWords();
            for (const QString& word : list)
                set.insert(word);
            return set;
        }();

        QVariantList unknown;
        for (const QString& word : words)
            // NOT toLower(). Util::validateMnemonic and mnemonicToBIP39Seed
            // both compare against the lowercase list case-sensitively, so a
            // capitalised word really is unusable — lowercasing here declared
            // every word known and then blamed the checksum, which sends
            // somebody hunting for the wrong mistake.
            if (!vocabulary.contains(word))
                unknown.append(word);
        out.insert(QStringLiteral("unknownWords"), unknown);

        const bool valid = !words.isEmpty() && Util::validateMnemonic(phrase);
        out.insert(QStringLiteral("valid"), valid);

        QString reason;
        if (words.isEmpty())
            reason = QStringLiteral("nothing typed yet");
        else if (!unknown.isEmpty())
            reason = QStringLiteral("%1 word(s) are not in the BIP-39 list")
                         .arg(unknown.size());
        else if (words.size() % 3 != 0 || words.size() < 12 || words.size() > 24)
            reason = QStringLiteral("%1 words \u2014 a phrase is 12, 15, 18, 21 or 24")
                         .arg(words.size());
        else if (!valid)
            reason = QStringLiteral("every word is real and the length is right, "
                                    "so the checksum failed \u2014 usually two words "
                                    "in the wrong order");
        else
            reason = QStringLiteral("valid");
        out.insert(QStringLiteral("reason"), reason);
        return out;
    });
}

namespace {

/**
 * deriveAccountRange and deriveAccountRangeLegacyPassphrase: the same range,
 * from the BIP-39 seed or from the one 1.0.0-1.0.3 derived with the
 * passphrase as typed.
 */
QVariantMap accountRange(const QString& mnemonic, const QVariant& firstIndex, const QVariant& howMany,
                         const QString& passphrase, bool legacyPassphrase)
{
    // Read exactly: the QML engine filled an int with ToInt32, so 2^32 + 100
    // was taken as 100 and 1.5 as 1.
    const qint64 count = exactSigned(howMany, QStringLiteral("count"), std::numeric_limits<qint64>::min(),
                                     std::numeric_limits<qint64>::max());
    const qint64 first = exactSigned(firstIndex, QStringLiteral("first"), std::numeric_limits<qint64>::min(),
                                     std::numeric_limits<qint64>::max());
    if (count < 1 || count > 20)
        throw std::runtime_error("ask for between 1 and 20 accounts");
    if (first < 0)
        throw std::runtime_error("the first index cannot be negative");
    // 2^31 - 1 is the last account: the index is hardened. A range past it
    // overflowed the int, and index -2^31, its top bit set, was derived as
    // an already hardened 0: account 0's keys, shown as another account.
    if (first > std::numeric_limits<int>::max() - (count - 1))
        throw std::runtime_error("the last index is 2147483647 (2^31 - 1)");
    // A phrase whose checksum fails is a typo, most often two words swapped:
    // derived anyway it gives a valid-looking wallet that is not the user's.
    // py-stellar-sdk and js (stellar-hd-wallet) refuse it, and so does
    // Stellar.keyPairFromMnemonic; inspectMnemonic() says why.
    if (!Util::validateMnemonic(mnemonic.simplified()))
        throw std::runtime_error("invalid mnemonic phrase (inspectMnemonic says why)");

    const QByteArray seed = legacyPassphrase
        ? Util::mnemonicToBIP39SeedLegacyPassphrase(mnemonic.simplified(), passphrase)
        : Util::mnemonicToBIP39Seed(mnemonic.simplified(), passphrase);
    QVariantList accounts;
    for (int i = 0; i < int(count); ++i) {
        const int index = int(first) + i;
        QScopedPointer<KeyPair> key(KeyPair::fromBip39Seed(seed, index));

        QVariantMap entry;
        entry.insert(QStringLiteral("index"), index);
        // Hardened all the way down, which is why there is no way to derive
        // a public key here without the secret: the apostrophes are not
        // decoration.
        entry.insert(QStringLiteral("path"),
                     QStringLiteral("m/44'/148'/%1'").arg(index));
        entry.insert(QStringLiteral("accountId"), key->getAccountId());
        entry.insert(QStringLiteral("secretSeed"), key->getSecretSeed());
        accounts.append(entry);
    }

    QVariantMap out;
    out.insert(QStringLiteral("bip39SeedHex"), QString::fromLatin1(seed.toHex()));
    out.insert(QStringLiteral("accounts"), accounts);
    out.insert(QStringLiteral("passphraseUsed"), !passphrase.isEmpty());
    // True when a wallet made with this passphrase by 1.0.3 or earlier is at
    // other accounts, which deriveAccountRangeLegacyPassphrase finds.
    out.insert(QStringLiteral("legacyPassphraseDiffers"),
               Util::legacyPassphraseDerivesDifferently(passphrase));
    if (legacyPassphrase)
        out.insert(QStringLiteral("legacy"), true);
    return out;
}

} // namespace

QVariantMap SepsQml::deriveAccountRange(const QString& mnemonic, const QVariant& first,
                                        const QVariant& count, const QString& passphrase) const
{
    return guarded([&]() {
        return accountRange(mnemonic, first, count, passphrase, false);
    });
}

QVariantMap SepsQml::deriveAccountRangeLegacyPassphrase(const QString& mnemonic, const QVariant& first,
                                                        const QVariant& count, const QString& passphrase) const
{
    // The passphrase as typed: what 1.0.0-1.0.3 derived, not BIP-39.
    return guarded([&]() {
        return accountRange(mnemonic, first, count, passphrase, true);
    });
}

// ─── SEP-53 ──────────────────────────────────────────────────────────

QVariantMap SepsQml::signMessage(const QVariantMap& args) const
{
    return guarded([&]() {
        refuseUnknownArgs(args, {"message", "encoding", "secretSeed"}, QStringLiteral("signMessage"));
        const QByteArray message = messageBytes(args.value(QStringLiteral("message")).toString(),
                                                arg(args, "encoding"));
        QScopedPointer<KeyPair> key(KeyPair::fromSecretSeed(arg(args, "secretSeed")));
        const QByteArray signature = MessageAuth::signMessage(message, key.data());

        QVariantMap out;
        out.insert(QStringLiteral("accountId"), key->getAccountId());
        out.insert(QStringLiteral("hash"),
                   QString::fromLatin1(MessageAuth::messageHash(message).toHex()));
        out.insert(QStringLiteral("signature"),
                   QString::fromLatin1(signature.toBase64()));
        out.insert(QStringLiteral("signatureHex"),
                   QString::fromLatin1(signature.toHex()));
        out.insert(QStringLiteral("messageBytes"), message.size());
        out.insert(QStringLiteral("messageHex"), QString::fromLatin1(message.toHex()));
        return out;
    });
}

QVariantMap SepsQml::verifyMessage(const QVariantMap& args) const
{
    return guarded([&]() {
        refuseUnknownArgs(args, {"message", "encoding", "signature", "accountId"},
                          QStringLiteral("verifyMessage"));
        const QByteArray message = messageBytes(args.value(QStringLiteral("message")).toString(),
                                                arg(args, "encoding"));
        const auto decoded = QByteArray::fromBase64Encoding(
            arg(args, "signature").toLatin1(),
            QByteArray::AbortOnBase64DecodingErrors);
        if (!decoded)
            throw std::runtime_error("the signature is not valid base64");

        QVariantMap out;
        // A signature that does not match is an ANSWER. Only an unreadable
        // input is an error.
        out.insert(QStringLiteral("valid"),
                   MessageAuth::verifyMessage(arg(args, "accountId"), message, *decoded));
        out.insert(QStringLiteral("hash"),
                   QString::fromLatin1(MessageAuth::messageHash(message).toHex()));
        out.insert(QStringLiteral("signatureBytes"), (*decoded).size());
        out.insert(QStringLiteral("messageBytes"), message.size());
        return out;
    });
}

QVariantMap SepsQml::messageHash(const QString& message, const QString& encoding) const
{
    return guarded([&]() {
        const QByteArray bytes = messageBytes(message, encoding);
        // Showing the preimage is the whole lesson: the prefix is what stops a
        // website handing you a "message" that is really a transaction hash.
        const QByteArray prefix = QByteArrayLiteral("Stellar Signed Message:\n");

        QVariantMap out;
        out.insert(QStringLiteral("hash"),
                   QString::fromLatin1(MessageAuth::messageHash(bytes).toHex()));
        out.insert(QStringLiteral("prefix"), QString::fromLatin1(prefix));
        out.insert(QStringLiteral("preimageHex"),
                   QString::fromLatin1((prefix + bytes).toHex()));
        out.insert(QStringLiteral("preimageBytes"), prefix.size() + bytes.size());
        out.insert(QStringLiteral("messageBytes"), bytes.size());
        return out;
    });
}

// ─── SEP-11 ──────────────────────────────────────────────────────────

namespace {

/** The envelope's type, by its XDR name: a V0 one is not a V1 one. */
QString envelopeTypeName(AbstractTransaction* envelope)
{
    switch (envelope->toEnvelopeXdr().type) {
    case stellar::EnvelopeType::ENVELOPE_TYPE_TX_V0:
        return QStringLiteral("ENVELOPE_TYPE_TX_V0");
    case stellar::EnvelopeType::ENVELOPE_TYPE_TX:
        return QStringLiteral("ENVELOPE_TYPE_TX");
    case stellar::EnvelopeType::ENVELOPE_TYPE_TX_FEE_BUMP:
        return QStringLiteral("ENVELOPE_TYPE_TX_FEE_BUMP");
    default:
        throw std::runtime_error("not a transaction envelope");
    }
}

/**
 * The fields worth showing about whatever a txrep or an envelope described.
 * For a fee bump, the envelope's own (the account that pays, what it pays,
 * its signatures) and, each under an inner name, the transaction it wraps:
 * that one's source did not sign the fee bump, and its fee is not charged.
 */
void describeTransaction(QVariantMap& out, AbstractTransaction* envelope)
{
    out.insert(QStringLiteral("hash"), envelope->hashHex());
    out.insert(QStringLiteral("envelopeType"), envelopeTypeName(envelope));
    out.insert(QStringLiteral("signatureCount"), envelope->getSignatures().size());
    if (auto* tx = dynamic_cast<Transaction*>(envelope)) {
        out.insert(QStringLiteral("isFeeBump"), false);
        out.insert(QStringLiteral("sourceAccount"), tx->getSourceAccount());
        // int64 as text: a sequence number above 2^53 does not survive a JS Number.
        out.insert(QStringLiteral("sequence"), QString::number(tx->getSequenceNumber()));
        out.insert(QStringLiteral("fee"), QString::number(tx->getFee()));
        out.insert(QStringLiteral("operationCount"), tx->getOperations().size());
        return;
    }
    auto* feeBump = dynamic_cast<FeeBumpTransaction*>(envelope);
    if (!feeBump)
        throw std::runtime_error("not a transaction envelope");
    Transaction* inner = feeBump->getInnerTransaction();
    out.insert(QStringLiteral("isFeeBump"), true);
    out.insert(QStringLiteral("feeSource"), feeBump->getFeeAccount());
    out.insert(QStringLiteral("fee"), QString::number(feeBump->getFee()));
    out.insert(QStringLiteral("innerHash"), inner->hashHex());
    out.insert(QStringLiteral("innerSourceAccount"), inner->getSourceAccount());
    out.insert(QStringLiteral("innerSequence"), QString::number(inner->getSequenceNumber()));
    out.insert(QStringLiteral("innerFee"), QString::number(inner->getFee()));
    out.insert(QStringLiteral("operationCount"), inner->getOperations().size());
    out.insert(QStringLiteral("innerSignatureCount"), inner->getSignatures().size());
}

} // namespace

QVariantMap SepsQml::txrepFromXdr(const QVariantMap& args) const
{
    return guarded([&]() {
        refuseUnknownArgs(args, {"xdr", "network", "networkPassphrase"}, QStringLiteral("txrepFromXdr"));
        Network* network = networkFrom(args);
        QScopedPointer<AbstractTransaction> parsed(
            AbstractTransaction::fromEnvelopeXdr(arg(args, "xdr"), network));
        if (!parsed)
            throw std::runtime_error("could not read the envelope XDR");

        // The envelope's own document, whatever its type: a V0 one stays V0,
        // and a fee bump is the fee bump's, never its inner transaction's
        // (which is not what is signed). A fee bump used to be refused, the
        // emitter taking plain transactions only.
        QVariantMap out;
        out.insert(QStringLiteral("txrep"), Txrep::toTxrep(parsed.data()));
        describeTransaction(out, parsed.data());
        return out;
    });
}

QVariantMap SepsQml::xdrFromTxrep(const QVariantMap& args) const
{
    return guarded([&]() {
        refuseUnknownArgs(args, {"txrep", "network", "networkPassphrase"}, QStringLiteral("xdrFromTxrep"));
        Network* network = networkFrom(args);
        // The envelope the document describes, fee bump and V0 included, with
        // the signatures it carries. Read with fromTxrep, a fee bump was
        // refused and every document came back unsigned.
        QScopedPointer<AbstractTransaction> envelope(
            Txrep::fromTxrepEnvelope(args.value(QStringLiteral("txrep")).toString(), network));
        if (!envelope)
            throw std::runtime_error("the txrep did not describe a transaction");

        QVariantMap out;
        out.insert(QStringLiteral("xdr"), envelope->toEnvelopeXdrBase64());
        describeTransaction(out, envelope.data());
        return out;
    });
}

QVariantMap SepsQml::txrepRoundTrip(const QVariantMap& args) const
{
    return guarded([&]() {
        refuseUnknownArgs(args, {"xdr", "network", "networkPassphrase"}, QStringLiteral("txrepRoundTrip"));
        const QVariantMap forward = txrepFromXdr(args);
        if (forward.contains(QStringLiteral("error")))
            return forward;

        QVariantMap back = args;
        back.remove(QStringLiteral("xdr"));
        back.insert(QStringLiteral("txrep"), forward.value(QStringLiteral("txrep")));
        const QVariantMap reverse = xdrFromTxrep(back);
        if (reverse.contains(QStringLiteral("error")))
            return reverse;

        const QString xdrIn = arg(args, "xdr");
        const QString xdrOut = reverse.value(QStringLiteral("xdr")).toString();

        QVariantMap out;
        out.insert(QStringLiteral("txrep"), forward.value(QStringLiteral("txrep")));
        out.insert(QStringLiteral("hashIn"), forward.value(QStringLiteral("hash")));
        out.insert(QStringLiteral("hashOut"), reverse.value(QStringLiteral("hash")));
        out.insert(QStringLiteral("xdrIn"), xdrIn);
        out.insert(QStringLiteral("xdrOut"), xdrOut);
        // The hash is the transaction's, and the envelope's bytes are the
        // transaction and the signatures on it: the two are compared apart,
        // because a signature list is not covered by the hash (a fee bump's
        // inner one is, being part of the transaction the fee bump signs).
        out.insert(QStringLiteral("matched"),
                   forward.value(QStringLiteral("hash"))
                       == reverse.value(QStringLiteral("hash")));
        out.insert(QStringLiteral("bytesMatched"), xdrIn == xdrOut);
        const int before = forward.value(QStringLiteral("signatureCount")).toInt();
        const int after = reverse.value(QStringLiteral("signatureCount")).toInt();
        out.insert(QStringLiteral("signaturesIn"), before);
        out.insert(QStringLiteral("signaturesOut"), after);
        // Said out loud, because `matched` is about the transaction and a
        // reader will take it to be about the envelope. TxRep writes the
        // signatures, a fee bump's two lists, and xdrFromTxrep reads them
        // back, so a signed envelope comes back signed: none is dropped. It
        // used to write `signatures.len: 0` for everything and read nothing
        // back, and a signed envelope returned unsigned with the same hash.
        out.insert(QStringLiteral("signaturesDropped"),
                   (before - after)
                       + (forward.value(QStringLiteral("innerSignatureCount")).toInt()
                          - reverse.value(QStringLiteral("innerSignatureCount")).toInt()));
        out.insert(QStringLiteral("carries"),
                   QStringLiteral("the envelope: the transaction and the signatures on it"));
        return out;
    });
}

// ─── SEP-10 ──────────────────────────────────────────────────────────

QVariantMap SepsQml::buildChallenge(const QVariantMap& args) const
{
    return guarded([&]() {
        refuseUnknownArgs(args, {"serverSeed", "clientAccountId", "homeDomain", "webAuthDomain",
                                 "timeoutSeconds", "timeoutSec", "network", "networkPassphrase"},
                          QStringLiteral("buildChallenge"));
        QScopedPointer<KeyPair> server(KeyPair::fromSecretSeed(arg(args, "serverSeed")));
        const qint64 timeout = challengeTimeout(args);
        // The web_auth_domain operation is part of every challenge, and one
        // with an empty value is refused by every reader, this SDK's included.
        if (arg(args, "webAuthDomain").trimmed().isEmpty())
            throw std::runtime_error("webAuthDomain is required");
        // Blank, the first key was " auth", which no reader matches.
        if (arg(args, "homeDomain").isEmpty())
            throw std::runtime_error("homeDomain is required");

        QScopedPointer<Transaction> challenge(Sep10Challenge::buildChallengeTx(
            server.data(), arg(args, "clientAccountId"), arg(args, "homeDomain"),
            arg(args, "webAuthDomain"), timeout, networkFrom(args)));

        QVariantMap out;
        out.insert(QStringLiteral("xdr"), challenge->toEnvelopeXdrBase64());
        out.insert(QStringLiteral("serverAccountId"), server->getAccountId());
        putClientAccount(out, arg(args, "clientAccountId"));
        out.insert(QStringLiteral("homeDomain"), arg(args, "homeDomain"));
        out.insert(QStringLiteral("webAuthDomain"), arg(args, "webAuthDomain"));
        // Zero, and that is the whole safety argument: an account's sequence
        // starts far above zero and only ever rises, so this envelope is valid
        // nowhere and forever.
        out.insert(QStringLiteral("sequence"),
                   QString::number(challenge->getSequenceNumber()));
        out.insert(QStringLiteral("fee"), QString::number(challenge->getFee()));
        out.insert(QStringLiteral("operationCount"), challenge->getOperations().size());
        out.insert(QStringLiteral("signatureCount"), challenge->getSignatures().size());
        if (TimeBounds* bounds = challenge->getTimeBounds()) {
            out.insert(QStringLiteral("minTime"), QString::number(bounds->getMinTimePoint()));
            out.insert(QStringLiteral("maxTime"), QString::number(bounds->getMaxTimePoint()));
        }
        return out;
    });
}

QVariantMap SepsQml::readChallenge(const QVariantMap& args) const
{
    return guarded([&]() {
        // The keys signChallenge takes are allowed too, unread: one map is
        // naturally read, shown, then signed. Anything else is a misspelling.
        refuseUnknownArgs(args, {"xdr", "serverAccountId", "homeDomain", "homeDomains", "webAuthDomain",
                                 "clientDomain", "clientDomainSigningKey", "network", "networkPassphrase",
                                 "secretSeed", "clientAccountId", "clientDomainSeed", "alreadySignedBy"},
                          QStringLiteral("readChallenge"));
        const QStringList domains = homeDomainsFrom(args);
        // The client domain the client asked for, bound to its toml's
        // SIGNING_KEY; the two together, or neither.
        const QString clientDomain = arg(args, "clientDomain");
        const QString clientDomainKey = arg(args, "clientDomainSigningKey");
        requireClientDomainPair(clientDomain, clientDomainKey);

        QScopedPointer<Sep10Challenge::ChallengeTransaction> read(
            Sep10Challenge::readChallengeTransaction(
                arg(args, "xdr"), arg(args, "serverAccountId"), domains,
                arg(args, "webAuthDomain"), clientDomain, clientDomainKey, networkFrom(args)));

        Transaction* tx = read->getTransaction();
        QVariantMap out;
        putClientAccount(out, read->getClientAccountId());
        out.insert(QStringLiteral("matchedHomeDomain"), read->getMatchedHomeDomain());
        putClientDomain(out, *read, !clientDomain.isEmpty());
        // The memo names the custodial sub-account the token will be for; it
        // is shown at the step where the user decides whether to sign.
        if (auto* memoId = dynamic_cast<MemoId*>(tx->getMemo()))
            out.insert(QStringLiteral("memoId"), QString::number(memoId->getId()));
        out.insert(QStringLiteral("serverAccountId"), tx->getSourceAccount());
        out.insert(QStringLiteral("sequence"), QString::number(tx->getSequenceNumber()));
        out.insert(QStringLiteral("fee"), QString::number(tx->getFee()));
        out.insert(QStringLiteral("operationCount"), tx->getOperations().size());
        out.insert(QStringLiteral("signatureCount"), tx->getSignatures().size());
        if (TimeBounds* bounds = tx->getTimeBounds()) {
            out.insert(QStringLiteral("minTime"), QString::number(bounds->getMinTimePoint()));
            out.insert(QStringLiteral("maxTime"), QString::number(bounds->getMaxTimePoint()));
            // A maximum of 2^63 or more passed the qint64 as negative and
            // showed a challenge that "expired" long ago (it is refused as
            // expired anyway, by the reader above).
            const quint64 maxTime = bounds->getMaxTimePoint();
            const qint64 now = QDateTime::currentSecsSinceEpoch();
            out.insert(QStringLiteral("expiresInSeconds"),
                       maxTime > quint64(std::numeric_limits<qint64>::max())
                           ? std::numeric_limits<qint64>::max()
                           : qint64(maxTime) - now);
        }
        // Reading it at all means the server's signature checked out — that is
        // part of what readChallengeTransaction does, and it is the half people
        // forget to do by hand.
        out.insert(QStringLiteral("serverSignatureVerified"), true);
        return out;
    });
}

QVariantMap SepsQml::signChallenge(const QVariantMap& args) const
{
    return guarded([&]() {
        // Validate, then sign - never the other way round. This used to sign
        // whatever envelope it was handed, so a malicious or compromised auth
        // server could answer the challenge request with a payment from the
        // user's account at its next sequence number and get it back signed.
        // A client has to prove a challenge IS one before its key touches it,
        // which is what the other SDKs' client flows do (js-stellar-sdk's
        // WebAuth.readChallengeTx, the Flutter, iOS and Kotlin jwtToken):
        // sequence 0, source and signature of the server's SIGNING_KEY, live
        // timebounds, ManageData operations only, "<home domain> auth" first,
        // web_auth_domain matching the host the challenge came from.
        //
        // A key it does not read is refused: a misspelt clientDomainSigningKey
        // would sign a challenge whose client_domain nobody checked.
        refuseUnknownArgs(args, {"xdr", "secretSeed", "serverAccountId", "homeDomain", "homeDomains",
                                 "webAuthDomain", "network", "networkPassphrase", "clientAccountId",
                                 "clientDomain", "clientDomainSigningKey", "clientDomainSeed",
                                 "alreadySignedBy"},
                          QStringLiteral("signChallenge"));
        const QString serverAccountId = arg(args, "serverAccountId");
        if (serverAccountId.isEmpty())
            throw std::runtime_error("serverAccountId is required: the SIGNING_KEY "
                                     "published in the domain's stellar.toml");
        const QStringList domains = homeDomainsFrom(args);
        if (domains.isEmpty())
            throw std::runtime_error("homeDomain is required: the domain whose "
                                     "stellar.toml named the auth server");
        const QString webAuthDomain = arg(args, "webAuthDomain");
        if (webAuthDomain.isEmpty())
            throw std::runtime_error("webAuthDomain is required: the host of the "
                                     "WEB_AUTH_ENDPOINT the challenge came from");

        QScopedPointer<KeyPair> signer(KeyPair::fromSecretSeed(arg(args, "secretSeed")));

        // The account being logged in: the signer's own unless the caller says
        // otherwise, as a signer of a multisig account does, or the owner of a
        // muxed M account, which must be named: its signers are its G's.
        QString expectedClient = arg(args, "clientAccountId");
        if (expectedClient.isEmpty())
            expectedClient = signer->getAccountId();

        // The client domain: the operation must name the domain the client
        // asked for, from the SIGNING_KEY that domain's toml publishes. Signing
        // for it here as well, the key signing is that SIGNING_KEY, and the
        // domain it vouches for has to be said.
        const QString clientDomain = arg(args, "clientDomain");
        QString clientDomainKey = arg(args, "clientDomainSigningKey");
        QScopedPointer<KeyPair> domainSigner;
        if (!arg(args, "clientDomainSeed").isEmpty()) {
            domainSigner.reset(KeyPair::fromSecretSeed(arg(args, "clientDomainSeed")));
            if (clientDomain.isEmpty())
                throw std::runtime_error("clientDomain is required with clientDomainSeed: "
                                         "the domain that key signs for");
            if (!clientDomainKey.isEmpty() && clientDomainKey != domainSigner->getAccountId())
                throw std::runtime_error("clientDomainSigningKey is not the account of clientDomainSeed");
            // One key cannot give both signatures: the server counts it once.
            if (domainSigner->getAccountId() == signer->getAccountId())
                throw std::runtime_error("clientDomainSeed is the key signing for the client: the "
                                         "server needs a signature from each");
            clientDomainKey = domainSigner->getAccountId();
        }
        requireClientDomainPair(clientDomain, clientDomainKey);
        const QStringList alreadySigned = signerKeysFrom(args, "alreadySignedBy");

        QScopedPointer<Sep10Challenge::ChallengeTransaction> read(
            Sep10Challenge::readChallengeTransaction(
                arg(args, "xdr"), serverAccountId, domains, webAuthDomain,
                clientDomain, clientDomainKey, networkFrom(args)));

        // A valid challenge for somebody else's account is still not ours to
        // sign: the token it earns would log this key in as that account.
        if (read->getClientAccountId() != expectedClient)
            throw std::runtime_error(QStringLiteral("this challenge is for %1, not %2")
                                         .arg(read->getClientAccountId(), expectedClient)
                                         .toStdString());

        Transaction* tx = read->getTransaction();

        // Signing twice with the same key adds a byte-identical second
        // signature, and the server then refuses the challenge for carrying a
        // signature it cannot attribute. A second press of "Sign" is a no-op
        // the caller should hear about, not a broken login.
        if (hasSigned(tx, signer.data()))
            throw std::runtime_error("this key has already signed this challenge");
        if (domainSigner && hasSigned(tx, domainSigner.data()))
            throw std::runtime_error("the client domain key has already signed this challenge");
        // Any other signature, the same: this used to sign a challenge that
        // carried somebody else's, and the server refused the result. It must
        // carry the server's and those the caller says came before, no more.
        QStringList expectedSigners{serverAccountId};
        for (const QString& key : alreadySigned) {
            if (key == serverAccountId)
                throw std::runtime_error("alreadySignedBy cannot name the server: its signature "
                                         "is always required");
            expectedSigners.append(key);
        }
        requireSignaturesOfExactly(tx, expectedSigners);

        tx->sign(signer.data());
        if (domainSigner)
            tx->sign(domainSigner.data());

        QVariantMap out;
        out.insert(QStringLiteral("xdr"), tx->toEnvelopeXdrBase64());
        out.insert(QStringLiteral("signatureCount"), tx->getSignatures().size());
        out.insert(QStringLiteral("signedBy"), signer->getAccountId());
        if (domainSigner)
            out.insert(QStringLiteral("clientDomainSignedBy"), domainSigner->getAccountId());
        putClientAccount(out, read->getClientAccountId());
        out.insert(QStringLiteral("matchedHomeDomain"), read->getMatchedHomeDomain());
        putClientDomain(out, *read, !clientDomain.isEmpty());
        // The memo decides whose account the token is for (SEP-10 memo ids
        // mean a custodial sub-account), so it is shown, never hidden.
        if (auto* memoId = dynamic_cast<MemoId*>(tx->getMemo()))
            out.insert(QStringLiteral("memoId"), QString::number(memoId->getId()));
        out.insert(QStringLiteral("hash"), tx->hashHex());
        return out;
    });
}

QVariantMap SepsQml::verifySigners(const QVariantMap& args) const
{
    return guarded([&]() {
        // As readChallenge: signChallenge's keys are allowed, unread.
        refuseUnknownArgs(args, {"xdr", "serverAccountId", "homeDomain", "homeDomains", "webAuthDomain",
                                 "signers", "network", "networkPassphrase", "secretSeed",
                                 "clientAccountId", "clientDomain", "clientDomainSigningKey",
                                 "clientDomainSeed", "alreadySignedBy"},
                          QStringLiteral("verifySigners"));
        const QStringList domains = homeDomainsFrom(args);

        QSet<QString> offered;
        for (const QVariant& v : args.value(QStringLiteral("signers")).toList())
            offered.insert(v.toString().trimmed());

        const QSet<QString> used = Sep10Challenge::verifyChallengeTransactionSigners(
            arg(args, "xdr"), arg(args, "serverAccountId"), domains,
            arg(args, "webAuthDomain"), offered, networkFrom(args));
        // A login no client key signed is not a login, whatever let it through:
        // this reported valid with count 0 for a challenge signed only by the
        // server and the client_domain key.
        if (used.isEmpty())
            throw std::runtime_error("Transaction not signed by any client signer.");

        QVariantList signers;
        for (const QString& key : used)
            signers.append(key);

        QVariantMap out;
        out.insert(QStringLiteral("valid"), true);
        out.insert(QStringLiteral("signers"), signers);
        out.insert(QStringLiteral("count"), signers.size());
        out.insert(QStringLiteral("offered"), offered.size());
        // A key that was offered and did not sign is ignored rather than
        // refused — which is what lets a multisig account present every key it
        // has and let the server work out which ones turned up.
        out.insert(QStringLiteral("ignored"), offered.size() - signers.size());
        return out;
    });
}

QVariantMap SepsQml::sep10Constants() const
{
    QVariantMap out;
    out.insert(QStringLiteral("gracePeriodSeconds"),
               Sep10Challenge::GRACE_PERIOD_SECONDS);
    out.insert(QStringLiteral("nonceBytes"), 48);
    out.insert(QStringLiteral("nonceBase64Chars"), 64);
    out.insert(QStringLiteral("homeDomainSuffix"), QStringLiteral(" auth"));
    out.insert(QStringLiteral("webAuthDomainKey"),
               Sep10Challenge::WEB_AUTH_DOMAIN_MANAGER_DATA_NAME);
    out.insert(QStringLiteral("clientDomainKey"),
               Sep10Challenge::CLIENT_DOMAIN_DATA_NAME);
    // Named for what it is rather than for the field it lands in: this is the
    // sequence a challenge MUST carry, not the one some transaction happens to
    // have, and a reader checking a challenge is checking against this.
    out.insert(QStringLiteral("requiredSequence"), QStringLiteral("0"));
    return out;
}

// ─── SEP-23 ──────────────────────────────────────────────────────────

QVariantMap SepsQml::decodeStrKey(const QString& value) const
{
    return guarded([&]() {
        const QString trimmed = value.trimmed();
        if (trimmed.isEmpty())
            throw std::runtime_error("nothing to decode");

        const StrKey::VersionByte byte = StrKey::decodeVersionByte(trimmed);
        const QByteArray body = StrKey::decodeCheck(byte, trimmed.toLatin1());

        QVariantMap out;
        out.insert(QStringLiteral("versionByte"), int(byte));
        out.insert(QStringLiteral("versionByteName"),
                   QString::fromLatin1(versionByteName(byte)));
        out.insert(QStringLiteral("prefix"), trimmed.left(1));
        out.insert(QStringLiteral("length"), trimmed.size());
        // The body of an S-strkey IS the private key. Handing it to QML as
        // hex is revealing it, whatever the panel says three lines below about
        // decoding not being the same act as revealing — and the panel did say
        // exactly that, which is how this was found. Length only.
        if (byte != StrKey::VersionByte::SEED)
            out.insert(QStringLiteral("bodyHex"), QString::fromLatin1(body.toHex()));
        out.insert(QStringLiteral("bodyLength"), body.size());
        // The checksum only proves nobody mistyped it. It says nothing at all
        // about whether the account exists, and that is the single most useful
        // thing to know about this encoding.
        out.insert(QStringLiteral("checksumOk"), true);
        out.insert(QStringLiteral("exists"), QStringLiteral("unknown \u2014 a strkey "
                   "is a spell-check, not a lookup"));

        switch (byte) {
        case StrKey::VersionByte::ACCOUNT_ID:
            out.insert(QStringLiteral("kind"), QStringLiteral("account"));
            out.insert(QStringLiteral("accountId"), trimmed);
            break;
        case StrKey::VersionByte::MUXED_ACCOUNT: {
            out.insert(QStringLiteral("kind"), QStringLiteral("muxed"));
            const MuxedAccount muxed = MuxedAccount::fromAddress(trimmed);
            out.insert(QStringLiteral("accountId"), muxed.getAccountId());
            out.insert(QStringLiteral("muxedId"), QString::number(muxed.getMuxedId()));
            break;
        }
        case StrKey::VersionByte::SEED:
            out.insert(QStringLiteral("kind"), QStringLiteral("seed"));
            // Deliberately no derived account id here. Decoding is not the same
            // act as revealing, and a page that quietly did both would train
            // the wrong habit.
            break;
        case StrKey::VersionByte::PRE_AUTH_TX:
            out.insert(QStringLiteral("kind"), QStringLiteral("preAuthTx"));
            break;
        case StrKey::VersionByte::SHA256_HASH:
            out.insert(QStringLiteral("kind"), QStringLiteral("sha256Hash"));
            break;
        case StrKey::VersionByte::SIGNED_PAYLOAD: {
            out.insert(QStringLiteral("kind"), QStringLiteral("signedPayload"));
            const QPair<QByteArray, QByteArray> parts =
                StrKey::decodeSignedPayload(trimmed);
            out.insert(QStringLiteral("accountId"),
                       StrKey::encodeStellarAccountId(parts.first));
            out.insert(QStringLiteral("payloadHex"),
                       QString::fromLatin1(parts.second.toHex()));
            out.insert(QStringLiteral("payloadLength"), parts.second.size());
            break;
        }
        case StrKey::VersionByte::CONTRACT:
            out.insert(QStringLiteral("kind"), QStringLiteral("contract"));
            break;
        case StrKey::VersionByte::CLAIMABLE_BALANCE:
            out.insert(QStringLiteral("kind"), QStringLiteral("claimableBalance"));
            break;
        case StrKey::VersionByte::LIQUIDITY_POOL:
            out.insert(QStringLiteral("kind"), QStringLiteral("liquidityPool"));
            break;
        }
        return out;
    });
}

QVariantMap SepsQml::muxedFromAccountId(const QString& accountId,
                                        const QString& muxedId) const
{
    return guarded([&]() {
        bool ok = false;
        // The id arrives as text because it is a uint64 and JavaScript would
        // round the interesting half of the range.
        const quint64 id = muxedId.trimmed().toULongLong(&ok);
        if (!ok)
            throw std::runtime_error("the muxed id must be a whole number, "
                                     "0 to 18446744073709551615");

        const MuxedAccount muxed =
            MuxedAccount::fromAccountIdAndMuxedId(accountId.trimmed(), id);

        QVariantMap out;
        out.insert(QStringLiteral("address"), muxed.toAddress());
        out.insert(QStringLiteral("accountId"), muxed.getAccountId());
        out.insert(QStringLiteral("muxedId"), QString::number(muxed.getMuxedId()));
        out.insert(QStringLiteral("isMuxed"), muxed.isMuxed());
        out.insert(QStringLiteral("length"), muxed.toAddress().size());
        return out;
    });
}

QVariantMap SepsQml::splitMuxed(const QString& address) const
{
    return guarded([&]() {
        const MuxedAccount muxed = MuxedAccount::fromAddress(address.trimmed());

        QVariantMap out;
        out.insert(QStringLiteral("isMuxed"), muxed.isMuxed());
        out.insert(QStringLiteral("accountId"), muxed.getAccountId());
        out.insert(QStringLiteral("muxedId"), QString::number(muxed.getMuxedId()));
        out.insert(QStringLiteral("address"), muxed.toAddress());
        // The ledger only ever sees the G. The id is routing information for
        // whoever runs the pooled account, and nothing on chain looks at it.
        out.insert(QStringLiteral("onLedger"), muxed.getAccountId());
        return out;
    });
}

QVariantMap SepsQml::strKeyKinds() const
{
    QVariantList kinds;
    kinds.append(kindRow("account", "G", StrKey::VersionByte::ACCOUNT_ID, 56, 32,
                         "an ed25519 public key: an account, or a signer"));
    kinds.append(kindRow("seed", "S", StrKey::VersionByte::SEED, 56, 32,
                         "the private half. Never shown to anyone, ever"));
    kinds.append(kindRow("muxed", "M", StrKey::VersionByte::MUXED_ACCOUNT, 69, 40,
                         "an account plus a 64-bit id, for pooled custody"));
    kinds.append(kindRow("contract", "C", StrKey::VersionByte::CONTRACT, 56, 32,
                         "a Soroban contract address"));
    kinds.append(kindRow("claimableBalance", "B", StrKey::VersionByte::CLAIMABLE_BALANCE,
                         58, 33, "a claimable balance id, one byte of type first"));
    kinds.append(kindRow("liquidityPool", "L", StrKey::VersionByte::LIQUIDITY_POOL,
                         56, 32, "a liquidity pool id"));
    kinds.append(kindRow("preAuthTx", "T", StrKey::VersionByte::PRE_AUTH_TX, 56, 32,
                         "a transaction hash used as a signer"));
    kinds.append(kindRow("sha256Hash", "X", StrKey::VersionByte::SHA256_HASH, 56, 32,
                         "a hash whose preimage is the signature"));
    // Length 0 means "varies": a signed payload is the key plus a payload of
    // 1 to 64 bytes, so there is no single number to print.
    kinds.append(kindRow("signedPayload", "P", StrKey::VersionByte::SIGNED_PAYLOAD,
                         0, 0, "a key plus the payload it must sign; length varies"));

    QVariantMap out;
    out.insert(QStringLiteral("kinds"), kinds);
    return out;
}

// ─── SEP-35 ──────────────────────────────────────────────────────────

QVariantMap SepsQml::toidEncode(const QVariantMap& args) const
{
    return guarded([&]() {
        refuseUnknownArgs(args, {"ledger", "txIndex", "opIndex"}, QStringLiteral("toidEncode"));
        // Read exactly: as text, Qt 6 prints the JS number 1000000 as "1e+06",
        // which toULongLong refused, and 1.5 was refused only by accident.
        const quint64 ledger = exactUnsigned(args.value(QStringLiteral("ledger")),
                                             QStringLiteral("ledger"), TOID::MAX_LEDGER);
        const quint64 txIndex = exactUnsigned(args.value(QStringLiteral("txIndex")),
                                              QStringLiteral("txIndex"), TOID::MAX_TX_INDEX);
        const quint64 opIndex = exactUnsigned(args.value(QStringLiteral("opIndex")),
                                              QStringLiteral("opIndex"), TOID::MAX_OP_INDEX);

        const TOID toid{quint32(ledger), quint32(txIndex), quint32(opIndex)};
        const qint64 packed = toid.toInt64();

        QVariantMap out;
        // Decimal as text: past ledger 2097151 the id exceeds 2^53 and a
        // JavaScript number stops being able to hold it exactly. Mainnet passed
        // that in 2018.
        out.insert(QStringLiteral("id"), QString::number(packed));
        out.insert(QStringLiteral("hex"),
                   QStringLiteral("0x%1").arg(quint64(packed), 16, 16, QLatin1Char('0')));
        out.insert(QStringLiteral("ledger"), QString::number(ledger));
        out.insert(QStringLiteral("txIndex"), QString::number(txIndex));
        out.insert(QStringLiteral("opIndex"), QString::number(opIndex));
        out.insert(QStringLiteral("negative"), packed < 0);
        const bool beyond = packed > 9007199254740991LL
                            || packed < -9007199254740991LL;
        out.insert(QStringLiteral("beyondJsSafe"), beyond);
        // Both spellings. A reader branching on "is this safe" should not have
        // to negate, and the negation is exactly what got dropped.
        out.insert(QStringLiteral("jsSafe"), !beyond);
        return out;
    });
}

QVariantMap SepsQml::toidDecode(const QString& packed) const
{
    return guarded([&]() {
        bool ok = false;
        const qint64 value = packed.trimmed().toLongLong(&ok);
        if (!ok)
            throw std::runtime_error("a TOID is a signed 64-bit number in decimal");

        const TOID toid = TOID::fromInt64(value);
        QVariantMap out;
        out.insert(QStringLiteral("ledger"), QString::number(toid.getLedger()));
        out.insert(QStringLiteral("txIndex"), QString::number(toid.getTxIndex()));
        out.insert(QStringLiteral("opIndex"), QString::number(toid.getOpIndex()));
        out.insert(QStringLiteral("id"), QString::number(toid.toInt64()));
        out.insert(QStringLiteral("hex"),
                   QStringLiteral("0x%1").arg(quint64(value), 16, 16, QLatin1Char('0')));
        out.insert(QStringLiteral("roundTrips"), toid.toInt64() == value);
        return out;
    });
}

QVariantMap SepsQml::toidLedgerRange(const QString& ledger) const
{
    return guarded([&]() {
        bool ok = false;
        const quint64 seq = ledger.trimmed().toULongLong(&ok);
        if (!ok || seq > 0xFFFFFFFFull)
            throw std::runtime_error("ledger must be 0 to 4294967295");

        const TOID first{quint32(seq), 0u, 0u};
        const TOID last{quint32(seq), TOID::MAX_TX_INDEX, TOID::MAX_OP_INDEX};

        QVariantMap out;
        out.insert(QStringLiteral("ledger"), QString::number(seq));
        out.insert(QStringLiteral("first"), QString::number(first.toInt64()));
        out.insert(QStringLiteral("last"), QString::number(last.toInt64()));
        // This is what a cursor really is: paging by TOID is paging by a range
        // of these, which is why "the next page" is arithmetic rather than a
        // token the server has to remember.
        out.insert(QStringLiteral("width"),
                   QString::number(last.toInt64() - first.toInt64() + 1));
        return out;
    });
}

QVariantMap SepsQml::toidLimits() const
{
    QVariantMap out;
    // One line saying where the bits go, because three separate numbers do not
    // make the layout obvious and every panel that shows them writes the same
    // sentence otherwise.
    out.insert(QStringLiteral("layout"),
               QStringLiteral("[ ledger : 32 ][ transaction : 20 ][ operation : 12 ]"));
    out.insert(QStringLiteral("ledgerBits"), 32);
    out.insert(QStringLiteral("txIndexBits"), 20);
    out.insert(QStringLiteral("opIndexBits"), 12);
    out.insert(QStringLiteral("maxTxIndex"), QString::number(TOID::MAX_TX_INDEX));
    out.insert(QStringLiteral("maxOpIndex"), QString::number(TOID::MAX_OP_INDEX));
    out.insert(QStringLiteral("maxLedger"), QStringLiteral("4294967295"));
    // Past this ledger the packed id no longer fits a JavaScript number
    // exactly. The public network went past it years ago, which is why every
    // id here crosses as a string.
    out.insert(QStringLiteral("jsSafeLedgerCeiling"), QStringLiteral("2097151"));
    // And past THIS one the id reaches into the sign bit and comes back
    // negative. The layout has no room above it and no version field to grow
    // into, which is a design decision rather than an oversight.
    out.insert(QStringLiteral("maxSafeLedger"), QStringLiteral("2147483647"));
    out.insert(QStringLiteral("maxPositiveId"), QStringLiteral("9223372036854775807"));
    return out;
}

// ─── SEP-29 ──────────────────────────────────────────────────────────

QVariantMap SepsQml::sep29Inspect(const QString& envelopeXdr,
                                  const QString& networkPassphrase) const
{
    return guarded([&]() {
        QVariantMap spec;
        spec.insert(QStringLiteral("networkPassphrase"), networkPassphrase);

        QScopedPointer<AbstractTransaction> parsed(
            AbstractTransaction::fromEnvelopeXdr(envelopeXdr.trimmed(),
                                                 networkFrom(spec)));
        Transaction* tx = dynamic_cast<Transaction*>(parsed.data());
        QVariantMap out;
        out.insert(QStringLiteral("isFeeBump"), tx == nullptr);
        if (!tx)
            throw std::runtime_error("this envelope is a fee-bump; inspect the "
                                     "inner transaction instead");

        // The SDK's own accept rule, mirrored so the page can show WHY a memo
        // counted: any memo does, an empty text and a MEMO_ID of 0 included,
        // as in js and py. Both used to count as forgotten, here and in
        // Server.
        QString memoType = QStringLiteral("none");
        QString memoValue;
        bool satisfies = false;
        if (Memo* memo = tx->getMemo()) {
            if (MemoText* text = dynamic_cast<MemoText*>(memo)) {
                memoType = QStringLiteral("text");
                memoValue = text->getText();
                satisfies = true;
            } else if (MemoId* id = dynamic_cast<MemoId*>(memo)) {
                memoType = QStringLiteral("id");
                memoValue = QString::number(id->getId());
                satisfies = true;
            } else if (MemoHashAbstract* hash = dynamic_cast<MemoHashAbstract*>(memo)) {
                memoType = QStringLiteral("hash");
                memoValue = hash->getHexValue();
                satisfies = true;
            }
        }
        out.insert(QStringLiteral("memoType"), memoType);
        out.insert(QStringLiteral("memoValue"), memoValue);
        out.insert(QStringLiteral("memoSatisfies"), satisfies);

        QVariantList destinations;
        QVariantList skippedMuxed;
        QVariantList unchecked;
        int ignoredCount = 0;
        QSet<QString> seen;

        const QVector<Operation*> operations = tx->getOperations();
        for (Operation* op : operations) {
            QString destination;
            if (auto* payment = dynamic_cast<PaymentOperation*>(op))
                destination = payment->getDestination();
            else if (auto* path = dynamic_cast<PathPaymentStrictReceiveOperation*>(op))
                destination = path->getDestination();
            // Operation::fromXdr builds the DEPRECATED PathPaymentOperation for
            // PATH_PAYMENT_STRICT_RECEIVE, and it derives from Operation rather
            // than from the class above — so the cast above never matches a
            // decoded envelope and this walk silently under-reported the
            // lookups a SEP-29 client owes. Exactly the trap the TxRep emitter
            // fell into, which is why emitDeprecatedPathPayment exists.
            else if (auto* legacy = dynamic_cast<PathPaymentOperation*>(op))
                destination = legacy->getDestination();
            else if (auto* path = dynamic_cast<PathPaymentStrictSendOperation*>(op))
                destination = path->getDestination();
            else if (auto* merge = dynamic_cast<AccountMergeOperation*>(op))
                destination = merge->getDestination();
            else if (auto* create = dynamic_cast<CreateAccountOperation*>(op)) {
                // The one that catches people out. createAccount moves XLM to
                // somebody else and SEP-29 does NOT look at it — the SDK's own
                // walk lists four operation types and this is not one of them.
                // getDestination() hands back a KeyPair the operation still
                // owns — this is the older accessor shape — so take the address
                // out of it rather than letting a pointer anywhere near QML.
                if (KeyPair* who = create->getDestination())
                    unchecked.append(who->getAccountId());
                continue;
            } else {
                // Everything else moves no value to a third party, so the
                // convention has no opinion about it.
                ++ignoredCount;
                continue;
            }

            // A muxed destination carries its own routing id, so the memo
            // question does not arise and a client asking anyway would be
            // spending a round trip on nothing.
            if (StrKey::encodeToXDRMuxedAccount(destination).type
                    == stellar::CryptoKeyType::KEY_TYPE_MUXED_ED25519) {
                skippedMuxed.append(destination);
                continue;
            }
            if (!seen.contains(destination)) {
                seen.insert(destination);
                destinations.append(destination);
            }
        }

        out.insert(QStringLiteral("destinations"), destinations);
        out.insert(QStringLiteral("skippedMuxed"), skippedMuxed);
        out.insert(QStringLiteral("uncheckedDestinations"), unchecked);
        out.insert(QStringLiteral("ignoredOperationCount"), ignoredCount);
        // The same information as a list, because a caller wanting to show
        // WHICH operations were passed over cannot reconstruct it from a
        // count. Each entry is { destination, why }.
        QVariantList ignoredOps;
        for (const QVariant& who : unchecked) {
            QVariantMap entry;
            entry.insert(QStringLiteral("destination"), who.toString());
            entry.insert(QStringLiteral("why"),
                         QStringLiteral("createAccount moves XLM to a third "
                                        "party and SEP-29 does not look at it"));
            ignoredOps.append(entry);
        }
        for (const QVariant& who : skippedMuxed) {
            QVariantMap entry;
            entry.insert(QStringLiteral("destination"), who.toString());
            entry.insert(QStringLiteral("why"),
                         QStringLiteral("a muxed address carries its own routing "
                                        "id, so the question does not arise"));
            ignoredOps.append(entry);
        }
        out.insert(QStringLiteral("ignoredOps"), ignoredOps);
        // Both spellings of the same number: the panel asked for wouldFetch.
        out.insert(QStringLiteral("wouldFetch"),
                   satisfies ? 0 : destinations.size());
        out.insert(QStringLiteral("operationCount"), operations.size());
        // What a client would do next, said out loud: one Horizon lookup per
        // destination, and only when the memo does not already satisfy it.
        out.insert(QStringLiteral("lookupsRequired"),
                   satisfies ? 0 : destinations.size());
        return out;
    });
}

QVariantMap SepsQml::sep29Flag() const
{
    QVariantMap out;
    out.insert(QStringLiteral("key"), QString::fromLatin1(Sep29::DATA_NAME));
    out.insert(QStringLiteral("value"), QString::fromLatin1(Sep29::DATA_VALUE));
    out.insert(QStringLiteral("encodedValue"), QString::fromLatin1(Sep29::DATA_VALUE_BASE64));
    out.insert(QStringLiteral("setWith"), QStringLiteral("manageData"));
    return out;
}

// ─── SEP-45 ──────────────────────────────────────────────────────────

namespace {

/** What every SEP-45 reader is told to expect, all required. */
struct Sep45Expectations
{
    QString entries;
    QString serverAccountId;
    QStringList homeDomains;
    QString webAuthDomain;
    QString webAuthContract;
    Network* network = nullptr;
};

Sep45Expectations sep45Expectations(const QVariantMap& args)
{
    Sep45Expectations e;
    e.entries = arg(args, "authorizationEntries");
    if (e.entries.isEmpty())
        throw std::runtime_error("authorizationEntries is required: the challenge, in base64");
    e.serverAccountId = arg(args, "serverAccountId");
    if (e.serverAccountId.isEmpty())
        throw std::runtime_error("serverAccountId is required: the SIGNING_KEY published in the "
                                 "domain's stellar.toml");
    // The SEP-10 reading: a string is one domain, and blanks are dropped.
    e.homeDomains = homeDomainsFrom(args);
    if (e.homeDomains.isEmpty())
        throw std::runtime_error("homeDomain is required: the domain whose stellar.toml named "
                                 "the auth server");
    e.webAuthDomain = arg(args, "webAuthDomain");
    if (e.webAuthDomain.isEmpty())
        throw std::runtime_error("webAuthDomain is required: the host of the "
                                 "WEB_AUTH_FOR_CONTRACTS_ENDPOINT the challenge came from");
    // Required: what the wallet signs is a call of this contract.
    e.webAuthContract = arg(args, "webAuthContract");
    if (e.webAuthContract.isEmpty())
        throw std::runtime_error("webAuthContract is required: the WEB_AUTH_CONTRACT_ID the "
                                 "domain's stellar.toml names");
    e.network = networkFrom(args);
    return e;
}

Sep45Challenge::ChallengeAuthorizationEntries readSep45(const Sep45Expectations& e,
                                                        const QString& entries)
{
    return Sep45Challenge::readChallengeAuthorizationEntries(
        entries, e.serverAccountId, e.homeDomains, e.webAuthDomain, e.webAuthContract, e.network);
}

/** A client domain is expected with its toml's SIGNING_KEY or not at all. */
void requireSep45ClientDomainPair(const QString& clientDomain, const QString& account)
{
    if (!clientDomain.isEmpty() && account.isEmpty())
        throw std::runtime_error("clientDomainAccount is required with clientDomain: the "
                                 "SIGNING_KEY of that domain's stellar.toml");
    if (clientDomain.isEmpty() && !account.isEmpty())
        throw std::runtime_error("clientDomain is required with clientDomainAccount");
}

/** A ledger number, exactly: 1 to 2^32 - 1. */
quint32 ledgerArg(const QVariantMap& args, const char* key, const char* why)
{
    const QVariant raw = args.value(QString::fromLatin1(key));
    if (isMissing(raw) || (raw.userType() == QMetaType::QString && raw.toString().trimmed().isEmpty()))
        throw std::runtime_error(QStringLiteral("%1 is required: %2")
                                     .arg(QString::fromLatin1(key), QString::fromLatin1(why))
                                     .toStdString());
    if (raw.userType() == QMetaType::Bool)
        throw std::runtime_error(QStringLiteral("%1 must be a ledger number").arg(QString::fromLatin1(key))
                                     .toStdString());
    const quint64 value = exactUnsigned(raw, QString::fromLatin1(key), 4294967295ULL);
    if (value == 0)
        throw std::runtime_error(QStringLiteral("%1 must be at least 1").arg(QString::fromLatin1(key))
                                     .toStdString());
    return quint32(value);
}

/**
 * The `simulation` map Soroban.simulate delivered, if it ran the call and the
 * call succeeded, carrying the entries of `read`. A failed one throws with the
 * host's own reason.
 */
QVariantMap sep45Simulation(const QVariantMap& args,
                            const Sep45Challenge::ChallengeAuthorizationEntries& read)
{
    const QVariant simulation = args.value(QStringLiteral("simulation"));
    if (simulation.userType() != QMetaType::QVariantMap)
        throw std::runtime_error("simulation is required: what Soroban.simulate answered for "
                                 "sep45VerificationTransaction's xdr");
    const QVariantMap sim = simulation.toMap();
    const QString error = sim.value(QStringLiteral("error")).toString();
    if (!error.isEmpty())
        throw std::runtime_error(QStringLiteral("Validation failed during simulation: %1")
                                     .arg(error).toStdString());
    if (sim.value(QStringLiteral("ok")).userType() != QMetaType::Bool
            || !sim.value(QStringLiteral("ok")).toBool())
        throw std::runtime_error("the simulation did not succeed");
    // An answer with neither an error nor a result did not run the call.
    if (sim.value(QStringLiteral("returnValueXdr")).toString().isEmpty())
        throw std::runtime_error("the simulation returned no result: the call was not run");

    // Which call ran: one that carries its own entries is simulated with them,
    // and the simulation reports them back as its `auth`. They must be these.
    // The anchor's first simulation, of the challenge before anybody signed
    // it, succeeds as well, and taken for this one it verified a login that
    // nobody had signed.
    const QVariant auth = sim.value(QStringLiteral("auth"));
    if (auth.userType() != QMetaType::QVariantList && auth.userType() != QMetaType::QStringList)
        throw std::runtime_error("the simulation reports no auth entries, so nothing says it ran "
                                 "these: simulate sep45VerificationTransaction's xdr");
    QList<QByteArray> reported;
    for (const QVariant& entry : auth.toList()) {
        if (entry.userType() != QMetaType::QString)
            throw std::runtime_error("the simulation's auth entries are base64 strings");
        const auto bytes = QByteArray::fromBase64Encoding(entry.toString().toLatin1(),
                                                          QByteArray::AbortOnBase64DecodingErrors);
        if (!bytes)
            throw std::runtime_error("the simulation's auth entries are base64 strings");
        reported.append(*bytes);
    }
    QList<QByteArray> expected;
    for (const stellar::SorobanAuthorizationEntry& entry : read.authorizationEntries) {
        QByteArray bytes;
        QDataStream out(&bytes, QIODevice::WriteOnly);
        out << entry;
        expected.append(bytes);
    }
    std::sort(reported.begin(), reported.end());
    std::sort(expected.begin(), expected.end());
    if (reported != expected)
        throw std::runtime_error("the simulation did not run these entries: it is of another "
                                 "call, or of the challenge before it was signed");
    return sim;
}

/** Everything a SEP-45 challenge says, for a page to show. */
void putSep45Challenge(QVariantMap& out, const Sep45Challenge::ChallengeAuthorizationEntries& read)
{
    // What a client's signature authorizes, shown rather than implied: this
    // call of this contract, and nothing under it.
    out.insert(QStringLiteral("contract"), read.webAuthContract);
    out.insert(QStringLiteral("function"), Sep45Challenge::DEFAULT_WEB_AUTH_FUNCTION_NAME);
    out.insert(QStringLiteral("clientAccountId"), read.clientAccountId);
    out.insert(QStringLiteral("matchedHomeDomain"), read.matchedHomeDomain);
    out.insert(QStringLiteral("webAuthDomain"), read.webAuthDomain);
    out.insert(QStringLiteral("serverAccountId"), read.serverAccountId);
    out.insert(QStringLiteral("nonce"), read.nonce);
    put(out, "clientDomain", read.clientDomain);
    put(out, "clientDomainAccount", read.clientDomainAccount);
    out.insert(QStringLiteral("serverSignatureExpirationLedger"),
               qint64(read.serverSignatureExpirationLedger));
    QVariantList entries;
    for (const stellar::SorobanAuthorizationEntry& entry : read.authorizationEntries) {
        const stellar::SorobanAddressCredentials& credentials = entry.credentials.address;
        const QString address = Scv::fromAddress(Scv::toAddress(credentials.address)).toString();
        QVariantMap row;
        row.insert(QStringLiteral("address"), address);
        row.insert(QStringLiteral("role"), address == read.serverAccountId ? QStringLiteral("server")
                                           : address == read.clientAccountId ? QStringLiteral("client")
                                                                             : QStringLiteral("clientDomain"));
        // An int64, so a string.
        row.insert(QStringLiteral("nonce"), QString::number(credentials.nonce));
        row.insert(QStringLiteral("signatureExpirationLedger"),
                   qint64(credentials.signatureExpirationLedger));
        row.insert(QStringLiteral("signed"),
                   credentials.signature.type != stellar::SCValType::SCV_VOID);
        entries.append(row);
    }
    out.insert(QStringLiteral("entries"), entries);
    out.insert(QStringLiteral("entryCount"), entries.size());
}

} // namespace

QVariantMap SepsQml::sep45ChallengeTransaction(const QVariantMap& args) const
{
    return guarded([&]() {
        refuseUnknownArgs(args, {"serverAccountId", "clientAccountId", "homeDomain", "webAuthDomain",
                                 "webAuthContract", "nonce", "clientDomain", "clientDomainAccount",
                                 "network", "networkPassphrase"},
                          QStringLiteral("sep45ChallengeTransaction"));
        requireSep45ClientDomainPair(arg(args, "clientDomain"), arg(args, "clientDomainAccount"));
        // py's default: 48 random bytes, base64. Returned, because the
        // simulated entries are checked against it.
        QString nonce = arg(args, "nonce");
        if (nonce.isEmpty())
            nonce = QString::fromLatin1(Util::generateRandomNonce(48).toBase64());
        const QString xdr = Sep45Challenge::challengeTransaction(
            arg(args, "serverAccountId"), arg(args, "clientAccountId"), arg(args, "homeDomain"),
            arg(args, "webAuthDomain"), arg(args, "webAuthContract"), networkFrom(args), nonce,
            arg(args, "clientDomain"), arg(args, "clientDomainAccount"));
        QVariantMap out;
        out.insert(QStringLiteral("xdr"), xdr);
        out.insert(QStringLiteral("nonce"), nonce);
        out.insert(QStringLiteral("sourceAccount"), Sep45Challenge::NULL_ACCOUNT);
        out.insert(QStringLiteral("sequence"), QStringLiteral("1"));
        out.insert(QStringLiteral("fee"), QStringLiteral("100"));
        return out;
    });
}

QVariantMap SepsQml::newSep45Challenge(const QVariantMap& args) const
{
    return guarded([&]() {
        // As buildChallenge: a misspelt key would be dropped in silence, and
        // the server would sign a challenge other than the one asked for.
        refuseUnknownArgs(args, {"serverSeed", "clientAccountId", "homeDomain", "webAuthDomain",
                                 "webAuthContract", "nonce", "clientDomain", "clientDomainAccount",
                                 "latestLedger", "expireInLedgers", "simulatedAuth", "network",
                                 "networkPassphrase"},
                          QStringLiteral("newSep45Challenge"));
        QScopedPointer<KeyPair> server(KeyPair::fromSecretSeed(arg(args, "serverSeed")));
        if (arg(args, "homeDomain").isEmpty())
            throw std::runtime_error("homeDomain is required");
        if (arg(args, "webAuthDomain").isEmpty())
            throw std::runtime_error("webAuthDomain is required");
        if (arg(args, "webAuthContract").isEmpty())
            throw std::runtime_error("webAuthContract is required: the WEB_AUTH_CONTRACT_ID of the "
                                     "server's stellar.toml");
        const QString clientDomain = arg(args, "clientDomain");
        const QString clientDomainAccount = arg(args, "clientDomainAccount");
        requireSep45ClientDomainPair(clientDomain, clientDomainAccount);
        const quint32 latestLedger = ledgerArg(args, "latestLedger",
                                               "the ledger the simulation ran at, or the network's latest");
        quint32 expireInLedgers = Sep45Challenge::DEFAULT_EXPIRE_IN_LEDGERS;
        if (!isMissing(args.value(QStringLiteral("expireInLedgers"))))
            expireInLedgers = ledgerArg(args, "expireInLedgers", "how many ledgers the signature lasts");
        Network* network = networkFrom(args);

        QString entries;
        const QVariant simulated = args.value(QStringLiteral("simulatedAuth"));
        const bool fromSimulation = !isMissing(simulated);
        if (fromSimulation) {
            if (simulated.userType() != QMetaType::QVariantList
                    && simulated.userType() != QMetaType::QStringList)
                throw std::runtime_error("simulatedAuth is the list of base64 entries the "
                                         "simulation returned");
            QStringList auth;
            for (const QVariant& entry : simulated.toList()) {
                if (entry.userType() != QMetaType::QString)
                    throw std::runtime_error("simulatedAuth holds base64 entries, as strings");
                auth.append(entry.toString());
            }
            // The one the simulated transaction carries: without it there is
            // nothing to check the simulated entries against.
            if (arg(args, "nonce").isEmpty())
                throw std::runtime_error("nonce is required with simulatedAuth: the one "
                                         "sep45ChallengeTransaction put in the simulated call");
            entries = Sep45Challenge::signSimulatedChallengeEntries(
                auth, latestLedger, server.data(), arg(args, "clientAccountId"), arg(args, "homeDomain"),
                arg(args, "webAuthDomain"), arg(args, "webAuthContract"), network, arg(args, "nonce"),
                expireInLedgers, clientDomain, clientDomainAccount);
        } else {
            entries = Sep45Challenge::buildChallengeAuthorizationEntries(
                server.data(), arg(args, "clientAccountId"), arg(args, "homeDomain"),
                arg(args, "webAuthDomain"), arg(args, "webAuthContract"), network, latestLedger,
                arg(args, "nonce"), expireInLedgers, clientDomain, clientDomainAccount);
        }

        // Read back as a wallet will read it, so what is shown is what was
        // issued and what was issued is what a wallet accepts.
        const Sep45Challenge::ChallengeAuthorizationEntries read =
            Sep45Challenge::readChallengeAuthorizationEntries(
                entries, server->getAccountId(), QStringList{arg(args, "homeDomain")},
                arg(args, "webAuthDomain"), arg(args, "webAuthContract"), network);
        QVariantMap out;
        putSep45Challenge(out, read);
        out.insert(QStringLiteral("authorizationEntries"), entries);
        out.insert(QStringLiteral("homeDomain"), read.matchedHomeDomain);
        out.insert(QStringLiteral("webAuthContract"), read.webAuthContract);
        out.insert(QStringLiteral("simulated"), fromSimulation);
        return out;
    });
}

QVariantMap SepsQml::inspectSep45Challenge(const QVariantMap& args) const
{
    return guarded([&]() {
        // A misspelt webAuthContract or clientDomainAccount would otherwise
        // leave what it names unchecked.
        refuseUnknownArgs(args, {"authorizationEntries", "serverAccountId", "homeDomain", "homeDomains",
                                 "webAuthDomain", "webAuthContract", "clientAccountId", "clientDomain",
                                 "clientDomainAccount", "network", "networkPassphrase"},
                          QStringLiteral("inspectSep45Challenge"));
        const Sep45Expectations expected = sep45Expectations(args);
        const QString clientDomain = arg(args, "clientDomain");
        const QString clientDomainAccount = arg(args, "clientDomainAccount");
        requireSep45ClientDomainPair(clientDomain, clientDomainAccount);

        const Sep45Challenge::ChallengeAuthorizationEntries read = readSep45(expected, expected.entries);
        const QString client = arg(args, "clientAccountId");
        if (!client.isEmpty() && read.clientAccountId != client)
            throw std::runtime_error(QStringLiteral("this challenge is for %1, not %2")
                                         .arg(read.clientAccountId, client).toStdString());
        if (!clientDomain.isEmpty()
                && (read.clientDomain != clientDomain || read.clientDomainAccount != clientDomainAccount))
            throw std::runtime_error("the challenge does not carry the client domain asked for, "
                                     "with that domain's signing key");

        QVariantMap out;
        putSep45Challenge(out, read);
        if (!read.clientDomain.isEmpty())
            out.insert(QStringLiteral("clientDomainChecked"), !clientDomain.isEmpty());
        out.insert(QStringLiteral("structurallyValid"), true);
        // Reading it at all means the server's signature on its entry checked
        // out, the half the SEP asks the wallet to do and py leaves undone.
        out.insert(QStringLiteral("serverSignatureVerified"), true);
        // Said plainly rather than implied by omission: only the anchor's
        // simulation shows that the client's contract accepts its signature.
        out.insert(QStringLiteral("clientProven"), false);
        out.insert(QStringLiteral("simulated"), false);
        return out;
    });
}

QVariantMap SepsQml::signSep45Challenge(const QVariantMap& args) const
{
    return guarded([&]() {
        refuseUnknownArgs(args, {"authorizationEntries", "serverAccountId", "homeDomain", "homeDomains",
                                 "webAuthDomain", "webAuthContract", "clientAccountId", "secretSeed",
                                 "validUntilLedger", "clientDomain", "clientDomainAccount",
                                 "clientDomainSeed", "network", "networkPassphrase"},
                          QStringLiteral("signSep45Challenge"));
        const Sep45Expectations expected = sep45Expectations(args);
        const QString client = arg(args, "clientAccountId");
        if (client.isEmpty())
            throw std::runtime_error("clientAccountId is required: the contract account logging in");
        const quint32 validUntil = ledgerArg(args, "validUntilLedger",
                                             "the last ledger the signature is good for");

        QScopedPointer<KeyPair> signer;
        if (!arg(args, "secretSeed").isEmpty())
            signer.reset(KeyPair::fromSecretSeed(arg(args, "secretSeed")));
        const QString clientDomain = arg(args, "clientDomain");
        QString clientDomainAccount = arg(args, "clientDomainAccount");
        QScopedPointer<KeyPair> domainSigner;
        if (!arg(args, "clientDomainSeed").isEmpty()) {
            domainSigner.reset(KeyPair::fromSecretSeed(arg(args, "clientDomainSeed")));
            if (clientDomain.isEmpty())
                throw std::runtime_error("clientDomain is required with clientDomainSeed: the "
                                         "domain that key signs for");
            if (!clientDomainAccount.isEmpty() && clientDomainAccount != domainSigner->getAccountId())
                throw std::runtime_error("clientDomainAccount is not the account of clientDomainSeed");
            clientDomainAccount = domainSigner->getAccountId();
        }
        if (!signer && !domainSigner)
            throw std::runtime_error("secretSeed is required: the key the client contract accepts "
                                     "(or clientDomainSeed, to sign for the client domain alone)");
        requireSep45ClientDomainPair(clientDomain, clientDomainAccount);

        // Validated before anything is signed, inside the SDK call: this is
        // the step that cannot be taken back.
        const QString signedEntries = Sep45Challenge::signChallengeAuthorizationEntries(
            expected.entries, expected.serverAccountId, expected.homeDomains, expected.webAuthDomain,
            expected.webAuthContract, client, signer.data(), validUntil, expected.network,
            clientDomain, clientDomainAccount, domainSigner.data());

        const Sep45Challenge::ChallengeAuthorizationEntries read = readSep45(expected, signedEntries);
        QVariantMap out;
        putSep45Challenge(out, read);
        if (!read.clientDomain.isEmpty())
            out.insert(QStringLiteral("clientDomainChecked"), !clientDomain.isEmpty());
        out.insert(QStringLiteral("authorizationEntries"), signedEntries);
        if (signer)
            out.insert(QStringLiteral("signedBy"), signer->getAccountId());
        if (domainSigner)
            out.insert(QStringLiteral("clientDomainSignedBy"), domainSigner->getAccountId());
        out.insert(QStringLiteral("validUntilLedger"), qint64(validUntil));
        return out;
    });
}

QVariantMap SepsQml::sep45VerificationTransaction(const QVariantMap& args) const
{
    return guarded([&]() {
        refuseUnknownArgs(args, {"authorizationEntries", "serverAccountId", "homeDomain", "homeDomains",
                                 "webAuthDomain", "webAuthContract", "network", "networkPassphrase"},
                          QStringLiteral("sep45VerificationTransaction"));
        const Sep45Expectations expected = sep45Expectations(args);
        const Sep45Challenge::ChallengeAuthorizationEntries read = readSep45(expected, expected.entries);
        QVariantMap out;
        putSep45Challenge(out, read);
        out.insert(QStringLiteral("xdr"), Sep45Challenge::verificationTransaction(read, expected.network));
        return out;
    });
}

QVariantMap SepsQml::checkSep45Simulation(const QVariantMap& args) const
{
    return guarded([&]() {
        refuseUnknownArgs(args, {"authorizationEntries", "serverAccountId", "homeDomain", "homeDomains",
                                 "webAuthDomain", "webAuthContract", "simulation", "network",
                                 "networkPassphrase"},
                          QStringLiteral("checkSep45Simulation"));
        const Sep45Expectations expected = sep45Expectations(args);
        const Sep45Challenge::ChallengeAuthorizationEntries read = readSep45(expected, expected.entries);
        const QVariantMap sim = sep45Simulation(args, read);
        // What the call writes is what signing it does: the challenge's own
        // nonces, or a side effect the wallet never asked for.
        Sep45Challenge::checkSimulationFootprint(read, sim.value(QStringLiteral("transactionData")).toString());
        QVariantMap out;
        putSep45Challenge(out, read);
        out.insert(QStringLiteral("sideEffectFree"), true);
        out.insert(QStringLiteral("simulated"), true);
        return out;
    });
}

QVariantMap SepsQml::verifySep45Challenge(const QVariantMap& args) const
{
    return guarded([&]() {
        refuseUnknownArgs(args, {"authorizationEntries", "serverAccountId", "homeDomain", "homeDomains",
                                 "webAuthDomain", "webAuthContract", "simulation", "network",
                                 "networkPassphrase"},
                          QStringLiteral("verifySep45Challenge"));
        const Sep45Expectations expected = sep45Expectations(args);
        const Sep45Challenge::ChallengeAuthorizationEntries read = readSep45(expected, expected.entries);
        // A failed simulation is the contract refusing the client's signature
        // (or the client domain's): the answer, and the end of the login.
        sep45Simulation(args, read);

        QVariantMap out;
        putSep45Challenge(out, read);
        out.insert(QStringLiteral("verified"), true);
        out.insert(QStringLiteral("clientProven"), true);
        out.insert(QStringLiteral("simulated"), true);
        return out;
    });
}

QVariantMap SepsQml::sep45Constants() const
{
    QVariantMap out;
    out.insert(QStringLiteral("functionName"), Sep45Challenge::DEFAULT_WEB_AUTH_FUNCTION_NAME);
    out.insert(QStringLiteral("argumentKeys"),
               QStringList{QStringLiteral("account"), QStringLiteral("client_domain"),
                           QStringLiteral("client_domain_account"), QStringLiteral("home_domain"),
                           QStringLiteral("nonce"), QStringLiteral("web_auth_domain"),
                           QStringLiteral("web_auth_domain_account")});
    out.insert(QStringLiteral("nullAccount"), Sep45Challenge::NULL_ACCOUNT);
    out.insert(QStringLiteral("defaultExpireInLedgers"), qint64(Sep45Challenge::DEFAULT_EXPIRE_IN_LEDGERS));
    return out;
}

// ─── SEP-2 ───────────────────────────────────────────────────────────

QVariantMap SepsQml::describeAddress(const QString& value) const
{
    return guarded([&]() {
        const QString trimmed = value.trimmed();
        const QStringList parts = trimmed.split(QLatin1Char('*'));

        QVariantMap out;
        // What resolveFederation would refuse, with its reason: a blank value
        // was described as an account id, and "*example.com" or
        // "bob*good.example@evil.example" as addresses worth a lookup.
        try {
            Federation::validate(trimmed);
        } catch (const std::exception& e) {
            out.insert(QStringLiteral("kind"), QStringLiteral("malformed"));
            out.insert(QStringLiteral("wouldQuery"), false);
            out.insert(QStringLiteral("note"), QString::fromUtf8(e.what()));
            return out;
        }
        if (parts.size() == 1) {
            // Federation::resolve treats this as an ANSWER rather than a
            // question: it hands the value straight back, having checked only
            // that it is spelt as an account id and asked nobody. People read
            // that as proof the account exists.
            out.insert(QStringLiteral("kind"), QStringLiteral("accountId"));
            out.insert(QStringLiteral("accountId"), trimmed);
            out.insert(QStringLiteral("wouldQuery"), false);
            out.insert(QStringLiteral("note"),
                       QStringLiteral("handed straight back \u2014 its spelling is "
                                      "checked, nothing is looked up"));
        } else if (parts.size() == 2) {
            out.insert(QStringLiteral("kind"), QStringLiteral("address"));
            out.insert(QStringLiteral("name"), parts.at(0));
            out.insert(QStringLiteral("domain"), parts.at(1));
            out.insert(QStringLiteral("tomlUrl"),
                       QStringLiteral("https://%1/.well-known/stellar.toml")
                           .arg(parts.at(1)));
            out.insert(QStringLiteral("wouldQuery"), true);
            out.insert(QStringLiteral("roundTrips"), 2);
        } else {
            out.insert(QStringLiteral("kind"), QStringLiteral("malformed"));
            out.insert(QStringLiteral("wouldQuery"), false);
            out.insert(QStringLiteral("note"),
                       QStringLiteral("more than one * \u2014 the SDK throws "
                                      "\"Malformed address\""));
        }
        return out;
    });
}

QVariantMap SepsQml::parseFederationJson(const QString& json) const
{
    return guarded([&]() {
        QJsonParseError problem;
        const QJsonDocument document =
            QJsonDocument::fromJson(json.toUtf8(), &problem);
        if (problem.error != QJsonParseError::NoError)
            throw std::runtime_error(problem.errorString().toUtf8().constData());
        if (!document.isObject())
            throw std::runtime_error("a federation answer is a JSON object");

        const QJsonObject object = document.object();
        // Checked as resolveFederation checks a live answer: an account id
        // that is not one, or a memo invalid for its type, is refused rather
        // than shown as something a wallet could pay.
        {
            FederationResponse check(nullptr);
            check.loadFromJson(json.toUtf8());
        }
        QVariantMap out;
        put(out, "stellarAddress",
            object.value(QStringLiteral("stellar_address")).toString());
        put(out, "accountId", object.value(QStringLiteral("account_id")).toString());
        put(out, "memoType", object.value(QStringLiteral("memo_type")).toString());
        put(out, "memo", object.value(QStringLiteral("memo")).toString());
        // The memo is the half people forget. A federated address can route to
        // one shared account and identify the customer by memo, so dropping it
        // sends money to the exchange rather than to the person.
        out.insert(QStringLiteral("routesByMemo"),
                   !object.value(QStringLiteral("memo")).toString().isEmpty());
        return out;
    });
}

int SepsQml::nextRequestId()
{
    return m_nextRequestId++;
}

void SepsQml::failWith(int requestId, const QString& kind, const QString& message)
{
    QVariantMap error;
    error.insert(QStringLiteral("kind"), kind);
    error.insert(QStringLiteral("message"), message);
    // Queued: this is called from inside resolveFederation(), which has not
    // returned the request id yet, so a caller that correlates on the id —
    // which the facade tells them to do — would never see this at all.
    QMetaObject::invokeMethod(this, [this, requestId, error]() {
        emit failed(requestId, error);
    }, Qt::QueuedConnection);
}

QVariantMap SepsQml::resolveFederation(const QString& address)
{
    const int id = nextRequestId();
    QVariantMap out;
    out.insert(QStringLiteral("requestId"), id);

    // Every refusal before a request goes out answers twice: in this map, at
    // once, and on failed() for a caller that only listens. A blank value used
    // to come back from the SDK as the "account id" "" with no reply behind
    // it, and neither signal ever came.
    auto refuse = [&](const QString& kind, const QString& message) {
        out.insert(QStringLiteral("error"), message);
        failWith(id, kind, message);
        return out;
    };

    FederationResponse* response = nullptr;
    try {
        // Throws on a blank value, more than one '*', an empty name or a
        // domain that is not a host name, and a bare value that is not an
        // account id.
        response = Federation::resolve(address.trimmed());
    } catch (const std::exception& e) {
        return refuse(QStringLiteral("validation"), QString::fromUtf8(e.what()));
    } catch (...) {
        return refuse(QStringLiteral("unknown"), QStringLiteral("the address could not be read"));
    }
    if (!response)
        return refuse(QStringLiteral("unknown"), QStringLiteral("the SDK returned no response object"));

    // A bare account id comes back already answered and with no reply behind
    // it, so no signal will ever arrive. Answer now instead of hanging.
    if (!response->getAccountId().isEmpty()
            && response->getStellarAddress().isEmpty()) {
        QVariantMap result;
        result.insert(QStringLiteral("accountId"), response->getAccountId());
        result.insert(QStringLiteral("resolved"), false);
        result.insert(QStringLiteral("note"),
                      QStringLiteral("this was already an account id \u2014 nothing "
                                     "was looked up"));
        response->deleteLater();
        // Same reason as failWith: the caller does not have `id` yet.
        QMetaObject::invokeMethod(this, [this, id, result]() {
            emit resolved(id, result);
        }, Qt::QueuedConnection);
        return out;
    }

    m_inFlight.insert(id, response);
    emit pendingChanged();

    connect(response, &Response::ready, this, [this, id, response]() {
        if (!m_inFlight.contains(id))
            return;
        QVariantMap result;
        result.insert(QStringLiteral("stellarAddress"), response->getStellarAddress());
        result.insert(QStringLiteral("accountId"), response->getAccountId());
        result.insert(QStringLiteral("memoType"), response->getMemoType());
        result.insert(QStringLiteral("memo"), response->getMemo());
        result.insert(QStringLiteral("resolved"), true);
        m_inFlight.remove(id);
        response->deleteLater();
        emit pendingChanged();
        emit resolved(id, result);
    });

    connect(response, &Response::error, this, [this, id, response]() {
        if (!m_inFlight.contains(id))
            return;
        m_inFlight.remove(id);
        emit pendingChanged();
        // An answer that came and was refused (an account id that is not one,
        // a memo invalid for its type) is a "badResponse". Every other failure
        // carries its reason as well, and says it: a stellar.toml that could
        // not be read, or named no https FEDERATION_SERVER (a
        // FederationException since 1.0.4), the federation server's HTTP
        // error, the time limit. It was one fixed text for all of them, so a
        // 404 toml and a missing FEDERATION_SERVER read the same.
        const qstellar::exception::SdkException* why = response->lastException();
        if (response->getStatus() == 200 && why) {
            failWith(id, QStringLiteral("badResponse"),
                     QStringLiteral("the federation server's answer was refused: %1")
                         .arg(QString::fromUtf8(why->what())));
        } else {
            QString reason = why ? QString::fromUtf8(why->what()) : QString();
            // "Bad Request." says less than the status behind it: a federation
            // server answers a name it does not know with 404.
            const auto* http = dynamic_cast<const qstellar::exception::NetworkException*>(why);
            if (http && http->getCode() > 0)
                reason = QStringLiteral("the federation server answered HTTP %1").arg(http->getCode());
            if (reason.isEmpty())
                reason = QStringLiteral("the domain or its federation server gave no answer");
            failWith(id, QStringLiteral("connection"), QStringLiteral("the lookup failed: ") + reason);
        }
        response->deleteLater();
    });

    return out;
}

int SepsQml::pending() const
{
    return m_inFlight.size();
}

QSTELLAR_END_NS
