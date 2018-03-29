#include "memo.h"

Memo::Memo()
{

}

Memo::~Memo(){

}
Memo *Memo::fromXdr(stellar::Memo memo)
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
        QByteArray data = QByteArray::fromRawData(memo.text.value.data(),memo.text.value.size());
        return Memo::hash(data);
    }
    case stellar::MemoType::MEMO_RETURN:
    {
        QByteArray data = QByteArray::fromRawData(memo.text.value.data(),memo.text.value.size());
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
            return Memo::id(memo.toLongLong());
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

stellar::Memo MemoNone::toXdr() {
    using namespace stellar;
    stellar::Memo memo;
    memo.type = MemoType::MEMO_NONE;
    return memo;
}

MemoNone::~MemoNone(){

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

MemoHashAbstract::~MemoHashAbstract(){

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

MemoHash::~MemoHash(){

}

MemoId::~MemoId(){

}
