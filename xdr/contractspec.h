#ifndef QSTELLAR_XDR_CONTRACTSPEC_H
#define QSTELLAR_XDR_CONTRACTSPEC_H

#include <QByteArray>
#include <QDataStream>
#include <QList>
#include <QSharedPointer>
#include <stdexcept>

namespace stellar {

/*
 * Stellar-contract-spec.x — the machine-readable interface a compiled contract
 * carries inside itself, in its `contractspecv0` WebAssembly section.
 *
 * Transcribed from stellar-xdr branch `curr`, commit c40231c7 (which carries
 * stellar-xdr #312, raising type and event names to 1024 bytes). Branch `curr`
 * and not `next`: `next` carries material for protocols that have not shipped,
 * and this file has been burned by that distinction before.
 *
 * Four things here are easy to get wrong from memory, and all four have been
 * checked against the canonical file rather than assumed:
 *
 *   - SCSpecType's values are NOT sequential. 15 and 18 do not exist, and
 *     neither does 1003. Every value below is written literally for that
 *     reason; deriving them by counting produces a decoder that is wrong for
 *     every type after the first gap.
 *
 *   - The name limits are inconsistent by container: 1024 for the name of a
 *     type (struct, union, enum, error enum, a UDT reference) and of an event,
 *     60 for a union or enum case, 30 for a field or a parameter, 80 for a
 *     `lib`, 1024 for a `doc`, and 32 for a function name and an event's
 *     prefix topics, which are SCSymbols rather than strings. Type and event
 *     names were 60 and 32 before #312; case names were NOT raised with them.
 *
 *   - SCSpecFunctionV0::outputs is `SCSpecTypeDef outputs<1>` — a variable
 *     length array capped at one, NOT an optional. The bytes coincide for the
 *     zero and one cases, so an optional-shaped implementation appears to
 *     work; it just cannot reject a count of two.
 *
 *   - SCSpecTypeDef is mutually recursive and the XDR sets no depth bound, so
 *     a hostile spec can nest until the stack gives out. The guard below is
 *     not defensive tidiness: these bytes arrive inside a file the user was
 *     handed by someone else.
 */

/** Longest doc comment an entry may carry. */
const qint32 SC_SPEC_DOC_LIMIT = 1024;

/** A field or parameter name; shorter than every other name in this file. */
const qint32 SC_SPEC_NAME_LIMIT = 30;

/**
 * The name of a type — struct, union, enum, error enum, a UDT reference — and
 * of an event: SC_SPEC_TYPE_NAME_LIMIT in the XDR. It was 60 (and an event
 * name an SCSymbol of 32) until stellar-xdr #312, so a contract whose type or
 * event names were longer was refused as malformed.
 */
const qint32 SC_SPEC_TYPE_NAME_LIMIT = 1024;

/** A union case or an (error) enum case: still `name<60>` in the XDR. */
const qint32 SC_SPEC_CASE_NAME_LIMIT = 60;

/** The library a user-defined type came from. */
const qint32 SC_SPEC_LIB_LIMIT = 80;

/** SCSymbol, from Stellar-contract.x: function names and event prefix topics. */
const qint32 SC_SYMBOL_LIMIT = 32;

/** A tuple carries at most twelve members. */
const qint32 SC_SPEC_TUPLE_LIMIT = 12;

/** An event may prefix at most two topics. */
const qint32 SC_SPEC_EVENT_PREFIX_TOPIC_LIMIT = 2;

/**
 * How deep a type may nest before we call the file malformed.
 *
 * The XDR imposes no limit at all, and SCSpecTypeDef contains itself through
 * option, result, vec, map and tuple. Sixteen is far past anything a real
 * contract signature reaches and far short of what would exhaust a stack.
 */
const int MAX_SPEC_TYPE_DEPTH = 16;

/** Bound on any unbounded `<>` array, so a bad count cannot ask for a huge one. */
const qint32 MAX_SPEC_ARRAY = 4096;

inline int& specTypeDepth()
{
    static thread_local int depth = 0;
    return depth;
}

struct SpecTypeDepthGuard
{
    SpecTypeDepthGuard() {
        if (++specTypeDepth() > MAX_SPEC_TYPE_DEPTH) {
            --specTypeDepth();      // no destructor runs when a ctor throws
            throw std::runtime_error("SCSpecTypeDef: type nesting too deep");
        }
    }
    ~SpecTypeDepthGuard() { --specTypeDepth(); }
};

// ─── XDR string, the one shape every name in this file uses ──────────
//
// A 4-byte length, the bytes, then zero padding up to a 4-byte boundary. Not
// null-terminated, and the `<N>` is a maximum rather than a fixed size — a
// shorter string is legal and is not padded out to N.

/**
 * Reads a 4-byte discriminant, and refuses to invent one.
 *
 * QDataStream does not throw when it runs off the end: it sets a status and
 * hands back a ZERO. Zero is SC_SPEC_TYPE_VAL and SC_SPEC_ENTRY_FUNCTION_V0 —
 * both perfectly valid — so a decoder that skips this check does not fail on a
 * truncated file, it silently invents a plausible type and carries on. That is
 * strictly worse than an error, because the caller believes the result.
 */
inline qint32 readSpecTag(QDataStream& in, const char* what)
{
    qint32 raw = 0;
    in >> raw;
    if (in.status() != QDataStream::Ok)
        throw std::runtime_error(std::string(what) + ": truncated");
    return raw;
}

inline void writeSpecString(QDataStream& out, const QByteArray& value)
{
    out << static_cast<qint32>(value.size());
    if (!value.isEmpty())
        out.writeRawData(value.constData(), value.size());
    const int pad = (4 - (value.size() % 4)) % 4;
    if (pad) { char zero[4] = {0, 0, 0, 0}; out.writeRawData(zero, pad); }
}

/**
 * Writes a string that must fit its field. The writer used to take any length,
 * so a name every reader (this one and stellar-xdr's) refuses went out as if
 * it were well-formed.
 */
inline void writeSpecString(QDataStream& out, const QByteArray& value, qint32 limit,
                            const char* what)
{
    if (value.size() > limit)
        throw std::runtime_error(std::string(what) + ": length out of bounds");
    writeSpecString(out, value);
}

inline void readSpecString(QDataStream& in, QByteArray& value, qint32 limit,
                           const char* what)
{
    const qint32 n = readSpecTag(in, what);
    if (n < 0 || n > limit)
        throw std::runtime_error(std::string(what) + ": length out of bounds");
    value.resize(n);
    if (n > 0 && in.readRawData(value.data(), n) != n)
        throw std::runtime_error(std::string(what) + ": truncated");
    // The padding is zeros, and stellar-xdr refuses anything else. It used to
    // be read and thrown away unchecked, so bytes it rejects decoded here.
    const int pad = (4 - (n % 4)) % 4;
    if (pad) {
        char padding[4] = {0, 0, 0, 0};
        if (in.readRawData(padding, pad) != pad)
            throw std::runtime_error(std::string(what) + ": truncated");
        for (int i = 0; i < pad; ++i) {
            if (padding[i] != 0)
                throw std::runtime_error(std::string(what) + ": non-zero padding");
        }
    }
}

/** Reads an array count, refusing one that could not possibly be there. */
inline qint32 readSpecCount(QDataStream& in, qint32 limit, const char* what)
{
    const qint32 n = readSpecTag(in, what);
    if (n < 0 || n > limit)
        throw std::runtime_error(std::string(what) + ": count out of bounds");
    return n;
}

// ─── SCSpecType ──────────────────────────────────────────────────────

enum class SCSpecType : qint32
{
    SC_SPEC_TYPE_VAL = 0,

