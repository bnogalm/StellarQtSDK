#include "response.h"
#include "problem.h"
#include "../exception/sdkexception.h"
#include "../exception/networkexception.h"
#include "../exception/toomanyrequestsexception.h"
#include "../exception/badrequestexception.h"
#include "../exception/badresponseexception.h"
#include "../exception/requesttimeoutexception.h"
#include "../exception/unknownresponseexception.h"
#include "../exception/connectionerrorexception.h"
#include "../price.h"
#include "../predicate.h"
#include "../claimant.h"
#include "../strkey.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaProperty>
#include <QMutex>
#include <QMutexLocker>
#include <QNetworkRequest>
#include <QPointer>
#include <QRegularExpression>
#include <QSet>
#include <QTimer>
#include <QTimerEvent>
#include <atomic>
#include <cmath>
#include <cstring>
#include <initializer_list>
#include <limits>
#include <memory>
#include <stdexcept>

QSTELLAR_BEGIN_NS


#define RECONNECT_DELAY 1000

namespace {

std::atomic<int> s_replyTimeout(STELLAR_QT_REPLY_TIMEOUT);

QHash<int, ResponseConverters::ExactReader>& exactReaders()
{
    static QHash<int, ResponseConverters::ExactReader> readers;
    return readers;
}

[[noreturn]] void refuse(const QString& field, const QString& why)
{
    throw std::runtime_error((field.isEmpty() ? why : field + QStringLiteral(": ") + why).toStdString());
}

QString member(const QString& path, const QString& key)
{
    return path.isEmpty() ? key : path + QLatin1Char('.') + key;
}

QString element(const QString& field, int index)
{
    return field + QLatin1Char('[') + QString::number(index) + QLatin1Char(']');
}

/** A JSON value as an error message shows it. */
QString describe(const QJsonValue& value)
{
    switch (value.type()) {
    case QJsonValue::Null: return QStringLiteral("null");
    case QJsonValue::Bool: return value.toBool() ? QStringLiteral("true") : QStringLiteral("false");
    case QJsonValue::Double:
        return QStringLiteral("the number ") + QString::number(value.toDouble(), 'g', 17);
    case QJsonValue::String: {
        QString text = value.toString();
        if (text.size() > 40)
            text = text.left(40) + QStringLiteral("...");
        return QStringLiteral("the text \"") + text + QLatin1Char('"');
    }
    case QJsonValue::Array: return QStringLiteral("an array");
    case QJsonValue::Object: return QStringLiteral("an object");
    default: return QStringLiteral("nothing");
    }
}

/**
 * The whole number `value` holds, as a sign and a magnitude: a JSON number
 * that is whole and below 2^53 in magnitude (past that the parser may already
 * have rounded it: Qt 5 reads every number as a double; Horizon writes such
 * values as text), or decimal text: an optional '-' and digits, nothing else.
 */
bool wholeNumber(const QJsonValue& value, bool& negative, quint64& magnitude, QString& why)
{
    if (value.isDouble()) {
        const double d = value.toDouble();
        if (!std::isfinite(d) || std::floor(d) != d) {
            why = QStringLiteral("expected a whole number, found ") + describe(value);
            return false;
        }
        if (std::fabs(d) >= 9007199254740992.0) {
            why = QStringLiteral("%1 is 2^53 or more, where a JSON number may already have been "
                                 "rounded (Horizon writes such values as text)").arg(describe(value));
            return false;
        }
        negative = d < 0;
        magnitude = static_cast<quint64>(std::fabs(d));
        return true;
    }
    if (value.isString()) {
        const QString text = value.toString();
        negative = text.startsWith(QLatin1Char('-'));
        int i = negative ? 1 : 0;
        if (i >= text.size()) {
            why = QStringLiteral("expected a whole number, found ") + describe(value);
            return false;
        }
        magnitude = 0;
        for (; i < text.size(); ++i) {
            const QChar c = text.at(i);
            if (c < QLatin1Char('0') || c > QLatin1Char('9')) {
                why = QStringLiteral("expected a whole number, found ") + describe(value);
                return false;
            }
            const quint64 digit = static_cast<quint64>(c.unicode() - '0');
            if (magnitude > (std::numeric_limits<quint64>::max() - digit) / 10) {
                why = describe(value) + QStringLiteral(" is out of range");
                return false;
            }
            magnitude = magnitude * 10 + digit;
        }
        return true;
    }
    why = QStringLiteral("expected a whole number, found ") + describe(value);
    return false;
}

qint64 exactInt64(const QJsonValue& value, const QString& field, qint64 min, qint64 max)
{
    bool negative = false;
    quint64 magnitude = 0;
    QString why;
    if (!wholeNumber(value, negative, magnitude, why))
        refuse(field, why);
    const bool inRange = negative
        ? (magnitude == 0 || (min < 0 && magnitude <= quint64(0) - quint64(min)))
        : (max >= 0 && magnitude <= quint64(max));
    if (!inRange)
        refuse(field, QStringLiteral("%1 is outside [%2, %3]").arg(describe(value)).arg(min).arg(max));
    // Two's complement, so -(2^63) comes out right too.
    const qint64 result = negative ? qint64(quint64(0) - magnitude) : qint64(magnitude);
    if (result < min)
        refuse(field, QStringLiteral("%1 is outside [%2, %3]").arg(describe(value)).arg(min).arg(max));
    return result;
}

quint64 exactUInt64(const QJsonValue& value, const QString& field, quint64 max)
{
    bool negative = false;
    quint64 magnitude = 0;
    QString why;
    if (!wholeNumber(value, negative, magnitude, why))
        refuse(field, why);
    if ((negative && magnitude != 0) || magnitude > max)
        refuse(field, QStringLiteral("%1 is outside [0, %2]").arg(describe(value)).arg(max));
    return magnitude;
}

/** A number as a JSON number, or as decimal text ("0.97"); finite. */
double exactDouble(const QJsonValue& value, const QString& field)
{
    if (value.isDouble())
        return value.toDouble();
    if (value.isString()) {
        const QString text = value.toString();
        bool digits = false, point = false, plain = true;
        for (int i = 0; i < text.size(); ++i) {
            const QChar c = text.at(i);
            if (c >= QLatin1Char('0') && c <= QLatin1Char('9'))
                digits = true;
            else if (c == QLatin1Char('.') && !point)
                point = true;
            else if (!(c == QLatin1Char('-') && i == 0))
                plain = false;
        }
        bool ok = false;
        const double d = text.toDouble(&ok);
        if (plain && digits && ok && std::isfinite(d))
            return d;
    }
    refuse(field, QStringLiteral("expected a number, found ") + describe(value));
}

/**
 * An amount as Horizon writes it: decimal text with at most 7 fraction
 * digits (a stroop), never negative. Anything else cannot be the amount; it
 * used to be passed on, for the caller to read as 0.
 */
void checkAmount(const QJsonValue& value, const QString& field)
{
    const QString text = value.toString();
    int point = -1;
    for (int i = 0; i < text.size(); ++i) {
        const QChar c = text.at(i);
        if (c == QLatin1Char('.') && point < 0 && i > 0)
            point = i;
        else if (c < QLatin1Char('0') || c > QLatin1Char('9'))
            refuse(field, QStringLiteral("expected an amount (decimal text), found ") + describe(value));
    }
    if (text.isEmpty() || point == text.size() - 1 || (point >= 0 && text.size() - point - 1 > 7))
        refuse(field, QStringLiteral("expected an amount with at most 7 decimals, found ") + describe(value));
}

/**
 * Members holding an amount, or a price as decimal text, in whichever record
 * (as text properties; the same key held as a number is not checked here).
 * Every text property of a response class that holds one is listed: an
 * amount missing from this list was kept as whatever text came, for the
 * caller to read as 0.
 */
bool isAmountKey(const QString& key)
{
    static const QSet<QString> keys = {
        QStringLiteral("amount"), QStringLiteral("balance"), QStringLiteral("limit"),
        QStringLiteral("buying_liabilities"), QStringLiteral("selling_liabilities"),
        QStringLiteral("starting_balance"), QStringLiteral("source_amount"),
        QStringLiteral("destination_amount"), QStringLiteral("source_max"),
        QStringLiteral("destination_min"), QStringLiteral("base_amount"),
        QStringLiteral("counter_amount"), QStringLiteral("bought_amount"),
        QStringLiteral("sold_amount"), QStringLiteral("shares"), QStringLiteral("shares_received"),
        QStringLiteral("shares_redeemed"), QStringLiteral("shares_revoked"),
        QStringLiteral("total_shares"), QStringLiteral("total_coins"), QStringLiteral("fee_pool"),
        QStringLiteral("base_reserve"), // a ledger's, before Horizon 0.15
        QStringLiteral("base_volume"), QStringLiteral("counter_volume"), QStringLiteral("price"),
        QStringLiteral("min_price"), QStringLiteral("max_price"),
        QStringLiteral("avg"), QStringLiteral("high"), QStringLiteral("low"), QStringLiteral("open"),
        QStringLiteral("close"), QStringLiteral("claimable_balances_amount"),
        QStringLiteral("liquidity_pools_amount"), QStringLiteral("contracts_amount"),
        QStringLiteral("archived_contracts_amount"),
        QStringLiteral("authorized"), QStringLiteral("authorized_to_maintain_liabilities"),
        QStringLiteral("unauthorized")
    };
    return keys.contains(key);
}

const QMetaObject* gadgetMetaObject(int type)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const QMetaType metaType(type);
    return (metaType.flags() & QMetaType::IsGadget) ? metaType.metaObject() : nullptr;
#else
    return (QMetaType::typeFlags(type) & QMetaType::IsGadget) ? QMetaType::metaObjectForType(type)
                                                              : nullptr;
#endif
}

