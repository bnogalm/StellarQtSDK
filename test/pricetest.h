#ifndef PRICETEST_H
#define PRICETEST_H
#include <type_traits>
#include <limits>
#include <QObject>
#include <QtTest>
#include <stdexcept>
#include "testcollector.h"
#include "src/price.h"
#ifdef STELLAR_QT_QML
#include <QJSEngine>
#include <QQmlEngine>
#include "src/qml/stellarqml.h"
#endif

class PriceTest: public QObject
{
    Q_OBJECT


private slots:
    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }

    void testFromDouble() {
        bool zeroRefused = false;                       // not a price
        try { delete Price::fromString("0"); } catch (const std::exception&) { zeroRefused = true; }
        QVERIFY(zeroRefused);
        QVERIFY(Price::fromString("0.1")->equals( new Price(1,10)));
        QVERIFY(Price::fromString("0.01")->equals( new Price(1,100)));
        QVERIFY(Price::fromString("0.001")->equals( new Price(1,1000)));
        QVERIFY(Price::fromString("543.01793")->equals( new Price(54301793,100000)));
        QVERIFY(Price::fromString("319.69983")->equals( new Price(31969983,100000)));
        QVERIFY(Price::fromString("0.93")->equals( new Price(93,100)));
        QVERIFY(Price::fromString("0.5")->equals( new Price(1,2)));
        QVERIFY(Price::fromString("1.730")->equals( new Price(173,100)));
        QVERIFY(Price::fromString("0.85334384")->equals( new Price(5333399,6250000)));
        QVERIFY(Price::fromString("5.5")->equals( new Price(11,2)));
        QVERIFY(Price::fromString("2.72783")->equals( new Price(272783,100000)));
        QVERIFY(Price::fromString("638082.0")->equals( new Price(638082,1)));
        QVERIFY(Price::fromString("2.93850088")->equals( new Price(36731261,12500000)));
        QVERIFY(Price::fromString("58.04")->equals( new Price(1451,25)));
        QVERIFY(Price::fromString("41.265")->equals( new Price(8253,200)));
        QVERIFY(Price::fromString("5.1476")->equals( new Price(12869,2500)));
        QVERIFY(Price::fromString("95.14")->equals( new Price(4757,50)));
        QVERIFY(Price::fromString("0.74580")->equals( new Price(3729,5000)));
        QVERIFY(Price::fromString("4119.0")->equals( new Price(4119,1)));
        QVERIFY(Price::fromString("1073742464.5")->equals( new Price(1073742464,1)));
        QVERIFY(Price::fromString("1635962526.2")->equals( new Price(1635962526,1)));
        QVERIFY(Price::fromString("2147483647")->equals( new Price(2147483647,1)));
    }

    /**
     * S1 regression: a value no int32 fraction can approximate must throw,
     * never wrap to a corrupt price. That is 2^31 and above, and anything
     * positive at or below 1/2^31. py-stellar-sdk 14.1.0 refuses these too;
     * js-stellar-sdk 17.1.0 signs them as 2147483647/1 or 1/2147483647
     * ("3e9", "1e15", "1e-11", "1e-20" among them), a different price.
     *
     * The two 18446744073709551617 exponents are 2^64 + 1: without the clamp on
     * the exponent they wrap to 1 and the price comes out as 10 and 1/2.
     */
    void testRejectsOutOfInt32Range() {
        for (const char* text : {"2147483648", "3e9", "1e10", "1e15", "99999999999.5",
                                 "4e-10", "1e-11", "1e-20", "1e99999999999999999999",
                                 "0.0000000004656612873077392578125",   // exactly 1/2^31
                                 "1e18446744073709551617", "5e-18446744073709551617",
                                 "0e99999999999999999999"}) {     // zero is not a price
            bool threw = false;
            try { Price p{QString::fromLatin1(text)}; Q_UNUSED(p) }
            catch (const std::exception&) { threw = true; }
            QVERIFY2(threw, text);
        }
        // The edges that do fit.
        QVERIFY(isPrice(Price(QStringLiteral("2147483647")), 2147483647, 1));
        QVERIFY(isPrice(Price(QStringLiteral("2147483647.4")), 2147483647, 1));
        QVERIFY(isPrice(Price(QStringLiteral("5e-10")), 1, 2000000000));
        // Between 1/2^31 and 1/(2^31 - 1): the smallest price there is.
        QVERIFY(isPrice(Price(QStringLiteral("4.656612874e-10")), 1, 2147483647));
    }

    /**
     * The whole text, read exactly. The old parser kept the first 11
     * characters and went through a double, so "1.234567e-06" was read as
     * "1.234567e-0" — an offer priced a million times too high, signed without
     * a word — and "0000000000012" as zero. QML hands a JS number over in
     * exactly that notation.
     */
    void testReadsTheWholeTextExactly() {
        const Price small(QStringLiteral("1.234567e-06"));
        QVERIFY(relativeError(small, 1.234567e-06) < 1.0 / 2147483647.0);

        QVERIFY(isPrice(Price(QStringLiteral("0000000000012")), 12, 1));
        QVERIFY(isPrice(Price(QStringLiteral("1.2345e-3")), 2469, 2000000));
        QVERIFY(isPrice(Price(QStringLiteral("12.5E2")), 1250, 1));
        QVERIFY(isPrice(Price(QStringLiteral("1e-7")), 1, 10000000));
        QVERIFY(isPrice(Price(QStringLiteral(".5")), 1, 2));
        QVERIFY(isPrice(Price(QStringLiteral("5.")), 5, 1));
        QVERIFY(isPrice(Price(QStringLiteral("+1.5")), 3, 2));
        QVERIFY(isPrice(Price(QStringLiteral(" 1.5 ")), 3, 2));
        // More digits than a double holds: read, not refused, and not rounded
        // through a double on the way.
        QVERIFY(relativeError(Price(QStringLiteral("0.12345678901234567890123456789012345")),
                              0.12345678901234567890123456789012345) < 1.0 / 2147483647.0);
    }

    /** Anything that is not plainly a non-negative decimal is refused. */
    void testRejectsWhatIsNotADecimal() {
        const QStringList bad = {
            "", "   ", ".", "+", "e5", "1e", "1e+", "1.2.3", "1,5", "1 000",
            "0x10", "nan", "NaN", "inf", "+inf", "-1", "-0", "--1", "abc",
            "0", "0.000", "+0", ".0",               // zero is not a price
            QString::fromUtf8("\xEF\xBC\x91"),     // FULLWIDTH DIGIT ONE
            QString(300, QLatin1Char('1')),
            // In range, but longer than the 256 characters the parser reads.
            QStringLiteral("0.") + QString(298, QLatin1Char('3'))
        };
        for (const QString& text : bad) {
            bool threw = false;
            try { Price p(text); Q_UNUSED(p) }
            catch (const std::exception&) { threw = true; }
            QVERIFY2(threw, qPrintable(QStringLiteral("accepted: '") + text + "'"));
        }
    }

    /**
     * A decimal that is not itself an int32 fraction is approximated by one
     * (the fraction java and js sign, see below), here never further off than
     * one part in 2^31 - 1. This value used to be refused; it is the case the
     * S1 guard was really about, and it now has an answer instead of an
     * exception.
     */
    void testApproximatesOnlyWhenItMust() {
        const Price p(QStringLiteral("5000000.123456789"));
        QVERIFY(isPrice(p, 405000010, 81));
        QVERIFY(relativeError(p, 5000000.123456789) < 1.0 / 2147483647.0);

        QVERIFY(isPrice(Price(QStringLiteral("0.3333333333")), 1, 3));
        QVERIFY(isPrice(Price(QStringLiteral("0.7777777778")), 7, 9));
        QVERIFY(relativeError(Price(QStringLiteral("3.14159265358979323846")),
                              3.14159265358979323846) < 1.0 / 2147483647.0);

        // What the Java and JavaScript SDKs sign: they round while expanding,
        // and here that lands on 1442/1874674987, three times closer to
        // 7.692e-7 than the last exact convergent (481/625325013) this SDK used
        // to sign instead. The same text now gives the same offer everywhere.
        QVERIFY(isPrice(Price(QStringLiteral("7.692E-7")), 1442, 1874674987));
    }

    /**
     * A decoded offer's price string is n/d exactly, as js-stellar-sdk 17.1.0
     * prints it (BigNumber(n).div(d): 20 decimals rounded half up, and
     * exponential from 1e-7 down). It went through a double: 36% of 111,730
     * real offers read differently from js, and some had a wrong last digit.
     */
    void testPriceStringsAreJsExactDecimals() {
        const QList<QPair<QPair<qint32, qint32>, QString>> js = {
            {{1, 3}, QStringLiteral("0.33333333333333333333")},
            {{1, 7}, QStringLiteral("0.14285714285714285714")},
            {{2147483647, 3}, QStringLiteral("715827882.33333333333333333333")},
            {{1, 2147483647}, QStringLiteral("4.6566128752e-10")},
            {{10000000, 2288460}, QStringLiteral("4.3697508368072852486")},
            {{779698349, 2021290}, QStringLiteral("385.74294089418143858625")},
            {{91, 7624545}, QStringLiteral("0.00001193513842465354")},
            {{2, 1}, QStringLiteral("2")},
            {{1, 10000000}, QStringLiteral("1e-7")},
            {{1, 100000000}, QStringLiteral("1e-8")},
            {{1, 99999999}, QStringLiteral("1.00000001e-8")},
            {{5, 100000000}, QStringLiteral("5e-8")},
            {{123, 1000000000}, QStringLiteral("1.23e-7")},
            {{0x27c3299d, 0x2fee8}, QStringLiteral("3397.89330609999592518642")},
        };
        for (const auto& c : js)
            QCOMPARE(Price::toString(c.first.first, c.first.second), c.second);
    }

    /**
     * What Price::toString prints, Price(QString) reads back as the same
     * fraction whenever the decimal pins it down — which covers the fractions
     * people write. It is not a promise for every n/d: 2147483647/2147483646
     * prints as 1.00000000046566128752, whose expansion may stop at another
     * fraction within int32.
     * Exact prices go through Price(n, d).
     */
    void testPrintedPricesReadBackAsTheSameFraction() {
        const QList<QPair<qint32, qint32>> fractions = {
            {1, 3}, {7, 9}, {3, 7}, {22, 7}, {355, 113}, {1, 2}, {5, 1}, {1, 10000000}
        };
        for (const auto& f : fractions) {
            const Price back(Price::toString(f.first, f.second));
            QVERIFY2(isPrice(back, f.first, f.second),
                     qPrintable(QStringLiteral("%1/%2 -> %3 -> %4/%5")
                                    .arg(f.first).arg(f.second)
                                    .arg(Price::toString(f.first, f.second))
                                    .arg(back.getNumerator()).arg(back.getDenominator())));
        }
    }

    /**
     * A decimal that is not an int32 fraction is approximated as java and js
     * approximate it, so the same text signs the same offer in every SDK.
     * The last exact convergent used here before was further from the value
     * in every one of these cases (87674244.96 was 87674245/1).
     */
    void testApproximatesAsJavaAndJs()
    {
        QVERIFY(isPrice(Price(QStringLiteral("87674244.96")), 2104181879, 24));
        QVERIFY(isPrice(Price(QStringLiteral("92458130.52")), 2126537002, 23));
        QVERIFY(isPrice(Price(QStringLiteral("86698736.24")), 1820673461, 21));
        QVERIFY(isPrice(Price(QStringLiteral("214.7483669")), 1702003429, 7925571));
        // And exact whenever the decimal is an int32 fraction.
        QVERIFY(isPrice(Price(QStringLiteral("214.7483647")), 2147483647, 10000000));
        QVERIFY(isPrice(Price(QStringLiteral("0.0000001")), 1, 10000000));
    }

    /** Price(n, d) takes qint32 terms and nothing that narrows into them. */
    void testExactTermsDoNotNarrow()
    {
        static_assert(std::is_constructible<Price, int, int>::value, "int terms");
        static_assert(!std::is_constructible<Price, double, int>::value, "1.5 was 1");
        static_assert(!std::is_constructible<Price, qint64, qint64>::value, "2^31 wrapped");
        static_assert(!std::is_constructible<Price, quint32, quint32>::value, "3e9 wrapped");
        QVERIFY(isPrice(Price(7, 9), 7, 9));
    }

    /**
     * Where js's rounded expansion is not the closest int32 fraction, the
     * price is still js's. Price(QString) used to keep the closer last exact
     * convergent there, so 0.40000774603453 (an ordinary JS number) signed
     * 844598183/2111454569 while js-stellar-sdk 17.1.0 signs
     * 648788396/1621939581: a different offer and hash for the same text.
     * Each expected pair is what js-stellar-sdk 17.1.0 puts in the XDR.
     */
    void testSignsWhatJsSignsWhenItIsNotTheClosest()
    {
        struct Case { const char* text; qint32 n; qint32 d; };
        const Case cases[] = {
            {"7.692E-7", 1442, 1874674987},
            {"0.40000774603453", 648788396, 1621939581},
            {"0.8611104900241705", 158210177, 183728080},
            {"0.94890784578760532", 1387826783, 1462551700},
            {"9.496302054607783667903", 811746796, 85480305},
            {"0.3643280041525207", 481323632, 1321127189},
            {"0.995823784524247535", 1104568885, 1109201148},
            {"0.896761962795432006549", 827222803, 922455275},
            {"0.8631239749369278964", 149798053, 173553345},
            {"0.48831382949261", 822773502, 1684927709},
            {"0.510942421828961", 757544919, 1482642440},
            {"0.30603956311414967", 277536928, 906866175},
            {"0.9205308593082151969", 630376109, 684796281},
            {"0.75121821185676", 968512158, 1289255429},
        };
        for (const Case& c : cases) {
            const Price p{QString::fromLatin1(c.text)};
            QVERIFY2(isPrice(p, c.n, c.d),
                     qPrintable(QStringLiteral("%1 -> %2/%3, js signs %4/%5")
                                    .arg(QLatin1String(c.text)).arg(p.getNumerator())
                                    .arg(p.getDenominator()).arg(c.n).arg(c.d)));
        }
        // An exact int32 fraction is still the fraction itself.
        QVERIFY(isPrice(Price(QStringLiteral("0.4")), 2, 5));
        QVERIFY(isPrice(Price(QStringLiteral("214.7483647")), 2147483647, 10000000));
    }

    /**
     * toString: a whole price has no trailing '.' ("2", as js and java print
     * it, used to be "2."), and INT32_MIN/-1, which a decoded envelope can
     * carry, is 2147483648: the 32-bit division used to kill the process
     * with STATUS_INTEGER_OVERFLOW.
     */
    void testToStringWholeAndExtremeTerms()
    {
        const qint32 min = std::numeric_limits<qint32>::min();
        QCOMPARE(Price::toString(min, -1), QStringLiteral("2147483648"));
        QCOMPARE(Price::toString(min, 1), QStringLiteral("-2147483648"));
        QCOMPARE(Price::toString(2147483647, -1), QStringLiteral("-2147483647"));
        QCOMPARE(Price::toString(2, 1), QStringLiteral("2"));
        QCOMPARE(Price::toString(10, 5), QStringLiteral("2"));
        QCOMPARE(Price::toString(10, 1), QStringLiteral("10"));
        QCOMPARE(Price::toString(2147483647, 1), QStringLiteral("2147483647"));
        QCOMPARE(Price::toString(0, 1), QStringLiteral("0"));
        QCOMPARE(Price::toString(-7, 1), QStringLiteral("-7"));
        QCOMPARE(Price(QStringLiteral("2")).toString(), QStringLiteral("2"));
        // Fractions keep their digits.
        QCOMPARE(Price::toString(1, 2), QStringLiteral("0.5"));
        QCOMPARE(Price::toString(5, 2), QStringLiteral("2.5"));
        QCOMPARE(Price::toString(2147483647, 2), QStringLiteral("1073741823.5"));
        QCOMPARE(Price::toString(1, 3), QStringLiteral("0.33333333333333333333"));
    }

    /** int64 terms into a Price: as given, else reduced, never wrapped. */
    void testFromInt64NeverWraps()
    {
        Price p(7, 9);
        QVERIFY(Price::fromInt64(2, 4, p));
        QVERIFY(isPrice(p, 2, 4));                          // fits: kept as sent
        QVERIFY(Price::fromInt64(4294967296LL, 2147483648LL, p));
        QVERIFY(isPrice(p, 2, 1));                          // reduced to fit
        QVERIFY(Price::fromInt64(2147483647, 1, p));
        QVERIFY(isPrice(p, 2147483647, 1));
        p = Price(7, 9);
        // 4294967297/4294967296 used to read as 1/0 through qint32.
        for (const auto& t : {qMakePair(4294967297LL, 4294967296LL),
                              qMakePair(37286884178LL, 822576135LL),
                              qMakePair(477LL, 964549061895LL),
                              qMakePair(2147483648LL, 1LL),
                              qMakePair(-1LL, 2LL), qMakePair(1LL, 0LL), qMakePair(1LL, -2LL),
                              qMakePair(std::numeric_limits<qint64>::max(), 1LL)}) {
            QVERIFY2(!Price::fromInt64(t.first, t.second, p),
                     qPrintable(QStringLiteral("%1/%2").arg(t.first).arg(t.second)));
            QVERIFY(isPrice(p, 7, 9));                      // untouched
        }
    }

    /** requirePositive: what the offer and deposit builders refuse. */
    void testRequirePositive()
    {
        QVERIFY(isPrice(Price(1, 2).requirePositive(), 1, 2));
        for (const Price& bad : {Price(), Price(0, 1), Price(1, 0), Price(-1, 2),
                                 Price(1, -2), Price(-1, -2),
                                 Price(std::numeric_limits<qint32>::min(), 1)}) {
            bool threw = false;
            try { bad.requirePositive("minPrice"); }
            catch (const std::runtime_error& e) {
                threw = QString::fromUtf8(e.what()) == QStringLiteral("minPrice must be positive");
            }
            QVERIFY2(threw, qPrintable(bad.toString()));
        }
    }

