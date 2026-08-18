#ifndef CLAIMANT_H
#define CLAIMANT_H
#include <QtCore>
#include "predicate.h"


#include "strkey.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

class Claimant
{
    Q_GADGET
public:
    Q_PROPERTY(QString destination READ getDestination WRITE setDestination )
    Q_PROPERTY(Predicate predicate READ getPredicate WRITE setPredicate )

    Claimant(QString destination, Predicate* predicate);
    Claimant();
    Claimant(const Claimant& other);
    /** Rule of three. With a copy ctor and destructor of our own but no
     *  assignment operator, the implicit one copied m_predicate and both
     *  objects deleted it: double-free. And Claimant is a Q_GADGET that lives
     *  in QList<Claimant>, where assigning is routine. */
    Claimant& operator=(const Claimant& other);
    ~Claimant();
    QString getDestination() const;
    const Predicate &getPredicate() const;

    stellar::Claimant toXdr() const;

    static Claimant fromXdr(stellar::Claimant c);

    bool operator==(const Claimant& other) const;
    bool operator!=(const Claimant& other) const;
public slots:
    void setDestination(QString destination);
    void setPredicate(Predicate& predicate);


private:
    QString m_destination;
    Predicate* m_predicate;
};
namespace ClaimantConverter
{
    // Defined (non-inline) in claimant.cpp, where it is registered as a
    // QVariantList->QList<Claimant> converter.
    QList<Claimant> convertData(const QVariantList &source);
}

QList<Claimant> checkNotNull(QList<Claimant> claimants, const char *error);

QSTELLAR_END_NS

QSTELLAR_ALIAS(Claimant)
QSTELLAR_NS_ALIAS(ClaimantConverter)

#endif // CLAIMANT_H
