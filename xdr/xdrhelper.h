#ifndef XDRHELPER_H
#define XDRHELPER_H
#include <QtGlobal>
#include <QVector>
#include <QDataStream>
#include <QDebug>
#include <QIODevice>
#include <stdexcept>
#include <string>
#include <limits>   // std::numeric_limits used below as a default template arg;
                    // include it here so this widely-included header is
                    // self-sufficient regardless of include order (GCC's leaner
                    // libstdc++ no longer guarantees it transitively).


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

/*
 * An XDR bool, and the "present" flag of an optional `T *x`, is 0 or 1
 * (RFC 4506 §4.4, §4.19). Any other value used to read as true and was
 * written back as 1: bytes that decoded, and re-encoded as different bytes.
 * Now it marks the stream corrupt on the way in and fails the write on the
 * way out.
 */
inline bool isXdrBool(qint32 flag)
{
    return flag == 0 || flag == 1;
}

inline QDataStream &readXdrBool(QDataStream &in, bool &value)
{
    qint32 flag = 0;
    in >> flag;
    value = flag == 1;
    if (!isXdrBool(flag))
        in.setStatus(QDataStream::ReadCorruptData);
    return in;
}

inline QDataStream &writeXdrBool(QDataStream &out, bool value)
{
    out << qint32(value ? 1 : 0);
    return out;
}

template<class T>
QDataStream &operator<<(QDataStream &out, const  Optional<T> &obj) {
    if (!isXdrBool(obj.filled)) {       // neither absent nor present: nothing to write
        out.setStatus(QDataStream::WriteFailed);
        return out;
    }
    out << obj.filled;
    if(obj.filled)
        out << obj.value;
   return out;
}

template<class T>
QDataStream &operator>>(QDataStream &in,  Optional<T> &obj) {
    in >> obj.filled;
    if (!isXdrBool(obj.filled)) {       // see isXdrBool: 0 or 1 only
        obj.filled = 0;
        in.setStatus(QDataStream::ReadCorruptData);
        return in;
    }
    if(obj.filled)
        in >> obj.value;
   return in;
}

