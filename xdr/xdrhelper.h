#ifndef XDRHELPER_H
#define XDRHELPER_H
#include <QtGlobal>
#include <QVector>
#include <QDataStream>
#include <QDebug>
#include <QIODevice>
#include <stdexcept>


namespace xdr{
/**
 *  Template helper class to define xdr optional fields, it's designed to set filled to 1 on asignations
 *  It also has an alternative way to fill the value, using filler() so you don't need to make copies.
 */
template<class T>
struct Optional
{

    Optional<T>():filled(0)
    {
    }

    Optional<T>(T v):filled(1)
    {
        value = v;
    }
    void clear()
    {
        filled=0;
    }
    operator T()
    {
        if(filled)
            return value;
        else
            return T();
    }
    /**
     * @brief filler sets filled to true and returns a reference to value, so you can fill it. Usefull if value is a struct with more fields
     * @return value reference
     */
    T& filler()
    {
        filled=1;
        return value;
    }

    qint32 filled;
    T value;
};

template<class T>
QDataStream &operator<<(QDataStream &out, const  Optional<T> &obj) {
    out << obj.filled;
    if(obj.filled)
        out << obj.value;
   return out;
}

template<class T>
QDataStream &operator>>(QDataStream &in,  Optional<T> &obj) {
    in >> obj.filled;
    if(obj.filled)
        in >> obj.value;
   return in;
}

struct alignas(4) Reserved {
    qint32 reserved;
    Reserved():reserved(0){}
};

QDataStream &operator<<(QDataStream &out, const  Reserved &obj);

QDataStream &operator>>(QDataStream &in,  Reserved &obj);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
QDataStream &operator<<(QDataStream &out, const  char c);

QDataStream &operator>>(QDataStream &in,  char& c);
#endif
template <class T, int max=std::numeric_limits<int>::max()>
struct Array{
    QVector<T> value;
    static int maxSize(){
        return max;
    }

    void append(const T& v){
        if(value.length()<max)
            value.append(v);
    }
    void clear()
    {
        value.clear();
    }

    /**
     * @brief set sets array to value from C style array, data will be copied
     * @param v pointer to c style array
     * @param amount n elements to copy, it will be trimmed to max size defined, if defined
     */
    void set(T* v, int amount)
    {
        // FIX §3.3: old code had two bugs — (a) memcpy copied `allowed`
        // bytes instead of `allowed * sizeof(T)` (only worked for sizeof(T)==1),
        // and (b) the size formula suggested append but memcpy overwrote
        // from the start. New semantics: replace with the first `amount`
        // elements, truncating to `max`.
        int allowed = qMin(amount, max);
        value.resize(allowed);
        if (allowed > 0)
            memcpy(value.data(), v, static_cast<size_t>(allowed) * sizeof(T));
    }
    //returns continuos data
    QByteArray binary() const
    {
        QByteArray ba(sizeof(T) * value.size(), Qt::Uninitialized);
        // FIX §3.4: was `#elif` with no condition → on Qt 5 `stream` was
        // never declared and the code didn't compile. Use `#else`.
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QDataStream stream(&ba, QIODeviceBase::WriteOnly);
#else
        QDataStream stream(&ba, QIODevice::WriteOnly);
#endif
        for (int i = 0; i < value.size(); i++) {
            stream << static_cast<T>(value[i]);
        }
        return ba;
    }
};

template <class T, int max=std::numeric_limits<int>::max()>
inline QDataStream &operator<<(QDataStream &out, const  Array<T,max> &obj) {
    auto pos = out.device()->pos();
    out << static_cast<qint32>(obj.value.length());
    for(int i=0;i<obj.value.length();i++)
        out << static_cast<T>(obj.value[i]);
    auto diff = out.device()->pos() - pos;
    qint32 missingBytes = 4-(diff&3);
    if(missingBytes<4){
        qint32 zero=0;
        out.writeRawData(reinterpret_cast<char*>(&zero),missingBytes);
    }

   return out;
}

template <class T, int max=std::numeric_limits<int>::max()>
inline QDataStream &operator>>(QDataStream &in,  Array<T,max> &obj) {
    qint32 n;
    auto pos = in.device()->pos();
    in >> n;
    // FIX §3.5: previously read N elements with no max check. A hostile
    // stream could request GB allocations and OOM the app. Reject upfront.
    if (n < 0 || n > max) {
        throw std::runtime_error("xdr array length out of bounds");
    }
    obj.value.reserve(n);
    while (n > 0 && !in.atEnd()) {
        n--;
        T v;
        in >> v;
        obj.value.append(v);
    }
    auto diff = in.device()->pos() - pos;
    int missingBytes = 4 - (diff & 3);
    if (missingBytes < 4) {
        // FIX §3.6: previous code read into a qint32 and compared as int —
        // worked by accident on little-endian, false-positive on big-endian.
        // Compare byte-by-byte now.
        char pad[4] = {0, 0, 0, 0};
        in.readRawData(pad, missingBytes);
        for (int i = 0; i < missingBytes; ++i) {
            if (pad[i] != 0)
                throw std::runtime_error("padding must be zero");
        }
    }
    return in;
}


//use only for fixed array of chars, as it will not take care of byteorder
#define XDR_SERIALIZER(Type) \
inline typename std::enable_if<std::is_standard_layout<Type>::value,QDataStream&>::type \
operator<<(QDataStream &out, const Type &t)\
{\
    out.writeRawData(reinterpret_cast<const char*>(&t),sizeof(Type));\
    return out;\
}\
inline typename std::enable_if<std::is_standard_layout<Type>::value,QDataStream&>::type \
operator>>(QDataStream &in, Type &t)\
{\
    in.readRawData(reinterpret_cast<char*>(&t),sizeof(Type));\
    return in;\
}




}
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    //since Qt5.14 this is defined already inside QDataStream
#else

template<typename Type>
inline typename std::enable_if<std::is_enum<Type>::value,QDataStream&>::type
    operator<<(QDataStream &out, const Type& t)
{
    out << static_cast<typename std::underlying_type<Type>::type>(t);
    return out;
}
template<typename Type>
inline typename std::enable_if<std::is_enum<Type>::value,QDataStream&>::type
    operator>>(QDataStream &in, Type &t)
{
    typename std::underlying_type<Type>::type underlying;
    in >> underlying;
    t= static_cast<Type>(underlying);//we could do it directly
    return in;
}

#endif

#endif // XDRHELPER_H
