#ifndef PRICE_H
#define PRICE_H
#include <QtCore>
#include "xdr/stellartransaction.h"
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
    qint32 getNumerator() const{
        return m_n;
    }

    /**
    * Returns denominator
    */
    qint32 getDenominator() const{
        return m_d;
    }

    /**
    * Approximates <code>price</code> to a fraction.
    * @param price Ex. "1.25"
    */
    static Price* fromString(QString price);

    static QString toString(qint32 n,qint32 d);

    QString toString() const;
    /**
         * Generates Price XDR object.
         */
    stellar::Price toXdr();

    bool equals(Price* price);

    bool operator !=(Price& price)
    {
        return (price.m_d!=this->m_d)
                || (price.m_n !=this->m_n)
                || (price.toString()!=this->toString());
    }
};
Q_DECLARE_METATYPE(Price)
#endif // PRICE_H
