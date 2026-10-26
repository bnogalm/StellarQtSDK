#ifndef STELLARTOMLTEST_H
#define STELLARTOMLTEST_H

#include <QObject>
#include <QtTest>
#include "testcollector.h"

#include "../src/stellartoml.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QElapsedTimer>
#include <cmath>
#include <limits>
#include <stdexcept>

/**
 * SEP-1 stellar.toml parser — asserted against the exact structure
 * py-stellar-sdk / tomllib produce for the same fixture.
 */
class StellarTomlTest: public QObject
{
    Q_OBJECT
private slots:

    /**
     * SEP-1's [[CURRENCIES]] lookup: by issuer and code, or code_template
     * with `?` for one character; nothing for an asset the toml does not
     * list (another issuer, another code).
     */
    void testACurrencyIsFoundByIssuerAndCode()
    {
        const QString issuer = QStringLiteral("GCW667JUHCOP5Y7KY6KGDHNPHFM4CS3FCBQ7QWDUALXTX3PGXLSOEALY");
        const QString other = QStringLiteral("GA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVSGZ");
        const QVariantMap toml = StellarToml::parse(QStringLiteral(
            "[[CURRENCIES]]\n"
            "code = \"USD\"\n"
            "issuer = \"%1\"\n"
            "display_decimals = 2\n"
            "[[CURRENCIES]]\n"
            "code_template = \"CAT????\"\n"
            "issuer = \"%1\"\n").arg(issuer));
        QCOMPARE(StellarToml::currency(toml, QStringLiteral("USD"), issuer).value(QStringLiteral("display_decimals")).toLongLong(), 2LL);
        QVERIFY(!StellarToml::currency(toml, QStringLiteral("CAT1234"), issuer).isEmpty());
        QVERIFY(StellarToml::currency(toml, QStringLiteral("CAT123"), issuer).isEmpty());
        QVERIFY(StellarToml::currency(toml, QStringLiteral("DOG1234"), issuer).isEmpty());
        QVERIFY(StellarToml::currency(toml, QStringLiteral("USD"), other).isEmpty());
        QVERIFY(StellarToml::currency(toml, QStringLiteral("EUR"), issuer).isEmpty());
        QVERIFY(StellarToml::currency(QVariantMap(), QStringLiteral("USD"), issuer).isEmpty());
    }

