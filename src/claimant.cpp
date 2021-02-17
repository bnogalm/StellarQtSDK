#include "claimant.h"
#include <QCoreApplication>
#include "responses/response.h"
static void registerTypes()
{    
    QMetaType::registerConverter<QVariantList ,QList<Claimant> >(&ClaimantConverter::convertData);
}

Q_COREAPP_STARTUP_FUNCTION(registerTypes)
Claimant::Claimant(QString destination, Predicate *predicate): m_destination(destination), m_predicate(predicate)
{

}

Claimant::Claimant():m_predicate(nullptr)
{

}

Claimant::Claimant(const Claimant &other)
{
    this->m_destination=other.m_destination;
    this->m_predicate=Predicate::create(*(other.m_predicate));
}

Claimant::~Claimant(){
    if(m_predicate)
        delete m_predicate;
}

QString Claimant::getDestination() const
{
    return m_destination;
}

const Predicate& Claimant::getPredicate() const
{
    return *m_predicate;
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

void Claimant::setPredicate(Predicate &predicate)
{
    if (m_predicate && m_predicate->equals(&predicate))
        return;

    m_predicate = Predicate::create(predicate);
}

QList<Claimant> ClaimantConverter::convertData(const QVariantList &source)
{
    QList<Claimant> res;
    for(int n = 0;n<source.count();n++){

        QVariantMap m = source[n].toMap();
        Claimant r(m.value("destination").toString(), Predicate::create(m.value("predicate")));
        res.append(r);

    }
    return res;
}
