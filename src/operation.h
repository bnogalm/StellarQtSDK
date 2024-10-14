#ifndef OPERATION_H
#define OPERATION_H
#include "accountconverter.h"
#include "keypair.h"
#include "util.h"

class Operation
{

    static const qint32 FRACTIONAL = 7;
    static constexpr qint64 ONE = get_power_s<10,FRACTIONAL, qint64>::value;

protected:
    QString m_sourceAccount;
public:
    Operation();
    virtual ~Operation();

    static qint64 toXdrAmount(QString value);

    static QString fromXdrAmount(qint64 value);

    /**
     * Generates Operation XDR object.
     */
    stellar::Operation toXdr(AccountConverter accountConverter = AccountConverter().enableMuxed());

    /**
     * Returns base64-encoded Operation XDR object.
     */
    QString toXdrBase64();

    /**
     * Returns new Operation object from Operation XDR object.
     * @param xdr XDR object
     */
    static Operation* fromXdr(AccountConverter accountConverter, stellar::Operation& xdr);

    /**
     * Returns new Operation object from Operation XDR object using muxed accounts.
     * @param xdr XDR object
     */
    static Operation* fromXdr(stellar::Operation& xdr);

    /**
     * Returns operation source account.
     */
    QString getSourceAccount() const;

    /**
     * Sets operation source account.
     * @param sourceAccount
     */
    Operation* setSourceAccount(QString sourceAccount);


    /**
     * Fills OperationBody XDR object
     * @return OperationBody XDR object
     */
    virtual void fillOperationBody(AccountConverter& accountConverter, stellar::Operation &operation)=0; // abstract org.stellar.sdk.xdr.Operation.OperationBody toOperationBody();


};

inline Operation * checkNotNull(Operation *t, const char *error)
{
    if(!t)
        throw std::runtime_error(error);
    return t;
}
#endif // OPERATION_H
