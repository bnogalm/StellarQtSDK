#ifndef QSTELLAR_STELLARQML_H
#define QSTELLAR_STELLARQML_H

#include <QObject>
#include <QString>
#include <QVariantMap>

#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

class AssetRegistry;

/**
 * QML facade over the stateless parts of the SDK — the ones a UI calls on every
 * keystroke: key generation, strkey validation, amount and price parsing,
 * mnemonics.
 *
 * Two rules govern everything here, and they are the reason this class exists
 * rather than QML calling the SDK directly:
 *
 * 1. NOTHING THROWS INTO QML. The SDK signals invalid input with exceptions
 *    (Operation::toXdrAmount, Price(QString), StrKey decoders); an exception
 *    crossing into the QML engine is undefined behaviour at worst and an
 *    unhelpful crash at best. Every method here returns the failure as a value:
 *    a bool, an empty string, or a QVariantMap carrying an "error" key. The
 *    SDK's own message is preserved verbatim, because that message is part of
 *    what a laboratory-style app is built to show.
 *
 * 2. NO SDK POINTERS CROSS THE BOUNDARY. QML cannot express ownership, and
 *    Asset ownership in particular is inconsistent across the SDK. Assets are
 *    named by canonical string and resolved through AssetRegistry.
 *
 * Registered as the `Stellar` singleton by QStellarQml::registerTypes().
 */
class StellarQml : public QObject
{
    Q_OBJECT
public:
    explicit StellarQml(QObject* parent = nullptr);
    ~StellarQml() override;

    // ─── Keys ────────────────────────────────────────────────────────

    /**
     * A fresh random secret seed (S...).
     *
     * Note the SDK's own warning on KeyPair::random(): it is documented as
     * unsuitable for anything but prototyping and tests. This wrapper exists so
     * a UI can generate throwaway testnet keys; do not build a wallet on it.
     */
    Q_INVOKABLE QString randomSecretSeed() const;

    /** Account id (G...) for a secret seed. Empty string if the seed is invalid. */
    Q_INVOKABLE QString accountIdFromSecretSeed(const QString& secretSeed) const;

    /** {accountId, secretSeed} for a fresh key, or {error} on failure. */
    Q_INVOKABLE QVariantMap generateKeyPair() const;

    // ─── Address validation (never throws) ───────────────────────────

    Q_INVOKABLE bool isValidAccountId(const QString& value) const;
    Q_INVOKABLE bool isValidSecretSeed(const QString& value) const;
    Q_INVOKABLE bool isValidMuxedAccount(const QString& value) const;
    Q_INVOKABLE bool isValidContractId(const QString& value) const;
    Q_INVOKABLE bool isValidClaimableBalanceId(const QString& value) const;
    Q_INVOKABLE bool isValidLiquidityPoolId(const QString& value) const;

    /**
     * Which kind of strkey this is: "account", "secret", "muxed", "contract",
     * "claimableBalance", "liquidityPool", "preAuthTx", "sha256Hash",
     * "signedPayload" (the three signer-key forms), or "" when it is none of
     * them.
     * Lets a single address field colour itself and explain what it accepted.
     */
    Q_INVOKABLE QString strKeyKind(const QString& value) const;

    // ─── Amounts ─────────────────────────────────────────────────────

    /**
     * Empty string when `amount` is a valid Stellar amount, otherwise the SDK's
     * own error message. This is the primitive an amount field binds to: the
     * SDK is the single source of truth on what is acceptable, including the
     * cases it deliberately rejects such as "1,000.5". A negative amount is a
     * valid amount here (an int64 of stroops); the operations that move one
     * (payment, offers, ...) refuse it in Tx.build.
     */
    Q_INVOKABLE QString validateAmount(const QString& amount) const;

    /**
     * Stroops for a decimal amount, AS A STRING.
     *
     * Not a number: an int64 of stroops passes 2^53 at about 900 million XLM,
     * which is inside the range the network actually uses, and a JavaScript
     * number would round it silently. Everything else in this layer already
     * crossed as text for exactly this reason; this one did not.
     *
     * Empty on a malformed amount — validateAmount() carries the reason.
     */
    Q_INVOKABLE QString amountToStroops(const QString& amount) const;

    /** Decimal amount for a stroop count. */
    Q_INVOKABLE QString stroopsToAmount(const QString& stroops) const;

    // ─── Prices ──────────────────────────────────────────────────────

