#ifndef QSTELLAR_WASMSECTIONS_H
#define QSTELLAR_WASMSECTIONS_H

#include <QByteArray>
#include <QStringList>

#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * Reads the custom sections out of a WebAssembly module.
 *
 * A compiled Soroban contract carries its own description inside it: the
 * machine-readable ABI in one custom section and the environment version it was
 * built against in another. That means a tool can be handed nothing but a .wasm
 * file and still say what the contract exports, what each function takes and
 * what it returns — no registry, no source, no toolchain.
 *
 * This class knows nothing about Soroban. It walks the WebAssembly container
 * and hands back the bytes of a named section; interpreting them is somebody
 * else's job. Keeping the two apart matters because the container format is
 * fixed by the WebAssembly specification while the contents are Stellar's, and
 * they change on different schedules.
 *
 * EVERY INPUT IS HOSTILE. A .wasm arrives from wherever the user got it, and a
 * length prefix inside it is a number a file can lie about. Nothing here trusts
 * a size, an offset or a count without checking it against what is actually
 * there; a malformed module produces an empty result, never a read past the
 * end of the buffer.
 */
class WasmSections
{
public:
    /** True if `wasm` starts with the WebAssembly magic number and version 1. */
    static bool looksLikeWasm(const QByteArray& wasm);

    /**
     * The payload of the first custom section called `name`, or an empty array.
     *
     * "First" is deliberate. The specification permits a module to carry
     * several custom sections sharing a name, and a reader that concatenated
     * them, or that silently took the last, would be inventing a rule nobody
     * agreed on.
     */
    static QByteArray customSection(const QByteArray& wasm, const QByteArray& name);

    /**
     * The names of every custom section, in the order they appear.
     *
     * Useful for showing a user what is actually in the file when the section
     * they expected is not — which, for a contract built with the wrong target
     * or an old SDK, is exactly the question they need answered.
     */
    static QStringList customSectionNames(const QByteArray& wasm);
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(WasmSections)

#endif // QSTELLAR_WASMSECTIONS_H
