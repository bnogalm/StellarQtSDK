#include "txrep.h"
#include <QTimeZone>
#include <QDateTime>
#include <memory>
#include <limits>
#include <vector>

#include <QHash>
#include <QSet>
#include <QStringList>
#include <QTextStream>
#include <QDate>
#include <QRegularExpression>
#include <QCryptographicHash>
#include <cstring>
#include <stdexcept>

#include "transaction.h"
#include "feebumptransaction.h"
#include "transactionbuilder.h"
#include "util.h"
#include "network.h"
#include "keypair.h"
#include "account.h"
#include "memo.h"
#include "managedataoperation.h"
#include "paymentoperation.h"
#include "createaccountoperation.h"
#include "accountmergeoperation.h"
#include "bumpsequenceoperation.h"
#include "setoptionsoperation.h"
#include "changetrustoperation.h"
#include "changetrustasset.h"
#include "liquiditypool.h"
#include "inflationoperation.h"
#include "muxedaccount.h"
#include "allowtrustoperation.h"
#include "managesellofferoperation.h"
#include "managebuyofferoperation.h"
#include "createpassivesellofferoperation.h"
#include "pathpaymentoperation.h"
#include "pathpaymentstrictreceiveoperation.h"
#include "pathpaymentstrictsendoperation.h"
#include "beginsponsoringfuturereservesoperation.h"
#include "endsponsoringfuturereservesoperation.h"
#include "clawbackoperation.h"
#include "clawbackclaimablebalanceoperation.h"
#include "settrustlineflagsoperation.h"
#include "claimclaimablebalanceoperation.h"
#include "liquiditypooldepositoperation.h"
#include "liquiditypoolwithdrawoperation.h"
#include "createclaimablebalanceoperation.h"
#include "claimant.h"
#include "predicate.h"
#include "revokeaccountsponsorshipoperation.h"
#include "revoketrustlinesponsorshipoperation.h"
#include "revokeoffersponsorshipoperation.h"
#include "revokedatasponsorshipoperation.h"
#include "revokeclaimablebalancesponsorshipoperation.h"
#include "revokeliquiditypoolsponsorshipoperation.h"
#include "revokesignersponsorshipoperation.h"
#include "price.h"
#include "asset.h"
#include "assettypenative.h"
#include "assettypecreditalphanum4.h"
#include "assettypecreditalphanum12.h"
#include "timebounds.h"
#include "ledgerbounds.h"
#include "signerkey.h"
#include "transactionpreconditions.h"

QSTELLAR_BEGIN_NS


namespace {

void appendLine(QString& out, const QString& key, const QString& value)
{
    out.append(key).append(": ").append(value).append('\n');
}

/**
 * An amount in stroops followed by what it is in units, as py-stellar-sdk
 * writes it: "125000000 (12.5)". The units are py's (Python's Decimal):
 * trailing zeros dropped, and 1 to 9 stroops written "1E-7" ... "9E-7".
 */
QString withUnits(qint64 stroops)
{
    const bool negative = stroops < 0;
    // As unsigned: the most negative int64 has no positive counterpart.
    const quint64 magnitude = negative ? quint64(0) - quint64(stroops) : quint64(stroops);
    QString units;
    if (magnitude > 0 && magnitude < 10) {
        units = QString::number(magnitude) + QStringLiteral("E-7");
    } else {
        units = QString::number(magnitude / 10000000u);
        QString fraction = QStringLiteral("%1").arg(magnitude % 10000000u, 7, 10, QLatin1Char('0'));
        while (fraction.endsWith(QLatin1Char('0')))
            fraction.chop(1);
        if (!fraction.isEmpty())
            units += QLatin1Char('.') + fraction;
    }
    if (negative)
        units.prepend(QLatin1Char('-'));
    return QString::number(stroops) + QStringLiteral(" (") + units + QLatin1Char(')');
}

/**
 * A time in seconds followed by the date it is in UTC, as py-stellar-sdk
 * writes it: "1700000000 (2023-11-14 22:13:20.000000+00:00 (UTC))", 0
 * included. Past year 9999, which py cannot write at all, the number alone.
 */
QString withUtc(quint64 seconds)
{
    static const quint64 lastSecondOf9999 = 253402300799ull;
    if (seconds > lastSecondOf9999)
        return QString::number(seconds);
    const QDateTime when = QDateTime::fromSecsSinceEpoch(qint64(seconds), QTimeZone::utc());
    return QString::number(seconds) + QStringLiteral(" (")
           + when.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")) + QStringLiteral(".000000+00:00 (UTC))");
}

QString hex(const QByteArray& bytes) { return QString::fromLatin1(bytes.toHex()); }

/**
 * A character a reader cannot see, or that breaks or reorders the line it is
 * on: C0/C1 controls (but TAB), DEL, zero-width and direction marks (U+061C,
 * U+200B-200F), the line and paragraph separators, the bidi embeddings,
 * overrides and isolates, U+FEFF, and the other format characters that draw
 * nothing: the soft hyphen, the combining grapheme joiner, the Hangul and
 * Mongolian fillers and vowel separator, the Khmer inherent vowels, the
 * interlinear annotation marks, the shorthand and musical format controls
 * and the language tags (U+E0000-E007F). In a document someone reviews before
 * signing, each of these makes the screen say something else than the bytes:
 * a right-to-left override shows "87654321" as "12345678", a soft hyphen
 * shows "123<U+00AD>456" as "123456". The table is explicit (not the Unicode
 * category), so Qt 5 and Qt 6 agree. Variation selectors are not in it: they
 * only pick a glyph's presentation, and real memos carry them (emoji).
 * `c` is a whole code point.
 */
bool isHiddenOrBreaking(uint c)
{
    return (c < 0x20 && c != '\t') || (c >= 0x7f && c <= 0x9f) || c == 0xad || c == 0x34f
           || c == 0x61c || c == 0x115f || c == 0x1160 || c == 0x17b4 || c == 0x17b5
           || (c >= 0x180b && c <= 0x180f) || (c >= 0x200b && c <= 0x200f)
           || (c >= 0x2028 && c <= 0x202e) || (c >= 0x2060 && c <= 0x206f) || c == 0x3164
           || c == 0xfeff || c == 0xffa0 || (c >= 0xfff9 && c <= 0xfffb)
           || (c >= 0x1bca0 && c <= 0x1bca3) || (c >= 0x1d173 && c <= 0x1d17a)
           || (c >= 0xe0000 && c <= 0xe007f);
}

/** The code point at `i` of `s` (a surrogate pair read whole), and its length in `units`. */
uint codePointAt(const QString& s, int i, int* units)
{
    const QChar c = s.at(i);
    if (c.isHighSurrogate() && i + 1 < s.size() && s.at(i + 1).isLowSurrogate()) {
        *units = 2;
        return QChar::surrogateToUcs4(c, s.at(i + 1));
    }
    *units = 1;
    return c.unicode();
}

bool hasUnpairedSurrogate(const QString& s)
{
    for (int i = 0; i < s.size(); ++i) {
        const QChar c = s.at(i);
        if (c.isHighSurrogate()) {
            if (i + 1 >= s.size() || !s.at(i + 1).isLowSurrogate())
                return true;
            ++i;
        } else if (c.isLowSurrogate()) {
            return true;
        }
    }
    return false;
}

/**
 * Whether `bytes` are UTF-8: well-formed sequences, no overlong forms, no
 * surrogates, nothing above U+10FFFF. A scan of the bytes: the check used to
 * be a round trip through QString::fromUtf8, which drops a leading byte-order
 * mark, so "\xEF\xBB\xBFhi" counted as not UTF-8 (a home domain refused, a
 * memo written as hex where py writes the text).
 */
bool isValidUtf8(const QByteArray& bytes)
{
    const int size = bytes.size();
    for (int i = 0; i < size;) {
        const uchar c = uchar(bytes.at(i));
        int extra = 0;
        uint codePoint = 0;
        if (c < 0x80) { ++i; continue; }
        else if (c >= 0xc2 && c <= 0xdf) { extra = 1; codePoint = c & 0x1f; }
        else if (c >= 0xe0 && c <= 0xef) { extra = 2; codePoint = c & 0x0f; }
        else if (c >= 0xf0 && c <= 0xf4) { extra = 3; codePoint = c & 0x07; }
        else return false;
        if (i + extra >= size)
            return false;
        for (int k = 1; k <= extra; ++k) {
            const uchar cc = uchar(bytes.at(i + k));
            if ((cc & 0xc0) != 0x80)
                return false;
            codePoint = (codePoint << 6) | (cc & 0x3f);
        }
        if ((extra == 2 && codePoint < 0x800) || (extra == 3 && codePoint < 0x10000)
                || (codePoint >= 0xd800 && codePoint <= 0xdfff) || codePoint > 0x10ffff)
            return false;
        i += extra + 1;
    }
    return true;
}

/** UTF-8 `bytes` as text, leading byte-order marks kept (QString::fromUtf8 drops one). */
QString utf8Text(const QByteArray& bytes)
{
    return Util::fromUtf8KeepingBom(bytes);
}

/** An asset code as core takes it: 1 to 12 of A-Z, a-z, 0-9. */
QString checkedAssetCode(const QString& code)
{
    bool ok = !code.isEmpty() && code.size() <= 12;
    for (const QChar c : code) {
        const ushort u = c.unicode();
        if (!((u >= 'A' && u <= 'Z') || (u >= 'a' && u <= 'z') || (u >= '0' && u <= '9')))
            ok = false;
    }
    if (!ok)
        throw std::runtime_error(("txrep: an asset code is 1 to 12 letters and digits, not \""
                                  + code + "\"").toStdString());
    return code;
}

/**
 * A string value, JSON-quoted: backslash, quote, and every character that
 * could hide or break the line (see isHiddenOrBreaking) are escaped, so a
 * newline or a bidi override in a memo cannot change what the document
 * shows. Everything else is written as it is, in UTF-8.
 */
QString quoted(const QString& s)
{
    QString out(QLatin1Char('"'));
    for (int i = 0; i < s.size();) {
        int units = 1;
        const uint c = codePointAt(s, i, &units);
        switch (c) {
        case '"':  out += QLatin1String("\\\""); break;
        case '\\': out += QLatin1String("\\\\"); break;
        case '\n': out += QLatin1String("\\n"); break;
        case '\r': out += QLatin1String("\\r"); break;
        case '\t': out += QLatin1String("\\t"); break;
        default:
            if (isHiddenOrBreaking(c)) {
                // One \uXXXX per UTF-16 unit: a surrogate pair as two, as JSON does.
                for (int k = 0; k < units; ++k)
                    out += QString::asprintf("\\u%04x", s.at(i + k).unicode());
            } else {
                out += s.mid(i, units);
            }
        }
        i += units;
    }
    out += QLatin1Char('"');
    return out;
}

/**
 * What a value was read as, which says what a comment after it may state
 * (checkComments). Any other value takes no comment.
 */
enum class ValueKind
{
    Other,      // a word, an asset, hex, a string, a key: no comment
    Number,     // an integer: the same number (an amount or a time, by its name, more)
    Account,    // a G... or M... address: checked where it is read
    Hint        // a signature hint: checked against the signature, once it is built
};

/**
 * The document's `key: value` lines.
 *
 * It refuses a key given twice: a second `tx.fee` line used to win over the
 * first, the one a reviewer reads. And it remembers which keys the parser
 * read, so that a line nothing reads (a memo text under MEMO_NONE, a source
 * account marked absent, a field of an arm the type does not select) is
 * refused instead of silently dropped from what gets signed.
 */
class TxrepMap
{
public:
    void insert(const QString& key, const QString& value, const QString& raw)
    {
        if (m_kv.contains(key))
            throw std::runtime_error(("txrep: key given twice: " + key).toStdString());
        m_kv.insert(key, value);
        m_raw.insert(key, raw);
    }
    /** The value as written, before a trailing ` (comment)` was taken off. */
    QString raw(const QString& key) const { return m_raw.value(key); }
    /** The value, without marking the key as read (for the checks on comments). */
    QString peek(const QString& key) const { return m_kv.value(key); }
    /** Whether the line is there, without marking it as read (to tell two forms apart). */
    bool has(const QString& key) const { return m_kv.contains(key); }
    /** Says what the parser read a value as, for the checks on its comment. */
    void setKind(const QString& key, ValueKind kind) const { m_kind.insert(key, kind); }
    ValueKind kind(const QString& key) const { return m_kind.value(key, ValueKind::Other); }
    /** stc's name for the native asset of the network read for ("TestXLM"), or empty. */
    void setNativeName(const QString& name) { m_nativeName = name; }
    QString nativeName() const { return m_nativeName; }
    const QString* find(const QString& key) const
    {
        m_read.insert(key);
        const auto it = m_kv.constFind(key);
        return it == m_kv.constEnd() ? nullptr : &it.value();
    }
    QString value(const QString& key) const
    {
        const QString* v = find(key);
        return v ? *v : QString();
    }
    QStringList keys() const { return m_kv.keys(); }
    /** Marks keys as read that the parser deliberately does not carry over. */
    void markRead(const QString& key) const { m_read.insert(key); }
    QStringList unread() const
    {
        QStringList out;
        for (auto it = m_kv.constBegin(); it != m_kv.constEnd(); ++it) {
            if (!m_read.contains(it.key()))
                out.append(it.key());
        }
        out.sort();
        return out;
    }

private:
    QHash<QString, QString> m_kv;
    QHash<QString, QString> m_raw;
    mutable QSet<QString> m_read;
    mutable QHash<QString, ValueKind> m_kind;
    QString m_nativeName;
};

QString assetToTxrep(Asset* a)
{
    if (!a) throw std::runtime_error("asset cannot be null");
    // SEP-11 / stc / py-stellar-sdk render the native asset as "native"
    // (NOT "XLM") — emit that for cross-tool interoperability. The parser
    // still accepts both spellings.
    if (dynamic_cast<AssetTypeNative*>(a)) return QStringLiteral("native");
    auto* alpha = dynamic_cast<AssetTypeCreditAlphaNum*>(a);
    if (!alpha) throw std::runtime_error("unsupported asset type");
    return alpha->getCode() + ":" + alpha->getIssuer().getAccountId();
}

// The XDR asset-type discriminator string (used by ChangeTrust's `line.type`).
QString assetTypeString(Asset* a)
{
    if (dynamic_cast<AssetTypeNative*>(a))            return QStringLiteral("ASSET_TYPE_NATIVE");
    if (dynamic_cast<AssetTypeCreditAlphaNum4*>(a))   return QStringLiteral("ASSET_TYPE_CREDIT_ALPHANUM4");
    if (dynamic_cast<AssetTypeCreditAlphaNum12*>(a))  return QStringLiteral("ASSET_TYPE_CREDIT_ALPHANUM12");
    throw std::runtime_error("unsupported asset type");
}

Asset* assetFromTxrep(const QString& s)
{
    if (s == "XLM" || s == "native") return new AssetTypeNative();
    int colon = s.indexOf(':');
    if (colon < 0) throw std::runtime_error("malformed asset in txrep");
    return Asset::create(checkedAssetCode(s.left(colon)), s.mid(colon + 1));
}

/**
 * py-stellar-sdk's comment on a muxed account: the account and the id an
 * M... address carries, which the address itself does not show a reader.
 * Empty for a G... address.
 */
QString muxedComment(const QString& address)
{
    const MuxedAccount account = MuxedAccount::fromAddress(address);
    if (!account.isMuxed())
        return QString();
    return QStringLiteral("(accountID: ") + account.getAccountId()
           + QStringLiteral(", accountMuxedID: ") + QString::number(account.getMuxedId())
           + QLatin1Char(')');
}

/**
 * A MuxedAccount field (a source account, a payment destination...): the
 * G... or M... address, and after an M... address the comment above, as
 * py-stellar-sdk writes it.
 */
void appendAccountLine(QString& out, const QString& key, const QString& address)
{
    const QString comment = muxedComment(address);
    appendLine(out, key, comment.isEmpty() ? address : address + QLatin1Char(' ') + comment);
}

/** A SignerKeyType's XDR name, and the field its strkey is written under. */
struct SignerKeyForm
{
    stellar::SignerKeyType type;
    const char* name;
    const char* field;
};

const SignerKeyForm kSignerKeyForms[] = {
    {stellar::SignerKeyType::SIGNER_KEY_TYPE_ED25519, "SIGNER_KEY_TYPE_ED25519", "ed25519"},
    {stellar::SignerKeyType::SIGNER_KEY_TYPE_PRE_AUTH_TX, "SIGNER_KEY_TYPE_PRE_AUTH_TX", "preAuthTx"},
    {stellar::SignerKeyType::SIGNER_KEY_TYPE_HASH_X, "SIGNER_KEY_TYPE_HASH_X", "hashX"},
    {stellar::SignerKeyType::SIGNER_KEY_TYPE_ED25519_SIGNED_PAYLOAD,
     "SIGNER_KEY_TYPE_ED25519_SIGNED_PAYLOAD", "ed25519SignedPayload"},
};

/**
 * A SignerKey as py-stellar-sdk writes an extra signer or a revoked signer:
 * `.type`, and the strkey under the field that type names.
 */
void emitSignerKey(QString& out, const QString& path, const SignerKey& key)
{
    for (const SignerKeyForm& form : kSignerKeyForms) {
        if (form.type == key.getType()) {
            appendLine(out, path + QStringLiteral(".type"), QLatin1String(form.name));
            appendLine(out, path + QLatin1Char('.') + QLatin1String(form.field), key.toString());
            return;
        }
    }
    throw std::runtime_error("Txrep emitter: unknown signer key type");
}

/**
 * Whether py-stellar-sdk reads `s` back exactly when it is written bare, as
 * py writes a revoke-data name: it keeps a bare value up to its first space,
 * and takes a quote as part of the name.
 */
bool readsBackBare(const QString& s)
{
    if (s.isEmpty() || s.startsWith(QLatin1Char('"')) || hasUnpairedSurrogate(s))
        return false;
    for (int i = 0; i < s.size();) {
        int units = 1;
        const uint c = codePointAt(s, i, &units);
        if (QChar::isSpace(c) || isHiddenOrBreaking(c))
            return false;
        i += units;
    }
    return true;
}

