#ifndef QSTELLAR_OPERATIONFACTORY_H
#define QSTELLAR_OPERATIONFACTORY_H

#include <QString>
#include <QVariantMap>
#include <initializer_list>

#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

class AssetRegistry;
class Operation;

/**
 * Builds one operation from the plain map a QML form produced.
 *
 * Kept apart from TxBuilderQml::build() because it is the only part that grows:
 * build() is a dozen lines about sequence numbers, fees and memos that will not
 * change again, and this is thirty branches that gain one every time the
 * protocol does.
 *
 * Throws std::runtime_error on anything it cannot build. That is safe here and
 * nowhere above: build() catches everything and turns it into an { error } map,
 * because an exception reaching the QML engine is undefined behaviour.
 *
 * OWNERSHIP, which is not uniform in this SDK and has caused real crashes:
 * assets come from the registry, which owns them for the process lifetime, and
 * almost every operation constructor copies an Asset into XDR rather than
 * taking it. Those must not be deleted here. The exceptions are called out at
 * the branches that need them — LiquidityPoolShareChangeTrustAsset takes
 * ownership of the two assets it is handed, so those come from copyOf().
 *
 * The returned Operation* is the caller's; TransactionBuilder::addOperation
 * takes it.
 */
Operation* makeOperation(const QVariantMap& op, AssetRegistry* assets);

/**
 * The one way this layer reads an integer out of QML: exactly the number the
 * caller meant, or an exception naming `what`. Never a nearby number.
 *
 * Accepted: an integer QVariant (int, qlonglong, ...), as is; a double only
 * when it is finite, whole and below 2^53 in magnitude (Number.isSafeInteger:
 * 2^53 itself is where 2^53 + 1 rounds to); text of ASCII digits with an optional leading
 * '-', which is exact at any size and is how 64-bit values should travel.
 * Everything else — fractions, NaN, a bool, a list, "12abc", "1e5", "", -0 —
 * is refused, and so is any value outside [min, max].
 *
 * Why so strict: every conversion this replaces dropped a failure flag or
 * narrowed a type. A negative base fee became 4294967291 stroops, a blank
 * timeout became "never expires", u32 4294967296 became 0, a memo id above
 * 2^53 became a different memo id — each signed without a word. And the
 * obvious fix of reading the value as text breaks the other way: Qt 6 prints
 * the JS number 100000 as "1e+05".
 */
qint64 exactSigned(const QVariant& value, const QString& what, qint64 min, qint64 max);
quint64 exactUnsigned(const QVariant& value, const QString& what, quint64 max);

/**
 * Whether a field was left out: absent, JS undefined or JS null. A string —
 * even an empty one, which Qt 5 hands over as a null QString — is never
 * "left out": it is something the caller typed.
 */
bool isMissing(const QVariant& value);

/**
 * Refuses an argument a call does not read, naming it. An unknown key used to
 * be ignored: a misspelt one ("netwrok", "networkPasphrase") left the default
 * in place (testnet) and the call went on without a word.
 */
void refuseUnknownArgs(const QVariantMap& args, std::initializer_list<const char*> allowed,
                       const QString& call);

/**
 * Text exactly as the caller meant it: a string as it is, an integer (or a
 * whole JS number below 2^53) written out in full, and nothing else. What it
 * replaces, QVariant::toString(), printed the JS number 1000000 as "1e+06"
 * (and 100000 as "1e+05" on Qt 6 but not Qt 5) and 0.1 + 0.2 as
 * "0.30000000000000004" — into memos, data entries and contract arguments.
 * Left out gives an empty string; the caller decides what that means.
 */
QString exactText(const QVariant& value, const QString& what);

/**
 * Hex bytes, strictly: an even number of the digits 0-9, a-f, A-F and nothing
 * else, or it throws naming `what`. (QByteArray::fromHex skips what it does
 * not understand, so "0xab" and a digit short decoded to other bytes.)
 */
QByteArray strictHex(const QString& text, const QString& what);

/**
 * The network a QML call names, one vocabulary for every facade: "testnet"
 * (also when both are left out), "public" or "mainnet", "futurenet", or an
 * explicit passphrase. Any other name throws — "pubnet" or a typo used to
 * mean testnet in silence — and so does a name that contradicts the
 * passphrase given with it. The Network is owned here; never delete it.
 */
class Network;
Network* qmlNetwork(const QString& name, const QString& passphrase);

QSTELLAR_END_NS

#endif // QSTELLAR_OPERATIONFACTORY_H
