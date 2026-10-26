#ifndef PAGE_H
#define PAGE_H

#include "response.h"
#include "link.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

namespace PageAttach
{
  /**
   * Links connected to page response.
   */
class Links {
    Q_GADGET
    Q_PROPERTY(Link next MEMBER m_next)
    Q_PROPERTY(Link prev MEMBER m_prev)
    Q_PROPERTY(Link self MEMBER m_self)
    Link m_next;
    Link m_prev;
    Link m_self;
public:
    Link getNext() {
        return m_next;
    }

    Link getPrev() {
        return m_prev;
    }

    Link getSelf() {
        return m_self;
    }

};

}


/**
 * Represents page of objects.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/page.html" target="_blank">Page documentation</a>
 */
class PageBase : public Response
{
    Q_OBJECT
    Q_PROPERTY(QList<Response*> records READ getRecords WRITE setRecords)
    Q_PROPERTY(PageAttach::Links links READ getLinks WRITE setLinks)

    int m_responseType;
    PageAttach::Links m_links;
protected:
    QList<Response*> m_records;
public:
    PageBase(QNetworkReply *reply=nullptr);
    virtual ~PageBase();

    template<class T>
    void setType(){
        m_responseType = qMetaTypeId<T*>();
    }
    virtual void processRecords(const QJsonArray& records );

    QList<Response*> getRecords() const
    {
        return m_records;
    }
    int size() const
    {
        return m_records.size();
    }
    PageAttach::Links getLinks() const
    {
        return m_links;
    }
    /**
     * Requests the page _links.next names, into this same object, and
     * returns this: wait for ready() or error() as for the first page (the
     * records and links are replaced when it arrives). js's next() and py's
     * next() follow the same link. Returns null when the page has no next
     * link, and when it has no reply to ask through: a page read with
     * loadFromJson(), or one whose reply or QNetworkAccessManager was
     * deleted (that used to crash).
     */
    PageBase* getNextPage();
    /**
     * The same for _links.prev, as js's prev() and py's prev(). Horizon's
     * prev link asks for the records before this page in the other order,
     * and they come in that order, as in js and py.
     */
    PageBase* getPrevPage();
    /**
     * Reads the page exactly (see Response::fillObject): a record, its
     * `_embedded.records` or its `_links` that cannot be read throws
     * std::runtime_error naming the member, and the page keeps none of the
     * records it had read by then.
     */
    virtual void loadFromJson(QByteArray data);
protected:
    /** Record `index` of `records`, which must be a JSON object; throws otherwise. */
    QJsonObject recordObject(const QJsonArray& records, int index) const;
    /** How errors name record `index` ("_embedded.records[3]"; nothing in a stream). */
    QString recordField(int index) const;
    virtual void reset();
private:
    /** getNextPage() and getPrevPage(), following `href`. */
    PageBase* followLink(const QString& href);

public slots:
    void setRecords(QList<Response*> records);
    void setLinks(PageAttach::Links links);
};
//Base, QT MOC cant handle templates, so we make the thing in two classes
template<class T>
class Page : public PageBase
{
public:
    Page(QNetworkReply * reply = nullptr):PageBase(reply)
    {
        setType<T>();
    }
    virtual ~Page(){

    }
    /**
     * @brief get instead of getRecords().get(index) you can call straight get
     * @param index
     * @return reference to the record
     */
    T& get(size_t index)
    {
        return static_cast<T&>(*this->m_records[static_cast<int>(index)]);
    }
    /**
     * @brief at    same as get but returns a pointer.
     * @param index
     * @return pointer to record
     */
    T* at(size_t index)
    {
        return static_cast<T*>(this->m_records[static_cast<int>(index)]);
    }
    /**
     * @brief streamedElement We use pages to also get streamed elements
     * @return last updated streamed element, or nullptr before the first
     * one and after an event that could not be read (it indexed an empty
     * list). The record is the page's, valid until the next event or
     * reconnect.
     */
    T* streamedElement()
    {
        return this->m_records.isEmpty() ? nullptr : static_cast<T*>(this->m_records[0]);
    }
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(PageBase)
QSTELLAR_ALIAS(Page)
QSTELLAR_NS_ALIAS(PageAttach)




#endif // PAGE_H



//public class Page<T> extends Response {
//  @SerializedName("records")
//  private ArrayList<T> records;
//  @SerializedName("links")
//  private Links links;

//  Page() {}

//  public ArrayList<T> getRecords() {
//    return records;
//  }

//  public Links getLinks() {
//    return links;
//  }

//  /**
//   * @return The next page of results or null when there is no more results
//   * @throws URISyntaxException
//   * @throws IOException
//   */
//  public Page<T> getNextPage() throws URISyntaxException, IOException {
//    if (this.getLinks().getNext() == null) {
//      return null;
//    }
//    TypeToken type = new TypeToken<Page<T>>() {};
//    ResponseHandler<Page<T>> responseHandler = new ResponseHandler<Page<T>>(type);
//    URI uri = new URI(this.getLinks().getNext().getHref());
//    return (Page<T>) Request.Get(uri).execute().handleResponse(responseHandler);
//  }

//  /**
//   * Links connected to page response.
//   */
//  public static class Links {
//    @SerializedName("next")
//    private final Link next;
//    @SerializedName("prev")
//    private final Link prev;
//    @SerializedName("self")
//    private final Link self;

//    Links(Link next, Link prev, Link self) {
//      this.next = next;
//      this.prev = prev;
//      this.self = self;
//    }

//    public Link getNext() {
//      return next;
//    }

//    public Link getPrev() {
//      return prev;
//    }

//    public Link getSelf() {
//      return self;
//    }
//  }
//}
