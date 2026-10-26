#include "contractspec.h"

#include <QDataStream>

#include "wasmsections.h"

QSTELLAR_BEGIN_NS

namespace {

const char* const SECTION_SPEC = "contractspecv0";
const char* const SECTION_ENV_META = "contractenvmetav0";
const char* const SECTION_META = "contractmetav0";

QString text(const QByteArray& bytes)
{
    return QString::fromUtf8(bytes);
}

/**
 * The environment metadata: one XDR union whose only arm today carries the
 * protocol version the contract was built for.
 *
 * The host reads this with an iterator and takes the first entry without
 * checking that the section was fully consumed, so trailing bytes are tolerated
 * on chain. Asserting an exact length here would reject modules the network
 * accepts.
 */
bool readEnvMeta(const QByteArray& section, quint32& protocol, quint32& preRelease)
{
    if (section.size() < 12)
        return false;
    QDataStream in(section);          // big-endian by default, which XDR is
    qint32 kind = 0;
    in >> kind;
    if (kind != 0)                    // SC_ENV_META_KIND_INTERFACE_VERSION
        return false;
    in >> protocol >> preRelease;
    return in.status() == QDataStream::Ok;
}

/**
 * Author metadata: like the spec section, a concatenation of entries rather
 * than one blob, for the same reason — one static per `contractmeta!`.
 */
QList<QPair<QString, QString>> readMetadata(const QByteArray& section)
{
    QList<QPair<QString, QString>> out;
    if (section.isEmpty())
        return out;

    QDataStream in(section);
    while (!in.atEnd()) {
        qint32 kind = 0;
        in >> kind;
        if (in.status() != QDataStream::Ok || kind != 0)   // SC_META_V0
            break;
        QByteArray key;
        QByteArray value;
        try {
            // The lengths here are unbounded in the XDR, so they are bounded
            // against the section rather than trusted.
            stellar::readSpecString(in, key, section.size(), "SCMetaV0.key");
            stellar::readSpecString(in, value, section.size(), "SCMetaV0.val");
        } catch (const std::exception&) {
            break;                    // stop at the first entry we cannot read
        }
        if (in.status() != QDataStream::Ok)
            break;
        out.append({ text(key), text(value) });
    }
    return out;
}

} // namespace

QList<stellar::SCSpecEntry> ContractSpec::decodeSpec(const QByteArray& specSection)
{
    QList<stellar::SCSpecEntry> entries;
    if (specSection.isEmpty())
        return entries;

    QDataStream in(specSection);
    // No count and no separators: read entries until the bytes run out. A
    // reader that decodes one gets the first function and quietly loses the
    // rest of the interface.
    while (!in.atEnd()) {
        stellar::SCSpecEntry entry;
        in >> entry;
        if (in.status() != QDataStream::Ok)
            throw std::runtime_error("contract spec: truncated entry");
        entries.append(entry);
        if (entries.size() > stellar::MAX_SPEC_ARRAY)
            throw std::runtime_error("contract spec: implausibly many entries");
    }
    return entries;
}