QVariant defaultValue(int type)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return QVariant(QMetaType(type), nullptr);
#else
    return QVariant(type, nullptr);
#endif
}

/** Class name without the namespace ("AccountResponseAttach::Balance"). */
QByteArray shortName(const QMetaObject* mo)
{
    QByteArray name(mo->className());
    if (name.startsWith("qstellar::"))
        name.remove(0, 10);
    return name;
}

bool inherits(const QMetaObject* mo, const char* name)
{
    for (const QMetaObject* m = mo; m; m = m->superClass())
        if (shortName(m) == name)
            return true;
    return false;
}

/**
 * The members Horizon always sends in a record of each class (and of its
 * subclasses): missing or null, the property would read as 0 or empty, which
 * for these is a value a caller acts on: a sequence to sign with, a balance
 * or a fee to show, an offer id to update, a price, a paging token to ask for
 * the next page with.
 *
 * The list is every member of Horizon's resource (stellar/go
 * protocols/horizon, .../operations, .../effects) that has no omitempty, is
 * not a pointer that may render as null, and has a property here, _links
 * included; checked against real documents of every class from SDF's,
 * LOBSTR's, Ankr's and Tatum's Horizons (28.x, 29.x), testnet and
 * futurenet, pages and streams. Not listed: what today's Horizon no longer
 * sends (an asset's amount, num_accounts and archived contracts, a root's
 * protocol_version), slices Go renders as null (an invocation's parameters
 * and asset_balance_changes), and times that are null when Horizon has none
 * (last_modified_time). A ledger's transaction count, base fee and base
 * reserve go by an older name too; checkRecord takes either. An
 * asset/amount pair's asset is omitempty: Horizon leaves it out of a pool
 * deposit's or withdrawal's reserves when the transaction failed, so
 * checkRecord requires it only where Horizon always knows it.
 */
struct RequiredMembers { const char* className; const char* members; };
const RequiredMembers kRequired[] = {
    { "AccountResponse", "account_id sequence subentry_count thresholds flags balances signers data "
                         "num_sponsoring num_sponsored paging_token _links" },
    { "AccountResponseAttach::Thresholds", "low_threshold med_threshold high_threshold" },
    { "AccountResponseAttach::Flags", "auth_required auth_revocable auth_immutable auth_clawback_enabled" },
    { "AccountResponseAttach::Balance", "asset_type balance" },
    { "AccountResponseAttach::Signer", "weight type" },
    { "AccountResponseAttach::Links", "effects offers operations self transactions" },
    { "AssetResponse", "asset_type paging_token accounts balances claimable_balances_amount "
                       "num_claimable_balances liquidity_pools_amount num_liquidity_pools contracts_amount "
                       "num_contracts flags _links" },
    { "AssetResponseAttach::Accounts", "authorized authorized_to_maintain_liabilities unauthorized" },
    { "AssetResponseAttach::Balances", "authorized authorized_to_maintain_liabilities unauthorized" },
    { "AssetResponseAttach::Flags", "auth_required auth_revocable" },
    { "AssetResponseAttach::Links", "toml" },
    { "LedgerResponse", "sequence hash paging_token closed_at total_coins fee_pool operation_count "
                        "max_tx_set_size protocol_version header_xdr _links" },
    { "LedgerResponseAttach::Links", "effects operations self transactions" },
    { "FeeStatsResponse", "last_ledger last_ledger_base_fee ledger_capacity_usage fee_charged max_fee" },
    { "AccountDataResponse", "value" },
    { "FeeDistribution", "min max mode p10 p20 p30 p40 p50 p60 p70 p80 p90 p95 p99" },
    { "OfferResponse", "id paging_token seller selling buying amount price price_r last_modified_ledger _links" },
    { "OfferResponseAttach::Links", "self offer_maker" },
    { "TradeResponse", "id paging_token ledger_close_time trade_type base_amount base_asset_type counter_amount "
                       "counter_asset_type base_is_seller price _links" },
    { "TradeResponseAttach::Links", "base counter operation" },
    { "TradeAggregationResponse", "timestamp trade_count base_volume counter_volume avg high high_r low low_r "
                                  "open open_r close close_r" },
    { "OrderBookResponse", "base counter bids asks" },
    { "OrderBookResponseAttach::Row", "amount price price_r" },
    { "PathResponse", "source_amount destination_amount source_asset_type destination_asset_type path" },
    { "ClaimableBalanceResponse", "id asset amount claimants paging_token last_modified_ledger _links" },
    { "ClaimableBalanceResponseAttach::Links", "self" },
    { "LiquidityPoolResponse", "id paging_token fee_bp type total_trustlines total_shares reserves "
                               "last_modified_ledger _links" },
    { "LiquidityPoolResponseAttach::Links", "self operations transactions" },
    { "LiquidityPool", "id fee_bp type total_trustlines total_shares reserves" },
    { "AssetAmount", "amount" },
    { "LiquidityPoolClaimableAssetAmount", "asset amount claimable_balance_id" },
    { "TransactionResponse", "hash ledger created_at successful source_account source_account_sequence "
                             "fee_account fee_charged max_fee operation_count envelope_xdr result_xdr "
                             "signatures paging_token memo_type _links" },
    { "TransactionResponseAttach::Links", "account effects ledger operations precedes self succeeds" },
    { "TransactionResponseAttach::InnerTransaction", "hash signatures max_fee" },
    { "TransactionResponseAttach::FeeBumpTransaction", "hash signatures" },
    { "TransactionResponseAttach::LedgerBounds", "min_ledger" },
    { "OperationResponse", "id paging_token type_i type source_account created_at transaction_hash "
                           "transaction_successful _links" },
    { "OperationResponseAttach::Links", "effects precedes self succeeds transaction" },
    { "EffectResponse", "id paging_token type_i type account created_at _links" },
    { "EffectResponseAttach::Links", "operation precedes succeeds" },
    { "RootResponse", "horizon_version core_version history_latest_ledger history_elder_ledger core_latest_ledger "
                      "network_passphrase current_protocol_version core_supported_protocol_version" },
    { "Link", "href" },
    // What an operation moved, and at what price.
    { "CreateAccountOperationResponse", "starting_balance account funder" },
    { "PaymentOperationResponse", "amount asset_type from to" },
    { "PathPaymentBaseOperationResponse", "amount asset_type source_asset_type from to path source_amount" },
    { "PathPaymentStrictReceiveOperationResponse", "source_max" },
    { "PathPaymentStrictSendOperationResponse", "destination_min" },
    { "ManageSellOfferOperationResponse", "offer_id amount price price_r buying_asset_type selling_asset_type" },
    { "ManageBuyOfferOperationResponse", "offer_id amount price price_r buying_asset_type selling_asset_type" },
    { "CreatePassiveSellOfferOperationResponse", "amount price price_r buying_asset_type selling_asset_type" },
    { "ChangeTrustOperationResponse", "limit asset_type trustor" },
    { "AllowTrustOperationResponse", "asset_type trustee trustor authorize authorize_to_maintain_liabilities" },
    { "AccountMergeOperationResponse", "account into" },
    { "ManageDataOperationResponse", "name value" },
    { "CreateClaimableBalanceOperationResponse", "asset amount claimants" },
    { "ClaimClaimableBalanceOperationResponse", "balance_id claimant" },
    { "BeginSponsoringFutureReservesOperationResponse", "sponsored_id" },
    { "ClawbackOperationResponse", "amount asset_type from" },
    { "ClawbackClaimableBalanceOperationResponse", "balance_id" },
    { "SetTrustLineFlagsOperationResponse", "asset_type trustor" },
    { "LiquidityPoolDepositOperationResponse", "liquidity_pool_id reserves_max min_price min_price_r "
                                               "max_price max_price_r reserves_deposited shares_received" },
    { "LiquidityPoolWithdrawOperationResponse", "liquidity_pool_id reserves_min shares reserves_received" },
    { "BumpSequenceOperationResponse", "bump_to" },
    { "InvokeHostFunctionOperationResponse", "function address salt" },
    { "InvokeHostFunctionOperationResponseAttach::Parameter", "type value" },
    { "InvokeHostFunctionOperationResponseAttach::AssetBalanceChange", "type amount asset_type" },
    { "ExtendFootprintTTLOperationResponse", "extend_to" },
    // What an effect moved, or changed.
    { "AccountCreatedEffectResponse", "starting_balance" },
    { "AccountCreditedEffectResponse", "amount asset_type" },
    { "AccountDebitedEffectResponse", "amount asset_type" },
    { "AccountThresholdsUpdatedEffectResponse", "low_threshold med_threshold high_threshold" },
    { "AccountHomeDomainUpdatedEffectResponse", "home_domain" },
    { "SignerEffectResponse", "weight public_key" },
    { "TrustlineCUDResponse", "limit asset_type" },
    { "TrustlineAuthorizationResponse", "trustor asset_type" },
    { "TrustlineFlagsUpdatedEffectResponse", "trustor asset_type" },
    { "TradeEffectResponse", "seller offer_id sold_amount bought_amount sold_asset_type bought_asset_type" },
    { "SequenceBumpedEffectResponse", "new_seq" },
    { "ClaimableBalanceCreatedEffectResponse", "balance_id asset amount" },
    { "ClaimableBalanceClaimedEffectResponse", "balance_id asset amount" },
    { "ClaimableBalanceClaimantCreatedEffectResponse", "balance_id asset amount predicate" },
    { "ClaimableBalanceClawedBackEffectResponse", "balance_id" },
    { "AccountSponsorshipCreatedEffectResponse", "sponsor" },
    { "AccountSponsorshipUpdatedEffectResponse", "former_sponsor new_sponsor" },
    { "AccountSponsorshipRemovedEffectResponse", "former_sponsor" },
    { "TrustlineSponsorshipCreatedEffectResponse", "sponsor" },
    { "TrustlineSponsorshipUpdatedEffectResponse", "former_sponsor new_sponsor" },
    { "TrustlineSponsorshipRemovedEffectResponse", "former_sponsor" },
    { "DataSponsorshipCreatedEffectResponse", "data_name sponsor" },
    { "DataSponsorshipUpdatedEffectResponse", "data_name former_sponsor new_sponsor" },
    { "DataSponsorshipRemovedEffectResponse", "data_name former_sponsor" },
    { "ClaimableBalanceSponsorshipCreatedEffectResponse", "balance_id sponsor" },
    { "ClaimableBalanceSponsorshipUpdatedEffectResponse", "balance_id former_sponsor new_sponsor" },
    { "ClaimableBalanceSponsorshipRemovedEffectResponse", "balance_id former_sponsor" },
    { "SignerSponsorshipCreatedEffectResponse", "signer sponsor" },
    { "SignerSponsorshipUpdatedEffectResponse", "signer former_sponsor new_sponsor" },
    { "SignerSponsorshipRemovedEffectResponse", "signer former_sponsor" },
    { "LiquidityPoolCreatedEffectResponse", "liquidity_pool" },
    { "LiquidityPoolDepositedEffectResponse", "liquidity_pool reserves_deposited shares_received" },
    { "LiquidityPoolWithdrewEffectResponse", "liquidity_pool reserves_received shares_redeemed" },
    { "LiquidityPoolTradeEffectResponse", "liquidity_pool sold bought" },
    { "LiquidityPoolRemovedEffectResponse", "liquidity_pool_id" },
    { "LiquidityPoolRevokedEffectResponse", "liquidity_pool reserves_revoked shares_revoked" },
    { "ContractCreditedEffectResponse", "amount asset_type contract" },
    { "ContractDebitedEffectResponse", "amount asset_type contract" },
};

