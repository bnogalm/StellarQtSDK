#include "stellarqml.h"

#include <QScopedPointer>
#include <cmath>
#include <limits>
#include <stdexcept>

#include "assetregistry.h"
#include "operationfactory.h"
#include "asset.h"
#include "assettypecreditalphanum.h"
#include "assettypenative.h"
#include "keypair.h"
#include "network.h"
#include "operation.h"
#include "price.h"
#include "strkey.h"
#include "util.h"

QSTELLAR_BEGIN_NS

namespace {

/** Runs `fn` and reports failure the way QML can consume: a map with "error". */
template <typename Fn>
QVariantMap guarded(Fn fn)
{
    try {
        return fn();
    } catch (const std::exception& e) {
        QVariantMap out;
        out.insert(QStringLiteral("error"), QString::fromUtf8(e.what()));
        return out;
    } catch (...) {
        // A pointer-to-exception, which this SDK threw in six places until
        // recently and which the arm above does not match.
        QVariantMap out;
        out.insert(QStringLiteral("error"), QStringLiteral("unknown failure"));
        return out;
    }
}

/**
 * A price term from QML: a whole number in the int32 range, as a number or as
 * integer text. Refused rather than coerced; ToInt32 turned 1.9 into 1 and
 * 2147483648 into -2147483648.
 */
bool wholeInt32(const QVariant& v, qint32& out)
{
    qint64 whole = 0;
    switch (v.userType()) {
    case QMetaType::Int:
    case QMetaType::LongLong:
        whole = v.toLongLong();
        break;
    case QMetaType::UInt:
    case QMetaType::ULongLong:
        if (v.toULongLong() > quint64(std::numeric_limits<qint32>::max()))
            return false;
        whole = qint64(v.toULongLong());
        break;
    case QMetaType::Double:
    case QMetaType::Float: {
        const double x = v.toDouble();
        if (!std::isfinite(x) || x != std::trunc(x)
                || x < std::numeric_limits<qint32>::min()
                || x > std::numeric_limits<qint32>::max())
            return false;
        whole = qint64(x);
        break;
    }
    case QMetaType::QString: {
        bool ok = false;
        whole = v.toString().trimmed().toLongLong(&ok, 10);
        if (!ok)
            return false;
        break;
    }
    default:
        return false;       // bool, null, undefined, objects
    }
    if (whole < std::numeric_limits<qint32>::min() || whole > std::numeric_limits<qint32>::max())
        return false;
    out = qint32(whole);
    return true;
}

/** True when `fn` completes without throwing. */
template <typename Fn>
bool accepts(Fn fn)
{
    try {
        fn();
        return true;
    } catch (const std::exception&) {
        return false;
    } catch (...) {
        return false;
    }
}

} // namespace

StellarQml::StellarQml(QObject* parent)
    : QObject(parent), m_assets(new AssetRegistry())
{
}

StellarQml::~StellarQml()
{
    delete m_assets;
}

// ─── Keys ────────────────────────────────────────────────────────────

QString StellarQml::randomSecretSeed() const
{
    QScopedPointer<KeyPair> kp(KeyPair::random());
    return kp->getSecretSeed();
}

QString StellarQml::accountIdFromSecretSeed(const QString& secretSeed) const
{
    try {
        QScopedPointer<KeyPair> kp(KeyPair::fromSecretSeed(secretSeed));
        return kp->getAccountId();
    } catch (const std::exception&) {
        return QString();
    } catch (...) {
        return QString();
    }
}

QVariantMap StellarQml::generateKeyPair() const
{
    return guarded([]() {
        QScopedPointer<KeyPair> kp(KeyPair::random());
        QVariantMap out;
        out.insert(QStringLiteral("accountId"), kp->getAccountId());
        out.insert(QStringLiteral("secretSeed"), kp->getSecretSeed());
        return out;
    });
}

// ─── Address validation ──────────────────────────────────────────────

bool StellarQml::isValidAccountId(const QString& value) const
{
    return accepts([&]() { StrKey::decodeStellarAccountId(value); });
}

bool StellarQml::isValidSecretSeed(const QString& value) const
{
    return accepts([&]() { StrKey::decodeStellarSecretSeed(value.toUtf8()); });
}

