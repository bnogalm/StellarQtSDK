#ifndef QSTELLAR_OPERATIONTABLE_H
#define QSTELLAR_OPERATIONTABLE_H

#include <QString>
#include <QVector>

#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * What every operation the QML layer can build is called, and what it takes.
 *
 * This table exists so that a UI does not have to keep its own list. A form
 * that hardcodes "payment needs a destination, an asset and an amount" is a
 * second source of truth, and the moment the SDK gains an operation or changes
 * a field the two disagree silently — the form still renders, the build still
 * fails, and nothing says why.
 *
 * So the table drives three things at once: which types build() accepts, which
 * inputs a form draws, and what a demo mode fills them with. Adding an
 * operation means adding a row here and a branch in makeOperation(); leaving
 * either out is caught by the test that walks the table and builds every row.
 */
struct FieldSpec
{
    const char* name;       ///< key in the operation spec map
    const char* kind;       ///< which editor a form should draw (see below)
    const char* label;      ///< what the form calls it
    bool required;
    const char* note;       ///< the thing callers get wrong: units, encoding, range
    const char* demo;       ///< a plausible testnet value, or ""

    /*
     * kind is one of:
     *   address    G... account id (some accept M... muxed)
     *   asset      canonical "native" or "CODE:GISSUER"
     *   assetList  several of the above, comma separated
     *   amount     decimal string, seven places — never stroops at this layer
     *   price      decimal string, or n/d through priceR
     *   text       free text
     *   uint32 / uint64 / int64   digits, carried as TEXT because a JavaScript
     *              number cannot hold an int64 and several of these are int64
     *   bool       on or off
     *   hex32 / hex64   hex-encoded 32- or 64-byte value
     *   poolId     64-hex liquidity pool id
     *   balanceId  72-hex claimable balance id
     *   signerKey  G... / T... / X... / P... strkey
     *   claimants  JSON array of { destination, predicate }
     *   dataValue  value of a data entry, up to 64 bytes once decoded; the
     *              operation's valueFormat field says whether it is text
     *              (UTF-8, the default), hex or base64
     *   flags      uint32 bitfield
     */
};

struct OpSpec
{
    const char* type;         ///< camelCase key used in the spec map
    const char* displayName;
    const char* summary;      ///< one line a form can show under the picker
    bool deprecated;
    QVector<FieldSpec> fields;
};

/** Every buildable operation, in the order a picker should offer them. */
const QVector<OpSpec>& operationTable();

/** The row for `type`, or nullptr. */
const OpSpec* operationSpec(const QString& type);

QSTELLAR_END_NS

#endif // QSTELLAR_OPERATIONTABLE_H