QString ContractSpec::describeType(const stellar::SCSpecTypeDef& type)
{
    using stellar::SCSpecType;
    switch (type.type) {
    case SCSpecType::SC_SPEC_TYPE_VAL:           return QStringLiteral("val");
    case SCSpecType::SC_SPEC_TYPE_BOOL:          return QStringLiteral("bool");
    case SCSpecType::SC_SPEC_TYPE_VOID:          return QStringLiteral("void");
    case SCSpecType::SC_SPEC_TYPE_ERROR:         return QStringLiteral("error");
    case SCSpecType::SC_SPEC_TYPE_U32:           return QStringLiteral("u32");
    case SCSpecType::SC_SPEC_TYPE_I32:           return QStringLiteral("i32");
    case SCSpecType::SC_SPEC_TYPE_U64:           return QStringLiteral("u64");
    case SCSpecType::SC_SPEC_TYPE_I64:           return QStringLiteral("i64");
    case SCSpecType::SC_SPEC_TYPE_TIMEPOINT:     return QStringLiteral("timepoint");
    case SCSpecType::SC_SPEC_TYPE_DURATION:      return QStringLiteral("duration");
    case SCSpecType::SC_SPEC_TYPE_U128:          return QStringLiteral("u128");
    case SCSpecType::SC_SPEC_TYPE_I128:          return QStringLiteral("i128");
    case SCSpecType::SC_SPEC_TYPE_U256:          return QStringLiteral("u256");
    case SCSpecType::SC_SPEC_TYPE_I256:          return QStringLiteral("i256");
    case SCSpecType::SC_SPEC_TYPE_BYTES:         return QStringLiteral("bytes");
    case SCSpecType::SC_SPEC_TYPE_STRING:        return QStringLiteral("string");
    case SCSpecType::SC_SPEC_TYPE_SYMBOL:        return QStringLiteral("symbol");
    case SCSpecType::SC_SPEC_TYPE_ADDRESS:       return QStringLiteral("address");
    case SCSpecType::SC_SPEC_TYPE_MUXED_ADDRESS: return QStringLiteral("muxed address");

    case SCSpecType::SC_SPEC_TYPE_OPTION:
        return QStringLiteral("option<%1>").arg(
            type.valueType ? describeType(*type.valueType) : QStringLiteral("?"));
    case SCSpecType::SC_SPEC_TYPE_RESULT:
        return QStringLiteral("result<%1, %2>")
            .arg(type.okType ? describeType(*type.okType) : QStringLiteral("?"),
                 type.errorType ? describeType(*type.errorType) : QStringLiteral("?"));
    case SCSpecType::SC_SPEC_TYPE_VEC:
        return QStringLiteral("vec<%1>").arg(
            type.elementType ? describeType(*type.elementType) : QStringLiteral("?"));
    case SCSpecType::SC_SPEC_TYPE_MAP:
        return QStringLiteral("map<%1, %2>")
            .arg(type.keyType ? describeType(*type.keyType) : QStringLiteral("?"),
                 type.valueType ? describeType(*type.valueType) : QStringLiteral("?"));
    case SCSpecType::SC_SPEC_TYPE_TUPLE: {
        QStringList members;
        for (const auto& member : type.valueTypes)
            members.append(describeType(member));
        return QStringLiteral("tuple<%1>").arg(members.join(QStringLiteral(", ")));
    }
    case SCSpecType::SC_SPEC_TYPE_BYTES_N:
        return QStringLiteral("bytes[%1]").arg(type.n);
    case SCSpecType::SC_SPEC_TYPE_UDT:
        return text(type.name);
    }
    return QStringLiteral("unknown");
}

QString ContractSpec::scvTypeFor(const stellar::SCSpecTypeDef& type)
{
    using stellar::SCSpecType;
    switch (type.type) {
    case SCSpecType::SC_SPEC_TYPE_BOOL:       return QStringLiteral("bool");
    case SCSpecType::SC_SPEC_TYPE_VOID:       return QStringLiteral("void");
    case SCSpecType::SC_SPEC_TYPE_U32:        return QStringLiteral("u32");
    case SCSpecType::SC_SPEC_TYPE_I32:        return QStringLiteral("i32");
    case SCSpecType::SC_SPEC_TYPE_U64:        return QStringLiteral("u64");
    case SCSpecType::SC_SPEC_TYPE_I64:        return QStringLiteral("i64");
    case SCSpecType::SC_SPEC_TYPE_U128:       return QStringLiteral("u128");
    case SCSpecType::SC_SPEC_TYPE_I128:       return QStringLiteral("i128");
    case SCSpecType::SC_SPEC_TYPE_U256:       return QStringLiteral("u256");
    case SCSpecType::SC_SPEC_TYPE_I256:       return QStringLiteral("i256");
    case SCSpecType::SC_SPEC_TYPE_TIMEPOINT:  return QStringLiteral("timepoint");
    case SCSpecType::SC_SPEC_TYPE_DURATION:   return QStringLiteral("duration");
    case SCSpecType::SC_SPEC_TYPE_BYTES:      return QStringLiteral("bytes");
    case SCSpecType::SC_SPEC_TYPE_STRING:     return QStringLiteral("string");
    case SCSpecType::SC_SPEC_TYPE_SYMBOL:     return QStringLiteral("symbol");
    case SCSpecType::SC_SPEC_TYPE_ADDRESS:    return QStringLiteral("address");
    case SCSpecType::SC_SPEC_TYPE_VEC:        return QStringLiteral("vec");
    case SCSpecType::SC_SPEC_TYPE_MAP:        return QStringLiteral("map");
    default:
        // Tuples, options, results, fixed-size bytes and every user-defined
        // type. Returning nothing is the honest answer: a form that offered a
        // near type would produce a call the host rejects after the user had
        // already been told it was fine.
        return QString();
    }
}