/**
 * A claim predicate's time with py-stellar-sdk's gloss (withUtc): absBefore's
 * date in UTC, and for relBefore, a number of seconds after the ledger
 * closes, the date that many seconds after 1970, as py writes it (and as the
 * reader checks it). Below 0, which stellar-core refuses (validatePredicate)
 * and py dates in 1969, the number alone.
 */
QString predicateTime(qint64 seconds)
{
    return seconds < 0 ? QString::number(seconds) : withUtc(quint64(seconds));
}

// Recursively emit a claim predicate tree (CreateClaimableBalance claimants).
void emitPredicate(QString& out, const QString& path, const Predicate& pred)
{
    if (dynamic_cast<const Predicate::Unconditional*>(&pred)) {
        appendLine(out, path + ".type", "CLAIM_PREDICATE_UNCONDITIONAL");
    } else if (auto* ab = dynamic_cast<const Predicate::AbsBefore*>(&pred)) {
        appendLine(out, path + ".type", "CLAIM_PREDICATE_BEFORE_ABSOLUTE_TIME");
        appendLine(out, path + ".absBefore",
                   predicateTime(const_cast<Predicate::AbsBefore*>(ab)->getTimestampSeconds()));
    } else if (auto* rb = dynamic_cast<const Predicate::RelBefore*>(&pred)) {
        appendLine(out, path + ".type", "CLAIM_PREDICATE_BEFORE_RELATIVE_TIME");
        appendLine(out, path + ".relBefore",
                   predicateTime(const_cast<Predicate::RelBefore*>(rb)->getSecondsSinceClose()));
    } else if (auto* notp = dynamic_cast<const Predicate::Not*>(&pred)) {
        appendLine(out, path + ".type", "CLAIM_PREDICATE_NOT");
        appendLine(out, path + ".notPredicate._present", "true");
        emitPredicate(out, path + ".notPredicate", notp->getInner());
    } else if (auto* andp = dynamic_cast<const Predicate::And*>(&pred)) {
        appendLine(out, path + ".type", "CLAIM_PREDICATE_AND");
        const QList<Predicate*> inner = andp->getInner();
        appendLine(out, path + ".andPredicates.len", QString::number(inner.size()));
        for (int j = 0; j < inner.size(); ++j)
            emitPredicate(out, path + QString(".andPredicates[%1]").arg(j), *inner.at(j));
    } else if (auto* orp = dynamic_cast<const Predicate::Or*>(&pred)) {
        appendLine(out, path + ".type", "CLAIM_PREDICATE_OR");
        const QList<Predicate*> inner = orp->getInner();
        appendLine(out, path + ".orPredicates.len", QString::number(inner.size()));
        for (int j = 0; j < inner.size(); ++j)
            emitPredicate(out, path + QString(".orPredicates[%1]").arg(j), *inner.at(j));
    } else {
        throw std::runtime_error("Txrep emitter: unknown claim predicate type");
    }
}

// AllowTrust is deprecated (Protocol 17's SetTrustLineFlags supersedes it), but
// its XDR operation still appears in pre-P17 transactions, so txrep must still
// round-trip it. Isolate the deprecated-symbol use in these two helpers so the
// warning is suppressed in exactly one place, cross-compiler.
#if defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable: 4996)
#elif defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
/**
 * The deprecated PathPaymentOperation, which is what Operation::fromXdr hands
 * back for PATH_PAYMENT_STRICT_RECEIVE (operation.cpp) even though
 * PathPaymentStrictReceiveOperation is the class everything else uses.
 *
 * The two are siblings under Operation, not related to each other, so the
 * dynamic_cast in the main chain misses this one entirely. Without this branch
 * any transaction the SDK decoded from XDR and then tried to emit as TxRep
 * failed with "unsupported operation type" — for an operation the SDK fully
 * supports and had just written itself.
 *
 * Emits exactly what the non-deprecated branch emits: it is the same operation.
 */
bool emitDeprecatedPathPayment(QString& out, const QString& prefix, Operation* op)
{
    auto* pp = dynamic_cast<PathPaymentOperation*>(op);
    if (!pp) return false;
    appendLine(out, prefix + ".body.type", "PATH_PAYMENT_STRICT_RECEIVE");
    const QString p = prefix + ".body.pathPaymentStrictReceiveOp";
    appendLine(out, p + ".sendAsset",   assetToTxrep(pp->getSendAsset()));
    appendLine(out, p + ".sendMax",     withUnits(Operation::toXdrAmount(pp->getSendMax())));
    appendAccountLine(out, p + ".destination", pp->getDestination());
    appendLine(out, p + ".destAsset",   assetToTxrep(pp->getDestAsset()));
    appendLine(out, p + ".destAmount",  withUnits(Operation::toXdrAmount(pp->getDestAmount())));
    const QList<Asset*> path = pp->getPath();
    appendLine(out, p + ".path.len", QString::number(path.size()));
    for (int k = 0; k < path.size(); ++k)
        appendLine(out, p + QString(".path[%1]").arg(k), assetToTxrep(path.at(k)));
    return true;
}

bool emitAllowTrust(QString& out, const QString& prefix, Operation* op)
{
    auto* at = dynamic_cast<AllowTrustOperation*>(op);
    if (!at) return false;
    appendLine(out, prefix + ".body.type", "ALLOW_TRUST");
    appendLine(out, prefix + ".body.allowTrustOp.trustor", at->getTrustor()->getAccountId());
    appendLine(out, prefix + ".body.allowTrustOp.asset", at->getAssetCode());
    // py-stellar-sdk / stc render `authorize` as a bool for the common 0/1
    // case. Preserve the rarer AUTHORIZED_TO_MAINTAIN_LIABILITIES flag (2)
    // numerically (no bool covers it).
    if (at->getAuthorize())
        appendLine(out, prefix + ".body.allowTrustOp.authorize", "true");
    else if (at->getAuthorizeToMaintainLiabilities())
        appendLine(out, prefix + ".body.allowTrustOp.authorize", "2");
    else
        appendLine(out, prefix + ".body.allowTrustOp.authorize", "false");
    return true;
}
#if defined(_MSC_VER)
#  pragma warning(pop)
#elif defined(__GNUC__)
#  pragma GCC diagnostic pop
#endif

/** Operation `i` of the transaction written under `tx` ("tx", or a fee bump's inner one). */
void emitOperation(QString& out, const QString& tx, Operation* op, int i)
{
    const QString prefix = tx + QString(".operations[%1]").arg(i);
    if (op->getSourceAccount().isEmpty()) {
        appendLine(out, prefix + ".sourceAccount._present", "false");
    } else {
        appendLine(out, prefix + ".sourceAccount._present", "true");
        appendAccountLine(out, prefix + ".sourceAccount", op->getSourceAccount());
    }

    if (auto* md = dynamic_cast<ManageDataOperation*>(op)) {
        appendLine(out, prefix + ".body.type", "MANAGE_DATA");
        appendLine(out, prefix + ".body.manageDataOp.dataName", quoted(md->getName()));
        const QByteArray value = md->getValue();
        if (value.isNull()) {                    // absent: a delete; empty: an empty value
            appendLine(out, prefix + ".body.manageDataOp.dataValue._present", "false");
        } else {
            appendLine(out, prefix + ".body.manageDataOp.dataValue._present", "true");
            appendLine(out, prefix + ".body.manageDataOp.dataValue", hex(value));
        }
    } else if (auto* pay = dynamic_cast<PaymentOperation*>(op)) {
        appendLine(out, prefix + ".body.type", "PAYMENT");
        appendAccountLine(out, prefix + ".body.paymentOp.destination", pay->getDestination());
        appendLine(out, prefix + ".body.paymentOp.asset", assetToTxrep(pay->getAsset()));
        appendLine(out, prefix + ".body.paymentOp.amount",
                   withUnits(Operation::toXdrAmount(pay->getAmount())));
    } else if (auto* ca = dynamic_cast<CreateAccountOperation*>(op)) {
        appendLine(out, prefix + ".body.type", "CREATE_ACCOUNT");
        appendLine(out, prefix + ".body.createAccountOp.destination",
                   ca->getDestination()->getAccountId());
        appendLine(out, prefix + ".body.createAccountOp.startingBalance",
                   withUnits(Operation::toXdrAmount(ca->getStartingBalance())));
    } else if (auto* am = dynamic_cast<AccountMergeOperation*>(op)) {
        // ACCOUNT_MERGE carries its destination directly in the body union
        // (no per-op struct), per the XDR / SEP-11 layout.
        appendLine(out, prefix + ".body.type", "ACCOUNT_MERGE");
        appendAccountLine(out, prefix + ".body.destination", am->getDestination());
    } else if (auto* bs = dynamic_cast<BumpSequenceOperation*>(op)) {
        appendLine(out, prefix + ".body.type", "BUMP_SEQUENCE");
        appendLine(out, prefix + ".body.bumpSequenceOp.bumpTo", QString::number(bs->getBumpTo()));
    } else if (auto* so = dynamic_cast<SetOptionsOperation*>(op)) {
        appendLine(out, prefix + ".body.type", "SET_OPTIONS");
        const QString p = prefix + ".body.setOptionsOp";
        auto emitOptUint = [&](const QString& field, Integer opt) {
            if (opt.filled) {
                appendLine(out, p + "." + field + "._present", "true");
                appendLine(out, p + "." + field, QString::number(opt.value));
            } else {
                appendLine(out, p + "." + field + "._present", "false");
            }
        };
        KeyPair* inflation = so->getInflationDestination();
        if (inflation) {
            appendLine(out, p + ".inflationDest._present", "true");
            appendLine(out, p + ".inflationDest", inflation->getAccountId());
        } else {
            appendLine(out, p + ".inflationDest._present", "false");
        }
        emitOptUint("clearFlags",    so->getClearFlags());
        emitOptUint("setFlags",      so->getSetFlags());
        emitOptUint("masterWeight",  so->getMasterKeyWeight());
        emitOptUint("lowThreshold",  so->getLowThreshold());
        emitOptUint("medThreshold",  so->getMediumThreshold());
        emitOptUint("highThreshold", so->getHighThreshold());
        // The signed bytes: getHomeDomain() decodes IDN, and a lookalike
        // xn-- domain would be shown as the name it imitates.
        const QByteArray homeDomain = so->getHomeDomainBytes();
        if (!homeDomain.isNull()) {
            if (!isValidUtf8(homeDomain))
                throw std::runtime_error("Txrep emitter: a home domain that is not UTF-8 text");
            appendLine(out, p + ".homeDomain._present", "true");
            appendLine(out, p + ".homeDomain", quoted(utf8Text(homeDomain)));
        } else {
            appendLine(out, p + ".homeDomain._present", "false");
        }
        stellar::SignerKey* signer = so->getSigner();
        if (signer) {
            appendLine(out, p + ".signer._present", "true");
            appendLine(out, p + ".signer.key", SignerKey::fromXdr(*signer).toString());
            appendLine(out, p + ".signer.weight", QString::number(so->getSignerWeight().value));
        } else {
            appendLine(out, p + ".signer._present", "false");
        }
    } else if (auto* ct = dynamic_cast<ChangeTrustOperation*>(op)) {
        const QString p = prefix + ".body.changeTrustOp";
        Asset* line = ct->getAsset();  // null for the POOL_SHARE (liquidity pool) variant
        appendLine(out, prefix + ".body.type", "CHANGE_TRUST");
        if (line) {
            appendLine(out, p + ".line.type", assetTypeString(line));
            appendLine(out, p + ".line", assetToTxrep(line));
        } else {
            // Shares of a pool, named by its parameters, as py-stellar-sdk
            // writes them (the pool type, constant product, is the only one).
            auto* pool = dynamic_cast<LiquidityPoolShareChangeTrustAsset*>(ct->getLine());
            if (!pool)
                throw std::runtime_error("Txrep emitter: unsupported ChangeTrust line");
            appendLine(out, p + ".line.type", "ASSET_TYPE_POOL_SHARE");
            appendLine(out, p + ".line.liquidityPool.constantProduct.assetA", assetToTxrep(pool->getAssetA()));
            appendLine(out, p + ".line.liquidityPool.constantProduct.assetB", assetToTxrep(pool->getAssetB()));
            appendLine(out, p + ".line.liquidityPool.constantProduct.fee", QString::number(pool->getFee()));
        }
        appendLine(out, p + ".limit", withUnits(Operation::toXdrAmount(ct->getLimit())));
    } else if (emitAllowTrust(out, prefix, op)) {
        // ALLOW_TRUST handled by the helper (isolates the deprecated symbol).
    } else if (auto* mso = dynamic_cast<ManageSellOfferOperation*>(op)) {
        appendLine(out, prefix + ".body.type", "MANAGE_SELL_OFFER");
        const QString p = prefix + ".body.manageSellOfferOp";
        const Price pr = mso->getPriceR();   // exact fraction from the XDR
        appendLine(out, p + ".selling", assetToTxrep(mso->getSelling()));
        appendLine(out, p + ".buying",  assetToTxrep(mso->getBuying()));
        appendLine(out, p + ".amount",  withUnits(Operation::toXdrAmount(mso->getAmount())));
        appendLine(out, p + ".price.n", QString::number(pr.getNumerator()));
        appendLine(out, p + ".price.d", QString::number(pr.getDenominator()));
        appendLine(out, p + ".offerID", QString::number(mso->getOfferId()));
    } else if (auto* mbo = dynamic_cast<ManageBuyOfferOperation*>(op)) {
        appendLine(out, prefix + ".body.type", "MANAGE_BUY_OFFER");
        const QString p = prefix + ".body.manageBuyOfferOp";
        const Price pr = mbo->getPriceR();   // exact fraction from the XDR
        appendLine(out, p + ".selling",   assetToTxrep(mbo->getSelling()));
        appendLine(out, p + ".buying",    assetToTxrep(mbo->getBuying()));
        appendLine(out, p + ".buyAmount", withUnits(Operation::toXdrAmount(mbo->getAmount())));
        appendLine(out, p + ".price.n",   QString::number(pr.getNumerator()));
        appendLine(out, p + ".price.d",   QString::number(pr.getDenominator()));
        appendLine(out, p + ".offerID",   QString::number(mbo->getOfferId()));
    } else if (auto* cpo = dynamic_cast<CreatePassiveSellOfferOperation*>(op)) {
        appendLine(out, prefix + ".body.type", "CREATE_PASSIVE_SELL_OFFER");
        const QString p = prefix + ".body.createPassiveSellOfferOp";
        const Price pr = cpo->getPriceR();   // exact fraction from the XDR
        appendLine(out, p + ".selling", assetToTxrep(cpo->getSelling()));
        appendLine(out, p + ".buying",  assetToTxrep(cpo->getBuying()));
        appendLine(out, p + ".amount",  withUnits(Operation::toXdrAmount(cpo->getAmount())));
        appendLine(out, p + ".price.n", QString::number(pr.getNumerator()));
        appendLine(out, p + ".price.d", QString::number(pr.getDenominator()));
    } else if (auto* ppr = dynamic_cast<PathPaymentStrictReceiveOperation*>(op)) {
        appendLine(out, prefix + ".body.type", "PATH_PAYMENT_STRICT_RECEIVE");
        const QString p = prefix + ".body.pathPaymentStrictReceiveOp";
        appendLine(out, p + ".sendAsset",  assetToTxrep(ppr->getSendAsset()));
        appendLine(out, p + ".sendMax",    withUnits(Operation::toXdrAmount(ppr->getSendMax())));
        appendAccountLine(out, p + ".destination", ppr->getDestination());
        appendLine(out, p + ".destAsset",  assetToTxrep(ppr->getDestAsset()));
        appendLine(out, p + ".destAmount", withUnits(Operation::toXdrAmount(ppr->getDestAmount())));
        const QList<Asset*> path = ppr->getPath();
        appendLine(out, p + ".path.len", QString::number(path.size()));
        for (int k = 0; k < path.size(); ++k) {
            appendLine(out, p + QString(".path[%1]").arg(k), assetToTxrep(path.at(k)));
        }
    } else if (emitDeprecatedPathPayment(out, prefix, op)) {
        // Same operation, older class — see the helper.
    } else if (auto* pps = dynamic_cast<PathPaymentStrictSendOperation*>(op)) {
        appendLine(out, prefix + ".body.type", "PATH_PAYMENT_STRICT_SEND");
        const QString p = prefix + ".body.pathPaymentStrictSendOp";
        appendLine(out, p + ".sendAsset",   assetToTxrep(pps->getSendAsset()));
        appendLine(out, p + ".sendAmount",  withUnits(Operation::toXdrAmount(pps->getSendAmount())));
        appendAccountLine(out, p + ".destination", pps->getDestination());
        appendLine(out, p + ".destAsset",   assetToTxrep(pps->getDestAsset()));
        appendLine(out, p + ".destMin",     withUnits(Operation::toXdrAmount(pps->getDestMin())));
        const QList<Asset*> path = pps->getPath();
        appendLine(out, p + ".path.len", QString::number(path.size()));
        for (int k = 0; k < path.size(); ++k) {
            appendLine(out, p + QString(".path[%1]").arg(k), assetToTxrep(path.at(k)));
        }
    } else if (auto* bsp = dynamic_cast<BeginSponsoringFutureReservesOperation*>(op)) {
        appendLine(out, prefix + ".body.type", "BEGIN_SPONSORING_FUTURE_RESERVES");
        appendLine(out, prefix + ".body.beginSponsoringFutureReservesOp.sponsoredID",
                   bsp->getSponsoredID());
    } else if (dynamic_cast<EndSponsoringFutureReservesOperation*>(op)) {
        // END_SPONSORING_FUTURE_RESERVES carries no fields.
        appendLine(out, prefix + ".body.type", "END_SPONSORING_FUTURE_RESERVES");
    } else if (auto* cb = dynamic_cast<ClawbackOperation*>(op)) {
        appendLine(out, prefix + ".body.type", "CLAWBACK");
        const QString p = prefix + ".body.clawbackOp";
        appendLine(out, p + ".asset",  assetToTxrep(cb->getAsset()));
        appendAccountLine(out, p + ".from", cb->getFrom());
        appendLine(out, p + ".amount", withUnits(Operation::toXdrAmount(cb->getAmount())));
    } else if (auto* ccb = dynamic_cast<ClawbackClaimableBalanceOperation*>(op)) {
        appendLine(out, prefix + ".body.type", "CLAWBACK_CLAIMABLE_BALANCE");
        appendLine(out, prefix + ".body.clawbackClaimableBalanceOp.balanceID", ccb->getBalanceId());
    } else if (auto* stf = dynamic_cast<SetTrustlineFlagsOperation*>(op)) {
        appendLine(out, prefix + ".body.type", "SET_TRUST_LINE_FLAGS");
        const QString p = prefix + ".body.setTrustLineFlagsOp";
        appendLine(out, p + ".trustor",    stf->getTrustor());
        appendLine(out, p + ".asset",      assetToTxrep(stf->getAsset()));
        appendLine(out, p + ".clearFlags", QString::number(static_cast<quint32>(stf->getClearFlags())));
        appendLine(out, p + ".setFlags",   QString::number(static_cast<quint32>(stf->getSetFlags())));
    } else if (auto* claim = dynamic_cast<ClaimClaimableBalanceOperation*>(op)) {
        appendLine(out, prefix + ".body.type", "CLAIM_CLAIMABLE_BALANCE");
        appendLine(out, prefix + ".body.claimClaimableBalanceOp.balanceID", claim->getBalanceId());
    } else if (auto* lpd = dynamic_cast<LiquidityPoolDepositOperation*>(op)) {
        appendLine(out, prefix + ".body.type", "LIQUIDITY_POOL_DEPOSIT");
        const QString p = prefix + ".body.liquidityPoolDepositOp";
        const Price minP = lpd->getMinPrice();
        const Price maxP = lpd->getMaxPrice();
        appendLine(out, p + ".liquidityPoolID", hex(lpd->getLiquidityPoolID()));
        appendLine(out, p + ".maxAmountA", withUnits(Operation::toXdrAmount(lpd->getMaxAmountA())));
        appendLine(out, p + ".maxAmountB", withUnits(Operation::toXdrAmount(lpd->getMaxAmountB())));
        appendLine(out, p + ".minPrice.n", QString::number(minP.getNumerator()));
        appendLine(out, p + ".minPrice.d", QString::number(minP.getDenominator()));
        appendLine(out, p + ".maxPrice.n", QString::number(maxP.getNumerator()));
        appendLine(out, p + ".maxPrice.d", QString::number(maxP.getDenominator()));
    } else if (auto* lpw = dynamic_cast<LiquidityPoolWithdrawOperation*>(op)) {
        appendLine(out, prefix + ".body.type", "LIQUIDITY_POOL_WITHDRAW");
        const QString p = prefix + ".body.liquidityPoolWithdrawOp";
        appendLine(out, p + ".liquidityPoolID", hex(lpw->getLiquidityPoolID()));
        appendLine(out, p + ".amount",     withUnits(Operation::toXdrAmount(lpw->getAmount())));
        appendLine(out, p + ".minAmountA", withUnits(Operation::toXdrAmount(lpw->getMinAmountA())));
        appendLine(out, p + ".minAmountB", withUnits(Operation::toXdrAmount(lpw->getMinAmountB())));
    } else if (auto* cclb = dynamic_cast<CreateClaimableBalanceOperation*>(op)) {
        appendLine(out, prefix + ".body.type", "CREATE_CLAIMABLE_BALANCE");
        const QString p = prefix + ".body.createClaimableBalanceOp";
        appendLine(out, p + ".asset",  assetToTxrep(cclb->getAsset()));
        appendLine(out, p + ".amount", withUnits(Operation::toXdrAmount(cclb->getAmount())));
        const QList<Claimant> claimants = cclb->getClaimants();
        appendLine(out, p + ".claimants.len", QString::number(claimants.size()));
        for (int j = 0; j < claimants.size(); ++j) {
            const QString cp = p + QString(".claimants[%1]").arg(j);
            appendLine(out, cp + ".type", "CLAIMANT_TYPE_V0");
            appendLine(out, cp + ".v0.destination", claimants.at(j).getDestination());
            emitPredicate(out, cp + ".v0.predicate", claimants.at(j).getPredicate());
        }
    } else if (auto* ra = dynamic_cast<RevokeAccountSponsorshipOperation*>(op)) {
        const QString p = prefix + ".body.revokeSponsorshipOp";
        appendLine(out, prefix + ".body.type", "REVOKE_SPONSORSHIP");
        appendLine(out, p + ".type", "REVOKE_SPONSORSHIP_LEDGER_ENTRY");
        appendLine(out, p + ".ledgerKey.type", "ACCOUNT");
        appendLine(out, p + ".ledgerKey.account", ra->getAccountId());
    } else if (auto* rt = dynamic_cast<RevokeTrustlineSponsorshipOperation*>(op)) {
        const QString p = prefix + ".body.revokeSponsorshipOp";
        appendLine(out, prefix + ".body.type", "REVOKE_SPONSORSHIP");
        appendLine(out, p + ".type", "REVOKE_SPONSORSHIP_LEDGER_ENTRY");
        appendLine(out, p + ".ledgerKey.type", "TRUSTLINE");
        appendLine(out, p + ".ledgerKey.trustLine.accountID", rt->getAccountId());
        // A trustline of pool shares names the pool, not an asset. SEP-11
        // writes that TrustLineAsset as the pool id in lowercase hex and
        // ":lp". py-stellar-sdk has no form for it: it refuses to write one,
        // and refuses this one as an asset code.
        appendLine(out, p + ".ledgerKey.trustLine.asset",
                   rt->isLiquidityPoolShare() ? hex(rt->getLiquidityPoolID()) + QStringLiteral(":lp")
                                              : assetToTxrep(rt->getAsset()));
    } else if (auto* ro = dynamic_cast<RevokeOfferSponsorshipOperation*>(op)) {
        const QString p = prefix + ".body.revokeSponsorshipOp";
        appendLine(out, prefix + ".body.type", "REVOKE_SPONSORSHIP");
        appendLine(out, p + ".type", "REVOKE_SPONSORSHIP_LEDGER_ENTRY");
        appendLine(out, p + ".ledgerKey.type", "OFFER");
        appendLine(out, p + ".ledgerKey.offer.sellerID", ro->getSeller());
        appendLine(out, p + ".ledgerKey.offer.offerID", QString::number(ro->getOfferId()));
    } else if (auto* rd = dynamic_cast<RevokeDataSponsorshipOperation*>(op)) {
        const QString p = prefix + ".body.revokeSponsorshipOp";
        appendLine(out, prefix + ".body.type", "REVOKE_SPONSORSHIP");
        appendLine(out, p + ".type", "REVOKE_SPONSORSHIP_LEDGER_ENTRY");
        appendLine(out, p + ".ledgerKey.type", "DATA");
        appendLine(out, p + ".ledgerKey.data.accountID", rd->getAccountId());
        // py-stellar-sdk writes and reads this name bare (unlike ManageData's)
        // and takes quotes around it as part of it: "k" quoted was revoked
        // there as the three-character name "k" with its quotes. So bare, as
        // py reads it back. A name no bare form says (a space, a leading
        // quote, a character that must be escaped) used to be written quoted,
        // which py signed as another entry: it has no form both read alike.
        const QString name = rd->getDataName();
        if (!readsBackBare(name))
            throw std::runtime_error("Txrep emitter: this revoke-data name has no TxRep form that "
                                     "py-stellar-sdk reads as the same entry (it holds a space, a "
                                     "leading quote or a character that must be escaped)");
        appendLine(out, p + ".ledgerKey.data.dataName", name);
    } else if (auto* rcb = dynamic_cast<RevokeClaimableBalanceSponsorshipOperation*>(op)) {
        const QString p = prefix + ".body.revokeSponsorshipOp";
        appendLine(out, prefix + ".body.type", "REVOKE_SPONSORSHIP");
        appendLine(out, p + ".type", "REVOKE_SPONSORSHIP_LEDGER_ENTRY");
        appendLine(out, p + ".ledgerKey.type", "CLAIMABLE_BALANCE");
        appendLine(out, p + ".ledgerKey.claimableBalance.balanceID", rcb->getBalanceId());
    } else if (auto* rlp = dynamic_cast<RevokeLiquidityPoolSponsorshipOperation*>(op)) {
        const QString p = prefix + ".body.revokeSponsorshipOp";
        appendLine(out, prefix + ".body.type", "REVOKE_SPONSORSHIP");
        appendLine(out, p + ".type", "REVOKE_SPONSORSHIP_LEDGER_ENTRY");
        appendLine(out, p + ".ledgerKey.type", "LIQUIDITY_POOL");
        appendLine(out, p + ".ledgerKey.liquidityPool.liquidityPoolID", rlp->getLiquidityPoolId());
    } else if (auto* rs = dynamic_cast<RevokeSignerSponsorshipOperation*>(op)) {
        const QString p = prefix + ".body.revokeSponsorshipOp";
        appendLine(out, prefix + ".body.type", "REVOKE_SPONSORSHIP");
        appendLine(out, p + ".type", "REVOKE_SPONSORSHIP_SIGNER");
        appendLine(out, p + ".signer.accountID", rs->getAccountId());
        // `.type` and a field per type, as py-stellar-sdk writes and reads it;
        // it read nothing from the single `signer.signerKey` line.
        emitSignerKey(out, p + ".signer.signerKey", SignerKey::fromXdr(rs->getSigner()));
    } else if (dynamic_cast<InflationOperation*>(op)) {
        // INFLATION carries no fields.
        appendLine(out, prefix + ".body.type", "INFLATION");
    } else {
        // Soroban ops (InvokeHostFunction / ExtendFootprintTTL / RestoreFootprint)
        // are intentionally NOT emitted: txrep is a pre-Soroban format and no
        // mainstream SDK supports them, so emitting a non-standard form would
        // not be interoperable. (A base64-XDR round-trip impl is stashed in
        // upgrade-plan/txrep-soroban-base64-stash.md.)
        throw std::runtime_error("Txrep emitter: unsupported operation type");
    }
}

