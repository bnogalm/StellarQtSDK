#include "transaction.h"
#include <QDateTime>
#include <limits>
#include "createclaimablebalanceoperation.h"
#include "accountconverter.h"

// TransactionBuilder lives in transactionbuilder.cpp.
// Transaction::Builder is now a deprecated alias (see header).

Transaction::Transaction(AccountConverter accountConverter, QString sourceAccount, qint64 fee, qint64 sequenceNumber, QVector<Operation *> operations, Memo *memo, TimeBounds *timeBounds, Network *network)
    :AbstractTransaction(accountConverter, network),m_envelopeType(stellar::EnvelopeType::ENVELOPE_TYPE_TX)
{
    m_sourceAccount = checkNotNull(sourceAccount, "sourceAccount cannot be null");
    m_sequenceNumber=sequenceNumber;//we cant check this, all the values are valid


    checkArgument(operations.length() > 0, "At least one operation required");
    m_operations = operations;
    m_fee = fee;
    m_memo =(memo) ? memo : Memo::none();
    // Wrap legacy TimeBounds* into the preconditions bundle (takes ownership).
    m_preconditions.setTimeBounds(timeBounds);
}

Transaction::Transaction(AccountConverter accountConverter, QString sourceAccount, qint64 fee, qint64 sequenceNumber, QVector<Operation *> operations, Memo *memo, TransactionPreconditions preconditions, Network *network)
    :AbstractTransaction(accountConverter, network),m_envelopeType(stellar::EnvelopeType::ENVELOPE_TYPE_TX)
{
    m_sourceAccount = checkNotNull(sourceAccount, "sourceAccount cannot be null");
    m_sequenceNumber=sequenceNumber;
    checkArgument(operations.length() > 0, "At least one operation required");
    m_operations = operations;
    m_fee = fee;
    m_memo = (memo) ? memo : Memo::none();
    // Move-assign so any owned pointers (TimeBounds etc.) keep their identity.
    m_preconditions = std::move(preconditions);
}

Transaction::~Transaction(){
    m_sourceAccount.fill('\0');
    if(m_memo)
        delete m_memo;
    for(Operation * o : m_operations){
        delete o;
    }
}



QByteArray Transaction::signatureBase() const{
    try {

        QByteArray output;
        QDataStream outputStream(&output,QIODevice::WriteOnly);
        stellar::TransactionSignaturePayload payload(this->toV1Xdr(), m_network->getNetworkId());
        outputStream << payload;
        return output;
    } catch (...) {
        return QByteArray();
    }
}

QString Transaction::getSourceAccount() const {
    return m_sourceAccount;
}

qint64 Transaction::getSequenceNumber() const{
    return m_sequenceNumber;
}

Memo *Transaction::getMemo() const
{
    return m_memo;
}

Network *Transaction::getNetwork() const
{
    return m_network;
}

TimeBounds *Transaction::getTimeBounds() const
{
    return m_preconditions.getTimeBounds();
}

QVector<Operation *> Transaction::getOperations() const{
    return m_operations;
}

qint64 Transaction::getFee() const{
    return m_fee;
}

QString Transaction::getClaimableBalanceId(int index) {
    if (index < 0 || index >= m_operations.length()) {
        throw std::runtime_error("index is outside the bounds of the operations within this transaction");
    }
    if (!(dynamic_cast<CreateClaimableBalanceOperation*>(m_operations[index]))) {
        throw std::runtime_error("operation at index is not of type CreateClaimableBalanceOperation");
    }

    // We mimic the relevant code from Stellar Core
    // https://github.com/stellar/stellar-core/blob/9f3cc04e6ec02c38974c42545a86cdc79809252b/src/test/TestAccount.cpp#L285
    //
    // Note that the source account must be *unmuxed* for this to work.

    stellar::OperationID id;
    id.type = stellar::EnvelopeType::ENVELOPE_TYPE_OP_ID;

    id.id.opNum = index;
    id.id.seqNum =getSequenceNumber();
    id.id.sourceAccount = AccountConverter(false).encode(m_sourceAccount);


    stellar::ClaimableBalanceID result;
    result.type = stellar::ClaimableBalanceIDType::CLAIMABLE_BALANCE_ID_TYPE_V0;


    QByteArray serializedId;
    QDataStream outputStream(&serializedId,QIODevice::WriteOnly);
    outputStream << id;
    auto v0 = Util::hash(serializedId);
    memcpy(result.v0, v0,sizeof(result.v0));

    QByteArray outputResult;
    QDataStream outputStreamResult(&outputResult,QIODevice::WriteOnly);
    outputStreamResult << result;

    return Util::bytesToHex(outputResult).toLower();
}

