#ifndef PRICE_H
#define PRICE_H
#include <QtCore>
#include "xdr/stellartransaction.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * Represents Price. Price in Stellar is represented as a fraction.
 */
class Price
{
    Q_GADGET
    qint32 m_n;
    qint32 m_d;

    Q_PROPERTY(qint32 n MEMBER m_n)
    Q_PROPERTY(qint32 d MEMBER m_d)
public:

    /**
    * Create a new price. Price in Stellar is represented as a fraction.
    * @param n numerator
    * @param d denominator
    */
    Price(qint32 n, qint32 d);

    /**
     * Only qint32 terms: any other argument types do not compile. A double, a
     * qint64 or a quint32 used to be narrowed silently into a different price
     * (1.5 became 1, 2147483648 became -2147483648). Convert explicitly, after
     * checking the range, or use Price(QString).
     */
    template <typename A, typename B>
    Price(A, B) = delete;

    Price(stellar::Price& price);

    /**
     * Reads a decimal price, e.g. "1.25", "0.0000012345" or "1.2345e-6".
     *
     * The text is read exactly — never through a double, never shortened. When
     * the decimal, reduced, is an n/d with both at most 2^31 - 1, that is the
     * price, exactly: every price Horizon prints (7 decimals) up to
     * 214.7483647 is one.
     *
     * When it is not, it has to be approximated, and the price is the one
     * java-stellar-sdk (Price.fromString) and js-stellar-base (best_r) sign
     * for the same text: their continued-fraction expansion, which rounds 1/f
     * half-up to 20 decimal places at each step, reproduced here in exact
     * integer arithmetic. So the same text gives the same offer and the same
     * transaction hash as in those SDKs, even for the few long decimals where
     * theirs is not the closest int32 fraction: "0.40000774603453" is
     * 648788396/1621939581 here and in js-stellar-sdk. py-stellar-sdk rounds
     * to 28 digits instead and can differ in the last term (it signs
     * 844598183/2111454569 for that text).
     *
     * Throws std::runtime_error for anything that is not a plain positive
     * decimal of at most 256 characters ("abc", "", "-1", "1,5", "nan", "0")
     * and for a value no int32 fraction can approximate: 2^31 and above, or
     * at most 1/2^31. There js-stellar-sdk 17.1.0 parts from this: its best_r
     * signs those as the extreme price, 2147483647/1 or 1/2147483647, so
     * "1e15" becomes an offer some 466,000 times lower. This refuses them,
     * as py-stellar-sdk does ("Couldn't find approximation").
     *
     * To set an exact fraction, use Price(n, d).
     */
    Price(QString price);

    Price();

    /**
    * Returns numerator.
    */
    Q_INVOKABLE qint32 getNumerator() const{
        return m_n;
    }

    /**
    * Returns denominator
    */
    Q_INVOKABLE qint32 getDenominator() const{
        return m_d;
    }

    /**
    * Same as Price(QString), on the heap.
    * @param price Ex. "1.25"
    */
    static Price* fromString(QString price);

    /**
     * n/d with int32 terms, for a fraction that arrives with int64 terms
     * (Horizon's trade prices): as given when both fit, otherwise divided by
     * their gcd. False, leaving `out` untouched, when even the reduced
     * fraction does not fit, or when n < 0 or d <= 0: the caller refuses
     * rather than wrap it into a different price.
     */
    static bool fromInt64(qint64 n, qint64 d, Price& out);

    /**
     * Returns *this, or throws std::runtime_error("<what> must be positive")
     * unless both terms are above zero. Core refuses an offer or a pool
     * deposit at such a price as malformed and js-stellar-sdk will not build
     * one; a default-constructed Price() is 0/0.
     */
    const Price& requirePositive(const char* what = "price") const;

    /** Decimal text for n/d: "0.5", "2", "0.3333333333333333". */
    /**
     * n/d as a decimal, exactly as js-stellar-sdk prints a decoded price
     * (BigNumber(n).div(d)): 20 decimals rounded half up, trailing zeros
     * dropped, exponential notation from 1e-7 down ("4.6566128752e-10").
     * The exact fraction is getNumerator()/getDenominator().
     */
    static QString toString(qint32 n,qint32 d);

    Q_INVOKABLE QString toString() const;
    /**
         * Generates Price XDR object.
         */
    stellar::Price toXdr() const;

    bool equals(Price* price);

    bool operator ==(const Price& price) const
    {
        return m_d == price.m_d && m_n == price.m_n;
    }
    bool operator !=(const Price& price) const
    {
        return !(*this == price);
    }
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(Price)
Q_DECLARE_METATYPE(qstellar::Price)

#endif // PRICE_H