/**
 * A JSON string value (what quoted() writes, and what py-stellar-sdk writes
 * with json.dumps, \\uXXXX escapes included), read in one pass. Nothing may
 * follow the closing quote. Anything else is refused: the two sequential
 * replace() calls this used to be read some escapes wrongly.
 */
QString unquote(const QString& s)
{
    if (s.size() < 2 || s.at(0) != QLatin1Char('"'))
        throw std::runtime_error("malformed quoted string in txrep");
    QString out(QLatin1String(""));      // "" is text, not "no text"
    int i = 1;
    for (; i < s.size(); ++i) {
        const QChar c = s.at(i);
        if (c == QLatin1Char('"'))
            break;
        if (c != QLatin1Char('\\')) {
            out += c;
            continue;
        }
        if (++i >= s.size())
            break;
        switch (s.at(i).unicode()) {
        case '"':  out += QLatin1Char('"'); break;
        case '\\': out += QLatin1Char('\\'); break;
        case '/':  out += QLatin1Char('/'); break;
        case 'b':  out += QLatin1Char('\b'); break;
        case 'f':  out += QLatin1Char('\f'); break;
        case 'n':  out += QLatin1Char('\n'); break;
        case 'r':  out += QLatin1Char('\r'); break;
        case 't':  out += QLatin1Char('\t'); break;
        case 'u': {
            // Exactly four hex digits: toUShort alone also took "0x41", " 041"
            // and "+041".
            const QString digits = s.mid(i + 1, 4);
            bool ok = digits.size() == 4;
            for (const QChar d : digits) {
                const ushort u = d.unicode();
                if (!((u >= '0' && u <= '9') || (u >= 'a' && u <= 'f') || (u >= 'A' && u <= 'F')))
                    ok = false;
            }
            const ushort code = ok ? digits.toUShort(&ok, 16) : 0;
            if (!ok)
                throw std::runtime_error("malformed \\u escape in a txrep string");
            out += QChar(code);
            i += 4;
            break;
        }
        default:
            throw std::runtime_error("unknown escape in a txrep string");
        }
    }
    if (i >= s.size())
        throw std::runtime_error("malformed quoted string in txrep: no closing quote");
    const QString rest = s.mid(i + 1).trimmed();
    // Nor a comment: none states a string checkably, and a reader takes
    // `"12345" (54321)` for what the memo says. It used to be read past.
    if (!rest.isEmpty())
        throw std::runtime_error("malformed quoted string in txrep: text after the closing quote "
                                 "(a string takes no comment)");
    // Half a surrogate pair is not text: signed, it became "?" on Qt 5 and
    // vanished on Qt 6 — a different home domain or data name.
    if (hasUnpairedSurrogate(out))
        throw std::runtime_error("a txrep string holds half of a surrogate pair");
    return out;
}

const QString& need(const TxrepMap& kv, const QString& key)
{
    const QString* v = kv.find(key);
    if (!v) {
        throw std::runtime_error(("missing txrep key: " + key).toStdString());
    }
    return *v;
}

/**
 * A true/false field, and only those two words. Every `_present` and flag
 * line used to be compared with "true", so a missing line, "TRUE" or "1" read
 * as false: the memo, a time bound or an operation's source account vanished
 * from what was signed while the document still showed them.
 */
bool needFlag(const TxrepMap& kv, const QString& key)
{
    const QString v = need(kv, key).trimmed();
    if (v == QLatin1String("true"))
        return true;
    if (v == QLatin1String("false"))
        return false;
    throw std::runtime_error(("txrep: " + key + " must be true or false").toStdString());
}

/** Whether an optional field is there, by its required `._present` line. */
bool present(const TxrepMap& kv, const QString& field)
{
    return needFlag(kv, field + QStringLiteral("._present"));
}

/**
 * Hex bytes, strictly: an even number of hex digits and nothing else, and
 * exactly `bytes` of them when given. QByteArray::fromHex skips what it does
 * not understand, so "0xab" or a digit short read as other bytes.
 */
QByteArray hexBytes(const QString& value, const QString& key, int bytes = -1)
{
    const QByteArray text = value.trimmed().toLatin1();
    bool ok = text.size() % 2 == 0;
    for (const char c : text) {
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')))
            ok = false;
    }
    if (!ok || (bytes >= 0 && text.size() != bytes * 2)) {
        throw std::runtime_error(("txrep: " + key + " must be "
                                  + (bytes >= 0 ? QString::number(bytes) + " bytes of " : QString())
                                  + "hex").toStdString());
    }
    return QByteArray::fromHex(text);
}

QByteArray needHex(const TxrepMap& kv, const QString& key, int bytes = -1)
{
    return hexBytes(need(kv, key), key, bytes);
}

/**
 * A variable-length opaque (a data value, a signature), in hex, or stc's
 * "0 bytes" for an empty one (its source: "the bytes is a comment, but just
 * 0 might be unintuitive"), read as the no bytes it says. It was refused,
 * and with it stc's document setting a data entry to an empty value.
 */
QByteArray needOpaque(const TxrepMap& kv, const QString& key)
{
    if (need(kv, key) == QLatin1String("0 bytes"))
        return QByteArray();
    return needHex(kv, key);
}

/**
 * The one comment an account may carry: py-stellar-sdk's after an M...
 * address, naming the account and id it holds, which the address does not
 * show a reader. A reader trusts what a comment says the account is, and no
 * other comment can be checked against it: `GA... (GB...)` showed GB... and
 * paid GA..., and a G... address, which holds no id, took a comment of any
 * other form but py's.
 */
void checkAccountComment(const TxrepMap& kv, const QString& key, const QString& value)
{
    // stc writes the writer's own label for an account after it ("GA...
    // (the exchange)", from its stc.conf): nothing in the document can
    // confirm it, and whoever wrote the document would tell the reviewer
    // who is paid. Refused with every other comment.
    kv.setKind(key, ValueKind::Account);
    const QString comment = kv.raw(key).mid(value.size()).trimmed();
    if (!comment.isEmpty() && comment != muxedComment(value)) {
        throw std::runtime_error(("txrep: the comment on " + key + " does not describe the "
                                  "account written there").toStdString());
    }
}

/**
 * An AccountID field: a G... address, exactly as its strkey is written. Read
 * here rather than left to the operation, which may only look at it when
 * the transaction is signed.
 */
QString needAccountId(const TxrepMap& kv, const QString& key)
{
    const QString value = need(kv, key).trimmed();
    bool ok = value.size() == StrKey::ACCOUNT_ID_ADDRESS_LENGTH;
    try {
        ok = ok && MuxedAccount::fromAddress(value).toAddress() == value;
    } catch (const std::exception&) {
        ok = false;
    }
    if (!ok)
        throw std::runtime_error(("txrep: " + key + " must be a G... account").toStdString());
    checkAccountComment(kv, key, value);
    return value;
}

/**
 * A MuxedAccount field (a source account, a payment destination...): a G...
 * or an M... address, exactly as its strkey is written.
 *
 * After it, only a comment checkAccountComment() finds true.
 */
QString needMuxedAccount(const TxrepMap& kv, const QString& key)
{
    const QString value = need(kv, key).trimmed();
    bool ok = !value.isEmpty();
    try {
        ok = ok && MuxedAccount::fromAddress(value).toAddress() == value;
    } catch (const std::exception&) {
        ok = false;
    }
    if (!ok)
        throw std::runtime_error(("txrep: " + key + " must be a G... or M... account").toStdString());
    checkAccountComment(kv, key, value);
    return value;
}

/**
 * An Asset field: `native` (or `XLM`), or CODE:ISSUER. stc names the native
 * asset after the network it writes for: "TestXLM" for the test network and
 * futurenet, "StandaloneXLM" for a standalone one. That name is read only
 * for its own network: "TestXLM" in a document read for the public network
 * would show test coins where lumens are spent.
 */
Asset* needAsset(const TxrepMap& kv, const QString& key)
{
    const QString value = need(kv, key).trimmed();
    if (!kv.nativeName().isEmpty() && value == kv.nativeName())
        return new AssetTypeNative();
    return assetFromTxrep(value);
}

/** stc's name for the native asset of `network`, if it is not "XLM" (see needAsset). */
QString stcNativeName(const Network* network)
{
    const QString passphrase = network->getNetworkPassphrase();
    if (passphrase == Network::TESTNET_NETWORK_PASSPHRASE()
        || passphrase == Network::FUTURENET_NETWORK_PASSPHRASE())
        return QStringLiteral("TestXLM");
    if (passphrase == Network::STANDALONE_NETWORK_PASSPHRASE())
        return QStringLiteral("StandaloneXLM");
    return QString();
}

