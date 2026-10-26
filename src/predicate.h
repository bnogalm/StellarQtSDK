#ifndef PREDICATE_H
#define PREDICATE_H
#include <QtCore>
#include "xdr/stellarledgerentries.h"
#include "qstellar_namespace.h"
#include <stdexcept>

QSTELLAR_BEGIN_NS

class Predicate;

class Predicate
{
protected:
    /** The operands of an XDR and/or. When one is refused ("Corrupted
     *  predicate": an and/or of fewer than 2, which a decoded envelope's
     *  claimant can carry), those converted so far are freed, as
     *  copyOperands frees its copies; they leaked. */
    static QList<Predicate*> convertXDRPredicates(QVector<stellar::ClaimPredicate> predicates) {
        QList<Predicate*> list;
        try {
            for (stellar::ClaimPredicate& p : predicates) {
                list.append(fromXdr(p));
            }
        } catch (...) {
            qDeleteAll(list);
            throw;
        }
        return list;
    }

    /**
     * The same operands in the same order, position by position (a null
     * operand equals only a null one). The XDR array is ordered, so and(a, b)
     * and and(b, a) encode, and so sign, differently: they are two
     * predicates, as in py-stellar-sdk (ClaimPredicateGroup compares left
     * with left and right with right). And/Or equals() used to ask only that
     * each operand be among the other node's, so or(a, a) equalled or(a, b);
     * then it matched them in any order, so and(b, a) equalled and(a, b).
     * Either way Claimant == took two claimants for one, and
     * Claimant::setPredicate kept the predicate it held and signed that.
     */
    static bool sameOperands(const QList<Predicate*>& mine, const QList<Predicate*>& theirs)
    {
        if (mine.size() != theirs.size())
            return false;
        for (qsizetype i = 0; i < mine.size(); ++i) {
            Predicate* m = mine.at(i);
            Predicate* t = theirs.at(i);
            if (m && t ? !m->equals(t) : m != t)
                return false;
        }
        return true;
    }

    /** From the operands' own hashes in order (h = h * 31 + operand), as
     *  equals() compares them. This hashed their addresses, so equal nodes
     *  hashed apart, and then summed their hashes, so and(a, b) and
     *  and(b, a) always hashed alike. Unsigned, so it wraps without UB. */
    static int operandsHash(const QList<Predicate*>& operands)
    {
        uint hash = 0;
        for (Predicate* p : operands)
            hash = hash * 31u + (p ? static_cast<uint>(p->hashCode()) : 0u);
        return static_cast<int>(hash);
    }

    /** Deep copies of `operands`, each in its place, a missing (null) one
     *  too. Or and And copied only the operands that were there, so a copy
     *  of and(null, b) was and(b): its operands had moved, and it no longer
     *  equalled what it copied (a Claimant and its copy included). When a
     *  copy throws (a type create() does not know), the copies made so far
     *  are freed; they leaked. Or/And assignment copies before it deletes
     *  what it holds, so a throw leaves the node as it was, not half
     *  assigned. */
    static QList<Predicate*> copyOperands(const QList<Predicate*>& operands)
    {
        QList<Predicate*> copies;
        try {
            for (Predicate* p : operands)
                copies.append(p ? create(*p) : nullptr);
        } catch (...) {
            qDeleteAll(copies);
            throw;
        }
        return copies;
    }



public:
    /**
      * Generates Predicate object from a given XDR object
      * @param xdr XDR object
      */
    static Predicate* fromXdr(stellar::ClaimPredicate& xdr);
    static Predicate* create(Predicate &predicate);
    /**
     * The predicate `predicate` holds as Horizon writes one (a QVariantMap,
     * or anything QJsonValue::fromVariant makes an object of), read exactly
     * by fromJson. The caller owns the result.
     */
    static Predicate* create(const QVariant &predicate);

