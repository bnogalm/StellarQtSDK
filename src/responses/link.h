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
    bool operator !=(Link& link);
    bool operator ==(Link& link);
};
Q_DECLARE_METATYPE(Link)
#endif // LINK_H