/**
 * An issued asset as stc writes a ChangeTrust line, the union's arm under
 * `path`: `.alphaNum4.assetCode: 55534400` (the code field's bytes, in hex)
 * and `.alphaNum4.issuer: G...`. The code is one core takes, letters and
 * digits padded with zeros at the end only; the caller refuses one under the
 * other arm than its length gives ("USD" under alphaNum12 is another asset).
 */
Asset* stcIssuedAsset(const TxrepMap& kv, const QString& path, bool twelve)
{
    const QString arm = path + (twelve ? QStringLiteral(".alphaNum12") : QStringLiteral(".alphaNum4"));
    QByteArray code = needHex(kv, arm + QStringLiteral(".assetCode"), twelve ? 12 : 4);
    const int end = code.indexOf('\0');
    if (end >= 0) {
        if (code.mid(end).count('\0') != code.size() - end)
            throw std::runtime_error(("txrep: " + arm + ".assetCode holds a zero inside the code")
                                         .toStdString());
        code.truncate(end);
    }
    const QString issuer = needAccountId(kv, arm + QStringLiteral(".issuer"));
    return Asset::create(checkedAssetCode(QString::fromLatin1(code)), issuer);
}

/**
 * A ClaimableBalanceID. py-stellar-sdk writes it on one line, the type's
 * four bytes first (`balanceID: 00000000ab...`); stc writes the union
 * (`balanceID.type: CLAIMABLE_BALANCE_ID_TYPE_V0`, `balanceID.v0: ab...`).
 * Either is read, as the one-line form; a document with both leaves the
 * other's lines unread, and is refused.
 */
QString needBalanceId(const TxrepMap& kv, const QString& key)
{
    if (kv.has(key) || !kv.has(key + QStringLiteral(".type")))
        return need(kv, key);
    if (need(kv, key + QStringLiteral(".type")).trimmed() != QLatin1String("CLAIMABLE_BALANCE_ID_TYPE_V0"))
        throw std::runtime_error(("txrep: " + key + ".type must be CLAIMABLE_BALANCE_ID_TYPE_V0").toStdString());
    return QStringLiteral("00000000") + hex(needHex(kv, key + QStringLiteral(".v0"), 32));
}

/** A SignerKey written as emitSignerKey writes it (py-stellar-sdk's form). */
SignerKey parseSignerKey(const TxrepMap& kv, const QString& path)
{
    const QString type = need(kv, path + QStringLiteral(".type")).trimmed();
    for (const SignerKeyForm& form : kSignerKeyForms) {
        if (type != QLatin1String(form.name))
            continue;
        // The key must be of the type the line says: a G... key under
        // PRE_AUTH_TX used to be signed as an ed25519 signer.
        const SignerKey key = SignerKey::fromString(
            need(kv, path + QLatin1Char('.') + QLatin1String(form.field)).trimmed());
        if (key.getType() != form.type)
            throw std::runtime_error(("txrep: " + path + " does not hold the key type its .type "
                                      "names").toStdString());
        return key;
    }
    throw std::runtime_error(("txrep: unknown signer key type: " + type).toStdString());
}

/**
 * An integer field, read exactly or refused.
 *
 * Every one of these used to be a bare toLongLong()/toInt()/toUInt(), whose
 * failure value is 0: an amount written "10,5" or "abc" became 0 stroops, a
 * price "x/y" became 0/0, a time bound became "unbounded" — each rebuilt into
 * a transaction without a word, and handed back for signing as the document
 * the user had just read.
 */
/**
 * Digits as SEP-11 writes them: an optional '-' for signed fields, no '+', and
 * no leading zero. "0144" is 100 to a C-syntax reader such as stc and 144 to
 * this one, so it is refused rather than read as either. "-0" is never
 * written either, so it is refused too.
 */
bool canonicalInteger(const QString& text, bool allowMinus)
{
    int i = 0;
    if (allowMinus && text.startsWith(QLatin1Char('-')))
        i = 1;
    if (i >= text.size())
        return false;
    if (text.at(i) == QLatin1Char('0') && (i > 0 || text.size() > 1))
        return false;
    for (; i < text.size(); ++i) {
        if (text.at(i) < QLatin1Char('0') || text.at(i) > QLatin1Char('9'))
            return false;
    }
    return true;
}

qint64 needInteger(const QString& text, const QString& key, qint64 min, qint64 max)
{
    // A sign only where the range has negatives: "-0" was read as 0 for
    // tx.fee and every unsigned field, while the uint64 ones refused it.
    bool ok = canonicalInteger(text.trimmed(), min < 0);
    const qint64 value = ok ? text.trimmed().toLongLong(&ok) : 0;
    if (!ok || value < min || value > max) {
        throw std::runtime_error(("txrep: " + key + " must be a whole number from "
                                  + QString::number(min) + " to " + QString::number(max))
                                     .toStdString());
    }
    return value;
}

/** The integer field `key`, from `min` to `max`; a comment after it may restate it. */
qint64 needInteger(const TxrepMap& kv, const QString& key, qint64 min, qint64 max)
{
    const qint64 value = needInteger(need(kv, key), key, min, max);
    kv.setKind(key, ValueKind::Number);
    return value;
}

qint64 needInt64(const TxrepMap& kv, const QString& key)
{
    return needInteger(kv, key, std::numeric_limits<qint64>::min(),
                       std::numeric_limits<qint64>::max());
}

qint32 needInt32(const TxrepMap& kv, const QString& key)
{
    return static_cast<qint32>(needInteger(kv, key, std::numeric_limits<qint32>::min(),
                                           std::numeric_limits<qint32>::max()));
}

/** A price term: 1 to 2^31 - 1. Core refuses 0 and below as malformed. */
qint32 needPriceTerm(const TxrepMap& kv, const QString& key)
{
    return static_cast<qint32>(needInteger(kv, key, 1, std::numeric_limits<qint32>::max()));
}

quint32 needUInt32(const TxrepMap& kv, const QString& key)
{
    return static_cast<quint32>(needInteger(kv, key, 0, std::numeric_limits<quint32>::max()));
}

quint64 unsignedInteger(const QString& raw, const QString& key)
{
    const QString text = raw.trimmed();
    bool ok = canonicalInteger(text, false);
    const quint64 value = ok ? text.toULongLong(&ok) : 0;
    if (!ok) {
        throw std::runtime_error(("txrep: " + key + " must be a whole number from 0 to "
                                  "18446744073709551615").toStdString());
    }
    return value;
}

quint64 needUInt64(const TxrepMap& kv, const QString& key)
{
    const quint64 value = unsignedInteger(need(kv, key), key);
    kv.setKind(key, ValueKind::Number);
    return value;
}

/**
 * A time bound: a TimePoint, uint64 on the wire, read as one — negative text
 * used to be accepted and cast to a time 584 billion years out, and another
 * tool's 18446744073709551615 was refused. TimeBounds::fromTimePoints takes
 * any of them (the constructor refuses a negative qint64).
 */
quint64 needTime(const TxrepMap& kv, const QString& key)
{
    return needUInt64(kv, key);
}

/**
 * A document that lists more entries than its .len says would be signed
 * with fewer — a path with no hops, an operation or an extra signer missing —
 * and look complete to anybody reading it. Refused.
 */
void refuseEntriesPast(const TxrepMap& kv, const QString& list, int count)
{
    const QString open = list + QLatin1Char('[');
    for (const QString& key : kv.keys()) {
        if (!key.startsWith(open))
            continue;
        const int close = key.indexOf(QLatin1Char(']'), open.size());
        bool ok = false;
        const int index = close > 0 ? key.mid(open.size(), close - open.size()).toInt(&ok) : -1;
        if (!ok || index >= count)
            throw std::runtime_error(("txrep: " + list + " lists more entries than its "
                                      ".len").toStdString());
    }
}

/** A length field: 0 to `max`, the most the XDR array can hold. */
int needCount(const TxrepMap& kv, const QString& key, int max)
{
    return static_cast<int>(needInteger(kv, key, 0, max));
}



// Recursively parse a claim predicate tree. The returned Predicate owns its
// children (Not/And/Or delete their inners), so a single delete frees the tree.
// A child is held until the node that takes it exists: a second child that
// was refused used to leak the first.
std::unique_ptr<Predicate> parsePredicate(const TxrepMap& kv, const QString& path)
{
    const QString type = need(kv, path + ".type");
    if (type == "CLAIM_PREDICATE_UNCONDITIONAL") {
        return std::unique_ptr<Predicate>(new Predicate::Unconditional());
    } else if (type == "CLAIM_PREDICATE_BEFORE_ABSOLUTE_TIME") {
        return std::unique_ptr<Predicate>(new Predicate::AbsBefore(needInt64(kv, path + ".absBefore")));
    } else if (type == "CLAIM_PREDICATE_BEFORE_RELATIVE_TIME") {
        return std::unique_ptr<Predicate>(new Predicate::RelBefore(needInt64(kv, path + ".relBefore")));
    } else if (type == "CLAIM_PREDICATE_NOT") {
        // py-stellar-sdk writes the inner predicate with no _present line; this
        // emitter writes one. Either way the inner predicate is required.
        const QString* flag = kv.find(path + ".notPredicate._present");
        if (flag && flag->trimmed() != QLatin1String("true"))
            throw std::runtime_error("txrep: a NOT predicate needs its inner predicate");
        std::unique_ptr<Predicate> inner = parsePredicate(kv, path + ".notPredicate");
        std::unique_ptr<Predicate> node(new Predicate::Not(inner.get()));
        inner.release();                    // the node owns it now
        return node;
    } else if (type == "CLAIM_PREDICATE_AND" || type == "CLAIM_PREDICATE_OR") {
        const bool isAnd = type == "CLAIM_PREDICATE_AND";
        const QString list = path + (isAnd ? ".andPredicates" : ".orPredicates");
        // Exactly two: core refuses anything else, and so does the emitter.
        const int n = static_cast<int>(needInteger(kv, list + ".len", 2, 2));
        refuseEntriesPast(kv, list, n);
        std::vector<std::unique_ptr<Predicate>> children;
        QList<Predicate*> inner;
        for (int j = 0; j < n; ++j) {
            children.push_back(parsePredicate(kv, list + QString("[%1]").arg(j)));
            inner.append(children.back().get());
        }
        std::unique_ptr<Predicate> node;
        if (isAnd)
            node.reset(new Predicate::And(inner));
        else
            node.reset(new Predicate::Or(inner));
        for (std::unique_ptr<Predicate>& child : children)
            child.release();                // the node owns them now
        return node;
    }
    throw std::runtime_error(("Txrep parser: unknown claim predicate type: " + type).toStdString());
}

// See emitAllowTrust above — deprecated symbol isolated + warning suppressed.
#if defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable: 4996)
#elif defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
std::unique_ptr<Operation> parseAllowTrust(const TxrepMap& kv, const QString& prefix)
{
    // Held until the operation has copied it: a refused asset code leaked it.
    std::unique_ptr<KeyPair> trustor(
        KeyPair::fromAccountId(needAccountId(kv, prefix + ".body.allowTrustOp.trustor")));
    const QString assetCode = checkedAssetCode(need(kv, prefix + ".body.allowTrustOp.asset"));
    // Accept the bool form (py/stc) and the numeric flag form interchangeably.
    // Anything else used to fall through to "false" — a revocation, signed
    // for a document that said something else entirely.
    const QString av = need(kv, prefix + ".body.allowTrustOp.authorize").trimmed();
    if (av != "true" && av != "false" && av != "0" && av != "1" && av != "2")
        throw std::runtime_error("txrep: allowTrustOp.authorize must be true, false, 0, 1 or 2");
    const bool authorize = (av == "true" || av == "1");
    const bool maintain  = (av == "2");
    // The ctor copies the XDR pubkey, it does not retain the pointer.
    return std::unique_ptr<Operation>(new AllowTrustOperation(trustor.get(), assetCode, authorize, maintain));
}
#if defined(_MSC_VER)
#  pragma warning(pop)
#elif defined(__GNUC__)
#  pragma GCC diagnostic pop
#endif

/**
 * A path of up to five assets. The operations copy the assets' XDR and do not
 * retain the pointers, so `owners` keeps them until the operation exists.
 */
QList<Asset*> parsePath(const TxrepMap& kv, const QString& p,
                        std::vector<std::unique_ptr<Asset>>& owners)
{
    QList<Asset*> path;
    const int pathLen = needCount(kv, p + ".path.len", 5);
    refuseEntriesPast(kv, p + ".path", pathLen);
    for (int k = 0; k < pathLen; ++k) {
        owners.emplace_back(needAsset(kv, p + QString(".path[%1]").arg(k)));
        path.append(owners.back().get());
    }
    return path;
}