/*
 * A reserved `ext`: the union `switch (int v) { case 0: void; }`. Its only
 * arm is 0, so any other value is corrupt data on the way in and a failed
 * write on the way out; it used to be read and written back as whatever it
 * was. The extensions that do have arms (AccountEntry's v1, v2, v3 and the
 * like) read and write their discriminant themselves.
 */
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
    // Decoding replaces, never merges. And the count is checked against what
    // is left: the loop used to stop quietly at the end of the input, so an
    // envelope declaring two signatures and carrying one read as complete.
    obj.value.clear();
    const qint64 minElementSize = sizeof(T) == 1 ? 1 : 4;
    if (in.status() != QDataStream::Ok)
        return in;
    if (n > in.device()->bytesAvailable() / minElementSize) {
        in.setStatus(QDataStream::ReadPastEnd);
        return in;
    }
    obj.value.reserve(n);
    for (qint32 i = 0; i < n; ++i) {
        if (in.atEnd()) {
            in.setStatus(QDataStream::ReadPastEnd);
            return in;
        }
        T v;
        in >> v;
        if (in.status() != QDataStream::Ok)
            return in;          // a half-read element is not kept
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

/*
 * An unbounded XDR array (`T x<>`) held in a QVector: a uint32 count, then the
 * elements. Never QDataStream's own QVector/QList operators for this: they
 * reserve whatever count arrives (a 20-byte result asked for gigabytes and
 * threw std::bad_alloc), and since Qt 6.7 they read 0xFFFFFFFE as "a 64-bit
 * size follows", a framing XDR does not have.
 *
 * Every element of the XDR types read this way is at least four bytes, so a
 * count the rest of the input cannot hold is refused before anything is
 * allocated. Nothing is reserved up front either: an element in memory is
 * far larger than its four-byte minimum on the wire (a ClaimAtom is hundreds
 * of bytes), so the list grows only with what was actually read. Reading
 * stops at the first stream error.
 */
template <class T>
inline QDataStream &readXdrList(QDataStream &in, QVector<T> &list) {
    list.clear();
    quint32 n = 0;
    in >> n;
    if (in.status() != QDataStream::Ok)
        return in;
    if (in.device() && quint64(n) > quint64(in.device()->bytesAvailable()) / 4) {
        in.setStatus(QDataStream::ReadPastEnd);
        return in;
    }
    for (quint32 i = 0; i < n; ++i) {
        T v;
        in >> v;
        if (in.status() != QDataStream::Ok)
            break;
        list.append(v);
    }
    return in;
}

template <class T>
inline QDataStream &writeXdrList(QDataStream &out, const QVector<T> &list) {
    out << quint32(list.size());
    for (const T &v : list)
        out << v;
    return out;
}


/*
 * XDR_SERIALIZER is a raw memcpy and does NOT swap bytes. That is correct for a
 * fixed array of chars — a hash, an asset code, a signature hint — and wrong for
 * anything numeric, because XDR is big-endian and the machines this runs on are
 * not.
 *
 * It was applied to every result code enum for years and nothing caught it,
 * because the mistake is symmetric: the writer and the reader were wrong in the
 * same direction, so every round trip agreed with itself. What it never agreed
 * with was Stellar Core. The two codes anybody tested, txSUCCESS (0) and
 * txFAILED (-1), are the only ones whose bytes are palindromes, so the suite
 * stayed green while every code that says WHY something failed came back as a
 * large nonsense number.
 *
 * Use XDR_ENUM_SERIALIZER for an enum, XDR_CODE_STRUCT_SERIALIZER for the
 * single-member structs that wrap one, and XDR_SERIALIZER only for bytes.
 */

/** A four-byte enum, big-endian, as RFC 4506 requires. */
#define XDR_ENUM_SERIALIZER(Type) \
inline QDataStream& operator<<(QDataStream &out, const Type &t)\
{\
    out << static_cast<qint32>(t);\
    return out;\
}\
inline QDataStream& operator>>(QDataStream &in, Type &t)\
{\
    qint32 raw = 0;\
    in >> raw;\
    t = static_cast<Type>(raw);\
    return in;\
}

/**
 * A result code: a four-byte enum like any other, from a closed set. Each
 * result union declares an arm for every code from `Lowest` to `Highest` and
 * none for anything else (none of them has a `default:`), so a code outside
 * that range is corrupt data on the way in and a failed write on the way
 * out. It used to be read as a void arm, and whatever followed it was read
 * as the next field.
 */
#define XDR_RESULT_CODE_SERIALIZER(Type, Lowest, Highest) \
inline bool isKnownResultCode(Type t)\
{\
    const qint32 raw = static_cast<qint32>(t);\
    return raw >= static_cast<qint32>(Type::Lowest) && raw <= static_cast<qint32>(Type::Highest);\
}\
inline QDataStream& operator<<(QDataStream &out, const Type &t)\
{\
    if (!isKnownResultCode(t)) {\
        out.setStatus(QDataStream::WriteFailed);\
        return out;\
    }\
    out << static_cast<qint32>(t);\
    return out;\
}\
inline QDataStream& operator>>(QDataStream &in, Type &t)\
{\
    qint32 raw = 0;\
    in >> raw;\
    t = static_cast<Type>(raw);\
    if (!isKnownResultCode(t))\
        in.setStatus(QDataStream::ReadCorruptData);\
    return in;\
}

/**
 * The XDR of `value`, or an exception. A writer that meets something it
 * cannot encode (an arm it does not know, a flag that is neither 0 nor 1)
 * marks the stream failed, and whatever was written before that point is a
 * prefix of nothing. Bytes that are hashed, signed or sent are encoded
 * through here, so they are either whole or not produced at all.
 */
template <class T>
inline QByteArray encodeOrThrow(const T &value, const char *what)
{
    QByteArray bytes;
    QDataStream out(&bytes, QIODevice::WriteOnly);
    out << value;
    if (out.status() != QDataStream::Ok)
        throw std::runtime_error(std::string(what) + " cannot be encoded as XDR: "
                                 "it holds a value the format has no encoding for");
    return bytes;
}

/** A degenerate union: a struct whose only member is its result code. */
#define XDR_CODE_STRUCT_SERIALIZER(Type) \
inline QDataStream& operator<<(QDataStream &out, const Type &t)\
{\
    out << t.code;\
    return out;\
}\
inline QDataStream& operator>>(QDataStream &in, Type &t)\
{\
    in >> t.code;\
    return in;\
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
