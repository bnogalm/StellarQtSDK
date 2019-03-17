#ifndef PAGE_H
#define PAGE_H

#include "response.h"
#include "link.h"

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
       * @return The next page of results or null when there is no more results
       * @throws URISyntaxException
       * @throws IOException
       */
    PageBase* getNextPage();
    virtual void loadFromJson(QByteArray data);
protected:
    virtual void reset();

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
     * @return last updated streamed element
     */
    T* streamedElement()
    {
        return static_cast<T*>(this->m_records[0]);
    }
};

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