    /**
     * currency() against py 14.1.0 (fetch_stellar_toml, tomllib) and js
     * 17.1.0 (StellarToml.Resolver, smol-toml) reading the same stellar.toml.
     * Neither has a currency helper, so each picked the first
     * [[CURRENCIES]] table with the issuer and the code, or a code_template
     * whose '?' stands for one character, as SEP-1 reads them; py and js
     * picked the same tables, member for member (the JSON below is theirs),
     * and so does currency(). Where they find nothing (a code one character
     * short, a code no table lists, a linked or contract-only table) it
     * finds nothing too.
     */
    void testACurrencyIsTheTableJsAndPyRead()
    {
        const QVariantMap toml = StellarToml::parse(QString::fromUtf8(
            "VERSION = \"2.7.0\"\n"
            "NETWORK_PASSPHRASE = \"Public Global Stellar Network ; September 2015\"\n"
            "FEDERATION_SERVER = \"https://fed.example.com/federation\"\n"
            "\n"
            "[DOCUMENTATION]\n"
            "ORG_NAME = \"Example Anchor\"\n"
            "\n"
            "[[CURRENCIES]]\n"
            "code = \"USD\"\n"
            "issuer = \"GCW667JUHCOP5Y7KY6KGDHNPHFM4CS3FCBQ7QWDUALXTX3PGXLSOEALY\"\n"
            "status = \"live\"\n"
            "display_decimals = 2\n"
            "name = \"US dollar\"\n"
            "desc = \"One dollar, \\\"redeemable\\\" at the anchor\"\n"
            "is_asset_anchored = true\n"
            "anchor_asset_type = \"fiat\"\n"
            "anchor_asset = \"USD\"\n"
            "redemption_instructions = '''\n"
            "Wire it back.'''\n"
            "collateral_addresses = [\"GA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVSGZ\"]\n"
            "regulated = false\n"
            "\n"
            "[[CURRENCIES]]\n"
            "code = \"USD\"\n"
            "issuer = \"GA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVSGZ\"\n"
            "status = \"test\"\n"
            "\n"
            "[[CURRENCIES]]\n"
            "code_template = \"BOND????\"\n"
            "issuer = \"GCW667JUHCOP5Y7KY6KGDHNPHFM4CS3FCBQ7QWDUALXTX3PGXLSOEALY\"\n"
            "display_decimals = 7\n"
            "fixed_number = 1_000\n"
            "\n"
            "[[CURRENCIES]]\n"
            "toml = \"https://example.com/.well-known/EUR.toml\"\n"
            "\n"
            "[[CURRENCIES]]\n"
            "contract = \"CAS3J7GYLGXMF6TDJBBYYSE3HQ6BBSMLNUQ34T6TZMYMW2EVH34XOWMA\"\n"
            "code = \"USDC\"\n"));
        QCOMPARE(toml.value(QStringLiteral("CURRENCIES")).toList().size(), 5);
        const char* issuer = "GCW667JUHCOP5Y7KY6KGDHNPHFM4CS3FCBQ7QWDUALXTX3PGXLSOEALY";
        const char* other = "GA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVSGZ";
        struct Vector { const char* code; const char* issuer; const char* json; };
        const Vector vectors[] = {
            {"USD", issuer,
             "{\"anchor_asset\":\"USD\",\"anchor_asset_type\":\"fiat\",\"code\":\"USD\",\"collateral_addresses\":[\"GA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVSGZ\"],\"desc\":\"One dollar, \\\"redeemable\\\" at the anchor\",\"display_decimals\":2,\"is_asset_anchored\":true,\"issuer\":\"GCW667JUHCOP5Y7KY6KGDHNPHFM4CS3FCBQ7QWDUALXTX3PGXLSOEALY\",\"name\":\"US dollar\",\"redemption_instructions\":\"Wire it back.\",\"regulated\":false,\"status\":\"live\"}"},
            {"USD", other, "{\"code\":\"USD\",\"issuer\":\"GA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVSGZ\",\"status\":\"test\"}"},
            {"BOND2030", issuer,
             "{\"code_template\":\"BOND????\",\"display_decimals\":7,\"fixed_number\":1000,\"issuer\":\"GCW667JUHCOP5Y7KY6KGDHNPHFM4CS3FCBQ7QWDUALXTX3PGXLSOEALY\"}"},
            {"BOND203", issuer, "{}"},
            {"EUR", issuer, "{}"},
            {"USDC", "", "{}"},
        };
        for (const Vector& vector : vectors) {
            const QJsonObject expected = QJsonDocument::fromJson(QByteArray(vector.json)).object();
            const QJsonObject found = QJsonObject::fromVariantMap(
                StellarToml::currency(toml, QString::fromLatin1(vector.code), QString::fromLatin1(vector.issuer)));
            QVERIFY2(found == expected, vector.code);
        }
    }
    void initTestCase() {}
    void cleanupTestCase() {}

