#ifndef SEP7TEST_H
#define SEP7TEST_H

#include <QObject>
#include <QtTest>
#include "testcollector.h"

#include "../src/sep7.h"
#include "../src/keypair.h"
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

    // Precise percent-encoding check (+ / = → %2B %2F %3D) + param order +
    // round-trip, without embedding a giant xdr literal. The network-passphrase
    // encoding (%20 / %3B) is verbatim from py-stellar-sdk. The signing path is
    // shared with Sep7Pay (proven byte-exact above), so it isn't re-checked here.
    void testTxEncodingAndRoundTrip()
    {
        Sep7Tx t("AAAA+/==");
        t.msg = "Sign me";
        t.networkPassphrase = "Test SDF Network ; September 2015";
        t.originDomain = "example.com";
        const QString uri = t.toUri();
        QCOMPARE(uri, QString(
            "web+stellar:tx?xdr=AAAA%2B%2F%3D%3D&msg=Sign%20me"
            "&network_passphrase=Test%20SDF%20Network%20%3B%20September%202015&origin_domain=example.com"));

        Sep7Tx p = Sep7Tx::fromUri(uri);
        QCOMPARE(p.xdr, QString("AAAA+/=="));   // %2B%2F%3D%3D decoded back
        QCOMPARE(p.msg, QString("Sign me"));
        QCOMPARE(p.networkPassphrase, QString("Test SDF Network ; September 2015"));
        QCOMPARE(p.originDomain, QString("example.com"));
    }

    void testTxReplaceRoundTrip()
    {
        Sep7Tx t("AAAAAA==");
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