    // Types with no parameters. Note the gaps at 15 and 18.
    SC_SPEC_TYPE_BOOL = 1,
    SC_SPEC_TYPE_VOID = 2,
    SC_SPEC_TYPE_ERROR = 3,
    SC_SPEC_TYPE_U32 = 4,
    SC_SPEC_TYPE_I32 = 5,
    SC_SPEC_TYPE_U64 = 6,
    SC_SPEC_TYPE_I64 = 7,
    SC_SPEC_TYPE_TIMEPOINT = 8,
    SC_SPEC_TYPE_DURATION = 9,
    SC_SPEC_TYPE_U128 = 10,
    SC_SPEC_TYPE_I128 = 11,
    SC_SPEC_TYPE_U256 = 12,
    SC_SPEC_TYPE_I256 = 13,
    SC_SPEC_TYPE_BYTES = 14,
    SC_SPEC_TYPE_STRING = 16,
    SC_SPEC_TYPE_SYMBOL = 17,
    SC_SPEC_TYPE_ADDRESS = 19,
    SC_SPEC_TYPE_MUXED_ADDRESS = 20,

    // Types with parameters. Note the gap at 1003.
    SC_SPEC_TYPE_OPTION = 1000,
    SC_SPEC_TYPE_RESULT = 1001,
    SC_SPEC_TYPE_VEC = 1002,
    SC_SPEC_TYPE_MAP = 1004,
    SC_SPEC_TYPE_TUPLE = 1005,
    SC_SPEC_TYPE_BYTES_N = 1006,

