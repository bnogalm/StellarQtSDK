#include "wasmsections.h"

#include <cstring>

QSTELLAR_BEGIN_NS

namespace {

// A WebAssembly module opens with "\0asm" and a four-byte version.
const char WASM_MAGIC[4] = { '\0', 'a', 's', 'm' };
const int WASM_HEADER_SIZE = 8;

/** The id every custom section carries; all the numbered ones mean something else. */
const quint8 CUSTOM_SECTION_ID = 0;

/**
 * Reads one unsigned LEB128 integer at `offset`, advancing it.
 *
 * Three separate ways to get this wrong, and it is worth naming all three.
 *
 * A run of continuation bytes that never ends: five bytes is the most a 32-bit
 * value can occupy, so a sixth is not a large number, it is a lie, and a loop
 * without a counter shifts forever on `FF FF FF FF FF FF...`.
 *
 * The fifth byte carries only FOUR usable bits, because 4x7 + 4 = 32. A reader
 * that accepts `FF FF FF FF 7F` and lets the shift fall off the end hands back
 * a plausible-looking size that the file never encoded — which is worse than
 * refusing, because the caller then trusts it.
 *
 * And the one that goes the other way: a NON-MINIMAL encoding is legal.
 * `80 80 80 80 00` is a well-formed zero, and tools that pad a size to a fixed
 * width so it can be patched later emit exactly that. Rejecting it would
 * refuse modules the network accepts.
 */
bool readUnsigned(const QByteArray& data, int& offset, quint32& value)
{
    quint32 result = 0;
    int shift = 0;
    for (int consumed = 0; consumed < 5; ++consumed) {
        if (offset >= data.size())
            return false;                       // ran out mid-number
        const quint8 byte = quint8(data.at(offset++));
        const quint8 payload = byte & 0x7F;

        if (consumed == 4 && payload > 0x0F)
            return false;                       // more bits than a u32 can hold

        result |= quint32(payload) << shift;
        if ((byte & 0x80) == 0) {
            value = result;
            return true;
        }
        shift += 7;
    }
    return false;                               // never terminated
}

/**
 * Walks the sections, calling `visit(name, payload)` for each custom one.
 *
 * `visit` returns true to stop. Every bound is checked against the real size of
 * the buffer, so a section header claiming more bytes than the file holds ends
 * the walk instead of reading past it.
 */
template <typename Visitor>
void walkCustomSections(const QByteArray& wasm, Visitor visit)
{
    if (!WasmSections::looksLikeWasm(wasm))
        return;

    int offset = WASM_HEADER_SIZE;
    while (offset < wasm.size()) {
        const quint8 id = quint8(wasm.at(offset++));

        quint32 size = 0;
        if (!readUnsigned(wasm, offset, size))
            return;
        // The size is a claim the file makes about itself. Check it.
        if (size > quint32(wasm.size() - offset))
            return;

        const int sectionStart = offset;
        const int sectionEnd = offset + int(size);

        if (id == CUSTOM_SECTION_ID) {
            // A custom section begins with its own name, and the name counts
            // towards the section size rather than sitting outside it.
            int cursor = sectionStart;
            quint32 nameLength = 0;
            if (readUnsigned(wasm, cursor, nameLength)
                && nameLength <= quint32(sectionEnd - cursor)) {
                const QByteArray name = wasm.mid(cursor, int(nameLength));
                const int payloadStart = cursor + int(nameLength);
                const QByteArray payload =
                    wasm.mid(payloadStart, sectionEnd - payloadStart);
                if (visit(name, payload))
                    return;
            }
            // A custom section we cannot parse is skipped rather than fatal:
            // the specification says custom sections carry no meaning to a host
            // that does not recognise them, so one bad name length must not
            // hide the sections after it.
        }

        offset = sectionEnd;
    }
}

} // namespace

bool WasmSections::looksLikeWasm(const QByteArray& wasm)
{
    if (wasm.size() < WASM_HEADER_SIZE)
        return false;
    if (std::memcmp(wasm.constData(), WASM_MAGIC, sizeof(WASM_MAGIC)) != 0)
        return false;
    // Version 1, little-endian. There has never been another one, and a module
    // claiming otherwise is not something this reader should guess about.
    return quint8(wasm.at(4)) == 1 && quint8(wasm.at(5)) == 0
           && quint8(wasm.at(6)) == 0 && quint8(wasm.at(7)) == 0;
}

QByteArray WasmSections::customSection(const QByteArray& wasm, const QByteArray& name)
{
    QByteArray found;
    walkCustomSections(wasm, [&](const QByteArray& sectionName,
                                 const QByteArray& payload) {
        if (sectionName != name)
            return false;
        found = payload;
        return true;                            // first match wins
    });
    return found;
}

QStringList WasmSections::customSectionNames(const QByteArray& wasm)
{
    QStringList names;
    walkCustomSections(wasm, [&](const QByteArray& sectionName, const QByteArray&) {
        names.append(QString::fromUtf8(sectionName));
        return false;
    });
    return names;
}

QSTELLAR_END_NS
