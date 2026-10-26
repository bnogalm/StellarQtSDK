#ifndef SEP7TEST_H
#define SEP7TEST_H

#include <QObject>
#include <QtTest>
#include <QScopedPointer>
#include <QUrl>
#include "testcollector.h"

#include "../src/sep7.h"
#include "../src/keypair.h"
#include "../src/network.h"
#include "../src/transaction.h"
#include <QVariantMap>
#include <memory>
#include <stdexcept>

/**
 * SEP-7 URI scheme — verified byte-for-byte against py-stellar-sdk 14.1.0
 * `to_uri()` / `sign()` output (the reference strings below are verbatim).
 */
class Sep7Test: public QObject
{
    Q_OBJECT

    // Keypair from secret SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK
    static const char* SECRET()  { return "SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"; }
    static const char* SIGNER()  { return "GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF"; }
    static const char* DEST()    { return "GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7"; }

private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    void testPayToUriMatchesReference()
    {
        Sep7Pay p(DEST());
        p.amount = "120.1234567";
        p.msg = "Pay me please";
        p.originDomain = "example.com";
        QCOMPARE(p.toUri(), QString(
            "web+stellar:pay?destination=GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7"
            "&amount=120.1234567&msg=Pay%20me%20please&origin_domain=example.com"));
    }

    void testPaySignatureMatchesReference()
    {
        Sep7Pay p(DEST());
        p.amount = "120.1234567";
        p.msg = "Pay me please";
        p.originDomain = "example.com";
        p.sign(KeyPair::fromSecretSeed(QString(SECRET())));
        QCOMPARE(p.signature, QString(
            "YusUj79K0WoGq30BfD9NO7a3i1z64JkQJI4L8Lil2g+df7+4vTY8D4GWyFKVkQkLkYoPkSnNO+aOYOF/6p6EDQ=="));
    }

    void testPayParseAndVerify()
    {
        const QString signed_ =
            "web+stellar:pay?destination=GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7"
            "&amount=120.1234567&msg=Pay%20me%20please&origin_domain=example.com"
            "&signature=YusUj79K0WoGq30BfD9NO7a3i1z64JkQJI4L8Lil2g%2Bdf7%2B4vTY8D4GWyFKVkQkLkYoPkSnNO"
            "%2BaOYOF%2F6p6EDQ%3D%3D";
        Sep7Pay p = Sep7Pay::fromUri(signed_);
        QCOMPARE(p.destination, QString(DEST()));
        QCOMPARE(p.amount, QString("120.1234567"));
        QCOMPARE(p.msg, QString("Pay me please"));            // %20 decoded back to space
        QCOMPARE(p.originDomain, QString("example.com"));
        QVERIFY(!p.signature.isEmpty());
        // Verify against the origin domain's signing key (public account only).
        QVERIFY(p.verify(KeyPair::fromAccountId(QString(SIGNER()))));
        // Tampered destination must fail.
        p.destination = "GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO";
        QVERIFY(!p.verify(KeyPair::fromAccountId(QString(SIGNER()))));
    }

    /** verify() checks the signature against the canonical re-serialisation,
     *  and toUri() emits `memo` only alongside `memo_type` (same for
     *  `asset_issuer` / `asset_code`). An orphan would therefore be retained on
     *  the object but excluded from the signed form — appending `&memo=` to a
     *  legitimately signed URI would leave verify() returning true while the
     *  caller reads attacker-controlled data. fromUri() must reject it. */
    void testPayRejectsOrphanMemoAndAssetIssuer()
    {
        const QString base =
            "web+stellar:pay?destination=GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7";

        bool threw = false;
        try { Sep7Pay::fromUri(base + "&memo=1234567"); }
        catch (const std::runtime_error&) { threw = true; }
        QVERIFY(threw);

        threw = false;
        try { Sep7Pay::fromUri(base + "&asset_issuer=" + QString(SIGNER())); }
        catch (const std::runtime_error&) { threw = true; }
        QVERIFY(threw);

        // The properly paired forms still parse.
        Sep7Pay ok = Sep7Pay::fromUri(base + "&memo=1234567&memo_type=MEMO_ID");
        QCOMPARE(ok.memo, QString("1234567"));
        QCOMPARE(ok.memoType, QString("MEMO_ID"));
    }

    /**
     * The signature is checked against the URI as received, which is what
     * SEP-7 signs: a signer that writes its parameters in another order than
     * this SDK did not verify. Nothing may follow `signature` (it would be
     * read without being signed), no parameter may come twice, and a request
     * toUri() would refuse is refused on the way in, so verify() answers and
     * never throws. The message limit counts characters (code points).
     */
    void testVerifyChecksTheReceivedUri()
    {
        QScopedPointer<KeyPair> signer(KeyPair::fromSecretSeed(QString(SECRET())));
        QScopedPointer<KeyPair> key(KeyPair::fromAccountId(QString(SIGNER())));
        const QString reordered =
            "web+stellar:pay?amount=120.1234567&destination=GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7"
            "&origin_domain=example.com";
        const QString sig = QString::fromLatin1(signer->sign(Sep7::signaturePayload(reordered)).toBase64());
        const QString signedUri = reordered + "&signature=" + QString::fromLatin1(QUrl::toPercentEncoding(sig));
        Sep7Pay p = Sep7Pay::fromUri(signedUri);
        QVERIFY(p.verify(key.data()));
        p.amount = "1";                                 // changed after reading: no longer verifies
        QVERIFY(!p.verify(key.data()));

        QVERIFY_EXCEPTION_THROWN(Sep7Pay::fromUri(signedUri + "&memo=1&memo_type=MEMO_ID"), std::runtime_error);
        const QString base = "web+stellar:pay?destination=GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7";
        QVERIFY_EXCEPTION_THROWN(Sep7Pay::fromUri(base + "&amount=1&amount=2"), std::runtime_error);
        QVERIFY_EXCEPTION_THROWN(Sep7Pay::fromUri(base + "&amount=0"), std::runtime_error);
        QVERIFY_EXCEPTION_THROWN(Sep7Pay::fromUri(base + "&msg=" + QString(301, QLatin1Char('a'))), std::runtime_error);

        Sep7Pay bad(QStringLiteral("not an account"));
        bad.signature = sig;
        QVERIFY(!bad.verify(key.data()));               // false, not an exception

        Sep7Pay emoji{QString(DEST())};
        QString smiles;
        for (int i = 0; i < 300; ++i)
            smiles += QString::fromUcs4(U"\U0001F600", 1);
        emoji.msg = smiles;                             // 300 characters, 600 UTF-16 units
        QVERIFY(!emoji.toUri().isEmpty());
        emoji.msg += QStringLiteral("x");
        QVERIFY_EXCEPTION_THROWN(emoji.toUri(), std::runtime_error);

        Sep7Tx t(ENVELOPE());
        t.replace.append(Sep7Replacement("sourceAccount", "X", "one, two"));
        QVERIFY_EXCEPTION_THROWN(t.toUri(), std::runtime_error);
        t.replace = {Sep7Replacement("source:Account", "X", "hint")};
        QVERIFY_EXCEPTION_THROWN(t.toUri(), std::runtime_error);
        t.replace = {Sep7Replacement("sourceAccount", "X", "a"), Sep7Replacement("seqNum", "X", "b")};
        QVERIFY_EXCEPTION_THROWN(t.toUri(), std::runtime_error);
    }

