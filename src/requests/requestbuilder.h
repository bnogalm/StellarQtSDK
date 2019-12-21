#ifndef REQUESTBUILDER_H
#define REQUESTBUILDER_H
#include <QtGlobal>
#include <QString>
#include <QNetworkRequest>
#include <QUrlQuery>
#include <utility>
#include <QVector>


class Server;
class Asset;

class RequestBuilder
{
    QUrl m_uri;//origin of the uri
    QStringList m_segments;
    QVector<std::pair<QString,QString> > m_queries;
    bool m_segmentsAdded;
    Server* m_server;//server that created this Request
    QUrl m_requestUri;//final uri used to request the server
    bool m_streamMode;
public:
    /**
     * Represents possible <code>order</code> parameter values.
     */
    enum class Order {
      ASC//("asc"),
      ,DESC//("desc");
   };
    RequestBuilder(Server *server, QString defaultSegment = QString());
    Server * server();
    bool streamMode();

protected:
    void setStreamMode();

    RequestBuilder& setSegments(QString segment);

    RequestBuilder& setSegments(QStringList segments);

    void addParameter(QString key, QString value);

    QString parameter(QString key) const;
public:
     /**
      * Sets <code>cursor</code> parameter on the request.
      * A cursor is a value that points to a specific location in a collection of resources.
      * The cursor attribute itself is an opaque value meaning that users should not try to parse it.
      * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/page.html">Page documentation</a>
      * @param cursor
      */
     RequestBuilder& cursor(QString cursor);

     /**
      * Sets <code>limit</code> parameter on the request.
      * It defines maximum number of records to return.
      * For range and default values check documentation of the endpoint requested.
      * @param number maxium number of records to return
      */
     RequestBuilder& limit(int number);

     /**
      * Sets <code>order</code> parameter on the request.
      * @param direction {@link org.stellar.sdk.requests.RequestBuilder.Order}
      */
     RequestBuilder& order(RequestBuilder::Order direction);
     RequestBuilder& asc();
     RequestBuilder& desc();

     /**
      * Sets a parameter consisting of a comma separated list of assets on the request.
      * @param name the name of the query parameter
      * @param assets the list of assets to be serialized into the query parameter value
      */
     RequestBuilder& setAssetsParameter(QString name, QList<Asset*> assets);

     QUrl buildUri();
     void setRequestUri(QUrl &uri);//useful for the links
     QUrl &requestUri();


     QString toString(Order order);
     Order orderFromString(QString s);
};


#endif // REQUESTBUILDER_H