std::unique_ptr<Operation> parseOperation(const TxrepMap& kv, const QString& tx, int i)
{
    const QString prefix = tx + QString(".operations[%1]").arg(i);
    const QString type = need(kv, prefix + ".body.type");

    // The operation, and every asset, key and predicate read for it, is held
    // until it is complete: a refusal part way through (a weight above 255, an
    // amount that does not parse, a source account that is none) used to leak
    // them. The operations copy the XDR of what they are given and retain
    // none of the pointers, unless said otherwise.
    std::unique_ptr<Operation> op;
    if (type == "MANAGE_DATA") {
        QString name = unquote(need(kv, prefix + ".body.manageDataOp.dataName"));
        QByteArray value;
        if (present(kv, prefix + ".body.manageDataOp.dataValue")) {
            value = needOpaque(kv, prefix + ".body.manageDataOp.dataValue");
            if (value.isNull())
                value = QByteArray("");          // present and empty, not a delete
        }
        op.reset(new ManageDataOperation(name, value));
    } else if (type == "PAYMENT") {
        const QString dest = needMuxedAccount(kv, prefix + ".body.paymentOp.destination");
        std::unique_ptr<Asset> asset(needAsset(kv, prefix + ".body.paymentOp.asset"));
        const qint64 stroops = needInt64(kv, prefix + ".body.paymentOp.amount");
        op.reset(new PaymentOperation(dest, asset.get(), Operation::fromXdrAmount(stroops)));
    } else if (type == "CREATE_ACCOUNT") {
        const QString dest = needAccountId(kv, prefix + ".body.createAccountOp.destination");
        const qint64 stroops = needInt64(kv, prefix + ".body.createAccountOp.startingBalance");
        op.reset(new CreateAccountOperation(dest, Operation::fromXdrAmount(stroops)));
    } else if (type == "ACCOUNT_MERGE") {
        op.reset(new AccountMergeOperation(needMuxedAccount(kv, prefix + ".body.destination")));
    } else if (type == "BUMP_SEQUENCE") {
        // Read as decoded, not built: a document is an operation that exists,
        // and one with a negative bumpTo (core refuses it) still reads back.
        stellar::BumpSequenceOp bump;
        bump.bumpTo = needInt64(kv, prefix + ".body.bumpSequenceOp.bumpTo");
        op.reset(BumpSequenceOperation::build(bump));
    } else if (type == "SET_OPTIONS") {
        std::unique_ptr<SetOptionsOperation> so(SetOptionsOperation::create());
        const QString p = prefix + ".body.setOptionsOp";
        if (present(kv, p + ".inflationDest")) {
            std::unique_ptr<KeyPair> dest(KeyPair::fromAccountId(needAccountId(kv, p + ".inflationDest")));
            so->setInflationDestination(dest.get());   // copies the XDR pubkey
        }
        if (present(kv, p + ".clearFlags"))
            so->setClearFlags(needUInt32(kv, p + ".clearFlags"));
        if (present(kv, p + ".setFlags"))
            so->setSetFlags(needUInt32(kv, p + ".setFlags"));
        if (present(kv, p + ".masterWeight"))
            so->setMasterKeyWeight(needUInt32(kv, p + ".masterWeight"));
        if (present(kv, p + ".lowThreshold"))
            so->setLowThreshold(needUInt32(kv, p + ".lowThreshold"));
        if (present(kv, p + ".medThreshold"))
            so->setMediumThreshold(needUInt32(kv, p + ".medThreshold"));
        if (present(kv, p + ".highThreshold"))
            so->setHighThreshold(needUInt32(kv, p + ".highThreshold"));
        if (present(kv, p + ".homeDomain"))
            so->setHomeDomain(unquote(need(kv, p + ".homeDomain")));
        if (present(kv, p + ".signer"))
            so->setSigner(SignerKey::fromString(need(kv, p + ".signer.key")).toXdr(),
                          needUInt32(kv, p + ".signer.weight"));
        op = std::move(so);
    } else if (type == "CHANGE_TRUST") {
        const QString p = prefix + ".body.changeTrustOp";
        const QString lineType = need(kv, p + ".line.type").trimmed();
        if (lineType == QLatin1String("ASSET_TYPE_POOL_SHARE")) {
            // Shares of a pool, named by its parameters as py-stellar-sdk
            // writes them. The ChangeTrustAsset takes the two assets. stc
            // writes the pool's type as well; there is one.
            const QString poolType = p + ".line.liquidityPool.type";
            if (kv.has(poolType)
                && need(kv, poolType).trimmed() != QLatin1String("LIQUIDITY_POOL_CONSTANT_PRODUCT"))
                throw std::runtime_error("txrep: a liquidity pool's type is LIQUIDITY_POOL_CONSTANT_PRODUCT");
            const QString cp = p + ".line.liquidityPool.constantProduct";
            std::unique_ptr<Asset> assetA(needAsset(kv, cp + ".assetA"));
            std::unique_ptr<Asset> assetB(needAsset(kv, cp + ".assetB"));
            // 30 is the only fee a pool has: shares of a pool with any other
            // fee name no pool, and core refuses them.
            if (needInt32(kv, cp + ".fee") != LiquidityPool::FEE)
                throw std::runtime_error("txrep: a liquidity pool's fee is 30, the only fee a pool has");
            // Refuses two assets out of their canonical order, which name no
            // pool either.
            std::unique_ptr<ChangeTrustAsset> line(
                new LiquidityPoolShareChangeTrustAsset(assetA.get(), assetB.get(), LiquidityPool::FEE));
            assetA.release();
            assetB.release();
            const qint64 limit = needInt64(kv, p + ".limit");
            op.reset(new ChangeTrustOperation(line.get(), Operation::fromXdrAmount(limit)));
        } else {
            // py-stellar-sdk writes the asset on one line (`line: USD:G...`),
            // stc the union's arm (see stcIssuedAsset); a document gives one.
            const bool twelve = lineType == QLatin1String("ASSET_TYPE_CREDIT_ALPHANUM12");
            const bool stcArm = !kv.has(p + ".line")
                                && (twelve || lineType == QLatin1String("ASSET_TYPE_CREDIT_ALPHANUM4"));
            std::unique_ptr<Asset> asset(stcArm ? stcIssuedAsset(kv, p + ".line", twelve)
                                                : needAsset(kv, p + ".line"));
            // The line's type is written beside it; one that disagrees with the
            // asset is a document that does not say one thing.
            if (lineType != assetTypeString(asset.get()))
                throw std::runtime_error("txrep: changeTrustOp.line.type does not match its asset");
            const qint64 limit = needInt64(kv, p + ".limit");
            op.reset(new ChangeTrustOperation(asset.get(), Operation::fromXdrAmount(limit)));
        }
    } else if (type == "ALLOW_TRUST") {
        op = parseAllowTrust(kv, prefix);
    } else if (type == "MANAGE_SELL_OFFER") {
        const QString p = prefix + ".body.manageSellOfferOp";
        std::unique_ptr<Asset> selling(needAsset(kv, p + ".selling"));
        std::unique_ptr<Asset> buying(needAsset(kv, p + ".buying"));
        const QString amount = Operation::fromXdrAmount(needInt64(kv, p + ".amount"));
        // txrep carries the exact n and d: build the Price directly. Going
        // through the decimal string fell back to the Price(QString)
        // approximation, and 7/9 came back as 777777777/1000000000.
        const Price price(needPriceTerm(kv, p + ".price.n"),
                          needPriceTerm(kv, p + ".price.d"));
        op.reset(new ManageSellOfferOperation(selling.get(), buying.get(), amount, price,
                                              needInt64(kv, p + ".offerID")));
    } else if (type == "MANAGE_BUY_OFFER") {
        const QString p = prefix + ".body.manageBuyOfferOp";
        std::unique_ptr<Asset> selling(needAsset(kv, p + ".selling"));
        std::unique_ptr<Asset> buying(needAsset(kv, p + ".buying"));
        const QString amount = Operation::fromXdrAmount(needInt64(kv, p + ".buyAmount"));
        // Exact n and d, as for MANAGE_SELL_OFFER.
        const Price price(needPriceTerm(kv, p + ".price.n"),
                          needPriceTerm(kv, p + ".price.d"));
        op.reset(new ManageBuyOfferOperation(selling.get(), buying.get(), amount, price,
                                             needInt64(kv, p + ".offerID")));
    } else if (type == "CREATE_PASSIVE_SELL_OFFER") {
        const QString p = prefix + ".body.createPassiveSellOfferOp";
        std::unique_ptr<Asset> selling(needAsset(kv, p + ".selling"));
        std::unique_ptr<Asset> buying(needAsset(kv, p + ".buying"));
        const QString amount = Operation::fromXdrAmount(needInt64(kv, p + ".amount"));
        // Exact n and d, as for MANAGE_SELL_OFFER.
        const Price price(needPriceTerm(kv, p + ".price.n"),
                          needPriceTerm(kv, p + ".price.d"));
        op.reset(new CreatePassiveSellOfferOperation(selling.get(), buying.get(), amount, price));
    } else if (type == "PATH_PAYMENT_STRICT_RECEIVE") {
        const QString p = prefix + ".body.pathPaymentStrictReceiveOp";
        std::unique_ptr<Asset> sendAsset(needAsset(kv, p + ".sendAsset"));
        std::unique_ptr<Asset> destAsset(needAsset(kv, p + ".destAsset"));
        const QString sendMax = Operation::fromXdrAmount(needInt64(kv, p + ".sendMax"));
        const QString destAmount = Operation::fromXdrAmount(needInt64(kv, p + ".destAmount"));
        std::vector<std::unique_ptr<Asset>> hops;
        const QList<Asset*> path = parsePath(kv, p, hops);
        op.reset(new PathPaymentStrictReceiveOperation(sendAsset.get(), sendMax,
                     needMuxedAccount(kv, p + ".destination"), destAsset.get(), destAmount, path));
    } else if (type == "PATH_PAYMENT_STRICT_SEND") {
        const QString p = prefix + ".body.pathPaymentStrictSendOp";
        std::unique_ptr<Asset> sendAsset(needAsset(kv, p + ".sendAsset"));
        std::unique_ptr<Asset> destAsset(needAsset(kv, p + ".destAsset"));
        const QString sendAmount = Operation::fromXdrAmount(needInt64(kv, p + ".sendAmount"));
        const QString destMin = Operation::fromXdrAmount(needInt64(kv, p + ".destMin"));
        std::vector<std::unique_ptr<Asset>> hops;
        const QList<Asset*> path = parsePath(kv, p, hops);
        op.reset(new PathPaymentStrictSendOperation(sendAsset.get(), sendAmount,
                     needMuxedAccount(kv, p + ".destination"), destAsset.get(), destMin, path));
    } else if (type == "BEGIN_SPONSORING_FUTURE_RESERVES") {
        op.reset(new BeginSponsoringFutureReservesOperation(
                     needAccountId(kv, prefix + ".body.beginSponsoringFutureReservesOp.sponsoredID")));
    } else if (type == "END_SPONSORING_FUTURE_RESERVES") {
        op.reset(new EndSponsoringFutureReservesOperation());
    } else if (type == "CLAWBACK") {
        const QString p = prefix + ".body.clawbackOp";
        std::unique_ptr<Asset> asset(needAsset(kv, p + ".asset"));
        const QString amount = Operation::fromXdrAmount(needInt64(kv, p + ".amount"));
        op.reset(new ClawbackOperation(needMuxedAccount(kv, p + ".from"), asset.get(), amount));
    } else if (type == "CLAWBACK_CLAIMABLE_BALANCE") {
        op.reset(new ClawbackClaimableBalanceOperation(
                     needBalanceId(kv, prefix + ".body.clawbackClaimableBalanceOp.balanceID")));
    } else if (type == "SET_TRUST_LINE_FLAGS") {
        const QString p = prefix + ".body.setTrustLineFlagsOp";
        std::unique_ptr<Asset> asset(needAsset(kv, p + ".asset"));
        // uint32 in the XDR, as the emitter writes them.
        const QFlags<stellar::TrustLineFlags> clearFlags(
            QFlag(static_cast<int>(needUInt32(kv, p + ".clearFlags"))));
        const QFlags<stellar::TrustLineFlags> setFlags(
            QFlag(static_cast<int>(needUInt32(kv, p + ".setFlags"))));
        op.reset(new SetTrustlineFlagsOperation(needAccountId(kv, p + ".trustor"), asset.get(),
                                                clearFlags, setFlags));
    } else if (type == "CLAIM_CLAIMABLE_BALANCE") {
        op.reset(new ClaimClaimableBalanceOperation(
                     needBalanceId(kv, prefix + ".body.claimClaimableBalanceOp.balanceID")));
    } else if (type == "LIQUIDITY_POOL_DEPOSIT") {
        const QString p = prefix + ".body.liquidityPoolDepositOp";
        const QByteArray poolId = needHex(kv, p + ".liquidityPoolID", 32);
        const QString maxA = Operation::fromXdrAmount(needInt64(kv, p + ".maxAmountA"));
        const QString maxB = Operation::fromXdrAmount(needInt64(kv, p + ".maxAmountB"));
        const Price minPrice(needPriceTerm(kv, p + ".minPrice.n"), needPriceTerm(kv, p + ".minPrice.d"));
        const Price maxPrice(needPriceTerm(kv, p + ".maxPrice.n"), needPriceTerm(kv, p + ".maxPrice.d"));
        op.reset(new LiquidityPoolDepositOperation(poolId, maxA, maxB, minPrice, maxPrice));
    } else if (type == "LIQUIDITY_POOL_WITHDRAW") {
        const QString p = prefix + ".body.liquidityPoolWithdrawOp";
        const QByteArray poolId = needHex(kv, p + ".liquidityPoolID", 32);
        const QString amount = Operation::fromXdrAmount(needInt64(kv, p + ".amount"));
        const QString minA = Operation::fromXdrAmount(needInt64(kv, p + ".minAmountA"));
        const QString minB = Operation::fromXdrAmount(needInt64(kv, p + ".minAmountB"));
        op.reset(new LiquidityPoolWithdrawOperation(poolId, amount, minA, minB));
    } else if (type == "CREATE_CLAIMABLE_BALANCE") {
        const QString p = prefix + ".body.createClaimableBalanceOp";
        std::unique_ptr<Asset> asset(needAsset(kv, p + ".asset"));
        const QString amount = Operation::fromXdrAmount(needInt64(kv, p + ".amount"));
        QList<Claimant> claimants;
        const int n = needCount(kv, p + ".claimants.len", 10);
        refuseEntriesPast(kv, p + ".claimants", n);
        for (int j = 0; j < n; ++j) {
            const QString cp = p + QString(".claimants[%1]").arg(j);
            if (need(kv, cp + ".type").trimmed() != QLatin1String("CLAIMANT_TYPE_V0"))
                throw std::runtime_error("txrep: a claimant's type must be CLAIMANT_TYPE_V0");
            const QString destination = needAccountId(kv, cp + ".v0.destination");
            std::unique_ptr<Predicate> predicate = parsePredicate(kv, cp + ".v0.predicate");
            // The Claimant owns the predicate from here; the list keeps copies.
            const Claimant claimant(destination, predicate.get());
            predicate.release();
            claimants.append(claimant);
        }
        op.reset(new CreateClaimableBalanceOperation(amount, asset.get(), claimants));
    } else if (type == "REVOKE_SPONSORSHIP") {
        const QString p = prefix + ".body.revokeSponsorshipOp";
        const QString rtype = need(kv, p + ".type");
        if (rtype == "REVOKE_SPONSORSHIP_SIGNER") {
            // py-stellar-sdk's form: `.signerKey.type` and a field per type.
            // This SDK wrote the key as one strkey line (`.signerKey: G...`,
            // SEP-11's form for a SignerKey) until 1.0.4, and that is still
            // read: a strkey states its own key type in its first letter and
            // carries a checksum, so it cannot be taken for another key. A
            // document with both forms leaves the single line unread, and is
            // refused.
            const QString keyPath = p + ".signer.signerKey";
            const SignerKey key = kv.find(keyPath + ".type")
                                      ? parseSignerKey(kv, keyPath)
                                      : SignerKey::fromString(need(kv, keyPath).trimmed());
            op.reset(new RevokeSignerSponsorshipOperation(
                         needAccountId(kv, p + ".signer.accountID"), key.toXdr()));
        } else if (rtype == "REVOKE_SPONSORSHIP_LEDGER_ENTRY") {
            const QString lk = need(kv, p + ".ledgerKey.type");
            if (lk == "ACCOUNT") {
                // py-stellar-sdk: `ledgerKey.account`; stc: the struct's one
                // field, `ledgerKey.account.accountID`.
                const QString account = p + ".ledgerKey.account";
                op.reset(new RevokeAccountSponsorshipOperation(needAccountId(
                             kv, !kv.has(account) && kv.has(account + ".accountID")
                                     ? account + ".accountID" : account)));
            } else if (lk == "TRUSTLINE") {
                const QString account = needAccountId(kv, p + ".ledgerKey.trustLine.accountID");
                const QString assetKey = p + ".ledgerKey.trustLine.asset";
                const QString line = need(kv, assetKey).trimmed();
                if (line.endsWith(QLatin1String(":lp"))) {
                    // A trustline of pool shares, as SEP-11 writes it: the
                    // pool id in hex and ":lp". No asset code is 64
                    // characters long and no issuer is "lp", so it is never
                    // an asset read the wrong way.
                    op.reset(RevokeTrustlineSponsorshipOperation::create(
                                 account, hexBytes(line.left(line.size() - 3), assetKey, 32)));
                } else {
                    std::unique_ptr<Asset> asset(assetFromTxrep(line));
                    op.reset(new RevokeTrustlineSponsorshipOperation(account, asset.get()));
                }
            } else if (lk == "OFFER") {
                op.reset(new RevokeOfferSponsorshipOperation(
                             needAccountId(kv, p + ".ledgerKey.offer.sellerID"),
                             needInt64(kv, p + ".ledgerKey.offer.offerID")));
            } else if (lk == "DATA") {
                // Bare, as py-stellar-sdk writes it, and only a name py reads
                // back whole. py reads a bare value up to its first space and
                // takes quotes as part of the name: its document for the name
                // "x" (quotes included) was signed here as x, another entry,
                // and `a b` was a b here and a to py. Nothing may be taken off
                // it as a comment either: "k (1)" used to be signed as "k".
                const QString key = p + ".ledgerKey.data.dataName";
                const QString dataName = need(kv, key);
                if (kv.raw(key) != dataName || !readsBackBare(dataName))
                    throw std::runtime_error("txrep: a revoke-data name is written bare, as "
                                             "py-stellar-sdk reads it: no quotes, no space, "
                                             "no comment");
                op.reset(new RevokeDataSponsorshipOperation(
                             needAccountId(kv, p + ".ledgerKey.data.accountID"), dataName));
            } else if (lk == "CLAIMABLE_BALANCE") {
                op.reset(new RevokeClaimableBalanceSponsorshipOperation(
                             needBalanceId(kv, p + ".ledgerKey.claimableBalance.balanceID")));
            } else if (lk == "LIQUIDITY_POOL") {
                op.reset(new RevokeLiquidityPoolSponsorshipOperation(
                             need(kv, p + ".ledgerKey.liquidityPool.liquidityPoolID")));
            } else {
                throw std::runtime_error(
                    ("Txrep parser: unknown revoke ledgerKey type: " + lk).toStdString());
            }
        } else {
            throw std::runtime_error(
                ("Txrep parser: unknown revoke sponsorship type: " + rtype).toStdString());
        }
    } else if (type == "INFLATION") {
        // No fields. py-stellar-sdk writes it; it used to be refused here.
        op.reset(new InflationOperation());
    } else {
        // Soroban op types are not parsed (see emit note + the stash file).
        throw std::runtime_error(("Txrep parser: unsupported operation type: " + type).toStdString());
    }

    if (present(kv, prefix + ".sourceAccount"))
        op->setSourceAccount(needMuxedAccount(kv, prefix + ".sourceAccount"));
    return op;
}

/**
 * A transaction's fields, from its source account to its ext, under `tx`:
 * "tx", or "feeBump.tx.innerTx.tx" for the one a fee bump wraps.
 */
void emitTransaction(QString& out, const QString& tx, Transaction* transaction)
{
    // Soroban data is the ext's v1 arm, which TxRep has no form for:
    // `ext.v: 0` would describe another transaction.
    if (transaction->getSorobanData())
        throw std::runtime_error("Txrep emitter: a transaction with Soroban data has no TxRep form");
    appendAccountLine(out, tx + ".sourceAccount", transaction->getSourceAccount());
    appendLine(out, tx + ".fee", QString::number(transaction->getFee()));
    appendLine(out, tx + ".seqNum", QString::number(transaction->getSequenceNumber()));

    // Preconditions: PRECOND_NONE / PRECOND_TIME / PRECOND_V2.
    const TransactionPreconditions& pre = transaction->getPreconditions();
    TimeBounds* tb = pre.getTimeBounds();
    if (pre.hasV2()) {
        // PRECOND_V2: py-stellar-sdk renders the V2 fields directly under
        // `tx.cond.` (the union arm `.v2` is collapsed), and decomposes each
        // extra signer into `.type` + a key-type-specific strkey field. (stc
        // writes them under `tx.cond.v2.`, which is read too.)
        appendLine(out, tx + ".cond.type", "PRECOND_V2");
        if (tb) {
            appendLine(out, tx + ".cond.timeBounds._present", "true");
            appendLine(out, tx + ".cond.timeBounds.minTime", withUtc(tb->getMinTimePoint()));
            appendLine(out, tx + ".cond.timeBounds.maxTime", withUtc(tb->getMaxTimePoint()));
        } else {
            appendLine(out, tx + ".cond.timeBounds._present", "false");
        }
        LedgerBounds* lb = pre.getLedgerBounds();
        if (lb) {
            appendLine(out, tx + ".cond.ledgerBounds._present", "true");
            appendLine(out, tx + ".cond.ledgerBounds.minLedger", QString::number(lb->getMinLedger()));
            appendLine(out, tx + ".cond.ledgerBounds.maxLedger", QString::number(lb->getMaxLedger()));
        } else {
            appendLine(out, tx + ".cond.ledgerBounds._present", "false");
        }
        qint64* minSeq = pre.getMinSeqNumber();
        if (minSeq) {
            appendLine(out, tx + ".cond.minSeqNum._present", "true");
            appendLine(out, tx + ".cond.minSeqNum", QString::number(*minSeq));
        } else {
            appendLine(out, tx + ".cond.minSeqNum._present", "false");
        }
        appendLine(out, tx + ".cond.minSeqAge", QString::number(pre.getMinSeqAge()));
        appendLine(out, tx + ".cond.minSeqLedgerGap", QString::number(pre.getMinSeqLedgerGap()));
        const QList<SignerKey>& signers = pre.getExtraSigners();
        appendLine(out, tx + ".cond.extraSigners.len", QString::number(signers.size()));
        for (int i = 0; i < signers.size(); ++i)
            emitSignerKey(out, tx + QString(".cond.extraSigners[%1]").arg(i), signers.at(i));
    } else if (tb) {
        appendLine(out, tx + ".cond.type", "PRECOND_TIME");
        appendLine(out, tx + ".cond.timeBounds.minTime", withUtc(tb->getMinTimePoint()));
        appendLine(out, tx + ".cond.timeBounds.maxTime", withUtc(tb->getMaxTimePoint()));
    } else {
        appendLine(out, tx + ".cond.type", "PRECOND_NONE");
    }

    // Memo — all SEP-11 types.
    Memo* memo = transaction->getMemo();
    if (!memo || dynamic_cast<MemoNone*>(memo)) {
        appendLine(out, tx + ".memo.type", "MEMO_NONE");
    } else if (auto* mt = dynamic_cast<MemoText*>(memo)) {
        appendLine(out, tx + ".memo.type", "MEMO_TEXT");
        // The signed bytes, shown as text when they are UTF-8 and as hex when
        // they are not (as py-stellar-sdk does): decoding them as UTF-8 showed
        // replacement characters that were not what was signed.
        const QByteArray bytes = mt->getBytes();
        if (isValidUtf8(bytes))
            appendLine(out, tx + ".memo.text", quoted(utf8Text(bytes)));
        else
            appendLine(out, tx + ".memo.text", hex(bytes));
    } else if (auto* mi = dynamic_cast<MemoId*>(memo)) {
        appendLine(out, tx + ".memo.type", "MEMO_ID");
        appendLine(out, tx + ".memo.id", QString::number(mi->getId()));
    } else if (auto* mh = dynamic_cast<MemoHash*>(memo)) {
        appendLine(out, tx + ".memo.type", "MEMO_HASH");
        appendLine(out, tx + ".memo.hash", mh->getHexValue());
    } else if (auto* mr = dynamic_cast<MemoReturnHash*>(memo)) {
        appendLine(out, tx + ".memo.type", "MEMO_RETURN");
        appendLine(out, tx + ".memo.retHash", mr->getHexValue());
    } else {
        throw std::runtime_error("Txrep emitter: unsupported memo type");
    }

    QVector<Operation*> ops = transaction->getOperations();
    appendLine(out, tx + ".operations.len", QString::number(ops.size()));
    for (int i = 0; i < ops.size(); ++i) {
        emitOperation(out, tx, ops.at(i), i);
    }

    appendLine(out, tx + ".ext.v", "0");
}

