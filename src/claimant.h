#ifndef CLAIMANT_H
#define CLAIMANT_H
#include <QtCore>
#include "predicate.h"


#include "strkey.h"


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

    stellar::Claimant toXdr() const;

    static Claimant fromXdr(stellar::Claimant c);

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

QList<Claimant> checkNotNull(QList<Claimant> claimants, const char *error);
#endif // CLAIMANT_H
