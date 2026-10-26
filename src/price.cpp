#include "price.h"
#include "util.h"

#include <numeric>
#include <limits>
#include <stdexcept>
#include <cmath>
#include <utility>
#include <algorithm>

QSTELLAR_BEGIN_NS

Price::Price(qint32 n, qint32 d) {
    m_n = n;
    m_d = d;
}

Price::Price(stellar::Price &price):m_n(price.n),m_d(price.d)
{

}

namespace {

const quint64 PRICE_TERM_MAX = static_cast<quint64>(std::numeric_limits<qint32>::max());

/**
 * Just enough unsigned big-integer arithmetic to run Euclid's algorithm on a
 * decimal exactly: little-endian 32-bit limbs, always normalised (no zero limb
 * at the top), so an empty vector is zero.
 *
 * It exists because a price typed as text has to be read as the number that
 * was typed. A double holds 15-17 significant digits and rounds the rest, and
 * the old way round that — keeping only the first 11 characters of the text —
 * turned "1.234567e-06" into "1.234567e-0", an offer a million times too
 * expensive. The inputs are bounded (see Price(QString)), so these numbers
 * never grow past a few hundred bits.
 */
class BigUInt
{
public:
    bool isZero() const { return m_limbs.isEmpty(); }

    /** *this = *this * mul + add. */
    void mulAdd(quint32 mul, quint32 add)
    {
        quint64 carry = add;
        for (quint32& limb : m_limbs) {
            const quint64 t = static_cast<quint64>(limb) * mul + carry;
            limb = static_cast<quint32>(t);
            carry = t >> 32;
        }
        if (carry)
            m_limbs.append(static_cast<quint32>(carry));
    }

    int compare(const BigUInt& other) const
    {
        if (m_limbs.size() != other.m_limbs.size())
            return m_limbs.size() < other.m_limbs.size() ? -1 : 1;
        for (int i = static_cast<int>(m_limbs.size()) - 1; i >= 0; --i) {
            if (m_limbs.at(i) != other.m_limbs.at(i))
                return m_limbs.at(i) < other.m_limbs.at(i) ? -1 : 1;
        }
        return 0;
    }

    /** *this << bits, for 0 <= bits < 32. */
    BigUInt shiftedLeft(int bits) const
    {
        BigUInt out;
        out.m_limbs.reserve(m_limbs.size() + 1);
        quint32 carry = 0;
        for (quint32 limb : m_limbs) {
            const quint64 t = (static_cast<quint64>(limb) << bits) | carry;
            out.m_limbs.append(static_cast<quint32>(t));
            carry = static_cast<quint32>(t >> 32);
        }
        out.m_limbs.append(carry);
        out.normalise();
        return out;
    }

    /** *this += other. */
    void add(const BigUInt& other)
    {
        quint64 carry = 0;
        const int n = std::max(m_limbs.size(), other.m_limbs.size());
        m_limbs.resize(n);
        for (int i = 0; i < n; ++i) {
            const quint64 t = static_cast<quint64>(m_limbs.at(i))
                              + (i < other.m_limbs.size() ? other.m_limbs.at(i) : 0u) + carry;
            m_limbs[i] = static_cast<quint32>(t);
            carry = t >> 32;
        }
        if (carry)
            m_limbs.append(static_cast<quint32>(carry));
        normalise();
    }

    int bitLength() const
    {
        if (m_limbs.isEmpty())
            return 0;
        int bits = (static_cast<int>(m_limbs.size()) - 1) * 32;
        for (quint32 top = m_limbs.last(); top; top >>= 1)
            ++bits;
        return bits;
    }

    bool testBit(int bit) const
    {
        const int limb = bit / 32;
        return limb < m_limbs.size() && ((m_limbs.at(limb) >> (bit % 32)) & 1u);
    }

    void setBit(int bit)
    {
        const int limb = bit / 32;
        if (m_limbs.size() <= limb)
            m_limbs.resize(limb + 1);
        m_limbs[limb] |= (1u << (bit % 32));
    }

    /** floor(*this / divisor), and the remainder. The divisor is not zero. */
    BigUInt dividedBy(const BigUInt& divisor, BigUInt& remainder) const
    {
        BigUInt quotient;
        remainder = BigUInt();
        for (int bit = bitLength() - 1; bit >= 0; --bit) {
            remainder = remainder.shiftedLeft(1);
            if (testBit(bit))
                remainder.mulAdd(1, 1);
            if (remainder.compare(divisor) >= 0) {
                remainder.subtract(divisor);
                quotient.setBit(bit);
            }
        }
        return quotient;
    }

