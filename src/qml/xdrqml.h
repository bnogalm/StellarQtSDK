#ifndef QSTELLAR_XDRQML_H
#define QSTELLAR_XDRQML_H

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * Decodes the base64 XDR that Stellar hands out, for people to read.
 *
 * Everything on Stellar is described in XDR — the transaction you send, the
 * result that comes back, the entries in the ledger — and everything that
 * travels over HTTP is that XDR in base64. So the ability to paste a blob and
 * be told what it says is the difference between debugging and guessing.
 *
 * THE TYPE IS NOT IN THE BYTES. XDR has no self-describing header: the same
 * bytes are a valid something-else under a different reading, and a decoder
 * given the wrong type does not usually fail — it produces confident nonsense.
 * So the caller says which type it is, and the honest failure mode is a garbled
 * answer, not an error. `supportedTypes()` lists only what this SDK can really
 * read; where it cannot, the note says so instead of the menu quietly omitting
 * it.
 *
 * Offline and synchronous, so no thread and no request ids. Nothing throws into
 * QML: a blob that cannot be read comes back as { error }.
 *
 * Registered as the `Xdr` singleton by QStellarQml::registerTypes().
 */
class XdrQml : public QObject
{
    Q_OBJECT
public:
    explicit XdrQml(QObject* parent = nullptr);

    /**
     * The types this can decode, as { name, label, note }.
     *
     * The list is deliberately short. Several XDR types in this SDK have a
     * stream operator that silently reads nothing for the arms it does not
     * know, and offering those would be a menu entry that always looks like it
     * worked.
     */
    Q_INVOKABLE QVariantList supportedTypes() const;

    /**
     * Decodes `base64` as `type`, returning { columns, rows } or { error }.
     *
     * The rows are field-and-value pairs already rendered as text, and result
     * codes are rendered by NAME rather than as the negative number they are
     * on the wire — `tx_bad_seq` rather than -5 — because the number is the
     * one thing nobody can look up from memory.
     *
     * `base64` is the standard alphabet with = padding. The ASCII
     * whitespace in it (space, tab, LF, form feed and CR), at its ends or
     * inside, is skipped, as js-stellar-sdk and py-stellar-sdk skip it, so
     * line-wrapped XDR reads; any other character is an error, a vertical
     * tab or a non-breaking space included, as js makes it (py skips any
     * character outside the alphabet). Before 1.0.4, whitespace inside the
     * text was an error too.
     */
    Q_INVOKABLE QVariantMap decode(const QString& type, const QString& base64) const;

    /**
     * True if `base64` is valid base64 at all, read as decode() reads it:
     * space, tab, LF, form feed and CR skipped, any other character outside
     * the alphabet refused.
     *
     * Qt's own decoder discards invalid characters rather than failing, so a
     * typo yields plausible-looking garbage instead of a complaint. This is
     * the check that gives the caller a real answer.
     */
    Q_INVOKABLE bool looksLikeBase64(const QString& base64) const;
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(XdrQml)

#endif // QSTELLAR_XDRQML_H
