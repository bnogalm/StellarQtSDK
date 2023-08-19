#include "predicate.h"


Predicate *Predicate::fromXdr(stellar::ClaimPredicate &xdr) {
    switch (xdr.type) {
    case stellar::ClaimPredicateType::CLAIM_PREDICATE_UNCONDITIONAL:
        return new Unconditional();
    case stellar::ClaimPredicateType::CLAIM_PREDICATE_AND:
        if(xdr.notPredicate.value.size()>1)
            return new And(Predicate::convertXDRPredicates(xdr.andPredicates.value));
        else
            throw std::runtime_error("Corrupted predicate");
    case stellar::ClaimPredicateType::CLAIM_PREDICATE_OR:
        if(xdr.notPredicate.value.size()>1)
            return new Or(Predicate::convertXDRPredicates(xdr.orPredicates.value));
        else
            throw std::runtime_error("Corrupted predicate");
    case stellar::ClaimPredicateType::CLAIM_PREDICATE_NOT:
        if(xdr.notPredicate.value.size()>0)
            return new Not(Predicate::fromXdr(xdr.notPredicate.value[0]));
        else
            throw std::runtime_error("Corrupted predicate");
    case stellar::ClaimPredicateType::CLAIM_PREDICATE_BEFORE_RELATIVE_TIME:
        return new RelBefore(xdr.relBefore);
    case stellar::ClaimPredicateType::CLAIM_PREDICATE_BEFORE_ABSOLUTE_TIME:
        return new AbsBefore(xdr.absBefore);
    default:
        throw std::runtime_error("Unknown predicate");
    }
}

Predicate *Predicate::create(Predicate &predicate)
{
    try {
        Predicate::AbsBefore &rm = dynamic_cast<Predicate::AbsBefore&>(predicate);
        return new Predicate::AbsBefore(rm);
    }
    catch (const std::bad_cast&) {
    }
    try {
        Predicate::And &rm = dynamic_cast<Predicate::And&>(predicate);
        return new Predicate::And(rm);
    }
    catch (const std::bad_cast&) {
    }
    try {
        Predicate::Not &rm = dynamic_cast<Predicate::Not&>(predicate);
        return new Predicate::Not(rm);
    }
    catch (const std::bad_cast&) {
    }
    try {
        Predicate::Or &rm = dynamic_cast<Predicate::Or&>(predicate);
        return new Predicate::Or(rm);
    }
    catch (const std::bad_cast&) {
    }
    try {
        Predicate::RelBefore &rm = dynamic_cast<Predicate::RelBefore&>(predicate);
        return new Predicate::RelBefore(rm);
    }
    catch (const std::bad_cast&) {
    }
    try {
        Predicate::Unconditional &rm = dynamic_cast<Predicate::Unconditional&>(predicate);
        return new Predicate::Unconditional(rm);
    }
    catch (const std::bad_cast&) {
    }
    throw std::runtime_error("Unsupported predicate");
}

Predicate *Predicate::create(const QVariant &predicate)
{
    QVariantMap vm= predicate.toMap();
    if(vm.contains("unconditional"))
        return new Predicate::Unconditional();
    if(vm.contains("or"))
    {
        QList<Predicate*> orPredicates;
        QList<QVariant> predicatesToConvert = vm.value("or").toList();
        for(QVariant v:predicatesToConvert)
        {
            orPredicates.append(Predicate::create(v));
        }
        return new Predicate::Or(orPredicates);
    }
    if(vm.contains("and"))
    {
        QList<Predicate*> andPredicates;
        QList<QVariant> predicatesToConvert = vm.value("and").toList();
        for(QVariant v:predicatesToConvert)
        {
            andPredicates.append(Predicate::create(v));
        }
        return new Predicate::And(andPredicates);
    }
    if(vm.contains("not"))
    {
        return new Predicate::Not(Predicate::create(vm.value("not")));
    }
    if(vm.contains("abs_before"))
    {

        return new Predicate::AbsBefore(QDateTime::fromString(vm.value("abs_before").toString(),Qt::ISODate).toSecsSinceEpoch());
    }
    if(vm.contains("rel_before"))
    {
        return new Predicate::RelBefore(vm.value("rel_before").toLongLong());
    }
    throw std::runtime_error("Unsupported predicate");
}

Predicate::~Predicate(){}
static void registerTypes()
{

    qRegisterMetaType<Predicate::AbsBefore>();
    qRegisterMetaType<Predicate::And>();
    qRegisterMetaType<Predicate::Not>();
    qRegisterMetaType<Predicate::Or>();
    qRegisterMetaType<Predicate::RelBefore>();
    qRegisterMetaType<Predicate::Unconditional>();
}

Q_COREAPP_STARTUP_FUNCTION(registerTypes)
