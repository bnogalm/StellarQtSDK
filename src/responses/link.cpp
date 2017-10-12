#include "link.h"

Link::Link():m_templated(false)
{

}

QString Link::getHref() const
{
    return m_href;
}

bool Link::getTemplated() const
{
    return m_templated;
}

QUrl Link::getUri() {
    // TODO templated
    return QUrl(m_href);
}

