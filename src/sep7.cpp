#include "sep7.h"

#include <QHash>
#include <QStringList>
#include <QUrl>
#include <QUrlQuery>
#include <algorithm>
#include <stdexcept>
#include <utility>

#include "keypair.h"
#include "abstracttransaction.h"
#include "network.h"
#include <memory>
#include "operation.h"
#include "strkey.h"

QSTELLAR_BEGIN_NS

const QString Sep7::SCHEME = QStringLiteral("web+stellar");
const QString Sep7::SIGNATURE_PREFIX = QStringLiteral("stellar.sep.7 - URI Scheme");

namespace {

// Percent-encode a query value byte-identically to py-stellar-sdk: it uses
// urlencode(..., quote_via=quote), and urlencode passes safe='' to quote, so
// EVERYTHING outside the RFC-3986 unreserved set is %XX-encoded — including
// '/', '+', '=' (so base64 xdr is fully encoded). This must match exactly
// because the SEP-7 signature is computed over the URI string.
QString enc(const QString& v)
{
    return QString::fromLatin1(QUrl::toPercentEncoding(v));
}

void addParam(QString& q, const QString& key, const QString& value)
{
    if (!q.isEmpty()) q.append('&');
    q.append(key).append('=').append(enc(value));
}

// Split `web+stellar:<path>?<query>` into (scheme, path, query).
void splitUri(const QString& uri, QString& scheme, QString& path, QString& query)
{
    const int colon = uri.indexOf(':');
    if (colon < 0) throw std::runtime_error("Sep7: not a URI");
    scheme = uri.left(colon);
    QString rest = uri.mid(colon + 1);
    const int q = rest.indexOf('?');
    if (q >= 0) { path = rest.left(q); query = rest.mid(q + 1); }
    else        { path = rest;         query.clear(); }
}

QString stripSignature(const QString& uri)
{
    const int idx = uri.indexOf(QStringLiteral("&signature="));
    return (idx >= 0) ? uri.left(idx) : uri;
}

QString parseCallback(const QString& cb)
{
    if (cb.isEmpty()) return QString();
    if (!cb.startsWith(QStringLiteral("url:")))
        throw std::runtime_error("Sep7: `callback` must start with `url:`");
    return cb.mid(4);
}

bool verifyUri(const QString& uri, const QString& signatureB64, KeyPair* signingKey)
{
    if (signatureB64.isEmpty() || !signingKey) return false;
    const QByteArray payload = Sep7::signaturePayload(stripSignature(uri));
    // Strict base64: the lenient decoder skipped what it could not read.
    const auto sig = QByteArray::fromBase64Encoding(signatureB64.toLatin1(),
                                                    QByteArray::AbortOnBase64DecodingErrors);
    return sig && signingKey->verify(payload, *sig);
}

/** Characters, as py counts a message (code points, not UTF-16 units). */
int codePoints(const QString& s)
{
    return s.toUcs4().size();
}

/**
 * The query of a received URI, checked: no parameter twice (which value a
 * wallet shows would be a guess), and `signature`, if present, last — the
 * signature covers the URI up to it, so anything after it is unsigned.
 */
QUrlQuery checkedQuery(const QString& query)
{
    const QUrlQuery qq(query);
    QStringList seen;
    const auto items = qq.queryItems(QUrl::FullyEncoded);
    for (int i = 0; i < items.size(); ++i) {
        const QString& key = items.at(i).first;
        if (seen.contains(key))
            throw std::runtime_error(("Sep7: `" + key + "` is given more than once").toStdString());
        seen.append(key);
        if (key == QLatin1String("signature") && i != items.size() - 1)
            throw std::runtime_error("Sep7: `signature` must be the last parameter");
    }
    return qq;
}

/**
 * The query with a raw '+' read as a space, as py's parse_qsl and every
 * URLSearchParams-based writer have it (QUrlQuery keeps a '+'): a request
 * written "msg=Pay+me&network_passphrase=Test+SDF+Network+%3B+..." says
 * "Pay me" and is for testnet. An encoded plus (%2B) stays a '+'. The
 * base64 values (xdr, signature, a hash or return memo) are read from the
 * query as it is: a space is never base64, so a raw '+' there can only be
 * a '+' (py reads a space and breaks them).
 */
QUrlQuery spacedQuery(const QString& query)
{
    return QUrlQuery(QString(query).replace(QLatin1Char('+'), QLatin1String("%20")));
}

/** A letter or a digit, in ASCII. */
bool isAsciiAlnum(uint c)
{
    return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

/** An asset code core can hold, as py's Asset checks it: 1 to 12 ASCII letters or digits. */
bool isAssetCode(const QString& code)
{
    return !code.isEmpty() && code.size() <= 12
           && std::all_of(code.cbegin(), code.cend(), [](QChar c) { return isAsciiAlnum(c.unicode()); });
}

/** An account id (G...) spelled exactly: kind, checksum and canonical form. */
bool isAccountId(const QString& key)
{
    try {
        return StrKey::decodeVersionByte(key) == StrKey::VersionByte::ACCOUNT_ID
               && StrKey::encodeStellarAccountId(StrKey::decodeStellarAccountId(key)) == key;
    } catch (const std::exception&) {
        return false;
    }
}

/** A replace field, reference or hint may not hold its own delimiters. */
void checkReplacePart(const QString& part, const char* what, const QString& forbidden)
{
    for (const QChar c : forbidden) {
        if (part.contains(c))
            throw std::runtime_error(QStringLiteral("Sep7: a `replace` %1 may not contain '%2'")
                                         .arg(QLatin1String(what)).arg(c).toStdString());
    }
}

} // namespace

QByteArray Sep7::signaturePayload(const QString& uriWithoutSignature)
{
    QByteArray data;
    data.append(QByteArray(35, '\0'));
    data.append(char(0x04));
    data.append(SIGNATURE_PREFIX.toUtf8());
    data.append(uriWithoutSignature.toUtf8());
    return data;
}

// ───────────────────────────── Sep7Pay ──────────────────────────────────────

Sep7Pay::Sep7Pay(QString dest) : destination(std::move(dest)) {}

QString Sep7Pay::toUri() const
{
    if (codePoints(msg) > 300) throw std::runtime_error("Sep7: message exceeds 300 characters");
    // What is asked for is what the URI says, or nothing is written. A memo
    // without its type and an issuer without its code used to be left out in
    // silence: a USDC deposit with its reference became a bare XLM payment.
    const StrKey::VersionByte kind = StrKey::decodeVersionByte(destination);
    if (kind != StrKey::VersionByte::ACCOUNT_ID && kind != StrKey::VersionByte::MUXED_ACCOUNT)
        throw std::runtime_error("Sep7: destination must be a G... or M... account");
    StrKey::decodeCheck(kind, destination.toLatin1());          // checksum, or it throws
    if (!amount.isEmpty() && Operation::toXdrAmountNonNegative(amount) <= 0)
        throw std::runtime_error("Sep7: amount must be more than zero");
    if (!assetIssuer.isEmpty() && assetCode.isEmpty())
        throw std::runtime_error("Sep7: asset_issuer without asset_code");
    // asset_code=XLM with no issuer is the native asset, as py's Asset reads
    // it; like py's to_uri(), nothing is written for it.
    const bool native = assetCode == QLatin1String("XLM") && assetIssuer.isEmpty();
    if (!assetCode.isEmpty() && assetIssuer.isEmpty() && !native)
        throw std::runtime_error("Sep7: asset_code without asset_issuer");
    // An asset core can hold, or none: a wallet was shown a request for
    // "USDC" issued by "garbage" (py's Asset refuses both).
    if (!assetIssuer.isEmpty()) {
        if (!isAssetCode(assetCode))
            throw std::runtime_error("Sep7: asset_code is 1 to 12 letters or digits");
        if (!isAccountId(assetIssuer))
            throw std::runtime_error("Sep7: asset_issuer must be an account id (G...)");
    }
    if (!memo.isEmpty() && memoType.isEmpty())
        throw std::runtime_error("Sep7: a memo needs its memo_type");
    if (!memoType.isEmpty()) {
        if (memoType == QLatin1String("MEMO_TEXT")) {
            if (memo.toUtf8().size() > 28)
                throw std::runtime_error("Sep7: a text memo is at most 28 bytes");
        } else if (memoType == QLatin1String("MEMO_ID")) {
            // Decimal digits only, as js Memo.id and Memo::parse("id", ...)
            // take it: toULongLong() skips spaces, and a raw '+' is read as
            // one, so "memo=+5" was the id " 5", kept with its space.
            bool ok = !memo.isEmpty()
                      && std::all_of(memo.cbegin(), memo.cend(),
                                     [](QChar c) { return c.unicode() >= '0' && c.unicode() <= '9'; });
            if (ok)
                memo.toULongLong(&ok);
            if (!ok)
                throw std::runtime_error("Sep7: a MEMO_ID is a whole number from 0 to 18446744073709551615");
        } else if (memoType == QLatin1String("MEMO_HASH") || memoType == QLatin1String("MEMO_RETURN")) {
            const auto bytes = QByteArray::fromBase64Encoding(memo.toLatin1(),
                                                              QByteArray::AbortOnBase64DecodingErrors);
            if (!bytes || (*bytes).size() != 32)
                throw std::runtime_error("Sep7: a MEMO_HASH or MEMO_RETURN is 32 bytes in base64");
        } else {
            throw std::runtime_error("Sep7: memo_type is MEMO_TEXT, MEMO_ID, MEMO_HASH or MEMO_RETURN");
        }
    }
    QString q;
    addParam(q, "destination", destination);
    if (!amount.isEmpty())      addParam(q, "amount", amount);
    if (!assetCode.isEmpty() && !native) { addParam(q, "asset_code", assetCode);
                                           addParam(q, "asset_issuer", assetIssuer); }
    if (!memoType.isEmpty())   { addParam(q, "memo", memo);
                                 addParam(q, "memo_type", memoType); }
    if (!callback.isEmpty())    addParam(q, "callback", "url:" + callback);
    if (!msg.isEmpty())         addParam(q, "msg", msg);
    if (!networkPassphrase.isEmpty()) addParam(q, "network_passphrase", networkPassphrase);
    if (!originDomain.isEmpty()) addParam(q, "origin_domain", originDomain);
    if (!signature.isEmpty())   addParam(q, "signature", signature);
    return Sep7::SCHEME + ":pay?" + q;
}

Sep7Pay Sep7Pay::fromUri(const QString& uri)
{
    QString scheme, path, query;
    splitUri(uri, scheme, path, query);
    if (scheme != Sep7::SCHEME) throw std::runtime_error("Sep7: wrong scheme");
    if (path != QStringLiteral("pay")) throw std::runtime_error("Sep7: path must be `pay`");
    const QUrlQuery qq = checkedQuery(query);
    const QUrlQuery text = spacedQuery(query);

    if (!qq.hasQueryItem("destination"))
        throw std::runtime_error("Sep7: `destination` missing");
    Sep7Pay p(text.queryItemValue("destination", QUrl::FullyDecoded));
    p.amount       = text.queryItemValue("amount", QUrl::FullyDecoded);
    p.assetCode    = text.queryItemValue("asset_code", QUrl::FullyDecoded);
    p.assetIssuer  = text.queryItemValue("asset_issuer", QUrl::FullyDecoded);
    p.memoType     = text.queryItemValue("memo_type", QUrl::FullyDecoded);
    const bool base64Memo = p.memoType == QLatin1String("MEMO_HASH") || p.memoType == QLatin1String("MEMO_RETURN");
    p.memo         = (base64Memo ? qq : text).queryItemValue("memo", QUrl::FullyDecoded);
    p.callback     = parseCallback(text.queryItemValue("callback", QUrl::FullyDecoded));
    p.msg          = text.queryItemValue("msg", QUrl::FullyDecoded);
    p.networkPassphrase = text.queryItemValue("network_passphrase", QUrl::FullyDecoded);
    p.originDomain = text.queryItemValue("origin_domain", QUrl::FullyDecoded);
    p.signature    = qq.queryItemValue("signature", QUrl::FullyDecoded);

    // Every field kept here must be one `toUri()` re-emits, because `verify()`
    // checks the signature against the canonical re-serialisation rather than
    // against the received string. `toUri()` writes `memo` only when
    // `memo_type` is set, and `asset_issuer` only when `asset_code` is set, so
    // an orphan of either would be retained on the object yet stay outside the
    // signed form — an attacker could append `&memo=...` to a legitimately
    // signed URI and `verify()` would still return true while the caller reads
    // the injected value. Reject the orphans instead. (py-stellar-sdk avoids
    // this by decoding the memo through its memo_type, so one never survives.)
    if (!p.memo.isEmpty() && p.memoType.isEmpty())
        throw std::runtime_error("Sep7: `memo` requires `memo_type`");
    // A memo_type with no memo at all is refused, as py refuses it: it was
    // read as an empty text memo. An empty one (`memo=`) is still an empty
    // text memo, which core takes and toUri() writes as py's to_uri() does
    // (py's reader drops the blank value and refuses its own URI).
    if (!p.memoType.isEmpty() && !qq.hasQueryItem("memo"))
        throw std::runtime_error("Sep7: `memo_type` requires `memo`");
    if (!p.assetIssuer.isEmpty() && p.assetCode.isEmpty())
        throw std::runtime_error("Sep7: `asset_issuer` requires `asset_code`");
    if (p.assetCode == QLatin1String("XLM") && p.assetIssuer.isEmpty())
        p.assetCode.clear();    // native, which assetCode says by being empty
    // Every rule toUri() applies (destination, amount, asset pair, memo, message
    // length): a request it would refuse to write is refused on the way in, so
    // a parsed request can always be written back and verify() never throws.
    p.toUri();
    p.m_received = uri;
    p.m_receivedFields = p.fields();
    return p;
}

QStringList Sep7Pay::fields() const
{
    return {destination, amount, assetCode, assetIssuer, memo, memoType, callback, msg,
            networkPassphrase, originDomain, signature};
}

void Sep7Pay::sign(KeyPair* signer)
{
    if (!signer) throw std::runtime_error("Sep7: null signer");
    const QByteArray payload = Sep7::signaturePayload(stripSignature(toUri()));
    signature = QString::fromLatin1(signer->sign(payload).toBase64());
}

bool Sep7Pay::verify(KeyPair* signingKey) const
{
    // The URI as received, which is what was signed (a signer may order or
    // encode its parameters otherwise); this object's own form once a field
    // was changed after reading it.
    if (!m_received.isEmpty() && fields() == m_receivedFields)
        return verifyUri(m_received, signature, signingKey);
    try {
        return verifyUri(toUri(), signature, signingKey);
    } catch (const std::exception&) {
        return false;           // a request toUri() cannot write is not one that verifies
    }
}

// ───────────────────────────── Sep7Tx ───────────────────────────────────────

Sep7Tx::Sep7Tx(QString envelopeXdrBase64) : xdr(std::move(envelopeXdrBase64)) {}

namespace {
QString encodeReplace(const QList<Sep7Replacement>& replace)
{
    if (replace.isEmpty()) return QString();
    QStringList refs;
    QStringList hints;
    QStringList seen;
    QHash<QString, QString> hintOf;
    for (const Sep7Replacement& r : replace) {
        // A delimiter inside a part made a link the reader splits otherwise,
        // or refuses: the builder must not write what cannot be read back.
        checkReplacePart(r.field, "field", QStringLiteral(",;:"));
        checkReplacePart(r.reference, "reference", QStringLiteral(",;:"));
        checkReplacePart(r.hint, "hint", QStringLiteral(",;"));
        if (r.field.isEmpty() || r.reference.isEmpty())
            throw std::runtime_error("Sep7: a `replace` entry needs a field and a reference");
        if (hintOf.contains(r.reference) && hintOf.value(r.reference) != r.hint)
            throw std::runtime_error("Sep7: one `replace` reference with two different hints");
        hintOf.insert(r.reference, r.hint);
        refs.append(r.field + ":" + r.reference);
        if (!seen.contains(r.reference)) {
            seen.append(r.reference);
            hints.append(r.reference + ":" + r.hint);
        }
    }
    return refs.join(",") + ";" + hints.join(",");
}

QList<Sep7Replacement> decodeReplace(const QString& raw)
{
    QList<Sep7Replacement> out;
    if (raw.isEmpty()) return out;
    const int semi = raw.indexOf(';');
    if (semi < 0) throw std::runtime_error("Sep7: invalid `replace`");
    const QString idsPart = raw.left(semi);
    const QString hintsPart = raw.mid(semi + 1);
    QHash<QString, QString> hintMap;
    const QStringList hintList = hintsPart.split(',', Qt::SkipEmptyParts);
    for (const QString& h : hintList) {
        const int c = h.indexOf(':');
        if (c < 0) throw std::runtime_error("Sep7: invalid `replace` hint");
        if (hintMap.contains(h.left(c)))
            throw std::runtime_error("Sep7: a `replace` reference has two hints");
        hintMap.insert(h.left(c), h.mid(c + 1));
    }
    const QStringList idList = idsPart.split(',', Qt::SkipEmptyParts);
    for (const QString& id : idList) {
        const int c = id.indexOf(':');
        if (c < 0) throw std::runtime_error("Sep7: invalid `replace` id");
        const QString field = id.left(c);
        const QString ref = id.mid(c + 1);
        if (!hintMap.contains(ref)) throw std::runtime_error("Sep7: `replace` missing hint");
        out.append(Sep7Replacement(field, ref, hintMap.value(ref)));
    }
    return out;
}
} // namespace

QString Sep7Tx::effectiveNetworkPassphrase() const
{
    return networkPassphrase.isEmpty() ? Network::PUBLIC_NETWORK_PASSPHRASE() : networkPassphrase;
}

AbstractTransaction* Sep7Tx::transaction(Network* network) const
{
    if (!network)
        throw std::runtime_error("Sep7: a network is needed to read the transaction");
    if (network->getNetworkPassphrase() != effectiveNetworkPassphrase())
        throw std::runtime_error("Sep7: the request is for another network than the one given");
    try {
        return AbstractTransaction::fromEnvelopeXdr(xdr, network);
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Sep7: `xdr` is not a transaction envelope: ") + e.what());
    }
}

QString Sep7Tx::toUri() const
{
    if (codePoints(msg) > 300) throw std::runtime_error("Sep7: message exceeds 300 characters");
    // The envelope decodes whole, as py decodes it when it reads a request:
    // it was carried as any text, so a damaged one went out, or came in,
    // as a request to sign. (Decoding needs no network; signing does.)
    if (xdr.isEmpty())
        throw std::runtime_error("Sep7: `xdr` missing");
    {
        Network anyNetwork(effectiveNetworkPassphrase());
        std::unique_ptr<AbstractTransaction> decoded(transaction(&anyNetwork));
    }
    QString q;
    addParam(q, "xdr", xdr);
    if (!callback.isEmpty()) addParam(q, "callback", "url:" + callback);
    const QString rep = encodeReplace(replace);
    if (!rep.isEmpty())      addParam(q, "replace", rep);
    if (!pubkey.isEmpty())   addParam(q, "pubkey", pubkey);
    if (!msg.isEmpty())      addParam(q, "msg", msg);
    if (!networkPassphrase.isEmpty()) addParam(q, "network_passphrase", networkPassphrase);
    if (!originDomain.isEmpty()) addParam(q, "origin_domain", originDomain);
    if (!signature.isEmpty()) addParam(q, "signature", signature);
    return Sep7::SCHEME + ":tx?" + q;
}

Sep7Tx Sep7Tx::fromUri(const QString& uri)
{
    QString scheme, path, query;
    splitUri(uri, scheme, path, query);
    if (scheme != Sep7::SCHEME) throw std::runtime_error("Sep7: wrong scheme");
    if (path != QStringLiteral("tx")) throw std::runtime_error("Sep7: path must be `tx`");
    const QUrlQuery qq = checkedQuery(query);
    const QUrlQuery text = spacedQuery(query);

    if (!qq.hasQueryItem("xdr")) throw std::runtime_error("Sep7: `xdr` missing");
    Sep7Tx t(qq.queryItemValue("xdr", QUrl::FullyDecoded));
    t.callback     = parseCallback(text.queryItemValue("callback", QUrl::FullyDecoded));
    t.replace      = decodeReplace(text.queryItemValue("replace", QUrl::FullyDecoded));
    t.pubkey       = text.queryItemValue("pubkey", QUrl::FullyDecoded);
    t.msg          = text.queryItemValue("msg", QUrl::FullyDecoded);
    t.networkPassphrase = text.queryItemValue("network_passphrase", QUrl::FullyDecoded);
    t.originDomain = text.queryItemValue("origin_domain", QUrl::FullyDecoded);
    t.signature    = qq.queryItemValue("signature", QUrl::FullyDecoded);
    t.toUri();                  // the rules toUri() applies, as for pay
    t.m_received = uri;
    t.m_receivedFields = t.fields();
    return t;
}

Sep7Tx Sep7Tx::fromUri(const QString& uri, const QString& expectedNetworkPassphrase)
{
    Sep7Tx t = fromUri(uri);
    // py's from_uri(uri, network_passphrase), case by case.
    if (t.networkPassphrase.isEmpty() && expectedNetworkPassphrase.isEmpty())
        throw std::runtime_error("Sep7: the URI names no network_passphrase and none is expected");
    if (!t.networkPassphrase.isEmpty() && !expectedNetworkPassphrase.isEmpty()
        && t.networkPassphrase != expectedNetworkPassphrase)
        throw std::runtime_error("Sep7: the network_passphrase in the URI does not match the expected one");
    if (t.networkPassphrase.isEmpty()) {
        t.networkPassphrase = expectedNetworkPassphrase;
        // Not a change to what was received: verify() checks the URI as it came.
        t.m_receivedFields = t.fields();
    }
    return t;
}

QStringList Sep7Tx::fields() const
{
    QStringList out{xdr, callback, pubkey, msg, networkPassphrase, originDomain, signature};
    for (const Sep7Replacement& r : replace)
        out << r.field << r.reference << r.hint;
    return out;
}

void Sep7Tx::sign(KeyPair* signer)
{
    if (!signer) throw std::runtime_error("Sep7: null signer");
    const QByteArray payload = Sep7::signaturePayload(stripSignature(toUri()));
    signature = QString::fromLatin1(signer->sign(payload).toBase64());
}

bool Sep7Tx::verify(KeyPair* signingKey) const
{
    if (!m_received.isEmpty() && fields() == m_receivedFields)
        return verifyUri(m_received, signature, signingKey);
    try {
        return verifyUri(toUri(), signature, signingKey);
    } catch (const std::exception&) {
        return false;
    }
}

// ───────────────────────────── origin ───────────────────────────────────────

QString Sep7::originDomain(const QString& uri)
{
    QString scheme, path, query;
    splitUri(uri, scheme, path, query);
    QString origin;
    if (path == QLatin1String("pay"))
        origin = Sep7Pay::fromUri(uri).originDomain;
    else if (path == QLatin1String("tx"))
        origin = Sep7Tx::fromUri(uri).originDomain;
    else
        throw std::runtime_error("Sep7: not a `pay` or `tx` request");
    // The ACE form, the one isFullyQualifiedDomainName() checks and the
    // toml is read from: example.com spelled with a Cyrillic a (U+0430)
    // looked like example.com; it is xn--exmple-4nf.com. What is not a
    // domain name comes back as it was, for isFullyQualifiedDomainName()
    // to refuse.
    return isFullyQualifiedDomainName(origin) ? QString::fromLatin1(QUrl::toAce(origin)) : origin;
}

bool Sep7::isFullyQualifiedDomainName(const QString& domain)
{
    // In ASCII only letters, digits, '-' and '.': a port, a path, a user, a
    // space or an IPv6 bracket is not part of a name. A non-ASCII label is an
    // internationalised one, checked in its ACE form.
    for (const QChar c : domain) {
        const uint u = c.unicode();
        if (u < 0x80 && !isAsciiAlnum(u) && u != '-' && u != '.')
            return false;
    }
    // Not left to toAce(), which need not keep a final dot.
    if (domain.endsWith(QLatin1Char('.')))
        return false;
    const QByteArray ace = QUrl::toAce(domain);
    if (ace.isEmpty() || ace.size() > 253)
        return false;
    const QList<QByteArray> labels = ace.split('.');
    if (labels.size() < 2)
        return false;
    for (const QByteArray& label : labels) {
        // An empty label too: a trailing dot, "a..example".
        if (label.isEmpty() || label.size() > 63 || label.startsWith('-') || label.endsWith('-'))
            return false;
        for (const char c : label) {
            if (!isAsciiAlnum(uchar(c)) && c != '-')
                return false;
        }
    }
    const QByteArray& top = labels.last();
    return !std::all_of(top.cbegin(), top.cend(), [](char c) { return c >= '0' && c <= '9'; });
}

QString Sep7::verifyOrigin(const QString& uri, const QVariantMap& originToml)
{
    QString scheme, path, query;
    splitUri(uri, scheme, path, query);
    if (path != QLatin1String("pay") && path != QLatin1String("tx"))
        throw std::runtime_error("Sep7: not a `pay` or `tx` request");
    const bool pay = path == QLatin1String("pay");
    const Sep7Pay payRequest = pay ? Sep7Pay::fromUri(uri) : Sep7Pay(QString());
    const Sep7Tx txRequest = pay ? Sep7Tx(QString()) : Sep7Tx::fromUri(uri);
    const QString origin = pay ? payRequest.originDomain : txRequest.originDomain;
    const QString signature = pay ? payRequest.signature : txRequest.signature;
    if (origin.isEmpty())
        throw std::runtime_error("Sep7: the request names no origin_domain");
    // SEP-7: a request whose origin_domain is not a fully qualified domain
    // name is not to be signed. A wallet that built the toml's URL itself
    // would have fetched "bank.example@evil.example" from evil.example.
    if (!isFullyQualifiedDomainName(origin))
        throw std::runtime_error("Sep7: the origin_domain is not a fully qualified domain name");
    if (signature.isEmpty())
        throw std::runtime_error("Sep7: the request names an origin_domain but is not signed");

    const QVariant value = originToml.value(QStringLiteral("URI_REQUEST_SIGNING_KEY"));
    const QString key = value.toString();
    if (value.userType() != QMetaType::QString || !isAccountId(key))
        throw std::runtime_error("Sep7: the origin's stellar.toml has no URI_REQUEST_SIGNING_KEY (an account id)");

    std::unique_ptr<KeyPair> signer(KeyPair::fromAccountId(key));
    const bool signedByOrigin = pay ? payRequest.verify(signer.get()) : txRequest.verify(signer.get());
    if (!signedByOrigin)
        throw std::runtime_error("Sep7: the request is not signed by its origin's URI_REQUEST_SIGNING_KEY");
    return key;
}

QSTELLAR_END_NS