QSet<QString> requiredMembers(const QMetaObject* mo)
{
    static QMutex mutex;
    static QHash<const QMetaObject*, QSet<QString> > cache;
    QMutexLocker lock(&mutex);
    auto cached = cache.constFind(mo);
    if (cached != cache.constEnd())
        return cached.value();
    QSet<QString> required;
    for (const QMetaObject* m = mo; m; m = m->superClass()) {
        const QByteArray name = shortName(m);
        for (const RequiredMembers& r : kRequired) {
            if (name == r.className) {
                const QStringList members = QString::fromLatin1(r.members).split(QLatin1Char(' '),
                                                                                 Qt::SkipEmptyParts);
                for (const QString& key : members)
                    required.insert(key);
            }
        }
    }
    cache.insert(mo, required);
    return required;
}

void requireRange(const QJsonObject& json, const char* key, qint64 min, qint64 max, const QString& path)
{
    const QJsonValue value = json.value(QLatin1String(key));
    if (!value.isUndefined() && !value.isNull())
        exactInt64(value, member(path, QLatin1String(key)), min, max);
}

QString requireString(const QJsonObject& json, const QString& key, const QString& path)
{
    const QJsonValue value = json.value(key);
    if (!value.isString() || value.toString().isEmpty())
        refuse(member(path, key), value.isUndefined() ? QStringLiteral("missing")
                                                      : QStringLiteral("expected text, found ") + describe(value));
    return value.toString();
}

bool isAccountId(const QString& id)
{
    try {
        return id.size() == 56 && StrKey::decodeVersionByte(id) == StrKey::VersionByte::ACCOUNT_ID
               && StrKey::encodeStellarAccountId(StrKey::decodeStellarAccountId(id)) == id;
    } catch (const std::exception&) {
        return false;
    }
}

bool present(const QJsonValue& value)
{
    return !value.isUndefined() && !value.isNull();
}

/**
 * Refuses an asset/amount pair with no asset: the one object `key` holds,
 * or each element of the list it holds. `because` finishes the message
 * ("missing, but Horizon ...").
 */
void requireAssets(const QJsonObject& json, const char* key, const QString& path, const char* because)
{
    const QJsonValue value = json.value(QLatin1String(key));
    if (!present(value))
        return;                                     // required, or not, by kRequired
    const QString field = member(path, QLatin1String(key));
    const QJsonArray pairs = value.isArray() ? value.toArray() : QJsonArray{ value };
    for (int i = 0; i < pairs.size(); ++i) {
        const QJsonValue asset = pairs.at(i).toObject().value(QStringLiteral("asset"));
        if (!present(asset))
            refuse(member(value.isArray() ? element(field, i) : field, QStringLiteral("asset")),
                   (asset.isUndefined() ? QStringLiteral("missing, but Horizon ")
                                        : QStringLiteral("null, but Horizon "))
                       + QLatin1String(because));
    }
}

/**
 * The asset named by `prefix`type, `prefix`code and `prefix`issuer. The SDK
 * builds it from the code's length, so a credit_alphanum4 type with a
 * 5-character code, or a native type with a code, would have become another
 * asset than the one Horizon named. With `complete` false the code and the
 * issuer may be left out (some effects name the asset elsewhere), but what
 * is there must agree with the type.
 */
void checkAsset(const QJsonObject& json, const QString& prefix, const QString& path, bool complete)
{
    const QString type = requireString(json, prefix + QStringLiteral("type"), path);
    const QString codeKey = prefix + QStringLiteral("code");
    const QString issuerKey = prefix + QStringLiteral("issuer");
    // An empty text is no code: Horizon leaves the members out instead.
    const bool hasCode = present(json.value(codeKey)) && json.value(codeKey) != QJsonValue(QString());
    const bool hasIssuer = present(json.value(issuerKey)) && json.value(issuerKey) != QJsonValue(QString());
    if (type == QLatin1String("native") || type == QLatin1String("liquidity_pool_shares")) {
        if (hasCode || hasIssuer)
            refuse(member(path, hasCode ? codeKey : issuerKey),
                   QStringLiteral("present for a %1 asset").arg(type));
        return;
    }
    int minLength = 0, maxLength = 0;
    if (type == QLatin1String("credit_alphanum4")) {
        minLength = 1;
        maxLength = 4;
    } else if (type == QLatin1String("credit_alphanum12")) {
        minLength = 5;
        maxLength = 12;
    } else {
        refuse(member(path, prefix + QStringLiteral("type")),
               QStringLiteral("unknown asset type \"%1\"").arg(type));
    }
    if (complete || hasCode) {
        const QString code = requireString(json, codeKey, path);
        bool alphanumeric = true;
        for (const QChar c : code)
            alphanumeric = alphanumeric && c.unicode() < 128 && c.isLetterOrNumber();
        if (code.size() < minLength || code.size() > maxLength || !alphanumeric)
            refuse(member(path, codeKey), QStringLiteral("\"%1\" is not a %2 asset code").arg(code, type));
    }
    if (complete || hasIssuer) {
        const QString issuer = requireString(json, issuerKey, path);
        if (!isAccountId(issuer))
            refuse(member(path, issuerKey), QStringLiteral("\"%1\" is not an account id").arg(issuer));
    }
}

/**
 * {"n": .., "d": ..} with d > 0 and n >= 0: int32 terms, or with `wide`
 * int64 ones (Horizon writes those as text).
 */
void checkRational(const QJsonValue& value, const QString& field, bool wide)
{
    if (!value.isObject())
        refuse(field, QStringLiteral("expected {\"n\": .., \"d\": ..}, found ") + describe(value));
    const QJsonObject rational = value.toObject();
    const qint64 max = wide ? std::numeric_limits<qint64>::max() : std::numeric_limits<qint32>::max();
    for (const char* term : { "n", "d" }) {
        const QJsonValue v = rational.value(QLatin1String(term));
        if (!present(v))
            refuse(member(field, QLatin1String(term)), QStringLiteral("missing"));
        exactInt64(v, member(field, QLatin1String(term)), term[0] == 'd' ? 1 : 0, max);
    }
}

