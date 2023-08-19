#ifndef MANAGEDATAOPERATION_H
#define MANAGEDATAOPERATION_H
#include "operation.h"

/**
 * Represents <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html#manage-data" target="_blank">ManageData</a> operation.
 * @see <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html" target="_blank">List of Operations</a>
 */

class ManageDataOperation : public Operation
{
    stellar::ManageDataOp m_op;
public:

    /**
     * Creates a new ManageData builder. If you want to delete data entry don't pass anything as a <code>value</code> param.
     * @param name The name of data entry
     * @param value The value of data entry. <code>QByteArray()</code>QByteArray() will delete data entry.
     */
     ManageDataOperation(QString name, QByteArray value = QByteArray());
     virtual ~ManageDataOperation();

     ManageDataOperation(stellar::ManageDataOp &op);
     /**
      * The name of the data value
      */
     QString getName();

     /**
      * Data value
      */
     QByteArray getValue();
     void fillOperationBody(AccountConverter& accountConverter, stellar::Operation &operation);

     /**
      * Construct a new ManageOffer builder from a ManageDataOp XDR.
      * @param op {@link ManageDataOp}
      */
    static ManageDataOperation* build(stellar::ManageDataOp& op);
    static ManageDataOperation* create(QString name, QByteArray value = QByteArray());


    /**
         * Sets the source account for this operation.
         * @param sourceAccount The operation's source account.
         * @return ManageDataOperation object so you can chain methods.
         */
    ManageDataOperation* setSourceAccount(QString sourceAccount);


};

#endif // MANAGEDATAOPERATION_H
