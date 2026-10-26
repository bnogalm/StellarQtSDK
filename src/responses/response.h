#ifndef RESPONSE_H
#define RESPONSE_H
#include <QObject>
#include <QNetworkReply>
#include <QMetaProperty>
#include <QVariantMap>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <memory>
#include "qstellar_namespace.h"

namespace qstellar { namespace exception { class SdkException; } }

QSTELLAR_BEGIN_NS

class Price;
class Predicate;

namespace ResponseConverters{
template<class T>
inline QList<T> convert(QVariantList source)
{
    QList<T> res;
    for(int n = 0;n<source.count();n++){
        T r;
        QVariantMap m = source[n].toMap();
        for(auto& key : m.keys())
        {
            int index = T::staticMetaObject.indexOfProperty(key.toLatin1().data());
            if(index>=T::staticMetaObject.propertyOffset()){
                QMetaProperty p = T::staticMetaObject.property(index);
                p.writeOnGadget(&r,m.value(QString(p.name())));
            }
        }
        res.append(r);

    }
    return res;
}
template<class T>
inline QList<T> convertPlain(QVariantList source)
{
    QList<T> res;
    for(int n = 0;n<source.count();n++){
        res.append(source[n].value<T>());
    }
    return res;
}

/**
 * Reads the JSON value of a property exactly, as a QVariant of the
 * property's own type, or throws std::runtime_error naming `field`. Never
 * given null: Response::fillObject handles null itself.
 */
typedef QVariant (*ExactReader)(const QJsonValue& value, const QString& field);

/**
 * Makes Response::fillObject read properties of type `metaTypeId` with
 * `reader`. For the types its own rules cannot build exactly: lists of
 * gadgets, which the converters above filled without checking a single value
 * (a limit given as a number, a weight given as text, all became 0 or
 * empty), and types with a JSON form of their own.
 */
void registerExactReader(int metaTypeId, ExactReader reader);

/**
 * Fills `gadget`, an instance of `mo`, from `value`, which must be a JSON
 * object, by the rules of Response::fillObject; throws std::runtime_error
 * naming `field` otherwise.
 */
void fillGadget(const QMetaObject* mo, void* gadget, const QJsonValue& value, const QString& field);

/**
 * The whole number `value` holds, in [min, max], by fillObject's rules (a
 * whole JSON number below 2^53 in magnitude, or decimal text); throws
 * std::runtime_error naming `field` otherwise, or when it is missing.
 */
qint64 exactInteger(const QJsonValue& value, const QString& field, qint64 min, qint64 max);

/** A JSON array of T (a Q_GADGET), each element read by fillGadget. */
template<class T>
QVariant exactGadgetList(const QJsonValue& value, const QString& field)
{
    QList<T> list;
    if (!value.isArray())
        fillGadget(nullptr, nullptr, value, field); // throws: not an array
    const QJsonArray array = value.toArray();
    for (int i = 0; i < array.size(); ++i) {
        T item;
        fillGadget(&T::staticMetaObject, &item, array.at(i),
                   field + QLatin1Char('[') + QString::number(i) + QLatin1Char(']'));
        list.append(item);
    }
    return QVariant::fromValue(list);
}

/** Registers exactGadgetList<T> for QList<T> properties. */
template<class T>
void registerExactGadgetList()
{
    registerExactReader(qMetaTypeId<QList<T> >(), &exactGadgetList<T>);
}

/**
 * The price Horizon writes as {"n": .., "d": ..} with int32 terms (an
 * offer's price_r, an order book row's, min_price_r / max_price_r), as a
 * Price, exactly. Throws std::runtime_error naming `field` when a term is
 * missing, is not a whole number (a JSON number, or decimal text), is
 * outside int32, or the denominator is not positive: the getters read the
 * terms with toInt(), which made each of these 0, or another price.
 */
Price exactPrice(const QVariantMap& rational, const char* field);

/**
 * A claim predicate as Horizon writes it, read exactly by
 * Predicate::fromJson (see there for the rules). The caller owns the
 * result. Throws std::runtime_error naming `field` for anything else.
 */
Predicate* exactPredicate(const QJsonValue& value, const QString& field);
}

class ResponseReader;


