#include "feebumptransaction.h"


FeeBumpTransaction::FeeBumpTransaction(AccountConverter accountConverter, QString feeAccount, qint64 fee, Transaction *innerTransaction):AbstractTransaction(accountConverter, innerTransaction ? innerTransaction->getNetwork(): nullptr)
{
    m_feeAccount = checkNotNull(feeAccount, "feeAccount cannot be null");
    m_inner = checkNotNull(innerTransaction, "innerTransaction cannot be null");
    m_fee = fee;
}

FeeBumpTransaction::~FeeBumpTransaction()
{

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

FeeBumpTransaction::Builder::Builder(AccountConverter accountConverter, Transaction *inner) {
    m_inner = checkNotNull(inner, "inner cannot be null");
    m_baseFee = 0;
    m_accountConverter = accountConverter;

    stellar::EnvelopeType txType = inner->toEnvelopeXdr().type;
    if (txType == stellar::EnvelopeType::ENVELOPE_TYPE_TX_V0) {
        m_inner = new Transaction(
                    accountConverter,
                    inner->getSourceAccount(),
                    inner->getFee(),
                    inner->getSequenceNumber(),
                    inner->getOperations(),
                    inner->getMemo(),
                    inner->getTimeBounds(),
                    inner->getNetwork()
                    );
        m_inner->m_signatures = inner->getSignatures();
    }
}

FeeBumpTransaction::Builder::~Builder()
{
    if(m_inner)
        delete m_inner;
}

FeeBumpTransaction::Builder &FeeBumpTransaction::Builder::setBaseFee(qint64 baseFee) {
    if (m_baseFee != 0) {
        throw std::runtime_error("base fee has been already set.");
    }

    if (baseFee < AbstractTransaction::MIN_BASE_FEE) {
        throw std::runtime_error("baseFee cannot be smaller than the BASE_FEE");
    }

    qint64 innerBaseFee = m_inner->getFee();
    qint64 numOperations = m_inner->getOperations().length();
    if (numOperations > 0) {
        innerBaseFee = innerBaseFee / numOperations;
    }

    if (baseFee < innerBaseFee) {
        throw std::runtime_error("base fee cannot be lower than provided inner transaction base fee");
    }

    qint64 maxFee = baseFee * (numOperations + 1);
    if (maxFee < 0) {
        throw std::runtime_error("fee overflows 64 bit int");
    }

    m_baseFee = maxFee;
    return *this;
}

FeeBumpTransaction::Builder &FeeBumpTransaction::Builder::setFeeAccount(QString feeAccount) {
    if (!m_feeAccount.isEmpty()) {
        throw std::runtime_error("fee account has been already been set.");
    }

    m_feeAccount = checkNotNull(feeAccount, "feeAccount cannot be null");
    return *this;
}

FeeBumpTransaction *FeeBumpTransaction::Builder::build() {
    FeeBumpTransaction* result = new FeeBumpTransaction(
                m_accountConverter,
                checkNotNull(m_feeAccount, "fee account has to be set. you must call setFeeAccount()."),
                checkNotNull(m_baseFee, "base fee has to be set. you must call setBaseFee()."),
                m_inner
                );
    m_inner = nullptr;
    return result;
}
