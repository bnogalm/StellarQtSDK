#ifndef OPERATION_H
#define OPERATION_H
#include "accountconverter.h"
#include "keypair.h"
#include "util.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

class Operation
{
    // Operations are handled by pointer. Many cache an Asset* or KeyPair*
    // they own, and an implicit copy shared it, so both copies deleted it.
    Q_DISABLE_COPY(Operation)

    static const qint32 FRACTIONAL = 7;
    static constexpr qint64 ONE = get_power_s<10,FRACTIONAL, qint64>::value;

protected:
    QString m_sourceAccount;
public:
    Operation();
    virtual ~Operation();

    /**
     * An amount in XLM units as stroops, read exactly: an optional sign,
     * digits with an optional point (".5" and "5." included) and an optional
     * exponent ("1e3"), as js-stellar-base and py-stellar-sdk read them. More
     * than seven decimals that are not zeros, or a value past int64 stroops,
     * throws; nothing is rounded.
     *
     * Refused on purpose, though a reference SDK reads them: digits grouped
     * with '_' ("1_000.5", which js and py both sign, and disagree on as
     * "_1" or "1__0") and js's radix forms "0x10", "0b11", "0o7" (py refuses
     * them). In an amount about to be signed they are easy to misread. A
     * comma is refused too, never taken for a point.
     */
    static qint64 toXdrAmount(QString value);

    /**
     * toXdrAmount for an amount that must be more than zero: 0 is refused
     * where js-stellar-base (isValidAmount without allowZero) and stellar-core
     * refuse it (payment, path payments, passive offer, claimable balance,
     * clawback, pool withdrawal). They used to sign 0.
     */
    static qint64 toXdrAmountPositive(QString value);

    /**
     * toXdrAmount for an operation's amount field: below zero is refused. No
     * Stellar operation takes a negative amount; "-5" used to be signed as
     * -50000000 stroops into a payment the network then rejected.
     */
    static qint64 toXdrAmountNonNegative(QString value);

    static QString fromXdrAmount(qint64 value);

    /**
     * Generates Operation XDR object.
     */
    stellar::Operation toXdr(AccountConverter accountConverter = AccountConverter().enableMuxed());

    /**
     * Returns base64-encoded Operation XDR object. Throws std::runtime_error
     * when the operation cannot be encoded (it holds a value the XDR has no
     * encoding for); it used to return an empty string, or the base64 of the
     * part written before the write failed.
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

QSTELLAR_END_NS

QSTELLAR_ALIAS(Operation)

#endif // OPERATION_H