    /**
     * A real envelope (a bump sequence by GAB2CB...QZ6Q, built with
     * py-stellar-sdk 14.1.0), whose base64 has '+', '/' and '='. The URI is
     * byte for byte the one py's TransactionStellarUri.to_uri() gives.
     */
    static const char* ENVELOPE()
    {
        return "AAAAAgAAAAADoQe/884Qvh1w3RjnS8CZZ+TWMJulDV8d3IZkElUxuAAAAGQAAAAAAAAAAgAAAAEAAAAAAAAAAAAAAABqw3FSAAAAAAAAAAEAAAAAAAAACwAAAAAAAAALAAAAAAAAAAA=";
    }

    void testTxEncodingAndRoundTrip()
    {
        Sep7Tx t(ENVELOPE());
        t.msg = "Sign me";
        t.networkPassphrase = "Test SDF Network ; September 2015";
        t.originDomain = "example.com";
        const QString uri = t.toUri();
        QCOMPARE(uri, QString(
            "web+stellar:tx?xdr=AAAAAgAAAAADoQe%2F884Qvh1w3RjnS8CZZ%2BTWMJulDV8d3IZkElUxuAAAAGQAAAAAAAAAAgAAAAEAAAAAAAAAAAAAAABqw3FSAAAAAAAAAAEAAAAAAAAACwAAAAAAAAALAAAAAAAAAAA%3D"
            "&msg=Sign%20me&network_passphrase=Test%20SDF%20Network%20%3B%20September%202015&origin_domain=example.com"));

        Sep7Tx p = Sep7Tx::fromUri(uri);
        QCOMPARE(p.xdr, QString(ENVELOPE()));   // %2B %2F %3D decoded back
        QCOMPARE(p.msg, QString("Sign me"));
        QCOMPARE(p.networkPassphrase, QString("Test SDF Network ; September 2015"));
        QCOMPARE(p.originDomain, QString("example.com"));
    }

    /**
     * The xdr is a transaction envelope, decoded as py decodes it when it
     * reads a request: any text went out and came in as a request to sign.
     * A network given to fromUri() or transaction() has to be the request's
     * (py: "does not match the network_passphrase in the uri"); none named
     * means the public network, except to fromUri(uri, expected), which
     * reads it for the expected one as py does.
     */
    void testTxXdrIsAnEnvelopeForTheRequestsNetwork()
    {
        for (const char* junk : {"AAAA+/==", "AAAAAA==", "not base64", ""}) {
            Sep7Tx t{QString(junk)};
            QVERIFY_EXCEPTION_THROWN(t.toUri(), std::runtime_error);
            QVERIFY_EXCEPTION_THROWN(Sep7Tx::fromUri(QStringLiteral("web+stellar:tx?xdr=")
                                                     + QString::fromLatin1(QUrl::toPercentEncoding(junk))),
                                     std::runtime_error);
        }
        // Truncated by one byte: refused, not read with zeros.
        const QByteArray whole = QByteArray::fromBase64(ENVELOPE());
        QVERIFY_EXCEPTION_THROWN(Sep7Tx(QString::fromLatin1(whole.left(whole.size() - 4).toBase64())).toUri(),
                                 std::runtime_error);

        Sep7Tx t(ENVELOPE());
        t.networkPassphrase = Network::TESTNET_NETWORK_PASSPHRASE();
        const QString uri = t.toUri();
        QCOMPARE(Sep7Tx::fromUri(uri, Network::TESTNET_NETWORK_PASSPHRASE()).xdr, QString(ENVELOPE()));
        QVERIFY_EXCEPTION_THROWN(Sep7Tx::fromUri(uri, Network::PUBLIC_NETWORK_PASSPHRASE()), std::runtime_error);

        Sep7Tx unnamed(ENVELOPE());
        QCOMPARE(unnamed.effectiveNetworkPassphrase(), Network::PUBLIC_NETWORK_PASSPHRASE());
        QCOMPARE(Sep7Tx::fromUri(unnamed.toUri(), Network::PUBLIC_NETWORK_PASSPHRASE()).xdr, QString(ENVELOPE()));
        QCOMPARE(Sep7Tx::fromUri(unnamed.toUri(), Network::TESTNET_NETWORK_PASSPHRASE()).networkPassphrase,
                 Network::TESTNET_NETWORK_PASSPHRASE());

        QVERIFY_EXCEPTION_THROWN(delete t.transaction(Network::publicNetwork()), std::runtime_error);
        std::unique_ptr<AbstractTransaction> tx(t.transaction(Network::testnetNetwork()));
        auto* plain = dynamic_cast<Transaction*>(tx.get());
        QVERIFY(plain);
        QCOMPARE(plain->getSourceAccount(), QStringLiteral("GAB2CB576PHBBPQ5ODORRZ2LYCMWPZGWGCN2KDK7DXOIMZASKUY3QZ6Q"));
        QCOMPARE(plain->getSequenceNumber(), qint64(2));
        QCOMPARE(tx->toEnvelopeXdrBase64(), QString(ENVELOPE()));
    }