    void testParse()
    {
        const QString toml =
            "# Stellar.toml fixture\n"
            "VERSION = \"2.0.0\"\n"
            "NETWORK_PASSPHRASE = \"Public Global Stellar Network ; September 2015\"\n"
            "SIGNING_KEY = \"GBVOL67TMUQBGL4TZYNMY3ZQ5WGQYFPFD5VJRWXR72VA33VFNL225PL5\"\n"
            "ACCOUNTS = [\n"
            "  \"GAOEOVMSDQI5VWAIPDQUTV4OFTXLZB4GENZESHGFMTGFSGAUMA56WLY7\",\n"
            "  \"GDIRGGTBE3H4CUJHNBCPA34QM2A4GAQ2QO6FWHJN6NEKWBJ3GR2GUE5W\"\n"
            "]\n"
            "MAX_AGE = 86400\n"
            "ENABLED = true\n"
            "\n"
            "[DOCUMENTATION]\n"
            "ORG_NAME = \"Example Anchor\"\n"
            "ORG_DESCRIPTION = \"\"\"A multi\n"
            "line description.\"\"\"\n"
            "\n"
            "[[CURRENCIES]]\n"
            "code = \"USD\"\n"
            "is_asset_anchored = true\n"
            "\n"
            "[[CURRENCIES]]\n"
            "code = \"EUR\"\n";

        const QVariantMap t = StellarToml::parse(toml);

        QCOMPARE(t.value("VERSION").toString(), QString("2.0.0"));
        QCOMPARE(t.value("NETWORK_PASSPHRASE").toString(),
                 QString("Public Global Stellar Network ; September 2015"));
        QCOMPARE(t.value("SIGNING_KEY").toString(),
                 QString("GBVOL67TMUQBGL4TZYNMY3ZQ5WGQYFPFD5VJRWXR72VA33VFNL225PL5"));

        const QVariantList accounts = t.value("ACCOUNTS").toList();
        QCOMPARE(accounts.size(), 2);
        QCOMPARE(accounts.at(0).toString(),
                 QString("GAOEOVMSDQI5VWAIPDQUTV4OFTXLZB4GENZESHGFMTGFSGAUMA56WLY7"));
        QCOMPARE(accounts.at(1).toString(),
                 QString("GDIRGGTBE3H4CUJHNBCPA34QM2A4GAQ2QO6FWHJN6NEKWBJ3GR2GUE5W"));

        QCOMPARE(t.value("MAX_AGE").toLongLong(), qlonglong(86400));
        QCOMPARE(t.value("ENABLED").toBool(), true);

        const QVariantMap doc = t.value("DOCUMENTATION").toMap();
        QCOMPARE(doc.value("ORG_NAME").toString(), QString("Example Anchor"));
        QCOMPARE(doc.value("ORG_DESCRIPTION").toString(), QString("A multi\nline description."));

        const QVariantList currencies = t.value("CURRENCIES").toList();
        QCOMPARE(currencies.size(), 2);
        QCOMPARE(currencies.at(0).toMap().value("code").toString(), QString("USD"));
        QCOMPARE(currencies.at(0).toMap().value("is_asset_anchored").toBool(), true);
        QCOMPARE(currencies.at(1).toMap().value("code").toString(), QString("EUR"));
    }

    void testTrailingCommentAndBoolFalse()
    {
        const QString toml =
            "A = 42  # the answer\n"
            "B = false\n"
            "C = \"has # hash inside\"\n";
        const QVariantMap t = StellarToml::parse(toml);
        QCOMPARE(t.value("A").toLongLong(), qlonglong(42));   // trailing comment stripped
        QCOMPARE(t.value("B").toBool(), false);
        QCOMPARE(t.value("C").toString(), QString("has # hash inside")); // '#' inside string kept
    }

    /** An unterminated `"""` used to make QString::mid clamp a negative length
     *  to "rest of the string", so parse() returned success with the whole
     *  remainder of the document swallowed into that one value — every key
     *  after it silently lost. The header promises a throw on malformed
     *  input. */
    void testUnterminatedMultilineStringThrows()
    {
        const QString toml =
            "ORG_DESCRIPTION = \"\"\"unterminated\n"
            "FEDERATION_SERVER = \"https://fed.example.com\"\n";
        bool threw = false;
        try { StellarToml::parse(toml); }
        catch (const std::runtime_error&) { threw = true; }
        QVERIFY(threw);
    }

    // ── K4: one test per defect of the old reader ─────────────────────