/**
 * A signature list, as py-stellar-sdk writes it, under `prefix`: "" for a
 * transaction's, "feeBump.tx.innerTx." and "feeBump." for a fee bump's two.
 */
void emitSignatures(QString& out, const QString& prefix,
                    const QVector<stellar::DecoratedSignature>& signatures)
{
    appendLine(out, prefix + "signatures.len", QString::number(signatures.size()));
    for (int i = 0; i < signatures.size(); ++i) {
        const QString sp = prefix + QString("signatures[%1]").arg(i);
        const stellar::DecoratedSignature& signature = signatures.at(i);
        appendLine(out, sp + ".hint",
                   hex(QByteArray(reinterpret_cast<const char*>(signature.hint.signatureHint), 4)));
        appendLine(out, sp + ".signature",
                   hex(QByteArray(reinterpret_cast<const char*>(signature.signature.value.constData()),
                                  signature.signature.value.size())));
    }
}

/**
 * The document's `key: value` lines, after the checks on its text as a whole.
 */
TxrepMap readLines(const QString& txrep)
{
    // A byte-order mark in front is not part of the document.
    QString text = txrep;
    if (text.startsWith(QChar(0xfeff)))
        text.remove(0, 1);
    // Nothing invisible and nothing that breaks or reorders a line, anywhere,
    // comment lines included: what a reviewer sees must be what is signed.
    // Inside a string such a character is written as a \uXXXX escape, which
    // is how this emitter writes it. A CR is taken only as the first half of
    // a CRLF line end: alone, an editor or a terminal shows it as a line
    // break (or goes back to the start of the line), so "# ok\r<key>: <value>"
    // showed a line the parser never read.
    for (int i = 0; i < text.size();) {
        int units = 1;
        const uint c = codePointAt(text, i, &units);
        const bool lineEnd = c == '\n'
                || (c == '\r' && (i + 1 == text.size() || text.at(i + 1) == QLatin1Char('\n')));
        if (!lineEnd && isHiddenOrBreaking(c))
            throw std::runtime_error(QStringLiteral("txrep: the document contains an invisible or "
                                                    "line-breaking character (U+%1); inside a string, "
                                                    "write it as a \\u escape")
                                         .arg(c, 4, 16, QLatin1Char('0')).toStdString());
        i += units;
    }

    // Parse the dotted-path key/value lines into a flat map.
    TxrepMap kv;
    for (const QString& rawLine : text.split('\n')) {
        QString line = rawLine.trimmed();
        // '#' and SEP-11's ':' begin a comment line.
        if (line.isEmpty() || line.startsWith('#') || line.startsWith(':')) continue;
        int sep = line.indexOf(':');
        // A line that is not `key: value` is not skipped: it may be the field
        // the reviewer read.
        if (sep < 0)
            throw std::runtime_error(("txrep: not a `key: value` line: " + line).toStdString());
        QString value = line.mid(sep + 1).trimmed();
        const QString raw = value;
        // stc / Laboratory / py-stellar-sdk append a human-readable comment to
        // typed values, e.g. `amount: 105000000 (10.5)` or
        // `minTime: 0 (1970-01-01 ... (UTC))`. Take a trailing ` (...)` off
        // the value; checkComments() then holds it to the value. Quoted
        // strings are taken verbatim (their content may legitimately contain
        // " (").
        if (!value.startsWith('"')) {
            int comment = value.indexOf(" (");
            if (comment >= 0) value = value.left(comment).trimmed();
        }
        kv.insert(line.left(sep).trimmed(), value, raw);
    }
    return kv;
}

/**
 * A transaction a document describes, in a builder ready to build it, and
 * beside it what the builder cannot be told: Txrep::read, a friend of the
 * builder and of Transaction, puts that in place.
 */
struct ParsedTransaction
{
    std::unique_ptr<Account> account;           // the builder's source; it does not own it
    std::unique_ptr<TransactionBuilder> builder;
    bool untimed = false;       // no time bounds: setTimeout() would add {0, 0}
    qint64 fee = 0;             // exactly the document's: the builder was given a placeholder
    QString sourceAccount;      // exactly as written: the builder writes the account's G... key
};

/**
 * The transaction a document describes under `tx`: "tx", or
 * "feeBump.tx.innerTx.tx" for the one a fee bump wraps. A V0 transaction
 * (`v0`) has a G... source and at most time bounds: a V0 document that gives
 * it an M... source or V2 preconditions is refused, not signed without them.
 */
ParsedTransaction parseTransaction(const TxrepMap& kv, const QString& tx, Network* network, bool v0)
{
    ParsedTransaction parsed;
    const auto field = [&tx](const char* name) -> QString {
        return tx + QLatin1Char('.') + QLatin1String(name);
    };

    // G... or M...: a muxed source used to be refused, though this emitter
    // wrote it (and py-stellar-sdk writes it).
    const QString sourceAccount = v0 ? needAccountId(kv, field("sourceAccount"))
                                     : needMuxedAccount(kv, field("sourceAccount"));
    // The fee is the envelope's uint32, exactly as the document states it.
    const qint64 fee = needInteger(kv, field("fee"), 0, std::numeric_limits<quint32>::max());
    // build() increments the account's sequence, so it must start one below
    // and cannot start below the smallest value that leaves it valid.
    const qint64 seqNum = needInteger(kv, field("seqNum"), std::numeric_limits<qint64>::min() + 1,
                                      std::numeric_limits<qint64>::max());
    // More than MAX_OPS_PER_TX would not fit the envelope.
    const qint64 opCount = needInteger(kv, field("operations.len"), 0, stellar::MAX_OPS_PER_TX);
    refuseEntriesPast(kv, field("operations"), static_cast<int>(opCount));

    // Account starts at seqNum-1 so that build() increments it to seqNum.
    // Owned here: the builder does not take it, and any parse error below used
    // to leak it. The builder writes the account's G... key; an M... source is
    // put in place after build(), as the fee is.
    parsed.account.reset(
        new Account(MuxedAccount::fromAddress(sourceAccount).getAccountId(), seqNum - 1));
    parsed.builder.reset(
        new TransactionBuilder(AccountConverter().enableMuxed(), parsed.account.get(), network));
    parsed.fee = fee;
    parsed.sourceAccount = sourceAccount;
    TransactionBuilder& builder = *parsed.builder;
    // A placeholder that satisfies the builder; the document's exact total is
    // put in place after build(). It used to be quint32(fee / opCount) with the
    // builder multiplying back, which dropped the remainder (1000 over three
    // operations came back as 999) and wrapped what a uint32 could not hold —
    // so the transaction signed was not the document reviewed.
    builder.setBaseFee(TransactionBuilder::BASE_FEE);

    // A V0 transaction's own field, as stc writes it (SEP-11's path):
    // `tx.timeBounds._present` and the bounds. py-stellar-sdk writes a V0
    // one's as a V1 one's, under `tx.cond.`; a document gives one of them.
    const bool v0TimeBounds = v0 && !kv.has(field("cond.type")) && kv.has(field("timeBounds._present"));
    // Required: a document whose type line was missing used to be signed with
    // no preconditions at all, whatever time bounds it listed.
    const QString condType = v0TimeBounds ? QString() : need(kv, field("cond.type"));
    if (v0TimeBounds) {
        if (present(kv, field("timeBounds"))) {
            builder.addTimeBounds(TimeBounds::fromTimePoints(needTime(kv, field("timeBounds.minTime")),
                                                             needTime(kv, field("timeBounds.maxTime"))));
        } else {
            parsed.untimed = true;
        }
    } else if (condType == "PRECOND_TIME") {
        const quint64 minT = needTime(kv, field("cond.timeBounds.minTime"));
        const quint64 maxT = needTime(kv, field("cond.timeBounds.maxTime"));
        builder.addTimeBounds(TimeBounds::fromTimePoints(minT, maxT));
    } else if (condType == "PRECOND_V2" && !v0) {
        TransactionPreconditions pre;
        // A V2 document is a V2 envelope, even when every V2 field is zero: it
        // is a different transaction (and hash) from the PRECOND_TIME one.
        pre.setV2(true);
        // py-stellar-sdk writes the fields directly under `cond.`; stc under
        // `cond.v2.` (SEP-11's path: the arm is named), each extra signer as
        // one strkey line. A document gives one of them: the other's lines
        // would be left unread, and are refused. stc always writes minSeqAge.
        const bool stcForm = kv.has(field("cond.v2.minSeqAge"));
        const QString cond = field(stcForm ? "cond.v2." : "cond.");
        if (present(kv, cond + "timeBounds")) {
            pre.setTimeBounds(TimeBounds::fromTimePoints(
                needTime(kv, cond + "timeBounds.minTime"),
                needTime(kv, cond + "timeBounds.maxTime")));
        }
        if (present(kv, cond + "ledgerBounds")) {
            pre.setLedgerBounds(new LedgerBounds(
                needUInt32(kv, cond + "ledgerBounds.minLedger"),
                needUInt32(kv, cond + "ledgerBounds.maxLedger")));
        }
        if (present(kv, cond + "minSeqNum")) {
            pre.setMinSeqNumber(new qint64(needInt64(kv, cond + "minSeqNum")));
        }
        // Always written (they are not optional in the XDR); read as 0 when
        // absent, a document's age or gap condition vanished from the signed
        // transaction.
        pre.setMinSeqAge(needUInt64(kv, cond + "minSeqAge"));
        pre.setMinSeqLedgerGap(needUInt32(kv, cond + "minSeqLedgerGap"));
        // Required, as the emitter always writes it: read as 0 when absent, a
        // document listing extra signers lost them — a transaction with fewer
        // preconditions than the one shown.
        const int signerCount = needCount(kv, cond + "extraSigners.len", 2);
        refuseEntriesPast(kv, cond + "extraSigners", signerCount);
        // Each extra signer is `.type` + a key-type-specific strkey field
        // (py-stellar-sdk's form), or one strkey (stc's), which states its
        // own type in its first letter and carries a checksum.
        for (int i = 0; i < signerCount; ++i) {
            const QString signer = cond + QString("extraSigners[%1]").arg(i);
            pre.addExtraSigner(stcForm ? SignerKey::fromString(need(kv, signer).trimmed())
                                       : parseSignerKey(kv, signer));
        }
        builder.addPreconditions(pre);
        // PRECOND_V2 without timeBounds still needs the builder's timeout
        // invariant satisfied (build() requires bounds or an explicit timeout).
        // Not through setTimeout(TIMEOUT_INFINITE): that now writes {0, 0},
        // and the document's own hash has none.
        parsed.untimed = !pre.getTimeBounds();
    } else if (condType == "PRECOND_NONE") {
        parsed.untimed = true;
    } else if (v0 && condType == "PRECOND_V2") {
        throw std::runtime_error("txrep: a V0 transaction holds time bounds at most, not PRECOND_V2");
    } else {
        throw std::runtime_error(("Txrep parser: unsupported cond type: " + condType).toStdString());
    }

    // Required, like the cond type: a missing type line used to mean "no memo"
    // while the document still showed the memo text — the deposit reference
    // an exchange needs.
    const QString memoType = need(kv, field("memo.type"));
    if (memoType == "MEMO_NONE") {
        builder.addMemo(Memo::none());
    } else if (memoType == "MEMO_TEXT") {
        // Quoted text, or bare hex: py-stellar-sdk writes a memo that is not
        // UTF-8 as hex, and the bytes are what is signed.
        const QString text = need(kv, field("memo.text")).trimmed();
        if (text.startsWith(QLatin1Char('"'))) {
            builder.addMemo(Memo::text(unquote(text)));
        } else {
            // Bare hex is how py-stellar-sdk writes a memo that is not UTF-8,
            // and only that. Text that could have been quoted must be: bare,
            // "12345678" would be signed as the four bytes 12 34 56 78.
            if (kv.raw(field("memo.text")) != need(kv, field("memo.text")))
                throw std::runtime_error("txrep: a bare memo text cannot carry a comment");
            const QByteArray bytes = needHex(kv, field("memo.text"));
            if (isValidUtf8(bytes))
                throw std::runtime_error("txrep: quote the memo text; bare hex is only for a memo "
                                         "that is not UTF-8");
            builder.addMemo(MemoText::fromBytes(bytes));
        }
    } else if (memoType == "MEMO_ID") {
        builder.addMemo(Memo::id(needUInt64(kv, field("memo.id"))));
    } else if (memoType == "MEMO_HASH") {
        // Exactly 32 bytes: Memo::hash pads a short one with zeros.
        builder.addMemo(Memo::hash(needHex(kv, field("memo.hash"), 32)));
    } else if (memoType == "MEMO_RETURN") {
        builder.addMemo(Memo::returnHash(needHex(kv, field("memo.retHash"), 32)));
    } else {
        throw std::runtime_error(("Txrep parser: unsupported memo type: " + memoType).toStdString());
    }

    for (int i = 0; i < opCount; ++i) {
        std::unique_ptr<Operation> op = parseOperation(kv, tx, i);
        builder.addOperation(op.get());     // the builder owns it from here
        op.release();
    }

    needInteger(kv, field("ext.v"), 0, 0);
    return parsed;
}

/**
 * A signature list under `prefix`: "" for a transaction's, "feeBump." and
 * "feeBump.tx.innerTx." for a fee bump's two. Each is what it says: a count
 * with the entries it counts, a 4-byte hint and a signature of at most 64
 * bytes. A list that is not there is no signatures, as SEP-11 reads a
 * missing array. A comment on a hint is checked against the transaction the
 * signature is for, once it is built (checkSignatureNotes).
 */
QVector<stellar::DecoratedSignature> parseSignatures(const TxrepMap& kv, const QString& prefix)
{
    QVector<stellar::DecoratedSignature> signatures;
    const QString len = prefix + QStringLiteral("signatures.len");
    if (!kv.find(len))
        return signatures;
    const int count = needCount(kv, len, stellar::MAX_SIGNATURES_PER_TX);
    refuseEntriesPast(kv, prefix + QStringLiteral("signatures"), count);
    for (int i = 0; i < count; ++i) {
        const QString sp = prefix + QStringLiteral("signatures[%1]").arg(i);
        const QByteArray hint = needHex(kv, sp + QStringLiteral(".hint"), 4);
        kv.setKind(sp + QStringLiteral(".hint"), ValueKind::Hint);
        QByteArray signature = needOpaque(kv, sp + QStringLiteral(".signature"));
        if (signature.size() > 64)
            throw std::runtime_error("txrep: a signature is at most 64 bytes");
        stellar::DecoratedSignature decorated;
        memcpy(decorated.hint.signatureHint, hint.constData(), 4);
        decorated.signature.set(reinterpret_cast<quint8*>(signature.data()), signature.size());
        signatures.append(decorated);
    }
    return signatures;
}

/**
 * Whether `said` names the signer of `signature`, on the transaction whose
 * hash is `hash`: the signer key's strkey, as SEP-11's example and stc write
 * it (`4aa07ed0 (GAVRMS...)`), of the signature's hint, and one the
 * signature is valid for. A reader takes it for who signed, and stc names a
 * key only then; a hint is four bytes, and a key that ends in any four is
 * found in moments, so the hint alone let a comment name a signer who had
 * not signed. An ed25519 key signs the hash; a hash-x signer's signature is
 * its preimage; a signed-payload signer signs its payload, under the hint
 * CAP-40 gives it. A pre-authorized transaction signs nothing.
 */
bool namesTheSigner(const QString& said, const stellar::DecoratedSignature& signature,
                    const QByteArray& hash)
{
    const QByteArray hint(reinterpret_cast<const char*>(signature.hint.signatureHint), 4);
    const QByteArray bytes(reinterpret_cast<const char*>(signature.signature.value.constData()),
                           signature.signature.value.size());
    try {
        const SignerKey key = SignerKey::fromString(said);
        if (key.toString() != said)
            return false;
        switch (key.getType()) {
        case stellar::SignerKeyType::SIGNER_KEY_TYPE_ED25519: {
            std::unique_ptr<KeyPair> signer(KeyPair::fromPublicKey(key.getEd25519()));
            return key.getEd25519().right(4) == hint && signer->verify(hash, bytes);
        }
        case stellar::SignerKeyType::SIGNER_KEY_TYPE_HASH_X:
            return key.getHashX().right(4) == hint
                   && QCryptographicHash::hash(bytes, QCryptographicHash::Sha256) == key.getHashX();
        case stellar::SignerKeyType::SIGNER_KEY_TYPE_ED25519_SIGNED_PAYLOAD: {
            const QByteArray payload = key.getSignedPayload();
            const QByteArray tail = payload.size() >= 4 ? payload.right(4)
                                                        : payload + QByteArray(4 - payload.size(), '\0');
            QByteArray expected = key.getEd25519().right(4);
            for (int i = 0; i < 4; ++i)
                expected[i] = static_cast<char>(expected.at(i) ^ tail.at(i));
            std::unique_ptr<KeyPair> signer(KeyPair::fromPublicKey(key.getEd25519()));
            return expected == hint && signer->verify(payload, bytes);
        }
        default:
            return false;
        }
    } catch (const std::exception&) {
        return false;
    }
}