    /**
     * SEP-7's origin check against the origin_domain's stellar.toml: signed
     * by its URI_REQUEST_SIGNING_KEY, or refused saying why (no origin, not
     * signed, no key in the toml, another key).
     */
    void testTheOriginIsCheckedAgainstItsToml()
    {
        std::unique_ptr<KeyPair> origin(KeyPair::random());
        std::unique_ptr<KeyPair> other(KeyPair::random());
        QVariantMap toml;
        toml.insert(QStringLiteral("URI_REQUEST_SIGNING_KEY"), origin->getAccountId());

        Sep7Pay pay{QString(DEST())};
        pay.amount = QStringLiteral("10");
        pay.originDomain = QStringLiteral("example.com");
        pay.sign(origin.get());
        const QString payUri = pay.toUri();
        QCOMPARE(Sep7::originDomain(payUri), QStringLiteral("example.com"));
        QCOMPARE(Sep7::verifyOrigin(payUri, toml), origin->getAccountId());

        Sep7Tx tx(ENVELOPE());
        tx.originDomain = QStringLiteral("example.com");
        tx.sign(origin.get());
        QCOMPARE(Sep7::verifyOrigin(tx.toUri(), toml), origin->getAccountId());

        auto refusal = [](const QString& uri, const QVariantMap& t) {
            try {
                Sep7::verifyOrigin(uri, t);
            } catch (const std::exception& e) {
                return QString::fromUtf8(e.what());
            }
            return QString();
        };
        QVariantMap otherToml;
        otherToml.insert(QStringLiteral("URI_REQUEST_SIGNING_KEY"), other->getAccountId());
        QVERIFY(refusal(payUri, otherToml).contains(QStringLiteral("not signed by its origin")));
        QVERIFY(refusal(payUri, QVariantMap()).contains(QStringLiteral("URI_REQUEST_SIGNING_KEY")));
        QVariantMap seedToml;
        seedToml.insert(QStringLiteral("URI_REQUEST_SIGNING_KEY"), origin->getSecretSeed());
        QVERIFY(refusal(payUri, seedToml).contains(QStringLiteral("URI_REQUEST_SIGNING_KEY")));

        Sep7Pay unsigned_{QString(DEST())};
        unsigned_.originDomain = QStringLiteral("example.com");
        QVERIFY(refusal(unsigned_.toUri(), toml).contains(QStringLiteral("not signed")));
        Sep7Pay noOrigin{QString(DEST())};
        noOrigin.sign(origin.get());
        QVERIFY(refusal(noOrigin.toUri(), toml).contains(QStringLiteral("no origin_domain")));
        QCOMPARE(Sep7::originDomain(noOrigin.toUri()), QString());
        QVERIFY_EXCEPTION_THROWN(Sep7::originDomain(QStringLiteral("web+stellar:other?x=1")), std::runtime_error);
    }

    /**
     * A raw '+' in a value is a space, as py's parse_qsl and URLSearchParams
     * read it (QUrlQuery kept the '+'), and %2B is a '+': a testnet request
     * written with '+' for its spaces is for testnet. The base64 values (xdr,
     * a hash memo) keep a raw '+', since a space is never base64; py breaks
     * them. toUri() still writes a space as %20, as py's to_uri().
     */
    void testAPlusInAValueIsASpace()
    {
        const QString base = "web+stellar:pay?destination=GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7";
        const Sep7Pay p = Sep7Pay::fromUri(base + "&memo=a+b&memo_type=MEMO_TEXT&msg=Pay+me+1%2B1"
                                                  "&network_passphrase=Test+SDF+Network+%3B+September+2015");
        QCOMPARE(p.memo, QStringLiteral("a b"));
        QCOMPARE(p.msg, QStringLiteral("Pay me 1+1"));
        QCOMPARE(p.networkPassphrase, Network::TESTNET_NETWORK_PASSPHRASE());
        // py 14.1.0's to_uri() of the same request.
        QCOMPARE(p.toUri(), base + "&memo=a%20b&memo_type=MEMO_TEXT&msg=Pay%20me%201%2B1"
                                   "&network_passphrase=Test%20SDF%20Network%20%3B%20September%202015");

        // "+/v7+/v7...+/s=": 32 bytes, read whole.
        const QString hash = QString::fromLatin1(QByteArray(32, char(0xfb)).toBase64());
        const Sep7Pay hashed = Sep7Pay::fromUri(base + "&memo=" + QString(hash).replace(QLatin1Char('='), QStringLiteral("%3D"))
                                                + "&memo_type=MEMO_HASH");
        QCOMPARE(hashed.memo, hash);

        QString raw = QString(ENVELOPE()).replace(QLatin1Char('/'), QStringLiteral("%2F"))
                                            .replace(QLatin1Char('='), QStringLiteral("%3D"));
        QVERIFY(raw.contains(QLatin1Char('+')));
        const Sep7Tx t = Sep7Tx::fromUri("web+stellar:tx?xdr=" + raw + "&msg=Sign+me"
                                         "&replace=sourceAccount%3AX%3BX%3Aaccount+to+use");
        QCOMPARE(t.xdr, QString(ENVELOPE()));
        QCOMPARE(t.msg, QStringLiteral("Sign me"));
        QCOMPARE(t.replace.size(), 1);
        QCOMPARE(t.replace.at(0).hint, QStringLiteral("account to use"));
    }

