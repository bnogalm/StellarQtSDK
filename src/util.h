#ifndef UTIL_H
#define UTIL_H
#include <QString>
#include <QCryptographicHash>
#include <QList>
#include <QRegularExpression>
#include "xdr/stellartransaction.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

// Forward-declare Asset / AssetTypeCreditAlphaNum (now in qstellar::).
// Util methods take/return these types; the actual headers are included
// from util.cpp.
class Asset;
class AssetTypeCreditAlphaNum;

QString checkNotNull(QString p, const char* error);
QByteArray checkNotNull(QByteArray p, const char* error);


#define XDR_BASE64ENCODING QByteArray::Base64Encoding

template<typename Type>
inline typename std::enable_if<std::is_integral<Type>::value,Type>::type
checkNotNull(Type p, const char* error){
    if(p==0){
        throw std::runtime_error(error);
    }
    return p;
}


template<typename Type>
inline QVector<Type> checkNotNull(QVector<Type> &p, const char *error)
{
    if(p.isEmpty()){
        throw std::runtime_error(error);
    }
    return p;
}


#define checkArgument(EXP,error)\
    if(!(EXP)) throw std::runtime_error(error)


// Forward-decl of qstellar::Transaction. We are already inside QSTELLAR_BEGIN_NS,
// so a plain `class Transaction;` declares it in the right namespace. The
// global alias is emitted by transaction.h's QSTELLAR_ALIAS(Transaction).
class Transaction;


class Util
{
public:
    Util();
    /**
     * Half of a surrogate pair: text UTF-8 cannot hold as given. Checked
     * directly, not by a UTF-8 round trip, which also changed a leading
     * U+FEFF and (on Qt 5) stopped at a NUL.
     */
    static bool hasUnpairedSurrogate(const QString& s)
    {
        for (int i = 0; i < s.size(); ++i) {
            if (s.at(i).isHighSurrogate()) {
                if (i + 1 >= s.size() || !s.at(i + 1).isLowSurrogate())
                    return true;
                ++i;
            } else if (s.at(i).isLowSurrogate()) {
                return true;
            }
        }
        return false;
    }

    /**
     * Signed UTF-8 text (a data name, a home domain) as text, every leading
     * byte order mark kept as U+FEFF. QString::fromUtf8 drops a leading EF BB
     * BF, so "\xEF\xBB\xBFk" read as "k", another name; js, java and py keep
     * it. The rest is decoded on its own, so it never starts with a BOM on
     * either Qt 5 or Qt 6.
     */
    static QString fromUtf8KeepingBom(const QByteArray& raw)
    {
        static const QByteArray bom("\xEF\xBB\xBF", 3);
        QString leading;
        int i = 0;
        while (raw.size() - i >= 3 && raw.mid(i, 3) == bom) {
            leading += QChar(0xFEFF);
            i += 3;
        }
        return leading + QString::fromUtf8(raw.constData() + i, raw.size() - i);
    }

    static QString bytesToHex(QByteArray bytes)
    {
        return QString::fromLatin1(bytes.toHex());
    }
    /**
     * Hex to bytes, strictly: an even number of hex digits and nothing else,
     * or it throws. QByteArray::fromHex skips what it does not understand, so
     * a claimable balance id with an 'l' for a '1' was signed shifted by a
     * nibble — a different id.
     */
    static QByteArray hexToBytes(QString s){
        const QByteArray latin = s.toLatin1();
        bool ok = latin.size() % 2 == 0 && QString::fromLatin1(latin) == s;
        for (const char c : latin) {
            if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')))
                ok = false;
        }
        if (!ok)
            throw std::runtime_error("not hex: an even number of the digits 0-9 and a-f was expected");
        return QByteArray::fromHex(latin);
    }
    /**
     * Returns SHA-256 hash of <code>data</code>.
     * @param data
     */
    static QByteArray hash(QByteArray data);

    /**
     * Pads <code>bytes</code> array to <code>length</code> with zeros.
     * @param bytes
     * @param length
     */
    static QByteArray paddedByteArray(QByteArray bytes, int length);

