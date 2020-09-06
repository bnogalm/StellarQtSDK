#ifndef SEP10CHALLENGE_H
#define SEP10CHALLENGE_H

#include <QObject>
#include "keypair.h"
#include "network.h"
#include "transaction.h"
#include "managedataoperation.h"
#include "network.h"


class Sep10Challenge
{
     /**
      * Returns a valid <a href="https://github.com/stellar/stellar-protocol/blob/master/ecosystem/sep-0010.md#response" target="_blank">SEP 10</a> challenge, for use in web authentication.
      * @param signer           The server's signing account.
      * @param network          The Stellar network used by the server.
      * @param clientAccountId  The stellar account belonging to the client.
      * @param domainName       The <a href="https://en.wikipedia.org/wiki/Fully_qualified_domain_name" target="_blank">fully qualified domain name</a> of the service requiring authentication.
      * @param timebounds       The lifetime of the challenge token.
      */
public:
    /**
     * Used to store the results produced by {@link Sep10Challenge#readChallengeTransaction(String, String, Network, String)}.
     */
    class ChallengeTransaction {
      Transaction* m_transaction;
      QString m_clientAccountId;
      public:
      ChallengeTransaction(Transaction* transaction, QString clientAccountId);
      Transaction* getTransaction() const;
      QString getClientAccountId() const;
      int hashCode() const;
      bool equals(const ChallengeTransaction* other) const;
    };

    /**
     * Represents a transaction signer.
     */
    class Signer {
      QString m_key;
      int m_weight;
        public:
      Signer(QString key, int weight);

      QString getKey();

      int getWeight() const;
      int hashCode() const;
      bool equals(const Signer* other) const;
    };


    // BuildChallengeTx is a factory method that creates a valid SEP 10 challenge, for use in web authentication.
    // "timebound" is the time duration the transaction should be valid for, O means infinity.
    // More details on SEP 10: https://github.com/stellar/stellar-protocol/blob/master/ecosystem/sep-0010.md
    //returned Transaction must be deleted after use, you can use the Transaction class to obtain the base64 encoded
    static Transaction* buildChallengeTx(KeyPair* serverSignerSecret, QString clientAccountID, QString domainName, qint64 timebound, Network *network= Network::current());
    /**
       * Reads a SEP 10 challenge transaction and returns the decoded transaction envelope and client account ID contained within.
       * <p>
       * It also verifies that transaction is signed by the server.
       * <p>
       * It does not verify that the transaction has been signed by the client or
       * that any signatures other than the servers on the transaction are valid. Use
       * one of the following functions to completely verify the transaction:
       * {@link Sep10Challenge#verifyChallengeTransactionSigners(String, String, Network, String, Set)} or
       * {@link Sep10Challenge#verifyChallengeTransactionThreshold(String, String, Network, String, int, Set)}
       *
       * @param challengeXdr    SEP-0010 transaction challenge transaction in base64.
       * @param serverAccountId Account ID for server's account.
       * @param network         The network to connect to for verifying and retrieving.
       * @param domainName      The <a href="https://en.wikipedia.org/wiki/Fully_qualified_domain_name" target="_blank">fully qualified domain name</a> of the service requiring authentication.
       * @return {@link ChallengeTransaction}, the decoded transaction envelope and client account ID contained within.
       * @throws InvalidSep10ChallengeException If the SEP-0010 validation fails, the exception will be thrown.
       * @throws IOException                    If read XDR string fails, the exception will be thrown.
       */
      static ChallengeTransaction* readChallengeTransaction(QString challengeXdr, QString serverAccountId, QString domainName, Network* network= Network::current());

      /**
       * Verifies that for a SEP 10 challenge transaction
       * all signatures on the transaction are accounted for. A transaction is
       * verified if it is signed by the server account, and all other signatures
       * match a signer that has been provided as an argument. Additional signers can
       * be provided that do not have a signature, but all signatures must be matched
       * to a signer for verification to succeed. If verification succeeds a list of
       * signers that were found is returned, excluding the server account ID.
       *
       * @param challengeXdr    SEP-0010 transaction challenge transaction in base64.
       * @param serverAccountId Account ID for server's account.
       * @param network         The network to connect to for verifying and retrieving.
       * @param domainName      The <a href="https://en.wikipedia.org/wiki/Fully_qualified_domain_name" target="_blank">fully qualified domain name</a> of the service requiring authentication.
       * @param signers         The signers of client account.
       * @return a list of signers that were found is returned, excluding the server account ID.
       * @throws InvalidSep10ChallengeException If the SEP-0010 validation fails, the exception will be thrown.
       * @throws IOException                    If read XDR string fails, the exception will be thrown.
       */
      static QSet<QString> verifyChallengeTransactionSigners(QString challengeXdr, QString serverAccountId, QString domainName, QSet<QString> signers, Network* network= Network::current());

      /**
       * Verifies that for a SEP-0010 challenge transaction
       * all signatures on the transaction are accounted for and that the signatures
       * meet a threshold on an account. A transaction is verified if it is signed by
       * the server account, and all other signatures match a signer that has been
       * provided as an argument, and those signatures meet a threshold on the
       * account.
       *
       * @param challengeXdr    SEP-0010 transaction challenge transaction in base64.
       * @param serverAccountId Account ID for server's account.
       * @param network         The network to connect to for verifying and retrieving.
       * @param domainName      The <a href="https://en.wikipedia.org/wiki/Fully_qualified_domain_name" target="_blank">fully qualified domain name</a> of the service requiring authentication.
       * @param threshold       The threshold on the client account.
       * @param signers         The signers of client account.
       * @return a list of signers that were found is returned, excluding the server account ID.
       * @throws InvalidSep10ChallengeException If the SEP-0010 validation fails, the exception will be thrown.
       * @throws IOException                    If read XDR string fails, the exception will be thrown.
       */
      static QSet<QString> verifyChallengeTransactionThreshold(QString challengeXdr, QString serverAccountId, QString domainName, int threshold, QSet<Signer> signers,  Network* network=Network::current());
private:
      static QSet<QString> verifyTransactionSignatures(Transaction* transaction, QString domainName, QSet<QString> signers);
public:
      static bool verifyTransactionSignature(Transaction* transaction, QString accountId, QString domainName);



};

#endif // SEP10CHALLENGE_H