    /**
     * A MEMO_ID is decimal digits, as js 17.1.0's Memo.id and
     * Memo::parse("id", ...) take it. A raw '+' is a space and
     * toULongLong() skips spaces, so "memo=+5" was the id " 5", kept with
     * its space and written back as memo=%205. py 14.1.0 reads "+5",
     * "%205", "5+", "%2B5", "-0" and "1_0" as 5, 5, 5, 5, 0 and 10 (its
     * int()); js Memo.id refuses " 5", "+5", "-0" and "1_0", and so does
     * this. "05" is digits, which js takes too, and is kept as it came.
     */
    void testAMemoIdIsDigitsOnly()
    {
        const QString base = "web+stellar:pay?destination=GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7";
        for (const char* memo : {"+5", "%205", "5+", "%2B5", "-0", "%095", "1_0", "%D9%A5", "+",
                                 "18446744073709551616"}) {
            try {
                Sep7Pay::fromUri(base + "&memo=" + QLatin1String(memo) + "&memo_type=MEMO_ID");
                QFAIL(memo);
            } catch (const std::exception&) {
            }
        }
        QCOMPARE(Sep7Pay::fromUri(base + "&memo=5&memo_type=MEMO_ID").memo, QStringLiteral("5"));
        const Sep7Pay padded = Sep7Pay::fromUri(base + "&memo=05&memo_type=MEMO_ID");
        QCOMPARE(padded.memo, QStringLiteral("05"));
        QCOMPARE(padded.toUri(), base + "&memo=05&memo_type=MEMO_ID");

        Sep7Pay built{QString(DEST())};
        built.memoType = QStringLiteral("MEMO_ID");
        built.memo = QStringLiteral(" 5");
        QVERIFY_EXCEPTION_THROWN(built.toUri(), std::runtime_error);
        built.memo = QStringLiteral("5");
        QCOMPARE(built.toUri(), base + "&memo=5&memo_type=MEMO_ID");
    }

    /**
     * asset_code=XLM with no issuer is the native asset, as py's Asset reads
     * it: fromUri() leaves assetCode empty, and toUri() writes no asset for
     * it, as py's to_uri(). With an issuer XLM is a credit asset like any
     * other; another code, or "xlm", still needs its issuer (py agrees).
     */
    void testAssetCodeXlmWithoutIssuerIsNative()
    {
        const QString base = "web+stellar:pay?destination=GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7";
        const Sep7Pay p = Sep7Pay::fromUri(base + "&amount=1&asset_code=XLM");
        QVERIFY(p.assetCode.isEmpty());
        QCOMPARE(p.toUri(), base + "&amount=1");
        Sep7Pay built{QString(DEST())};
        built.assetCode = QStringLiteral("XLM");
        QCOMPARE(built.toUri(), base);

        const Sep7Pay credit = Sep7Pay::fromUri(base + "&asset_code=XLM&asset_issuer=" + QString(SIGNER()));
        QCOMPARE(credit.assetCode, QStringLiteral("XLM"));
        QCOMPARE(credit.assetIssuer, QString(SIGNER()));
        QVERIFY_EXCEPTION_THROWN(Sep7Pay::fromUri(base + "&asset_code=xlm"), std::runtime_error);
        QVERIFY_EXCEPTION_THROWN(Sep7Pay::fromUri(base + "&asset_code=USDC"), std::runtime_error);
    }

    /**
     * The asset of a pay request is one core can hold, as py's Asset checks
     * it: asset_code is 1 to 12 ASCII letters or digits, and asset_issuer an
     * account id (G..., checksum and spelling; not an M..., not a seed), on
     * the way in and on the way out.
     */
    void testPayAssetIsChecked()
    {
        const QString base = "web+stellar:pay?destination=GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7";
        const QString issuer = QString(SIGNER());
        QString damaged = issuer;
        damaged[20] = damaged[20] == QLatin1Char('A') ? QLatin1Char('B') : QLatin1Char('A');
        QStringList bad;
        bad << "&asset_code=USDC&asset_issuer=garbage"
            << "&asset_code=USDC&asset_issuer=" + damaged
            << "&asset_code=USDC&asset_issuer=" + issuer.toLower()
            << "&asset_code=USDC&asset_issuer=" + QString(SECRET())
            << "&asset_code=USDC&asset_issuer=MC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXYAAAAAAAAAAAAGGVG"
            << "&asset_code=ABCDEFGHIJKLM&asset_issuer=" + issuer
            << "&asset_code=US-D&asset_issuer=" + issuer
            << "&asset_code=%C3%A9UR&asset_issuer=" + issuer;
        for (const QString& query : bad)
            QVERIFY_EXCEPTION_THROWN(Sep7Pay::fromUri(base + query), std::runtime_error);

        QCOMPARE(Sep7Pay::fromUri(base + "&asset_code=ABCDEFGHIJKL&asset_issuer=" + issuer).assetCode,
                 QStringLiteral("ABCDEFGHIJKL"));
        QCOMPARE(Sep7Pay::fromUri(base + "&asset_code=a1&asset_issuer=" + issuer).assetCode, QStringLiteral("a1"));
        Sep7Pay built{QString(DEST())};
        built.assetCode = QStringLiteral("USDC");
        built.assetIssuer = QStringLiteral("garbage");
        QVERIFY_EXCEPTION_THROWN(built.toUri(), std::runtime_error);
    }

    /**
     * A memo_type with no memo is refused, as py refuses it (it was read as
     * an empty text memo). `memo=&memo_type=MEMO_TEXT` is an empty text memo,
     * which core takes: toUri() writes it as py's to_uri(TextMemo("")) does,
     * and fromUri() reads it back, where py's parse_qsl drops the blank value.
     */
    void testMemoTypeNeedsAMemo()
    {
        const QString base = "web+stellar:pay?destination=GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7";
        QVERIFY_EXCEPTION_THROWN(Sep7Pay::fromUri(base + "&memo_type=MEMO_TEXT"), std::runtime_error);
        QVERIFY_EXCEPTION_THROWN(Sep7Pay::fromUri(base + "&memo_type=MEMO_ID"), std::runtime_error);

        Sep7Pay built{QString(DEST())};
        built.memoType = QStringLiteral("MEMO_TEXT");
        QCOMPARE(built.toUri(), base + "&memo=&memo_type=MEMO_TEXT");
        const Sep7Pay empty = Sep7Pay::fromUri(built.toUri());
        QCOMPARE(empty.memoType, QStringLiteral("MEMO_TEXT"));
        QVERIFY(empty.memo.isEmpty());
    }