bool isCanonicalBase64(const QJsonValue& value, int minBytes, int maxBytes)
{
    if (!value.isString())
        return false;
    const QByteArray raw = value.toString().toLatin1();
    const QByteArray::FromBase64Result decoded =
        QByteArray::fromBase64Encoding(raw, QByteArray::AbortOnBase64DecodingErrors);
    return decoded.decodingStatus == QByteArray::Base64DecodingStatus::Ok
           && decoded.decoded.size() >= minBytes && decoded.decoded.size() <= maxBytes
           && decoded.decoded.toBase64() == raw;
}

/**
 * The checks a record needs beyond each member on its own: the members its
 * kind requires, value ranges, and members that must agree with each other.
 */
void checkRecord(const QMetaObject* mo, const QJsonObject& json, const QString& path)
{
    for (const QString& key : requiredMembers(mo)) {
        const QJsonValue value = json.value(key);
        if (value.isUndefined())
            refuse(member(path, key), QStringLiteral("missing, but Horizon always sends it"));
        if (value.isNull())
            refuse(member(path, key), QStringLiteral("null, but Horizon always sends a value"));
    }

    // An asset named by type, code and issuer, wherever a record has one.
    // Trustline (de)authorizations name only the code (the issuer is the
    // account). Trustline sponsorships name a credit trustline's asset in
    // "asset" (canonical form) next to its asset_type; a pool-share
    // trustline has no "asset" and comes as asset_type "liquidity_pool" plus
    // liquidity_pool_id (stellar-horizon effects_processor.go, sponsorship
    // effects). "liquidity_pool" is no asset type anywhere else.
    const bool sponsorship = shortName(mo).startsWith("TrustlineSponsorship");
    const bool partialAssets = inherits(mo, "TrustlineAuthorizationResponse") || sponsorship;
    // A record of a type this SDK does not know is kept as its base class,
    // which reads no asset: what that type sends is not checked as one.
    const bool baseRecord = shortName(mo) == "EffectResponse" || shortName(mo) == "OperationResponse";
    const bool poolSponsorship = sponsorship
            && json.value(QStringLiteral("asset_type")) == QJsonValue(QStringLiteral("liquidity_pool"));
    if (poolSponsorship) {
        const QString pool = requireString(json, QStringLiteral("liquidity_pool_id"), path);
        static const QRegularExpression hex64(QStringLiteral("^[0-9a-f]{64}$"));
        if (!hex64.match(pool).hasMatch())
            refuse(member(path, QStringLiteral("liquidity_pool_id")),
                   QStringLiteral("\"%1\" is not a liquidity pool id").arg(pool));
        if (present(json.value(QStringLiteral("asset"))))
            refuse(member(path, QStringLiteral("asset")), QStringLiteral("present for a pool-share trustline"));
    }
    for (auto it = json.constBegin(); it != json.constEnd(); ++it) {
        const QString key = it.key();
        if (poolSponsorship && key == QLatin1String("asset_type"))
            continue;                                   // checked above
        if (!baseRecord && key.endsWith(QLatin1String("asset_type")) && !it.value().isNull())
            checkAsset(json, key.left(key.size() - 4), path, !partialAssets);
    }

    // Thresholds are one byte each on the ledger, and so is a master key's
    // weight: stellar-core has refused more than 255 for them from the start.
    // Signer weights are not: before protocol 10 core took any 32-bit weight
    // (and counts it as 255), such signers are still on the ledger, and
    // Horizon sends them as they are, as an int32 (negative from 2^31 on).
    // They are read as that exact integer, in the property's own range; a
    // 0..255 range here refused those accounts, a page holding one, and a
    // payment to one (its SEP-29 lookup).
    if (inherits(mo, "AccountResponseAttach::Thresholds")
            || inherits(mo, "AccountThresholdsUpdatedEffectResponse")) {
        for (const char* key : { "low_threshold", "med_threshold", "high_threshold" })
            requireRange(json, key, 0, 255, path);
    }
    if (inherits(mo, "AccountResponseAttach::Signer")) {
        if (!present(json.value(QStringLiteral("key"))) && !present(json.value(QStringLiteral("public_key"))))
            refuse(member(path, QStringLiteral("key")), QStringLiteral("missing, but Horizon always sends it"));
    }
    if (inherits(mo, "SetOptionsOperationResponse")) {
        for (const char* key : { "low_threshold", "med_threshold", "high_threshold", "master_key_weight" })
            requireRange(json, key, 0, 255, path);
    }

    // A ledger's base fee and reserve, which fees and minimum balances are
    // computed from: *_in_stroops since Horizon 0.15, base_fee and
    // base_reserve before it. Its transaction count likewise:
    // successful_transaction_count since Horizon 0.18, transaction_count
    // before it.
    if (inherits(mo, "LedgerResponse")) {
        for (const char* pair : { "base_fee", "base_reserve" }) {
            const QString key = QLatin1String(pair);
            if (!present(json.value(key + QStringLiteral("_in_stroops"))) && !present(json.value(key)))
                refuse(member(path, key + QStringLiteral("_in_stroops")),
                       QStringLiteral("missing, but Horizon always sends it"));
        }
        if (!present(json.value(QStringLiteral("successful_transaction_count")))
                && !present(json.value(QStringLiteral("transaction_count"))))
            refuse(member(path, QStringLiteral("successful_transaction_count")),
                   QStringLiteral("missing, but Horizon always sends it"));
    }

    // A pool's trustline count, kept as text: text that is not a whole
    // number was kept, for the caller to read as 0.
    if (inherits(mo, "LiquidityPool")) {
        const QJsonValue count = json.value(QStringLiteral("total_trustlines"));
        if (present(count))
            exactUInt64(count, member(path, QStringLiteral("total_trustlines")), std::numeric_limits<quint64>::max());
    }

    // The asset of each asset/amount pair, where Horizon always knows it: a
    // pool's reserves, what a pool effect moved (effects are of transactions
    // that succeeded), and a pool deposit's or withdrawal's reserves when its
    // transaction succeeded. For one that failed Horizon leaves the assets
    // out (go-stellar-sdk base.AssetAmount: asset omitempty, "may be empty
    // when unknown"; the ingester names the pool's assets only for a
    // successful transaction) and gives 0 for what it cannot know. AssetAmount
    // required its asset everywhere, which refused every failed pool
    // operation, a page holding one (include_failed, a failed transaction's
    // operations) and that event in a stream.
    if (inherits(mo, "LiquidityPoolResponse") || inherits(mo, "LiquidityPool"))
        requireAssets(json, "reserves", path, "always sends it");
    if (inherits(mo, "LiquidityPoolDepositedEffectResponse"))
        requireAssets(json, "reserves_deposited", path, "always sends it");
    if (inherits(mo, "LiquidityPoolWithdrewEffectResponse"))
        requireAssets(json, "reserves_received", path, "always sends it");
    if (inherits(mo, "LiquidityPoolTradeEffectResponse")) {
        requireAssets(json, "sold", path, "always sends it");
        requireAssets(json, "bought", path, "always sends it");
    }
    if (json.value(QStringLiteral("transaction_successful")) == QJsonValue(true)) {
        const char* const succeeded = "sends it when the transaction succeeded";
        if (inherits(mo, "LiquidityPoolDepositOperationResponse")) {
            requireAssets(json, "reserves_max", path, succeeded);
            requireAssets(json, "reserves_deposited", path, succeeded);
        }
        if (inherits(mo, "LiquidityPoolWithdrawOperationResponse")) {
            requireAssets(json, "reserves_min", path, succeeded);
            requireAssets(json, "reserves_received", path, succeeded);
        }
    }

    // A submission Horizon took is the transaction as applied: without its
    // ledger isSuccess() read false, without its result the offer ids read 0,
    // and without its hash and envelope they read empty. A problem document
    // (a refused submission) carries none of them.
    if (inherits(mo, "SubmitTransactionResponse") && !json.contains(QStringLiteral("extras"))
            && !json.contains(QStringLiteral("status"))) {
        for (const char* key : { "hash", "ledger", "envelope_xdr", "result_xdr" }) {
            if (!present(json.value(QLatin1String(key))))
                refuse(member(path, QLatin1String(key)), QStringLiteral("missing, but Horizon always sends it"));
        }
    }

    // Times and sequence numbers the classes keep as text, being uint64 or
    // int64: text that is not one was kept, for the caller to read as 0.
    if (inherits(mo, "TransactionResponseAttach::TimeBounds")) {
        for (const char* key : { "min_time", "max_time" }) {
            const QJsonValue value = json.value(QLatin1String(key));
            if (present(value))
                exactUInt64(value, member(path, QLatin1String(key)), std::numeric_limits<quint64>::max());
        }
    }
    if (inherits(mo, "TransactionResponseAttach::Preconditions")) {
        for (const char* key : { "min_account_sequence", "min_account_sequence_age" }) {
            const QJsonValue value = json.value(QLatin1String(key));
            if (present(value))
                exactUInt64(value, member(path, QLatin1String(key)), std::numeric_limits<quint64>::max());
        }
    }
    if (inherits(mo, "AccountResponse")) {
        const QJsonValue time = json.value(QStringLiteral("sequence_time"));
        if (present(time))
            exactUInt64(time, member(path, QStringLiteral("sequence_time")), std::numeric_limits<quint64>::max());
    }

    // What a balance holds depends on its kind.
    if (inherits(mo, "AccountResponseAttach::Balance")) {
        const QString type = json.value(QStringLiteral("asset_type")).toString();
        // Horizon leaves out only what a kind does not have: a native balance
        // has no limit, and pool shares no liabilities.
        QStringList needs;
        if (type == QLatin1String("liquidity_pool_shares"))
            needs << QStringLiteral("liquidity_pool_id") << QStringLiteral("limit");
        else if (type == QLatin1String("native"))
            needs << QStringLiteral("buying_liabilities") << QStringLiteral("selling_liabilities");
        else
            needs << QStringLiteral("limit") << QStringLiteral("buying_liabilities")
                  << QStringLiteral("selling_liabilities");
        for (const QString& key : needs) {
            if (!present(json.value(key)))
                refuse(member(path, key), QStringLiteral("missing from a %1 balance").arg(type));
        }
    }

    // Prices as rationals: int32 terms where Horizon copies them from an
    // offer or an operation, int64 (as text) where it computed them.
    for (const char* key : { "price_r", "min_price_r", "max_price_r" }) {
        const QJsonValue value = json.value(QLatin1String(key));
        if (present(value))
            checkRational(value, member(path, QLatin1String(key)), false);
    }
    // Missing, their getters throw (they read the terms exactly).
    if (inherits(mo, "TradeResponse") && present(json.value(QStringLiteral("price"))))
        checkRational(json.value(QStringLiteral("price")), member(path, QStringLiteral("price")), true);
    if (inherits(mo, "TradeAggregationResponse")) {
        for (const char* key : { "high_r", "low_r", "open_r", "close_r" }) {
            if (present(json.value(QLatin1String(key))))
                checkRational(json.value(QLatin1String(key)), member(path, QLatin1String(key)), true);
        }
    }

    // Assets written as objects: an offer's selling and buying, an order
    // book's base and counter, the hops of a path.
    if (inherits(mo, "OfferResponse") || inherits(mo, "OrderBookResponse")) {
        for (const char* key : { "selling", "buying", "base", "counter" }) {
            const QJsonValue value = json.value(QLatin1String(key));
            if (value.isObject())
                checkAsset(value.toObject(), QStringLiteral("asset_"), member(path, QLatin1String(key)), true);
        }
    }
    if (inherits(mo, "PathResponse") || inherits(mo, "PathPaymentBaseOperationResponse")) {
        const QJsonArray hops = json.value(QStringLiteral("path")).toArray();
        for (int i = 0; i < hops.size(); ++i) {
            const QString field = element(member(path, QStringLiteral("path")), i);
            if (!hops.at(i).isObject())
                refuse(field, QStringLiteral("expected an asset object, found ") + describe(hops.at(i)));
            checkAsset(hops.at(i).toObject(), QStringLiteral("asset_"), field, true);
        }
    }

    // Booleans the classes keep in a QVariant.
    if (inherits(mo, "TransactionResponse") || inherits(mo, "OperationResponse")) {
        for (const char* key : { "successful", "transaction_successful" }) {
            const QJsonValue value = json.value(QLatin1String(key));
            if (present(value) && !value.isBool())
                refuse(member(path, QLatin1String(key)),
                       QStringLiteral("expected true or false, found ") + describe(value));
        }
    }

    // The memo, which getMemo() builds later: refused here, not there. An id
    // is decimal digits, as Memo::parse() (js's Memo.id) reads it: "-0", a
    // whole number not below 0, passed this check and getMemo() then threw.
    if (inherits(mo, "TransactionResponse")) {
        const QString type = json.value(QStringLiteral("memo_type")).toString();
        const QJsonValue memo = json.value(QStringLiteral("memo"));
        const QString field = member(path, QStringLiteral("memo"));
        if (type == QLatin1String("id")) {
            if (!memo.isString())
                refuse(field, QStringLiteral("expected an id memo as text, found ") + describe(memo));
            if (memo.toString().startsWith(QLatin1Char('-')))
                refuse(field, QStringLiteral("expected an id memo in decimal digits, found ") + describe(memo));
            exactUInt64(memo, field, std::numeric_limits<quint64>::max());
        } else if (type == QLatin1String("hash") || type == QLatin1String("return")) {
            if (!isCanonicalBase64(memo, 32, 32))
                refuse(field, QStringLiteral("expected the base64 of 32 bytes, found ") + describe(memo));
        } else if (type == QLatin1String("text")) {
            if (present(memo) && !memo.isString())
                refuse(field, QStringLiteral("expected text, found ") + describe(memo));
        } else if (type != QLatin1String("none")) {
            refuse(member(path, QStringLiteral("memo_type")),
                   QStringLiteral("unknown memo type \"%1\"").arg(type));
        }
        const QJsonValue bytes = json.value(QStringLiteral("memo_bytes"));
        if (present(bytes) && !isCanonicalBase64(bytes, 0, 28))
            refuse(member(path, QStringLiteral("memo_bytes")),
                   QStringLiteral("expected the base64 of a text memo, found ") + describe(bytes));
    }
}

