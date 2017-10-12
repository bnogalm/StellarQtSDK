#ifndef MEMO_H
#define MEMO_H
#include <QString>
#include <QByteArray>
#include "xdr/stellartransaction.h"
#include "util.h"
/**
 * <p>The memo contains optional extra information. It is the responsibility of the client to interpret this value. Memos can be one of the following types:</p>
 * <ul>
 *  <li><code>MEMO_NONE</code>: Empty memo.</li>
 *  <li><code>MEMO_TEXT</code>: A string up to 28-bytes long.</li>
 *  <li><code>MEMO_ID</code>: A 64 bit unsigned integer.</li>
 *  <li><code>MEMO_HASH</code>: A 32 byte hash.</li>
 *  <li><code>MEMO_RETURN</code>: A 32 byte hash intended to be interpreted as the hash of the transaction the sender is refunding.</li>
 * </ul>
 * <p>Use static methods to generate any of above types.</p>
 * @see Transaction
 */
class MemoNone;
class MemoText;
class MemoId;
class MemoHash;
class MemoReturnHash;
class Memo
{
public:
    Memo();
    virtual ~Memo();

    /**
     * @brief parse parse a memo according to the type
     * @param type
     * @param memo memo text/id/or HEXADECIMAL, usefull for user interface, for responses you must use QByteArray version
     * @return
     */

    static Memo* parse(QString type, QString memo);//memo
    /**
     * @brief parse parse a memo according to the type
     * @param type
     * @param memo memo text (utf8) /id/or BASE64, usefull for user interface, for responses you must use QByteArray version
     * @return
     */
    static Memo* parse(QString type, QByteArray memo);
    /**
      * Creates new MemoNone instance.
      */
    static MemoNone* none();

    /**
      * Creates new {@link MemoText} instance.
      * @param text
      */
    static MemoText* text(QString text);

    /**
      * Creates new {@link MemoId} instance.
      * @param id
      */
    static MemoId* id(qint64 id);

    /**
      * Creates new {@link MemoHash} instance from byte array.
      * @param bytes
      */
    static MemoHash* hash(QByteArray bytes);

    /**
      * Creates new {@link MemoHash} instance from hex-encoded string
      * @param hexString
      * @throws DecoderException
      */
    static MemoHash* hash(QString hexString);

    /**
      * Creates new {@link MemoReturnHash} instance from byte array.
      * @param bytes
      */
    static MemoReturnHash* returnHash(QByteArray bytes);

    /**
      * Creates new {@link MemoReturnHash} instance from hex-encoded string.
      * @param hexString
      * @throws DecoderException
      */
    static MemoReturnHash* returnHash(QString hexString);
    virtual stellar::Memo toXdr() = 0;

};


class MemoHashAbstract : public Memo {
protected:
    QByteArray m_bytes;

public:
    MemoHashAbstract(QByteArray bytes) {
        if (bytes.length() < 32) {
            this->m_bytes = Util::paddedByteArray(bytes, 32);
        } else if (bytes.length() > 32) {
             throw std::runtime_error("MEMO_HASH can contain 32 bytes at max.");
        }
        else{

            this->m_bytes = bytes;
        }
    }

    MemoHashAbstract(QString hexString): MemoHashAbstract(QByteArray::fromHex(hexString.toLatin1())){// throws DecoderException {
    }
    virtual ~MemoHashAbstract();

    /**
   * Returns 32 bytes long array contained in this memo.
   */
    QByteArray getBytes() {
        return m_bytes;
    }

    /**
   * <p>Returns hex representation of bytes contained in this memo.</p>
   *
   * <p>Example:</p>
   * <code>
   *   MemoHash memo = new MemoHash("4142434445");
   *   memo.getHexValue(); // 4142434445000000000000000000000000000000000000000000000000000000
   *   memo.getTrimmedHexValue(); // 4142434445
   * </code>
   */
    QString getHexValue() {
        return QString(m_bytes.toHex());
    }

    /**
   * <p>Returns hex representation of bytes contained in this memo until null byte (0x00) is found.</p>
   *
   * <p>Example:</p>
   * <code>
   *   MemoHash memo = new MemoHash("4142434445");
   *   memo.getHexValue(); // 4142434445000000000000000000000000000000000000000000000000000000
   *   memo.getTrimmedHexValue(); // 4142434445
   * </code>
   */
    QString getTrimmedHexValue();
    stellar::Memo toXdr() = 0;


};



/**
 * Represents MEMO_HASH.
 */
class MemoHash : public MemoHashAbstract {
    MemoHash(CheckHex hex): MemoHashAbstract(hex.value){
    }
public:
    MemoHash(QByteArray bytes):MemoHashAbstract(bytes) {

    }

    MemoHash(QString hexString):MemoHash(CheckHex(hexString)) {
    }


    virtual ~MemoHash();

    stellar::Memo toXdr() {
        using namespace stellar;
        stellar::Memo memo;
        memo.type = MemoType::MEMO_HASH;
        memcpy(memo.hash,this->m_bytes.constData(),sizeof(memo.hash));
        return memo;
    }
};


/**
 * Represents MEMO_ID.
 */
class MemoId : public Memo {
  private:
    qint64 m_id;

  public:
    MemoId(qint64 id) {
    if (id < 0) {
        //throw new IllegalArgumentException("id must be a positive number");
    }
    this->m_id = id;
  }
    virtual ~MemoId();

  qint64 getId() {
    return m_id;
  }

  stellar::Memo toXdr()
  {
      using namespace stellar;
      stellar::Memo memo;
      memo.type = MemoType::MEMO_ID;
      memo.id=m_id;
      return memo;
  }

};

/**
 * Represents MEMO_NONE.
 */
class MemoNone : public Memo {
public:
    stellar::Memo toXdr();
    virtual ~MemoNone();
};

/**
 * Represents MEMO_RETURN.
 */
class MemoReturnHash : public MemoHashAbstract {
public:
  MemoReturnHash(QByteArray bytes):MemoHashAbstract(bytes) {
  }

  MemoReturnHash(QString hexString):MemoHashAbstract(hexString){// throws DecoderException {
  }
  virtual ~MemoReturnHash();

  stellar::Memo toXdr();
};


/**
 * Represents MEMO_TEXT.
 */
class MemoText : public Memo {
  private:
    QString m_text;

  public:
    MemoText(QString text);
    virtual ~MemoText();
  QString getText() {
    return m_text;
  }

  stellar::Memo toXdr() {
      using namespace stellar;
      stellar::Memo memo;
      memo.type = MemoType::MEMO_TEXT;
      QByteArray ba  = m_text.toUtf8();
      memo.text.set(ba.data(),ba.length());
    return memo;
  }
};

inline Memo * checkNotNull(Memo *t, const char *error)
{
    if(!t)
        throw std::runtime_error(error);
    return t;
}
#endif // MEMO_H
