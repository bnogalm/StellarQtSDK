#ifndef STELLARTOMLTEST_H
#define STELLARTOMLTEST_H

#include <QObject>
#include <QtTest>
#include "testcollector.h"

#include "../src/stellartoml.h"
#include <stdexcept>

/**
 * SEP-1 stellar.toml parser — asserted against the exact structure
 * py-stellar-sdk / tomllib produce for the same fixture.
 */
class StellarTomlTest: public QObject
{
    Q_OBJECT
private slots:
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
};

ADD_TEST(StellarTomlTest)
#endif // STELLARTOMLTEST_H
