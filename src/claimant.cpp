#include "claimant.h"

Claimant::Claimant(QString destination, Predicate *predicate): m_destination(destination), m_predicate(predicate)
{

}

Claimant::~Claimant(){
    if(m_predicate)
        delete m_predicate;
}

QString Claimant::destination() const
{
    return m_destination;
}

Predicate *Claimant::predicate() const
{
    return m_predicate;
}

bool Claimant::operator==(const Claimant &other)
{
    return m_destination==other.m_destination && m_predicate && other.m_predicate && m_predicate->equals(other.m_predicate);
}

bool Claimant::operator!=(const Claimant &other)
{
    return m_destination!=other.m_destination || !m_predicate || !other.m_predicate || !m_predicate->equals(other.m_predicate);
}

void Claimant::setDestination(QString destination)
{
    if (m_destination == destination)
        return;

    m_destination = destination;
}

void Claimant::setPredicate(Predicate *predicate)
{
    if (m_predicate == predicate)
        return;

    m_predicate = predicate;
}
