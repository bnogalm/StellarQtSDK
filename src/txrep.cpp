#include "txrep.h"

#include <QHash>
#include <QStringList>
#include <QTextStream>
#include <stdexcept>

#include "transaction.h"
#include "transactionbuilder.h"
#include "network.h"
#include "keypair.h"
#include "account.h"
#include "memo.h"
#include "managedataoperation.h"
#include "paymentoperation.h"
#include "createaccountoperation.h"
#include "accountmergeoperation.h"
#include "bumpsequenceoperation.h"
#include "setoptionsoperation.h"
#include "changetrustoperation.h"
#include "allowtrustoperation.h"
#include "managesellofferoperation.h"
#include "managebuyofferoperation.h"
#include "createpassivesellofferoperation.h"
#include "pathpaymentstrictreceiveoperation.h"
#include "pathpaymentstrictsendoperation.h"
#include "beginsponsoringfuturereservesoperation.h"
#include "endsponsoringfuturereservesoperation.h"
#include "clawbackoperation.h"
#include "clawbackclaimablebalanceoperation.h"
#include "settrustlineflagsoperation.h"
#include "claimclaimablebalanceoperation.h"
#include "liquiditypooldepositoperation.h"
#include "liquiditypoolwithdrawoperation.h"
#include "createclaimablebalanceoperation.h"
#include "claimant.h"
#include "predicate.h"
#include "revokeaccountsponsorshipoperation.h"
#include "revoketrustlinesponsorshipoperation.h"
#include "revokeoffersponsorshipoperation.h"
#include "revokedatasponsorshipoperation.h"
#include "revokeclaimablebalancesponsorshipoperation.h"
#include "revokeliquiditypoolsponsorshipoperation.h"
#include "revokesignersponsorshipoperation.h"
#include "price.h"
#include "asset.h"
#include "assettypenative.h"
#include "assettypecreditalphanum4.h"
#include "assettypecreditalphanum12.h"
#include "timebounds.h"
#include "ledgerbounds.h"
#include "signerkey.h"
#include "transactionpreconditions.h"

QSTELLAR_BEGIN_NS


