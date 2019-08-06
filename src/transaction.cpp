#include "transaction.h"
#include "account.h"
#include <QDateTime>

quint32 Transaction::Builder::s_defaultOperationFee = Transaction::Builder::BASE_FEE;

Transaction::Transaction(KeyPair *sourceAccount, quint32 fee, qint64 sequenceNumber, QVector<Operation *> operations, Memo *memo, TimeBounds *timeBounds) {
    m_sourceAccount = checkNotNull(sourceAccount, "sourceAccount cannot be null");
    m_sequenceNumber=sequenceNumber;//we cant check this, all the values are valid


    checkArgument(operations.length() > 0, "At least one operation required");
    m_operations = operations;
    m_fee = fee;
    m_memo =(memo) ? memo : Memo::none();
    m_timeBounds = timeBounds;
}

Transaction::~Transaction(){
    if(m_sourceAccount)
        delete m_sourceAccount;
    if(m_memo)
        delete m_memo;
    if(m_timeBounds)
        delete m_timeBounds;
    for(Operation * o : m_operations){
        delete o;
    }
}

void Transaction::sign(KeyPair *signer) {
    checkNotNull(signer, "signer cannot be null");
    QByteArray txHash = this->hash();

    m_signatures.append(signer->signDecorated(txHash));
}