/**
 * stc's note on a signature it could not match to a key it knows: "bad
 * signature/unknown key/test is wrong network", `test` being its name for
 * the network. It names no signer and states no value, so it is read; the
 * name is one word, so that nothing else can be written into the note.
 */
bool isStcUnmatchedSignatureNote(const QString& said)
{
    static const QRegularExpression note(QStringLiteral(
        "^bad signature/unknown key/[A-Za-z0-9_.-]+ is wrong network$"));
    return note.match(said).hasMatch();
}

/**
 * The comments on the hints of a signature list under `prefix` (see
 * parseSignatures), against the transaction the signatures are for: each
 * names the key that made the signature (namesTheSigner) or is stc's note
 * on one it could not match, or the document is refused. A comment naming
 * another key than the hint's was read past before 1.0.4, and until now one
 * naming a key whose signature it is not.
 */
void checkSignatureNotes(const TxrepMap& kv, const QString& prefix,
                         const QVector<stellar::DecoratedSignature>& signatures, const QByteArray& hash)
{
    for (int i = 0; i < signatures.size(); ++i) {
        const QString key = prefix + QStringLiteral("signatures[%1].hint").arg(i);
        const QString comment = kv.raw(key).mid(kv.peek(key).size()).trimmed();
        if (comment.isEmpty())
            continue;
        const QString said = comment.mid(1, comment.size() - 2);
        const bool ok = comment.startsWith(QLatin1Char('(')) && comment.endsWith(QLatin1Char(')'))
                        && (namesTheSigner(said, signatures.at(i), hash)
                            || isStcUnmatchedSignatureNote(said));
        if (!ok) {
            throw std::runtime_error(("txrep: the comment on " + key + " must name the key that made "
                                      "the signature, or be left out").toStdString());
        }
    }
}

/**
 * A decimal in its one spelling ("10.5", "0", "-3"), exponent applied; empty
 * when the text is not a plain number. ASCII digits only: "٩٩" is
 * ninety-nine to some readers.
 */
QString canonicalDecimal(const QString& text)
{
    static const QRegularExpression number(
        QStringLiteral("^([+-]?)([0-9]*)(?:\\.([0-9]*))?(?:[eE]([+-]?[0-9]+))?$"));
    const QRegularExpressionMatch m = number.match(text);
    if (!m.hasMatch() || (m.captured(2).isEmpty() && m.captured(3).isEmpty()))
        return QString();
    QString digits = m.captured(2) + m.captured(3);
    int point = m.captured(2).size();
    if (!m.captured(4).isEmpty()) {
        bool ok = false;
        const int exponent = m.captured(4).toInt(&ok);
        if (!ok || exponent > 64 || exponent < -64)
            return QString();
        point += exponent;
    }
    while (point < 0) { digits.prepend(QLatin1Char('0')); ++point; }
    while (point > digits.size()) digits.append(QLatin1Char('0'));
    QString whole = digits.left(point), fraction = digits.mid(point);
    while (whole.size() > 1 && whole.startsWith(QLatin1Char('0'))) whole.remove(0, 1);
    if (whole.isEmpty()) whole = QStringLiteral("0");
    while (fraction.endsWith(QLatin1Char('0'))) fraction.chop(1);
    QString out = fraction.isEmpty() ? whole : whole + QLatin1Char('.') + fraction;
    if (m.captured(1) == QLatin1String("-") && out != QLatin1String("0"))
        out.prepend(QLatin1Char('-'));
    return out;
}

/**
 * The number a comment states, in its one spelling (canonicalDecimal), and
 * whether it is written with the exponent 7: SEP-11's and stc's mark of an
 * amount in stroops ("(40.0004e7)", "(10.5e7)"). Digits grouped by commas
 * are read only exactly as stc writes them (its ScaleFmt: "(1,000e7)",
 * "(-12,345.6789012e7)"): "1,000" alone is a thousand to one reader and one
 * to another. And ScaleFmt's first group is never 0 when commas follow: a
 * "0," is how a locale with a decimal comma writes a fraction, so
 * "(0,100e7)" after 100 units is 0.1e7 there, a thousandth of what is
 * signed (it was read, as "(00,001,000e7)" and "(0,000,000,100e7)" were).
 * Empty when the text is not a number.
 */
QString statedNumber(const QString& text, bool* e7)
{
    static const QRegularExpression grouped(
        QStringLiteral("^-?[1-9][0-9]{0,2}(?:,[0-9]{3})+(?:\\.[0-9]*[1-9])?e7$"));
    static const QRegularExpression exponent(QStringLiteral("[eE]([+-]?[0-9]+)$"));
    QString plain = text;
    if (plain.contains(QLatin1Char(','))) {
        if (!grouped.match(plain).hasMatch())
            return QString();
        plain.remove(QLatin1Char(','));
    }
    const QRegularExpressionMatch m = exponent.match(plain);
    *e7 = m.hasMatch() && m.captured(1).toInt() == 7;
    return canonicalDecimal(plain);
}

/** Seconds in `hours`:`minutes`:`seconds`, for the table below. */
constexpr int east(int hours, int minutes, int seconds = 0)
{
    return (hours * 60 + minutes) * 60 + seconds;
}

/** A zone abbreviation that names one offset, and it, in seconds east of UTC. */
struct ZoneAbbreviation
{
    const char* name;
    int offset;
};

/**
 * The zone abbreviations that name one UTC offset to whoever reads them,
 * and it. Fixed here, and never taken from the machine's time-zone
 * database, so that a document reads alike everywhere.
 *
 * UTC and GMT are an explicit exemption from the rules below: they are read
 * at +00:00, before this table is (zoneOffset), because that is what they
 * mean to every reader and in every tz release, though not every writer
 * has kept to it. Old Go releases on Windows wrote UTC, the capitals of the
 * zone's name, for Windows' "UTC-09", "UTC-08" and "UTC+13" zones, at
 * -09:00, -08:00 and +13:00; and Java gives GMT to some past instants at
 * other offsets. A date written so does not name its instant at +00:00,
 * and the document is refused, not read at another time.
 *
 * Derived once, offline, from the 57 abbreviations that name one offset in
 * tzdata 2026d alone (the first table of 1.0.4). One is kept only if all of
 * these hold:
 *  - In every tz release obtained, every zone that uses it at any instant
 *    of [1970-01-01, 2100-01-01) UTC uses it at this one offset. The
 *    releases: all 278 IANA ones, from tzdata93b (1993) to tzdata2026d
 *    (2026), and the 1997 tzdatabeta, each compiled by zic 2026d over that
 *    range (-r @0/@4102444800, every transition explicit) and read back,
 *    and those from 2014g (the first with backzone) compiled again with
 *    backzone, as a PACKRATDATA build does (2017c's does not build); and
 *    the compiled data of every pytz release on PyPI (124: 2004a to 2026.4)
 *    and of every 'tzdata' package (37: 1.2016.5a1 to 2026.4).
 *  - Nothing else in wide use writes it for another offset, or names a
 *    region's time by it at another offset. Checked: Go 1.2 to 1.27 on
 *    Windows (Go's table of Windows zones, or the capitals of the zone's
 *    English name it falls back to), under the Windows rules in force from
 *    each release on; Java 8u491 and 17.0.12 (Date.toString(), and the
 *    short names of every locale), from their release on, since Java gives
 *    a zone today's names at past instants too (CET to Algiers' +00:00
 *    before 1981), a slip no reader learns a name from; the short names of
 *    CLDR 27 to 48 in every locale, from 1970 to each release (standard,
 *    daylight and generic names: a generic name is the zone's wall time, so
 *    +11:00 in a Sydney summer); Wikipedia's list of time zone
 *    abbreviations; and the capitals of Windows' zone names (JST's aside:
 *    see below).
 *  - It spells no other name of a time: a summer's S is read as "Standard"
 *    where that names the standard time (Hong Kong Standard Time, Pakistan
 *    Standard Time, and Western European Standard Time, CLDR's name for
 *    western Europe's standard +00:00; its "Western European Time" is the
 *    generic name, the wall time, +01:00 in a Lisbon summer).
 *
 * Dropped, with the other offsets each is read at:
 *  - In tz releases: AST (+03:00 in Arabia and Iraq, 93g-2016j; -05:00 in
 *    Acre, 93b-1999d), ADT (Baghdad's summer +04:00, 93g-2016j, and +03:00,
 *    1996n-2000f; Acre's -04:00, 93b-1999d), GST (the Gulf's +04:00,
 *    93g-2016j, and South Georgia's -02:00, 96k-2016j; Java and CLDR still
 *    write it for Dubai, and Go did to 1.8), BDT (Dhaka +06:00, 96k-2016j),
 *    MMT (Yangon +06:30, 96k-2016j), EST (+10:00, and +11:00 in summer, in
 *    eastern Australia, 93b-2014e), EST and EDT (-07:00 and -06:00 in
 *    Denver, 95m-96e; -03:00 and -02:00 in eastern Brazil, 93b-1999d; -06:00
 *    and -05:00 on Easter Island, 93b-93f), ACST (Acre -04:00, 1999e-2016j),
 *    CAST (Casey +11:00, 2009r-2016f; N'Djamena +02:00, 93g-1997h), IDT
 *    (Saigon +08:00, 2014i-2016j; Tehran +04:30, 93b-96i), WIT (Jakarta
 *    +07:00, 2001c-2013d), WAT (Bissau -01:00, 1997i-2016j; West Africa's
 *    +00:00, 93g-1997h), EET (+03:00 in Turkey, 93b-93f, Amman, 1999j-2000b,
 *    and Kaliningrad, 2011h; Go and Java write it for Istanbul, Amman and
 *    Damascus at +03:00), CAT (+01:00, 93g-1997h), SAST (Windhoek +03:00,
 *    1996n-1997h), AKST (Aqtobe +06:00, 1996n-2001d), GDT (Tehran +05:00,
 *    93g-96i), MSD (Minsk +03:00, 93g-94d; Magadan +11:00 and +12:00,
 *    95m-96i), NST (Nauru and Norfolk +11:30, 93g-96i), NDT (St. John's
 *    -01:30, 93g-95l) and SST (Singapore +08:00, 93b-93f).
 *  - Elsewhere: EEST (+02:00, Go 1.2-1.7 on Windows in Moldova), HST and
 *    HDT (-05:00 and -04:00, Go 1.2-1.7 in Haiti), MST and MDT (-03:00, Go
 *    1.2-1.8 in Punta Arenas; MST is Malaysia's +08:00 too), KDT (+13:00,
 *    Go on Windows' old Kamchatka zone), YST and YDT (-07:00: Windows'
 *    "Yukon Standard Time", written YST by Go 1.2-1.20), AWST (+11:00, CLDR
 *    34-37 and Java at Casey), WAST (+08:00, Windows' "W. Australia
 *    Standard Time"; the WAST of the tz releases, +02:00, is N'Djamena's
 *    summer of 1979-1980, and was Namibia's summer to 2017); WET, AEST,
 *    NZST, PKT and HKT, the names CLDR gives their zones' wall time (+01:00
 *    in a Lisbon summer, +11:00 and +13:00 in Sydney's and Auckland's,
 *    +06:00 in Pakistan's of 2002 and 2008-2009, +09:00 in Hong Kong's to
 *    1979; timeanddate.com gives AEST as Sydney's summer time too); and
 *    HKST, PKST and WEST, Hong Kong's summer +09:00 to 1979, Pakistan's
 *    +06:00 of 2002 and 2008-2009 and western Europe's summer +01:00, which
 *    spell "Hong Kong Standard Time" (listed so), "Pakistan Standard Time"
 *    and "Western European Standard Time", the names of +08:00, +05:00 and
 *    (CLDR's) +00:00. Java writes WEST at +02:00 too, for Lisbon's summers
 *    of 1992-1996.
 *
 * CET and CEST are kept, against the second rule and, CEST, the third
 * ("Central European Standard Time" is CLDR's name of +01:00), as an
 * explicit exemption: they are what stc writes on Windows in Madrid, and
 * every source above has them at +01:00 and +02:00 but these, which neither
 * Go nor stc follows: CLDR's generic "CET" in 96 locales (Central European
 * wall time, so +02:00 in summer), the capitals of Windows' "Central
 * European Standard Time" (Warsaw, +01:00), and Java's for Algiers and
 * Ceuta at their western European times before 1984. A date at CET is read
 * at +01:00 only, and one at CEST at +02:00 only.
 *
 * JST is kept at +09:00: it spells "Japan Standard Time", which is +09:00
 * itself, and no tz release, CLDR locale or Java has it at another offset.
 * The capitals of Windows' "Jordan Standard Time" and "Jerusalem Standard
 * Time" spell it too, but they name no one's time: Go, which falls back to
 * the capitals of a zone's name only for a zone missing from its table,
 * has written Jordan's and Israel's own abbreviations since 1.2, and stc
 * needs 1.15.
 */
const ZoneAbbreviation oneOffsetAbbreviations[] = {
    { "ACDT", east(10, 30) }, { "ADDT", -east(2, 0) }, { "AEDT", east(11, 0) },
    { "AHDT", -east(9, 0) }, { "AHST", -east(10, 0) }, { "AKDT", -east(8, 0) },
    { "AWDT", east(9, 0) }, { "CEST", east(2, 0) }, { "CET", east(1, 0) },
    { "ChST", east(10, 0) }, { "EAT", east(3, 0) }, { "JST", east(9, 0) },
    { "NDDT", -east(1, 30) }, { "NZDT", east(13, 0) }, { "WIB", east(7, 0) },
    { "WITA", east(8, 0) },
};

/**
 * Seconds east of UTC in a zone written as UTC, GMT, Z, an offset ("+04",
 * "-0930", "+05:30") or an abbreviation that names one offset
 * (oneOffsetAbbreviations: CET, CEST, JST...). False for anything else: an
 * abbreviation read at several offsets is none (CST is -06:00 in Chicago,
 * +08:00 in Shanghai and -05:00 in Havana; EST was Sydney's +10:00 too, GST
 * is Dubai's +04:00 and Guam's +10:00; IST, PST and BST are as many things),
 * and no reader can check a date given in one.
 */
bool zoneOffset(const QString& zone, qint64* offset)
{
    if (zone == QLatin1String("UTC") || zone == QLatin1String("GMT") || zone == QLatin1String("Z")) {
        *offset = 0;
        return true;
    }
    static const QRegularExpression numeric(QStringLiteral("^([+-])([0-9]{2})(?::?([0-9]{2}))?$"));
    const QRegularExpressionMatch m = numeric.match(zone);
    if (!m.hasMatch()) {
        for (const ZoneAbbreviation& abbreviation : oneOffsetAbbreviations) {
            if (zone == QLatin1String(abbreviation.name)) {
                *offset = abbreviation.offset;
                return true;
            }
        }
        return false;
    }
    const int hours = m.captured(2).toInt();
    const int minutes = m.captured(3).isEmpty() ? 0 : m.captured(3).toInt();
    if (hours > 23 || minutes > 59)
        return false;
    *offset = (hours * 60 + minutes) * 60 * (m.captured(1) == QLatin1String("-") ? -1 : 1);
    return true;
}

/** A date and time of day, as a clock shows them, and the day of the week. */
struct ClockReading
{
    qint64 year;
    int month, day, hour, minute, second;
    int weekday;                                    // 1 is Monday, 7 Sunday
};

/**
 * What a clock `offset` seconds east of UTC (less than a day either way)
 * shows at `instant` (seconds since 1970), in the proleptic Gregorian
 * calendar Go and python count in. Exact over the whole of qint64, and with
 * no year out of range: a TimePoint is any uint64, and stc dates every one
 * that is a positive int64 (1790626169993, milliseconds given as seconds, is
 * in the year 58712). The days are counted as in H. Hinnant's
 * civil_from_days.
 */
ClockReading clockReading(qint64 instant, qint64 offset)
{
    qint64 days = instant / 86400;
    qint64 time = instant % 86400;                  // never multiplied back: no overflow
    if (time < 0) { time += 86400; --days; }
    time += offset;
    if (time < 0) { time += 86400; --days; }
    if (time >= 86400) { time -= 86400; ++days; }
    ClockReading c;
    c.hour = int(time / 3600);
    c.minute = int(time / 60 % 60);
    c.second = int(time % 60);
    const qint64 fromThursday = (days % 7 + 7) % 7;  // 1970-01-01 was a Thursday
    c.weekday = int((fromThursday + 3) % 7) + 1;
    const qint64 z = days + 719468;                 // days since 0000-03-01
    const qint64 era = (z >= 0 ? z : z - 146096) / 146097;
    const qint64 dayOfEra = z - era * 146097;
    const qint64 yearOfEra = (dayOfEra - dayOfEra / 1460 + dayOfEra / 36524 - dayOfEra / 146096) / 365;
    const qint64 dayOfYear = dayOfEra - (365 * yearOfEra + yearOfEra / 4 - yearOfEra / 100);
    const qint64 fromMarch = (5 * dayOfYear + 2) / 153;
    c.day = int(dayOfYear - (153 * fromMarch + 2) / 5 + 1);
    c.month = int(fromMarch < 10 ? fromMarch + 3 : fromMarch - 9);
    c.year = yearOfEra + era * 400 + (c.month <= 2 ? 1 : 0);
    return c;
}

/**
 * Whether a comment states `instant` (seconds since 1970) as a date with
 * its zone (zoneOffset): as py-stellar-sdk writes one
 * (`2023-11-14 22:13:20.000000+00:00 (UTC)`), as ISO 8601 does
 * (`2023-11-14T22:13:20Z`, `...T23:13:20+01:00`), or as stc does, in Go's
 * time.UnixDate (`Tue Nov 14 22:13:20 UTC 2023`, `Wed Nov 15 02:13:20 +04
 * 2023`, `Tue Nov 14 23:13:20 CET 2023`), whose weekday must be the date's.
 * Go writes the year with all its digits, four or more (`Sat Sep  7
 * 22:19:53 UTC 58712`: a longer year was refused). False for anything else,
 * for a date that names no zone (the same clock reading is another instant
 * in every zone), and for one whose zone is an abbreviation that names
 * several offsets: stc writes `(... IST 2023)` in Kolkata, and `(... BST
 * 2023)` in a London summer.
 */