namespace {

void appendLine(QString& out, const QString& key, const QString& value)
{
    out.append(key).append(": ").append(value).append('\n');
}

QString hex(const QByteArray& bytes) { return QString::fromLatin1(bytes.toHex()); }

QString quoted(const QString& s)
{
    QString escaped = s;
    escaped.replace('\\', QStringLiteral("\\\\"));
    escaped.replace('"', QStringLiteral("\\\""));
    return QString("\"%1\"").arg(escaped);
}

QString assetToTxrep(Asset* a)
{
    if (!a) throw std::runtime_error("asset cannot be null");
    // SEP-11 / stc / py-stellar-sdk render the native asset as "native"
    // (NOT "XLM") — emit that for cross-tool interoperability. The parser
    // still accepts both spellings.
    if (dynamic_cast<AssetTypeNative*>(a)) return QStringLiteral("native");
    auto* alpha = dynamic_cast<AssetTypeCreditAlphaNum*>(a);
    if (!alpha) throw std::runtime_error("unsupported asset type");
    return alpha->getCode() + ":" + alpha->getIssuer().getAccountId();
}

// The XDR asset-type discriminator string (used by ChangeTrust's `line.type`).
QString assetTypeString(Asset* a)
{
    if (dynamic_cast<AssetTypeNative*>(a))            return QStringLiteral("ASSET_TYPE_NATIVE");
    if (dynamic_cast<AssetTypeCreditAlphaNum4*>(a))   return QStringLiteral("ASSET_TYPE_CREDIT_ALPHANUM4");
    if (dynamic_cast<AssetTypeCreditAlphaNum12*>(a))  return QStringLiteral("ASSET_TYPE_CREDIT_ALPHANUM12");
    throw std::runtime_error("unsupported asset type");
}

Asset* assetFromTxrep(const QString& s)
{
    if (s == "XLM" || s == "native") return new AssetTypeNative();
    int colon = s.indexOf(':');
    if (colon < 0) throw std::runtime_error("malformed asset in txrep");
    return Asset::create(s.left(colon), s.mid(colon + 1));
}

// Recursively emit a claim predicate tree (CreateClaimableBalance claimants).
void emitPredicate(QString& out, const QString& path, const Predicate& pred)
{
    if (dynamic_cast<const Predicate::Unconditional*>(&pred)) {
        appendLine(out, path + ".type", "CLAIM_PREDICATE_UNCONDITIONAL");
    } else if (auto* ab = dynamic_cast<const Predicate::AbsBefore*>(&pred)) {
        appendLine(out, path + ".type", "CLAIM_PREDICATE_BEFORE_ABSOLUTE_TIME");
        appendLine(out, path + ".absBefore",
                   QString::number(const_cast<Predicate::AbsBefore*>(ab)->getTimestampSeconds()));
    } else if (auto* rb = dynamic_cast<const Predicate::RelBefore*>(&pred)) {
        appendLine(out, path + ".type", "CLAIM_PREDICATE_BEFORE_RELATIVE_TIME");
        appendLine(out, path + ".relBefore",
                   QString::number(const_cast<Predicate::RelBefore*>(rb)->getSecondsSinceClose()));
    } else if (auto* notp = dynamic_cast<const Predicate::Not*>(&pred)) {
        appendLine(out, path + ".type", "CLAIM_PREDICATE_NOT");
        appendLine(out, path + ".notPredicate._present", "true");
        emitPredicate(out, path + ".notPredicate", notp->getInner());
    } else if (auto* andp = dynamic_cast<const Predicate::And*>(&pred)) {
        appendLine(out, path + ".type", "CLAIM_PREDICATE_AND");
        const QList<Predicate*> inner = andp->getInner();
        appendLine(out, path + ".andPredicates.len", QString::number(inner.size()));
        for (int j = 0; j < inner.size(); ++j)
            emitPredicate(out, path + QString(".andPredicates[%1]").arg(j), *inner.at(j));
    } else if (auto* orp = dynamic_cast<const Predicate::Or*>(&pred)) {
        appendLine(out, path + ".type", "CLAIM_PREDICATE_OR");
        const QList<Predicate*> inner = orp->getInner();
        appendLine(out, path + ".orPredicates.len", QString::number(inner.size()));
        for (int j = 0; j < inner.size(); ++j)
            emitPredicate(out, path + QString(".orPredicates[%1]").arg(j), *inner.at(j));
    } else {
        throw std::runtime_error("Txrep emitter: unknown claim predicate type");
    }
}

// AllowTrust is deprecated (Protocol 17's SetTrustLineFlags supersedes it), but
// its XDR operation still appears in pre-P17 transactions, so txrep must still
// round-trip it. Isolate the deprecated-symbol use in these two helpers so the
// warning is suppressed in exactly one place, cross-compiler.
#if defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable: 4996)
#elif defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
bool emitAllowTrust(QString& out, const QString& prefix, Operation* op)
{
    auto* at = dynamic_cast<AllowTrustOperation*>(op);
    if (!at) return false;
    appendLine(out, prefix + ".body.type", "ALLOW_TRUST");
    appendLine(out, prefix + ".body.allowTrustOp.trustor", at->getTrustor()->getAccountId());
    appendLine(out, prefix + ".body.allowTrustOp.asset", at->getAssetCode());
    // py-stellar-sdk / stc render `authorize` as a bool for the common 0/1
    // case. Preserve the rarer AUTHORIZED_TO_MAINTAIN_LIABILITIES flag (2)
    // numerically (no bool covers it).
    if (at->getAuthorize())
        appendLine(out, prefix + ".body.allowTrustOp.authorize", "true");
    else if (at->getAuthorizeToMaintainLiabilities())
        appendLine(out, prefix + ".body.allowTrustOp.authorize", "2");
    else
        appendLine(out, prefix + ".body.allowTrustOp.authorize", "false");
    return true;
}
#if defined(_MSC_VER)
#  pragma warning(pop)
#elif defined(__GNUC__)
#  pragma GCC diagnostic pop
#endif

void emitOperation(QString& out, Operation* op, int i)
{
    const QString prefix = QString("tx.operations[%1]").arg(i);
    if (op->getSourceAccount().isEmpty()) {
        appendLine(out, prefix + ".sourceAccount._present", "false");
    } else {
        appendLine(out, prefix + ".sourceAccount._present", "true");
        appendLine(out, prefix + ".sourceAccount", op->getSourceAccount());
    }

    if (auto* md = dynamic_cast<ManageDataOperation*>(op)) {
        appendLine(out, prefix + ".body.type", "MANAGE_DATA");
        appendLine(out, prefix + ".body.manageDataOp.dataName", quoted(md->getName()));
        const QByteArray value = md->getValue();
        if (value.isEmpty()) {
            appendLine(out, prefix + ".body.manageDataOp.dataValue._present", "false");
        } else {
            appendLine(out, prefix + ".body.manageDataOp.dataValue._present", "true");
            appendLine(out, prefix + ".body.manageDataOp.dataValue", hex(value));
        }
    } else if (auto* pay = dynamic_cast<PaymentOperation*>(op)) {
        appendLine(out, prefix + ".body.type", "PAYMENT");
        appendLine(out, prefix + ".body.paymentOp.destination", pay->getDestination());
        appendLine(out, prefix + ".body.paymentOp.asset", assetToTxrep(pay->getAsset()));
        appendLine(out, prefix + ".body.paymentOp.amount",
                   QString::number(Operation::toXdrAmount(pay->getAmount())));
    } else if (auto* ca = dynamic_cast<CreateAccountOperation*>(op)) {
        appendLine(out, prefix + ".body.type", "CREATE_ACCOUNT");
        appendLine(out, prefix + ".body.createAccountOp.destination",
                   ca->getDestination()->getAccountId());
        appendLine(out, prefix + ".body.createAccountOp.startingBalance",
                   QString::number(Operation::toXdrAmount(ca->getStartingBalance())));
    } else if (auto* am = dynamic_cast<AccountMergeOperation*>(op)) {
        // ACCOUNT_MERGE carries its destination directly in the body union
        // (no per-op struct), per the XDR / SEP-11 layout.
        appendLine(out, prefix + ".body.type", "ACCOUNT_MERGE");
        appendLine(out, prefix + ".body.destination", am->getDestination());
    } else if (auto* bs = dynamic_cast<BumpSequenceOperation*>(op)) {
        appendLine(out, prefix + ".body.type", "BUMP_SEQUENCE");
        appendLine(out, prefix + ".body.bumpSequenceOp.bumpTo", QString::number(bs->getBumpTo()));
    } else if (auto* so = dynamic_cast<SetOptionsOperation*>(op)) {
        appendLine(out, prefix + ".body.type", "SET_OPTIONS");
        const QString p = prefix + ".body.setOptionsOp";
        auto emitOptUint = [&](const QString& field, Integer opt) {
            if (opt.filled) {
                appendLine(out, p + "." + field + "._present", "true");
                appendLine(out, p + "." + field, QString::number(opt.value));
            } else {
                appendLine(out, p + "." + field + "._present", "false");
            }
        };
        KeyPair* inflation = so->getInflationDestination();
        if (inflation) {
            appendLine(out, p + ".inflationDest._present", "true");
            appendLine(out, p + ".inflationDest", inflation->getAccountId());
        } else {
            appendLine(out, p + ".inflationDest._present", "false");
        }
        emitOptUint("clearFlags",    so->getClearFlags());
        emitOptUint("setFlags",      so->getSetFlags());
        emitOptUint("masterWeight",  so->getMasterKeyWeight());
        emitOptUint("lowThreshold",  so->getLowThreshold());
        emitOptUint("medThreshold",  so->getMediumThreshold());
        emitOptUint("highThreshold", so->getHighThreshold());
        const QString homeDomain = so->getHomeDomain();
        if (!homeDomain.isNull()) {
            appendLine(out, p + ".homeDomain._present", "true");
            appendLine(out, p + ".homeDomain", quoted(homeDomain));
        } else {
            appendLine(out, p + ".homeDomain._present", "false");
        }
        stellar::SignerKey* signer = so->getSigner();
        if (signer) {
            appendLine(out, p + ".signer._present", "true");
            appendLine(out, p + ".signer.key", SignerKey::fromXdr(*signer).toString());
            appendLine(out, p + ".signer.weight", QString::number(so->getSignerWeight().value));
        } else {
            appendLine(out, p + ".signer._present", "false");
        }
    } else if (auto* ct = dynamic_cast<ChangeTrustOperation*>(op)) {
        Asset* line = ct->getAsset();  // null for the POOL_SHARE (liquidity pool) variant
        if (!line) {
            throw std::runtime_error("Txrep emitter: liquidity-pool ChangeTrust not yet supported");
        }
        appendLine(out, prefix + ".body.type", "CHANGE_TRUST");
        appendLine(out, prefix + ".body.changeTrustOp.line.type", assetTypeString(line));
        appendLine(out, prefix + ".body.changeTrustOp.line", assetToTxrep(line));
        appendLine(out, prefix + ".body.changeTrustOp.limit",
                   QString::number(Operation::toXdrAmount(ct->getLimit())));
    } else if (emitAllowTrust(out, prefix, op)) {
        // ALLOW_TRUST handled by the helper (isolates the deprecated symbol).
    } else if (auto* mso = dynamic_cast<ManageSellOfferOperation*>(op)) {
        appendLine(out, prefix + ".body.type", "MANAGE_SELL_OFFER");
        const QString p = prefix + ".body.manageSellOfferOp";
        const Price pr = mso->getPriceR();   // exact fraction from the XDR
        appendLine(out, p + ".selling", assetToTxrep(mso->getSelling()));
        appendLine(out, p + ".buying",  assetToTxrep(mso->getBuying()));
        appendLine(out, p + ".amount",  QString::number(Operation::toXdrAmount(mso->getAmount())));
        appendLine(out, p + ".price.n", QString::number(pr.getNumerator()));
        appendLine(out, p + ".price.d", QString::number(pr.getDenominator()));
        appendLine(out, p + ".offerID", QString::number(mso->getOfferId()));
    } else if (auto* mbo = dynamic_cast<ManageBuyOfferOperation*>(op)) {
        appendLine(out, prefix + ".body.type", "MANAGE_BUY_OFFER");
        const QString p = prefix + ".body.manageBuyOfferOp";
        const Price pr = mbo->getPriceR();   // exact fraction from the XDR
        appendLine(out, p + ".selling",   assetToTxrep(mbo->getSelling()));
        appendLine(out, p + ".buying",    assetToTxrep(mbo->getBuying()));
        appendLine(out, p + ".buyAmount", QString::number(Operation::toXdrAmount(mbo->getAmount())));
        appendLine(out, p + ".price.n",   QString::number(pr.getNumerator()));
        appendLine(out, p + ".price.d",   QString::number(pr.getDenominator()));
        appendLine(out, p + ".offerID",   QString::number(mbo->getOfferId()));
    } else if (auto* cpo = dynamic_cast<CreatePassiveSellOfferOperation*>(op)) {
        appendLine(out, prefix + ".body.type", "CREATE_PASSIVE_SELL_OFFER");
        const QString p = prefix + ".body.createPassiveSellOfferOp";
        const Price pr = cpo->getPriceR();   // exact fraction from the XDR
        appendLine(out, p + ".selling", assetToTxrep(cpo->getSelling()));
        appendLine(out, p + ".buying",  assetToTxrep(cpo->getBuying()));
        appendLine(out, p + ".amount",  QString::number(Operation::toXdrAmount(cpo->getAmount())));
        appendLine(out, p + ".price.n", QString::number(pr.getNumerator()));
        appendLine(out, p + ".price.d", QString::number(pr.getDenominator()));
    } else if (auto* ppr = dynamic_cast<PathPaymentStrictReceiveOperation*>(op)) {
        appendLine(out, prefix + ".body.type", "PATH_PAYMENT_STRICT_RECEIVE");
        const QString p = prefix + ".body.pathPaymentStrictReceiveOp";
        appendLine(out, p + ".sendAsset",  assetToTxrep(ppr->getSendAsset()));
        appendLine(out, p + ".sendMax",    QString::number(Operation::toXdrAmount(ppr->getSendMax())));
        appendLine(out, p + ".destination", ppr->getDestination());
        appendLine(out, p + ".destAsset",  assetToTxrep(ppr->getDestAsset()));
        appendLine(out, p + ".destAmount", QString::number(Operation::toXdrAmount(ppr->getDestAmount())));
        const QList<Asset*> path = ppr->getPath();
        appendLine(out, p + ".path.len", QString::number(path.size()));
        for (int k = 0; k < path.size(); ++k) {
            appendLine(out, p + QString(".path[%1]").arg(k), assetToTxrep(path.at(k)));
        }
    } else if (auto* pps = dynamic_cast<PathPaymentStrictSendOperation*>(op)) {
        appendLine(out, prefix + ".body.type", "PATH_PAYMENT_STRICT_SEND");
        const QString p = prefix + ".body.pathPaymentStrictSendOp";
        appendLine(out, p + ".sendAsset",   assetToTxrep(pps->getSendAsset()));
        appendLine(out, p + ".sendAmount",  QString::number(Operation::toXdrAmount(pps->getSendAmount())));
        appendLine(out, p + ".destination", pps->getDestination());
        appendLine(out, p + ".destAsset",   assetToTxrep(pps->getDestAsset()));
        appendLine(out, p + ".destMin",     QString::number(Operation::toXdrAmount(pps->getDestMin())));
        const QList<Asset*> path = pps->getPath();
        appendLine(out, p + ".path.len", QString::number(path.size()));
        for (int k = 0; k < path.size(); ++k) {
            appendLine(out, p + QString(".path[%1]").arg(k), assetToTxrep(path.at(k)));
        }
    } else if (auto* bsp = dynamic_cast<BeginSponsoringFutureReservesOperation*>(op)) {
        appendLine(out, prefix + ".body.type", "BEGIN_SPONSORING_FUTURE_RESERVES");
        appendLine(out, prefix + ".body.beginSponsoringFutureReservesOp.sponsoredID",
                   bsp->getSponsoredID());
    } else if (dynamic_cast<EndSponsoringFutureReservesOperation*>(op)) {
        // END_SPONSORING_FUTURE_RESERVES carries no fields.
        appendLine(out, prefix + ".body.type", "END_SPONSORING_FUTURE_RESERVES");
    } else if (auto* cb = dynamic_cast<ClawbackOperation*>(op)) {
        appendLine(out, prefix + ".body.type", "CLAWBACK");
        const QString p = prefix + ".body.clawbackOp";
        appendLine(out, p + ".asset",  assetToTxrep(cb->getAsset()));
        appendLine(out, p + ".from",   cb->getFrom());
        appendLine(out, p + ".amount", QString::number(Operation::toXdrAmount(cb->getAmount())));
    } else if (auto* ccb = dynamic_cast<ClawbackClaimableBalanceOperation*>(op)) {
        appendLine(out, prefix + ".body.type", "CLAWBACK_CLAIMABLE_BALANCE");
        appendLine(out, prefix + ".body.clawbackClaimableBalanceOp.balanceID", ccb->getBalanceId());
    } else if (auto* stf = dynamic_cast<SetTrustlineFlagsOperation*>(op)) {
        appendLine(out, prefix + ".body.type", "SET_TRUST_LINE_FLAGS");
        const QString p = prefix + ".body.setTrustLineFlagsOp";
        appendLine(out, p + ".trustor",    stf->getTrustor());
        appendLine(out, p + ".asset",      assetToTxrep(stf->getAsset()));
        appendLine(out, p + ".clearFlags", QString::number(static_cast<quint32>(stf->getClearFlags())));
        appendLine(out, p + ".setFlags",   QString::number(static_cast<quint32>(stf->getSetFlags())));
    } else if (auto* claim = dynamic_cast<ClaimClaimableBalanceOperation*>(op)) {
        appendLine(out, prefix + ".body.type", "CLAIM_CLAIMABLE_BALANCE");
        appendLine(out, prefix + ".body.claimClaimableBalanceOp.balanceID", claim->getBalanceId());
    } else if (auto* lpd = dynamic_cast<LiquidityPoolDepositOperation*>(op)) {
        appendLine(out, prefix + ".body.type", "LIQUIDITY_POOL_DEPOSIT");
        const QString p = prefix + ".body.liquidityPoolDepositOp";
        const Price minP = lpd->getMinPrice();
        const Price maxP = lpd->getMaxPrice();
        appendLine(out, p + ".liquidityPoolID", hex(lpd->getLiquidityPoolID()));
        appendLine(out, p + ".maxAmountA", QString::number(Operation::toXdrAmount(lpd->getMaxAmountA())));
        appendLine(out, p + ".maxAmountB", QString::number(Operation::toXdrAmount(lpd->getMaxAmountB())));
        appendLine(out, p + ".minPrice.n", QString::number(minP.getNumerator()));
        appendLine(out, p + ".minPrice.d", QString::number(minP.getDenominator()));
        appendLine(out, p + ".maxPrice.n", QString::number(maxP.getNumerator()));
        appendLine(out, p + ".maxPrice.d", QString::number(maxP.getDenominator()));
    } else if (auto* lpw = dynamic_cast<LiquidityPoolWithdrawOperation*>(op)) {
        appendLine(out, prefix + ".body.type", "LIQUIDITY_POOL_WITHDRAW");
        const QString p = prefix + ".body.liquidityPoolWithdrawOp";
        appendLine(out, p + ".liquidityPoolID", hex(lpw->getLiquidityPoolID()));
        appendLine(out, p + ".amount",     QString::number(Operation::toXdrAmount(lpw->getAmount())));
        appendLine(out, p + ".minAmountA", QString::number(Operation::toXdrAmount(lpw->getMinAmountA())));
        appendLine(out, p + ".minAmountB", QString::number(Operation::toXdrAmount(lpw->getMinAmountB())));
    } else if (auto* cclb = dynamic_cast<CreateClaimableBalanceOperation*>(op)) {
        appendLine(out, prefix + ".body.type", "CREATE_CLAIMABLE_BALANCE");
        const QString p = prefix + ".body.createClaimableBalanceOp";
        appendLine(out, p + ".asset",  assetToTxrep(cclb->getAsset()));
        appendLine(out, p + ".amount", QString::number(Operation::toXdrAmount(cclb->getAmount())));
        const QList<Claimant> claimants = cclb->getClaimants();
        appendLine(out, p + ".claimants.len", QString::number(claimants.size()));
        for (int j = 0; j < claimants.size(); ++j) {
            const QString cp = p + QString(".claimants[%1]").arg(j);
            appendLine(out, cp + ".type", "CLAIMANT_TYPE_V0");
            appendLine(out, cp + ".v0.destination", claimants.at(j).getDestination());
            emitPredicate(out, cp + ".v0.predicate", claimants.at(j).getPredicate());
        }
    } else if (auto* ra = dynamic_cast<RevokeAccountSponsorshipOperation*>(op)) {
        const QString p = prefix + ".body.revokeSponsorshipOp";
        appendLine(out, prefix + ".body.type", "REVOKE_SPONSORSHIP");
        appendLine(out, p + ".type", "REVOKE_SPONSORSHIP_LEDGER_ENTRY");
        appendLine(out, p + ".ledgerKey.type", "ACCOUNT");
        appendLine(out, p + ".ledgerKey.account", ra->getAccountId());
    } else if (auto* rt = dynamic_cast<RevokeTrustlineSponsorshipOperation*>(op)) {
        const QString p = prefix + ".body.revokeSponsorshipOp";
        appendLine(out, prefix + ".body.type", "REVOKE_SPONSORSHIP");
        appendLine(out, p + ".type", "REVOKE_SPONSORSHIP_LEDGER_ENTRY");
        appendLine(out, p + ".ledgerKey.type", "TRUSTLINE");
        appendLine(out, p + ".ledgerKey.trustLine.accountID", rt->getAccountId());
        appendLine(out, p + ".ledgerKey.trustLine.asset", assetToTxrep(rt->getAsset()));
    } else if (auto* ro = dynamic_cast<RevokeOfferSponsorshipOperation*>(op)) {
        const QString p = prefix + ".body.revokeSponsorshipOp";
        appendLine(out, prefix + ".body.type", "REVOKE_SPONSORSHIP");
        appendLine(out, p + ".type", "REVOKE_SPONSORSHIP_LEDGER_ENTRY");
        appendLine(out, p + ".ledgerKey.type", "OFFER");
        appendLine(out, p + ".ledgerKey.offer.sellerID", ro->getSeller());
        appendLine(out, p + ".ledgerKey.offer.offerID", QString::number(ro->getOfferId()));
    } else if (auto* rd = dynamic_cast<RevokeDataSponsorshipOperation*>(op)) {
        const QString p = prefix + ".body.revokeSponsorshipOp";
        appendLine(out, prefix + ".body.type", "REVOKE_SPONSORSHIP");
        appendLine(out, p + ".type", "REVOKE_SPONSORSHIP_LEDGER_ENTRY");
        appendLine(out, p + ".ledgerKey.type", "DATA");
        appendLine(out, p + ".ledgerKey.data.accountID", rd->getAccountId());
        // py / stc emit the revoke-data dataName UNQUOTED (unlike ManageData).
        appendLine(out, p + ".ledgerKey.data.dataName", rd->getDataName());
    } else if (auto* rcb = dynamic_cast<RevokeClaimableBalanceSponsorshipOperation*>(op)) {
        const QString p = prefix + ".body.revokeSponsorshipOp";
        appendLine(out, prefix + ".body.type", "REVOKE_SPONSORSHIP");
        appendLine(out, p + ".type", "REVOKE_SPONSORSHIP_LEDGER_ENTRY");
        appendLine(out, p + ".ledgerKey.type", "CLAIMABLE_BALANCE");
        appendLine(out, p + ".ledgerKey.claimableBalance.balanceID", rcb->getBalanceId());
    } else if (auto* rlp = dynamic_cast<RevokeLiquidityPoolSponsorshipOperation*>(op)) {
        const QString p = prefix + ".body.revokeSponsorshipOp";
        appendLine(out, prefix + ".body.type", "REVOKE_SPONSORSHIP");
        appendLine(out, p + ".type", "REVOKE_SPONSORSHIP_LEDGER_ENTRY");
        appendLine(out, p + ".ledgerKey.type", "LIQUIDITY_POOL");
        appendLine(out, p + ".ledgerKey.liquidityPool.liquidityPoolID", rlp->getLiquidityPoolId());
    } else if (auto* rs = dynamic_cast<RevokeSignerSponsorshipOperation*>(op)) {
        const QString p = prefix + ".body.revokeSponsorshipOp";
        appendLine(out, prefix + ".body.type", "REVOKE_SPONSORSHIP");
        appendLine(out, p + ".type", "REVOKE_SPONSORSHIP_SIGNER");
        appendLine(out, p + ".signer.accountID", rs->getAccountId());
        appendLine(out, p + ".signer.signerKey", SignerKey::fromXdr(rs->getSigner()).toString());
    } else {
        // Soroban ops (InvokeHostFunction / ExtendFootprintTTL / RestoreFootprint)
        // are intentionally NOT emitted: txrep is a pre-Soroban format and no
        // mainstream SDK supports them, so emitting a non-standard form would
        // not be interoperable. (A base64-XDR round-trip impl is stashed in
        // upgrade-plan/txrep-soroban-base64-stash.md.)
        throw std::runtime_error("Txrep emitter: unsupported operation type");
    }
}

QString unquote(const QString& s)
{
    if (s.size() < 2 || s.at(0) != '"' || s.at(s.size() - 1) != '"') {
        throw std::runtime_error("malformed quoted string in txrep");
    }
    QString body = s.mid(1, s.size() - 2);
    body.replace(QStringLiteral("\\\""), QStringLiteral("\""));
    body.replace(QStringLiteral("\\\\"), QStringLiteral("\\"));
    return body;
}

const QString& need(const QHash<QString, QString>& kv, const QString& key)
{
    auto it = kv.constFind(key);
    if (it == kv.constEnd()) {
        throw std::runtime_error(("missing txrep key: " + key).toStdString());
    }
    return it.value();
}

// Recursively parse a claim predicate tree. The returned Predicate owns its
// children (Not/And/Or delete their inners), so a single delete frees the tree.
Predicate* parsePredicate(const QHash<QString, QString>& kv, const QString& path)
{
    const QString type = need(kv, path + ".type");
    if (type == "CLAIM_PREDICATE_UNCONDITIONAL") {
        return new Predicate::Unconditional();
    } else if (type == "CLAIM_PREDICATE_BEFORE_ABSOLUTE_TIME") {
        return new Predicate::AbsBefore(need(kv, path + ".absBefore").toLongLong());
    } else if (type == "CLAIM_PREDICATE_BEFORE_RELATIVE_TIME") {
        return new Predicate::RelBefore(need(kv, path + ".relBefore").toLongLong());
    } else if (type == "CLAIM_PREDICATE_NOT") {
        return new Predicate::Not(parsePredicate(kv, path + ".notPredicate"));
    } else if (type == "CLAIM_PREDICATE_AND") {
        QList<Predicate*> inner;
        const int n = need(kv, path + ".andPredicates.len").toInt();
        for (int j = 0; j < n; ++j)
            inner.append(parsePredicate(kv, path + QString(".andPredicates[%1]").arg(j)));
        return new Predicate::And(inner);
    } else if (type == "CLAIM_PREDICATE_OR") {
        QList<Predicate*> inner;
        const int n = need(kv, path + ".orPredicates.len").toInt();
        for (int j = 0; j < n; ++j)
            inner.append(parsePredicate(kv, path + QString(".orPredicates[%1]").arg(j)));
        return new Predicate::Or(inner);
    }
    throw std::runtime_error(("Txrep parser: unknown claim predicate type: " + type).toStdString());
}

// See emitAllowTrust above — deprecated symbol isolated + warning suppressed.
#if defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable: 4996)
#elif defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
Operation* parseAllowTrust(const QHash<QString, QString>& kv, const QString& prefix)
{
    KeyPair* trustor = KeyPair::fromAccountId(need(kv, prefix + ".body.allowTrustOp.trustor"));
    const QString assetCode = need(kv, prefix + ".body.allowTrustOp.asset");
    // Accept the bool form (py/stc) and the numeric flag form interchangeably.
    const QString av = need(kv, prefix + ".body.allowTrustOp.authorize");
    const bool authorize = (av == "true" || av == "1");
    const bool maintain  = (av == "2");
    Operation* op = new AllowTrustOperation(trustor, assetCode, authorize, maintain);
    delete trustor;  // ctor copies the XDR pubkey, does not retain the pointer
    return op;
}
#if defined(_MSC_VER)
#  pragma warning(pop)
#elif defined(__GNUC__)
#  pragma GCC diagnostic pop
#endif

Operation* parseOperation(const QHash<QString, QString>& kv, int i)
{
    const QString prefix = QString("tx.operations[%1]").arg(i);
    const QString type = need(kv, prefix + ".body.type");

    Operation* op = nullptr;
    if (type == "MANAGE_DATA") {
        QString name = unquote(need(kv, prefix + ".body.manageDataOp.dataName"));
        const bool present = need(kv, prefix + ".body.manageDataOp.dataValue._present") == "true";
        QByteArray value;
        if (present) {
            value = QByteArray::fromHex(need(kv, prefix + ".body.manageDataOp.dataValue").toLatin1());
        }
        op = new ManageDataOperation(name, value);
    } else if (type == "PAYMENT") {
        QString dest   = need(kv, prefix + ".body.paymentOp.destination");
        Asset* asset   = assetFromTxrep(need(kv, prefix + ".body.paymentOp.asset"));
        qint64 stroops = need(kv, prefix + ".body.paymentOp.amount").toLongLong();
        op = new PaymentOperation(dest, asset, Operation::fromXdrAmount(stroops));
        delete asset;   // the ctor copies asset->toXdr(); it does not take ownership
    } else if (type == "CREATE_ACCOUNT") {
        QString dest   = need(kv, prefix + ".body.createAccountOp.destination");
        qint64 stroops = need(kv, prefix + ".body.createAccountOp.startingBalance").toLongLong();
        op = new CreateAccountOperation(dest, Operation::fromXdrAmount(stroops));
    } else if (type == "ACCOUNT_MERGE") {
        op = new AccountMergeOperation(need(kv, prefix + ".body.destination"));
    } else if (type == "BUMP_SEQUENCE") {
        op = new BumpSequenceOperation(need(kv, prefix + ".body.bumpSequenceOp.bumpTo").toLongLong());
    } else if (type == "SET_OPTIONS") {
        SetOptionsOperation* so = SetOptionsOperation::create();
        const QString p = prefix + ".body.setOptionsOp";
        if (kv.value(p + ".inflationDest._present") == "true") {
            KeyPair* dest = KeyPair::fromAccountId(need(kv, p + ".inflationDest"));
            so->setInflationDestination(dest);
            delete dest;  // setInflationDestination copies the XDR pubkey, no ownership
        }
        if (kv.value(p + ".clearFlags._present") == "true")
            so->setClearFlags(need(kv, p + ".clearFlags").toUInt());
        if (kv.value(p + ".setFlags._present") == "true")
            so->setSetFlags(need(kv, p + ".setFlags").toUInt());
        if (kv.value(p + ".masterWeight._present") == "true")
            so->setMasterKeyWeight(need(kv, p + ".masterWeight").toUInt());
        if (kv.value(p + ".lowThreshold._present") == "true")
            so->setLowThreshold(need(kv, p + ".lowThreshold").toUInt());
        if (kv.value(p + ".medThreshold._present") == "true")
            so->setMediumThreshold(need(kv, p + ".medThreshold").toUInt());
        if (kv.value(p + ".highThreshold._present") == "true")
            so->setHighThreshold(need(kv, p + ".highThreshold").toUInt());
        if (kv.value(p + ".homeDomain._present") == "true")
            so->setHomeDomain(unquote(need(kv, p + ".homeDomain")));
        if (kv.value(p + ".signer._present") == "true")
            so->setSigner(SignerKey::fromString(need(kv, p + ".signer.key")).toXdr(),
                          need(kv, p + ".signer.weight").toUInt());
        op = so;
    } else if (type == "CHANGE_TRUST") {
        Asset* asset = assetFromTxrep(need(kv, prefix + ".body.changeTrustOp.line"));
        qint64 limit = need(kv, prefix + ".body.changeTrustOp.limit").toLongLong();
        op = new ChangeTrustOperation(asset, Operation::fromXdrAmount(limit));
        delete asset;  // ctor copies asset->toXdr(), does not retain the pointer
    } else if (type == "ALLOW_TRUST") {
        op = parseAllowTrust(kv, prefix);
    } else if (type == "MANAGE_SELL_OFFER") {
        const QString p = prefix + ".body.manageSellOfferOp";
        Asset* selling = assetFromTxrep(need(kv, p + ".selling"));
        Asset* buying  = assetFromTxrep(need(kv, p + ".buying"));
        const QString amount = Operation::fromXdrAmount(need(kv, p + ".amount").toLongLong());
        // txrep carries the exact n and d: build the Price directly. Going
        // through the decimal string fell back to the Price(QString)
        // approximation, and 7/9 came back as 777777777/1000000000.
        const Price price(need(kv, p + ".price.n").toInt(),
                          need(kv, p + ".price.d").toInt());
        op = new ManageSellOfferOperation(selling, buying, amount, price,
                                          need(kv, p + ".offerID").toLongLong());
        delete selling; delete buying;  // ctor copies toXdr(), does not retain the pointers
    } else if (type == "MANAGE_BUY_OFFER") {
        const QString p = prefix + ".body.manageBuyOfferOp";
        Asset* selling = assetFromTxrep(need(kv, p + ".selling"));
        Asset* buying  = assetFromTxrep(need(kv, p + ".buying"));
        const QString amount = Operation::fromXdrAmount(need(kv, p + ".buyAmount").toLongLong());
        // txrep carries the exact n and d: build the Price directly. Going
        // through the decimal string fell back to the Price(QString)
        // approximation, and 7/9 came back as 777777777/1000000000.
        const Price price(need(kv, p + ".price.n").toInt(),
                          need(kv, p + ".price.d").toInt());
        op = new ManageBuyOfferOperation(selling, buying, amount, price,
                                         need(kv, p + ".offerID").toLongLong());
        delete selling; delete buying;
    } else if (type == "CREATE_PASSIVE_SELL_OFFER") {
        const QString p = prefix + ".body.createPassiveSellOfferOp";
        Asset* selling = assetFromTxrep(need(kv, p + ".selling"));
        Asset* buying  = assetFromTxrep(need(kv, p + ".buying"));
        const QString amount = Operation::fromXdrAmount(need(kv, p + ".amount").toLongLong());
        // txrep carries the exact n and d: build the Price directly. Going
        // through the decimal string fell back to the Price(QString)
        // approximation, and 7/9 came back as 777777777/1000000000.
        const Price price(need(kv, p + ".price.n").toInt(),
                          need(kv, p + ".price.d").toInt());
        op = new CreatePassiveSellOfferOperation(selling, buying, amount, price);
        delete selling; delete buying;
    } else if (type == "PATH_PAYMENT_STRICT_RECEIVE") {
        const QString p = prefix + ".body.pathPaymentStrictReceiveOp";
        Asset* sendAsset = assetFromTxrep(need(kv, p + ".sendAsset"));
        Asset* destAsset = assetFromTxrep(need(kv, p + ".destAsset"));
        const QString sendMax = Operation::fromXdrAmount(need(kv, p + ".sendMax").toLongLong());
        const QString destAmount = Operation::fromXdrAmount(need(kv, p + ".destAmount").toLongLong());
        QList<Asset*> path;
        const int pathLen = need(kv, p + ".path.len").toInt();
        for (int k = 0; k < pathLen; ++k) {
            path.append(assetFromTxrep(need(kv, p + QString(".path[%1]").arg(k))));
        }
        op = new PathPaymentStrictReceiveOperation(sendAsset, sendMax,
                 need(kv, p + ".destination"), destAsset, destAmount, path);
        delete sendAsset; delete destAsset;
        for (Asset* a : path) delete a;  // ctor copies toXdr(), does not retain the pointers
    } else if (type == "PATH_PAYMENT_STRICT_SEND") {
        const QString p = prefix + ".body.pathPaymentStrictSendOp";
        Asset* sendAsset = assetFromTxrep(need(kv, p + ".sendAsset"));
        Asset* destAsset = assetFromTxrep(need(kv, p + ".destAsset"));
        const QString sendAmount = Operation::fromXdrAmount(need(kv, p + ".sendAmount").toLongLong());
        const QString destMin = Operation::fromXdrAmount(need(kv, p + ".destMin").toLongLong());
        QList<Asset*> path;
        const int pathLen = need(kv, p + ".path.len").toInt();
        for (int k = 0; k < pathLen; ++k) {
            path.append(assetFromTxrep(need(kv, p + QString(".path[%1]").arg(k))));
        }
        op = new PathPaymentStrictSendOperation(sendAsset, sendAmount,
                 need(kv, p + ".destination"), destAsset, destMin, path);
        delete sendAsset; delete destAsset;
        for (Asset* a : path) delete a;
    } else if (type == "BEGIN_SPONSORING_FUTURE_RESERVES") {
        op = new BeginSponsoringFutureReservesOperation(
                 need(kv, prefix + ".body.beginSponsoringFutureReservesOp.sponsoredID"));
    } else if (type == "END_SPONSORING_FUTURE_RESERVES") {
        op = new EndSponsoringFutureReservesOperation();
    } else if (type == "CLAWBACK") {
        const QString p = prefix + ".body.clawbackOp";
        Asset* asset = assetFromTxrep(need(kv, p + ".asset"));
        const QString amount = Operation::fromXdrAmount(need(kv, p + ".amount").toLongLong());
        op = new ClawbackOperation(need(kv, p + ".from"), asset, amount);
        delete asset;  // ctor copies asset->toXdr(), does not retain the pointer
    } else if (type == "CLAWBACK_CLAIMABLE_BALANCE") {
        op = new ClawbackClaimableBalanceOperation(
                 need(kv, prefix + ".body.clawbackClaimableBalanceOp.balanceID"));
    } else if (type == "SET_TRUST_LINE_FLAGS") {
        const QString p = prefix + ".body.setTrustLineFlagsOp";
        Asset* asset = assetFromTxrep(need(kv, p + ".asset"));
        const QFlags<stellar::TrustLineFlags> clearFlags(QFlag(need(kv, p + ".clearFlags").toInt()));
        const QFlags<stellar::TrustLineFlags> setFlags(QFlag(need(kv, p + ".setFlags").toInt()));
        op = new SetTrustlineFlagsOperation(need(kv, p + ".trustor"), asset, clearFlags, setFlags);
        delete asset;  // ctor copies asset->toXdr(), does not retain the pointer
    } else if (type == "CLAIM_CLAIMABLE_BALANCE") {
        op = new ClaimClaimableBalanceOperation(
                 need(kv, prefix + ".body.claimClaimableBalanceOp.balanceID"));
    } else if (type == "LIQUIDITY_POOL_DEPOSIT") {
        const QString p = prefix + ".body.liquidityPoolDepositOp";
        const QByteArray poolId = QByteArray::fromHex(need(kv, p + ".liquidityPoolID").toLatin1());
        const QString maxA = Operation::fromXdrAmount(need(kv, p + ".maxAmountA").toLongLong());
        const QString maxB = Operation::fromXdrAmount(need(kv, p + ".maxAmountB").toLongLong());
        const Price minPrice(need(kv, p + ".minPrice.n").toInt(), need(kv, p + ".minPrice.d").toInt());
        const Price maxPrice(need(kv, p + ".maxPrice.n").toInt(), need(kv, p + ".maxPrice.d").toInt());
        op = new LiquidityPoolDepositOperation(poolId, maxA, maxB, minPrice, maxPrice);
    } else if (type == "LIQUIDITY_POOL_WITHDRAW") {
        const QString p = prefix + ".body.liquidityPoolWithdrawOp";
        const QByteArray poolId = QByteArray::fromHex(need(kv, p + ".liquidityPoolID").toLatin1());
        const QString amount = Operation::fromXdrAmount(need(kv, p + ".amount").toLongLong());
        const QString minA = Operation::fromXdrAmount(need(kv, p + ".minAmountA").toLongLong());
        const QString minB = Operation::fromXdrAmount(need(kv, p + ".minAmountB").toLongLong());
        op = new LiquidityPoolWithdrawOperation(poolId, amount, minA, minB);
    } else if (type == "CREATE_CLAIMABLE_BALANCE") {
        const QString p = prefix + ".body.createClaimableBalanceOp";
        Asset* asset = assetFromTxrep(need(kv, p + ".asset"));
        const QString amount = Operation::fromXdrAmount(need(kv, p + ".amount").toLongLong());
        QList<Claimant> claimants;
        const int n = need(kv, p + ".claimants.len").toInt();
        for (int j = 0; j < n; ++j) {
            const QString cp = p + QString(".claimants[%1]").arg(j);
            // Claimant takes ownership of the predicate; the local list frees it.
            claimants.append(Claimant(need(kv, cp + ".v0.destination"),
                                      parsePredicate(kv, cp + ".v0.predicate")));
        }
        op = new CreateClaimableBalanceOperation(amount, asset, claimants);
        delete asset;  // ctor copies asset->toXdr(), does not retain the pointer
    } else if (type == "REVOKE_SPONSORSHIP") {
        const QString p = prefix + ".body.revokeSponsorshipOp";
        const QString rtype = need(kv, p + ".type");
        if (rtype == "REVOKE_SPONSORSHIP_SIGNER") {
            op = new RevokeSignerSponsorshipOperation(
                     need(kv, p + ".signer.accountID"),
                     SignerKey::fromString(need(kv, p + ".signer.signerKey")).toXdr());
        } else {  // REVOKE_SPONSORSHIP_LEDGER_ENTRY
            const QString lk = need(kv, p + ".ledgerKey.type");
            if (lk == "ACCOUNT") {
                op = new RevokeAccountSponsorshipOperation(
                         need(kv, p + ".ledgerKey.account"));
            } else if (lk == "TRUSTLINE") {
                Asset* asset = assetFromTxrep(need(kv, p + ".ledgerKey.trustLine.asset"));
                op = new RevokeTrustlineSponsorshipOperation(
                         need(kv, p + ".ledgerKey.trustLine.accountID"), asset);
                delete asset;  // ctor copies asset->toXdr(), does not retain the pointer
            } else if (lk == "OFFER") {
                op = new RevokeOfferSponsorshipOperation(
                         need(kv, p + ".ledgerKey.offer.sellerID"),
                         need(kv, p + ".ledgerKey.offer.offerID").toLongLong());
            } else if (lk == "DATA") {
                op = new RevokeDataSponsorshipOperation(
                         need(kv, p + ".ledgerKey.data.accountID"),
                         need(kv, p + ".ledgerKey.data.dataName"));
            } else if (lk == "CLAIMABLE_BALANCE") {
                op = new RevokeClaimableBalanceSponsorshipOperation(
                         need(kv, p + ".ledgerKey.claimableBalance.balanceID"));
            } else if (lk == "LIQUIDITY_POOL") {
                op = new RevokeLiquidityPoolSponsorshipOperation(
                         need(kv, p + ".ledgerKey.liquidityPool.liquidityPoolID"));
            } else {
                throw std::runtime_error(
                    ("Txrep parser: unknown revoke ledgerKey type: " + lk).toStdString());
            }
        }
    } else {
        // Soroban op types are not parsed (see emit note + the stash file).
        throw std::runtime_error(("Txrep parser: unsupported operation type: " + type).toStdString());
    }

    if (kv.value(prefix + ".sourceAccount._present") == "true") {
        op->setSourceAccount(need(kv, prefix + ".sourceAccount"));
    }
    return op;
}

} // namespace