    /**
     * fromUri(uri, expected) reads the network as py 14.1.0's from_uri(uri,
     * network_passphrase): a URI that names none is for the expected one,
     * which networkPassphrase then holds and toUri() writes (py's to_uri()
     * of the same request, below); one that names another is refused; with
     * no expected network the URI has to name one. A signed request still
     * verifies, against the URI received. fromUri(uri) reads an absent
     * network as SEP-7 does, the public one.
     */
    void testTxExpectedNetworkIsReadAsPy()
    {
        const QString testnet = Network::TESTNET_NETWORK_PASSPHRASE();
        std::unique_ptr<KeyPair> origin(KeyPair::random());
        Sep7Tx unnamed(ENVELOPE());
        unnamed.originDomain = QStringLiteral("example.com");
        unnamed.sign(origin.get());
        const Sep7Tx read = Sep7Tx::fromUri(unnamed.toUri(), testnet);
        QCOMPARE(read.networkPassphrase, testnet);
        QCOMPARE(read.effectiveNetworkPassphrase(), testnet);
        QVERIFY(read.verify(origin.get()));
        std::unique_ptr<AbstractTransaction> tx(read.transaction(Network::testnetNetwork()));
        QCOMPARE(tx->toEnvelopeXdrBase64(), QString(ENVELOPE()));

        const QString plain = Sep7Tx(ENVELOPE()).toUri();
        QCOMPARE(Sep7Tx::fromUri(plain, testnet).toUri(), QString(
            "web+stellar:tx?xdr=AAAAAgAAAAADoQe%2F884Qvh1w3RjnS8CZZ%2BTWMJulDV8d3IZkElUxuAAAAGQAAAAAAAAAAgAAAAEAAAAAAAAAAAAAAABqw3FSAAAAAAAAAAEAAAAAAAAACwAAAAAAAAALAAAAAAAAAAA%3D"
            "&network_passphrase=Test%20SDF%20Network%20%3B%20September%202015"));
        QVERIFY_EXCEPTION_THROWN(Sep7Tx::fromUri(plain, QString()), std::runtime_error);
        QCOMPARE(Sep7Tx::fromUri(plain).effectiveNetworkPassphrase(), Network::PUBLIC_NETWORK_PASSPHRASE());

        Sep7Tx named(ENVELOPE());
        named.networkPassphrase = testnet;
        QCOMPARE(Sep7Tx::fromUri(named.toUri(), QString()).networkPassphrase, testnet);
        QVERIFY_EXCEPTION_THROWN(Sep7Tx::fromUri(named.toUri(), Network::PUBLIC_NETWORK_PASSPHRASE()),
                                 std::runtime_error);
    }

    /**
     * SEP-7's step 3: a request whose origin_domain is not a fully qualified
     * domain name is not signed, whatever its toml says. verifyOrigin()
     * refuses an IP address, a single label, a port, a path, a user, a
     * trailing dot; an internationalised name is taken in its ACE form.
     */
    void testTheOriginIsAFullyQualifiedDomainName()
    {
        QStringList good;
        good << "example.com" << "pay.example.co.uk" << "a-1.example" << "xn--bcher-kva.de"
             << QString(63, QLatin1Char('a')) + ".example"
             << QStringLiteral("b") + QChar(ushort(0x00FC)) + QStringLiteral("cher.de");
        for (const QString& domain : good)
            QVERIFY2(Sep7::isFullyQualifiedDomainName(domain), qPrintable(domain));
        QStringList bad;
        bad << QString() << "localhost" << "1.2.3.4" << "10.0.0.1" << "example.123" << "example.com."
            << ".example.com" << "a..example" << "-a.example" << "a-.example" << "example.com:443"
            << "example.com/x" << "bank.example@evil.example" << "a b.example" << "[::1]"
            << QString(64, QLatin1Char('a')) + ".example";
        for (const QString& domain : bad)
            QVERIFY2(!Sep7::isFullyQualifiedDomainName(domain), qPrintable(domain));

        std::unique_ptr<KeyPair> origin(KeyPair::random());
        QVariantMap toml;
        toml.insert(QStringLiteral("URI_REQUEST_SIGNING_KEY"), origin->getAccountId());
        for (const char* domain : {"1.2.3.4", "localhost", "bank.example@evil.example"}) {
            Sep7Pay pay{QString(DEST())};
            pay.originDomain = QString::fromLatin1(domain);
            pay.sign(origin.get());
            try {
                Sep7::verifyOrigin(pay.toUri(), toml);
                QFAIL(domain);
            } catch (const std::exception& e) {
                QVERIFY2(QString::fromUtf8(e.what()).contains(QStringLiteral("fully qualified")), e.what());
            }
        }
    }

