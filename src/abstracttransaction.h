#ifndef ABSTRACTTRANSACTION_H
#define ABSTRACTTRANSACTION_H

#include <QObject>
#include "xdr/stellartransaction.h"
#include "keypair.h"
#include "operation.h"
#include "memo.h"
#include "network.h"
#include "accountconverter.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

class AbstractTransaction : public QObject
{
    Q_OBJECT
protected:
    AccountConverter m_accountConverter;
    Network* m_network;
    QVector<stellar::DecoratedSignature> m_signatures;

    explicit AbstractTransaction(AccountConverter accountConverter, Network* network);
public:
    static const int MIN_BASE_FEE = 100;
    /**
      * Adds a new signature ed25519PublicKey to this transaction.
      * @param signer {@link KeyPair} object representing a signer
      */
    void sign(KeyPair* signer);

    /**
      * Adds a new sha256Hash signature to this transaction by revealing preimage.
      * @param preimage the sha256 hash of preimage should be equal to signer hash
      */
    void sign(QByteArray preimage);

    /**
      * Appends a signature made elsewhere, as it is: a CAP-40 signed-payload
      * signature from KeyPair::signPayloadDecorated, or one from a wallet or
      * an HSM. Nothing is checked, as in js addDecoratedSignature and py
      * envelope.signatures.append.
      */
    void addSignature(const stellar::DecoratedSignature& signature);

    /**
      * Appends the ed25519 signature of this transaction's hash by
      * publicKey (a G... address), given in base64, as js addSignature does.
      * Throws unless it verifies against the hash, so a signature for another
      * transaction or network is not attached.
      */
    void addSignature(const QString& publicKey, const QString& signatureBase64);

    /**
      * Returns transaction hash.
      * Throws, like signatureBase(), when the transaction holds a value XDR
      * has no encoding for.
      */
    QByteArray hash() const;


    /**
      * Returns transaction hash encoded as a hexadecimal string.
      */
    QString hashHex() const;


    /**
      * Returns signature base.
      * Throws when the transaction holds a value XDR has no encoding for (an
      * arm the writer does not know): the bytes would be short, and a hash or
      * a signature over them would stand for a different transaction.
      */
    virtual QByteArray signatureBase() const= 0;


    Network* getNetwork() const;

    QVector<stellar::DecoratedSignature> getSignatures() const;

    /**
     * Generates TransactionEnvelope XDR object.
     * Throws when the envelope could not be encoded in full (see signatureBase()).
     */
    virtual stellar::TransactionEnvelope toEnvelopeXdr() = 0;

    /**
     * Returns base64-encoded TransactionEnvelope XDR object. Transaction need to have at least one signature.
     * Throws when the envelope could not be encoded in full (see signatureBase()).
     */
    QString toEnvelopeXdrBase64();

    /**
     * Returns new AbstractTransaction object from Transaction XDR object.
     * @param xdr XDR object
     */
    static AbstractTransaction* fromEnvelopeXdr(stellar::TransactionEnvelope& xdr, Network * network = Network::current());

    /**
     * Creates a <code>Transaction</code> instance from previously build <code>TransactionEnvelope</code>
     * @param envelope Base-64 encoded <code>TransactionEnvelope</code> (whitespace is ignored)
     * @return a new object the caller owns
     *
     * Throws unless the text is strict base64 of exactly one envelope, and
     * the SDK writes that envelope back byte for byte: a truncated envelope,
     * trailing bytes, or anything the SDK cannot represent exactly is refused
     * rather than read as a different transaction.
     */
    static AbstractTransaction* fromEnvelopeXdr(QString envelope, Network* network= Network::current());


signals:

};

QSTELLAR_END_NS

QSTELLAR_ALIAS(AbstractTransaction)

#endif // ABSTRACTTRANSACTION_H