stellar::TransactionV0 Transaction::toV0Xdr(AccountConverter accountConverter) const{

    stellar::TransactionV0 transaction;

    transaction.memo = m_memo->toXdr();
    // TransactionV0 only carries TimeBounds (no V2 preconditions). If the
    // caller set V2-only fields, they're silently dropped on the V0 path.
    TimeBounds* tb = m_preconditions.getTimeBounds();
    if (tb) {
        stellar::TimeBounds& tm = transaction.timeBounds.filler();
        tm.minTime=static_cast<quint64>(tb->getMinTime());
        tm.maxTime=static_cast<quint64>(tb->getMaxTime());
    }
    // fee
    transaction.fee = m_fee;
    // sequenceNumber
    transaction.seqNum = m_sequenceNumber;
    // sourceAccount
    stellar::AccountID accountID = StrKey::encodeToXDRAccountId(m_sourceAccount);
    memcpy(transaction.sourceAccountEd25519,accountID.ed25519,sizeof(transaction.sourceAccountEd25519));
    // operations
    for (int i = 0; i < m_operations.length(); i++) {
        transaction.operations.append(m_operations.at(i)->toXdr(accountConverter));
    }
    return transaction;
}

stellar::Transaction Transaction::toV1Xdr(AccountConverter accountConverter) const{

    stellar::Transaction transaction;

    transaction.memo = m_memo->toXdr();
    // CAP-21 — emits PRECOND_NONE / TIME / V2 based on which fields are set.
    transaction.cond = m_preconditions.toXdr();
    // fee
    transaction.fee = m_fee;
    // sequenceNumber
    transaction.seqNum = m_sequenceNumber;
    // sourceAccount
    transaction.sourceAccount =StrKey::encodeToXDRMuxedAccount(m_sourceAccount);
    // operations
    for (int i = 0; i < m_operations.length(); i++) {
        transaction.operations.append(m_operations.at(i)->toXdr(accountConverter));
    }
    // CAP-46 — Soroban ext: v=1 + SorobanTransactionData when set.
    if (m_sorobanData) {
        transaction.ext.v = 1;
        transaction.ext.sorobanData = *m_sorobanData;
    }
    return transaction;
}

Transaction *Transaction::fromV0EnvelopeXdr(stellar::TransactionV0Envelope &envelope, Network *network)
{
    return fromV0EnvelopeXdr(AccountConverter().enableMuxed(), envelope, network);
}

Transaction *Transaction::fromV1EnvelopeXdr(stellar::TransactionV1Envelope &envelope, Network *network)
{
    return fromV1EnvelopeXdr(AccountConverter().enableMuxed(), envelope, network);
}

Transaction *Transaction::fromV0EnvelopeXdr(AccountConverter accountConverter, stellar::TransactionV0Envelope &envelope, Network *network)
{    
    QString sourceAccount = StrKey::encodeStellarAccountId(envelope.tx.sourceAccountEd25519);
    QVector<Operation*> ops;
    for(auto op : envelope.tx.operations.value)
    {
        ops.append(Operation::fromXdr(accountConverter, op));
    }
    Transaction *t = new Transaction(accountConverter, sourceAccount,envelope.tx.fee,envelope.tx.seqNum,ops,Memo::fromXdr(envelope.tx.memo), envelope.tx.timeBounds.filled ? TimeBounds::fromXdr(envelope.tx.timeBounds.value) : nullptr, network);
    t->m_envelopeType = stellar::EnvelopeType::ENVELOPE_TYPE_TX_V0;
    for (stellar::DecoratedSignature& signature : envelope.signatures.value) {
        t->m_signatures.append(signature);
    }
    return t;

}

Transaction *Transaction::fromV1EnvelopeXdr(AccountConverter accountConverter, stellar::TransactionV1Envelope &envelope, Network *network)
{    
    QString sourceAccount = accountConverter.decode(envelope.tx.sourceAccount);
    QVector<Operation*> ops;
    for(auto op : envelope.tx.operations.value)
    {
        ops.append(Operation::fromXdr(accountConverter, op));
    }
    Transaction * t = new Transaction(accountConverter, sourceAccount,envelope.tx.fee,envelope.tx.seqNum,ops,Memo::fromXdr(envelope.tx.memo), TransactionPreconditions::fromXdr(envelope.tx.cond), network);
    t->m_envelopeType = stellar::EnvelopeType::ENVELOPE_TYPE_TX;
    if (envelope.tx.ext.v == 1) {
        t->m_sorobanData = QSharedPointer<stellar::SorobanTransactionData>::create(envelope.tx.ext.sorobanData);
    }
    for (stellar::DecoratedSignature& signature : envelope.signatures.value) {
        t->m_signatures.append(signature);
    }
    return t;
}


stellar::TransactionEnvelope Transaction::toEnvelopeXdr(){    
    switch(m_envelopeType)
    {
    case stellar::EnvelopeType::ENVELOPE_TYPE_TX:
    {
        stellar::TransactionV1Envelope envelope;        
        envelope.tx=toV1Xdr(m_accountConverter);
        for(stellar::DecoratedSignature& signature : this->m_signatures){
            envelope.signatures.append(signature);
        }
        return stellar::TransactionEnvelope(envelope);
    }
    case stellar::EnvelopeType::ENVELOPE_TYPE_TX_V0:
    {
        stellar::TransactionV0Envelope envelope;
        envelope.tx=toV0Xdr();
        for(stellar::DecoratedSignature& signature : this->m_signatures){
            envelope.signatures.append(signature);
        }
        return stellar::TransactionEnvelope(envelope);
        break;
    }
    default:
    {
        throw std::runtime_error("invalid envelope type");
    }
    }
}

Transaction *checkNotNull(Transaction *transaction, const char *error)
{
    if(!transaction){
        throw std::runtime_error(error);
    }
    return transaction;
}
