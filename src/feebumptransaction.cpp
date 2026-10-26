#include "feebumptransaction.h"

QSTELLAR_BEGIN_NS

FeeBumpTransaction::FeeBumpTransaction(AccountConverter accountConverter, QString feeAccount, qint64 fee, Transaction *innerTransaction):AbstractTransaction(accountConverter, innerTransaction ? innerTransaction->getNetwork(): nullptr)
{
    m_feeAccount = checkNotNull(feeAccount, "feeAccount cannot be null");
    m_inner = checkNotNull(innerTransaction, "innerTransaction cannot be null");
    // The builder checks the fee; this constructor is public too, and signed
    // whatever it was given, -5 included.
    if (fee < 0) {
        throw std::runtime_error("fee bump fee cannot be negative");
    }
    m_fee = fee;
}

FeeBumpTransaction::~FeeBumpTransaction()
{
    // FIX §1.1: empty destructor leaked the inner Transaction (and its ops,
    // memo, timeBounds). Builder transferred ownership; nothing freed it.
    if (m_inner)
        delete m_inner;
}

qint64 FeeBumpTransaction::getFee() const
{
    return m_fee;
}

QString FeeBumpTransaction::getFeeAccount() const
{
    return m_feeAccount;
}

Transaction *FeeBumpTransaction::getInnerTransaction() const
{
    return m_inner;
}

FeeBumpTransaction *FeeBumpTransaction::fromFeeBumpTransactionEnvelope(AccountConverter accountConverter, stellar::FeeBumpTransactionEnvelope envelope, Network *network)
{

    Transaction* inner = Transaction::fromV1EnvelopeXdr(accountConverter, envelope.tx.v1, network);
    QString feeAccount = accountConverter.decode(envelope.tx.feeSource);
    qint64 fee = envelope.tx.fee;

    FeeBumpTransaction* feeBump = new FeeBumpTransaction(accountConverter, feeAccount, fee, inner);

    for(stellar::DecoratedSignature& signature : envelope.signatures.value){
        feeBump->m_signatures.append(signature);
    }

    return feeBump;
}

FeeBumpTransaction *FeeBumpTransaction::fromFeeBumpTransactionEnvelope(stellar::FeeBumpTransactionEnvelope envelope, Network *network)
{
    return fromFeeBumpTransactionEnvelope(AccountConverter().enableMuxed(), envelope, network);
}

stellar::FeeBumpTransaction FeeBumpTransaction::toXdr() const
{
    stellar::FeeBumpTransaction xdr;
    xdr.fee = m_fee;
    // Through the converter, as the inner transaction's accounts are: with
    // muxing off an M... fee source was still written M...
    AccountConverter converter = m_accountConverter;
    xdr.feeSource = converter.encode(m_feeAccount);
    xdr.type = stellar::EnvelopeType::ENVELOPE_TYPE_TX;
    auto innerEnvelope= m_inner->toEnvelopeXdr();
    if (innerEnvelope.type == stellar::EnvelopeType::ENVELOPE_TYPE_TX) {
        xdr.v1.tx = innerEnvelope.v1.tx;
        xdr.v1.signatures = innerEnvelope.v1.signatures;
    } else if (innerEnvelope.type == stellar::EnvelopeType::ENVELOPE_TYPE_TX_V0) {
        // A fee bump wraps a v1 envelope. A v0 inner is the same transaction
        // written the old way (same hash, same signatures), so it is written
        // as v1, as FeeBumpTransactionBuilder does. Reading .v1 of a v0
        // envelope was reading the inactive union arm.
        xdr.v1.tx = m_inner->toV1Xdr(m_accountConverter);
        xdr.v1.signatures = innerEnvelope.v0.signatures;
    } else {
        throw std::runtime_error("a fee bump can only wrap a transaction");
    }
    return xdr;
}

QByteArray FeeBumpTransaction::signatureBase() const
{
    // No catch-all: see Transaction::signatureBase. An empty payload here was
    // signed as sha256("") and reported as a signature.
    // Nor a failed write: a payload cut short where the writer met an arm it
    // does not know was hashed and signed as if it were the fee bump.
    stellar::TransactionSignaturePayload payload(this->toXdr(), m_network->getNetworkId());
    return xdr::encodeOrThrow(payload, "the fee bump signature payload");
}

stellar::TransactionEnvelope FeeBumpTransaction::toEnvelopeXdr()
{
    stellar::FeeBumpTransactionEnvelope envelope;
    envelope.tx=toXdr();
    // Same guard as Transaction::toEnvelopeXdr: the array drops signatures
    // past its maximum without a word.
    if (this->m_signatures.size() > envelope.signatures.maxSize()) {
        throw std::runtime_error("fee bump exceeds the maximum number of signatures");
    }
    for(stellar::DecoratedSignature& signature : this->m_signatures){
        envelope.signatures.append(signature);
    }
    stellar::TransactionEnvelope result(envelope);
    // Whatever the caller does with it next, it can be encoded in full: see
    // AbstractTransaction::signatureBase.
    xdr::encodeOrThrow(result, "the fee bump envelope");
    return result;
}

// FeeBumpTransactionBuilder lives in feebumptransactionbuilder.cpp.
// FeeBumpTransaction::Builder is now a deprecated alias (see header).

QSTELLAR_END_NS
