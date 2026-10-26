#ifndef SEP10CHALLENGE_H
#define SEP10CHALLENGE_H

#include <QObject>
#include "keypair.h"
#include "network.h"
#include "transaction.h"
#include "managedataoperation.h"
#include "network.h"
#include "qstellar_namespace.h"
QSTELLAR_BEGIN_NS



class Sep10Challenge
{
public:
    // Values the SEP fixes rather than choices this class makes, so a consumer
    // can quote them instead of writing a second copy that drifts. They were
    // private until the lab needed to show a reader what a challenge is
    // supposed to contain.
    static const int GRACE_PERIOD_SECONDS;
    static const QString CLIENT_DOMAIN_DATA_NAME;
    static const QString HOME_DOMAIN_MANAGER_DATA_NAME_FLAG;
    static const QString WEB_AUTH_DOMAIN_MANAGER_DATA_NAME;

private:
     /**
      * Returns a valid <a href="https://github.com/stellar/stellar-protocol/blob/master/ecosystem/sep-0010.md#response" target="_blank">SEP 10</a> challenge, for use in web authentication.
      * @param signer           The server's signing account.
      * @param network          The Stellar network used by the server.
      * @param clientAccountId  The stellar account belonging to the client.
      * @param domainName       The <a href="https://en.wikipedia.org/wiki/Fully_qualified_domain_name" target="_blank">fully qualified domain name</a> of the service requiring authentication.
      *                        The domainName field is reserved for future use and not used.
      * @param webAuthDomain    The fully qualified domain name of the service issuing the challenge.
      * @param timebounds       The lifetime of the challenge token.
      */
public:

    /**
     * Used to store the results produced by {@link Sep10Challenge#readChallengeTransaction(String, String, Network, String)}.
     */
    class ChallengeTransaction {
        Transaction* m_transaction;
        QString m_clientAccountId;
        QString m_matchedHomeDomain;
        QString m_clientDomain;
        QString m_clientDomainSigningKey;
    public:
        ChallengeTransaction(Transaction* transaction, QString clientAccountId, QString matchedHomeDomain);
        // The same, with the challenge's client_domain operation: its value and its source.
        ChallengeTransaction(Transaction* transaction, QString clientAccountId, QString matchedHomeDomain,
                             QString clientDomain, QString clientDomainSigningKey);
        // Owns and frees m_transaction (was leaked previously).
        ~ChallengeTransaction();
        Transaction* getTransaction() const;
        // A G address, or an M address for a muxed client: its signers are
        // then those of the G account it is built on.
        QString getClientAccountId() const;
        QString getMatchedHomeDomain() const;
        // The client_domain operation's value, and its source: the key that
        // must sign for that domain. Both empty when the challenge has none;
        // they are reported whether or not the reader was told what to expect.
        QString getClientDomain() const;
        QString getClientDomainSigningKey() const;
        int hashCode() const;
        bool equals(const ChallengeTransaction* other) const;

    };