    // User defined types.
    SC_SPEC_TYPE_UDT = 2000
};

/** True for the arms that carry nothing but their discriminant. */
inline bool isVoidSpecType(SCSpecType type)
{
    switch (type) {
    case SCSpecType::SC_SPEC_TYPE_VAL:
    case SCSpecType::SC_SPEC_TYPE_BOOL:
    case SCSpecType::SC_SPEC_TYPE_VOID:
    case SCSpecType::SC_SPEC_TYPE_ERROR:
    case SCSpecType::SC_SPEC_TYPE_U32:
    case SCSpecType::SC_SPEC_TYPE_I32:
    case SCSpecType::SC_SPEC_TYPE_U64:
    case SCSpecType::SC_SPEC_TYPE_I64:
    case SCSpecType::SC_SPEC_TYPE_TIMEPOINT:
    case SCSpecType::SC_SPEC_TYPE_DURATION:
    case SCSpecType::SC_SPEC_TYPE_U128:
    case SCSpecType::SC_SPEC_TYPE_I128:
    case SCSpecType::SC_SPEC_TYPE_U256:
    case SCSpecType::SC_SPEC_TYPE_I256:
    case SCSpecType::SC_SPEC_TYPE_BYTES:
    case SCSpecType::SC_SPEC_TYPE_STRING:
    case SCSpecType::SC_SPEC_TYPE_SYMBOL:
    case SCSpecType::SC_SPEC_TYPE_ADDRESS:
    case SCSpecType::SC_SPEC_TYPE_MUXED_ADDRESS:
        return true;
    default:
        return false;
    }
}

/**
 * SCSpecTypeDef — a type, possibly built out of other types.
 *
 * The nested members are shared pointers because the union contains itself;
 * only the ones the discriminant selects are ever set.
 */
struct SCSpecTypeDef
{
    SCSpecType type = SCSpecType::SC_SPEC_TYPE_VAL;

