#include "memo.h"

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
        QString text = QString::fromUtf8(memo.text.value.data(),memo.text.value.size());
        return Memo::text(text);
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
            return Memo::id(memo.toULongLong());
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
            return Memo::id(QString::fromUtf8(memo).toLongLong());
        } else if (type=="hash") {
            return Memo::hash(QByteArray::fromBase64(memo,XDR_BASE64ENCODING));
        } else if (type=="return") {
            return Memo::returnHash(QByteArray::fromBase64(memo,XDR_BASE64ENCODING));
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

MemoId* Memo::id(qint64 id) {
    return new MemoId(id);
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
            return text->getText()==dynamic_cast<const MemoText&>(memo).getText();
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

    int length = text.toUtf8().length();
    if (length > 28) {
        throw std::runtime_error("text must be <= 28 bytes.");
    }
}

MemoText::~MemoText(){

}


QString MemoText::getText() const{
    return m_text;
}

stellar::Memo MemoText::toXdr() {
    using namespace stellar;
    stellar::Memo memo;
    memo.type = MemoType::MEMO_TEXT;
    QByteArray ba  = m_text.toUtf8();
    memo.text.set(ba.data(),ba.length());
    return memo;
}

MemoHashAbstract::MemoHashAbstract(QByteArray bytes) {
    if (bytes.length() < 32) {
        this->m_bytes = Util::paddedByteArray(bytes, 32);
    } else if (bytes.length() > 32) {
        throw std::runtime_error("MEMO_HASH can contain 32 bytes at max.");
    }
    else{
        this->m_bytes = bytes;
    }
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

MemoId::MemoId(qint64 id) {
    if (id < 0) {
        throw std::runtime_error("id must be a positive number");
    }
    this->m_id = static_cast<quint64>(id);
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