    /**
     * Represents a transaction signer. The weight is the account signer's as
     * Horizon gives it, an int32: a signer added before protocol 10 may weigh
     * more than 255, and from 2^31 on it comes negative.
     * verifyChallengeTransactionThreshold counts any weight outside 0..255 as
     * 255, as stellar-core does.
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
      bool operator==(const Signer& other) const;
    };


    // BuildChallengeTx is a factory method that creates a valid SEP 10 challenge, for use in web authentication.
    // "timebound" is the time duration the transaction should be valid for, in seconds (> 0).
    // More details on SEP 10: https://github.com/stellar/stellar-protocol/blob/master/ecosystem/sep-0010.md
    //returned Transaction must be deleted after use, you can use the Transaction class to obtain the base64 encoded
    // domainName must not be blank and webAuthDomain not null; clientDomain is written as UTF-8.
    // clientAccountID is a G address or, as js and py take it, a muxed M
    // address, which cannot be combined with a memo.
    // Takes ownership of memo, also when it throws.
    static Transaction* buildChallengeTx(KeyPair* serverSignerSecret, QString clientAccountID, QString domainName, QString webAuthDomain, qint64 timebound, Network *network= Network::current()
                                         , QString clientDomain=QString(), QString clientSigningKey=QString(), Memo *memo=nullptr);


    // The same with explicit time bounds, which must be what a reader accepts:
    // not null, maxTime not 0 and not past, minTime at most GRACE_PERIOD_SECONDS
    // ahead. Takes ownership of timebound and memo, also when it throws.
    static Transaction* buildChallengeTx(KeyPair* serverSignerSecret, QString clientAccountID, QString domainName, QString webAuthDomain, TimeBounds* timebound, Network *network= Network::current()
                                         , QString clientDomain=QString(), QString clientSigningKey=QString(), Memo *memo=nullptr);


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
       * @param domainNames     One of the home domains that is expected to be included in the first Manage Data operation's string key.
       * @param webAuthDomain    The fully qualified domain name of the service issuing the challenge.
       * @return {@link ChallengeTransaction}, the decoded transaction envelope and client account ID contained within.
       * @throws InvalidSep10ChallengeException If the SEP-0010 validation fails, the exception will be thrown.
       * @throws IOException                    If read XDR string fails, the exception will be thrown.
       */
      static ChallengeTransaction* readChallengeTransaction(QString challengeXdr, QString serverAccountId, QStringList domainNames, QString webAuthDomain, Network* network= Network::current());
      /**
       * The same, for a client that asked for a client domain. The challenge
       * must then carry a client_domain operation whose value is
       * expectedClientDomain (as UTF-8 bytes) and whose source is
       * expectedClientDomainSigningKey, the SIGNING_KEY published in that
       * domain's stellar.toml. The two are given together; both empty, the
       * operation is not checked, as by the overloads above, and is still
       * reported by getClientDomain() and getClientDomainSigningKey().
       *
       * Every overload refuses a challenge with more than one client_domain
       * operation, one whose source is not a G address, and a memo together
       * with a muxed client account.
       */
      static ChallengeTransaction* readChallengeTransaction(QString challengeXdr, QString serverAccountId, QStringList domainNames, QString webAuthDomain,
                                                            QString expectedClientDomain, QString expectedClientDomainSigningKey,
                                                            Network* network= Network::current());
      /**
       * Reads a SEP 10 challenge transaction and returns the decoded transaction envelope and client account ID contained within.
       * <p>
       * It also verifies that transaction is signed by the server.
       * <p>
       * It does not verify that the transaction has been signed by the client or
       * that any signatures other than the servers on the transaction are valid. Use
       * one of the following functions to completely verify the transaction:
       * {@link Sep10Challenge#verifyChallengeTransactionSigners(String, String, Network, String, Set)} or
       * {@link Sep10Challenge#verifyChallengeTransactionThreshold(String, String, Network, String, int, Set)} or
       * {@link Sep10Challenge#verifyChallengeTransactionSigners(String, String, Network, String[], Set)} or
       * {@link Sep10Challenge#verifyChallengeTransactionThreshold(String, String, Network, String[], int, Set)} or
       *
       * @param challengeXdr    SEP-0010 transaction challenge transaction in base64.
       * @param serverAccountId Account ID for server's account.
       * @param network         The network to connect to for verifying and retrieving.
       * @param domainName      The home domains that is expected to be included in the first Manage Data operation's string key.
       * @param webAuthDomain    The fully qualified domain name of the service issuing the challenge.
       * @return {@link ChallengeTransaction}, the decoded transaction envelope and client account ID contained within.
       * @throws InvalidSep10ChallengeException If the SEP-0010 validation fails, the exception will be thrown.
       * @throws IOException                    If read XDR string fails, the exception will be thrown.
       */
      static ChallengeTransaction* readChallengeTransaction(QString challengeXdr, QString serverAccountId, QString domainName, QString webAuthDomain, Network* network= Network::current());
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
       * @param domainName      The home domains that is expected to be included in the first Manage Data operation's string key.
       * @param webAuthDomain    The fully qualified domain name of the service issuing the challenge.
       * @param signers         The signers of client account (a secret seed, or a G... that is
       *                        not a valid account id, among them is refused).
       * @return a list of signers that were found is returned, excluding the server account ID.
       * @throws InvalidSep10ChallengeException If the SEP-0010 validation fails, the exception will be thrown.
       * @throws IOException                    If read XDR string fails, the exception will be thrown.
       */
      static QSet<QString> verifyChallengeTransactionSigners(QString challengeXdr, QString serverAccountId, QString domainName, QString webAuthDomain, QSet<QString> signers, Network* network= Network::current())
      {
          return verifyChallengeTransactionSigners(challengeXdr,serverAccountId,QStringList() << domainName,webAuthDomain,signers,network);
      }
      /**
         * Verifies that for a SEP 10 challenge transaction
         * all signatures on the transaction are accounted for. A transaction is
         * verified if it is signed by the server account, and all other signatures
         * match a signer that has been provided as an argument. Additional signers can
         * be provided that do not have a signature, but all signatures must be matched
         * to a signer for verification to succeed. If verification succeeds a list of
         * signers that were found is returned, excluding the server account ID.
         * The source of the client_domain operation must have signed, and its
         * signature is not a client's: at least one client signer must have signed
         * as well. A challenge with more than one client_domain operation is refused.
         * For a muxed client (M...) the signers are those of the G account it
         * is built on; an M address among them is ignored, as js ignores it.
         *
         * @param challengeXdr    SEP-0010 transaction challenge transaction in base64.
         * @param serverAccountId Account ID for server's account.
         * @param network         The network to connect to for verifying and retrieving.
         * @param domainNames     One of the home domains that is expected to be included in the first Manage Data operation's string key.
         * @param webAuthDomain    The fully qualified domain name of the service issuing the challenge.
         * @param signers         The signers of client account. Entries that are not
         *                        account ids (G...) are skipped, as js skips them (py
         *                        refuses them), except a secret seed (S...) and a G...
         *                        that is not a valid account id (a typo, a bad checksum),
         *                        which are refused: py refuses both, js the second.
         * @return a list of signers that were found is returned, excluding the server account ID.
         * @throws InvalidSep10ChallengeException If the SEP-0010 validation fails, the exception will be thrown.
         * @throws IOException                    If read XDR string fails, the exception will be thrown.
         */
        static QSet<QString> verifyChallengeTransactionSigners(QString challengeXdr, QString serverAccountId, QStringList domainNames, QString webAuthDomain, QSet<QString> signers, Network* network= Network::current());
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
       * @param domainName      One of the home domains that is expected to be included in the first Manage Data operation's string key.
       * @param threshold       The threshold on the client account.
       * @param webAuthDomain    The fully qualified domain name of the service issuing the challenge.
       * @param signers         The signers of client account (a secret seed, or a G... that is
       *                        not a valid account id, among them is refused).
       * @return a list of signers that were found is returned, excluding the server account ID.
       * @throws InvalidSep10ChallengeException If the SEP-0010 validation fails, the exception will be thrown.
       * @throws IOException                    If read XDR string fails, the exception will be thrown.
       */
      static QSet<QString> verifyChallengeTransactionThreshold(QString challengeXdr, QString serverAccountId, QStringList domainNames, QString webAuthDomain, int threshold, QSet<Signer> signers,  Network* network=Network::current());
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
       * @param domainName      The home domain that is expected to be included in the first Manage Data operation's string key.
       * @param webAuthDomain    The fully qualified domain name of the service issuing the challenge.
       * @param threshold       The threshold on the client account.
       * @param signers         The signers of client account (a secret seed, or a G... that is
       *                        not a valid account id, among them is refused).
       * @return a list of signers that were found is returned, excluding the server account ID.
       * @throws InvalidSep10ChallengeException If the SEP-0010 validation fails, the exception will be thrown.
       * @throws IOException                    If read XDR string fails, the exception will be thrown.
       */
      static QSet<QString> verifyChallengeTransactionThreshold(QString challengeXdr, QString serverAccountId, QString domainName, QString webAuthDomain, int threshold, QSet<Signer> signers,  Network* network=Network::current());
private:
      static QSet<QString> verifyTransactionSignatures(Transaction* transaction, QSet<QString> signers);
public:
      static bool verifyTransactionSignature(Transaction* transaction, QString accountId);

      /** Where a home domain's SEP-10 service is, and the key it signs with. */
      struct WebAuth
      {
          QString endpoint;     // WEB_AUTH_ENDPOINT
          QString signingKey;   // SIGNING_KEY, the serverAccountId of the verify calls
      };

      /**
       * SEP-10's two values from a home domain's stellar.toml (read with
       * StellarTomlResolver): WEB_AUTH_ENDPOINT, an https URL, and
       * SIGNING_KEY, an account id. Throws std::runtime_error naming the one
       * that is missing or is not that.
       */
      static WebAuth webAuthFromToml(const QVariantMap& toml);
};
// Qt 6 hashes with a size_t seed; Qt 5 with uint. Match the active ABI so the
// container never has to narrow size_t -> uint at the call site (MSVC C4267).
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
inline size_t qHash(const Sep10Challenge::Signer &t, size_t seed)
#else
inline uint qHash(const Sep10Challenge::Signer &t, uint seed)
#endif
{
    return t.hashCode() ^ seed;
}


QSTELLAR_END_NS

QSTELLAR_ALIAS(Sep10Challenge)

#endif // SEP10CHALLENGE_H