/** A JSON array of whole numbers in int32. */
QVariant exactInt32List(const QJsonValue& value, const QString& field)
{
    if (!value.isArray())
        refuse(field, QStringLiteral("expected an array, found ") + describe(value));
    QList<qint32> list;
    const QJsonArray array = value.toArray();
    for (int i = 0; i < array.size(); ++i)
        list.append(static_cast<qint32>(exactInt64(array.at(i), element(field, i),
                                                   std::numeric_limits<qint32>::min(),
                                                   std::numeric_limits<qint32>::max())));
    return QVariant::fromValue(list);
}

/**
 * An X-Ratelimit-* header: decimal digits that fit an int, or -1 for a header
 * that was not sent or is anything else. toInt() read both as 0, which for
 * X-Ratelimit-Remaining says that no request is left.
 */
int rateLimitHeader(const QByteArray& header)
{
    const QByteArray value = header.trimmed();
    if (value.isEmpty())
        return -1;
    qint64 number = 0;
    for (const char c : value) {
        if (c < '0' || c > '9')
            return -1;
        number = number * 10 + (c - '0');
        if (number > std::numeric_limits<int>::max())
            return -1;
    }
    return static_cast<int>(number);
}

/** Horizon's claimants: [{"destination": G.., "predicate": {..}}], read by
 *  ClaimantConverter::fromJson, which QVariantList conversions share. */
QVariant exactClaimants(const QJsonValue& value, const QString& field)
{
    return QVariant::fromValue(ClaimantConverter::fromJson(value, field));
}

void registerReaders()
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // Qt 5 finds a property's type by the name moc wrote, "QList<qint32>".
    qRegisterMetaType<QList<qint32> >("QList<qint32>");
#endif
    ResponseConverters::registerExactReader(qMetaTypeId<QList<qint32> >(), &exactInt32List);
    ResponseConverters::registerExactReader(qMetaTypeId<QList<Claimant> >(), &exactClaimants);
}

} // namespace

Q_COREAPP_STARTUP_FUNCTION(registerReaders)

/**
 * The reader behind Response::fillObject and ResponseConverters::fillGadget.
 * A friend of Response, to ask a record for the members it reads itself.
 */
class ResponseReader
{
public:
    static QVariant read(const QMetaProperty& property, const QString& key, const QJsonValue& value,
                         const QString& field);
    static void fill(const QMetaObject* mo, void* object, Response* qobject, const QJsonObject& json,
                     const QString& path);
};

