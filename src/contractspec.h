#ifndef QSTELLAR_CONTRACTSPEC_H
#define QSTELLAR_CONTRACTSPEC_H

#include <QByteArray>
#include <QList>
#include <QString>

#include "qstellar_namespace.h"
#include "xdr/contractspec.h"

QSTELLAR_BEGIN_NS

/**
 * The interface a compiled contract carries inside itself.
 *
 * A Soroban contract is a WebAssembly module with its own description bolted
 * on: the machine-readable ABI in a custom section called `contractspecv0`, the
 * environment version it was built against in `contractenvmetav0`, and
 * whatever the author chose to record in `contractmetav0`. So a tool handed
 * nothing but a .wasm can still say what the contract exports, what each
 * function takes and what it gives back — no registry, no source, no compiler.
 *
 * That is what this class is for: turning those bytes into something a form can
 * be generated from.
 *
 * TWO THINGS ABOUT THE SPEC SECTION THAT SURPRISE PEOPLE. It is a bare
 * CONCATENATION of XDR-encoded entries — no count, no separators, no index —
 * because each Rust macro emits its own static into the same link section and
 * the linker glues them together. A reader that decodes one entry gets the
 * first function and silently loses the rest of the interface. And the order
 * is link order, not source order, so it carries no meaning.
 *
 * The other: the spec section is OPTIONAL as far as the network is concerned.
 * Only `contractenvmetav0` is enforced on upload. A contract with no spec runs
 * perfectly well and simply cannot be introspected, which is a different
 * diagnosis from "this is not a contract" and deserves a different message.
 */
class ContractSpec
{
public:
    /** One argument of one function, already flattened for a form. */
    struct Argument
    {
        QString name;
        /** The type as a person reads it: "u32", "vec<address>", "Option<i128>". */
        QString type;
        /**
         * The argument type the transaction builder understands, or empty when
         * there is no simple mapping — a user-defined struct, say. Empty means
         * a form must fall back to asking for raw JSON rather than pretending.
         */
        QString scvType;
        QString doc;
    };

    struct Function
    {
        QString name;
        QList<Argument> inputs;
        /** Human-readable return type, or empty when the function returns nothing. */
        QString returns;
        QString doc;
    };

    /**
     * Everything worth knowing about a module, and whether it is one at all.
     *
     * `problem` is empty when the file parsed. When it is not, it says which
     * of the two failures happened — not a contract, or a contract with no
     * published interface — because that distinction is the whole diagnosis.
     */
    struct Contract
    {
        bool isWasm = false;
        bool hasEnvMeta = false;
        bool hasSpec = false;
        quint32 protocolVersion = 0;
        quint32 preRelease = 0;
        QList<Function> functions;
        /** Author-supplied key/value pairs: name, version, SDK version, and so on. */
        QList<QPair<QString, QString>> metadata;
        QStringList sectionNames;
        QString problem;
    };

    /** Reads a whole .wasm. Never throws: a bad file comes back as `problem`. */
    static Contract read(const QByteArray& wasm);

    /**
     * The entries of a `contractspecv0` payload, decoded until the bytes run
     * out. Throws std::runtime_error on anything malformed.
     */
    static QList<stellar::SCSpecEntry> decodeSpec(const QByteArray& specSection);

    /** A type as a person reads it. */
    static QString describeType(const stellar::SCSpecTypeDef& type);

    /**
     * The transaction builder's argument type for a spec type, or empty when
     * there is no honest mapping.
     */
    static QString scvTypeFor(const stellar::SCSpecTypeDef& type);
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(ContractSpec)

#endif // QSTELLAR_CONTRACTSPEC_H
