#include "predicate.h"


Predicate *Predicate::fromXdr(stellar::ClaimPredicate &xdr) {
    switch (xdr.type) {
    case stellar::ClaimPredicateType::CLAIM_PREDICATE_UNCONDITIONAL:
        return new Unconditional();
//    case stellar::ClaimPredicateType::CLAIM_PREDICATE_AND:
//        return And(convertXDRPredicates(xdr.getAndPredicates()));
//    case stellar::ClaimPredicateType::CLAIM_PREDICATE_OR:
//        return Or(convertXDRPredicates(xdr.getOrPredicates()));
//    case stellar::ClaimPredicateType::CLAIM_PREDICATE_NOT:
//        return Not(fromXdr(xdr.getNotPredicate()));
//    case stellar::ClaimPredicateType::CLAIM_PREDICATE_BEFORE_RELATIVE_TIME:
//        return RelBefore(xdr.getRelBefore().getInt64());
//    case stellar::ClaimPredicateType::CLAIM_PREDICATE_BEFORE_ABSOLUTE_TIME:
//        return AbsBefore(xdr.getAbsBefore().getInt64());
    default:
        throw std::runtime_error("Unknown asset type");
    }
}
