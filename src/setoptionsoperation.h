#ifndef SETOPTIONSOPERATION_H
#define SETOPTIONSOPERATION_H
#include "operation.h"



/**
 * Represents <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html#set-options">SetOptions</a> operation.
 * @see <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html">List of Operations</a>
 */
class SetOptionsOperation : public Operation
{
    KeyPair* m_inflationDestination;
    stellar::SetOptionsOp m_op;
    SetOptionsOperation(stellar::SetOptionsOp& op);

public:
    /**
     * @brief SetOptionsOperation Creates an empty SetOptionsOperation
     * If you want to enqueue set calls, you will have to use create static method
     */
    SetOptionsOperation();

    SetOptionsOperation(KeyPair *inflationDestination, Integer clearFlags
                        , Integer setFlags, Integer masterKeyWeight, Integer lowThreshold
                        , Integer mediumThreshold, Integer highThreshold, QString homeDomain
                        , stellar::SignerKey signer, quint32 signerWeight);

    virtual ~SetOptionsOperation();


    /**
     * Account of the inflation destination.
     */
    KeyPair *getInflationDestination();

    /**
     * Indicates which flags to clear. For details about the flags, please refer to the <a href="https://www.stellar.org/developers/learn/concepts/accounts.html" target="_blank">accounts doc</a>.
     * You can also use {@link AccountFlag} enum.
     */
    Integer getClearFlags();

    /**
     * Indicates which flags to set. For details about the flags, please refer to the <a href="https://www.stellar.org/developers/learn/concepts/accounts.html" target="_blank">accounts doc</a>.
     * You can also use {@link AccountFlag} enum.
     */
    Integer getSetFlags();

    /**
     * Weight of the master key.
     */
    Integer getMasterKeyWeight();

    /**
     * A number from 0-255 representing the threshold this account sets on all operations it performs that have <a href="https://www.stellar.org/developers/learn/concepts/multi-sig.html" target="_blank">a low threshold</a>.
     */
    Integer getLowThreshold();

    /**
     * A number from 0-255 representing the threshold this account sets on all operations it performs that have <a href="https://www.stellar.org/developers/learn/concepts/multi-sig.html" target="_blank">a medium threshold</a>.
     */
    Integer getMediumThreshold();

    /**
     * A number from 0-255 representing the threshold this account sets on all operations it performs that have <a href="https://www.stellar.org/developers/learn/concepts/multi-sig.html" target="_blank">a high threshold</a>.
     */
    Integer getHighThreshold();

    /**
     * The home domain of an account.
     */
    QString getHomeDomain();

    /**
     * Additional signer added/removed in this operation.
     */
    stellar::SignerKey* getSigner();

    /**
     * Additional signer weight. The signer is deleted if the weight is 0.
     */
    Integer getSignerWeight();
    void fillOperationBody(AccountConverter& accountConverter, stellar::Operation &op);
    /**
    * Builds SetOptions operation.
    * @see SetOptionsOperation
    */
    static SetOptionsOperation * build(stellar::SetOptionsOp &op);

    /**
     * @brief create new SetOptionsOperation
     * @return SetOptionsOperation
     */
    static SetOptionsOperation * create();


    /**
       * Sets the inflation destination for the account.
       * @param inflationDestination The inflation destination account.
       * @return SetOptionsOperation object so you can chain methods.
       */
    SetOptionsOperation* setInflationDestination(KeyPair* inflationDestination);

    /**
       * Clears the given flags from the account.
       * @param clearFlags For details about the flags, please refer to the <a href="https://www.stellar.org/developers/learn/concepts/accounts.html" target="_blank">accounts doc</a>.
       * @return SetOptionsOperation object so you can chain methods.
       */
    SetOptionsOperation* setClearFlags(quint32 clearFlags);

    /**
       * Sets the given flags on the account.
       * @param setFlags For details about the flags, please refer to the <a href="https://www.stellar.org/developers/learn/concepts/accounts.html" target="_blank">accounts doc</a>.
       * @return SetOptionsOperation object so you can chain methods.
       */
    SetOptionsOperation* setSetFlags(quint32 setFlags);

    /**
       * Weight of the master key.
       * @param masterKeyWeight Number between 0 and 255
       * @return SetOptionsOperation object so you can chain methods.
       */
    SetOptionsOperation* setMasterKeyWeight(quint32 masterKeyWeight);

    /**
       * A number from 0-255 representing the threshold this account sets on all operations it performs that have a low threshold.
       * @param lowThreshold Number between 0 and 255
       * @return SetOptionsOperation object so you can chain methods.
       */
    SetOptionsOperation* setLowThreshold(quint32 lowThreshold);

    /**
       * A number from 0-255 representing the threshold this account sets on all operations it performs that have a medium threshold.
       * @param mediumThreshold Number between 0 and 255
       * @return SetOptionsOperation object so you can chain methods.
       */
    SetOptionsOperation* setMediumThreshold(quint32 mediumThreshold);

    /**
       * A number from 0-255 representing the threshold this account sets on all operations it performs that have a high threshold.
       * @param highThreshold Number between 0 and 255
       * @return SetOptionsOperation object so you can chain methods.
       */
    SetOptionsOperation* setHighThreshold(quint32 highThreshold);

    /**
       * Sets the account's home domain address used in <a href="https://www.stellar.org/developers/learn/concepts/federation.html" target="_blank">Federation</a>.
       * @param homeDomain A string of the address which can be up to 32 characters.
       * @return SetOptionsOperation object so you can chain methods.
       */
    SetOptionsOperation* setHomeDomain(QString homeDomain);

    /**
       * Add, update, or remove a signer from the account. Signer is deleted if the weight = 0;
       * @param signer The signer key. Use {@link org.stellar.sdk.Signer} helper to create this object.
       * @param weight The weight to attach to the signer (0-255).
       * @return SetOptionsOperation object so you can chain methods.
       */
    SetOptionsOperation* setSigner(stellar::SignerKey signer, quint32 weight);

    /**
       * Sets the source account for this operation.
       * @param sourceAccount The operation's source account.
       * @return SetOptionsOperation object so you can chain methods.
       */
    SetOptionsOperation* setSourceAccount(QString sourceAccount);


};

#endif // SETOPTIONSOPERATION_H