    /**
     * Pads <code>string</code> to <code>length</code> with zeros.
     * @param string
     * @param length
     */
    static QByteArray paddedByteArray(QString string, int length);

    /**
     * Remove zeros from the end of <code>bytes</code> array.
     * @param bytes
     */
    static QString paddedByteArrayToString(QByteArray bytes);
    static QByteArray removeTailChars(QByteArray bytes, const char c);
    static QString removeTailChars(QString string, QChar c);

    // generateRandomNonce creates a cryptographically secure random slice of `n` bytes.
    static QByteArray generateRandomNonce(int n);

    /**
     * The BIP-39 wordlists, the eight py-stellar-sdk has: English, Japanese,
     * French, Spanish, Italian, Korean, Chinese (simplified, traditional).
     * A phrase is read against one list, as py reads it with the Language
     * given: the same words in another list are another phrase or none.
     */
    enum class SupportedMnemonicLanguages{ EN, JA, FR, ES, IT, KO, ZH_HANS, ZH_HANT };
    /** The 2048 words of a list, in index order, NFKD (as BIP-39 and py have them). */
    static QList<QString> mnemonicWords(SupportedMnemonicLanguages language = SupportedMnemonicLanguages::EN);
    /**
     * BIP-39 seed: PBKDF2-HMAC-SHA512, 2048 rounds, of the sentence and of
     * "mnemonic" + `passphrase`, both UTF-8 in NFKD as BIP-39 requires (so a
     * passphrase typed composed or decomposed gives the same wallet). The
     * words are read from the NFKD sentence, separated by spaces (runs of
     * them count as one, tabs and line breaks included, as the QML layer
     * reads a pasted phrase). py-stellar-sdk 14.1.0 refuses such a phrase,
     * and one with a leading or trailing space: it splits on single spaces.
     * The seed is the single-spaced phrase's either way, never another
     * wallet's. Throws on a word not in the list, a word count other than
     * 12/15/18/21/24, a checksum that does not match (as py-stellar-sdk; it
     * was not checked before 1.0.4, so a mistyped phrase opened another
     * wallet), or text that is not valid UTF-16. KeyPair::fromMnemonic goes
     * from the phrase to an account.
     */
    static QByteArray mnemonicToBIP39Seed(QString words, QString passphrase=QString(""),
                                          SupportedMnemonicLanguages language = SupportedMnemonicLanguages::EN);

    /**
     * The 1.0.0-1.0.3 derivation, for recovering wallets created with those
     * versions (and every earlier one); NOT BIP-39. Those releases put the
     * passphrase into the salt as typed, "mnemonic" + passphrase in UTF-8
     * without NFKD, so a passphrase whose NFKD form differs from it (a
     * precomposed accented letter, a compatibility character) opened other
     * accounts than mnemonicToBIP39Seed and every other BIP-39 wallet derive
     * from it. For any other passphrase, every ASCII one included, the two
     * seeds are the same: see legacyPassphraseDerivesDifferently.
     *
     * The words are read and refused as mnemonicToBIP39Seed reads them, which
     * for any phrase those releases accepted gives the words they used; the
     * checksum is NOT checked here, as those releases did not check it, so a
     * wallet made from a mistyped phrase can still be opened. A passphrase
     * that is not valid UTF-16 is refused here too. Use it to find and move
     * funds, never to create a wallet.
     */
    static QByteArray mnemonicToBIP39SeedLegacyPassphrase(QString words, QString passphrase=QString(""));

    /**
     * True when mnemonicToBIP39Seed and mnemonicToBIP39SeedLegacyPassphrase
     * derive different seeds from `passphrase`, which is when a wallet created
     * with it by 1.0.3 or earlier is only found through the legacy derivation.
     * False for a passphrase both refuse (not valid UTF-16).
     */
    static bool legacyPassphraseDerivesDifferently(const QString& passphrase);

