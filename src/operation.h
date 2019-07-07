#ifndef OPERATION_H
#define OPERATION_H
#include "keypair.h"
#include "util.h"

class Operation
{

    static const qint32 FRACTIONAL = 7;
    static constexpr qint64 ONE = get_power_s<10,FRACTIONAL, qint64>::value;

protected:
    KeyPair *m_sourceAccount;
public:
    Operation();
    virtual ~Operation();

    static qint64 toXdrAmount(QString value);

    static QString fromXdrAmount(qint64 value);

    /**
     * Generates Operation XDR object.
     */
    stellar::Operation toXdr();

    /**
     * Returns base64-encoded Operation XDR object.
     */
    QString toXdrBase64();

    /**
     * Returns new Operation object from Operation XDR object.
     * @param xdr XDR object
     */
    static Operation* fromXdr(stellar::Operation& xdr);

    /**
     * Returns operation source account.
     */
    KeyPair* getSourceAccount();

    /**
     * Sets operation source account.
     * Operation takes ownership of the keypair and it will be deleted.
     * @param keypair
     */
    Operation* setSourceAccount(KeyPair *keypair);
    /**
     * Sets operation source account making a copy.
     * Operation takes ownership of the keypair and it will be deleted.
     * @param keypair
     */
    Operation* setSourceAccount(KeyPair &keypair);

    /**
     * Fills OperationBody XDR object
     * @return OperationBody XDR object
     */
    virtual void fillOperationBody(stellar::Operation &operation)=0; // abstract org.stellar.sdk.xdr.Operation.OperationBody toOperationBody();


};

inline Operation * checkNotNull(Operation *t, const char *error)
{
    if(!t)
        throw std::runtime_error(error);
    return t;
}
#endif // OPERATION_H
