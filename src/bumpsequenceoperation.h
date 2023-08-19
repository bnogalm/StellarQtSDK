#ifndef BUMPSEQUENCEOPERATION_H
#define BUMPSEQUENCEOPERATION_H
#include "operation.h"

class BumpSequenceOperation : public Operation
{
    stellar::BumpSequenceOp m_op;
public:
    BumpSequenceOperation(qint64 bumpTo);
    BumpSequenceOperation(stellar::BumpSequenceOp& op);
    virtual ~BumpSequenceOperation();

    qint64 getBumpTo() const;

    void fillOperationBody(AccountConverter& accountConverter, stellar::Operation &operation);


    static BumpSequenceOperation* build(stellar::BumpSequenceOp &op);
    static BumpSequenceOperation* create(qint64 bumpTo);

    /**
         * Sets the source account for this operation.
         * @param sourceAccount The operation's source account.
         * @return BumpSequenceOperation object so you can chain methods.
         */
    BumpSequenceOperation* setSourceAccount(QString sourceAccount);

};

#endif // BUMPSEQUENCEOPERATION_H