    /**
     * Parses a decimal price into the fraction the XDR actually carries:
     * { n, d, text, error }. `error` is set and the rest omitted on failure.
     *
     * Worth surfacing in a UI: a Stellar price IS a fraction, and the decimal
     * string may only approximate one: Price(QString) is exact when the
     * decimal is an int32 fraction and otherwise takes a continued-fraction
     * approximation.
     * Showing the resulting n/d is how a user sees which fraction was meant —
     * "0.3333333333" becomes 1/3, "0.7777777778" becomes 7/9.
     */
    Q_INVOKABLE QVariantMap parsePrice(const QString& price) const;

    /**
     * Decimal rendering of an exact fraction, as js-stellar-sdk prints a
     * price (Price::toString): (1, 2) is "0.5", (2, 1) is "2", (1, 3) is
     * "0.33333333333333333333".
     *
     * Each term must be a whole number in the int32 range, given as a number
     * or as integer text (this layer passes int64 values as text). Anything
     * else (1.9, 2147483648, NaN, true, "abc") and a zero denominator give an
     * empty string. These used to be coerced through ToInt32, so 1.9 showed
     * as "1." and 2147483648 as "-2147483648.", prices nobody gave.
     */
    Q_INVOKABLE QString priceToString(const QVariant& numerator, const QVariant& denominator) const;

    // ─── Assets ──────────────────────────────────────────────────────

    /** True if the canonical form ("native", "CODE:ISSUER") parses. */
    Q_INVOKABLE bool isValidAsset(const QString& canonicalForm) const;

    /** { type, code, issuer } for a canonical form, or { error }. */
    Q_INVOKABLE QVariantMap describeAsset(const QString& canonicalForm) const;

    // ─── Mnemonics (BIP-39) ──────────────────────────────────────────

    /**
     * A fresh phrase. `strengthBits` in {128,160,192,224,256}; 128 = 12 words.
     * Read exactly (see exactSigned): 128.5 or 2^32 + 128 is refused, not
     * narrowed. Empty text on any refusal.
     */
    Q_INVOKABLE QString generateMnemonic(const QVariant& strengthBits = QVariant(128)) const;

    Q_INVOKABLE bool validateMnemonic(const QString& phrase) const;

    /**
     * Derives {accountId, secretSeed} for SEP-5 path m/44'/148'/index', or
     * { error }. `passphrase` is the optional BIP-39 passphrase, NFKD as
     * BIP-39 requires; a wallet created before 1.0.4 with a passphrase for
     * which legacyPassphraseDerivesDifferently() is true is found with
     * keyPairFromMnemonicLegacyPassphrase(). `index` is read exactly, from 0
     * to 2^31 - 1 (the path is hardened): a JS number is not narrowed, so
     * 2^31, -2^31 or 2^32 + 100 are refused instead of deriving account 0 or
     * 100. The map also carries the `index` and `path` derived.
     */
    Q_INVOKABLE QVariantMap keyPairFromMnemonic(const QString& phrase,
                                                const QVariant& index = QVariant(0),
                                                const QString& passphrase = QString()) const;

    /**
     * keyPairFromMnemonic as 1.0.0-1.0.3 derived it, for recovering wallets
     * created with those versions; NOT BIP-39. They used the passphrase as
     * typed rather than NFKD-normalised, so for a passphrase with, say, a
     * precomposed accented letter the accounts are others than
     * keyPairFromMnemonic finds (see Util::mnemonicToBIP39SeedLegacyPassphrase).
     * The same accounts for an ASCII passphrase. { accountId, secretSeed,
     * legacy: true } or { error }. Offer it to find and move old funds, never
     * to create a wallet. `index` is read as keyPairFromMnemonic reads it.
     */
    Q_INVOKABLE QVariantMap keyPairFromMnemonicLegacyPassphrase(const QString& phrase,
                                                                const QVariant& index = QVariant(0),
                                                                const QString& passphrase = QString()) const;

    /**
     * True when keyPairFromMnemonic and keyPairFromMnemonicLegacyPassphrase
     * derive different accounts from this passphrase: when a wallet created
     * with it before 1.0.4 is only found through the legacy derivation.
     */
    Q_INVOKABLE bool legacyPassphraseDerivesDifferently(const QString& passphrase) const;

    // ─── Networks ────────────────────────────────────────────────────

    /** Passphrase for "testnet", "public" or "futurenet"; empty if unknown. */
    Q_INVOKABLE QString networkPassphrase(const QString& network) const;

    /** The registry backing asset resolution. Not exposed to QML. */
    AssetRegistry* assetRegistry() const { return m_assets; }

private:
    AssetRegistry* m_assets;
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(StellarQml)

#endif // QSTELLAR_STELLARQML_H