#ifdef STELLAR_QT_QML
    /**
     * Stellar.priceToString takes each term only when it is a whole int32,
     * from C++ and from real JavaScript. It used to take int and let QML
     * coerce: 1.9 showed as "1.", 2147483648 as "-2147483648.", NaN as "0.",
     * and priceToString(2147483648, -1) became INT32_MIN/-1 and killed the
     * process. A whole price reads "2" here and in parsePrice's text, not "2.".
     */
    void testQmlPriceToStringRefusesWhatIsNotAWholeInt32()
    {
        StellarQml s;
        QCOMPARE(s.priceToString(1, 2), QStringLiteral("0.5"));
        QCOMPARE(s.priceToString(2, 1), QStringLiteral("2"));
        QCOMPARE(s.parsePrice(QStringLiteral("2")).value(QStringLiteral("text")).toString(),
                 QStringLiteral("2"));
        QCOMPARE(s.priceToString(QVariant(2147483647.0), QVariant(1)), QStringLiteral("2147483647"));
        QCOMPARE(s.priceToString(QStringLiteral("3"), 1), QStringLiteral("3"));
        QCOMPARE(s.priceToString(QVariant(qint64(-2147483647 - 1)), -1), QStringLiteral("2147483648"));
        const QList<QPair<QVariant, QVariant>> refused = {
            {QVariant(1.9), QVariant(1)},
            {QVariant(2147483648.0), QVariant(1)},
            {QVariant(1), QVariant(4294967295.0)},
            {QVariant(qint64(4294967298LL)), QVariant(1)},
            {QVariant(std::numeric_limits<double>::quiet_NaN()), QVariant(1)},
            {QVariant(std::numeric_limits<double>::infinity()), QVariant(1)},
            {QVariant(true), QVariant(1)},
            {QVariant(QStringLiteral("abc")), QVariant(1)},
            {QVariant(QStringLiteral("1.5")), QVariant(1)},
            {QVariant(), QVariant(1)},
            {QVariant(1), QVariant(0)},                     // "inf" is not a price
            {QVariant(0), QVariant(0)},                     // nor is "nan"
        };
        for (const auto& r : refused)
            QVERIFY2(s.priceToString(r.first, r.second).isEmpty(),
                     qPrintable(r.first.toString() + QLatin1Char('/') + r.second.toString()));

        QJSEngine engine;
        QQmlEngine::setObjectOwnership(&s, QQmlEngine::CppOwnership);   // on the stack
        engine.globalObject().setProperty(QStringLiteral("Stellar"), engine.newQObject(&s));
        const QList<QPair<QString, QString>> js = {
            {QStringLiteral("Stellar.priceToString(1, 3)"), QStringLiteral("0.33333333333333333333")},
            {QStringLiteral("Stellar.priceToString(2, 1)"), QStringLiteral("2")},
            {QStringLiteral("Stellar.priceToString('7', '9')"), QStringLiteral("0.77777777777777777778")},
            {QStringLiteral("Stellar.priceToString(-2147483648, -1)"), QStringLiteral("2147483648")},
            {QStringLiteral("Stellar.priceToString(2147483648, -1)"), QString()},
            {QStringLiteral("Stellar.priceToString(-2147483648, 4294967295)"), QString()},
            {QStringLiteral("Stellar.priceToString(1.9, 1)"), QString()},
            {QStringLiteral("Stellar.priceToString(4294967298, 1)"), QString()},
            {QStringLiteral("Stellar.priceToString(NaN, 1)"), QString()},
            {QStringLiteral("Stellar.priceToString(Infinity, 1)"), QString()},
            {QStringLiteral("Stellar.priceToString(undefined, 1)"), QString()},
            {QStringLiteral("Stellar.priceToString(null, 1)"), QString()},
            {QStringLiteral("Stellar.priceToString(1, 0)"), QString()},
        };
        for (const auto& c : js) {
            const QJSValue v = engine.evaluate(c.first);
            QVERIFY2(!v.isError(), qPrintable(c.first + QStringLiteral(": ") + v.toString()));
            QVERIFY2(v.toString() == c.second,
                     qPrintable(c.first + QStringLiteral(" -> '") + v.toString() + QLatin1Char('\'')));
        }
    }
#endif

private:
    static bool isPrice(const Price& p, qint32 n, qint32 d) {
        return p.getNumerator() == n && p.getDenominator() == d;
    }
    static double relativeError(const Price& p, double expected) {
        const double got = double(p.getNumerator()) / double(p.getDenominator());
        return qAbs(got - expected) / expected;
    }
};

ADD_TEST(PriceTest)
#endif // PRICETEST_H
