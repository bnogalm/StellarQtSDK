#include "transaction.h"
#include <QDateTime>
#include <limits>
#include <cstring>
#include <stdexcept>
#include <QScopedPointer>
#include "createclaimablebalanceoperation.h"
#include "accountconverter.h"

// TransactionBuilder lives in transactionbuilder.cpp.
// Transaction::Builder is now a deprecated alias (see header).

QSTELLAR_BEGIN_NS

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
    // No catch-all here. It used to turn any failure to build the envelope
    // into an empty payload, and sign() then signed sha256("") and returned
    // normally: a signature over nothing, reported as success. Nor a failed
    // write: a payload cut short where the writer met an arm it does not know
    // was hashed and signed as if it were the transaction.
    // With the converter the envelope is written with: a transaction built
    // with muxing off sends its M... accounts as G..., and signing the M...
    // form gave a signature over a transaction nobody submits.
    stellar::TransactionSignaturePayload payload(this->toV1Xdr(m_accountConverter),
                                                 m_network->getNetworkId());
    return xdr::encodeOrThrow(payload, "the transaction signature payload");
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


    // Hashed, so checked: short bytes would name a balance that is not this one.
    auto v0 = Util::hash(xdr::encodeOrThrow(id, "the claimable balance operation id"));
    memcpy(result.v0, v0,sizeof(result.v0));

    return Util::bytesToHex(xdr::encodeOrThrow(result, "the claimable balance id")).toLower();
}

