#include "setoptionsoperation.h"
#include "signer.h"
#include <stdexcept>

QSTELLAR_BEGIN_NS

namespace {

/**
 * Weights and thresholds are uint32 in the XDR but a byte to the network:
 * above 255 the operation was signed as given and the network refused it
 * (SET_OPTIONS_THRESHOLD_OUT_OF_RANGE, or SET_OPTIONS_BAD_SIGNER for a signer
 * weight). js-stellar-base refuses them before signing; so does this.
 */
void checkWeight(quint32 value, const char* what)
{
    if (value > 255) {
        throw std::runtime_error(QStringLiteral("%1 must be in the range 0-255")
                                     .arg(QLatin1String(what)).toStdString());
    }
}

void checkWeight(const Integer& value, const char* what)
{
    if (value.filled)
        checkWeight(value.value, what);
}

} // namespace


SetOptionsOperation::SetOptionsOperation(KeyPair *inflationDestination, Integer clearFlags
                                         , Integer setFlags, Integer masterKeyWeight, Integer lowThreshold
                                         , Integer mediumThreshold, Integer highThreshold, QString homeDomain
                                         , stellar::SignerKey signer, quint32 signerWeight)
    :m_inflationDestination(nullptr)
{
    checkWeight(masterKeyWeight, "master weight");
    checkWeight(lowThreshold, "low threshold");
    checkWeight(mediumThreshold, "medium threshold");
    checkWeight(highThreshold, "high threshold");
    // Same as in setSigner: masking turned 256 into 0, and weight 0 DELETES
    // the signer instead of adding it.
    checkWeight(signerWeight, "signer weight");
    // The home domain is signed as given, as setHomeDomain() and js/py/java
    // do. It went through QUrl::toAce: lower-cased and punycoded, and EMPTY
    // for what Qt takes for no host name ("stellar.org/", "a:b") — an empty
    // home domain clears the account's. A null QString leaves the home domain
    // as it is; "" clears it.
    if (!homeDomain.isNull())
        setHomeDomain(homeDomain);
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
    s.weight=signerWeight;
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
    if(m_op.homeDomain.filled) {
        // The signed text, as js and py return it. It went through
        // QUrl::fromAce: "Example.COM" (valid on the network) came back as
        // "example.com", UTF-8 garbled, and an xn-- name as its Unicode form.
        // Set means set, even when empty (a clear). A leading BOM is kept:
        // QString::fromUtf8 dropped it.
        const QByteArray raw(m_op.homeDomain.value.value.data(), m_op.homeDomain.value.value.size());
        return raw.isEmpty() ? QString(QLatin1String("")) : Util::fromUtf8KeepingBom(raw);
    }
    return QString();
}

QByteArray SetOptionsOperation::getHomeDomainBytes() const {
    if (!m_op.homeDomain.filled)
        return QByteArray();
    const QByteArray raw(m_op.homeDomain.value.value.data(), m_op.homeDomain.value.value.size());
    return raw.isNull() ? QByteArray("") : raw;
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

void SetOptionsOperation::fillOperationBody(AccountConverter &accountConverter, stellar::Operation &op)
{
    Q_UNUSED(accountConverter)
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
    checkWeight(masterKeyWeight, "master weight");
    m_op.masterWeight = masterKeyWeight;
    return this;
}

SetOptionsOperation *SetOptionsOperation::setLowThreshold(quint32 lowThreshold) {
    checkWeight(lowThreshold, "low threshold");
    m_op.lowThreshold = lowThreshold;
    return this;
}

SetOptionsOperation *SetOptionsOperation::setMediumThreshold(quint32 mediumThreshold) {
    checkWeight(mediumThreshold, "medium threshold");
    m_op.medThreshold = mediumThreshold;
    return this;
}

SetOptionsOperation *SetOptionsOperation::setHighThreshold(quint32 highThreshold) {
    checkWeight(highThreshold, "high threshold");
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
    // Was: weight & 0xFF. A weight of 256 became 0, and weight 0 means DELETE
    // the signer: an out-of-range mistake could lock the user out of their own
    // account. The 0-255 range was already documented in the header.
    // Checked before the signer is filled in: refused afterwards, the key was
    // already set with the weight it had (0, a delete), for a caller that
    // caught the exception and went on to sign.
    checkWeight(weight, "signer weight");
    stellar::Signer& s = m_op.signer.filler();
    s.key = signer;
    s.weight = weight;
    return this;
}

SetOptionsOperation *SetOptionsOperation::setSourceAccount(QString sourceAccount) {
    Operation::setSourceAccount(sourceAccount);
    return this;
}

QSTELLAR_END_NS