    /** *this -= other. The caller guarantees *this >= other. */
    void subtract(const BigUInt& other)
    {
        qint64 borrow = 0;
        for (int i = 0; i < m_limbs.size(); ++i) {
            qint64 t = static_cast<qint64>(m_limbs.at(i)) - borrow
                       - (i < other.m_limbs.size() ? static_cast<qint64>(other.m_limbs.at(i)) : 0);
            borrow = t < 0 ? 1 : 0;
            if (t < 0)
                t += (static_cast<qint64>(1) << 32);
            m_limbs[i] = static_cast<quint32>(t);
        }
        normalise();
    }

private:
    void normalise()
    {
        while (!m_limbs.isEmpty() && m_limbs.last() == 0)
            m_limbs.removeLast();
    }

    QVector<quint32> m_limbs;
};

/**
 * quotient = floor(num / den) and num = num mod den, provided the quotient is
 * at most 2^31 - 1, the largest term a price can hold. Returns false, leaving
 * num untouched, when it would be larger: the continued fraction stops there
 * anyway, so there is never a reason to compute it.
 */
bool divideIntoPriceTerm(BigUInt& num, const BigUInt& den, quint32& quotient)
{
    if (num.compare(den.shiftedLeft(31)) >= 0)
        return false;
    quotient = 0;
    for (int bit = 30; bit >= 0; --bit) {
        const BigUInt shifted = den.shiftedLeft(bit);
        if (num.compare(shifted) >= 0) {
            num.subtract(shifted);
            quotient |= (1u << bit);
        }
    }
    return true;
}

/**
 * What java-stellar-sdk's Price.fromString and js-stellar-base's best_r
 * return for num/den: the same continued-fraction expansion, but with 1/f
 * rounded half-up to 20 decimal places at every step (BigDecimal.divide(f,
 * 20, HALF_UP); BigNumber with DECIMAL_PLACES 20, ROUND_HALF_UP), and the
 * last pair (h, k) that fits kept. Done here in exact integer arithmetic, so
 * it gives their fraction bit for bit. False when their expansion finds no
 * pair. Out of the int32 range the caller has refused the text already (js
 * 17.1 signs 2147483647/1 or 1/2147483647 there, py refuses); otherwise the
 * caller keeps its exact convergent.
 */
bool javaJsPrice(const BigUInt& num0, const BigUInt& den0, quint32& n, quint32& d)
{
    BigUInt num = num0, den = den0;      // the current number is num / den
    BigUInt tenPow20;
    tenPow20.mulAdd(1, 1);
    for (int i = 0; i < 4; ++i)
        tenPow20.mulAdd(100000, 0);
    quint64 h2 = 0, h1 = 1, k2 = 1, k1 = 0;
    for (int guard = 0; guard < 200; ++guard) {
        BigUInt rest = num;
        quint32 a = 0;
        if (!divideIntoPriceTerm(rest, den, a))
            break;                                   // number >= 2^31
        if (a == PRICE_TERM_MAX && !rest.isZero())
            break;                                   // number > 2^31 - 1
        const quint64 h = static_cast<quint64>(a) * h1 + h2;
        const quint64 k = static_cast<quint64>(a) * k1 + k2;
        if (h > PRICE_TERM_MAX || k > PRICE_TERM_MAX)
            break;
        h2 = h1; h1 = h;
        k2 = k1; k1 = k;
        if (rest.isZero())
            break;
        // number = round_half_up(den / rest, 20 places)
        //        = floor((2 * den * 10^20 + rest) / (2 * rest)) / 10^20
        BigUInt top = den;
        for (int i = 0; i < 4; ++i)
            top.mulAdd(100000, 0);
        top.mulAdd(2, 0);
        top.add(rest);
        BigUInt bottom = rest;
        bottom.mulAdd(2, 0);
        BigUInt unused;
        num = top.dividedBy(bottom, unused);
        den = tenPow20;
    }
    if (h1 == 0 || k1 == 0)
        return false;
    n = static_cast<quint32>(h1);
    d = static_cast<quint32>(k1);
    return true;
}

bool isAsciiDigit(QChar c)
{
    return c >= QLatin1Char('0') && c <= QLatin1Char('9');
}

[[noreturn]] void rejectPriceText()
{
    throw std::runtime_error("price is not a valid non-negative decimal number");
}

[[noreturn]] void rejectPriceRange()
{
    throw std::runtime_error("price cannot be represented as an int32 fraction");
}

} // namespace

