#include "sep7.h"

#include <QHash>
#include <QStringList>
#include <QUrl>
#include <QUrlQuery>
#include <stdexcept>
#include <utility>

#include "keypair.h"

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
    const QByteArray sig = QByteArray::fromBase64(signatureB64.toLatin1());
    return signingKey->verify(payload, sig);
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
    if (msg.size() > 300) throw std::runtime_error("Sep7: message exceeds 300 characters");
    QString q;
    addParam(q, "destination", destination);
    if (!amount.isEmpty())      addParam(q, "amount", amount);
    if (!assetCode.isEmpty())  { addParam(q, "asset_code", assetCode);
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
    const QUrlQuery qq(query);

    if (!qq.hasQueryItem("destination"))
        throw std::runtime_error("Sep7: `destination` missing");
    Sep7Pay p(qq.queryItemValue("destination", QUrl::FullyDecoded));
    p.amount       = qq.queryItemValue("amount", QUrl::FullyDecoded);
    p.assetCode    = qq.queryItemValue("asset_code", QUrl::FullyDecoded);
    p.assetIssuer  = qq.queryItemValue("asset_issuer", QUrl::FullyDecoded);
    p.memo         = qq.queryItemValue("memo", QUrl::FullyDecoded);
    p.memoType     = qq.queryItemValue("memo_type", QUrl::FullyDecoded);
    p.callback     = parseCallback(qq.queryItemValue("callback", QUrl::FullyDecoded));
    p.msg          = qq.queryItemValue("msg", QUrl::FullyDecoded);
    p.networkPassphrase = qq.queryItemValue("network_passphrase", QUrl::FullyDecoded);
    p.originDomain = qq.queryItemValue("origin_domain", QUrl::FullyDecoded);
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
    if (!p.assetIssuer.isEmpty() && p.assetCode.isEmpty())
        throw std::runtime_error("Sep7: `asset_issuer` requires `asset_code`");
    return p;
}

void Sep7Pay::sign(KeyPair* signer)
{
    if (!signer) throw std::runtime_error("Sep7: null signer");
    const QByteArray payload = Sep7::signaturePayload(stripSignature(toUri()));
    signature = QString::fromLatin1(signer->sign(payload).toBase64());
}

bool Sep7Pay::verify(KeyPair* signingKey) const
{
    return verifyUri(toUri(), signature, signingKey);
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
    for (const Sep7Replacement& r : replace) {
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

QString Sep7Tx::toUri() const
{
    if (msg.size() > 300) throw std::runtime_error("Sep7: message exceeds 300 characters");
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
    const QUrlQuery qq(query);

    if (!qq.hasQueryItem("xdr")) throw std::runtime_error("Sep7: `xdr` missing");
    Sep7Tx t(qq.queryItemValue("xdr", QUrl::FullyDecoded));
    t.callback     = parseCallback(qq.queryItemValue("callback", QUrl::FullyDecoded));
    t.replace      = decodeReplace(qq.queryItemValue("replace", QUrl::FullyDecoded));
    t.pubkey       = qq.queryItemValue("pubkey", QUrl::FullyDecoded);
    t.msg          = qq.queryItemValue("msg", QUrl::FullyDecoded);
    t.networkPassphrase = qq.queryItemValue("network_passphrase", QUrl::FullyDecoded);
    t.originDomain = qq.queryItemValue("origin_domain", QUrl::FullyDecoded);
    t.signature    = qq.queryItemValue("signature", QUrl::FullyDecoded);
    return t;
}

void Sep7Tx::sign(KeyPair* signer)
{
    if (!signer) throw std::runtime_error("Sep7: null signer");
    const QByteArray payload = Sep7::signaturePayload(stripSignature(toUri()));
    signature = QString::fromLatin1(signer->sign(payload).toBase64());
}

bool Sep7Tx::verify(KeyPair* signingKey) const
{
    return verifyUri(toUri(), signature, signingKey);
}

QSTELLAR_END_NS
