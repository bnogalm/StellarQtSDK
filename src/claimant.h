#ifndef CLAIMANT_H
#define CLAIMANT_H
#include <QtCore>
#include "predicate.h"

class Claimant
{
    Q_GADGET
public:
    Q_PROPERTY(QString destination READ destination WRITE setDestination )
    Q_PROPERTY(Predicate* predicate READ predicate WRITE setPredicate )

    Claimant(QString destination, Predicate* predicate);
    QString destination() const;
    Predicate* predicate() const;

    bool operator==(const Claimant& other);
    bool operator!=(const Claimant& other);
public slots:
    void setDestination(QString destination);
    void setPredicate(Predicate* predicate);


private:
    QString m_destination;
    Predicate* m_predicate;
};

#endif // CLAIMANT_H
