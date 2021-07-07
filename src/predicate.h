#ifndef PREDICATE_H
#define PREDICATE_H
#include <QtCore>
#include "xdr/stellarledgerentries.h"
class Predicate;

class Predicate
{
protected:
    static QList<Predicate*> convertXDRPredicates(QVector<stellar::ClaimPredicate> predicates) {
        QList<Predicate*> list;
        for (stellar::ClaimPredicate& p : predicates) {
            list.append(fromXdr(p));
        }
        return list;
    }



public:
    /**
      * Generates Predicate object from a given XDR object
      * @param xdr XDR object
      */
    static Predicate* fromXdr(stellar::ClaimPredicate& xdr);
    static Predicate* create(Predicate &predicate);
    static Predicate* create(const QVariant &predicate);
    virtual ~Predicate();;
    virtual bool equals(Predicate* object) const = 0;

    virtual int hashCode() const = 0;
    /**
      * Generates XDR object from a given Asset object
     */
    virtual stellar::ClaimPredicate toXdr() const = 0;

    class Unconditional;
    class Not;
    class Or;
    class And;
    class AbsBefore;
    class RelBefore;



};

class Predicate::Unconditional :  public Predicate {
    Q_GADGET
public:
    Unconditional(){};
    virtual ~Unconditional(){};

    Unconditional(const QVariantMap& data)
    {
        Q_UNUSED(data)
    }
    bool equals(Predicate* o) const
    {
        if (this == o) {
            return true;
        }
        return (dynamic_cast<Predicate::Unconditional*>(o));
    }

    int hashCode() const
    {
        return 0;
    }


    stellar::ClaimPredicate toXdr() const  {
        stellar::ClaimPredicate xdr;
        xdr.type = stellar::ClaimPredicateType::CLAIM_PREDICATE_UNCONDITIONAL;
        return xdr;
    }
};


class Predicate::Not : public Predicate {
    Q_GADGET
    Predicate* m_inner;
public:
    Not(Predicate* inner): m_inner(inner)
    {
    }
    Not():m_inner(nullptr)
    {

    }
    Not(const Not& other)
    {
        m_inner = Predicate::create(*(other.m_inner));
    }
    virtual ~Not()
    {
        if(m_inner)
            delete m_inner;
    };

    Predicate& getInner() const
    {
        return *m_inner;
    }



    bool equals(Predicate* o) const
    {
        if (this == o) {
            return true;
        }
        auto oc = dynamic_cast<Predicate::Not*>(o);
        if(oc)
        {
            return this->m_inner->equals(oc->m_inner);
        }
        return false;
    }
    int hashCode() const
    {
        return m_inner->hashCode()^0xaa;
    }

    stellar::ClaimPredicate toXdr()  const {
        stellar::ClaimPredicate xdr;
        stellar::Array<stellar::ClaimPredicate,1>& predicate= xdr.fillNotPredicate();
        predicate.append(m_inner->toXdr());        
        return xdr;
    }
};

class Predicate::Or : public Predicate {
    Q_GADGET
    QList<Predicate*> m_inner;
public:
    Or(QList<Predicate*> inner): m_inner(inner)
    {

    }
    Or(const Or& other)
    {
        for(auto i :other.m_inner)
            m_inner.append(Predicate::create(*i));
    }
    Or()
    {

    }
    virtual ~Or(){
        for(auto i :m_inner)
            delete i;
    };

    QList<Predicate*> getInner() const{
        return m_inner;
    }

    Predicate& get(int i) const{
        return *m_inner.at(i);
    }

    bool equals(Predicate* o) const
    {
        if (this == o) {
            return true;
        }
        auto oc = dynamic_cast<Predicate::Or*>(o);
        if(oc)
        {

            for(auto& i : m_inner)
            {
                bool ocontainsi = false;
                for(auto& io : oc->m_inner)
                {
                    if(i->equals(io))
                    {
                        ocontainsi=true;
                        break;
                    }
                }
                if(!ocontainsi)
                    return false;
            }
            return true;
        }
        return false;
    }
    int hashCode() const
    {
        return qHash(m_inner)^0xbb;
    }