    /**
     * Sep7::originDomain() hands back an internationalised origin_domain in
     * its ACE form, the one verifyOrigin() checks and the toml is read
     * from: example.com with a Cyrillic a (U+0430) is xn--exmple-4nf.com
     * (WHATWG URL in node and py's idna codec agree), which does not pass
     * for example.com. The parsed field keeps what the URI says, as py; an
     * ASCII name and what is not a domain name come back as they are.
     */
    void testTheOriginIsHandedBackInItsAceForm()
    {
        std::unique_ptr<KeyPair> origin(KeyPair::random());
        QVariantMap toml;
        toml.insert(QStringLiteral("URI_REQUEST_SIGNING_KEY"), origin->getAccountId());
        const QString homograph = QStringLiteral("ex") + QChar(ushort(0x0430)) + QStringLiteral("mple.com");
        Sep7Pay pay{QString(DEST())};
        pay.originDomain = homograph;
        pay.sign(origin.get());
        const QString payUri = pay.toUri();
        QCOMPARE(Sep7Pay::fromUri(payUri).originDomain, homograph);
        QCOMPARE(Sep7::originDomain(payUri), QStringLiteral("xn--exmple-4nf.com"));
        QCOMPARE(Sep7::verifyOrigin(payUri, toml), origin->getAccountId());

        Sep7Tx tx(ENVELOPE());
        tx.originDomain = QStringLiteral("b") + QChar(ushort(0x00FC)) + QStringLiteral("cher.de");
        QCOMPARE(Sep7::originDomain(tx.toUri()), QStringLiteral("xn--bcher-kva.de"));
        tx.originDomain = QStringLiteral("pay.example.com");
        QCOMPARE(Sep7::originDomain(tx.toUri()), QStringLiteral("pay.example.com"));
        tx.originDomain = QStringLiteral("localhost");
        QCOMPARE(Sep7::originDomain(tx.toUri()), QStringLiteral("localhost"));
    }

    /**
     * A pay request with every parameter, once per memo type, against
     * py-stellar-sdk 14.1.0 (js-stellar-sdk 17.1.0 has no SEP-7):
     * PayStellarUri(...).to_uri(), sign(SECRET) and the signed to_uri() are
     * the strings below, byte for byte. The message has spaces, '+', '=',
     * '&', '?', a euro sign, an accent and an emoji; the callback has its
     * own query. fromUri() reads py's signed URI back, field for field, it
     * verifies against SIGNER, and toUri() writes it again unchanged.
     */
    void testPayEveryParameterAndMemoTypeMatchesPy()
    {
        QScopedPointer<KeyPair> signer(KeyPair::fromSecretSeed(QString(SECRET())));
        QScopedPointer<KeyPair> key(KeyPair::fromAccountId(QString(SIGNER())));
        const QString msg = QString::fromUtf8("Pay 1+1 = 2 \xE2\x82\xAC caf\xC3\xA9 \xF0\x9F\x98\x80 & more?");
        const QString callback = QStringLiteral("https://example.com/cb?a=1&b=two words");
        const QString head = QStringLiteral(
            "web+stellar:pay?destination=GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7"
            "&amount=0.0000001&asset_code=USDC&asset_issuer=GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF");
        const QString tail = QStringLiteral(
            "&callback=url%3Ahttps%3A%2F%2Fexample.com%2Fcb%3Fa%3D1%26b%3Dtwo%20words"
            "&msg=Pay%201%2B1%20%3D%202%20%E2%82%AC%20caf%C3%A9%20%F0%9F%98%80%20%26%20more%3F"
            "&network_passphrase=Test%20SDF%20Network%20%3B%20September%202015&origin_domain=example.com");
        struct Case { QString memo; const char* type; const char* encoded; const char* signature; const char* encodedSignature; };
        const QList<Case> cases = {
            {QString::fromUtf8("caf\xC3\xA9 + tip & co"), "MEMO_TEXT", "caf%C3%A9%20%2B%20tip%20%26%20co",
             "GcF71o4c6odON28UXHkSkRuIFkgPicDaEiXRQXsPhdLC2uwHtU4/Pj6pE8i9sf27l+0nyjNEVVehCAB4FFwqAg==",
             "GcF71o4c6odON28UXHkSkRuIFkgPicDaEiXRQXsPhdLC2uwHtU4%2FPj6pE8i9sf27l%2B0nyjNEVVehCAB4FFwqAg%3D%3D"},
            {QStringLiteral("18446744073709551615"), "MEMO_ID", "18446744073709551615",
             "S/zpq5YzIOhPXnmabIldpDNxPNORc1W8RCnlQSPHmBAGJBm2gMwWJAlLxqJezXD4IVAa4xEldaX+2zmIEMiHAA==",
             "S%2Fzpq5YzIOhPXnmabIldpDNxPNORc1W8RCnlQSPHmBAGJBm2gMwWJAlLxqJezXD4IVAa4xEldaX%2B2zmIEMiHAA%3D%3D"},
            {QStringLiteral("4OHi4+Tl5ufo6err7O3u7/Dx8vP09fb3+Pn6+/z9/v8="), "MEMO_HASH",
             "4OHi4%2BTl5ufo6err7O3u7%2FDx8vP09fb3%2BPn6%2B%2Fz9%2Fv8%3D",
             "PPAVAhdz5skn9MNQjrJijscMQS9eg8FytwQFnvsJdnTii/RzA0tWy5BMYg4CyGIN+BBFqy+cVTuha7Ge01lYDg==",
             "PPAVAhdz5skn9MNQjrJijscMQS9eg8FytwQFnvsJdnTii%2FRzA0tWy5BMYg4CyGIN%2BBBFqy%2BcVTuha7Ge01lYDg%3D%3D"},
            {QStringLiteral("+/v7+/v7+/v7+/v7+/v7+/v7+/v7+/v7+/v7+/v7+/s="), "MEMO_RETURN",
             "%2B%2Fv7%2B%2Fv7%2B%2Fv7%2B%2Fv7%2B%2Fv7%2B%2Fv7%2B%2Fv7%2B%2Fv7%2B%2Fv7%2B%2Fv7%2B%2Fs%3D",
             "RA4zT6JVfaaYjcum2BnwctUJ4rBXuiwNU2oJwotl4BHXjs2OW38gFLShHc/DsPZx02uLHp222dAvHqxTWh9cDg==",
             "RA4zT6JVfaaYjcum2BnwctUJ4rBXuiwNU2oJwotl4BHXjs2OW38gFLShHc%2FDsPZx02uLHp222dAvHqxTWh9cDg%3D%3D"},
        };
        for (const Case& c : cases) {
            Sep7Pay p{QString(DEST())};
            p.amount = QStringLiteral("0.0000001");
            p.assetCode = QStringLiteral("USDC");
            p.assetIssuer = QString(SIGNER());
            p.memo = c.memo;
            p.memoType = QString::fromLatin1(c.type);
            p.callback = callback;
            p.msg = msg;
            p.networkPassphrase = Network::TESTNET_NETWORK_PASSPHRASE();
            p.originDomain = QStringLiteral("example.com");
            const QString unsignedUri = head + "&memo=" + QString::fromLatin1(c.encoded)
                                        + "&memo_type=" + QString::fromLatin1(c.type) + tail;
            QCOMPARE(p.toUri(), unsignedUri);
            p.sign(signer.data());
            QCOMPARE(p.signature, QString::fromLatin1(c.signature));
            const QString signedUri = unsignedUri + "&signature=" + QString::fromLatin1(c.encodedSignature);
            QCOMPARE(p.toUri(), signedUri);

            const Sep7Pay read = Sep7Pay::fromUri(signedUri);
            QCOMPARE(read.destination, QString(DEST()));
            QCOMPARE(read.amount, QStringLiteral("0.0000001"));
            QCOMPARE(read.assetCode, QStringLiteral("USDC"));
            QCOMPARE(read.assetIssuer, QString(SIGNER()));
            QCOMPARE(read.memo, c.memo);
            QCOMPARE(read.memoType, QString::fromLatin1(c.type));
            QCOMPARE(read.callback, callback);
            QCOMPARE(read.msg, msg);
            QCOMPARE(read.networkPassphrase, Network::TESTNET_NETWORK_PASSPHRASE());
            QCOMPARE(read.originDomain, QStringLiteral("example.com"));
            QCOMPARE(read.signature, QString::fromLatin1(c.signature));
            QVERIFY(read.verify(key.data()));
            QCOMPARE(read.toUri(), signedUri);
        }
    }