bool StellarQml::isValidMuxedAccount(const QString& value) const
{
    return accepts([&]() { StrKey::decodeStellarMuxedAccount(value); });
}

bool StellarQml::isValidContractId(const QString& value) const
{
    return accepts([&]() { StrKey::decodeContract(value); });
}

bool StellarQml::isValidClaimableBalanceId(const QString& value) const
{
    return accepts([&]() { StrKey::decodeClaimableBalance(value); });
}

bool StellarQml::isValidLiquidityPoolId(const QString& value) const
{
    return accepts([&]() { StrKey::decodeLiquidityPool(value); });
}

QString StellarQml::strKeyKind(const QString& value) const
{
    if (value.isEmpty()) return QString();
    // Ordered by strkey version byte, which is unambiguous: no well-formed
    // value can satisfy two of these.
    if (isValidAccountId(value))          return QStringLiteral("account");
    if (isValidSecretSeed(value))         return QStringLiteral("secret");
    if (isValidMuxedAccount(value))       return QStringLiteral("muxed");
    if (isValidContractId(value))         return QStringLiteral("contract");
    if (isValidClaimableBalanceId(value)) return QStringLiteral("claimableBalance");
    if (isValidLiquidityPoolId(value))    return QStringLiteral("liquidityPool");
    // The signer-key forms (SEP-23): a pre-authorised transaction's hash (T),
    // a hash-x preimage's hash (X), and an ed25519 key with the payload it
    // must sign (P). They were reported as no strkey at all.
    try {
        if (StrKey::decodePreAuthTx(value).size() == 32) return QStringLiteral("preAuthTx");
    } catch (...) {}
    try {
        if (StrKey::decodeSha256Hash(value).size() == 32) return QStringLiteral("sha256Hash");
    } catch (...) {}
    try {
        StrKey::decodeSignedPayload(value);
        return QStringLiteral("signedPayload");
    } catch (...) {}
    return QString();
}

// ─── Amounts ─────────────────────────────────────────────────────────

QString StellarQml::validateAmount(const QString& amount) const
{
    try {
        Operation::toXdrAmount(amount);
        return QString();
    } catch (const std::exception& e) {
        return QString::fromUtf8(e.what());
    } catch (...) {
        return QStringLiteral("unknown failure");
    }
}

QString StellarQml::amountToStroops(const QString& amount) const
{
    try {
        // As text. An int64 of stroops passes 2^53 at around 900 million XLM,
        // which is inside the range the network uses, and a JavaScript number
        // would round it without saying so. The -1 sentinel this used to
        // return is gone too: validateAmount() already carries the reason, and
        // a sentinel that looks like a value is how a bad amount travels.
        return QString::number(Operation::toXdrAmount(amount));
    } catch (const std::exception&) {
        return QString();
    } catch (...) {
        return QString();
    }
}

QString StellarQml::stroopsToAmount(const QString& stroops) const
{
    bool ok = false;
    const qint64 value = stroops.trimmed().toLongLong(&ok);
    if (!ok)
        return QString();
    try {
        return Operation::fromXdrAmount(value);
    } catch (const std::exception&) {
        return QString();
    } catch (...) {
        return QString();
    }
}

// ─── Prices ──────────────────────────────────────────────────────────

QVariantMap StellarQml::parsePrice(const QString& price) const
{
    return guarded([&]() {
        const Price p(price);
        QVariantMap out;
        out.insert(QStringLiteral("n"), p.getNumerator());
        out.insert(QStringLiteral("d"), p.getDenominator());
        out.insert(QStringLiteral("text"), p.toString());
        return out;
    });
}

QString StellarQml::priceToString(const QVariant& numerator, const QVariant& denominator) const
{
    qint32 n = 0, d = 0;
    if (!wholeInt32(numerator, n) || !wholeInt32(denominator, d) || d == 0)
        return QString();
    try {
        return Price::toString(n, d);
    } catch (const std::exception&) {
        return QString();
    } catch (...) {
        return QString();
    }
}

// ─── Assets ──────────────────────────────────────────────────────────

bool StellarQml::isValidAsset(const QString& canonicalForm) const
{
    return AssetRegistry::isValid(canonicalForm);
}