    stellar::ClaimPredicate toXdr()  const {
        stellar::ClaimPredicate xdr;
        stellar::Array<stellar::ClaimPredicate,2>& predicates = xdr.fillOrPredicates();
        for (int i = 0; i < m_inner.size(); i++) {
            predicates.append(m_inner[i]->toXdr());
        }        
        return xdr;
    }
};


class Predicate::And :public Predicate {
    Q_GADGET
    QList<Predicate*> m_inner;
public:
    And(QList<Predicate*> inner):m_inner(inner)
    {
    }
    And()
    {

    }
    And(const And& other)
    {
        for(auto i :other.m_inner)
            m_inner.append(Predicate::create(*i));
    }
    virtual ~And(){
        for(auto i :m_inner)
            delete i;
    };

    QList<Predicate*> getInner() const{
        return m_inner;
    }

    Predicate& get(int i) const{
        return *m_inner.at(i);
    }


    bool equals(Predicate* o) const
    {
        if (this == o) {
            return true;
        }
        auto oc = dynamic_cast<Predicate::And*>(o);
        if(oc)
        {

            for(auto& i : m_inner)
            {
                bool ocontainsi = false;
                for(auto& io : oc->m_inner)
                {
                    if(i->equals(io))
                    {
                        ocontainsi=true;
                        break;
                    }
                }
                if(!ocontainsi)
                    return false;
            }
            return true;
        }
        return false;
    }

    int hashCode() const
    {
        return qHash(m_inner)^0xcc;
    }

    stellar::ClaimPredicate toXdr()  const {
        stellar::ClaimPredicate xdr;
        stellar::Array<stellar::ClaimPredicate,2>& predicates= xdr.fillAndPredicates();
        for (int i = 0; i < m_inner.size(); i++) {
            predicates.append(m_inner[i]->toXdr());
        }
        return xdr;
    }
};

class Predicate::AbsBefore :public Predicate {
    Q_GADGET
    qint64 m_epochSeconds;
public:
    AbsBefore(qint64 epochSeconds):m_epochSeconds(epochSeconds)
    {
    }
    AbsBefore():m_epochSeconds(0)
    {

    }
    virtual ~AbsBefore(){}

    qint64 getTimestampSeconds()
    {
        return m_epochSeconds;
    }

    QDateTime getDate() {
        return QDateTime::fromSecsSinceEpoch(m_epochSeconds,Qt::UTC);
    }

    bool equals(Predicate* o) const
    {
        if (this == o) {
            return true;
        }
        auto oc = dynamic_cast<Predicate::AbsBefore*>(o);
        if(oc)
        {
            return this->m_epochSeconds == oc->m_epochSeconds;
        }
        return false;
    }
    int hashCode() const
    {
        return qHash(m_epochSeconds)^0xdd;
    }


    stellar::ClaimPredicate toXdr()  const {
        stellar::ClaimPredicate xdr;
        xdr.setAbsBefore(m_epochSeconds);
        return xdr;
    }
};

class Predicate::RelBefore :public Predicate {
    Q_GADGET
    qint64 m_secondsSinceClose;
public:
    RelBefore(qint64 secondsSinceClose):m_secondsSinceClose(secondsSinceClose)
    {
    }
    RelBefore():m_secondsSinceClose(0)
    {

    }
    virtual ~RelBefore(){}

    qint64 getSecondsSinceClose() {
        return m_secondsSinceClose;
    }

    bool equals(Predicate* o) const
    {
        if (this == o) {
            return true;
        }
        auto oc = dynamic_cast<Predicate::RelBefore*>(o);
        if(oc)
        {
            return this->m_secondsSinceClose == oc->m_secondsSinceClose;
        }
        return false;
    }
    int hashCode() const
    {
        return qHash(m_secondsSinceClose)^0xee;
    }



    stellar::ClaimPredicate toXdr()  const {
        stellar::ClaimPredicate xdr;
        xdr.setRelBefore(m_secondsSinceClose);
        return xdr;
    }
};

Q_DECLARE_METATYPE(Predicate::AbsBefore);
Q_DECLARE_METATYPE(Predicate::And);
Q_DECLARE_METATYPE(Predicate::Not);
Q_DECLARE_METATYPE(Predicate::Or);
Q_DECLARE_METATYPE(Predicate::RelBefore);
Q_DECLARE_METATYPE(Predicate::Unconditional);

#endif // PREDICATE_H
