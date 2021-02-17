#ifndef CLAIMANT_H
#define CLAIMANT_H
#include <QtCore>
#include "predicate.h"




class Claimant
{
    Q_GADGET
public:
    Q_PROPERTY(QString destination READ getDestination WRITE setDestination )
    Q_PROPERTY(Predicate predicate READ getPredicate WRITE setPredicate )

    Claimant(QString destination, Predicate* predicate);
    Claimant();
    Claimant(const Claimant& other);
    ~Claimant();
    QString getDestination() const;
    const Predicate &getPredicate() const;

    bool operator==(const Claimant& other);
    bool operator!=(const Claimant& other);
public slots:
    void setDestination(QString destination);
    void setPredicate(Predicate& predicate);


private:
    QString m_destination;
    Predicate* m_predicate;
};
namespace ClaimantConverter
{
    inline QList<Claimant> convertData(const QVariantList &source);
}
#endif // CLAIMANT_H