    /** `\uXXXX` / `\UXXXXXXXX` used to lose the backslash: "Caf\u00e9"
     *  came back as "Cafu00e9". */
    void testUnicodeEscapes()
    {
        const QVariantMap t = StellarToml::parse(
            "A = \"Caf\\u00e9\"\n"
            "B = \"rocket \\U0001F680!\"\n"
            "C = \"\\b\\t\\n\\f\\r\\\"\\\\\"\n");
        QCOMPARE(t.value("A").toString(), QString::fromUtf8("Caf\xC3\xA9"));
        QCOMPARE(t.value("B").toString(), QString::fromUtf8("rocket \xF0\x9F\x9A\x80!"));
        QCOMPARE(t.value("C").toString(), QString("\b\t\n\f\r\"\\"));
    }

    /** An unknown escape (`\q`, `\/`, `\e`, `\x41`) used to be accepted;
     *  TOML 1.0 refuses it, and an escape that is not a Unicode scalar. */
    void testUnknownEscapeRefused()
    {
        QVERIFY(!parseError("A = \"\\q\"\n").isEmpty());
        QVERIFY(!parseError("A = \"\\/\"\n").isEmpty());
        QVERIFY(!parseError("A = \"\\e\"\n").isEmpty());
        QVERIFY(!parseError("A = \"\\x41\"\n").isEmpty());
        QVERIFY(!parseError("A = \"\\uD800\"\n").isEmpty());      // surrogate
        QVERIFY(!parseError("A = \"\\U00110000\"\n").isEmpty());  // beyond U+10FFFF
        QVERIFY(!parseError("A = \"\\u12\"\n").isEmpty());        // too short
    }

    /** 'literal' strings kept their quotes, and a '#' inside truncated them. */
    void testLiteralStrings()
    {
        const QVariantMap t = StellarToml::parse(
            "A = 'C:\\Users\\nodejs'\n"
            "B = 'a # not a comment' # a comment\n"
            "C = 'Tom \"Dubs\" Preston'\n"
            "D = ''\n");
        QCOMPARE(t.value("A").toString(), QString("C:\\Users\\nodejs"));
        QCOMPARE(t.value("B").toString(), QString("a # not a comment"));
        QCOMPARE(t.value("C").toString(), QString("Tom \"Dubs\" Preston"));
        QCOMPARE(t.value("D").toString(), QString(""));
        QVERIFY(!parseError("A = 'unterminated\n").isEmpty());
    }

    /** `'''` multi-line literal strings were not supported at all. */
    void testMultilineLiteralStrings()
    {
        const QVariantMap t = StellarToml::parse(
            "A = '''\n"
            "first newline trimmed\n"
            "  \\n stays as two characters'''\n"
            "B = ''''quoted,' she said''''\n");
        QCOMPARE(t.value("A").toString(),
                 QString("first newline trimmed\n  \\n stays as two characters"));
        QCOMPARE(t.value("B").toString(), QString("'quoted,' she said'"));
        QVERIFY(!parseError("A = '''never closed\nB = 1\n").isEmpty());
    }

    /** Escapes inside `"""..."""` were returned raw. */
    void testMultilineBasicEscapes()
    {
        const QVariantMap t = StellarToml::parse(
            "A = \"\"\"Caf\\u00e9\\tbar\"\"\"\n"
            "B = \"\"\"\\\n"
            "    The quick \\\n"
            "\n"
            "    fox.\"\"\"\n"
            "C = \"\"\"two \"\" quotes\"\"\"\"\n");
        QCOMPARE(t.value("A").toString(), QString::fromUtf8("Caf\xC3\xA9\tbar"));
        QCOMPARE(t.value("B").toString(), QString("The quick fox."));  // line-ending backslash
        QCOMPARE(t.value("C").toString(), QString("two \"\" quotes\""));
        QVERIFY(!parseError("A = \"\"\"\\q\"\"\"\n").isEmpty());
    }

