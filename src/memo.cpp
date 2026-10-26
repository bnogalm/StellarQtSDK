#include "memo.h"
#include <cstring>

QSTELLAR_BEGIN_NS

namespace {
// A MEMO_ID from text: decimal digits only, as js Memo.id reads it.
// toULongLong turned what it could not read ("abc", "", 2^64) into 0, a memo
// id an exchange routes deposits by, and took a sign or spaces.
quint64 memoIdFromText(const QString& text)
{
    bool ok = !text.isEmpty();
    for (const QChar c : text) {
        if (c.unicode() < '0' || c.unicode() > '9')
            ok = false;
    }
    const quint64 id = ok ? text.toULongLong(&ok, 10) : 0;
    if (!ok)
        throw std::runtime_error("memo id is not a uint64 in decimal digits");
    return id;
}
}

Memo::Memo()
{

}

Memo::~Memo(){

}
Memo *Memo::fromXdr(stellar::Memo& memo)
{
    switch(memo.type)
    {
    case stellar::MemoType::MEMO_NONE:
        return Memo::none();
    case stellar::MemoType::MEMO_TEXT:
    {
        // The bytes, not text decoded from them: a memo that is not UTF-8 was
        // re-encoded with U+FFFD in it, and signed as a different memo.
        return MemoText::fromBytes(QByteArray(memo.text.value.data(), memo.text.value.size()));
    }
    case stellar::MemoType::MEMO_ID:
    {
        return Memo::id(memo.id);
    }
    case stellar::MemoType::MEMO_HASH:
    {
        QByteArray data((const char*)memo.hash,sizeof(memo.hash));
        return Memo::hash(data);
    }
    case stellar::MemoType::MEMO_RETURN:
    {
        QByteArray data((const char*)memo.retHash,sizeof(memo.retHash));
        return Memo::returnHash(data);
    }
    };
    return nullptr;
}
Memo *Memo::parse(QString type, QString memo)
{
    if (type == "none") {
        return Memo::none();
    } else {
        if (type=="text") {
            // Because of the way "encoding/json" works on structs in Go, if transaction
            // has an empty `memo_text` value, the `memo` field won't be present in a JSON
            // representation of a transaction. That's why we need to handle a special case
            // here.
            if(!memo.isNull())
                return Memo::text(memo);
            else
                return Memo::text("");//null!=empty
        } else if (type=="id") {
            return Memo::id(memoIdFromText(memo));
        } else if (type=="hash") {
            return Memo::hash(memo);
        } else if (type=="return") {
            return Memo::returnHash(memo);
        } else {
            throw std::runtime_error("Unknown memo type.");
        }
    }
}

Memo *Memo::parse(QString type, QByteArray memo)
{
    if (type == "none") {
        return Memo::none();
    } else {
        if (type=="text") {
            // Because of the way "encoding/json" works on structs in Go, if transaction
            // has an empty `memo_text` value, the `memo` field won't be present in a JSON
            // representation of a transaction. That's why we need to handle a special case
            // here.
            if(!memo.isNull())
                return Memo::text(QString::fromUtf8(memo));
            else
                return Memo::text("");//null!=empty
        } else if (type=="id") {
            // FIX §9.8: Memo Id is uint64. toLongLong() rejected IDs > 2^63;
            // the full range is read, and nothing else.
            return Memo::id(memoIdFromText(QString::fromUtf8(memo)));
        } else if (type=="hash" || type=="return") {
            // Strict: the lenient decoder skipped what it could not read, so
            // a damaged value read as other bytes.
            const auto bytes = QByteArray::fromBase64Encoding(
                memo, XDR_BASE64ENCODING | QByteArray::AbortOnBase64DecodingErrors);
            if (!bytes)
                throw std::runtime_error("memo is not base64");
            if (type=="hash")
                return Memo::hash(*bytes);
            return Memo::returnHash(*bytes);
        } else {
            throw std::runtime_error("Unknown memo type.");
        }
    }
}

MemoNone* Memo::none() {
    return new MemoNone();
}

MemoText* Memo::text(QString text) {
    return new MemoText(text);
}

MemoId* Memo::id(quint64 id) {
    return new MemoId(id);
}

MemoHash* Memo::hash(QByteArray bytes) {
    return new MemoHash(bytes);
}

MemoHash* Memo::hash(QString hexString){// throws DecoderException {
    return new MemoHash(hexString);
}

MemoReturnHash* Memo::returnHash(QByteArray bytes) {
    return new MemoReturnHash(bytes);
}

MemoReturnHash* Memo::returnHash(QString hexString){// throws DecoderException {
    return new MemoReturnHash(hexString);
}

bool Memo::equals(Memo *memo) const
{
    return *this == *memo;
}