Price::Price(QString price){
    checkNotNull(price, "price cannot be null");

    // ── 1. Read the text as exactly the number it spells ──────────────────
    //
    // [+]digits[.digits][(e|E)[+|-]digits], surrounding whitespace allowed.
    // No minus sign, no locale separators, no hex, no nan/inf: this value is
    // signed straight into an offer, so anything that is not plainly a
    // non-negative decimal is refused rather than guessed at. The length cap
    // bounds the arithmetic below; no real price comes anywhere near it.
    const QString text = price.trimmed();
    if (text.size() > 256)
        throw std::runtime_error("price text is too long (256 characters at most)");
    if (text.isEmpty())
        rejectPriceText();

    int i = 0;
    if (text.at(i) == QLatin1Char('+'))
        ++i;
    QString intDigits;
    while (i < text.size() && isAsciiDigit(text.at(i)))
        intDigits.append(text.at(i++));
    QString fracDigits;
    if (i < text.size() && text.at(i) == QLatin1Char('.')) {
        ++i;
        while (i < text.size() && isAsciiDigit(text.at(i)))
            fracDigits.append(text.at(i++));
    }
    if (intDigits.isEmpty() && fracDigits.isEmpty())
        rejectPriceText();

    qint64 exponent = 0;
    if (i < text.size() && (text.at(i) == QLatin1Char('e') || text.at(i) == QLatin1Char('E'))) {
        ++i;
        bool negative = false;
        if (i < text.size() && (text.at(i) == QLatin1Char('+') || text.at(i) == QLatin1Char('-'))) {
            negative = text.at(i) == QLatin1Char('-');
            ++i;
        }
        const int exponentStart = i;
        while (i < text.size() && isAsciiDigit(text.at(i))) {
            // Clamped rather than allowed to wrap: anything this far out is
            // refused by the magnitude check below.
            if (exponent < 1000000)
                exponent = exponent * 10 + (text.at(i).unicode() - '0');
            ++i;
        }
        if (i == exponentStart)
            rejectPriceText();
        if (negative)
            exponent = -exponent;
    }
    if (i != text.size())
        rejectPriceText();

    // value = int(digits) * 10^(exponent - fracDigits.size())
    const QString digits = intDigits + fracDigits;
    int first = 0;
    while (first < digits.size() && digits.at(first) == QLatin1Char('0'))
        ++first;
    // Zero, however it was written, is not a price: core refuses an offer or a
    // pool deposit at 0 as malformed, and js and python refuse it too.
    if (first == digits.size())
        throw std::runtime_error("price must be greater than zero");

    // ── 2. Refuse what no int32 fraction can hold ─────────────────────────
    //
    // The decimal place of the leading digit: the value lies in
    // [10^place, 10^(place+1)). A price is n/d with both at most 2^31 - 1, so
    // nothing at or above 10^10 fits, and nothing positive below 10^-10 comes
    // within reach of the smallest price, 1/(2^31 - 1). The exact boundaries
    // are found by the expansion itself; this only keeps the numbers small.
    const qint64 place = (first < intDigits.size())
                             ? static_cast<qint64>(intDigits.size() - 1 - first) + exponent
                             : -static_cast<qint64>(first - intDigits.size() + 1) + exponent;
    if (place >= 10 || place <= -11)
        rejectPriceRange();

    BigUInt numerator;
    for (int k = first; k < digits.size(); ++k)
        numerator.mulAdd(10, static_cast<quint32>(digits.at(k).unicode() - '0'));
    BigUInt denominator;
    denominator.mulAdd(1, 1);
    const qint64 scale = static_cast<qint64>(fracDigits.size()) - exponent;
    for (qint64 k = 0; k < scale; ++k)
        denominator.mulAdd(10, 0);
    for (qint64 k = scale; k < 0; ++k)
        numerator.mulAdd(10, 0);

    // ── 3. The fraction: exact when it fits, else what java and js sign ────
    //
    // First the exact continued-fraction expansion. When the decimal is itself
    // an n/d with both at most 2^31 - 1 it ends on that fraction, and that is
    // the price, exactly.
    const BigUInt value = numerator, scaleDen = denominator;
    quint64 h2 = 0, h1 = 1;     // numerators of the previous two convergents
    quint64 k2 = 1, k1 = 0;     // denominators of the previous two convergents
    bool anyTerm = false;
    bool exact = false;
    while (true) {
        quint32 term = 0;
        if (!divideIntoPriceTerm(numerator, denominator, term))
            break;
        const quint64 h = static_cast<quint64>(term) * h1 + h2;
        const quint64 k = static_cast<quint64>(term) * k1 + k2;
        if (h > PRICE_TERM_MAX || k > PRICE_TERM_MAX)
            break;
        h2 = h1; h1 = h;
        k2 = k1; k1 = k;
        anyTerm = true;
        if (numerator.isZero()) {
            exact = true;               // the expansion has ended on the value
            break;
        }
        std::swap(numerator, denominator);
    }

    // No convergent at all means the whole part alone is above 2^31 - 1; a
    // zero numerator for a value known to be positive means it is below the
    // smallest price there is. Either way the only honest answer is none.
    if (!anyTerm || h1 == 0)
        rejectPriceRange();
    m_n = static_cast<qint32>(h1);
    m_d = static_cast<qint32>(k1);
    if (exact)
        return;

    // Not an int32 fraction, so it is approximated. The price is the one
    // java-stellar-sdk and js-stellar-base sign for this text (their rounded
    // expansion, reproduced exactly), so the same text gives the same offer
    // and the same transaction hash in those SDKs. That holds even when the
    // last exact convergent above is closer (0.40000774603453: theirs is
    // 648788396/1621939581, the convergent 844598183/2111454569): keeping the
    // closer one signed a different offer than js for the same text. The
    // convergent is only the answer when their algorithm finds none.
    quint32 jn = 0, jd = 0;
    if (javaJsPrice(value, scaleDen, jn, jd)) {
        m_n = static_cast<qint32>(jn);
        m_d = static_cast<qint32>(jd);
    }
}