class Response : public QObject
{
    Q_OBJECT    
    Q_PROPERTY(int rateLimitLimit READ getRateLimitLimit NOTIFY rateLimitChanged)
    Q_PROPERTY(int rateLimitRemaining READ getRateLimitRemaining NOTIFY rateLimitChanged)
    Q_PROPERTY(int rateLimitReset READ getRateLimitReset NOTIFY rateLimitChanged)
    Q_PROPERTY(QString type MEMBER m_type NOTIFY typeChanged)
    Q_PROPERTY(QString title MEMBER m_title NOTIFY titleChanged)
    Q_PROPERTY(int status MEMBER m_status READ getStatus NOTIFY statusChanged)
    Q_PROPERTY(QString instance MEMBER m_instance NOTIFY instanceChanged)
    Q_PROPERTY(QString detail MEMBER m_detail NOTIFY detailChanged)

    QNetworkReply::NetworkError m_lastErrorCode;
    QString m_type;
    QString m_title;
    int m_status;
    QString m_instance;

    QString m_detail;

    QByteArray m_pendingData;

    int m_timeoutTimerID;//timeout request
    int m_reconnectTimerID;//used for streamed request
    int m_retryTime;//for streamed responses SSE
    // A 429's Retry-After in milliseconds, for the next reconnect only; 0
    // when there is none.
    int m_retryAfter;
    QByteArray m_lastID;//last id received

    // Last typed exception, set just before emit error(). Cleared on each
    // successful loadFromReply / reset().
    std::unique_ptr<qstellar::exception::SdkException> m_lastException;

    // Set when this object's own timer gave up on m_reply, so the failure
    // the abort produces is reported as a timeout, not a connection error.
    bool m_timedOut;

    // Where m_reply is: none attached, sent with nothing of it handled yet,
    // a stream answered with an event stream, or its outcome delivered.
    enum ReplyState { ReplyNone, ReplyWaiting, ReplyStreaming, ReplyDone };
    ReplyState m_replyState;

    friend class ResponseReader;
protected:
    QNetworkReply * m_reply;
    int m_rateLimitLimit;
    int m_rateLimitRemaining;
    int m_rateLimitReset;

    /**
     * Writes each member of `jsonObj` that names a property of `mo` into
     * `obj` (a QObject when `qobject`, a gadget otherwise), exactly, or
     * throws std::runtime_error naming the member. Before 1.0.4 a value that
     * did not convert (text or a fraction into an integer, a number past the
     * property's range, an object where text was expected, an integer past
     * 2^53 given as a JSON number, ...) silently left the property at 0 or
     * empty, or at a truncated value. Now:
     *  - text properties take JSON strings; integers take a whole JSON number
     *    within the property's range and at most 2^53 in magnitude, or
     *    decimal text; booleans take JSON booleans; lists take arrays and
     *    gadgets objects, read by the same rules;
     *  - null writes the property's default, except for a member Horizon
     *    always sends, which is refused like one that is missing: the
     *    account's sequence, balances, thresholds, flags and signers, an
     *    amount, a price's terms, a paging token, ...;
     *  - members that name no property are ignored, as before.
     */
    void fillObject(const QMetaObject *mo, void *obj, const QJsonObject &jsonObj, bool qobject = false);
    /** The same, naming refused members from `path` (e.g. "_embedded.records[3]"). */
    void fillObject(const QMetaObject *mo, void *obj, const QJsonObject &jsonObj, bool qobject,
                    const QString& path);

    /**
     * Reads member `key` of the record into this object when its property
     * cannot be read by fillObject's rules (a pointer, or a JSON form of its
     * own); returns false to leave it to them. Throws std::runtime_error
     * naming `field` for a value it cannot read exactly. The default reads
     * none.
     */
    virtual bool readMember(const QString& key, const QJsonValue& value, const QString& field);

    /**
     * Called with the JSON body of an HTTP error answer (4xx other than 429,
     * 5xx) before error() is emitted, so a response can keep what the body
     * says about itself. The exception still carries the whole Problem. The
     * default reads nothing; a throw is ignored.
     */
    virtual void readFailureBody(int httpStatus, const QJsonObject& body);

    virtual void reset();
    bool isStreamingResponse() const;
    /** Stores `ex` (takes ownership) and emits error(). */
    void emitError(qstellar::exception::SdkException* ex);
public:
    explicit Response(QNetworkReply* reply=nullptr);

    virtual ~Response();
    void loadFromReply(QNetworkReply *reply);
    /**
     * Fills this response from a JSON document, exactly (see fillObject);
     * throws std::runtime_error naming the member that could not be read.
     * A reply whose body throws here emits error() with an
     * UnknownResponseException carrying that message; in a stream, the event
     * is dropped with error(), and the events after it are still read.
     */
    virtual void loadFromJson(QByteArray data);

