#include "feebumptransaction.h"

QSTELLAR_BEGIN_NS

FeeBumpTransaction::FeeBumpTransaction(AccountConverter accountConverter, QString feeAccount, qint64 fee, Transaction *innerTransaction):AbstractTransaction(accountConverter, innerTransaction ? innerTransaction->getNetwork(): nullptr)
{
    m_feeAccount = checkNotNull(feeAccount, "feeAccount cannot be null");
    m_inner = checkNotNull(innerTransaction, "innerTransaction cannot be null");
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
    xdr.feeSource = StrKey::encodeToXDRMuxedAccount(m_feeAccount);
    xdr.type = stellar::EnvelopeType::ENVELOPE_TYPE_TX;
    auto innerEnvelope= m_inner->toEnvelopeXdr();
    xdr.v1.tx = innerEnvelope.v1.tx;
    xdr.v1.signatures = innerEnvelope.v1.signatures;
    return xdr;
}

QByteArray FeeBumpTransaction::signatureBase() const
{
    try {

        stellar::TransactionSignaturePayload payload(this->toXdr(), m_network->getNetworkId());
        QByteArray output;
        QDataStream outputStream(&output,QIODevice::WriteOnly);
        outputStream << payload;
        return output;
    } catch (const std::exception& e) {
        Q_UNUSED(e)
        return QByteArray();
    }
}

stellar::TransactionEnvelope FeeBumpTransaction::toEnvelopeXdr()
{
    stellar::FeeBumpTransactionEnvelope envelope;
    envelope.tx=toXdr();
    for(stellar::DecoratedSignature& signature : this->m_signatures){
        envelope.signatures.append(signature);
    }
    return stellar::TransactionEnvelope(envelope);
}

// FeeBumpTransactionBuilder lives in feebumptransactionbuilder.cpp.
// FeeBumpTransaction::Builder is now a deprecated alias (see header).

QSTELLAR_END_NS