Price::Price():m_n(0),m_d(0)
{

}



//just to keep similar API
Price *Price::fromString(QString price) {
    return new Price(price);
}

bool Price::fromInt64(qint64 n, qint64 d, Price &out)
{
    if (n < 0 || d <= 0)
        return false;
    const qint64 max = std::numeric_limits<qint32>::max();
    if (n > max || d > max) {
        // The same value in smaller terms, when it has them. Never wrapped:
        // 4294967297/4294967296 used to come out as 1/0.
        const qint64 g = std::gcd(n, d);
        n /= g;
        d /= g;
        if (n > max || d > max)
            return false;
    }
    out = Price(static_cast<qint32>(n), static_cast<qint32>(d));
    return true;
}

const Price &Price::requirePositive(const char *what) const
{
    if (m_n <= 0 || m_d <= 0)
        throw std::runtime_error(std::string(what) + " must be positive");
    return *this;
}

QString Price::toString(qint32 n, qint32 d)
{
    // n/d as js-stellar-sdk prints a decoded price (BigNumber(n).div(d):
    // exact, 20 decimals rounded half up, trailing zeros dropped, and
    // exponential notation from 1e-7 down), computed with integers. It went
    // through a double printed to 16 digits: 36% of real offers read another
    // string than js's, and some a wrong last digit (1/7 as ...1428, not
    // ...1429; 1/2147483647 as 0.0000000004656613).
    if (d == 0)
        return n > 0 ? QStringLiteral("Infinity") : n < 0 ? QStringLiteral("-Infinity") : QStringLiteral("NaN");
    if (n == 0)
        return QStringLiteral("0");
    const bool negative = (n < 0) != (d < 0);
    const quint64 a = n < 0 ? quint64(-qint64(n)) : quint64(n);
    const quint64 b = d < 0 ? quint64(-qint64(d)) : quint64(d);
    quint64 integer = a / b;
    quint64 remainder = a % b;
    const int places = 20;
    QByteArray fraction(places, '0');
    for (int i = 0; i < places; ++i) {
        remainder *= 10;
        fraction[i] = char('0' + remainder / b);
        remainder %= b;
    }
    // Half up: the 21st digit decides (a tie rounds away from zero).
    remainder *= 10;
    if (remainder / b >= 5) {
        int i = places - 1;
        for (; i >= 0; --i) {
            if (fraction.at(i) == '9') {
                fraction[i] = '0';
            } else {
                fraction[i] = char(fraction.at(i) + 1);
                break;
            }
        }
        if (i < 0)
            ++integer;
    }
    while (fraction.endsWith('0'))
        fraction.chop(1);
    const QString sign = negative ? QStringLiteral("-") : QString();
    if (integer == 0 && !fraction.isEmpty()) {
        int zeros = 0;
        while (zeros < fraction.size() && fraction.at(zeros) == '0')
            ++zeros;
        const int exponent = -(zeros + 1);
        if (exponent <= -7) {
            const QByteArray digits = fraction.mid(zeros);
            QString coefficient = QString::fromLatin1(digits.left(1));
            if (digits.size() > 1)
                coefficient += QLatin1Char('.') + QString::fromLatin1(digits.mid(1));
            return sign + coefficient + QStringLiteral("e") + QString::number(exponent);
        }
    }
    QString out = sign + QString::number(integer);
    if (!fraction.isEmpty())
        out += QLatin1Char('.') + QString::fromLatin1(fraction);
    return out;
}

QString Price::toString() const{
    return Price::toString(m_n,m_d);
}

stellar::Price Price::toXdr() const{
    stellar::Price xdr;
    xdr.d=m_d;
    xdr.n=m_n;
    return xdr;
}

bool Price::equals(Price *price) {
    return this->getNumerator() == price->getNumerator() &&
            this->getDenominator() == price->getDenominator();

}

QSTELLAR_END_NS