QString Txrep::toTxrep(Transaction* transaction)
{
    if (!transaction) {
        throw std::runtime_error("transaction cannot be null");
    }

    QString out;
    appendLine(out, "type", "ENVELOPE_TYPE_TX");
    appendLine(out, "tx.sourceAccount", transaction->getSourceAccount());
    appendLine(out, "tx.fee", QString::number(transaction->getFee()));
    appendLine(out, "tx.seqNum", QString::number(transaction->getSequenceNumber()));

    // Preconditions: PRECOND_NONE / PRECOND_TIME / PRECOND_V2.
    const TransactionPreconditions& pre = transaction->getPreconditions();
    TimeBounds* tb = pre.getTimeBounds();
    if (pre.hasV2()) {
        // PRECOND_V2: stc / py-stellar-sdk render the V2 fields directly under
        // `tx.cond.` (the union arm `.v2` is collapsed), and decompose each
        // extra signer into `.type` + a key-type-specific strkey field.
        appendLine(out, "tx.cond.type", "PRECOND_V2");
        if (tb) {
            appendLine(out, "tx.cond.timeBounds._present", "true");
            appendLine(out, "tx.cond.timeBounds.minTime", QString::number(tb->getMinTime()));
            appendLine(out, "tx.cond.timeBounds.maxTime", QString::number(tb->getMaxTime()));
        } else {
            appendLine(out, "tx.cond.timeBounds._present", "false");
        }
        LedgerBounds* lb = pre.getLedgerBounds();
        if (lb) {
            appendLine(out, "tx.cond.ledgerBounds._present", "true");
            appendLine(out, "tx.cond.ledgerBounds.minLedger", QString::number(lb->getMinLedger()));
            appendLine(out, "tx.cond.ledgerBounds.maxLedger", QString::number(lb->getMaxLedger()));
        } else {
            appendLine(out, "tx.cond.ledgerBounds._present", "false");
        }
        qint64* minSeq = pre.getMinSeqNumber();
        if (minSeq) {
            appendLine(out, "tx.cond.minSeqNum._present", "true");
            appendLine(out, "tx.cond.minSeqNum", QString::number(*minSeq));
        } else {
            appendLine(out, "tx.cond.minSeqNum._present", "false");
        }
        appendLine(out, "tx.cond.minSeqAge", QString::number(pre.getMinSeqAge()));
        appendLine(out, "tx.cond.minSeqLedgerGap", QString::number(pre.getMinSeqLedgerGap()));
        const QList<SignerKey>& signers = pre.getExtraSigners();
        appendLine(out, "tx.cond.extraSigners.len", QString::number(signers.size()));
        for (int i = 0; i < signers.size(); ++i) {
            const QString sp = QString("tx.cond.extraSigners[%1]").arg(i);
            switch (signers.at(i).getType()) {
            case stellar::SignerKeyType::SIGNER_KEY_TYPE_ED25519:
                appendLine(out, sp + ".type", "SIGNER_KEY_TYPE_ED25519");
                appendLine(out, sp + ".ed25519", signers.at(i).toString()); break;
            case stellar::SignerKeyType::SIGNER_KEY_TYPE_PRE_AUTH_TX:
                appendLine(out, sp + ".type", "SIGNER_KEY_TYPE_PRE_AUTH_TX");
                appendLine(out, sp + ".preAuthTx", signers.at(i).toString()); break;
            case stellar::SignerKeyType::SIGNER_KEY_TYPE_HASH_X:
                appendLine(out, sp + ".type", "SIGNER_KEY_TYPE_HASH_X");
                appendLine(out, sp + ".hashX", signers.at(i).toString()); break;
            case stellar::SignerKeyType::SIGNER_KEY_TYPE_ED25519_SIGNED_PAYLOAD:
                appendLine(out, sp + ".type", "SIGNER_KEY_TYPE_ED25519_SIGNED_PAYLOAD");
                appendLine(out, sp + ".ed25519SignedPayload", signers.at(i).toString()); break;
            }
        }
    } else if (tb) {
        appendLine(out, "tx.cond.type", "PRECOND_TIME");
        appendLine(out, "tx.cond.timeBounds.minTime", QString::number(tb->getMinTime()));
        appendLine(out, "tx.cond.timeBounds.maxTime", QString::number(tb->getMaxTime()));
    } else {
        appendLine(out, "tx.cond.type", "PRECOND_NONE");
    }

    // Memo — all SEP-11 types.
    Memo* memo = transaction->getMemo();
    if (!memo || dynamic_cast<MemoNone*>(memo)) {
        appendLine(out, "tx.memo.type", "MEMO_NONE");
    } else if (auto* mt = dynamic_cast<MemoText*>(memo)) {
        appendLine(out, "tx.memo.type", "MEMO_TEXT");
        appendLine(out, "tx.memo.text", quoted(mt->getText()));
    } else if (auto* mi = dynamic_cast<MemoId*>(memo)) {
        appendLine(out, "tx.memo.type", "MEMO_ID");
        appendLine(out, "tx.memo.id", QString::number(mi->getId()));
    } else if (auto* mh = dynamic_cast<MemoHash*>(memo)) {
        appendLine(out, "tx.memo.type", "MEMO_HASH");
        appendLine(out, "tx.memo.hash", mh->getHexValue());
    } else if (auto* mr = dynamic_cast<MemoReturnHash*>(memo)) {
        appendLine(out, "tx.memo.type", "MEMO_RETURN");
        appendLine(out, "tx.memo.retHash", mr->getHexValue());
    } else {
        throw std::runtime_error("Txrep emitter: unsupported memo type");
    }

    QVector<Operation*> ops = transaction->getOperations();
    appendLine(out, "tx.operations.len", QString::number(ops.size()));
    for (int i = 0; i < ops.size(); ++i) {
        emitOperation(out, ops.at(i), i);
    }

    appendLine(out, "tx.ext.v", "0");

    appendLine(out, "signatures.len", "0");
    return out;
}

