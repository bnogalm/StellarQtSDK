#include "abstracttransaction.h"

#include <memory>
#include <stdexcept>
#include "transaction.h"
#include "feebumptransaction.h"
#include <cstring>

QSTELLAR_BEGIN_NS

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

    // Array::set clamps with qMin: a longer preimage was silently truncated
    // and we shipped a signature that can never satisfy the hash.
    if (preimage.length() > signature.maxSize()) {
        throw std::runtime_error("hash-x preimage exceeds 64 bytes");
    }
    signature.set(reinterpret_cast<uchar*>(preimage.data()),preimage.length());


    QByteArray hash = Util::hash(preimage);
    QByteArray signatureHintBytes =hash.right(4);

    stellar::SignatureHint &signatureHint = decoratedSignature.hint;
    memcpy(signatureHint.signatureHint,signatureHintBytes.data(),4);


    decoratedSignature.hint = signatureHint;
    decoratedSignature.signature = signature;


    m_signatures.append(decoratedSignature);
}

void AbstractTransaction::addSignature(const stellar::DecoratedSignature& signature)
{
    m_signatures.append(signature);
}

void AbstractTransaction::addSignature(const QString& publicKey, const QString& signatureBase64)
{
    const QByteArray bytes = signatureBase64.toLatin1();
    const auto decoded = QByteArray::fromBase64Encoding(bytes, QByteArray::AbortOnBase64DecodingErrors);
    if (!decoded || decoded.decoded.isEmpty())
        throw std::runtime_error("signature is not base64");
    std::unique_ptr<KeyPair> key(KeyPair::fromAccountId(publicKey));
    if (decoded.decoded.size() != 64 || !key->verify(hash(), decoded.decoded))
        throw std::runtime_error("Invalid signature");
    stellar::DecoratedSignature signature;
    signature.hint = key->getSignatureHint();
    QByteArray raw = decoded.decoded;
    signature.signature.set(reinterpret_cast<uchar*>(raw.data()), 64);
    m_signatures.append(signature);
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
    // Whitespace (an envelope pasted wrapped over lines) is not part of it;
    // anything else outside the base64 alphabet is an error, not skipped.
    QString compact;
    for (const QChar c : envelope) {
        if (!c.isSpace())                   // any whitespace, NBSP included
            compact.append(c);
    }
    const QByteArray text = compact.toLatin1();
    const auto decoded = QByteArray::fromBase64Encoding(
        text, XDR_BASE64ENCODING | QByteArray::AbortOnBase64DecodingErrors);
    if (!decoded || (*decoded).isEmpty()) {
        throw std::runtime_error("the transaction envelope is not base64");
    }
    QByteArray data = *decoded;
    QDataStream stream(&data, QIODevice::ReadOnly);
    stellar::TransactionEnvelope xdr;
    stream >> xdr;
    // In full or not at all. A truncated envelope used to decode with the
    // missing fields as zeros (an offer update became a new offer) and was
    // signed as such; trailing bytes were ignored.
    if (stream.status() != QDataStream::Ok || !stream.atEnd()) {
        throw std::runtime_error("the transaction envelope does not decode: it is truncated, "
                                 "malformed, or followed by other bytes");
    }
    std::unique_ptr<AbstractTransaction> transaction(fromEnvelopeXdr(xdr, network));

    // Written back, it must be the envelope given, byte for byte. Whatever the
    // SDK cannot hold exactly (a memo text that is not UTF-8, an arm it does
    // not know) would otherwise be signed, hashed and shown as a different
    // transaction from the one given.
    const QByteArray again = xdr::encodeOrThrow(transaction->toEnvelopeXdr(), "the transaction envelope");
    if (again != data) {
        throw std::runtime_error("this envelope cannot be represented exactly by this SDK, "
                                 "so it is not read: signing it would sign a different transaction");
    }
    return transaction.release();
}

QString AbstractTransaction::toEnvelopeXdrBase64() {
    // Checked: a writer that met something it has no encoding for (an arm it
    // does not know) left the stream failed and the bytes short, and those
    // were base64'd and handed out as the envelope to submit.
    return xdr::encodeOrThrow(this->toEnvelopeXdr(), "the transaction envelope")
        .toBase64(XDR_BASE64ENCODING);
}

QSTELLAR_END_NS