stellar::TransactionV0 Transaction::toV0Xdr(AccountConverter accountConverter) const{

    stellar::TransactionV0 transaction;

    transaction.memo = m_memo->toXdr();
    // TransactionV0 only carries TimeBounds (no V2 preconditions). If the
    // caller set V2-only fields, they're silently dropped on the V0 path.
    TimeBounds* tb = m_preconditions.getTimeBounds();
    if (tb) {
        stellar::TimeBounds& tm = transaction.timeBounds.filler();
        tm.minTime=tb->getMinTimePoint();
        tm.maxTime=tb->getMaxTimePoint();
    }
    // fee — a uint32 on the wire. Refuse rather than truncate: a truncated fee
    // would be signed while getFee() reports another.
    if (m_fee < 0 || m_fee > static_cast<qint64>(std::numeric_limits<quint32>::max())) {
        throw std::runtime_error("transaction fee does not fit the envelope's uint32 fee field");
    }
    transaction.fee = static_cast<quint32>(m_fee);
    // sequenceNumber
    transaction.seqNum = m_sequenceNumber;
    // sourceAccount
    stellar::AccountID accountID = StrKey::encodeToXDRAccountId(m_sourceAccount);
    memcpy(transaction.sourceAccountEd25519,accountID.ed25519,sizeof(transaction.sourceAccountEd25519));
    // operations
    // stellar::Array silently drops anything past its maximum, so a tx with
    // more than MAX_OPS_PER_TX operations got signed truncated.
    if (m_operations.size() > transaction.operations.maxSize()) {
        throw std::runtime_error("transaction exceeds the maximum number of operations");
    }
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
    // fee — a uint32 on the wire. Refuse rather than truncate: a truncated fee
    // would be signed while getFee() reports another.
    if (m_fee < 0 || m_fee > static_cast<qint64>(std::numeric_limits<quint32>::max())) {
        throw std::runtime_error("transaction fee does not fit the envelope's uint32 fee field");
    }
    transaction.fee = static_cast<quint32>(m_fee);
    // sequenceNumber
    transaction.seqNum = m_sequenceNumber;
    // sourceAccount: through the converter, as the operations' accounts are
    // (and as java writes it). With muxing off an M... source stayed M...
    // while everything else in the envelope went out as G...
    transaction.sourceAccount = accountConverter.encode(m_sourceAccount);
    // operations — the same guard as toV0Xdr: stellar::Array drops anything
    // past its maximum without a word, and this is the envelope that is
    // actually signed and sent.
    if (m_operations.size() > transaction.operations.maxSize()) {
        throw std::runtime_error("transaction exceeds the maximum number of operations");
    }
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

// Builds the Transaction from what an envelope decoded to. Anything in it can
// refuse malformed input (an operation it cannot read, no operations at all);
// the parts already decoded are freed then, not leaked.
Transaction* Transaction::fromParts(AccountConverter accountConverter, const QString& sourceAccount,
                                    qint64 fee, qint64 sequenceNumber,
                                    const QVector<stellar::Operation>& xdrOperations,
                                    stellar::Memo& xdrMemo, TransactionPreconditions preconditions,
                                    Network* network)
{
    QScopedPointer<Memo> memo(Memo::fromXdr(xdrMemo));
    QVector<Operation*> ops;
    try {
        for (auto op : xdrOperations)
            ops.append(Operation::fromXdr(accountConverter, op));
        Transaction* t = new Transaction(accountConverter, sourceAccount, fee, sequenceNumber,
                                         ops, memo.data(), std::move(preconditions), network);
        memo.take();
        return t;
    } catch (...) {
        qDeleteAll(ops);
        throw;
    }
}

Transaction *Transaction::fromV0EnvelopeXdr(AccountConverter accountConverter, stellar::TransactionV0Envelope &envelope, Network *network)
{    
    QString sourceAccount = StrKey::encodeStellarAccountId(envelope.tx.sourceAccountEd25519);
    TransactionPreconditions preconditions;
    if (envelope.tx.timeBounds.filled)
        preconditions.setTimeBounds(TimeBounds::fromXdr(envelope.tx.timeBounds.value));
    Transaction *t = fromParts(accountConverter, sourceAccount, envelope.tx.fee, envelope.tx.seqNum,
                               envelope.tx.operations.value, envelope.tx.memo,
                               std::move(preconditions), network);
    t->m_envelopeType = stellar::EnvelopeType::ENVELOPE_TYPE_TX_V0;
    for (stellar::DecoratedSignature& signature : envelope.signatures.value) {
        t->m_signatures.append(signature);
    }
    return t;

}

Transaction *Transaction::fromV1EnvelopeXdr(AccountConverter accountConverter, stellar::TransactionV1Envelope &envelope, Network *network)
{    
    QString sourceAccount = accountConverter.decode(envelope.tx.sourceAccount);
    Transaction * t = fromParts(accountConverter, sourceAccount, envelope.tx.fee, envelope.tx.seqNum,
                                envelope.tx.operations.value, envelope.tx.memo,
                                TransactionPreconditions::fromXdr(envelope.tx.cond), network);
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
        // Same as with the operations: past the maximum, Array drops them
        // silently and the tx ships with fewer signatures than were added.
        if (this->m_signatures.size() > envelope.signatures.maxSize()) {
            throw std::runtime_error("transaction exceeds the maximum number of signatures");
        }
        for(stellar::DecoratedSignature& signature : this->m_signatures){
            envelope.signatures.append(signature);
        }
        stellar::TransactionEnvelope result(envelope);
        // Whatever the caller does with it next, it can be encoded in full:
        // see AbstractTransaction::signatureBase.
        xdr::encodeOrThrow(result, "the transaction envelope");
        return result;
    }
    case stellar::EnvelopeType::ENVELOPE_TYPE_TX_V0:
    {
        stellar::TransactionV0Envelope envelope;
        envelope.tx=toV0Xdr();
        // Same as with the operations: past the maximum, Array drops them
        // silently and the tx ships with fewer signatures than were added.
        if (this->m_signatures.size() > envelope.signatures.maxSize()) {
            throw std::runtime_error("transaction exceeds the maximum number of signatures");
        }
        for(stellar::DecoratedSignature& signature : this->m_signatures){
            envelope.signatures.append(signature);
        }
        stellar::TransactionEnvelope result(envelope);
        xdr::encodeOrThrow(result, "the transaction envelope");    // as above
        return result;
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

QSTELLAR_END_NS