void Transaction::sign(QByteArray preimage) {
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

QByteArray Transaction::hash() {
    return Util::hash(this->signatureBase());
}

QByteArray Transaction::signatureBase() {
    if (Network::current() == nullptr) {
        throw new NoNetworkSelectedException();
    }

    try {

        QByteArray output;
        QDataStream outputStream(&output,QIODevice::WriteOnly);
        // Hashed NetworkID
        outputStream.writeRawData(Network::current()->getNetworkId(),Network::current()->getNetworkId().length());
        // Envelope Type - 4 bytes
        outputStream << stellar::EnvelopeType::ENVELOPE_TYPE_TX;
        // Transaction XDR bytes
        outputStream << this->toXdr();
        return output;
    } catch (std::exception e) {
        return QByteArray();
    }
}

KeyPair *Transaction::getSourceAccount() {
    return m_sourceAccount;
}

qint64 Transaction::getSequenceNumber() {
    return m_sequenceNumber;
}

QVector<stellar::DecoratedSignature> Transaction::getSignatures() {
    return m_signatures;
}

Memo *Transaction::getMemo() const
{
    return m_memo;
}

TimeBounds *Transaction::getTimeBounds() const
{
    return m_timeBounds;
}

QVector<Operation *> Transaction::getOperations() const{
    return m_operations;
}

quint32 Transaction::getFee() {
    return m_fee;
}

stellar::Transaction Transaction::toXdr() {

    stellar::Transaction transaction;

    transaction.memo = m_memo->toXdr();
    if(m_timeBounds)
    {
        stellar::TimeBounds& tm = transaction.timeBounds.filler();
        tm.minTime=static_cast<quint64>(m_timeBounds->getMinTime());
        tm.maxTime=static_cast<quint64>(m_timeBounds->getMaxTime());
    }
    // fee
    transaction.fee = m_fee;
    // sequenceNumber
    transaction.seqNum = m_sequenceNumber;
    // sourceAccount
    transaction.sourceAccount = m_sourceAccount->getXdrPublicKey();
    // operations
    for (int i = 0; i < m_operations.length(); i++) {
        transaction.operations.append(m_operations.at(i)->toXdr());
    }
    return transaction;
}

Transaction *Transaction::fromXdr(stellar::Transaction &xdr)
{
    KeyPair *sourceAccount = KeyPair::fromXdrPublicKey(xdr.sourceAccount);
    QVector<Operation*> ops;
    for(auto op : xdr.operations.value)
    {
        ops.append(Operation::fromXdr(op));
    }
    return new Transaction(sourceAccount,xdr.fee,xdr.seqNum,ops,Memo::fromXdr(xdr.memo), xdr.timeBounds.filled ? TimeBounds::fromXdr(xdr.timeBounds.value) : nullptr);
}

stellar::TransactionEnvelope Transaction::toEnvelopeXdr() {

    stellar::TransactionEnvelope xdr;
    xdr.tx=this->toXdr();
    for(stellar::DecoratedSignature& signature : this->m_signatures){
        xdr.signatures.append(signature);
    }
    return xdr;
}

Transaction *Transaction::fromXdrEnvelope(stellar::TransactionEnvelope &xdr)
{
    return Transaction::fromEnvelopeXdr(xdr);
}

Transaction *Transaction::fromEnvelopeXdr(stellar::TransactionEnvelope &xdr)
{
    Transaction * transaction = Transaction::fromXdr(xdr.tx);
    for(stellar::DecoratedSignature& signature : xdr.signatures.value){
        transaction->m_signatures.append(signature);
    }
    return transaction;
}

QString Transaction::toEnvelopeXdrBase64() {

    stellar::TransactionEnvelope envelope =this->toEnvelopeXdr();
    QByteArray outputStream;
    QDataStream xdrOutputStream(&outputStream,QIODevice::WriteOnly);
    xdrOutputStream<< envelope;
    return outputStream.toBase64(XDR_BASE64ENCODING);
}

Transaction::Builder::Builder(TransactionBuilderAccount *sourceAccount) {
    m_sourceAccount = checkNotNull(sourceAccount, "sourceAccount cannot be null");
    m_memo=nullptr;
    m_timeBounds=nullptr;
    m_operationFee = s_defaultOperationFee;
    m_timeoutSet=false;
}

Transaction::Builder::~Builder()
{
    //it will not destroy the account object
    if(m_memo)
        delete m_memo;
    if(m_timeBounds)
        delete m_timeBounds;
    for(Operation * o : m_operations){
        delete o;
    }
}

int Transaction::Builder::getOperationsCount() {
    return m_operations.size();
}

void Transaction::Builder::setDefaultOperationFee(quint32 opFee) {
    if (opFee < Builder::BASE_FEE) {
        throw std::runtime_error(QString("DefaultOperationFee cannot be smaller than the BASE_FEE (\" %1 \"): %2").arg(Builder::BASE_FEE).arg(opFee).toStdString());
    }
    s_defaultOperationFee = opFee;
}

Transaction::Builder &Transaction::Builder::addOperation(Operation *operation) {
    checkNotNull(operation, "operation cannot be null");
    m_operations.append(operation);
    return *this;
}

Transaction::Builder &Transaction::Builder::addMemo(Memo *memo) {
    if (this->m_memo) {
        throw std::runtime_error("Memo has been already added.");
    }
    m_memo = checkNotNull(memo, "memo cannot be null");
    return *this;
}

Transaction::Builder &Transaction::Builder::addTimeBounds(TimeBounds *timeBounds) {
    if (this->m_timeBounds) {
        throw std::runtime_error("TimeBounds has been already added.");
    }
    checkNotNull(reinterpret_cast<intptr_t>(timeBounds), "timeBounds cannot be null");
    m_timeBounds = timeBounds;
    m_timeoutSet = true;
    return *this;
}

Transaction::Builder &Transaction::Builder::setTimeout(qint64 timeout) {
    if (m_timeBounds && m_timeBounds->getMaxTime() > 0) {
        throw std::runtime_error("TimeBounds.max_time has been already set - setting timeout would overwrite it.");
    }

    if (timeout < 0) {
        throw std::runtime_error("timeout cannot be negative");
    }

    m_timeoutSet = true;
    if (timeout > 0) {
        qint64 timeoutTimestamp = QDateTime::currentMSecsSinceEpoch()/ 1000L + timeout;
        if (!m_timeBounds) {
            m_timeBounds = new TimeBounds(0, timeoutTimestamp);
        } else {
            qint64 min = m_timeBounds->getMinTime();
            delete m_timeBounds;
            m_timeBounds = new TimeBounds(min, timeoutTimestamp);
        }
    }

    return *this;
}

Transaction::Builder &Transaction::Builder::setOperationFee(quint32 operationFee) {
    if (operationFee < Builder::BASE_FEE) {
        throw std::runtime_error(QString("OperationFee cannot be smaller than the BASE_FEE (\" %1 \"): %2").arg(Builder::BASE_FEE).arg(operationFee).toStdString());
    }

    m_operationFee = operationFee;
    return *this;
}

Transaction *Transaction::Builder::build() {
    // Ensure setTimeout called or maxTime is set
    if ((!m_timeBounds || (m_timeBounds  && m_timeBounds->getMaxTime() == 0)) && !m_timeoutSet) {
      throw std::runtime_error("TimeBounds has to be set or you must call setTimeout(TIMEOUT_INFINITE).");
    }

    if (m_operationFee == 0) {
        qDebug()<< "[TransactionBuilder] The `operationFee` parameter of `TransactionBuilder` is required. Setting to BASE_FEE=" << Builder::BASE_FEE << ". Future versions of this library will error if not provided.";
        m_operationFee = Builder::BASE_FEE;
    }

    //we have to make a copy of the KeyPair in order to be able to destroy it without affecting Account object.
    //when we create a transaction from XDR, a KeyPair object is also created.
    Transaction *transaction = new Transaction(new KeyPair(*(m_sourceAccount->getKeypair())), static_cast<quint32>(m_operations.length()) * m_operationFee, m_sourceAccount->getIncrementedSequenceNumber(), m_operations, m_memo, m_timeBounds);
    // Increment sequence number when there were no exceptions when creating a transaction
    m_sourceAccount->incrementSequenceNumber();

    //so objects dont get destroyed with the builder, all the checks were done already
    m_memo=nullptr;
    m_timeBounds=nullptr;
    m_operations.clear();

    return transaction;
}

Transaction *checkNotNull(Transaction *transaction, const char *error)
{
    if(!transaction){
        throw std::runtime_error(error);
    }
    return transaction;
}