ContractSpec::Contract ContractSpec::read(const QByteArray& wasm)
{
    Contract out;

    out.isWasm = WasmSections::looksLikeWasm(wasm);
    if (!out.isWasm) {
        out.problem = QStringLiteral(
            "This is not a WebAssembly module. A Soroban contract is a .wasm "
            "file beginning with the bytes 00 61 73 6D; check that what you "
            "have is the compiled artefact rather than the source, an archive, "
            "or base64 text that was never decoded.");
        return out;
    }

    out.sectionNames = WasmSections::customSectionNames(wasm);

    const QByteArray envMeta =
        WasmSections::customSection(wasm, QByteArray(SECTION_ENV_META));
    out.hasEnvMeta = !envMeta.isEmpty()
                     && readEnvMeta(envMeta, out.protocolVersion, out.preRelease);

    out.metadata = readMetadata(WasmSections::customSection(wasm, QByteArray(SECTION_META)));

    const QByteArray spec = WasmSections::customSection(wasm, QByteArray(SECTION_SPEC));
    out.hasSpec = !spec.isEmpty();

    if (!out.hasEnvMeta && !out.hasSpec) {
        // Valid WebAssembly, but nothing Soroban put there.
        out.problem = QStringLiteral(
            "This is a WebAssembly module, but not a Soroban contract: it "
            "carries neither the environment metadata the network requires nor "
            "a published interface.");
        return out;
    }
    if (!out.hasSpec) {
        // A real contract that simply cannot be introspected. Worth saying so
        // precisely, because it is a different problem with a different fix.
        out.problem = QStringLiteral(
            "This contract has no published interface: the contractspecv0 "
            "section is absent, so its functions cannot be listed. It will "
            "still upload and run — only tooling is affected.");
        return out;
    }

    QList<stellar::SCSpecEntry> entries;
    try {
        entries = decodeSpec(spec);
    } catch (const std::exception& e) {
        out.problem = QStringLiteral("The interface section could not be read: ")
                      + QString::fromUtf8(e.what());
        return out;
    }

    for (const auto& entry : entries) {
        if (entry.kind != stellar::SCSpecEntryKind::SC_SPEC_ENTRY_FUNCTION_V0)
            continue;                 // structs, unions, enums and events

        Function function;
        function.name = text(entry.functionV0.name);
        function.doc = text(entry.functionV0.doc);
        for (const auto& input : entry.functionV0.inputs) {
            Argument argument;
            argument.name = text(input.name);
            argument.type = describeType(input.type);
            argument.scvType = scvTypeFor(input.type);
            argument.doc = text(input.doc);
            function.inputs.append(argument);
        }
        // outputs is an array of at most one; empty means the function returns
        // nothing, which is not the same as returning void explicitly.
        if (!entry.functionV0.outputs.isEmpty())
            function.returns = describeType(entry.functionV0.outputs.first());
        out.functions.append(function);
    }

    if (out.functions.isEmpty()) {
        out.problem = QStringLiteral(
            "The interface section holds no functions. That is legal — it may "
            "describe only types — but there is nothing here to call.");
    }
    return out;
}

QSTELLAR_END_NS
