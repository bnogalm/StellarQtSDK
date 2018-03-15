#include "transaction.h"
#include "account.h"

Transaction::Transaction(KeyPair *sourceAccount, quint64 sequenceNumber, QVector<Operation *> operations, Memo *memo) {
    m_sourceAccount = checkNotNull(sourceAccount, "sourceAccount cannot be null");
    m_sequenceNumber=sequenceNumber;//we cant check this, all the values are valid


    checkArgument(operations.length() > 0, "At least one operation required");
    m_operations = operations;
    m_fee = operations.length() * BASE_FEE;
    m_memo =(memo) ? memo : Memo::none();
}

Transaction::~Transaction(){
    if(m_sourceAccount)
        delete m_sourceAccount;
    if(m_memo)
        delete m_memo;
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

    signature.set((uchar*)preimage.data(),preimage.length());


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
    if (Network::current() == 0) {
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

quint64 Transaction::getSequenceNumber() {
    return m_sequenceNumber;
}

QVector<stellar::DecoratedSignature> Transaction::getSignatures() {
    return m_signatures;
}

Memo *Transaction::getMemo() {
    return m_memo;
}

int Transaction::getFee() {
    return m_fee;
}

stellar::Transaction Transaction::toXdr() {

    stellar::Transaction transaction;

    transaction.memo = m_memo->toXdr();
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

    Account* account = new Account(sourceAccount,xdr.seqNum);
    Transaction::Builder *builder = new Transaction::Builder(account);
    builder->addMemo(Memo::fromXdr(xdr.memo));
    for(auto op : xdr.operations.value)
    {
        Operation* opObject = Operation::fromXdr(op);
        builder->addOperation(opObject);
    }
    return builder->build();
}

stellar::TransactionEnvelope Transaction::toEnvelopeXdr() {
    if (m_signatures.size() == 0) {
        throw std::runtime_error("Transaction must be signed by at least one signer. Use transaction.sign().");
    }

    stellar::TransactionEnvelope xdr;
    xdr.tx=this->toXdr();
    for(stellar::DecoratedSignature& signature : this->m_signatures){
        xdr.signatures.append(signature);
    }
    return xdr;
}

Transaction *Transaction::fromXdrEnvelope(stellar::TransactionEnvelope &xdr)
{
    xdr.tx.seqNum--;//this is a signed envelope
    Transaction * transaction = Transaction::fromXdr(xdr.tx);
    xdr.tx.seqNum++;
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
    m_memo=0;
}

Transaction::Builder::~Builder()
{
    if(m_sourceAccount)
        delete m_sourceAccount;
    if(m_memo)
        delete m_memo;
    for(Operation * o : m_operations){
        delete o;
    }
}

int Transaction::Builder::getOperationsCount() {
    return m_operations.size();
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

Transaction *Transaction::Builder::build() {
    //so objects dont get destroyed on exception, we use a pointer copy
    auto account = m_sourceAccount;
    auto memo = m_memo;
    m_sourceAccount=0;
    m_memo=0;
    Transaction *transaction = new Transaction(account->getKeypair(), account->getIncrementedSequenceNumber(), m_operations, memo);
    // Increment sequence number when there were no exceptions when creating a transaction
    account->incrementSequenceNumber();
    //it lose ownership of the assigned data, we reset it.
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
