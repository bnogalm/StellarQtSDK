#include "setoptionsoperation.h"
#include "signer.h"
#include <QUrl>

SetOptionsOperation::SetOptionsOperation(KeyPair *inflationDestination, Integer clearFlags
                                         , Integer setFlags, Integer masterKeyWeight, Integer lowThreshold
                                         , Integer mediumThreshold, Integer highThreshold, QString homeDomain
                                         , stellar::SignerKey signer, quint32 signerWeight)
    :m_inflationDestination(nullptr)
{
    QByteArray homeDomainACE = QUrl::toAce(homeDomain);
    if (homeDomainACE.length() > 32) {
        throw std::runtime_error("Home domain must be <= 32 characters");
    }
    else{
        stellar::string32& hd = m_op.homeDomain.filler();
        hd.set(homeDomainACE.data(),homeDomainACE.size());
    }
    if(inflationDestination)
        m_op.inflationDest =  inflationDestination->getXdrPublicKey();

    m_op.clearFlags = clearFlags;
    m_op.setFlags = setFlags;
    m_op.masterWeight = masterKeyWeight;
    m_op.lowThreshold = lowThreshold;
    m_op.medThreshold = mediumThreshold;
    m_op.highThreshold = highThreshold;
    stellar::Signer& s = m_op.signer.filler();
    s.key=signer;
    s.weight=signerWeight& 0xFF;
}

SetOptionsOperation::SetOptionsOperation(stellar::SetOptionsOp &op):m_inflationDestination(nullptr),m_op(op)
{

}

SetOptionsOperation::SetOptionsOperation():m_inflationDestination(nullptr)
{

}

SetOptionsOperation::~SetOptionsOperation()
{
    if(m_inflationDestination)
        delete m_inflationDestination;
}

KeyPair* SetOptionsOperation::getInflationDestination() {
    if(!m_inflationDestination && m_op.inflationDest.filled){
        m_inflationDestination = KeyPair::fromXdrPublicKey(m_op.inflationDest.value);
    }
    return m_inflationDestination;
}

Integer SetOptionsOperation::getClearFlags() {
    return Integer(m_op.clearFlags);
}

Integer SetOptionsOperation::getSetFlags() {
    return Integer(m_op.setFlags);
}

Integer SetOptionsOperation::getMasterKeyWeight() {
    return Integer(m_op.masterWeight);
}

Integer SetOptionsOperation::getLowThreshold() {
    return Integer(m_op.lowThreshold);
}

Integer SetOptionsOperation::getMediumThreshold() {
    return Integer(m_op.medThreshold);
}

Integer SetOptionsOperation::getHighThreshold() {
    return Integer(m_op.highThreshold);
}

QString SetOptionsOperation::getHomeDomain() {
    if(m_op.homeDomain.filled)
        return QUrl::fromAce(QByteArray::fromRawData(m_op.homeDomain.value.value.data(),m_op.homeDomain.value.value.size()));
    return QString();
}

stellar::SignerKey *SetOptionsOperation::getSigner() {
    if(m_op.signer.filled)
        return &m_op.signer.value.key;
    return nullptr;
}

Integer SetOptionsOperation::getSignerWeight() {
    if(m_op.signer.filled)
        return Integer(m_op.signer.value.weight);
    return Integer();
}

void SetOptionsOperation::fillOperationBody(stellar::Operation &op)
{
    auto& o = op.fillSetOptionsOp();
    o = m_op;
}

SetOptionsOperation *SetOptionsOperation::build(stellar::SetOptionsOp &op)
{
    SetOptionsOperation *optionsOperation = new SetOptionsOperation(op);
    return optionsOperation;
}

SetOptionsOperation *SetOptionsOperation::create()
{
    return new SetOptionsOperation();
}

SetOptionsOperation *SetOptionsOperation::setInflationDestination(KeyPair *inflationDestination) {
    if(inflationDestination)
        m_op.inflationDest = inflationDestination->getXdrPublicKey();
    else
        m_op.inflationDest.clear();
    return this;
}

SetOptionsOperation *SetOptionsOperation::setClearFlags(quint32 clearFlags) {
    m_op.clearFlags = clearFlags;
    return this;
}

SetOptionsOperation *SetOptionsOperation::setSetFlags(quint32 setFlags) {
    m_op.setFlags = setFlags;
    return this;
}

SetOptionsOperation *SetOptionsOperation::setMasterKeyWeight(quint32 masterKeyWeight) {
    m_op.masterWeight = masterKeyWeight;
    return this;
}

SetOptionsOperation *SetOptionsOperation::setLowThreshold(quint32 lowThreshold) {
    m_op.lowThreshold = lowThreshold;
    return this;
}

SetOptionsOperation *SetOptionsOperation::setMediumThreshold(quint32 mediumThreshold) {
    m_op.medThreshold = mediumThreshold;
    return this;
}

SetOptionsOperation *SetOptionsOperation::setHighThreshold(quint32 highThreshold) {
    m_op.highThreshold= highThreshold;
    return this;
}

SetOptionsOperation *SetOptionsOperation::setHomeDomain(QString homeDomain) {
    if (homeDomain.toUtf8().length() > 32) {
        throw std::runtime_error("Home domain must be <= 32 characters");
    }
    stellar::string32& hd = m_op.homeDomain.filler();
    QByteArray utf8 = homeDomain.toUtf8();
    hd.set(utf8.data(),utf8.size());
    return this;
}

SetOptionsOperation *SetOptionsOperation::setSigner(stellar::SignerKey signer, quint32 weight) {
    stellar::Signer& s = m_op.signer.filler();
    s.key = signer;
    s.weight = weight & 0xFF;
    return this;
}

SetOptionsOperation *SetOptionsOperation::setSourceAccount(QString sourceAccount) {
    Operation::setSourceAccount(sourceAccount);
    return this;
}