    /**
     * A claim predicate as Horizon writes it, exactly: {"unconditional":
     * true}, {"and": [a, b]}, {"or": [a, b]}, {"not": p}, {"abs_before":
     * time, "abs_before_epoch": seconds} or {"rel_before": seconds}. The time
     * is UTC text as Horizon writes it (2020-09-28T17:57:04Z, with a '+'
     * before a year past 9999: +10000-01-01T00:00:00Z), and
     * abs_before_epoch, when there, must name the same second. Seconds are
     * decimal text (or a whole JSON number below 2^53) in [0, 2^63-1], and
     * nothing nests deeper than the 4 levels Core allows. The caller owns
     * the result. Throws std::runtime_error naming `field` (the path of
     * `json`, for the message) and the member at fault for anything else:
     * another key, another operand count, a time that is not one.
     */
    static Predicate* fromJson(const QJsonValue& json, const QString& field = QString());
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
        // The default ctor leaves m_inner null, so copying an empty Not
        // (which happens on its own inside a QList or when going through
        // QVariant) dereferenced a null pointer.
        m_inner = other.m_inner ? Predicate::create(*(other.m_inner)) : nullptr;
    }
    /** Rule of three: with a copy ctor and destructor of our own, the implicit
     *  assignment copied the pointer and both deleted it (double-free). It
     *  copies before it deletes, as And/Or do: when the copy threw (a type
     *  create() does not know), m_inner was left pointing at the operand it
     *  had just freed, and the destructor freed it again. */
    Not& operator=(const Not& other)
    {
        if (this == &other) return *this;
        Predicate* copy = other.m_inner ? Predicate::create(*(other.m_inner)) : nullptr;
        delete m_inner;
        m_inner = copy;
        return *this;
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
            // A default Not has no operand, which this dereferenced.
            if (!this->m_inner || !oc->m_inner)
                return !this->m_inner && !oc->m_inner;
            return this->m_inner->equals(oc->m_inner);
        }
        return false;
    }
    int hashCode() const
    {
        return (m_inner ? m_inner->hashCode() : 0)^0xaa;
    }

    stellar::ClaimPredicate toXdr()  const {
        if (!m_inner)
            throw std::runtime_error("claim predicate NOT has no operand");
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
    Or(const Or& other): m_inner(copyOperands(other.m_inner))
    {
    }
    Or()
    {

    }
    /** Rule of three: without this, assignment copied the pointers and both
     *  objects deleted them (double-free). */
    Or& operator=(const Or& other)
    {
        if (this == &other) return *this;
        const QList<Predicate*> copies = copyOperands(other.m_inner);
        for(auto i : m_inner) delete i;
        m_inner = copies;
        return *this;
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
            return sameOperands(m_inner, oc->m_inner);
        }
        return false;
    }
    int hashCode() const
    {
        return operandsHash(m_inner)^0xbb;
    }

    stellar::ClaimPredicate toXdr()  const {
        stellar::ClaimPredicate xdr;
        stellar::Array<stellar::ClaimPredicate,2>& predicates = xdr.fillOrPredicates();
        // stellar::Array::append silently DROPS anything past max, so a node
        // with 3 children went out carrying only 2 and no warning at all. The
        // XDR admits exactly 2 operands per node, and Core refuses a node with
        // fewer (validatePredicate), which went out too.
        if (m_inner.size() != 2 || !m_inner[0] || !m_inner[1]) {
            throw std::runtime_error(
                "claim predicate AND/OR admits exactly 2 operands; nest further nodes instead");
        }
        for (qsizetype i = 0; i < m_inner.size(); i++) {
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
    And(const And& other): m_inner(copyOperands(other.m_inner))
    {
    }
    /** Rule of three: without this, assignment copied the pointers and both
     *  objects deleted them (double-free). */
    And& operator=(const And& other)
    {
        if (this == &other) return *this;
        const QList<Predicate*> copies = copyOperands(other.m_inner);
        for(auto i : m_inner) delete i;
        m_inner = copies;
        return *this;
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
            return sameOperands(m_inner, oc->m_inner);
        }
        return false;
    }

    int hashCode() const
    {
        return operandsHash(m_inner)^0xcc;
    }

    stellar::ClaimPredicate toXdr()  const {
        stellar::ClaimPredicate xdr;
        stellar::Array<stellar::ClaimPredicate,2>& predicates= xdr.fillAndPredicates();
        // See the note in Or::toXdr: append silently drops anything past 2,
        // and Core refuses fewer.
        if (m_inner.size() != 2 || !m_inner[0] || !m_inner[1]) {
            throw std::runtime_error(
                "claim predicate AND/OR admits exactly 2 operands; nest further nodes instead");
        }
        for (qsizetype i = 0; i < m_inner.size(); i++) {
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
        return QDateTime::fromSecsSinceEpoch(m_epochSeconds, QTimeZone::UTC);
#else
        // Qt < 6.5 has no QTimeZone::UTC; Qt::UTC (TimeSpec) also serialises as "Z".
        return QDateTime::fromSecsSinceEpoch(m_epochSeconds, Qt::UTC);
#endif
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
        return static_cast<int>(qHash(m_epochSeconds))^0xdd;
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
        return static_cast<int>(qHash(m_secondsSinceClose))^0xee;
    }



    stellar::ClaimPredicate toXdr()  const {
        stellar::ClaimPredicate xdr;
        xdr.setRelBefore(m_secondsSinceClose);
        return xdr;
    }
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(Predicate)

// Q_DECLARE_METATYPE must be at global scope. The unqualified `Predicate`
// resolves via the alias to `qstellar::Predicate`.
Q_DECLARE_METATYPE(qstellar::Predicate::AbsBefore);
Q_DECLARE_METATYPE(qstellar::Predicate::And);
Q_DECLARE_METATYPE(qstellar::Predicate::Not);
Q_DECLARE_METATYPE(qstellar::Predicate::Or);
Q_DECLARE_METATYPE(qstellar::Predicate::RelBefore);
Q_DECLARE_METATYPE(qstellar::Predicate::Unconditional);

#endif // PREDICATE_H
