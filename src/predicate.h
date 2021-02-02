#ifndef PREDICATE_H
#define PREDICATE_H
#include <QtCore>
#include "xdr/stellarledgerentries.h"
class Predicate;

class Predicate
{
protected:
    static QList<Predicate*> convertXDRPredicates(stellar::Array<stellar::ClaimPredicate> predicates) {
        QList<Predicate*> list;
        for (stellar::ClaimPredicate& p : predicates.value) {
            list.append(fromXdr(p));
        }
        return list;
    }
    /**
      * Generates Predicate object from a given XDR object
      * @param xdr XDR object
      */
    static Predicate* fromXdr(stellar::ClaimPredicate& xdr);


public:
    virtual ~Predicate(){};
    virtual bool equals(Predicate* object) const =0;

    virtual int hashCode() const = 0;
    /**
      * Generates XDR object from a given Asset object
     */
    virtual stellar::ClaimPredicate toXdr() = 0;

    class Unconditional;
    class Not;
    class Or;
    class And;
    class AbsBefore;
    class RelBefore;



};

class Predicate::Unconditional :  public Predicate {

public:
    Unconditional(){};
    virtual ~Unconditional(){};
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


    stellar::ClaimPredicate toXdr() {
        stellar::ClaimPredicate xdr;
        xdr.type = stellar::ClaimPredicateType::CLAIM_PREDICATE_UNCONDITIONAL;
        return xdr;
    }
};


class Predicate::Not : public Predicate {
    Predicate* m_inner;
public:
    Not(Predicate* inner): m_inner(inner)
    {
    }
    virtual ~Not(){delete m_inner;};

    Predicate* getInner() const
    {
        return m_inner;
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

    stellar::ClaimPredicate toXdr() {
        stellar::ClaimPredicate xdr;
        stellar::Array<stellar::ClaimPredicate,1> predicate;
        predicate.append(m_inner->toXdr());
        xdr.setNotPredicate(predicate);
        return xdr;
    }
};

class Predicate::Or : public Predicate {
    QList<Predicate*> m_inner;
public:
    Or(QList<Predicate*> inner): m_inner(inner)
    {

    }
    virtual ~Or(){
        for(auto i :m_inner)
            delete i;
    };

    QList<Predicate*> getInner() {
        return m_inner;
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

    stellar::ClaimPredicate toXdr() {
        stellar::ClaimPredicate xdr;
        stellar::Array<stellar::ClaimPredicate,2> predicates;
        for (int i = 0; i < m_inner.size(); i++) {
            predicates.append(m_inner[i]->toXdr());
        }
        xdr.setOrPredicates(predicates);
        return xdr;
    }
};


class Predicate::And :public Predicate {
    QList<Predicate*> m_inner;
public:
    And(QList<Predicate*> inner):m_inner(inner)
    {
    }
    virtual ~And(){
        for(auto i :m_inner)
            delete i;
    };
    QList<Predicate*> getInner() {
        return m_inner;
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

    stellar::ClaimPredicate toXdr() {
        stellar::ClaimPredicate xdr;
        stellar::Array<stellar::ClaimPredicate,2> predicates;
        for (int i = 0; i < m_inner.size(); i++) {
            predicates.append(m_inner[i]->toXdr());
        }
        xdr.setAndPredicates(predicates);
        return xdr;
    }
};

class Predicate::AbsBefore :public Predicate {
    qint64 m_epochSeconds;
public:
    AbsBefore(qint64 epochSeconds):m_epochSeconds(epochSeconds)
    {
    }
    virtual ~AbsBefore(){}

    qint64 getTimestampSeconds()
    {
        return m_epochSeconds;
    }

    //           public Instant getDate() {
    //           return Instant.ofEpochSecond(epochSeconds);
    //       }

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


    stellar::ClaimPredicate toXdr() {
        stellar::ClaimPredicate xdr;
        xdr.setAbsBefore(m_epochSeconds);
        return xdr;
    }
};

class Predicate::RelBefore :public Predicate {
    qint64 m_secondsSinceClose;
public:
    RelBefore(qint64 secondsSinceClose):m_secondsSinceClose(secondsSinceClose)
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



    stellar::ClaimPredicate toXdr() {
        stellar::ClaimPredicate xdr;
        xdr.setRelBefore(m_secondsSinceClose);
        return xdr;
    }
};



#endif // PREDICATE_H