    /** BIP-39: turn entropy (16/20/24/28/32 bytes) into a mnemonic phrase,
     *  its words separated by spaces, or by U+3000 in Japanese (as BIP-39 and
     *  py write it). */
    static QString entropyToMnemonic(const QByteArray& entropy,
                                     SupportedMnemonicLanguages language = SupportedMnemonicLanguages::EN);
    /** BIP-39: generate a fresh mnemonic from secure random entropy.
     *  `strengthBits` ∈ {128,160,192,224,256} → 12/15/18/21/24 words. */
    static QString generateMnemonic(int strengthBits = 128,
                                    SupportedMnemonicLanguages language = SupportedMnemonicLanguages::EN);
    /** BIP-39: word-count, wordlist membership and checksum validation. */
    static bool validateMnemonic(const QString& mnemonic,
                                 SupportedMnemonicLanguages language = SupportedMnemonicLanguages::EN);
    /**
     * Why validateMnemonic() refuses `mnemonic`, empty when it does not: the
     * text is not valid Unicode, the word count is not 12/15/18/21/24, word
     * N is not in the list, or the checksum does not match. A word is named
     * by its position, never repeated: a phrase is a secret.
     */
    static QString mnemonicProblem(const QString& mnemonic,
                                   SupportedMnemonicLanguages language = SupportedMnemonicLanguages::EN);


    static void claimableBalanceIdToXDR(QString balanceID, stellar::ClaimableBalanceID& balanceIdToFill);

    static QString xdrToClaimableBalanceId(stellar::ClaimableBalanceID& balanceId);

    static AssetTypeCreditAlphaNum* assertNonNativeAsset(Asset* asset);

    /**
     * CAP-46-7 — derive the contract id (32-byte hash) for a contract created
     * from `fromAddress` + `salt` on the given network.
     *
     *   networkId        = SHA256(networkPassphrase)
     *   contractIdHash   = SHA256(XDR(HashIDPreimage::ContractID(networkId,
     *                                  FromAddress(fromAddress, salt))))
     */
    static QByteArray getContractAddress(const QString& networkPassphrase,
                                         const stellar::SCAddress& fromAddress,
                                         const QByteArray& salt32);

    /**
     * CAP-46-7 — derive the SAC (Stellar Asset Contract) contract id for an
     * Asset on the given network. Mirrors `Asset::toSAC` in the Java SDK.
     */
    static QByteArray getContractAddressFromAsset(const QString& networkPassphrase,
                                                  const stellar::Asset& asset);
};


/**
 * @brief The CheckHex class
 * Helper class to check hex parameters
 */
class CheckHex{

public:
    QByteArray value;
    CheckHex(QString v);
    operator QByteArray();
};



/**
 * @brief The Integer struct
 * helper struct to be able to have unassigned value as in java Integer type
 */

struct Integer: public stellar::Optional<quint32>
{
    Integer();
    Integer(quint32 v);
    Integer(stellar::Optional<quint32> optional);
    operator quint32();
    bool isNull() const;
    bool operator==(const Integer &other);
};
bool operator==(const Integer &a, const Integer &b);


/**
 * @brief The Boolean struct
 * helper struct to be able to have unassigned value as in java Boolean type
 */

struct Boolean: public stellar::Optional<bool>
{
    Boolean();
    Boolean(bool v);
    Boolean(stellar::Optional<bool> optional);
    Boolean(const QVariant& v);
    operator bool();
    bool isNull() const;
    bool operator==(const Boolean &other);
};
bool operator==(const Boolean &a, const Boolean &b);



template <int A, int B, typename T>
struct get_power_s
{
    static const T value = A * get_power_s<A, B - 1, T>::value;
};
template <int A, typename T>
struct get_power_s<A, 0, T>
{
    static const T value = 1;
};


qint64 get_power(quint32 a, quint32 b);

QSTELLAR_END_NS

QSTELLAR_ALIAS(Util)
QSTELLAR_ALIAS(CheckHex)
QSTELLAR_ALIAS(Integer)
QSTELLAR_ALIAS(Boolean)
#ifndef STELLAR_QT_LEGACY_GLOBAL_NS
using ::qstellar::checkNotNull;
using ::qstellar::get_power;
using ::qstellar::get_power_s;     // template — usable as e.g. get_power_s<10,7,qint64>
#endif

#endif // UTIL_H
