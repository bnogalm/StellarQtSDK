#include "util.h"

#include "keypair.h"
#include <QRandomGenerator>
#include <QDateTime>
#include "account.h"
#include "transaction.h"
#include "managedataoperation.h"

Util::Util()
{

}

QByteArray Util::hash(QByteArray data){
    return QCryptographicHash::hash(data,QCryptographicHash::Sha256);
}

QByteArray Util::paddedByteArray(QByteArray bytes, int length) {
    return bytes.leftJustified(length,'\0');
}

QByteArray Util::paddedByteArray(QString string, int length) {
    return paddedByteArray(string.toUtf8(), length);
}

QString Util::paddedByteArrayToString(QByteArray bytes) {
    return Util::removeTailChars(bytes,'\0');
}

QByteArray Util::removeTailChars(QByteArray bytes, const char c)
{
    int i=bytes.length()-1;
    for(;i>=0;i--){
        if(bytes[i]!=c){
            break;
        }
    }
    return bytes.left(i+1);
}

QString Util::removeTailChars(QString string, QChar c)
{
    int i=string.length()-1;
    for(;i>=0;i--){
        if(string[i]!=c){
            break;
        }
    }
    return string.left(i+1);
}

Transaction* Util::buildChallengeTx(KeyPair *serverSignerSecret, QString clientAccountID, QString anchorName, qint64 timebound)
{
    QByteArray randomNonce = generateRandomNonce(48);//48 random bytes converted to base64 is 64 bytes
    randomNonce = randomNonce.toBase64(QByteArray::Base64Option::Base64UrlEncoding| QByteArray::OmitTrailingEquals);

    // represent server signing account
    Account *sa = new Account(new KeyPair(*serverSignerSecret),-1);//as is a temporal account, we use a keypair copy.
    TimeBounds *timeBounds;
    if(timebound>0)
    {
        qint64 now = QDateTime::currentMSecsSinceEpoch()/ 1000L;
        qint64 timeoutTimestamp = now + timebound;
        timeBounds = new TimeBounds(now,timeoutTimestamp);
    }
    else
        timeBounds= new TimeBounds(0,0);
    ManageDataOperation* dataOp = new ManageDataOperation(anchorName + " auth",randomNonce);
    dataOp->setSourceAccount(KeyPair::fromAccountId(clientAccountID));
    Transaction *tx = Transaction::Builder(sa).addOperation(dataOp).addTimeBounds(timeBounds).setOperationFee(Transaction::Builder::BASE_FEE).build();
    tx->sign(serverSignerSecret);
    return tx;
}

QByteArray Util::generateRandomNonce(int n)
{
    QByteArray array(n,Qt::Initialization::Uninitialized);
    QRandomGenerator *r = QRandomGenerator::global();
    r->fillRange(reinterpret_cast<quint32*>(array.data()),n/static_cast<int>(sizeof(quint32)));
    return array;
}

QString checkNotNull(QString p, const char *error)
{
    if(p.isNull()){
        throw std::runtime_error(error);
    }
    return p;
}

QByteArray checkNotNull(QByteArray p, const char* error)
{
    if(p.isNull()){
        throw std::runtime_error(error);
    }
    return p;
}

CheckHex::CheckHex(QString v)
{
    static QRegularExpression hexMatcher("^([0-9A-Fa-f][0-9A-Fa-f])*$",QRegularExpression::CaseInsensitiveOption);
    auto match = hexMatcher.globalMatch(v);
    if (!match.hasNext())
    {
        throw std::runtime_error("invalid hex string");
    }
    value = QByteArray::fromHex(v.toLatin1());
}

CheckHex::operator QByteArray()
{
    return value;
}

Integer::Integer(){}

Integer::Integer(quint32 v):Optional<quint32>(v){}

Integer::Integer(stellar::Optional<quint32> optional){
    if(optional.filled){
        filled = optional.filled;
        value = optional.value;
    }
}

bool Integer::isNull() const{
    return !filled;
}

bool Integer::operator==(const Integer &other){
    return filled== other.filled && value==other.value;
}

Integer::operator quint32()
{
    if(filled)
        return value;
    return 0;
}

bool operator==(const Integer &a, const Integer &b){
    return a.filled== b.filled && a.value==b.value;
}


Boolean::Boolean(){}

Boolean::Boolean(bool v):Optional<bool>(v){}

Boolean::Boolean(stellar::Optional<bool> optional){
    if(optional.filled){
        filled = optional.filled;
        value = optional.value;
    }
}

Boolean::Boolean(const QVariant& v){
    if(!v.isNull()&&v.type()==QVariant::Bool)
    {
        filled = 1;
        value = v.toBool();
    }
}

bool Boolean::isNull() const{
    return !filled;
}

bool Boolean::operator==(const Boolean &other){
    return filled== other.filled && value==other.value;
}

Boolean::operator bool()
{
    if(filled)
        return value;
    return 0;
}

bool operator==(const Boolean &a, const Boolean &b){
    return a.filled== b.filled && a.value==b.value;
}

qint64 get_power(quint32 a, quint32 b)
{
    qint64 res = 1;
    for(quint32 i=0;i<b;i++)
    {
        res *= a ;
    }
    return res;
}