    QSharedPointer<SCSpecTypeDef> valueType;    // OPTION, and MAP's value
    QSharedPointer<SCSpecTypeDef> keyType;      // MAP
    QSharedPointer<SCSpecTypeDef> okType;       // RESULT
    QSharedPointer<SCSpecTypeDef> errorType;    // RESULT
    QSharedPointer<SCSpecTypeDef> elementType;  // VEC
    QList<SCSpecTypeDef> valueTypes;            // TUPLE
    quint32 n = 0;                              // BYTES_N
    QByteArray name;                            // UDT
};

QDataStream& operator<<(QDataStream& out, const SCSpecTypeDef& v);
QDataStream& operator>>(QDataStream& in, SCSpecTypeDef& v);

/** Writes a nested type, refusing a null the discriminant said would be there. */
inline void writeNested(QDataStream& out, const QSharedPointer<SCSpecTypeDef>& v,
                        const char* what)
{
    if (v.isNull())
        throw std::runtime_error(std::string(what) + ": missing nested type");
    out << *v;
}

inline void readNested(QDataStream& in, QSharedPointer<SCSpecTypeDef>& v)
{
    v = QSharedPointer<SCSpecTypeDef>::create();
    in >> *v;
}

inline QDataStream& operator<<(QDataStream& out, const SCSpecTypeDef& v)
{
    out << static_cast<qint32>(v.type);
    if (isVoidSpecType(v.type))
        return out;                     // discriminant and nothing else

    switch (v.type) {
    case SCSpecType::SC_SPEC_TYPE_OPTION:
        writeNested(out, v.valueType, "SCSpecTypeOption");
        break;
    case SCSpecType::SC_SPEC_TYPE_RESULT:
        writeNested(out, v.okType, "SCSpecTypeResult");
        writeNested(out, v.errorType, "SCSpecTypeResult");
        break;
    case SCSpecType::SC_SPEC_TYPE_VEC:
        writeNested(out, v.elementType, "SCSpecTypeVec");
        break;
    case SCSpecType::SC_SPEC_TYPE_MAP:
        writeNested(out, v.keyType, "SCSpecTypeMap");
        writeNested(out, v.valueType, "SCSpecTypeMap");
        break;
    case SCSpecType::SC_SPEC_TYPE_TUPLE:
        if (v.valueTypes.size() > SC_SPEC_TUPLE_LIMIT)
            throw std::runtime_error("SCSpecTypeTuple: too many members");
        out << static_cast<qint32>(v.valueTypes.size());
        for (const SCSpecTypeDef& member : v.valueTypes)
            out << member;
        break;
    case SCSpecType::SC_SPEC_TYPE_BYTES_N:
        out << v.n;
        break;
    case SCSpecType::SC_SPEC_TYPE_UDT:
        writeSpecString(out, v.name, SC_SPEC_TYPE_NAME_LIMIT, "SCSpecTypeUDT.name");
        break;
    default:
        // No default arm exists in the XDR, so an unknown discriminant is a
        // malformed value rather than something to pass through.
        throw std::runtime_error("SCSpecTypeDef: unknown type");
    }
    return out;
}

inline QDataStream& operator>>(QDataStream& in, SCSpecTypeDef& v)
{
    SpecTypeDepthGuard depthGuard;

    v.type = static_cast<SCSpecType>(readSpecTag(in, "SCSpecTypeDef"));
    if (isVoidSpecType(v.type))
        return in;

    switch (v.type) {
    case SCSpecType::SC_SPEC_TYPE_OPTION:
        readNested(in, v.valueType);
        break;
    case SCSpecType::SC_SPEC_TYPE_RESULT:
        readNested(in, v.okType);
        readNested(in, v.errorType);
        break;
    case SCSpecType::SC_SPEC_TYPE_VEC:
        readNested(in, v.elementType);
        break;
    case SCSpecType::SC_SPEC_TYPE_MAP:
        readNested(in, v.keyType);
        readNested(in, v.valueType);
        break;
    case SCSpecType::SC_SPEC_TYPE_TUPLE: {
        const qint32 count = readSpecCount(in, SC_SPEC_TUPLE_LIMIT, "SCSpecTypeTuple");
        v.valueTypes.clear();
        for (qint32 i = 0; i < count; ++i) {
            SCSpecTypeDef member;
            in >> member;
            v.valueTypes.append(member);
        }
        break;
    }
    case SCSpecType::SC_SPEC_TYPE_BYTES_N:
        in >> v.n;
        break;
    case SCSpecType::SC_SPEC_TYPE_UDT:
        readSpecString(in, v.name, SC_SPEC_TYPE_NAME_LIMIT, "SCSpecTypeUDT");
        break;
    default:
        throw std::runtime_error("SCSpecTypeDef: unknown type on read");
    }
    return in;
}

// ─── User-defined types ──────────────────────────────────────────────

struct SCSpecUDTStructFieldV0
{
    QByteArray doc;
    QByteArray name;
    SCSpecTypeDef type;
};

inline QDataStream& operator<<(QDataStream& out, const SCSpecUDTStructFieldV0& v)
{
    writeSpecString(out, v.doc, SC_SPEC_DOC_LIMIT, "SCSpecUDTStructFieldV0.doc");
    writeSpecString(out, v.name, SC_SPEC_NAME_LIMIT, "SCSpecUDTStructFieldV0.name");
    out << v.type;
    return out;
}
inline QDataStream& operator>>(QDataStream& in, SCSpecUDTStructFieldV0& v)
{
    readSpecString(in, v.doc, SC_SPEC_DOC_LIMIT, "SCSpecUDTStructFieldV0.doc");
    readSpecString(in, v.name, SC_SPEC_NAME_LIMIT, "SCSpecUDTStructFieldV0.name");
    in >> v.type;
    return in;
}

struct SCSpecUDTStructV0
{
    QByteArray doc;
    QByteArray lib;
    QByteArray name;
    QList<SCSpecUDTStructFieldV0> fields;
};

inline QDataStream& operator<<(QDataStream& out, const SCSpecUDTStructV0& v)
{
    writeSpecString(out, v.doc, SC_SPEC_DOC_LIMIT, "SCSpecUDTStructV0.doc");
    writeSpecString(out, v.lib, SC_SPEC_LIB_LIMIT, "SCSpecUDTStructV0.lib");
    writeSpecString(out, v.name, SC_SPEC_TYPE_NAME_LIMIT, "SCSpecUDTStructV0.name");
    out << static_cast<qint32>(v.fields.size());
    for (const auto& field : v.fields) out << field;
    return out;
}
inline QDataStream& operator>>(QDataStream& in, SCSpecUDTStructV0& v)
{
    readSpecString(in, v.doc, SC_SPEC_DOC_LIMIT, "SCSpecUDTStructV0.doc");
    readSpecString(in, v.lib, SC_SPEC_LIB_LIMIT, "SCSpecUDTStructV0.lib");
    readSpecString(in, v.name, SC_SPEC_TYPE_NAME_LIMIT, "SCSpecUDTStructV0.name");
    const qint32 count = readSpecCount(in, MAX_SPEC_ARRAY, "SCSpecUDTStructV0.fields");
    v.fields.clear();
    for (qint32 i = 0; i < count; ++i) {
        SCSpecUDTStructFieldV0 field;
        in >> field;
        v.fields.append(field);
    }
    return in;
}

struct SCSpecUDTUnionCaseVoidV0
{
    QByteArray doc;
    QByteArray name;
};

inline QDataStream& operator<<(QDataStream& out, const SCSpecUDTUnionCaseVoidV0& v)
{
    writeSpecString(out, v.doc, SC_SPEC_DOC_LIMIT, "SCSpecUDTUnionCaseVoidV0.doc");
    writeSpecString(out, v.name, SC_SPEC_CASE_NAME_LIMIT, "SCSpecUDTUnionCaseVoidV0.name");
    return out;
}
inline QDataStream& operator>>(QDataStream& in, SCSpecUDTUnionCaseVoidV0& v)
{
    readSpecString(in, v.doc, SC_SPEC_DOC_LIMIT, "SCSpecUDTUnionCaseVoidV0.doc");
    readSpecString(in, v.name, SC_SPEC_CASE_NAME_LIMIT, "SCSpecUDTUnionCaseVoidV0.name");
    return in;
}

struct SCSpecUDTUnionCaseTupleV0
{
    QByteArray doc;
    QByteArray name;
    /** Named `type` in the XDR despite being a list; kept so a reader can find it. */
    QList<SCSpecTypeDef> type;
};

inline QDataStream& operator<<(QDataStream& out, const SCSpecUDTUnionCaseTupleV0& v)
{
    writeSpecString(out, v.doc, SC_SPEC_DOC_LIMIT, "SCSpecUDTUnionCaseTupleV0.doc");
    writeSpecString(out, v.name, SC_SPEC_CASE_NAME_LIMIT, "SCSpecUDTUnionCaseTupleV0.name");
    out << static_cast<qint32>(v.type.size());
    for (const auto& member : v.type) out << member;
    return out;
}
inline QDataStream& operator>>(QDataStream& in, SCSpecUDTUnionCaseTupleV0& v)
{
    readSpecString(in, v.doc, SC_SPEC_DOC_LIMIT, "SCSpecUDTUnionCaseTupleV0.doc");
    readSpecString(in, v.name, SC_SPEC_CASE_NAME_LIMIT, "SCSpecUDTUnionCaseTupleV0.name");
    const qint32 count = readSpecCount(in, MAX_SPEC_ARRAY, "SCSpecUDTUnionCaseTupleV0.type");
    v.type.clear();
    for (qint32 i = 0; i < count; ++i) {
        SCSpecTypeDef member;
        in >> member;
        v.type.append(member);
    }
    return in;
}

enum class SCSpecUDTUnionCaseV0Kind : qint32
{
    SC_SPEC_UDT_UNION_CASE_VOID_V0 = 0,
    SC_SPEC_UDT_UNION_CASE_TUPLE_V0 = 1
};

struct SCSpecUDTUnionCaseV0
{
    SCSpecUDTUnionCaseV0Kind kind = SCSpecUDTUnionCaseV0Kind::SC_SPEC_UDT_UNION_CASE_VOID_V0;
    SCSpecUDTUnionCaseVoidV0 voidCase;
    SCSpecUDTUnionCaseTupleV0 tupleCase;
};

inline QDataStream& operator<<(QDataStream& out, const SCSpecUDTUnionCaseV0& v)
{
    out << static_cast<qint32>(v.kind);
    switch (v.kind) {
    case SCSpecUDTUnionCaseV0Kind::SC_SPEC_UDT_UNION_CASE_VOID_V0:  out << v.voidCase;  break;
    case SCSpecUDTUnionCaseV0Kind::SC_SPEC_UDT_UNION_CASE_TUPLE_V0: out << v.tupleCase; break;
    default: throw std::runtime_error("SCSpecUDTUnionCaseV0: unknown kind");
    }
    return out;
}
inline QDataStream& operator>>(QDataStream& in, SCSpecUDTUnionCaseV0& v)
{
    v.kind = static_cast<SCSpecUDTUnionCaseV0Kind>(
        readSpecTag(in, "SCSpecUDTUnionCaseV0"));
    switch (v.kind) {
    case SCSpecUDTUnionCaseV0Kind::SC_SPEC_UDT_UNION_CASE_VOID_V0:  in >> v.voidCase;  break;
    case SCSpecUDTUnionCaseV0Kind::SC_SPEC_UDT_UNION_CASE_TUPLE_V0: in >> v.tupleCase; break;
    default: throw std::runtime_error("SCSpecUDTUnionCaseV0: unknown kind on read");
    }
    return in;
}

struct SCSpecUDTUnionV0
{
    QByteArray doc;
    QByteArray lib;
    QByteArray name;
    QList<SCSpecUDTUnionCaseV0> cases;
};

inline QDataStream& operator<<(QDataStream& out, const SCSpecUDTUnionV0& v)
{
    writeSpecString(out, v.doc, SC_SPEC_DOC_LIMIT, "SCSpecUDTUnionV0.doc");
    writeSpecString(out, v.lib, SC_SPEC_LIB_LIMIT, "SCSpecUDTUnionV0.lib");
    writeSpecString(out, v.name, SC_SPEC_TYPE_NAME_LIMIT, "SCSpecUDTUnionV0.name");
    out << static_cast<qint32>(v.cases.size());
    for (const auto& entry : v.cases) out << entry;
    return out;
}
inline QDataStream& operator>>(QDataStream& in, SCSpecUDTUnionV0& v)
{
    readSpecString(in, v.doc, SC_SPEC_DOC_LIMIT, "SCSpecUDTUnionV0.doc");
    readSpecString(in, v.lib, SC_SPEC_LIB_LIMIT, "SCSpecUDTUnionV0.lib");
    readSpecString(in, v.name, SC_SPEC_TYPE_NAME_LIMIT, "SCSpecUDTUnionV0.name");
    const qint32 count = readSpecCount(in, MAX_SPEC_ARRAY, "SCSpecUDTUnionV0.cases");
    v.cases.clear();
    for (qint32 i = 0; i < count; ++i) {
        SCSpecUDTUnionCaseV0 entry;
        in >> entry;
        v.cases.append(entry);
    }
    return in;
}

/**
 * An enum case, and an error-enum case, which are byte-identical.
 *
 * The two are told apart ONLY by the discriminant of the SCSpecEntry that
 * contains them, so any code that dispatches on the shape of the payload
 * rather than on the entry kind will conflate them.
 */
struct SCSpecUDTEnumCaseV0
{
    QByteArray doc;
    QByteArray name;
    quint32 value = 0;
};

inline QDataStream& operator<<(QDataStream& out, const SCSpecUDTEnumCaseV0& v)
{
    writeSpecString(out, v.doc, SC_SPEC_DOC_LIMIT, "SCSpecUDTEnumCaseV0.doc");
    writeSpecString(out, v.name, SC_SPEC_CASE_NAME_LIMIT, "SCSpecUDTEnumCaseV0.name");
    out << v.value;
    return out;
}
inline QDataStream& operator>>(QDataStream& in, SCSpecUDTEnumCaseV0& v)
{
    readSpecString(in, v.doc, SC_SPEC_DOC_LIMIT, "SCSpecUDTEnumCaseV0.doc");
    readSpecString(in, v.name, SC_SPEC_CASE_NAME_LIMIT, "SCSpecUDTEnumCaseV0.name");
    in >> v.value;
    return in;
}

struct SCSpecUDTEnumV0
{
    QByteArray doc;
    QByteArray lib;
    QByteArray name;
    QList<SCSpecUDTEnumCaseV0> cases;
};

inline QDataStream& operator<<(QDataStream& out, const SCSpecUDTEnumV0& v)
{
    writeSpecString(out, v.doc, SC_SPEC_DOC_LIMIT, "SCSpecUDTEnumV0.doc");
    writeSpecString(out, v.lib, SC_SPEC_LIB_LIMIT, "SCSpecUDTEnumV0.lib");
    writeSpecString(out, v.name, SC_SPEC_TYPE_NAME_LIMIT, "SCSpecUDTEnumV0.name");
    out << static_cast<qint32>(v.cases.size());
    for (const auto& entry : v.cases) out << entry;
    return out;
}
inline QDataStream& operator>>(QDataStream& in, SCSpecUDTEnumV0& v)
{
    readSpecString(in, v.doc, SC_SPEC_DOC_LIMIT, "SCSpecUDTEnumV0.doc");
    readSpecString(in, v.lib, SC_SPEC_LIB_LIMIT, "SCSpecUDTEnumV0.lib");
    readSpecString(in, v.name, SC_SPEC_TYPE_NAME_LIMIT, "SCSpecUDTEnumV0.name");
    const qint32 count = readSpecCount(in, MAX_SPEC_ARRAY, "SCSpecUDTEnumV0.cases");
    v.cases.clear();
    for (qint32 i = 0; i < count; ++i) {
        SCSpecUDTEnumCaseV0 entry;
        in >> entry;
        v.cases.append(entry);
    }
    return in;
}

// ─── Functions ───────────────────────────────────────────────────────

struct SCSpecFunctionInputV0
{
    QByteArray doc;
    QByteArray name;
    SCSpecTypeDef type;
};

inline QDataStream& operator<<(QDataStream& out, const SCSpecFunctionInputV0& v)
{
    writeSpecString(out, v.doc, SC_SPEC_DOC_LIMIT, "SCSpecFunctionInputV0.doc");
    writeSpecString(out, v.name, SC_SPEC_NAME_LIMIT, "SCSpecFunctionInputV0.name");
    out << v.type;
    return out;
}
inline QDataStream& operator>>(QDataStream& in, SCSpecFunctionInputV0& v)
{
    readSpecString(in, v.doc, SC_SPEC_DOC_LIMIT, "SCSpecFunctionInputV0.doc");
    readSpecString(in, v.name, SC_SPEC_NAME_LIMIT, "SCSpecFunctionInputV0.name");
    in >> v.type;
    return in;
}

struct SCSpecFunctionV0
{
    QByteArray doc;
    /** An SCSymbol, so capped at 32 bytes: not raised with type names by #312. */
    QByteArray name;
    QList<SCSpecFunctionInputV0> inputs;
    /** `outputs<1>`: an array of at most one, NOT an optional. Empty means void. */
    QList<SCSpecTypeDef> outputs;
};

inline QDataStream& operator<<(QDataStream& out, const SCSpecFunctionV0& v)
{
    writeSpecString(out, v.doc, SC_SPEC_DOC_LIMIT, "SCSpecFunctionV0.doc");
    writeSpecString(out, v.name, SC_SYMBOL_LIMIT, "SCSpecFunctionV0.name");
    out << static_cast<qint32>(v.inputs.size());
    for (const auto& input : v.inputs) out << input;
    if (v.outputs.size() > 1)
        throw std::runtime_error("SCSpecFunctionV0: a function returns at most one value");
    out << static_cast<qint32>(v.outputs.size());
    for (const auto& output : v.outputs) out << output;
    return out;
}
inline QDataStream& operator>>(QDataStream& in, SCSpecFunctionV0& v)
{
    readSpecString(in, v.doc, SC_SPEC_DOC_LIMIT, "SCSpecFunctionV0.doc");
    readSpecString(in, v.name, SC_SYMBOL_LIMIT, "SCSpecFunctionV0.name");

    const qint32 inputCount = readSpecCount(in, MAX_SPEC_ARRAY, "SCSpecFunctionV0.inputs");
    v.inputs.clear();
    for (qint32 i = 0; i < inputCount; ++i) {
        SCSpecFunctionInputV0 input;
        in >> input;
        v.inputs.append(input);
    }

    // The bound of one is the whole difference between this and an optional.
    const qint32 outputCount = readSpecCount(in, 1, "SCSpecFunctionV0.outputs");
    v.outputs.clear();
    for (qint32 i = 0; i < outputCount; ++i) {
        SCSpecTypeDef output;
        in >> output;
        v.outputs.append(output);
    }
    return in;
}

// ─── Events ──────────────────────────────────────────────────────────

enum class SCSpecEventParamLocationV0 : qint32
{
    SC_SPEC_EVENT_PARAM_LOCATION_DATA = 0,
    SC_SPEC_EVENT_PARAM_LOCATION_TOPIC_LIST = 1
};

struct SCSpecEventParamV0
{
    QByteArray doc;
    QByteArray name;
    SCSpecTypeDef type;
    SCSpecEventParamLocationV0 location =
        SCSpecEventParamLocationV0::SC_SPEC_EVENT_PARAM_LOCATION_DATA;
};

inline QDataStream& operator<<(QDataStream& out, const SCSpecEventParamV0& v)
{
    writeSpecString(out, v.doc, SC_SPEC_DOC_LIMIT, "SCSpecEventParamV0.doc");
    writeSpecString(out, v.name, SC_SPEC_NAME_LIMIT, "SCSpecEventParamV0.name");
    out << v.type;
    out << static_cast<qint32>(v.location);
    return out;
}
inline QDataStream& operator>>(QDataStream& in, SCSpecEventParamV0& v)
{
    readSpecString(in, v.doc, SC_SPEC_DOC_LIMIT, "SCSpecEventParamV0.doc");
    readSpecString(in, v.name, SC_SPEC_NAME_LIMIT, "SCSpecEventParamV0.name");
    in >> v.type;
    const qint32 raw = readSpecTag(in, "SCSpecEventParamV0.location");
    if (raw < 0 || raw > 1)
        throw std::runtime_error("SCSpecEventParamV0: unknown location");
    v.location = static_cast<SCSpecEventParamLocationV0>(raw);
    return in;
}

enum class SCSpecEventDataFormat : qint32
{
    SC_SPEC_EVENT_DATA_FORMAT_SINGLE_VALUE = 0,
    SC_SPEC_EVENT_DATA_FORMAT_VEC = 1,
    SC_SPEC_EVENT_DATA_FORMAT_MAP = 2
};

struct SCSpecEventV0
{
    QByteArray doc;
    QByteArray lib;
    /** Up to 1024 bytes since stellar-xdr #312; it was an SCSymbol of 32. */
    QByteArray name;
    /** `prefixTopics<2>`: zero, one or two whole symbols. */
    QList<QByteArray> prefixTopics;
    QList<SCSpecEventParamV0> params;
    SCSpecEventDataFormat dataFormat =
        SCSpecEventDataFormat::SC_SPEC_EVENT_DATA_FORMAT_SINGLE_VALUE;
};

inline QDataStream& operator<<(QDataStream& out, const SCSpecEventV0& v)
{
    writeSpecString(out, v.doc, SC_SPEC_DOC_LIMIT, "SCSpecEventV0.doc");
    writeSpecString(out, v.lib, SC_SPEC_LIB_LIMIT, "SCSpecEventV0.lib");
    writeSpecString(out, v.name, SC_SPEC_TYPE_NAME_LIMIT, "SCSpecEventV0.name");
    if (v.prefixTopics.size() > SC_SPEC_EVENT_PREFIX_TOPIC_LIMIT)
        throw std::runtime_error("SCSpecEventV0: too many prefix topics");
    out << static_cast<qint32>(v.prefixTopics.size());
    for (const auto& topic : v.prefixTopics)
        writeSpecString(out, topic, SC_SYMBOL_LIMIT, "SCSpecEventV0.prefixTopics");
    out << static_cast<qint32>(v.params.size());
    for (const auto& param : v.params) out << param;
    out << static_cast<qint32>(v.dataFormat);
    return out;
}
inline QDataStream& operator>>(QDataStream& in, SCSpecEventV0& v)
{
    readSpecString(in, v.doc, SC_SPEC_DOC_LIMIT, "SCSpecEventV0.doc");
    readSpecString(in, v.lib, SC_SPEC_LIB_LIMIT, "SCSpecEventV0.lib");
    readSpecString(in, v.name, SC_SPEC_TYPE_NAME_LIMIT, "SCSpecEventV0.name");

    const qint32 topicCount =
        readSpecCount(in, SC_SPEC_EVENT_PREFIX_TOPIC_LIMIT, "SCSpecEventV0.prefixTopics");
    v.prefixTopics.clear();
    for (qint32 i = 0; i < topicCount; ++i) {
        QByteArray topic;
        readSpecString(in, topic, SC_SYMBOL_LIMIT, "SCSpecEventV0.prefixTopics");
        v.prefixTopics.append(topic);
    }

    const qint32 paramCount = readSpecCount(in, MAX_SPEC_ARRAY, "SCSpecEventV0.params");
    v.params.clear();
    for (qint32 i = 0; i < paramCount; ++i) {
        SCSpecEventParamV0 param;
        in >> param;
        v.params.append(param);
    }

    const qint32 raw = readSpecTag(in, "SCSpecEventV0.dataFormat");
    if (raw < 0 || raw > 2)
        throw std::runtime_error("SCSpecEventV0: unknown data format");
    v.dataFormat = static_cast<SCSpecEventDataFormat>(raw);
    return in;
}

// ─── The entry ───────────────────────────────────────────────────────

enum class SCSpecEntryKind : qint32
{
    SC_SPEC_ENTRY_FUNCTION_V0 = 0,
    SC_SPEC_ENTRY_UDT_STRUCT_V0 = 1,
    SC_SPEC_ENTRY_UDT_UNION_V0 = 2,
    SC_SPEC_ENTRY_UDT_ENUM_V0 = 3,
    SC_SPEC_ENTRY_UDT_ERROR_ENUM_V0 = 4,
    SC_SPEC_ENTRY_EVENT_V0 = 5
};

struct SCSpecEntry
{
    SCSpecEntryKind kind = SCSpecEntryKind::SC_SPEC_ENTRY_FUNCTION_V0;