QVariantMap StellarQml::describeAsset(const QString& canonicalForm) const
{
    return guarded([&]() {
        // Resolved through the registry so the pointer stays owned by it.
        Asset* a = m_assets->assetFor(canonicalForm);
        QVariantMap out;
        out.insert(QStringLiteral("type"), a->getType());
        if (auto* alpha = dynamic_cast<AssetTypeCreditAlphaNum*>(a)) {
            out.insert(QStringLiteral("code"), alpha->getCode());
            out.insert(QStringLiteral("issuer"), alpha->getIssuer().getAccountId());
        }
        return out;
    });
}

// ─── Mnemonics ───────────────────────────────────────────────────────

QString StellarQml::generateMnemonic(const QVariant& strengthBits) const
{
    try {
        return Util::generateMnemonic(
            int(exactSigned(strengthBits, QStringLiteral("strengthBits"), 128, 256)));
    } catch (const std::exception&) {
        return QString();
    } catch (...) {
        return QString();
    }
}

bool StellarQml::validateMnemonic(const QString& phrase) const
{
    return Util::validateMnemonic(phrase);
}

namespace {

/**
 * A SEP-5 account index, exactly: 0 to 2^31 - 1. It was an int, which the QML
 * engine fills with ToInt32: 2^31 became -2^31, whose top bit
 * KeyPair::fromBip39Seed takes as "already hardened", so account 0's keys
 * came back for it (and 2^32 + 100 gave account 100's), with nothing in the
 * map to tell.
 */
int accountIndex(const QVariant& index)
{
    return int(exactSigned(index, QStringLiteral("index"), 0, std::numeric_limits<qint32>::max()));
}

QVariantMap derivedKeyPair(const QByteArray& seed, int index)
{
    QScopedPointer<KeyPair> kp(KeyPair::fromBip39Seed(seed, index));
    QVariantMap out;
    out.insert(QStringLiteral("accountId"), kp->getAccountId());
    out.insert(QStringLiteral("secretSeed"), kp->getSecretSeed());
    out.insert(QStringLiteral("index"), index);
    out.insert(QStringLiteral("path"), QStringLiteral("m/44'/148'/%1'").arg(index));
    return out;
}

} // namespace

QVariantMap StellarQml::keyPairFromMnemonic(const QString& phrase, const QVariant& index,
                                            const QString& passphrase) const
{
    return guarded([&]() -> QVariantMap {
        const int i = accountIndex(index);
        if (!Util::validateMnemonic(phrase))
            throw std::runtime_error("invalid mnemonic phrase");
        return derivedKeyPair(Util::mnemonicToBIP39Seed(phrase, passphrase), i);
    });
}

QVariantMap StellarQml::keyPairFromMnemonicLegacyPassphrase(const QString& phrase, const QVariant& index,
                                                            const QString& passphrase) const
{
    return guarded([&]() -> QVariantMap {
        const int i = accountIndex(index);
        if (!Util::validateMnemonic(phrase))
            throw std::runtime_error("invalid mnemonic phrase");
        // The passphrase as typed: what 1.0.0-1.0.3 derived, not BIP-39.
        QVariantMap out = derivedKeyPair(Util::mnemonicToBIP39SeedLegacyPassphrase(phrase, passphrase), i);
        out.insert(QStringLiteral("legacy"), true);
        return out;
    });
}

bool StellarQml::legacyPassphraseDerivesDifferently(const QString& passphrase) const
{
    return Util::legacyPassphraseDerivesDifferently(passphrase);
}

// ─── Networks ────────────────────────────────────────────────────────

QString StellarQml::networkPassphrase(const QString& network) const
{
    const QString n = network.toLower();
    if (n == QLatin1String("testnet"))
        return Network::testnetNetwork()->getNetworkPassphrase();
    if (n == QLatin1String("public") || n == QLatin1String("mainnet"))
        return Network::publicNetwork()->getNetworkPassphrase();
    if (n == QLatin1String("futurenet")) {
        // No Network instance for futurenet (js and py have none either); an
        // app builds its own Network from the passphrase.
        return Network::FUTURENET_NETWORK_PASSPHRASE();
    }
    return QString();
}

QSTELLAR_END_NS
