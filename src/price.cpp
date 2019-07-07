#include "price.h"
#include "util.h"



Price::Price(qint32 n, qint32 d) {
    m_n = n;
    m_d = d;
}

Price::Price(stellar::Price &price):m_n(price.n),m_d(price.n)
{

}

//qint64 gcd(qint64 a, qint64 b)
//{
//    if (a == 0)
//        return b;
//    else if (b == 0)
//        return a;

//    if (a < b)
//        return gcd(a, b % a);
//    else
//        return gcd(b, a % b);
//}

qint64 gcd(qint64 a, qint64 b)
{
    while(b) b ^= a ^= b ^= a %= b;
    return a;
}

Price::Price(QString price){
    checkNotNull(price, "price cannot be null");
    QString rounded = price.left(11);//we can get better precision with this method, so we round the string
    double value = rounded.toDouble();//here we get 52 bit precision
    double integral = std::floor(value);
    double frac = value - integral;

    const qint64 precision = 1000000000; // This is the accuracy.

    qint64 gcd_ = gcd(round(frac * precision), precision);

    qint64 denominator = precision / gcd_;
    qint64 numerator = round(frac * precision) / gcd_;
    m_n = numerator+(integral*denominator);
    m_d = denominator;
}

Price::Price():m_n(0),m_d(0)
{

}



//just to keep similar API
Price *Price::fromString(QString price) {
    return new Price(price);
}

QString Price::toString(qint32 n, qint32 d)
{
    QString price;
    //we print too many decimals when integer part is not zero
    //this match java sdk decimals
    if(d!=0){
        qint32 integer= n/d;
        if(integer!=0)
        {
            price = QString::number(integer);
            int size=price.size();
            price = QString::number(static_cast<double>(n)/static_cast<double>(d),'f',16-size);
        }
        else
            price = QString::number(static_cast<double>(n)/static_cast<double>(d),'f',16);
    }
    else{
        price = QString::number(static_cast<double>(n)/static_cast<double>(d),'f',16);
    }
    if(price.contains('.')){
        price = Util::removeTailChars(price,'0');
    }
    return price;
}

QString Price::toString() const{
    return Price::toString(m_n,m_d);
}

stellar::Price Price::toXdr(){
    stellar::Price xdr;
    xdr.d=m_d;
    xdr.n=m_n;
    return xdr;
}

bool Price::equals(Price *price) {
    return this->getNumerator() == price->getNumerator() &&
            this->getDenominator() == price->getDenominator();

}
