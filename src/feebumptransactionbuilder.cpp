#include "feebumptransactionbuilder.h"
#include "feebumptransaction.h"
#include "transaction.h"
#include "abstracttransaction.h"
#include "util.h"
#include <limits>

QSTELLAR_BEGIN_NS

FeeBumpTransactionBuilder::FeeBumpTransactionBuilder(AccountConverter accountConverter, Transaction* inner)
{
    m_inner = checkNotNull(inner, "inner cannot be null");
    m_baseFee = 0;
    m_accountConverter = accountConverter;

    stellar::EnvelopeType txType = inner->toEnvelopeXdr().type;
    if (txType == stellar::EnvelopeType::ENVELOPE_TYPE_TX_V0) {
        // FIX §1.2: V0 inputs need a deep clone (XDR round-trip to V1) so we
        // don't share raw pointers with the caller's `inner` (double-free).
        stellar::TransactionV1Envelope v1env;
        v1env.tx = inner->toV1Xdr(accountConverter);
        for (stellar::DecoratedSignature& sig : inner->getSignatures()) {
            v1env.signatures.append(sig);
        }
        m_inner = Transaction::fromV1EnvelopeXdr(accountConverter, v1env, inner->getNetwork());
    }
}

FeeBumpTransactionBuilder::~FeeBumpTransactionBuilder()
{
    if (m_inner)
        delete m_inner;
}

FeeBumpTransactionBuilder& FeeBumpTransactionBuilder::setBaseFee(qint64 baseFee)
{
    if (m_baseFee != 0) {
        throw std::runtime_error("base fee has been already set.");
    }
    if (baseFee < AbstractTransaction::MIN_BASE_FEE) {
        throw std::runtime_error("baseFee cannot be smaller than the BASE_FEE");
    }

    const qint64 numOperations = qMax<qint64>(1, m_inner->getOperations().length());

    // A Soroban inner transaction's fee includes its resource fee, which is
    // not a per-operation rate. It used to be counted as one — refusing base
    // fees the network accepts and roughly doubling the smallest bid that
    // passed. As js-stellar-base does: compare against the inclusion fee
    // alone, and add the resource fee back once.
    qint64 resourceFee = 0;
    if (auto data = m_inner->getSorobanData()) {
        resourceFee = data->resourceFee;
        if (resourceFee < 0)
            throw std::runtime_error("the inner transaction carries a negative resource fee");
    }
    const qint64 innerInclusionFee = qMax<qint64>(0, m_inner->getFee() - resourceFee);

    // Compared exactly, the way core does (baseFee * ops >= inner inclusion
    // fee). A truncating per-operation division let bids through that core
    // then refused: a 301-stroop inner fee over three operations accepted 100.
    if (baseFee > std::numeric_limits<qint64>::max() / numOperations) {
        throw std::runtime_error("fee overflows 64 bit int");
    }
    if (baseFee * numOperations < innerInclusionFee) {
        throw std::runtime_error("base fee cannot be lower than provided inner transaction base fee");
    }
    if (baseFee > (std::numeric_limits<qint64>::max() - resourceFee) / (numOperations + 1)) {
        throw std::runtime_error("fee overflows 64 bit int");
    }
    qint64 maxFee = baseFee * (numOperations + 1) + resourceFee;

    m_baseFee = maxFee;
    return *this;
}

FeeBumpTransactionBuilder& FeeBumpTransactionBuilder::setFeeAccount(QString feeAccount)
{
    if (!m_feeAccount.isEmpty()) {
        throw std::runtime_error("fee account has been already been set.");
    }
    m_feeAccount = checkNotNull(feeAccount, "feeAccount cannot be null");
    return *this;
}

FeeBumpTransaction* FeeBumpTransactionBuilder::build()
{
    FeeBumpTransaction* result = new FeeBumpTransaction(
        m_accountConverter,
        checkNotNull(m_feeAccount, "fee account has to be set. you must call setFeeAccount()."),
        checkNotNull(m_baseFee, "base fee has to be set. you must call setBaseFee()."),
        m_inner);
    m_inner = nullptr;
    return result;
}

QSTELLAR_END_NS
