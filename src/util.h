#ifndef UTIL_H
#define UTIL_H
#include <QString>
#include <QCryptographicHash>
#include <QList>
#include <QRegularExpression>
#include "xdr/stellartransaction.h"
QString checkNotNull(QString p, const char* error);
QByteArray checkNotNull(QByteArray p, const char* error);


#define XDR_BASE64ENCODING QByteArray::Base64Encoding

template<typename Type>
inline typename std::enable_if<std::is_integral<Type>::value,Type>::type
checkNotNull(Type p, const char* error){
    if(p==0){
        throw std::runtime_error(error);
    }
    return p;
}


template<typename Type>
inline QVector<Type> checkNotNull(QVector<Type> &p, const char *error)
{
    if(p.isEmpty()){
        throw std::runtime_error(error);
    }
    return p;
}


#define checkArgument(EXP,error)\
    if(!(EXP)) throw std::runtime_error(error)

class Util
{
public:
    Util();
    static QString bytesToHex(QByteArray bytes)
    {
        return QString::fromLatin1(bytes.toHex());
    }
    static QByteArray hexToBytes(QString s){
        return QByteArray::fromHex(s.toLatin1());
    }
    /**
     * Returns SHA-256 hash of <code>data</code>.
     * @param data
     */
    static QByteArray hash(QByteArray data);

    /**
     * Pads <code>bytes</code> array to <code>length</code> with zeros.
     * @param bytes
     * @param length
     */
    static QByteArray paddedByteArray(QByteArray bytes, int length);

    /**
     * Pads <code>string</code> to <code>length</code> with zeros.
     * @param string
     * @param length
     */
    static QByteArray paddedByteArray(QString string, int length);

    /**
     * Remove zeros from the end of <code>bytes</code> array.
     * @param bytes
     */
    static QString paddedByteArrayToString(QByteArray bytes);
    static QByteArray removeTailChars(QByteArray bytes, const char c);
    static QString removeTailChars(QString string, QChar c);
};


/**
 * @brief The CheckHex class
 * Helper class to check hex parameters
 */
class CheckHex{

public:
    QByteArray value;
    CheckHex(QString v);
    operator QByteArray();
};



/**
 * @brief The Integer struct
 * helper struct to be able to have unassigned value as in java Integer type
 */

struct Integer: public stellar::Optional<quint32>
{
    Integer(){}
    Integer(quint32 v);
    Integer(stellar::Optional<quint32> optional);
    operator quint32();
    bool isNull() const;
    bool operator==(const Integer &other);
};
bool operator==(const Integer &a, const Integer &b);

#endif // UTIL_H
