#include "abstracttransaction.h"
#include "transaction.h"
#include "feebumptransaction.h"

AbstractTransaction::AbstractTransaction(AccountConverter accountConverter, Network *network) : m_accountConverter(accountConverter), m_network(network)
{    
    checkNotNull(network, "network cannot be null");
}
void AbstractTransaction::sign(KeyPair *signer) {
    checkNotNull(signer, "signer cannot be null");
    QByteArray txHash = this->hash();

    m_signatures.append(signer->signDecorated(txHash));
}

void AbstractTransaction::sign(QByteArray preimage) {
    checkNotNull(preimage, "preimage cannot be null");
    stellar::DecoratedSignature decoratedSignature;
    stellar::Signature &signature = decoratedSignature.signature;

    signature.set(reinterpret_cast<uchar*>(preimage.data()),preimage.length());


    QByteArray hash = Util::hash(preimage);
    QByteArray signatureHintBytes =hash.right(4);

    stellar::SignatureHint &signatureHint = decoratedSignature.hint;
    memcpy(signatureHint.signatureHint,signatureHintBytes.data(),4);


    decoratedSignature.hint = signatureHint;
    decoratedSignature.signature = signature;


    m_signatures.append(decoratedSignature);
}

QByteArray AbstractTransaction::hash() const
{
    return Util::hash(this->signatureBase());
}

QString AbstractTransaction::hashHex() const
{
    return QString::fromLatin1(hash().toHex());
}

Network *AbstractTransaction::getNetwork() const
{
    return m_network;
}

QVector<stellar::DecoratedSignature> AbstractTransaction::getSignatures() const {
    return m_signatures;
}

AbstractTransaction *AbstractTransaction::fromEnvelopeXdr(stellar::TransactionEnvelope &xdr, Network* network)
{
    switch(xdr.type)
    {
    case stellar::EnvelopeType::ENVELOPE_TYPE_TX:
    {
        return Transaction::fromV1EnvelopeXdr(xdr.v1,network);
    }
    case stellar::EnvelopeType::ENVELOPE_TYPE_TX_V0:
    {
        return Transaction::fromV0EnvelopeXdr(xdr.v0,network);
    }
    case stellar::EnvelopeType::ENVELOPE_TYPE_TX_FEE_BUMP:
    {
        return FeeBumpTransaction::fromFeeBumpTransactionEnvelope(xdr.feeBump, network);
    }
    default: throw std::runtime_error("transaction type is not supported");
    }


}

AbstractTransaction *AbstractTransaction::fromEnvelopeXdr(QString envelope, Network *network)
{
    QByteArray data = QByteArray::fromBase64(envelope.toLatin1(),XDR_BASE64ENCODING);
    QDataStream stream(&data, QIODevice::ReadOnly);
    stellar::TransactionEnvelope xdr;
    stream >> xdr;
    return fromEnvelopeXdr(xdr,network);
}

QString AbstractTransaction::toEnvelopeXdrBase64() {

    stellar::TransactionEnvelope envelope =this->toEnvelopeXdr();
    QByteArray outputStream;
    QDataStream xdrOutputStream(&outputStream,QIODevice::WriteOnly);
    xdrOutputStream<< envelope;
    return outputStream.toBase64(XDR_BASE64ENCODING);
}