bool Memo::operator==(const Memo &memo) const
{
    if(typeid(*this)==typeid(memo))
    {
        if(const MemoText * text = dynamic_cast<const MemoText*>(this))
        {
            return text->getBytes()==dynamic_cast<const MemoText&>(memo).getBytes();
        }
        if(dynamic_cast<const MemoNone*>(this))
        {
            return true;
        }
        if(const MemoHashAbstract * hash = dynamic_cast<const MemoHashAbstract*>(this))
        {
            return hash->getBytes()==dynamic_cast<const MemoHashAbstract&>(memo).getBytes();
        }
        if(const MemoId * id = dynamic_cast<const MemoId*>(this))
        {
            return id->getId()==dynamic_cast<const MemoId&>(memo).getId();
        }
    }
    return false;
}

stellar::Memo MemoNone::toXdr() {
    using namespace stellar;
    stellar::Memo memo;
    memo.type = MemoType::MEMO_NONE;
    return memo;
}

MemoNone::~MemoNone(){

}

MemoReturnHash::MemoReturnHash(QByteArray bytes):MemoHashAbstract(bytes) {
}


MemoReturnHash::MemoReturnHash(QString hexString):MemoHashAbstract(CheckHex(hexString)){// throws DecoderException {
}

MemoReturnHash::~MemoReturnHash(){

}

stellar::Memo MemoReturnHash::toXdr() {
    using namespace stellar;
    stellar::Memo memo;
    memo.type = MemoType::MEMO_RETURN;
    memcpy(memo.hash,this->m_bytes.constData(),sizeof(memo.hash));
    return memo;
}

MemoText::MemoText(QString text) {
    this->m_text = checkNotNull(text, "text cannot be null");
    m_bytes = text.toUtf8();
    if (Util::hasUnpairedSurrogate(text)) {
        throw std::runtime_error("memo text is not valid Unicode (an unpaired surrogate?)");
    }
    if (m_bytes.length() > 28) {
        throw std::runtime_error("text must be <= 28 bytes.");
    }
}

MemoText::MemoText(const QByteArray& bytes, FromBytes)
    : m_text(QString::fromUtf8(bytes.constData(), bytes.size())), m_bytes(bytes)
{
    if (m_bytes.length() > 28) {
        throw std::runtime_error("text must be <= 28 bytes.");
    }
}

MemoText* MemoText::fromBytes(const QByteArray& bytes)
{
    return new MemoText(bytes, FromBytes());
}

MemoText::~MemoText(){

}


QString MemoText::getText() const{
    return m_text;
}

QByteArray MemoText::getBytes() const{
    return m_bytes;
}

stellar::Memo MemoText::toXdr() {
    using namespace stellar;
    stellar::Memo memo;
    memo.type = MemoType::MEMO_TEXT;
    QByteArray bytes = m_bytes;
    memo.text.set(bytes.data(), bytes.length());
    return memo;
}

MemoHashAbstract::MemoHashAbstract(QByteArray bytes) {
    // Exactly 32 bytes (64 hex characters), as js and py require. Shorter
    // input was padded with zeros and signed: "" as the all-zero hash, 31
    // bytes as a hash nobody gave.
    if (bytes.length() != 32) {
        throw std::runtime_error(QStringLiteral("a MEMO_HASH / MEMO_RETURN is exactly 32 bytes "
                                                "(64 hex characters), not %1").arg(bytes.length()).toStdString());
    }
    this->m_bytes = bytes;
}


MemoHashAbstract::MemoHashAbstract(QString hexString): MemoHashAbstract(QByteArray::fromHex(hexString.toLatin1())){
}

MemoHashAbstract::~MemoHashAbstract(){

}


QByteArray MemoHashAbstract::getBytes() const {
    return m_bytes;
}

QString MemoHashAbstract::getHexValue() const {
    return QString(m_bytes.toHex());
}

QString MemoHashAbstract::getTrimmedHexValue() {
    QByteArray ba= m_bytes;
    int i=ba.size()-1;
    for(;i>=0;i--){
        if(ba[i]!='\0'){
            break;
        }
    }
    return QString::fromLatin1(m_bytes.left(i+1).toHex());

}

MemoHash::MemoHash(QByteArray bytes):MemoHashAbstract(bytes) {

}

MemoHash::MemoHash(QString hexString):MemoHash(CheckHex(hexString)) {
}

MemoHash::~MemoHash(){

}

stellar::Memo MemoHash::toXdr() {
    using namespace stellar;
    stellar::Memo memo;
    memo.type = MemoType::MEMO_HASH;
    memcpy(memo.hash,this->m_bytes.constData(),sizeof(memo.hash));
    return memo;
}

MemoId::MemoId(quint64 id) {
    this->m_id = id;
}

MemoId::~MemoId(){

}


quint64 MemoId::getId() const {
    return m_id;
}

stellar::Memo MemoId::toXdr()
{
    using namespace stellar;
    stellar::Memo memo;
    memo.type = MemoType::MEMO_ID;
    memo.id=m_id;
    return memo;
}

QSTELLAR_END_NS
