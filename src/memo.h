#ifndef MEMO_H
#define MEMO_H
#include <QString>
#include <QByteArray>
#include "xdr/stellartransaction.h"
#include "util.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS
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
     * An id is a uint64 in decimal digits, as js Memo.id takes it; any other
     * text throws (it was read as memo id 0).
     */

    static Memo* parse(QString type, QString memo);//memo
    /**
     * @brief parse parse a memo according to the type
     * @param type
     * @param memo memo text (utf8) /id/or BASE64, usefull for user interface, for responses you must use QByteArray version
     * @return
     * An id is a uint64 in decimal digits; any other text throws, as a hash
     * that is not base64 does.
     */
    static Memo* parse(QString type, QByteArray memo);

    static Memo* fromXdr(stellar::Memo &memo);
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
      * @param id  unsigned 64-bit memo id.
      *
      * NOTE: the historical `id(qint64)` overload was removed in 1.0.0 —
      * MemoId only carries a 64-bit unsigned value. Pass a `quint64` (or
      * any positive integer literal, which converts implicitly).
      */
    static MemoId* id(quint64 id);

    /**
      * Creates new {@link MemoHash} instance from byte array.
      * @param bytes exactly 32 bytes, as js and py require; anything else
      *        throws std::runtime_error (1.0.3 padded shorter input with zeros)
      */
    static MemoHash* hash(QByteArray bytes);

    /**
      * Creates new {@link MemoHash} instance from hex-encoded string of exactly
      * 64 hex characters (32 bytes); anything else throws
      * @param hexString
      * @throws DecoderException
      */
    static MemoHash* hash(QString hexString);

    /**
      * Creates new {@link MemoReturnHash} instance from byte array.
      * @param bytes exactly 32 bytes, as js and py require; anything else
      *        throws std::runtime_error (1.0.3 padded shorter input with zeros)
      */
    static MemoReturnHash* returnHash(QByteArray bytes);

    /**
      * Creates new {@link MemoReturnHash} instance from hex-encoded string of exactly
      * 64 hex characters (32 bytes); anything else throws.
      * @param hexString
      * @throws DecoderException
      */
    static MemoReturnHash* returnHash(QString hexString);
    virtual stellar::Memo toXdr() = 0;

    bool equals(Memo* memo) const;
    bool operator==(const Memo &memo) const;

};


class MemoHashAbstract : public Memo {
protected:
    QByteArray m_bytes;

public:
    MemoHashAbstract(QByteArray bytes);

    MemoHashAbstract(QString hexString);

    virtual ~MemoHashAbstract();

    /**
   * Returns 32 bytes long array contained in this memo.
   */

    QByteArray getBytes() const;

    /**
   * <p>Returns hex representation of bytes contained in this memo.</p>
   *
   * <p>Example:</p>
   * <code>
   *   MemoHash memo = new MemoHash("4142434445000000000000000000000000000000000000000000000000000000");
   *   memo.getHexValue(); // 4142434445000000000000000000000000000000000000000000000000000000
   *   memo.getTrimmedHexValue(); // 4142434445
   * </code>
   */

    QString getHexValue() const;

    /**
   * <p>Returns hex representation of bytes contained in this memo until null byte (0x00) is found.</p>
   *
   * <p>Example:</p>
   * <code>
   *   MemoHash memo = new MemoHash("4142434445000000000000000000000000000000000000000000000000000000");
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


public:
    MemoHash(QByteArray bytes);

    MemoHash(QString hexString);


    virtual ~MemoHash();

    stellar::Memo toXdr();
};


/**
 * Represents MEMO_ID.
 */
class MemoId : public Memo {
private:

    quint64 m_id;

public:
    MemoId(quint64 id);
    virtual ~MemoId();

    quint64 getId() const;

    stellar::Memo toXdr();

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

    MemoReturnHash(QByteArray bytes);

    MemoReturnHash(QString hexString);
    virtual ~MemoReturnHash();

    stellar::Memo toXdr();
};


/**
 * Represents MEMO_TEXT.
 */
class MemoText : public Memo {
private:
    QString m_text;
    QByteArray m_bytes;     // what is signed

    struct FromBytes {};
    MemoText(const QByteArray& bytes, FromBytes);

public:
    /**
     * UTF-8 of `text`, at most 28 bytes. Text that UTF-8 cannot hold as given
     * (an unpaired surrogate, e.g. an emoji cut in half) is refused: it used
     * to be signed with the surrogate dropped (Qt 6) or as '?' (Qt 5).
     */
    MemoText(QString text);

    /**
     * The memo's bytes exactly as they are on the wire, at most 28. A memo
     * text does not have to be UTF-8, and one decoded from XDR is written back
     * as the same bytes — not re-encoded from text, which changed them.
     */
    static MemoText* fromBytes(const QByteArray& bytes);

    virtual ~MemoText();

    /** For display: the bytes decoded as UTF-8 (invalid bytes become U+FFFD). */
    QString getText() const;

    /** The bytes that are signed. */
    QByteArray getBytes() const;

    stellar::Memo toXdr();
};

inline Memo * checkNotNull(Memo *t, const char *error)
{
    if(!t)
        throw std::runtime_error(error);
    return t;
}

QSTELLAR_END_NS

QSTELLAR_ALIAS(Memo)
QSTELLAR_ALIAS(MemoHashAbstract)
QSTELLAR_ALIAS(MemoNone)
QSTELLAR_ALIAS(MemoText)
QSTELLAR_ALIAS(MemoId)
QSTELLAR_ALIAS(MemoHash)
QSTELLAR_ALIAS(MemoReturnHash)

#endif // MEMO_H