QVariant ResponseReader::read(const QMetaProperty& property, const QString& key, const QJsonValue& value,
                              const QString& field)
{
    const int type = property.userType();
    if (ResponseConverters::ExactReader reader = exactReaders().value(type, nullptr))
        return reader(value, field);
    switch (type) {
    case QMetaType::QString:
        if (!value.isString())
            refuse(field, QStringLiteral("expected text, found ") + describe(value));
        if (isAmountKey(key))
            checkAmount(value, field);
        return value.toString();
    case QMetaType::QByteArray:
        if (!value.isString())
            refuse(field, QStringLiteral("expected text, found ") + describe(value));
        return value.toString().toUtf8();
    case QMetaType::Bool:
        if (!value.isBool())
            refuse(field, QStringLiteral("expected true or false, found ") + describe(value));
        return value.toBool();
    case QMetaType::Int:
        return static_cast<int>(exactInt64(value, field, std::numeric_limits<int>::min(),
                                           std::numeric_limits<int>::max()));
    case QMetaType::UInt:
        return static_cast<uint>(exactUInt64(value, field, std::numeric_limits<uint>::max()));
    case QMetaType::Short:
        return QVariant::fromValue(static_cast<short>(
            exactInt64(value, field, std::numeric_limits<short>::min(), std::numeric_limits<short>::max())));
    case QMetaType::UShort:
        return QVariant::fromValue(static_cast<ushort>(
            exactUInt64(value, field, std::numeric_limits<ushort>::max())));
    case QMetaType::Long:
        return QVariant::fromValue(static_cast<long>(
            exactInt64(value, field, std::numeric_limits<long>::min(), std::numeric_limits<long>::max())));
    case QMetaType::ULong:
        return QVariant::fromValue(static_cast<ulong>(
            exactUInt64(value, field, std::numeric_limits<ulong>::max())));
    case QMetaType::LongLong:
        return static_cast<qlonglong>(exactInt64(value, field, std::numeric_limits<qint64>::min(),
                                                 std::numeric_limits<qint64>::max()));
    case QMetaType::ULongLong:
        return static_cast<qulonglong>(exactUInt64(value, field, std::numeric_limits<quint64>::max()));
    case QMetaType::Float: {
        const double d = exactDouble(value, field);
        if (std::fabs(d) > static_cast<double>(std::numeric_limits<float>::max()))
            refuse(field, describe(value) + QStringLiteral(" is out of range"));
        return QVariant::fromValue(static_cast<float>(d));
    }
    case QMetaType::Double:
        return exactDouble(value, field);
    case QMetaType::QVariantMap:
        if (!value.isObject())
            refuse(field, QStringLiteral("expected an object, found ") + describe(value));
        return value.toObject().toVariantMap();
    case QMetaType::QVariantList:
        if (!value.isArray())
            refuse(field, QStringLiteral("expected an array, found ") + describe(value));
        return value.toArray().toVariantList();
    case QMetaType::QStringList: {
        if (!value.isArray())
            refuse(field, QStringLiteral("expected an array, found ") + describe(value));
        QStringList list;
        const QJsonArray array = value.toArray();
        for (int i = 0; i < array.size(); ++i) {
            if (!array.at(i).isString())
                refuse(element(field, i), QStringLiteral("expected text, found ") + describe(array.at(i)));
            list.append(array.at(i).toString());
        }
        return list;
    }
    case QMetaType::QVariant:
        // The classes that keep a QVariant filter it; checkRecord checks it.
        return value.toVariant();
    default:
        break;
    }
    if (const QMetaObject* gadget = gadgetMetaObject(type)) {
        if (!value.isObject())
            refuse(field, QStringLiteral("expected an object, found ") + describe(value));
        QVariant object = defaultValue(type);
        fill(gadget, object.data(), nullptr, value.toObject(), field);
        return object;
    }
    refuse(field, QStringLiteral("a property of type %1 cannot be read from JSON")
                      .arg(QString::fromLatin1(property.typeName())));
}

void ResponseReader::fill(const QMetaObject* mo, void* object, Response* qobject, const QJsonObject& json,
                          const QString& path)
{
    const QSet<QString> required = requiredMembers(mo);
    if (qobject)
        qobject->m_json = json;
    for (auto it = json.constBegin(); it != json.constEnd(); ++it) {
        const QString key = it.key();
        const QJsonValue value = it.value();
        const QString field = member(path, key);
        if (qobject && qobject->readMember(key, value, field))
            continue;
        const QByteArray name = key.toUtf8();
        const int index = mo->indexOfProperty(name.constData());
        if (index < 0)
            continue;
        const QMetaProperty property = mo->property(index);
        if (!property.isWritable())
            continue;
        QVariant v;
        if (value.isNull()) {
            if (required.contains(key))
                refuse(field, QStringLiteral("null, but Horizon always sends a value"));
            // An invalid QVariant writes the property's default: 0, empty.
        } else {
            v = read(property, key, value, field);
        }
        const bool written = qobject ? property.write(qobject, v) : property.writeOnGadget(object, v);
        if (!written)
            refuse(field, QStringLiteral("could not be stored in its %1 property")
                              .arg(QString::fromLatin1(property.typeName())));
    }
    checkRecord(mo, json, path);
}

void ResponseConverters::registerExactReader(int metaTypeId, ExactReader reader)
{
    exactReaders().insert(metaTypeId, reader);
}

qint64 ResponseConverters::exactInteger(const QJsonValue& value, const QString& field, qint64 min, qint64 max)
{
    return exactInt64(value, field, min, max);
}

void ResponseConverters::fillGadget(const QMetaObject* mo, void* gadget, const QJsonValue& value, const QString& field)
{
    if (!mo)
        refuse(field, QStringLiteral("expected an array, found ") + describe(value));
    if (!value.isObject())
        refuse(field, QStringLiteral("expected an object, found ") + describe(value));
    ResponseReader::fill(mo, gadget, nullptr, value.toObject(), field);
}

Price ResponseConverters::exactPrice(const QVariantMap& rational, const char* field)
{
    const QJsonValue json = QJsonValue::fromVariant(rational);
    const QString name = QString::fromLatin1(field);
    checkRational(json, name, false);
    const QJsonObject terms = json.toObject();
    return Price(static_cast<qint32>(exactInt64(terms.value(QStringLiteral("n")), name, 0,
                                                std::numeric_limits<qint32>::max())),
                 static_cast<qint32>(exactInt64(terms.value(QStringLiteral("d")), name, 1,
                                                std::numeric_limits<qint32>::max())));
}

Predicate* ResponseConverters::exactPredicate(const QJsonValue& value, const QString& field)
{
    // One reader for Horizon's predicates, shared with Predicate::create(QVariant):
    // it also checks an abs_before past the year 9999 against abs_before_epoch,
    // reads no time without its zone, and refuses what Core refuses (negative
    // times, nesting deeper than 4).
    return Predicate::fromJson(value, field);
}

Response::Response(QNetworkReply *reply)
    :m_lastErrorCode(QNetworkReply::NetworkError::NoError)
    ,m_status(0)
    ,m_timeoutTimerID(0)
    ,m_reconnectTimerID(0)
    ,m_retryTime(RECONNECT_DELAY)
    ,m_retryAfter(0)
    ,m_timedOut(false)
    ,m_replyState(ReplyNone)
    ,m_reply(nullptr)
    ,m_rateLimitLimit(-1)
    ,m_rateLimitRemaining(-1)
    ,m_rateLimitReset(-1)
{
    loadFromReply(reply);
}
void Response::clearReply(QObject* obj)
{
    if(this->m_reply==obj ){
        this->m_reply=nullptr;
        // Its timeout goes with it: the timer outlived the reply and, firing
        // later, dereferenced the null pointer and took the process down.
        if (m_timeoutTimerID) {
            this->killTimer(m_timeoutTimerID);
            m_timeoutTimerID = 0;
        }
        // It went before it finished: its network manager, its Server's, was
        // deleted. Said, so that a caller waiting on a response of its own
        // (a submission) is not left waiting forever, as it was.
        if (m_replyState == ReplyWaiting || m_replyState == ReplyStreaming) {
            m_replyState = ReplyNone;
            emitError(new qstellar::exception::ConnectionErrorException(
                QStringLiteral("the request was abandoned: its Server was deleted "
                               "(it may still have been received)")));
        }
    }
}

QNetworkReply::NetworkError Response::lastErrorCode() const
{
    return m_lastErrorCode;
}

int Response::getStatus() const
{
    return m_status;
}

int Response::replyTimeout()
{
    return s_replyTimeout.load();
}

void Response::setReplyTimeout(int milliseconds)
{
    s_replyTimeout.store(milliseconds > 0 ? milliseconds : STELLAR_QT_REPLY_TIMEOUT);
}

void Response::restartTimeoutTimer()
{
    if(m_timeoutTimerID)
    {
        this->killTimer(m_timeoutTimerID);
    }
    m_timeoutTimerID= this->startTimer(replyTimeout());
}
void Response::loadFromReply(QNetworkReply * reply)
{
    if(reply!=m_reply){
        if(m_reply){
            // Nothing more from the reply that is replaced.
            disconnect(m_reply, nullptr, this, nullptr);
            m_reply->deleteLater();
            m_reply=nullptr;
            m_replyState = ReplyNone;
            this->reset();//we set properties to zero
        }

        if(reply){
            m_reply = reply;
            m_timedOut = false;
            m_replyState = ReplyWaiting;
            connect(m_reply,&QObject::destroyed,this,&Response::clearReply);
            if(this->isStreamingResponse()){
                connect(reply, &QNetworkReply::readyRead, this,&Response::processPartialResponse);
                connect(reply, &QNetworkReply::finished, this,&Response::streamFinished);
                restartTimeoutTimer();
            }
            else{
                connect(reply, &QNetworkReply::finished, this,&Response::processResponse);
                restartTimeoutTimer();
            }
        }
    }
}

void Response::reset()
{
    m_lastErrorCode = QNetworkReply::NetworkError::NoError;
    m_lastException.reset();
    m_json = QJsonObject();
    const QMetaObject * mo = this->metaObject();
    for(int i=0;i<mo->propertyCount();i++)
    {
        if(mo->property(i).isWritable()){
            mo->property(i).write(this,QVariant());
        }
    }
}

