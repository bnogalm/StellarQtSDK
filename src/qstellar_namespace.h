#ifndef QSTELLAR_NAMESPACE_H
#define QSTELLAR_NAMESPACE_H

/*
 * StellarQtSDK 1.0.0 — public namespace control
 * =============================================
 *
 * Every public class in the SDK lives inside the `qstellar::` namespace.
 * The namespace itself is always present — there is no build mode in
 * which the classes live in the global namespace.
 *
 * STELLAR_QT_LEGACY_GLOBAL_NS
 * ---------------------------
 * Source-compatibility flag for downstream code written against the
 * unqualified 0.x API. When this macro is defined BEFORE any SDK header
 * is included (or via a compiler `-DSTELLAR_QT_LEGACY_GLOBAL_NS`), the
 * SDK additionally emits a global `using` alias for every public class
 * (and a `namespace Foo = ::qstellar::Foo` alias for helper namespaces),
 * so that unqualified names like `KeyPair`, `Asset`, `Server`, ... keep
 * resolving.
 *
 * Without the flag (the default), only the qualified `qstellar::Foo`
 * spellings are exported. Downstream code that was written against the
 * 0.x layout must either:
 *   - migrate to `qstellar::Foo` qualified names, or
 *   - `using namespace qstellar;` per translation unit, or
 *   - define `STELLAR_QT_LEGACY_GLOBAL_NS` to opt in to the global
 *     aliases.
 *
 * This SDK's own test suite (`StellarQtSDKTest.pro`) and the bundled
 * examples are built with the flag defined so that legacy unqualified
 * names keep working. New SDK consumers should NOT define it.
 *
 * Macros
 * ------
 *
 *   QSTELLAR_BEGIN_NS / QSTELLAR_END_NS
 *     Always expand to `namespace qstellar { ... }`. Use to wrap class
 *     definitions and free helpers that belong to the SDK surface.
 *
 *   QSTELLAR_ALIAS(Cls)
 *     With the legacy flag: `using ::qstellar::Cls;` at global scope.
 *     Without it: nothing.
 *     Use after `QSTELLAR_END_NS` for every public class.
 *
 *   QSTELLAR_NS_ALIAS(Ns)
 *     With the legacy flag: `namespace Ns = ::qstellar::Ns;` at global
 *     scope. Without it: nothing.
 *     Use for helper namespaces like `AccountResponseAttach` or
 *     `ResponseConverters` that live inside `qstellar::` and are
 *     referenced unqualified by 0.x callers.
 *
 *   QSTELLAR_FWD(Cls)
 *     Forward-declares `qstellar::Cls`. With the legacy flag it also
 *     emits a global `using ::qstellar::Cls;`. Use at global scope only
 *     — invoking it inside an open `QSTELLAR_BEGIN_NS` block creates a
 *     nested `qstellar::qstellar::Cls`.
 *
 * Q_DECLARE_METATYPE
 * ------------------
 * Qt's macro must remain at global scope (it expands to a template
 * specialisation of `QMetaTypeId<T>` that has to live outside
 * `qstellar::`). The type argument should always be qualified with
 * `qstellar::` so the call site resolves identically under both modes
 * — see e.g. `Q_DECLARE_METATYPE(qstellar::AssetAmount)`.
 *
 * The XDR layer (`namespace stellar { ... }`, in `xdr/*.h`) is an
 * implementation detail and is NOT touched by this scheme.
 */

#define QSTELLAR_BEGIN_NS namespace qstellar {
#define QSTELLAR_END_NS   }

#ifdef STELLAR_QT_LEGACY_GLOBAL_NS

#  define QSTELLAR_ALIAS(Cls)    using ::qstellar::Cls;
#  define QSTELLAR_NS_ALIAS(Ns)  namespace Ns = ::qstellar::Ns;
#  define QSTELLAR_FWD(Cls)      namespace qstellar { class Cls; } \
                                 using ::qstellar::Cls;

#else  // strict mode (default)

#  define QSTELLAR_ALIAS(Cls)
#  define QSTELLAR_NS_ALIAS(Ns)
#  define QSTELLAR_FWD(Cls)      namespace qstellar { class Cls; }

#endif

#endif // QSTELLAR_NAMESPACE_H
