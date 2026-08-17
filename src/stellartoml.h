#ifndef STELLARTOML_H
#define STELLARTOML_H

#include <QString>
#include <QVariantMap>

#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * SEP-1 — `stellar.toml`.
 *
 * `parse()` reads the subset of TOML that real `stellar.toml` files use and
 * returns a nested `QVariantMap`:
 *   - scalars  → QString / bool / qlonglong / double
 *   - arrays   → QVariantList
 *   - `[Table]` → nested QVariantMap (under the table name)
 *   - `[[Array]]` → QVariantList of QVariantMap (e.g. CURRENCIES, VALIDATORS)
 *
 * Supported: bare/quoted keys, basic strings (incl. `"""multi-line"""`),
 * arrays (incl. spanning lines), integers, booleans, comments (`#`, quote-
 * aware). NOT supported (rare in stellar.toml): inline tables, dotted keys,
 * datetimes, nested `[a.b]` tables. The output shape matches what
 * py-stellar-sdk / tomllib produce, so it is interoperable.
 *
 * Network resolution (`fetch https://<domain>/.well-known/stellar.toml`) is a
 * separate follow-up; this provides the parse half, fully unit-testable.
 */
class StellarToml
{
public:
    /** Parses stellar.toml text. Throws std::runtime_error on malformed input. */
    static QVariantMap parse(const QString& toml);
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(StellarToml)

#endif // STELLARTOML_H