    SCSpecFunctionV0 functionV0;
    SCSpecUDTStructV0 udtStructV0;
    SCSpecUDTUnionV0 udtUnionV0;
    SCSpecUDTEnumV0 udtEnumV0;
    /** Byte-identical to udtEnumV0; only `kind` tells them apart. */
    SCSpecUDTEnumV0 udtErrorEnumV0;
    SCSpecEventV0 eventV0;
};

inline QDataStream& operator<<(QDataStream& out, const SCSpecEntry& v)
{
    out << static_cast<qint32>(v.kind);
    switch (v.kind) {
    case SCSpecEntryKind::SC_SPEC_ENTRY_FUNCTION_V0:       out << v.functionV0;     break;
    case SCSpecEntryKind::SC_SPEC_ENTRY_UDT_STRUCT_V0:     out << v.udtStructV0;    break;
    case SCSpecEntryKind::SC_SPEC_ENTRY_UDT_UNION_V0:      out << v.udtUnionV0;     break;
    case SCSpecEntryKind::SC_SPEC_ENTRY_UDT_ENUM_V0:       out << v.udtEnumV0;      break;
    case SCSpecEntryKind::SC_SPEC_ENTRY_UDT_ERROR_ENUM_V0: out << v.udtErrorEnumV0; break;
    case SCSpecEntryKind::SC_SPEC_ENTRY_EVENT_V0:          out << v.eventV0;        break;
    default: throw std::runtime_error("SCSpecEntry: unknown kind");
    }
    return out;
}

inline QDataStream& operator>>(QDataStream& in, SCSpecEntry& v)
{
    v.kind = static_cast<SCSpecEntryKind>(readSpecTag(in, "SCSpecEntry"));
    switch (v.kind) {
    case SCSpecEntryKind::SC_SPEC_ENTRY_FUNCTION_V0:       in >> v.functionV0;     break;
    case SCSpecEntryKind::SC_SPEC_ENTRY_UDT_STRUCT_V0:     in >> v.udtStructV0;    break;
    case SCSpecEntryKind::SC_SPEC_ENTRY_UDT_UNION_V0:      in >> v.udtUnionV0;     break;
    case SCSpecEntryKind::SC_SPEC_ENTRY_UDT_ENUM_V0:       in >> v.udtEnumV0;      break;
    case SCSpecEntryKind::SC_SPEC_ENTRY_UDT_ERROR_ENUM_V0: in >> v.udtErrorEnumV0; break;
    case SCSpecEntryKind::SC_SPEC_ENTRY_EVENT_V0:          in >> v.eventV0;        break;
    default: throw std::runtime_error("SCSpecEntry: unknown kind on read");
    }
    return in;
}

} // namespace stellar

#endif // QSTELLAR_XDR_CONTRACTSPEC_H