bool statesTime(const QString& text, qint64 instant)
{
    static const QRegularExpression iso(QStringLiteral(
        "^([0-9]{4})-([0-9]{2})-([0-9]{2})[T ]([0-9]{2}):([0-9]{2}):([0-9]{2})(?:\\.([0-9]+))?"
        "\\s*(Z|UTC|GMT|[+-][0-9]{2}:?[0-9]{2})?\\s*(\\(UTC\\))?$"));
    // Go pads a year to four digits, and writes a longer one as it is.
    static const QRegularExpression unixDate(QStringLiteral(
        "^(Mon|Tue|Wed|Thu|Fri|Sat|Sun) (Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec) {1,2}"
        "([0-9]{1,2}) ([0-9]{2}):([0-9]{2}):([0-9]{2}) ([^ ]+) ([0-9]{4}|[1-9][0-9]{4,})$"));
    static const QStringList weekdays = {
        QStringLiteral("Mon"), QStringLiteral("Tue"), QStringLiteral("Wed"), QStringLiteral("Thu"),
        QStringLiteral("Fri"), QStringLiteral("Sat"), QStringLiteral("Sun") };
    static const QStringList months = {
        QStringLiteral("Jan"), QStringLiteral("Feb"), QStringLiteral("Mar"), QStringLiteral("Apr"),
        QStringLiteral("May"), QStringLiteral("Jun"), QStringLiteral("Jul"), QStringLiteral("Aug"),
        QStringLiteral("Sep"), QStringLiteral("Oct"), QStringLiteral("Nov"), QStringLiteral("Dec") };
    ClockReading said = {};                         // weekday 0: none stated
    qint64 offset = 0;                              // seconds east of UTC
    bool yearRead = false;
    QRegularExpressionMatch m = iso.match(text);
    if (m.hasMatch()) {
        said.year = m.captured(1).toLongLong(&yearRead);
        said.month = m.captured(2).toInt();
        said.day = m.captured(3).toInt();
        said.hour = m.captured(4).toInt();
        said.minute = m.captured(5).toInt();
        said.second = m.captured(6).toInt();
        const QString fraction = m.captured(7);
        const QString zone = m.captured(8);
        const bool utcNote = !m.captured(9).isEmpty();
        // A TimePoint is whole seconds.
        if (fraction.count(QLatin1Char('0')) != fraction.size() || (zone.isEmpty() && !utcNote))
            return false;
        if (!zone.isEmpty() && !zoneOffset(zone, &offset))
            return false;
        if (utcNote && offset != 0)                 // "+01:00 (UTC)" says two things
            return false;
    } else if ((m = unixDate.match(text)).hasMatch()) {
        // A year too long for a qint64 is no instant's.
        said.year = m.captured(8).toLongLong(&yearRead);
        said.month = months.indexOf(m.captured(2)) + 1;
        said.day = m.captured(3).toInt();
        said.hour = m.captured(4).toInt();
        said.minute = m.captured(5).toInt();
        said.second = m.captured(6).toInt();
        // "Wed Nov 14 2023" names two days.
        said.weekday = weekdays.indexOf(m.captured(1)) + 1;
        if (!zoneOffset(m.captured(7), &offset))
            return false;
    } else {
        return false;
    }
    // The clock reading of the instant, compared field by field: a date that
    // is no date ("Feb 30", "24:00:00", a leap second) is none's.
    const ClockReading at = clockReading(instant, offset);
    return yearRead && said.year == at.year && said.month == at.month && said.day == at.day
           && said.hour == at.hour && said.minute == at.minute && said.second == at.second
           && (said.weekday == 0 || said.weekday == at.weekday);
}

/**
 * A comment after a value is for the reader, who trusts it: a reviewer who
 * sees `amount: 1050000000 (10.5)` takes the 10.5, while the parser signs
 * the number. So on every field, a comment must state that field's value in
 * a form checked here, or the document is refused:
 *
 *  - an amount, and a fee (the same kind of number): in units, as
 *    py-stellar-sdk writes it ("(10.5)", "(1E-7)"), or in stroops with the
 *    exponent 7, as SEP-11 and stc write it ("(10.5e7)", "(1,000e7)"). A
 *    plain number that is the amount only in stroops, "(105000000)", is
 *    refused: a py-trained reader takes it for 10^7 times more. And the
 *    exponent 7 always means stroops: read as units, "(1e7)" would be 10^7
 *    times what an stc-trained reader takes it for;
 *  - a time (minTime, maxTime, and absBefore and relBefore, which py
 *    annotates as dates too): the instant, as a date with its zone
 *    (statesTime), or the number itself, as stc writes absBefore ("(180e7)");
 *  - any other integer (a sequence number, an offer id, a count...): the
 *    number itself, in any of those spellings (stc's "(0.0012345e7)");
 *  - an account: py-stellar-sdk's comment after an M... address, checked
 *    where it is read (checkAccountComment);
 *  - a signature hint: the key that made the signature, or stc's note that
 *    it could not tell (checkSignatureNotes).
 *
 * Any other value (a type, an asset, a key, hex, a flag) takes no comment,
 * and anything else is refused, words included, whatever the field: words
 * state values too ("(ninety-nine)"), a unit can name another asset
 * ("(1.05e1 USD)"), a check on a leading number read past "(XLM 99.5)", and
 * a label ("(the exchange)", which stc writes from the writer's own
 * stc.conf) names someone nothing in the document can confirm. So is the
 * list of names stc's editor writes after a type on a "?" (`MEMO_NONE
 * (MEMO_TEXT, MEMO_ID, MEMO_HASH, MEMO_RETURN, MEMO_NONE)`): it states every
 * value but the one signed as much as that one, and any capitals would pass
 * for it. A comment on a field py-stellar-sdk never annotates used to be
 * stripped and ignored: `tx.seqNum: 77 (78)` was signed as 77.
 */
void checkComments(const TxrepMap& kv)
{
    static const QStringList amountFields = {
        QStringLiteral("amount"), QStringLiteral("startingBalance"), QStringLiteral("limit"),
        QStringLiteral("buyAmount"), QStringLiteral("sendMax"), QStringLiteral("destAmount"),
        QStringLiteral("sendAmount"), QStringLiteral("destMin"), QStringLiteral("maxAmountA"),
        QStringLiteral("maxAmountB"), QStringLiteral("minAmountA"), QStringLiteral("minAmountB") };
    // By their whole key: a pool's `constantProduct.fee` is basis points.
    static const QStringList feeKeys = {
        QStringLiteral("tx.fee"), QStringLiteral("feeBump.tx.fee"),
        QStringLiteral("feeBump.tx.innerTx.tx.fee") };
    static const QStringList timeFields = {
        QStringLiteral("minTime"), QStringLiteral("maxTime"), QStringLiteral("absBefore"),
        QStringLiteral("relBefore") };
    for (const QString& key : kv.keys()) {
        const ValueKind kind = kv.kind(key);
        const QString value = kv.peek(key);
        // Checked where they are read; a string takes none (unquote).
        if (kind == ValueKind::Account || kind == ValueKind::Hint || value.startsWith(QLatin1Char('"')))
            continue;
        const QString comment = kv.raw(key).mid(value.size()).trimmed();
        if (comment.isEmpty())
            continue;
        const QString name = key.mid(key.lastIndexOf(QLatin1Char('.')) + 1);
        // One statement in parentheses, and nothing after them.
        const QString said = comment.mid(1, comment.size() - 2).trimmed();
        bool ok = comment.startsWith(QLatin1Char('(')) && comment.endsWith(QLatin1Char(')'));
        bool e7 = false;
        QString form;
        if (kind == ValueKind::Number && (amountFields.contains(name) || feeKeys.contains(key))) {
            const QString number = statedNumber(said, &e7);
            ok = ok && !number.isEmpty()
                 && number == canonicalDecimal(e7 ? value : value + QStringLiteral("e-7"));
            form = QStringLiteral("the amount signed, in units (as py-stellar-sdk writes it) or in "
                                  "stroops with the exponent 7 (as SEP-11 and stc write it, 10.5e7)");
        } else if (kind == ValueKind::Number && timeFields.contains(name)) {
            bool isNumber = false;
            const qint64 signedValue = value.toLongLong(&isNumber);
            ok = ok && (statedNumber(said, &e7) == canonicalDecimal(value)
                        || (isNumber && statesTime(said, signedValue)));
            form = QStringLiteral("the time signed, as a date in UTC, at an offset such as +04 (as "
                                  "py-stellar-sdk writes it) or at a zone's abbreviation that names one "
                                  "offset (such as the CET and CEST stc writes in Madrid; not EST, GST, "
                                  "IST, CST, PST or BST, which are read at several), or as the number");
        } else if (kind == ValueKind::Number) {
            ok = ok && statedNumber(said, &e7) == canonicalDecimal(value);
            form = QStringLiteral("the number signed");
        } else {
            ok = false;
        }
        if (!ok) {
            throw std::runtime_error((form.isEmpty()
                                          ? "txrep: " + key + " takes no comment"
                                          : "txrep: the comment on " + key + " must be " + form
                                                + ", or be left out").toStdString());
        }
    }
}

/** The checks on a whole document, once all of it is read. */
void finishReading(const TxrepMap& kv)
{
    // Every line must have been read. One that was not (a field of an arm the
    // type does not select, a value under a `_present: false`, a key misspelt)
    // would otherwise be dropped from what gets signed without a word.
    const QStringList unread = kv.unread();
    if (!unread.isEmpty()) {
        throw std::runtime_error(("txrep: this document has lines that do not belong to "
                                  "what it describes: " + unread.join(QStringLiteral(", ")))
                                     .toStdString());
    }
    checkComments(kv);
}

} // namespace

QString Txrep::toTxrep(Transaction* transaction)
{
    return toTxrep(static_cast<AbstractTransaction*>(transaction));
}

QString Txrep::toTxrep(FeeBumpTransaction* transaction)
{
    return toTxrep(static_cast<AbstractTransaction*>(transaction));
}

QString Txrep::toTxrep(AbstractTransaction* transaction)
{
    if (!transaction) {
        throw std::runtime_error("transaction cannot be null");
    }

    // Written from the envelope, read back into objects by the SDK's own
    // decoder: the objects do not always hold what goes out (a transaction
    // built with muxing off sends its M... accounts as G..., and they were
    // written as M...), and the document must be the envelope that is signed
    // and sent: its type (a V0 one stays V0) and its signatures included.
    const stellar::TransactionEnvelope xdrEnvelope = transaction->toEnvelopeXdr();
    const QByteArray envelope = xdr::encodeOrThrow(xdrEnvelope, "the transaction envelope");
    Network* network = transaction->getNetwork();
    std::unique_ptr<AbstractTransaction> exact(AbstractTransaction::fromEnvelopeXdr(
        QString::fromLatin1(envelope.toBase64(XDR_BASE64ENCODING)), network));

    QString out;
    if (xdrEnvelope.type == stellar::EnvelopeType::ENVELOPE_TYPE_TX_FEE_BUMP) {
        auto* feeBump = dynamic_cast<FeeBumpTransaction*>(exact.get());
        if (!feeBump)
            throw std::runtime_error("Txrep emitter: unsupported transaction type");
        appendLine(out, "type", "ENVELOPE_TYPE_TX_FEE_BUMP");
        appendAccountLine(out, "feeBump.tx.feeSource", feeBump->getFeeAccount());
        appendLine(out, "feeBump.tx.fee", QString::number(feeBump->getFee()));
        // A fee bump wraps a V1 envelope, and nothing else (a V0 inner one
        // goes out as the V1 envelope it is).
        appendLine(out, "feeBump.tx.innerTx.type", "ENVELOPE_TYPE_TX");
        Transaction* inner = feeBump->getInnerTransaction();
        emitTransaction(out, "feeBump.tx.innerTx.tx", inner);
        emitSignatures(out, "feeBump.tx.innerTx.", inner->getSignatures());
        appendLine(out, "feeBump.tx.ext.v", "0");
        emitSignatures(out, "feeBump.", feeBump->getSignatures());
    } else {
        auto* plain = dynamic_cast<Transaction*>(exact.get());
        if (!plain)
            throw std::runtime_error("Txrep emitter: unsupported transaction type");
        appendLine(out, "type", xdrEnvelope.type == stellar::EnvelopeType::ENVELOPE_TYPE_TX_V0
                                    ? "ENVELOPE_TYPE_TX_V0" : "ENVELOPE_TYPE_TX");
        emitTransaction(out, "tx", plain);
        emitSignatures(out, QString(), plain->getSignatures());
    }

    // Read back, it must be that envelope and that hash. The two could only
    // part if the objects and their envelope did (a V0 transaction given V2
    // preconditions hashes them, and its envelope has no room for them):
    // then no document says both, and none is written.
    std::unique_ptr<AbstractTransaction> back(fromTxrepEnvelope(out, network));
    if (xdr::encodeOrThrow(back->toEnvelopeXdr(), "the transaction envelope") != envelope
        || back->hash() != transaction->hash()) {
        throw std::runtime_error("Txrep emitter: this transaction has no TxRep document that reads "
                                 "back as itself");
    }
    return out;
}

Transaction* Txrep::fromTxrep(const QString& txrep, Network* network)
{
    return static_cast<Transaction*>(read(txrep, network, false));
}

AbstractTransaction* Txrep::fromTxrepEnvelope(const QString& txrep, Network* network)
{
    return read(txrep, network, true);
}

AbstractTransaction* Txrep::read(const QString& txrep, Network* network, bool envelope)
{
    if (!network) throw std::runtime_error("network cannot be null");

    // What the builder cannot be told, put in place: Txrep is a friend of
    // TransactionBuilder and of Transaction.
    const auto build = [](ParsedTransaction& parsed) {
        if (parsed.untimed)
            parsed.builder->markTimeoutHandled();
        std::unique_ptr<Transaction> tx(parsed.builder->build());
        tx->m_fee = parsed.fee;
        // And the source exactly as written: the builder wrote the account's
        // G... key, which for an M... source is another transaction.
        tx->m_sourceAccount = parsed.sourceAccount;
        return tx;
    };

    TxrepMap kv = readLines(txrep);
    kv.setNativeName(stcNativeName(network));
    const QString type = need(kv, QStringLiteral("type"));
    if (type == QLatin1String("ENVELOPE_TYPE_TX") || type == QLatin1String("ENVELOPE_TYPE_TX_V0")) {
        const bool v0 = type == QLatin1String("ENVELOPE_TYPE_TX_V0");
        ParsedTransaction parsed = parseTransaction(kv, QStringLiteral("tx"), network, v0);
        // Read, and they must be what they say, whether or not they are
        // carried over: fromTxrep hands the transaction back to be signed.
        const QVector<stellar::DecoratedSignature> signatures = parseSignatures(kv, QString());
        finishReading(kv);

        std::unique_ptr<Transaction> tx = build(parsed);
        if (v0)
            tx->m_envelopeType = stellar::EnvelopeType::ENVELOPE_TYPE_TX_V0;
        checkSignatureNotes(kv, QString(), signatures, tx->hash());
        if (envelope)
            tx->m_signatures = signatures;
        return tx.release();
    }
    if (type != QLatin1String("ENVELOPE_TYPE_TX_FEE_BUMP"))
        throw std::runtime_error(("Txrep parser: unsupported envelope type: " + type).toStdString());
    // Its inner transaction is not the fee bump: signed in its place, it
    // would be another transaction than the one reviewed.
    if (!envelope)
        throw std::runtime_error("Txrep parser: fromTxrep returns a transaction, and this document "
                                 "is a fee bump: read it with fromTxrepEnvelope");

    // G... or M..., with py-stellar-sdk's comment after an M....
    const QString feeSource = needMuxedAccount(kv, QStringLiteral("feeBump.tx.feeSource"));
    // An int64 on the wire, and never below zero (FeeBumpTransaction refuses it).
    const qint64 fee = needInteger(kv, QStringLiteral("feeBump.tx.fee"), 0,
                                   std::numeric_limits<qint64>::max());
    // py writes this line and does not read it: a fee bump wraps a V1
    // envelope, and nothing else.
    if (need(kv, QStringLiteral("feeBump.tx.innerTx.type")) != QLatin1String("ENVELOPE_TYPE_TX"))
        throw std::runtime_error("txrep: feeBump.tx.innerTx.type must be ENVELOPE_TYPE_TX");
    ParsedTransaction parsedInner = parseTransaction(kv, QStringLiteral("feeBump.tx.innerTx.tx"),
                                                     network, false);
    const QVector<stellar::DecoratedSignature> innerSignatures =
        parseSignatures(kv, QStringLiteral("feeBump.tx.innerTx."));
    needInteger(kv, QStringLiteral("feeBump.tx.ext.v"), 0, 0);
    const QVector<stellar::DecoratedSignature> signatures = parseSignatures(kv, QStringLiteral("feeBump."));
    finishReading(kv);

    const std::unique_ptr<Transaction> inner = build(parsedInner);
    stellar::FeeBumpTransactionEnvelope feeBump;
    feeBump.tx.feeSource = StrKey::encodeToXDRMuxedAccount(feeSource);
    feeBump.tx.fee = fee;
    feeBump.tx.type = stellar::EnvelopeType::ENVELOPE_TYPE_TX;
    feeBump.tx.v1.tx = inner->toV1Xdr();
    for (const stellar::DecoratedSignature& signature : innerSignatures)
        feeBump.tx.v1.signatures.append(signature);
    for (const stellar::DecoratedSignature& signature : signatures)
        feeBump.signatures.append(signature);
    // Through the SDK's own reader of that envelope, which refuses what it
    // would not write back byte for byte.
    const QByteArray bytes = xdr::encodeOrThrow(stellar::TransactionEnvelope(feeBump),
                                                "the fee bump envelope");
    std::unique_ptr<AbstractTransaction> result(AbstractTransaction::fromEnvelopeXdr(
        QString::fromLatin1(bytes.toBase64(XDR_BASE64ENCODING)), network));
    checkSignatureNotes(kv, QStringLiteral("feeBump.tx.innerTx."), innerSignatures, inner->hash());
    checkSignatureNotes(kv, QStringLiteral("feeBump."), signatures, result->hash());
    return result.release();
}
QSTELLAR_END_NS
