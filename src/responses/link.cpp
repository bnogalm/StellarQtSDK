#include "link.h"
#include <stdexcept>

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
    QUrl uri(m_href);
    if(!uri.isValid())
        throw std::runtime_error("invalid uri");
    return uri;
}

bool Link::operator !=(const Link &link) const
{
    return (m_href!=link.m_href)
            || (m_templated != link.m_templated);
}

bool Link::operator ==(const Link &link) const
{
    return (m_href==link.m_href)
            && (m_templated == link.m_templated);
}

