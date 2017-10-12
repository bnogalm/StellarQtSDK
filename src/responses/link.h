#ifndef LINK_H
#define LINK_H

#include <QObject>
#include <QUrl>

/**
 * Represents links in responses.
 */
class Link
{
    Q_GADGET
    Q_PROPERTY(QString href MEMBER m_href)
    Q_PROPERTY(bool templated MEMBER m_templated)
    QString m_href;
    bool m_templated;

public:
    explicit Link();

    QString getHref() const;
    bool getTemplated() const;
    QUrl getUri();
    bool operator !=(Link& link)
    {
        Q_UNUSED(link)
        return true;
    }
};
Q_DECLARE_METATYPE(Link)
#endif // LINK_H




//public class Link {
//  @SerializedName("href")
//  private final String href;
//  @SerializedName("templated")
//  private final boolean templated;

//  Link(String href, boolean templated) {
//    this.href = href;
//    this.templated = templated;
//  }

//  public String getHref() {
//    // TODO templated
//    return href;
//  }

//  public URI getUri() {
//    // TODO templated
//    try {
//      return new URI(href);
//    } catch (URISyntaxException e) {
//      throw new RuntimeException(e);
//    }
//  }

//  public boolean isTemplated() {
//    return templated;
//  }
//}