    /** A repeated key silently overwrote the first one. */
    void testDuplicateKeyRefused()
    {
        QVERIFY(parseError("A = 1\nA = 2\n").contains("duplicate key"));
        QVERIFY(!parseError("A = 1\n\"A\" = 2\n").isEmpty());       // same key, quoted
        QVERIFY(!parseError("[T]\nk = 1\nk = 2\n").isEmpty());
        QVERIFY(!parseError("A = { b = 1, b = 2 }\n").isEmpty());
        QVERIFY(!parseError("a.b = 1\na.b.c = 2\n").isEmpty());     // value, then table
    }

    /** A repeated `[TABLE]` silently replaced the first one. */
    void testDuplicateTableRefused()
    {
        QVERIFY(parseError("[DOCUMENTATION]\nA = 1\n[DOCUMENTATION]\nB = 2\n")
                    .contains("more than once"));
        QVERIFY(!parseError("[[CURRENCIES]]\n[CURRENCIES]\n").isEmpty());
        QVERIFY(!parseError("[A]\n[[A]]\n").isEmpty());
        QVERIFY(!parseError("A = 1\n[A]\n").isEmpty());
        QVERIFY(!parseError("A = { b = 1 }\n[A]\n").isEmpty());     // inline tables are closed
        QVERIFY(!parseError("A = []\n[[A]]\n").isEmpty());          // static arrays too
        QVERIFY(!parseError("[A]\nb.c = 1\n[A.b]\n").isEmpty());    // made by dotted keys
        // ... but a super-table named only implicitly may be declared once, later.
        QVERIFY(parseError("[A.B.C]\n[A]\n[A.B]\n").isEmpty());
    }

    /** `[DOC` without a closing bracket was accepted as table "DOC". */
    void testUnclosedHeaderRefused()
    {
        QVERIFY(parseError("[DOCUMENTATION\nORG_NAME = \"x\"\n").contains("']'"));
        QVERIFY(!parseError("[DOC").isEmpty());
        QVERIFY(!parseError("[[CURRENCIES]\n").isEmpty());
        QVERIFY(!parseError("[]\n").isEmpty());
        QVERIFY(!parseError("[A] B\n").isEmpty());
    }

    /** Anything after a closing quote (or any value) was ignored. */
    void testJunkAfterValueRefused()
    {
        QVERIFY(parseError("A = \"abc\" junk\n").contains("line 1"));
        QVERIFY(!parseError("A = \"abc\"\"def\"\n").isEmpty());
        QVERIFY(!parseError("A = 'abc' x\n").isEmpty());
        QVERIFY(!parseError("A = [1, 2] x\n").isEmpty());
        QVERIFY(!parseError("A = 12abc\n").isEmpty());
        QVERIFY(!parseError("A = truex\n").isEmpty());
        QVERIFY(!parseError("A = 1 B = 2\n").isEmpty());
        QVERIFY(!parseError("A = bare words\n").isEmpty());          // used to be a string
    }

    /** Inline tables were read silently as the string "{ ... }". */
    void testInlineTables()
    {
        const QVariantMap t = StellarToml::parse(
            "A = { code = \"USD\", n = 2, sub = { x = true }, d.e = 'f' }\n"
            "E = {}\n");
        const QVariantMap a = t.value("A").toMap();
        QCOMPARE(t.value("A").userType(), int(QMetaType::QVariantMap));
        QCOMPARE(a.value("code").toString(), QString("USD"));
        QCOMPARE(a.value("n").toLongLong(), qlonglong(2));
        QCOMPARE(a.value("sub").toMap().value("x").toBool(), true);
        QCOMPARE(a.value("d").toMap().value("e").toString(), QString("f"));
        QCOMPARE(t.value("E").userType(), int(QMetaType::QVariantMap));
        QVERIFY(t.value("E").toMap().isEmpty());
        QVERIFY(!parseError("A = { b = 1, }\n").isEmpty());          // no trailing comma
        QVERIFY(!parseError("A = { b = 1,\n c = 2 }\n").isEmpty());  // no newline
    }