Transaction* Txrep::fromTxrep(const QString& txrep, Network* network)
{
    if (!network) throw std::runtime_error("network cannot be null");

    // Parse the dotted-path key/value lines into a flat map.
    QHash<QString, QString> kv;
    for (const QString& rawLine : txrep.split('\n')) {
        QString line = rawLine.trimmed();
        if (line.isEmpty() || line.startsWith('#')) continue;
        int sep = line.indexOf(':');
        if (sep < 0) continue;
        QString value = line.mid(sep + 1).trimmed();
        // stc / Laboratory / py-stellar-sdk append a human-readable comment to
        // typed values, e.g. `amount: 105000000 (10.5)` or
        // `minTime: 0 (1970-01-01 ... (UTC))`. Strip a trailing ` (...)` so we
        // can parse those tools' output. Quoted strings are taken verbatim
        // (their content may legitimately contain " (").
        if (!value.startsWith('"')) {
            int comment = value.indexOf(" (");
            if (comment >= 0) value = value.left(comment).trimmed();
        }
        kv.insert(line.left(sep).trimmed(), value);
    }

    if (kv.value("type") != "ENVELOPE_TYPE_TX") {
        throw std::runtime_error("Txrep parser: only ENVELOPE_TYPE_TX is supported");
    }

    QString sourceAccount = need(kv, "tx.sourceAccount");
    qint64 fee = need(kv, "tx.fee").toLongLong();
    qint64 seqNum = need(kv, "tx.seqNum").toLongLong();

    // Account starts at seqNum-1 so that build() increments it to seqNum.
    KeyPair* kp = KeyPair::fromAccountId(sourceAccount);
    Account* account = new Account(kp, seqNum - 1);

    TransactionBuilder builder(AccountConverter().enableMuxed(), account, network);
    const qint64 opCount = need(kv, "tx.operations.len").toLongLong();
    builder.setBaseFee(opCount > 0 ? static_cast<quint32>(fee / opCount) : TransactionBuilder::BASE_FEE);

    QString condType = kv.value("tx.cond.type", "PRECOND_NONE");
    if (condType == "PRECOND_TIME") {
        qint64 minT = need(kv, "tx.cond.timeBounds.minTime").toLongLong();
        qint64 maxT = need(kv, "tx.cond.timeBounds.maxTime").toLongLong();
        builder.addTimeBounds(new TimeBounds(minT, maxT));
    } else if (condType == "PRECOND_V2") {
        TransactionPreconditions pre;
        if (kv.value("tx.cond.timeBounds._present") == "true") {
            pre.setTimeBounds(new TimeBounds(
                need(kv, "tx.cond.timeBounds.minTime").toLongLong(),
                need(kv, "tx.cond.timeBounds.maxTime").toLongLong()));
        }
        if (kv.value("tx.cond.ledgerBounds._present") == "true") {
            pre.setLedgerBounds(new LedgerBounds(
                need(kv, "tx.cond.ledgerBounds.minLedger").toUInt(),
                need(kv, "tx.cond.ledgerBounds.maxLedger").toUInt()));
        }
        if (kv.value("tx.cond.minSeqNum._present") == "true") {
            pre.setMinSeqNumber(new qint64(need(kv, "tx.cond.minSeqNum").toLongLong()));
        }
        pre.setMinSeqAge(kv.value("tx.cond.minSeqAge", "0").toULongLong());
        pre.setMinSeqLedgerGap(kv.value("tx.cond.minSeqLedgerGap", "0").toUInt());
        const int signerCount = kv.value("tx.cond.extraSigners.len", "0").toInt();
        for (int i = 0; i < signerCount; ++i) {
            // Each extra signer is `.type` + a key-type-specific strkey field;
            // SignerKey::fromString auto-detects from the G/T/X/P prefix, so we
            // just read whichever field the type names.
            const QString sp = QString("tx.cond.extraSigners[%1]").arg(i);
            const QString st = need(kv, sp + ".type");
            QString field = "ed25519";
            if (st == "SIGNER_KEY_TYPE_PRE_AUTH_TX")               field = "preAuthTx";
            else if (st == "SIGNER_KEY_TYPE_HASH_X")               field = "hashX";
            else if (st == "SIGNER_KEY_TYPE_ED25519_SIGNED_PAYLOAD") field = "ed25519SignedPayload";
            pre.addExtraSigner(SignerKey::fromString(need(kv, sp + "." + field)));
        }
        builder.addPreconditions(pre);
        // PRECOND_V2 without timeBounds still needs the builder's timeout
        // invariant satisfied (build() requires bounds or an explicit timeout).
        if (!pre.getTimeBounds()) {
            builder.setTimeout(TransactionBuilder::TIMEOUT_INFINITE);
        }
    } else if (condType == "PRECOND_NONE") {
        builder.setTimeout(TransactionBuilder::TIMEOUT_INFINITE);
    } else {
        delete account;
        throw std::runtime_error(("Txrep parser: unsupported cond type: " + condType).toStdString());
    }

    const QString memoType = kv.value("tx.memo.type", "MEMO_NONE");
    if (memoType == "MEMO_NONE") {
        builder.addMemo(Memo::none());
    } else if (memoType == "MEMO_TEXT") {
        builder.addMemo(Memo::text(unquote(need(kv, "tx.memo.text"))));
    } else if (memoType == "MEMO_ID") {
        builder.addMemo(Memo::id(need(kv, "tx.memo.id").toULongLong()));
    } else if (memoType == "MEMO_HASH") {
        builder.addMemo(Memo::hash(need(kv, "tx.memo.hash")));
    } else if (memoType == "MEMO_RETURN") {
        builder.addMemo(Memo::returnHash(need(kv, "tx.memo.retHash")));
    } else {
        delete account;
        throw std::runtime_error(("Txrep parser: unsupported memo type: " + memoType).toStdString());
    }

    for (int i = 0; i < opCount; ++i) {
        builder.addOperation(parseOperation(kv, i));
    }

    Transaction* tx = builder.build();
    delete account; // builder doesn't own it
    return tx;
}
QSTELLAR_END_NS
