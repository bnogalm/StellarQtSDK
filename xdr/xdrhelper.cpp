
#include "xdrhelper.h"

QDataStream &xdr::operator<<(QDataStream &out, const xdr::Reserved &obj) {
    out << obj.reserved;
    return out;
}

QDataStream &xdr::operator>>(QDataStream &in, xdr::Reserved &obj) {
    in >> obj.reserved;
    return in;
}
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
QDataStream &xdr::operator<<(QDataStream &out, const char c)
{
    out.writeRawData(&c,1);
    return out;
}

QDataStream &xdr::operator>>(QDataStream &in, char& c)
{
    in.readRawData(&c,1);
    return in;
}
#endif
