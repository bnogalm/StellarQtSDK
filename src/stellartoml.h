#ifndef STELLARTOML_H
#define STELLARTOML_H

#include <QString>
#include <QVariantMap>

#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * SEP-1 — `stellar.toml`.
 *
 * `parse()` reads a TOML 1.0 document and returns it as a nested
 * `QVariantMap`. Every construct of TOML 1.0 is read:
 *
 *   - basic strings (`"..."`, `"""..."""`) with all TOML escapes, including
 *     `\uXXXX` / `\UXXXXXXXX`, and line-ending backslashes; literal strings
 *     (`'...'`, `'''...'''`)                          → QString
 *   - integers: decimal, `0x` / `0o` / `0b`, `_` separators
 *                                                      → qlonglong
 *   - floats, including `inf` / `nan` with a sign      → double
 *   - `true` / `false`                                 → bool
 *   - offset date-times, local date-times, local dates and local times
 *                                                      → QString holding the
 *     value's ORIGINAL TEXT exactly as written in the file (for example
 *     `1979-05-27T07:32:00-08:00`). Python's tomllib returns datetime objects
 *     here; a QString keeps the value lossless and lets the caller pick the
 *     date type it wants. The text has been validated as a real date/time.
 *   - arrays                                           → QVariantList
 *   - `[table]`, `[a.b]`, dotted keys (`a.b = 1`) and inline tables
 *     (`{ a = 1 }`)                                    → nested QVariantMap
 *   - `[[array.of.tables]]` → QVariantList of QVariantMap (e.g. CURRENCIES,
 *     VALIDATORS)
 *
 * Anything that is not valid TOML 1.0 is REFUSED with a std::runtime_error
 * whose message names the line: a duplicate key, a table declared twice, an
 * unknown escape, junk after a value, an unterminated string or header, a
 * bare `\r`, a control character, and so on. Nothing is guessed.
 *
 * Deliberate differences from Python's tomllib (which otherwise agrees on
 * every document, as checked by differential testing):
 *   - date-times come back as their original text (see above);
 *   - an integer outside the signed 64-bit range is refused (tomllib returns
 *     an arbitrary-precision int; the old parser here silently rounded it);
 *   - nesting is capped at 64 levels (keys, tables, arrays and inline tables
 *     combined), so a hostile file cannot exhaust the stack.
 *
 * Fetching a domain's stellar.toml is StellarTomlResolver's
 * (stellartomlresolver.h): https only, no redirect, a timeout and a 100 KB cap,
 * as js's StellarToml.Resolver.
 */
class StellarToml
{
public:
    /** Parses stellar.toml text. Throws std::runtime_error (naming the line)
     *  on anything that is not valid TOML 1.0. */
    static QVariantMap parse(const QString& toml);

    /** Maximum nesting depth accepted by parse(). */
    static constexpr int MaxDepth = 64;

    /**
     * SEP-1: the [[CURRENCIES]] entry of `toml` for the asset `code` issued
     * by `issuer`, empty when there is none. An entry matches on its
     * `issuer` and on its `code`, or on its `code_template`, where each `?`
     * stands for any one character (SEP-1's tokenized asset classes).
     *
     * This is how a wallet checks an asset against its issuer: read the
     * issuer account's home_domain, that domain's stellar.toml
     * (StellarTomlResolver), and look the asset up here. An asset the toml
     * does not list is not one the domain vouches for, whatever its code.
     *
     * Only the entries written in `toml` are read. One that links out to a
     * file of its own (`toml = "https://DOMAIN/.well-known/CURRENCY.toml"`,
     * which SEP-1 allows) is not followed, and one known by its Soroban
     * `contract` alone has no code and issuer to match. When `toml` has such
     * entries, an empty result does not mean the asset is not listed: fetch
     * the linked file, parse() it, and look there. js and py have no such
     * helper.
     */
    static QVariantMap currency(const QVariantMap& toml, const QString& code, const QString& issuer);
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(StellarToml)

#endif // STELLARTOML_H