    /** Dotted keys were stored under the literal key "a.b". */
    void testDottedKeys()
    {
        const QVariantMap t = StellarToml::parse(
            "physical.color = \"orange\"\n"
            "physical . shape = \"round\"\n"
            "site.\"google.com\" = true\n");
        QCOMPARE(t.value("physical").toMap().value("color").toString(), QString("orange"));
        QCOMPARE(t.value("physical").toMap().value("shape").toString(), QString("round"));
        QCOMPARE(t.value("site").toMap().value("google.com").toBool(), true);
        QVERIFY(!t.contains("physical.color"));
    }

    /** Date-times were silently strings; now they are validated, and come
     *  back as their original text (documented in the header). */
    void testDateTimesKeptAsText()
    {
        const QVariantMap t = StellarToml::parse(
            "A = 1979-05-27T07:32:00Z\n"
            "B = 1979-05-27 00:32:00.999999-07:00\n"
            "C = 1979-05-27T07:32:00\n"
            "D = 1979-05-27\n"
            "E = 07:32:00\n");
        QCOMPARE(t.value("A").toString(), QString("1979-05-27T07:32:00Z"));
        QCOMPARE(t.value("B").toString(), QString("1979-05-27 00:32:00.999999-07:00"));
        QCOMPARE(t.value("C").toString(), QString("1979-05-27T07:32:00"));
        QCOMPARE(t.value("D").toString(), QString("1979-05-27"));
        QCOMPARE(t.value("E").toString(), QString("07:32:00"));
        QCOMPARE(t.value("A").userType(), int(QMetaType::QString));
        QVERIFY(!parseError("A = 1979-02-30\n").isEmpty());          // no such day
        QVERIFY(!parseError("A = 1979-05-27T24:00:00\n").isEmpty());
        QVERIFY(!parseError("A = 1979-05-27T07:32Z\n").isEmpty());   // seconds are required
    }

    /** `[a.b]` became a top-level key "a.b"; `[[a.b]]` likewise. */
    void testNestedTables()
    {
        const QVariantMap t = StellarToml::parse(
            "[[fruits]]\n"
            "name = \"apple\"\n"
            "[fruits.physical]\n"
            "color = \"red\"\n"
            "[[fruits.varieties]]\n"
            "name = \"red delicious\"\n"
            "[[fruits.varieties]]\n"
            "name = \"granny smith\"\n"
            "[[fruits]]\n"
            "name = \"banana\"\n"
            "[dog.\"tater.man\".type]\n"
            "name = \"pug\"\n");
        const QVariantList fruits = t.value("fruits").toList();
        QCOMPARE(fruits.size(), 2);
        const QVariantMap apple = fruits.at(0).toMap();
        QCOMPARE(apple.value("physical").toMap().value("color").toString(), QString("red"));
        const QVariantList varieties = apple.value("varieties").toList();
        QCOMPARE(varieties.size(), 2);
        QCOMPARE(varieties.at(1).toMap().value("name").toString(), QString("granny smith"));
        QCOMPARE(fruits.at(1).toMap().value("name").toString(), QString("banana"));
        QCOMPARE(t.value("dog").toMap().value("tater.man").toMap()
                     .value("type").toMap().value("name").toString(), QString("pug"));
        QVERIFY(!t.contains("fruits.physical"));
    }

