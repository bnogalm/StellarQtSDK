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

    Price(stellar::Price& price);

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
    * Approximates <code>price</code> to a fraction.
    * @param price Ex. "1.25"
    */
    static Price* fromString(QString price);

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
