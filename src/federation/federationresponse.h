#ifndef FEDERATIONRESPONSE_H
#define FEDERATIONRESPONSE_H

#include <QObject>
#include "../responses/response.h"
#include "../qstellar_namespace.h"
QSTELLAR_BEGIN_NS


/**
 * Object to hold a response from a federation server.
 * @see <a href="https://www.stellar.org/developers/learn/concepts/federation.html" target="_blank">Federation docs</a>
 */
/**
 * A SEP-2 answer: stellar_address, account_id, memo_type and memo through
 * their getters, checked (an account id, a memo that fits its type). Any
 * other member the server sends (SEP-2 lets servers add their own) is kept:
 * json() returns the whole object, as js and py hand the record back.
 */
class FederationResponse : public Response
{
    Q_OBJECT

    Q_PROPERTY(QString stellar_address READ getStellarAddress MEMBER m_stellarAddress)
    Q_PROPERTY(QString account_id READ getAccountId MEMBER m_accountId)
    Q_PROPERTY(QString memo_type READ getMemoType MEMBER m_memoType)//text id hash return?
    Q_PROPERTY(QString memo READ getMemo MEMBER m_memo)

    QString m_stellarAddress;
    QString m_accountId;
    QString m_memoType;
    QString m_memo;
    // The account a reverse lookup (type=id) asked for; empty for the others.
    QString m_reverseOf;
    friend class FederationServer;

public:

    FederationResponse(QNetworkReply* reply=0);

    /** The largest federation answer read, in bytes (js: FEDERATION_RESPONSE_MAX_SIZE). */
    static constexpr qint64 MaxSize = 100 * 1024;
    /**
     * Stops `reply` once more than MaxSize bytes are announced or have
     * arrived, and fails this response saying so. Done by the constructor
     * taking a reply; call it after loadFromReply() with another.
     */
    void limitSize(QNetworkReply* reply);
    FederationResponse(QString stellarAddress, QString accountId, QString memoType, QString memo);
    QString getStellarAddress();

    QString getAccountId();

    /**
     * @return Memo type or null when no memo attached
     */
    QString getMemoType();

    /**
     * @return Memo value or null when no memo attached
     */
    QString getMemo();

    /**
     * Reads a federation server's answer, and refuses one that cannot be paid
     * as given (the fields stay empty and error() is emitted): an account_id
     * that is not a G or M address, a memo_type other than text, id or hash,
     * a memo that is not valid for it (text of more than 28 bytes of UTF-8, an
     * id that is not a uint64 in decimal, a hash that is not 32 bytes of
     * base64), a memo without its type or a type without its memo, and any of
     * them given as something other than a JSON string. An empty text memo is
     * a memo, as core, js and py take it. The answer to a reverse lookup
     * (FederationServer::resolveAccountId) also needs its stellar_address,
     * and an account_id, when it has one, that is the one asked for. The
     * answer used to be passed through unread, and a wallet paid whatever it
     * said.
     */
    void loadFromJson(QByteArray data) override;

    /**
     * Ends the lookup with `reason`: before the federation server was asked
     * (the domain's stellar.toml failed), or when its answer passes MaxSize
     * while it arrives (limitSize()). error() is emitted and lastException()
     * is a FederationException saying why. It used to be a bare error() with
     * no reason anywhere.
     */
    void fail(const QString& reason);
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(FederationResponse)

#endif // FEDERATIONRESPONSE_H