    /**
     * A pay request to an M... account (G...EA7 with id 1234) for the
     * largest amount core holds, the native asset (py writes no asset for
     * it) and a message of the characters a percent-encoder may disagree
     * on ('~' '_' '.' '-' kept, '*' '\'' '(' ')' '!' encoded), against
     * py-stellar-sdk 14.1.0's to_uri() and sign(SECRET).
     */
    void testPayToMuxedAccountMatchesPy()
    {
        QScopedPointer<KeyPair> signer(KeyPair::fromSecretSeed(QString(SECRET())));
        QScopedPointer<KeyPair> key(KeyPair::fromAccountId(QString(SIGNER())));
        const QString muxed = QStringLiteral("MABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU4AAAAAAAAAAE2LEJ2");
        Sep7Pay p(muxed);
        p.amount = QStringLiteral("922337203685.4775807");
        p.msg = QStringLiteral("+ ~_.-*'()!");
        const QString unsignedUri = QStringLiteral(
            "web+stellar:pay?destination=MABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU4AAAAAAAAAAE2LEJ2"
            "&amount=922337203685.4775807&msg=%2B%20~_.-%2A%27%28%29%21");
        QCOMPARE(p.toUri(), unsignedUri);
        p.sign(signer.data());
        const QString signedUri = unsignedUri + QStringLiteral(
            "&signature=YYqDEneg9mWcoEvGhcGF0cJgKfBx16HQnFeiQyEsr%2FYYvcN55cEJ9YcIDA1bFbhZs7wFCUCu2zgVxJP2S%2Bd3CA%3D%3D");
        QCOMPARE(p.toUri(), signedUri);

        const Sep7Pay read = Sep7Pay::fromUri(signedUri);
        QCOMPARE(read.destination, muxed);
        QCOMPARE(read.amount, QStringLiteral("922337203685.4775807"));
        QVERIFY(read.assetCode.isEmpty());
        QCOMPARE(read.msg, QStringLiteral("+ ~_.-*'()!"));
        QVERIFY(read.verify(key.data()));
        QCOMPARE(read.toUri(), signedUri);
    }

