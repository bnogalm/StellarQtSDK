#ifndef LINK_H
#define LINK_H

#include <QObject>
#include <QUrl>
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

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
    bool operator !=(const Link& link) const;
    bool operator ==(const Link& link) const;
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(Link)



Q_DECLARE_METATYPE(qstellar::Link)
#endif // LINK_H