qstellar::exception::SdkException* Response::lastException() const
{
    return m_lastException.get();
}

void Response::emitError(qstellar::exception::SdkException* ex)
{
    // A request's outcome is delivered with its first error(): a reply
    // deleted after that (a federation answer stopped for its size) is not
    // reported again as abandoned. A stream's connection keeps its own state
    // (openStream).
    if (m_replyState == ReplyWaiting && !isStreamingResponse())
        m_replyState = ReplyDone;
    m_lastException.reset(ex);
    emit error();
}

bool Response::isStreamingResponse() const
{
    return m_reply && (this->m_reply->request().rawHeader("Accept")=="text/event-stream");
}

Response::~Response(){
    if(m_reply){
        // Later, not here: this may run inside one of the reply's own
        // signals (a receiver of ready() or error() deleting its stream), and
        // Qt goes on using the reply once the slot returns. It was deleted
        // on the spot, which freed it under Qt's feet. Nothing of it reaches
        // this object any more; a reply still running is aborted when it is
        // deleted.
        disconnect(m_reply, nullptr, this, nullptr);
        m_reply->deleteLater();
    }
}

void Response::fillObject(const QMetaObject* mo, void* obj,const QJsonObject& jsonObj, bool qobject)
{
    fillObject(mo, obj, jsonObj, qobject, QString());
}

void Response::fillObject(const QMetaObject* mo, void* obj, const QJsonObject& jsonObj, bool qobject,
                          const QString& path)
{
    ResponseReader::fill(mo, obj, qobject ? static_cast<Response*>(static_cast<QObject*>(obj)) : nullptr,
                         jsonObj, path);
}

bool Response::readMember(const QString& key, const QJsonValue& value, const QString& field)
{
    Q_UNUSED(key)
    Q_UNUSED(value)
    Q_UNUSED(field)
    return false;
}

void Response::readFailureBody(int, const QJsonObject&)
{
}

void Response::loadFromJson(QByteArray data)
{
    QJsonParseError check;
    QJsonDocument doc = QJsonDocument::fromJson(data,&check);

    if(check.error){
        throw std::runtime_error(check.errorString().toStdString());
    }
    // An array or a bare value read as an empty object: every property 0.
    if (!doc.isObject())
        throw std::runtime_error("expected a JSON object");

    QJsonObject object = doc.object();
    fillObject(this->metaObject(), this, object,true);


}

void Response::reconnectStream()
{
    if(!m_reconnectTimerID)
    {
        // A 429's Retry-After counts for this reconnect only; the next ones
        // wait the stream's own delay again.
        m_reconnectTimerID= this->startTimer(qMax(m_retryTime, m_retryAfter));
        m_retryAfter = 0;
    }
}
void Response::streamFinished()
{
    QNetworkReply* reply = static_cast<QNetworkReply*>(sender());
    if (reply == m_reply && m_replyState == ReplyWaiting) {
        // It ended before any of it was read: refused, a host that does not
        // resolve, TLS, a timeout, an HTTP error (read here, whole). It was
        // retried every second and nothing was said.
        QPointer<Response> alive(this);
        openStream(reply);
        if (!alive)
            return;
    }
    if (reply == m_reply)
        m_replyState = ReplyDone;
    reconnectStream();
}

bool Response::openStream(QNetworkReply* reply)
{
    using namespace qstellar::exception;

    m_replyState = ReplyDone;
    QPointer<Response> alive(this);
    if (!preprocessResponse(reply) || !alive)
        return false;
    // As js's EventSource: a 2xx that is not an event stream (a captive
    // portal's page, a proxy's answer) is a connection that failed. It was
    // read as a stream without events.
    const QByteArray type = reply->header(QNetworkRequest::ContentTypeHeader).toByteArray().trimmed().toLower();
    if (!type.startsWith("text/event-stream")) {
        emitError(new UnknownResponseException(m_status,
            QStringLiteral("not an event stream (Content-Type \"%1\")").arg(QString::fromLatin1(type))));
        // And ended, as js's EventSource gives it up: finishing reconnects
        // (streamFinished). It was left open, its bytes thrown away, so a
        // body with no end (a proxy relabelling Horizon's stream) kept the
        // stream connected forever, reading nothing and never reconnecting.
        if (alive && reply->isRunning())
            reply->abort();
        return false;
    }
    m_replyState = ReplyStreaming;
    return true;
}

void Response::reconnectStreamDelayed()
{
    // FIX §4.3: m_reply may be null after clearReply() (triggered by the
    // `destroyed` signal of a previous reply). Crash without this guard.
    if (!m_reply) {
        // Its Server (the network manager) was deleted while the stream
        // waited to reconnect. This was an error() with no exception.
        emitError(new qstellar::exception::ConnectionErrorException(
            QStringLiteral("the stream was abandoned: its Server was deleted")));
        return;
    }
    QNetworkAccessManager* manager = m_reply->manager();
    QNetworkRequest request(m_reply->request());
    // What the dropped connection left of an unfinished event is discarded,
    // as the SSE spec (and js's EventSource) does. It used to be joined with
    // the new connection's first bytes, read as one event carrying the lost
    // event's id: the id was taken as seen, and a second drop resumed after
    // it, so that record was never delivered.
    m_pendingData.clear();
    if (!m_lastID.isEmpty())
        request.setRawHeader("Last-Event-ID", m_lastID);
    request.setRawHeader("X-Client-Name", STELLAR_QT_SDK_CLIENT_NAME);
    request.setRawHeader("X-Client-Version", STELLAR_QT_SDK_CLIENT_VERSION);

    QNetworkReply * reply = manager->get(request);
    loadFromReply(reply);
}

int Response::getRateLimitLimit() const{
    return m_rateLimitLimit;
}

int Response::getRateLimitRemaining() const{
    return m_rateLimitRemaining;
}

int Response::getRateLimitReset() const{
    return m_rateLimitReset;
}

void Response::timerEvent(QTimerEvent *event)
{
    if(event->timerId()==m_reconnectTimerID)
    {
        // Stopped first: reconnecting may emit error(), and a receiver may
        // delete this object. The timer was killed after it, on the deleted
        // object.
        this->killTimer(m_reconnectTimerID);
        m_reconnectTimerID=0;
        this->reconnectStreamDelayed();
    }
    else if(event->timerId()==m_timeoutTimerID)
    {
        this->killTimer(m_timeoutTimerID);
        m_timeoutTimerID= 0;
        if(this->m_reply && this->m_reply->isRunning())
        {
            // Closing finishes the reply with OperationCanceledError, which
            // on its own reads as a connection that failed.
            m_timedOut = true;
            this->m_reply->close();
        }
    }

}
bool Response::preprocessResponse(QNetworkReply *response)
{
    using namespace qstellar::exception;

    QNetworkReply::NetworkError errorCode = response->error();
    m_lastErrorCode = errorCode;
    m_rateLimitLimit     = rateLimitHeader(response->rawHeader("X-Ratelimit-Limit"));
    m_rateLimitRemaining = rateLimitHeader(response->rawHeader("X-Ratelimit-Remaining"));
    m_rateLimitReset     = rateLimitHeader(response->rawHeader("X-Ratelimit-Reset"));
    emit rateLimitChanged();

    // FIX §4.1: old code compared QNetworkReply::NetworkError against HTTP
    // status codes (429, >=300). That enum is unrelated (e.g.
    // ContentNotFoundError=203) — broken. Use HttpStatusCodeAttribute.
    const QVariant statusAttr = response->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    const int httpStatus = statusAttr.isValid() ? statusAttr.toInt() : -1;

    // Surface the real HTTP status so getStatus() / getStatusCode() /
    // SubmitTransactionResponse::isTimeout() are meaningful. m_status was
    // otherwise never assigned from the reply (defaulted to 0), so e.g. a
    // Horizon 504 Gateway Timeout was invisible to callers — they could not
    // tell a timeout (resend the SAME envelope) from a hard failure (rebuild).
    // 0 means "no HTTP status" (connection-level failure, e.g. client timeout).
    m_status = (httpStatus > 0) ? httpStatus : 0;
    emit statusChanged();

    // 2xx → success; caller reads the body.
    if (httpStatus >= 200 && httpStatus < 300) {
        return true;
    }

    // Connection-level failure: no HTTP status but QNetworkReply errored.
    if (httpStatus == -1) {
        // The SDK's own timer gave up on the reply (or the network stack
        // timed out). The request may have reached Horizon: a submission may
        // still be applied. This was a plain ConnectionErrorException, which
        // reads as a request that never went out.
        if ((m_timedOut && errorCode == QNetworkReply::OperationCanceledError)
                || errorCode == QNetworkReply::TimeoutError) {
            emitError(new ClientTimeoutException(replyTimeout(), response->errorString()));
            return false;
        }
        if (errorCode != QNetworkReply::NoError) {
            emitError(new ConnectionErrorException(response->errorString()));
            return false;
        }
        // No status and no error — improbable; treat as unknown.
        emitError(new UnknownResponseException(-1, response->errorString()));
        return false;
    }

    // Snapshot body if available.
    QByteArray body;
    if (response->isOpen()) {
        body = response->readAll();
    }
    const QString bodyStr = QString::fromUtf8(body);

    // The body as a problem document whatever its Content-Type, as js
    // (axios parses the body) and py (BaseHorizonError calls
    // response.json()) read it: only application/problem+json was, and
    // behind a proxy that rewrote it the result codes were out of reach of
    // getProblem(). tryParse returns nullptr for what is not a JSON object.
    Problem* problem = nullptr;
    if (!body.isEmpty()) {
        problem = Problem::tryParse(body);
    }

    // What the error body says about this response (a failed submission's
    // result) is kept by the response too, before error() is emitted.
    if (httpStatus >= 400 && httpStatus != 429 && !body.isEmpty()) {
        const QJsonDocument failure = QJsonDocument::fromJson(body);
        if (failure.isObject()) {
            try {
                readFailureBody(httpStatus, failure.object());
            } catch (const std::exception&) {
                // The exception below still carries the body.
            }
        }
    }

    // Map by HTTP status. Exceptions take ownership of `problem`.
    if (httpStatus == 429) {
        delete problem; problem = nullptr;
        const QByteArray retryHdr = response->rawHeader("Retry-After");
        int retryAfter = -1;
        if (!retryHdr.isEmpty()) {
            bool ok = false;
            int v = retryHdr.toInt(&ok);
            if (ok) retryAfter = v;
        }
        // Retry-After is in seconds, the reconnect delay in milliseconds: it
        // was compared as is, so "Retry-After: 30" waited 1 s.
        if (retryAfter > 0)
            m_retryAfter = retryAfter > std::numeric_limits<int>::max() / 1000
                    ? std::numeric_limits<int>::max() : retryAfter * 1000;
        if (isStreamingResponse())
            reconnectStream();
        emitError(new TooManyRequestsException(retryAfter));
        return false;
    }
    if (httpStatus == 504) {
        emitError(new RequestTimeoutException(httpStatus, bodyStr, problem));
        return false;
    }
    if (httpStatus >= 500) {
        emitError(new BadResponseException(httpStatus, bodyStr, problem));
        return false;
    }
    if (httpStatus >= 400 && httpStatus < 500) {
        // Generic 4xx → BadRequestException. AccountNotFoundException etc.
        // are built at specific call sites in the SDK, not here.
        emitError(new BadRequestException(httpStatus, bodyStr, problem));
        return false;
    }

    // Unexpected 1xx / 3xx.
    delete problem; problem = nullptr;
    emitError(new UnknownResponseException(httpStatus, bodyStr));
    return false;
}