    /** `1_000`, `0xff`, `0o17`, `0b101` were read as strings. */
    void testIntegerForms()
    {
        const QVariantMap t = StellarToml::parse(
            "A = 1_000\nB = 0xDEAD_beef\nC = 0o755\nD = 0b1101\nE = +17\nF = -0\n");
        QCOMPARE(t.value("A").userType(), int(QMetaType::LongLong));
        QCOMPARE(t.value("A").toLongLong(), qlonglong(1000));
        QCOMPARE(t.value("B").toLongLong(), qlonglong(0xDEADBEEF));
        QCOMPARE(t.value("C").toLongLong(), qlonglong(0755));
        QCOMPARE(t.value("D").toLongLong(), qlonglong(13));
        QCOMPARE(t.value("E").toLongLong(), qlonglong(17));
        QCOMPARE(t.value("F").toLongLong(), qlonglong(0));
        QVERIFY(!parseError("A = 1__0\n").isEmpty());
        QVERIFY(!parseError("A = 10_\n").isEmpty());
        QVERIFY(!parseError("A = 0123\n").isEmpty());                // no leading zeros
        QVERIFY(!parseError("A = 0XFF\n").isEmpty());
        QVERIFY(!parseError("A = +0x10\n").isEmpty());
    }

    /** An integer beyond int64 became a rounded double. */
    void testIntegerOutOfRangeRefused()
    {
        const QVariantMap t = StellarToml::parse(
            "A = 9223372036854775807\nB = -9223372036854775808\n");
        QCOMPARE(t.value("A").toLongLong(), std::numeric_limits<qlonglong>::max());
        QCOMPARE(t.value("B").toLongLong(), std::numeric_limits<qlonglong>::min());
        QVERIFY(parseError("A = 9223372036854775808\n").contains("64-bit"));
        QVERIFY(!parseError("A = -9223372036854775809\n").isEmpty());
        QVERIFY(!parseError("A = 0xFFFFFFFFFFFFFFFF\n").isEmpty());
    }

    void testFloats()
    {
        const QVariantMap t = StellarToml::parse(
            "A = 3.14\nB = -2E-2\nC = 5e+22\nD = inf\nE = -inf\nF = nan\nG = 1_000.5\nH = 1e400\n");
        QCOMPARE(t.value("A").userType(), int(QMetaType::Double));
        QCOMPARE(t.value("A").toDouble(), 3.14);
        QCOMPARE(t.value("B").toDouble(), -0.02);
        QCOMPARE(t.value("C").toDouble(), 5e22);
        QVERIFY(std::isinf(t.value("D").toDouble()) && t.value("D").toDouble() > 0);
        QVERIFY(std::isinf(t.value("E").toDouble()) && t.value("E").toDouble() < 0);
        QVERIFY(std::isnan(t.value("F").toDouble()));
        QCOMPARE(t.value("G").toDouble(), 1000.5);
        QVERIFY(std::isinf(t.value("H").toDouble()));               // IEEE rounding, as tomllib
        QVERIFY(!parseError("A = .5\n").isEmpty());
        QVERIFY(!parseError("A = 5.\n").isEmpty());
        QVERIFY(!parseError("A = Inf\n").isEmpty());
    }

    /** The error names the line it is about. */
    void testErrorNamesLine()
    {
        const QString msg = parseError("A = 1\n\n[B]\nC = \"x\" y\n");
        QVERIFY2(msg.startsWith("stellar.toml: line 4:"), qPrintable(msg));
    }

    /** Control characters and a bare CR are not TOML. CRLF is. */
    void testControlCharactersRefused()
    {
        QVERIFY(!parseError("A = 1\rB = 2\n").isEmpty());
        QVERIFY(!parseError("A = \"a\x01b\"\n").isEmpty());
        QVERIFY(!parseError("# bad \x7f\n").isEmpty());
        QVERIFY(!parseError("A = \"a\nb\"\n").isEmpty());            // basic strings are one line
        const QVariantMap t = StellarToml::parse("A = 1\r\n[T]\r\nB = \"\"\"x\r\ny\"\"\"\r\n");
        QCOMPARE(t.value("A").toLongLong(), qlonglong(1));
        QCOMPARE(t.value("T").toMap().value("B").toString(), QString("x\ny"));
    }