    /**
     * A tx request with every parameter against py-stellar-sdk 14.1.0's
     * TransactionStellarUri(...).to_uri() and sign(SECRET): a callback with
     * its own query, three replacements of which two share reference X (one
     * hint written, in first-seen order, as py's dict), a hint with a '+',
     * a pubkey, a unicode message, the testnet passphrase and an
     * origin_domain. fromUri() reads py's signed URI back and verifies it.
     */
    void testTxEveryParameterMatchesPy()
    {
        QScopedPointer<KeyPair> signer(KeyPair::fromSecretSeed(QString(SECRET())));
        QScopedPointer<KeyPair> key(KeyPair::fromAccountId(QString(SIGNER())));
        const QString msg = QString::fromUtf8("Sign 1+1 \xE2\x82\xAC \xF0\x9F\x98\x80");
        Sep7Tx t(ENVELOPE());
        t.callback = QStringLiteral("https://example.com/tx?id=7");
        t.replace = {Sep7Replacement("sourceAccount", "X", "account to use"),
                     Sep7Replacement("seqNum", "Y", "sequence + 1"),
                     Sep7Replacement("operations[0].sourceAccount", "X", "account to use")};
        t.pubkey = QString(DEST());
        t.msg = msg;
        t.networkPassphrase = Network::TESTNET_NETWORK_PASSPHRASE();
        t.originDomain = QStringLiteral("pay.example.co.uk");
        const QString unsignedUri = QStringLiteral(
            "web+stellar:tx?xdr=AAAAAgAAAAADoQe%2F884Qvh1w3RjnS8CZZ%2BTWMJulDV8d3IZkElUxuAAAAGQAAAAAAAAAAgAAAAEAAAAAAAAAAAAAAABqw3FSAAAAAAAAAAEAAAAAAAAACwAAAAAAAAALAAAAAAAAAAA%3D"
            "&callback=url%3Ahttps%3A%2F%2Fexample.com%2Ftx%3Fid%3D7"
            "&replace=sourceAccount%3AX%2CseqNum%3AY%2Coperations%5B0%5D.sourceAccount%3AX%3BX%3Aaccount%20to%20use%2CY%3Asequence%20%2B%201"
            "&pubkey=GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7"
            "&msg=Sign%201%2B1%20%E2%82%AC%20%F0%9F%98%80"
            "&network_passphrase=Test%20SDF%20Network%20%3B%20September%202015&origin_domain=pay.example.co.uk");
        QCOMPARE(t.toUri(), unsignedUri);
        t.sign(signer.data());
        QCOMPARE(t.signature, QStringLiteral(
            "evjqiuFQ2Pt0xFeuK0RLg15057qIvOv9TtdWp1k5HCGF5qDW/lAE1eGb6RN1PGntfLNUzML01sdpoXNyh9gdBw=="));
        const QString signedUri = unsignedUri + QStringLiteral(
            "&signature=evjqiuFQ2Pt0xFeuK0RLg15057qIvOv9TtdWp1k5HCGF5qDW%2FlAE1eGb6RN1PGntfLNUzML01sdpoXNyh9gdBw%3D%3D");
        QCOMPARE(t.toUri(), signedUri);

        const Sep7Tx read = Sep7Tx::fromUri(signedUri, Network::TESTNET_NETWORK_PASSPHRASE());
        QCOMPARE(read.xdr, QString(ENVELOPE()));
        QCOMPARE(read.callback, QStringLiteral("https://example.com/tx?id=7"));
        QCOMPARE(read.replace.size(), 3);
        QVERIFY(read.replace.at(0) == Sep7Replacement("sourceAccount", "X", "account to use"));
        QVERIFY(read.replace.at(1) == Sep7Replacement("seqNum", "Y", "sequence + 1"));
        QVERIFY(read.replace.at(2) == Sep7Replacement("operations[0].sourceAccount", "X", "account to use"));
        QCOMPARE(read.pubkey, QString(DEST()));
        QCOMPARE(read.msg, msg);
        QCOMPARE(read.networkPassphrase, Network::TESTNET_NETWORK_PASSPHRASE());
        QCOMPARE(read.originDomain, QStringLiteral("pay.example.co.uk"));
        QVERIFY(read.verify(key.data()));
        QCOMPARE(read.toUri(), signedUri);
    }

    /**
     * A fee bump of ENVELOPE() (fee source SIGNER, base fee 200, built with
     * py-stellar-sdk 14.1.0's build_fee_bump_transaction) in a tx request:
     * to_uri() and the signed to_uri() after sign(SECRET), byte for byte;
     * fromUri() reads it back and it verifies.
     */
    void testTxFeeBumpMatchesPy()
    {
        QScopedPointer<KeyPair> signer(KeyPair::fromSecretSeed(QString(SECRET())));
        QScopedPointer<KeyPair> key(KeyPair::fromAccountId(QString(SIGNER())));
        const QString feeBump = QStringLiteral(
            "AAAABQAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAAAAAAGQAAAAAgAAAAADoQe/884Qvh1w3RjnS8CZZ+TWMJulDV8d3IZkElUxuAAAAGQAAAAAAAAAAgAAAAEAAAAAAAAAAAAAAABqw3FSAAAAAAAAAAEAAAAAAAAACwAAAAAAAAALAAAAAAAAAAAAAAAAAAAAAA==");
        Sep7Tx t(feeBump);
        t.networkPassphrase = Network::TESTNET_NETWORK_PASSPHRASE();
        const QString unsignedUri = QStringLiteral(
            "web+stellar:tx?xdr=AAAABQAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAAAAAAGQAAAAAgAAAAADoQe%2F884Qvh1w3RjnS8CZZ%2BTWMJulDV8d3IZkElUxuAAAAGQAAAAAAAAAAgAAAAEAAAAAAAAAAAAAAABqw3FSAAAAAAAAAAEAAAAAAAAACwAAAAAAAAALAAAAAAAAAAAAAAAAAAAAAA%3D%3D"
            "&network_passphrase=Test%20SDF%20Network%20%3B%20September%202015");
        QCOMPARE(t.toUri(), unsignedUri);
        t.sign(signer.data());
        const QString signedUri = unsignedUri + QStringLiteral(
            "&signature=MaL6AVxt7FxTqg7qzWpLwaYNuPzjhxyOztizrMRoqM9toOETB7ZGNzTpajkJHBmvRmJRcfqarN5q12%2BZg7iVBA%3D%3D");
        QCOMPARE(t.toUri(), signedUri);

        const Sep7Tx read = Sep7Tx::fromUri(signedUri);
        QCOMPARE(read.xdr, feeBump);
        QCOMPARE(read.networkPassphrase, Network::TESTNET_NETWORK_PASSPHRASE());
        QVERIFY(read.verify(key.data()));
        std::unique_ptr<AbstractTransaction> tx(read.transaction(Network::testnetNetwork()));
        QCOMPARE(tx->toEnvelopeXdrBase64(), feeBump);
    }

    void testTxReplaceRoundTrip()
    {
        Sep7Tx t(ENVELOPE());
        t.replace.append(Sep7Replacement("sourceAccount", "X", "account on which to create the trustline"));
        t.replace.append(Sep7Replacement("seqNum", "Y", "sequence for sourceAccount"));
        const QString uri = t.toUri();
        QVERIFY(uri.contains("replace="));
        Sep7Tx parsed = Sep7Tx::fromUri(uri);
        QCOMPARE(parsed.replace.size(), 2);
        QCOMPARE(parsed.replace.at(0).field, QString("sourceAccount"));
        QCOMPARE(parsed.replace.at(0).reference, QString("X"));
        QCOMPARE(parsed.replace.at(0).hint, QString("account on which to create the trustline"));
        QCOMPARE(parsed.replace.at(1).field, QString("seqNum"));
        QCOMPARE(parsed.replace.at(1).hint, QString("sequence for sourceAccount"));
    }
};

ADD_TEST(Sep7Test)
#endif // SEP7TEST_H