void Response::processPartialResponse()
{
    QNetworkReply* response = static_cast<QNetworkReply*>(sender());
    if (m_replyState == ReplyWaiting) {
        // An answer that is not 2xx is read whole when it finishes
        // (streamFinished), once: this read it on each readyRead, and each
        // piece emitted error() with the part of the body read so far.
        const QVariant status = response->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        if (!status.isValid() || status.toInt() < 200 || status.toInt() >= 300)
            return;
        if (!openStream(response))
            return;
    }
    if (m_replyState != ReplyStreaming) {
        response->readAll(); // the rest of an answer already reported
        return;
    }
    // Only an event stream's bytes keep its connection alive. Those of an
    // answer that is not 2xx restarted the timeout too: a server that kept
    // sending the body of a 500 (a slow drip, or one with no end) held the
    // reply open forever, with no error() and Qt buffering the body. Such
    // an answer has replyTimeout() from the request to finish now, as a
    // request has; when it does not, its status is reported without the
    // body, and the stream reconnects.
    this->restartTimeoutTimer();

    m_pendingData.append(response->readAll());
    // A line ends at CRLF, LF or a lone CR, as the SSE spec says and js's
    // eventsource reads; a lone CR was not known, and an event written with
    // them never ended. All three become LF here. A CR that ends what has
    // arrived waits for the next byte: it may be the start of a CRLF.
    const bool heldCR = m_pendingData.endsWith('\r');
    if (heldCR)
        m_pendingData.chop(1);
    m_pendingData.replace("\r\n", "\n");
    m_pendingData.replace('\r', '\n');
    if (heldCR)
        m_pendingData.append('\r');

    // Each event ends at a blank line. Its data is JSON, which cannot hold a
    // raw line break, so a blank line never falls inside a record.
    QPointer<Response> alive(this);
    for (;;) {
        const int end = m_pendingData.indexOf("\n\n");
        if (end < 0)
            break;
        const QByteArray event = m_pendingData.left(end);
        m_pendingData.remove(0, end + 2);
        processStreamEvent(event);
        // A receiver of ready() or error() may have deleted this object.
        if (!alive)
            return;
    }
}

void Response::processStreamEvent(const QByteArray& event)
{
    using namespace qstellar::exception;

    QByteArray data;
    bool hasData = false;
    QByteArray id;
    bool hasId = false;
    QByteArray type;
    for (QByteArray line : event.split('\n')) {
        if (line.endsWith('\r'))
            line.chop(1);
        if (line.isEmpty() || line.startsWith(':'))
            continue; // a comment: keep-alive
        const int colon = line.indexOf(':');
        const QByteArray field = colon < 0 ? line : line.left(colon);
        QByteArray value = colon < 0 ? QByteArray() : line.mid(colon + 1);
        if (value.startsWith(' '))
            value.remove(0, 1);
        if (field == "data") {
            if (hasData)
                data.append('\n');
            data.append(value);
            hasData = true;
        } else if (field == "id") {
            id = value;
            hasId = true;
        } else if (field == "event") {
            type = value;
        } else if (field == "retry") {
            bool ok = false;
            const int retry = value.toInt(&ok);
            if (ok)
                m_retryTime = qMax(retry, RECONNECT_DELAY);
        }
    }
    // The event is whole (it ended with a blank line): its id is the last
    // one seen from here on, whether or not its data then reads.
    if (hasId)
        m_lastID = id;
    // The event's type, as js's EventSource reads it. An "error" event (a
    // stream that failed once it had started) carries its message as text:
    // it was parsed as a record and reported as "JSON parse failed", the
    // message lost; it is reported with it now, and the stream goes on. A
    // record is an event without a type, or "message"; "open" (hello),
    // "close" (byebye) and any other type are not records.
    if (type == "error") {
        emitError(new UnknownResponseException(m_status,
            QStringLiteral("the stream sent an error event: %1").arg(QString::fromUtf8(data))));
        return;
    }
    if (!type.isEmpty() && type != "message")
        return;
    if (!hasData || data == "\"hello\"" || data == "\"byebye\"")
        return;

    try {
        // Each event is a whole record. Without this, a member one event had
        // and the next left out (a home domain cleared, an inner transaction)
        // kept the earlier event's value.
        const int status = m_status;
        this->reset();
        m_status = status;
        this->loadFromJson(data);
    } catch (const std::exception& err) {
        // Reported, and the stream goes on with the next event. An event that
        // did not parse used to be retried with the events after it appended,
        // which never parsed either: the stream went silent, saying nothing.
        emitError(new UnknownResponseException(200,
            QStringLiteral("JSON parse failed: %1").arg(QString::fromUtf8(err.what()))));
        return;
    } catch (...) {
        emitError(new UnknownResponseException(200, QStringLiteral("Unknown exception while parsing JSON")));
        return;
    }
    emit ready();
}

void Response::processResponse()
{
    using namespace qstellar::exception;

    QNetworkReply* response = static_cast<QNetworkReply*>(sender());
    if (response == m_reply)
        m_replyState = ReplyDone;

    // NOTE: preprocessResponse() now reads the body to map errors to Problem,
    // but only for non-2xx. For 2xx the body remains available — read it here.
    QByteArray entity;
#ifdef STELLAR_QT_DEBUG_NETWORK_REQUESTS
    qDebug() << "QUERY : " <<  response->request().url();
    qDebug() << "ERROR CODE : "<< response->error() << response->errorString();
#endif
    if (!preprocessResponse(response)) {
        // Typed exception already dispatched via emitError().
        return;
    }
    // 2xx — read full body
    if (response->isOpen())
        entity = response->readAll();
#ifdef STELLAR_QT_DEBUG_NETWORK_REQUESTS
    qDebug() << "RESPONSE : "<< QString::fromLatin1(entity);
#endif

    if (entity.isNull()) {
        emitError(new UnknownResponseException(200, QStringLiteral("Empty response body")));
        return;
    }
    try {
        this->loadFromJson(entity);
    }
    catch (const std::exception& err) {
        // FIX §4.2: was silent `return;` — caller hung waiting for
        // ready()/error(). Now emit error() with an UnknownResponseException
        // for diagnostics. Broad catch in case parsing throws something
        // other than std::runtime_error.
        emitError(new UnknownResponseException(200,
            QStringLiteral("JSON parse failed: %1").arg(QString::fromUtf8(err.what()))));
        return;
    }
    catch (...) {
        emitError(new UnknownResponseException(200, QStringLiteral("Unknown exception while parsing JSON")));
        return;
    }

    emit ready();
}
QSTELLAR_END_NS
