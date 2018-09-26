#include "util.h"

Util::Util()
{

}

QByteArray Util::hash(QByteArray data){
    return QCryptographicHash::hash(data,QCryptographicHash::Sha256);
}

QByteArray Util::paddedByteArray(QByteArray bytes, int length) {
    return bytes.leftJustified(length,'\0');
}

QByteArray Util::paddedByteArray(QString string, int length) {
    return paddedByteArray(string.toUtf8(), length);
}

QString Util::paddedByteArrayToString(QByteArray bytes) {
    return Util::removeTailChars(bytes,'\0');
}

QByteArray Util::removeTailChars(QByteArray bytes, const char c)
{
    int i=bytes.length()-1;
    for(;i>=0;i--){
        if(bytes[i]!=c){
            break;
        }
    }
    return bytes.left(i+1);
}

QString Util::removeTailChars(QString string, QChar c)
{
    int i=string.length()-1;
    for(;i>=0;i--){
        if(string[i]!=c){
            break;
        }
    }
    return string.left(i+1);
}

QString checkNotNull(QString p, const char *error)
{
    if(p.isNull()){
        throw std::runtime_error(error);
    }
    return p;
}

QByteArray checkNotNull(QByteArray p, const char* error)
{
    if(p.isNull()){
        throw std::runtime_error(error);
    }
    return p;
}

CheckHex::CheckHex(QString v)
{
    static QRegularExpression hexMatcher("^([0-9A-Fa-f][0-9A-Fa-f])*$",QRegularExpression::CaseInsensitiveOption);
    auto match = hexMatcher.globalMatch(v);
    if (!match.hasNext())
    {
        throw std::runtime_error("invalid hex string");
    }
    value = QByteArray::fromHex(v.toLatin1());
}

CheckHex::operator QByteArray()
{
    return value;
}

Integer::Integer(quint32 v):Optional<quint32>(v){}

Integer::Integer(stellar::Optional<quint32> optional){
    if(optional.filled){
        filled = optional.filled;
        value = optional.value;
    }
}

bool Integer::isNull() const{
    return !filled;
}

bool Integer::operator==(const Integer &other){
    return filled== other.filled && value==other.value;
}

Integer::operator quint32()
{
    return value;
}

bool operator==(const Integer &a, const Integer &b){
    return a.filled== b.filled && a.value==b.value;
}