    /**
     * How long, in milliseconds, a request may go without finishing (for a
     * stream: without receiving event-stream data; an HTTP error answering
     * it has that long from the request) before the SDK aborts it:
     * STELLAR_QT_REPLY_TIMEOUT unless set. A request aborted so emits error()
     * with a ClientTimeoutException, which is a ConnectionErrorException; it
     * was a plain ConnectionErrorException before 1.0.4. For a submission it
     * means the transaction may still be applied, like Horizon's 504.
     */
    static int replyTimeout();
    /** Sets replyTimeout() for requests started afterwards; 0 or less restores the default. */
    static void setReplyTimeout(int milliseconds);

    /*
     * A stream (a request builder's stream()) reconnects whenever its
     * connection ends, after RECONNECT_DELAY (1 s) or the delay the stream
     * set with an SSE retry: field. After a 429 it waits the answer's
     * Retry-After (seconds) when that is longer, for that reconnect only;
     * it was read as milliseconds, so a stream told to wait 30 s came back
     * after 1 s, and a rate-limited stream kept asking once a second.
     *
     * A connection that fails emits error() once, and the stream reconnects
     * as above: refused, a host that does not resolve, TLS, no answer within
     * replyTimeout() (lastException() a ConnectionErrorException or a
     * ClientTimeoutException), an HTTP error (the exception of its status,
     * read once the whole answer is in; one whose body is still coming
     * after replyTimeout() is reported without it), a 2xx that is not
     * text/event-stream (an UnknownResponseException; that connection is
     * closed then, however much more it sends). js's EventSource
     * reports each of these the same way. Before 1.0.4 a connection that
     * failed before any byte arrived (and an HTTP error with no body) was
     * retried every second with no error(), forever, and an HTTP error
     * emitted error() once per piece of its body. js stops after an HTTP
     * error until its own 15 s reconnect timeout, py ends the stream; this
     * one keeps the reconnect delay, so a 429's Retry-After is honoured.
     */
signals:
    void ready();
    void error();
    void rateLimitChanged();
    void typeChanged();
    void titleChanged();
    void statusChanged();
    void instanceChanged();
    void detailChanged();

private:
    QJsonObject m_json;
    bool preprocessResponse(QNetworkReply *response);
    void restartTimeoutTimer();
    /** One Server-Sent Event (the lines before a blank line). */
    void processStreamEvent(const QByteArray& event);
    /**
     * Reads the answer of a stream's connection, once: true when it is a 2xx
     * event stream; otherwise error() is emitted and false returned (also
     * when a receiver deleted this object).
     */
    bool openStream(QNetworkReply* reply);
private slots:
    void processResponse();    
    void processPartialResponse();
    void reconnectStream();
    void reconnectStreamDelayed();
    void streamFinished();
    void clearReply(QObject *obj);
public:
    /**
     * The JSON object this response (or this record of a page) was read
     * from, whole: every member the server sent, including those no getter
     * reads (a field a newer Horizon adds, a muxed id, ...), as js and py hand
     * the record back. Empty before it is read.
     */
    QJsonObject json() const { return m_json; }
    QNetworkReply::NetworkError lastErrorCode() const;
    /**
     * @brief Typed exception attached to the last error() emission, if any.
     * The Response retains ownership; do not delete the returned pointer.
     * Returns nullptr if no error has happened.
     */
    qstellar::exception::SdkException* lastException() const;
    int getStatus() const;
    /**
     * Returns X-RateLimit-Limit header from the response.
     * This number represents the he maximum number of requests that the current client can
     * make in one hour.
     * -1 when the header was not sent (Horizons that do not limit leave it
     * out), or is not a whole number that fits an int; before 1.0.4 either
     * read as 0. The same for the two below.
     * @see <a href="https://www.stellar.org/developers/horizon/learn/rate-limiting.html" target="_blank">Rate Limiting</a>
     */
    int getRateLimitLimit() const;

    /**
     * Returns X-RateLimit-Remaining header from the response.
     * The number of remaining requests for the current window; -1 when not
     * sent or unreadable (it read 0, as if no request were left).
     * @see <a href="https://www.stellar.org/developers/horizon/learn/rate-limiting.html" target="_blank">Rate Limiting</a>
     */
    int getRateLimitRemaining() const;

    /**
     * Returns X-RateLimit-Reset header from the response. Seconds until a new window starts;
     * -1 when not sent or unreadable.
     * @see <a href="https://www.stellar.org/developers/horizon/learn/rate-limiting.html" target="_blank">Rate Limiting</a>
     */
    int getRateLimitReset() const;

    // QObject interface
protected:
    void timerEvent(QTimerEvent *event);
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(Response)
QSTELLAR_NS_ALIAS(ResponseConverters)



Q_DECLARE_METATYPE(qstellar::Response*)
#endif // RESPONSE_H
