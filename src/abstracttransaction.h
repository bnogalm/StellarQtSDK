#ifndef ABSTRACTTRANSACTION_H
#define ABSTRACTTRANSACTION_H

#include <QObject>
#include "xdr/stellartransaction.h"
#include "keypair.h"
#include "operation.h"
#include "memo.h"
#include "network.h"
#include "accountconverter.h"

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
      * Returns transaction hash.
      */
    QByteArray hash() const;


    /**
      * Returns transaction hash encoded as a hexadecimal string.
      */
    QString hashHex() const;


    /**
      * Returns signature base.
      */
    virtual QByteArray signatureBase() const= 0;


    Network* getNetwork() const;

    QVector<stellar::DecoratedSignature> getSignatures() const;

    /**
     * Generates TransactionEnvelope XDR object.
     */
    virtual stellar::TransactionEnvelope toEnvelopeXdr() = 0;

    /**
     * Returns base64-encoded TransactionEnvelope XDR object. Transaction need to have at least one signature.
     */
    QString toEnvelopeXdrBase64();

    /**
     * Returns new AbstractTransaction object from Transaction XDR object.
     * @param xdr XDR object
     */
    static AbstractTransaction* fromEnvelopeXdr(stellar::TransactionEnvelope& xdr, Network * network = Network::current());

    /**
     * Creates a <code>Transaction</code> instance from previously build <code>TransactionEnvelope</code>
     * @param envelope Base-64 encoded <code>TransactionEnvelope</code>
     * @return
     * @throws IOException
     */
    static AbstractTransaction* fromEnvelopeXdr(QString envelope, Network* network= Network::current());


signals:

};

#endif // ABSTRACTTRANSACTION_H