    /**
     * Each number, date or time was matched without telling PCRE2 that the
     * document is valid UTF-16, so every match checked the document from the
     * value to its end: O(n^2), an estimated few seconds for a 100 KB
     * stellar.toml of ints, on the thread that resolves a federation address
     * or a SEP-7 origin. It is checked once now: 200,000 values (400 KB)
     * meant some 10^11 code units checked, and take well under a second.
     */
    void testALargeDocumentIsReadInLinearTime()
    {
        QString toml = QStringLiteral("A = [");
        toml.reserve(400100);
        for (int i = 0; i < 200000; ++i)
            toml += QStringLiteral("1,");
        toml += QStringLiteral("2]\nB = 1979-05-27\nC = 07:32:00\n");
        QElapsedTimer timer;
        timer.start();
        const QVariantMap t = StellarToml::parse(toml);
        const qint64 elapsed = timer.elapsed();
        QCOMPARE(t.value("A").toList().size(), 200001);
        QCOMPARE(t.value("A").toList().last().toLongLong(), qlonglong(2));
        QCOMPARE(t.value("B").toString(), QString("1979-05-27"));
        QCOMPARE(t.value("C").toString(), QString("07:32:00"));
        QVERIFY2(elapsed < 5000, qPrintable(QStringLiteral("%1 ms").arg(elapsed)));
    }

    /**
     * A [[CURRENCIES]] entry that links out to its own file (toml = "...")
     * or names a contract alone is not followed or matched: currency() reads
     * the entries written in the file it is given, as its header says.
     */
    void testALinkedCurrencyIsNotFollowed()
    {
        const QString issuer = QStringLiteral("GCW667JUHCOP5Y7KY6KGDHNPHFM4CS3FCBQ7QWDUALXTX3PGXLSOEALY");
        const QVariantMap toml = StellarToml::parse(QStringLiteral(
            "[[CURRENCIES]]\n"
            "toml = \"https://example.com/.well-known/USD.toml\"\n"
            "[[CURRENCIES]]\n"
            "code = \"EUR\"\n"
            "contract = \"CDLZFC3SYJYDZT7K67VZ75HPJVIEUVNIXF47ZG2FB2RMQQVU2HHGCYSC\"\n"));
        QCOMPARE(toml.value("CURRENCIES").toList().first().toMap().value("toml").toString(),
                 QString("https://example.com/.well-known/USD.toml"));
        QVERIFY(StellarToml::currency(toml, QStringLiteral("USD"), issuer).isEmpty());
        QVERIFY(StellarToml::currency(toml, QStringLiteral("EUR"), issuer).isEmpty());
    }

    /** A hostile file cannot exhaust the stack: nesting is capped. */
    void testNestingDepthBounded()
    {
        const int max = StellarToml::MaxDepth;
        // In `A = [[...]]` with n brackets the innermost array sits at depth n.
        const QString open(max, QLatin1Char('[')), close(max, QLatin1Char(']'));
        QVERIFY(parseError("A = " + open + close + "\n").isEmpty());
        QVERIFY(parseError("A = [" + open + close + "]\n").contains("nesting"));
        QVERIFY(!parseError("A = " + QString(200000, QLatin1Char('['))).isEmpty());
        QVERIFY(!parseError("A = " + QString(200000, QLatin1Char('{')).replace("{", "{a=")).isEmpty());
        QStringList deep;
        for (int i = 0; i <= max; ++i) deep << "k";
        QVERIFY(!parseError("[" + deep.join(QLatin1Char('.')) + "]\n").isEmpty());
        QVERIFY(!parseError(deep.join(QLatin1Char('.')) + " = 1\n").isEmpty());
    }

private:
    /** The parse error message, or an empty string if `toml` parses. */
    static QString parseError(const QString& toml)
    {
        try {
            StellarToml::parse(toml);
        } catch (const std::runtime_error& e) {
            return QString::fromUtf8(e.what());
        }
        return QString();
    }
};

ADD_TEST(StellarTomlTest)
#endif // STELLARTOMLTEST_H
