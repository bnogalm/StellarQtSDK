#ifndef SEP45CHALLENGE_H
#define SEP45CHALLENGE_H

#include <QByteArray>
#include <QList>
#include <QString>
#include <QStringList>
#include <QtGlobal>

#include "keypair.h"
#include "network.h"
#include "qstellar_namespace.h"
#include "xdr/sorobanops.h"
QSTELLAR_FWD(SorobanServer)

QSTELLAR_BEGIN_NS


/**
 * SEP-45: Stellar Web Authentication for Contract Accounts.
 *
 * TWO FORMATS LIVE HERE, AND ONLY ONE IS SEP-45.
 *
 * 1. SEP-45 v0.1.1, the published standard (since 1.0.4): what py-stellar-sdk
 *    (sep/stellar_soroban_web_authentication.py, the reference used here) and
 *    the Anchor Platform implement. The challenge is a base64
 *    `SorobanAuthorizationEntries`: an entry for the server's SIGNING_KEY,
 *    signed by the server, an entry for the client's contract account, and an
 *    entry for the client domain's SIGNING_KEY when one was asked for. Every
 *    entry authorizes the same call, `web_auth_verify` on the server's
 *    WEB_AUTH_CONTRACT_ID, whose one argument is a map of strings: account,
 *    home_domain, nonce, web_auth_domain, web_auth_domain_account and, with a
 *    client domain, client_domain and client_domain_account.
 *      - server:  buildChallengeAuthorizationEntries(), with an RPC to simulate
 *                 the call as py does, or without one when the server knows
 *                 the ledger; verifyChallengeAuthorizationEntries() on the
 *                 answer, which simulates the call with the returned entries.
 *      - wallet:  readChallengeAuthorizationEntries() to check a challenge,
 *                 signChallengeAuthorizationEntries() to check it and sign the
 *                 client's entry (and the client domain's), and
 *                 simulateSignedChallenge() to make sure the signed entries do
 *                 nothing but log in before posting them.
 *    What these produce is byte for byte what py produces from the same keys,
 *    nonce, ledger and simulation, and each side accepts what py builds.
 *
 * 2. newChallenge() / verifyChallenge(): DEPRECATED — this SDK's own earlier,
 *    NON-STANDARD format, kept only because the 1.0 API is frozen. A
 *    server-signed TransactionEnvelope whose call takes five positional
 *    arguments and carries one client entry. It does NOT interoperate: a
 *    SEP-45 server or client refuses what it builds, and it refuses what they
 *    build. Use it only where both ends run an older release of this SDK;
 *    everything new should use the v0.1.1 functions above.
 *
 * The client is a Soroban contract account (C...), not a classic account as
 * in SEP-10, and it proves control through its own `__check_auth`, which only
 * the Soroban host runs: the server's final check is always a simulation.
 */
class Sep45Challenge
{
public:
    // ─── SEP-45 v0.1.1 ───────────────────────────────────────────────

    /** The account the call is simulated from, as py does: the all-zero key,
     *  "GAAA…AWHF". The simulated transaction is never submitted. */
    static const QString NULL_ACCOUNT;
    /** How long the server's signature lasts unless told otherwise, in
     *  ledgers past the latest one: 180, about 15 minutes, as py. */
    static const quint32 DEFAULT_EXPIRE_IN_LEDGERS;

    /** A SEP-45 v0.1.1 challenge, read and checked. Optional values are empty
     *  when the challenge does not carry them. */
    struct ChallengeAuthorizationEntries
    {
        /** The entries as they came, in their order. */
        QList<stellar::SorobanAuthorizationEntry> authorizationEntries;
        QString clientAccountId;      // "account": the client's contract (C...)
        QString matchedHomeDomain;    // "home_domain", one of those expected
        QString nonce;                // "nonce": a server keeps it to refuse a replay
        QString webAuthDomain;        // "web_auth_domain"
        QString serverAccountId;      // "web_auth_domain_account": the SIGNING_KEY (G...)
        QString webAuthContract;      // the contract called (C...)
        QString clientDomain;         // "client_domain"
        QString clientDomainAccount;  // "client_domain_account": its SIGNING_KEY (G...)
        /** The last ledger the server's signature is good for. */
        quint32 serverSignatureExpirationLedger = 0;
    };

    /**
     * The transaction a server simulates to learn the challenge's entries —
     * py's _build_challenge_tx: one `web_auth_verify` call on
     * `webAuthContract`, from `simulateTxAccount` at sequence 1, time bounds
     * {0, 0}, fee 100, unsigned. Returned as a base64 envelope, the same bytes
     * py builds.
     *
     * Refused: a client that is not a contract (C...), a web-auth contract
     * that is not one, a server or client-domain account that is not an
     * account id (G...), a blank home domain, web-auth domain or nonce, and a
     * client domain without its account or the other way round.
     */
    static QString challengeTransaction(const QString& serverAccountId,
                                        const QString& clientAccountId,
                                        const QString& homeDomain,
                                        const QString& webAuthDomain,
                                        const QString& webAuthContract,
                                        Network* network,
                                        const QString& nonce,
                                        const QString& clientDomain = QString(),
                                        const QString& clientDomainAccount = QString(),
                                        const QString& simulateTxAccount = NULL_ACCOUNT);

    /**
     * The server's half once the simulation is in — py's
     * _process_build_challenge_response, with the checks it leaves out.
     *
     * `simulatedEntries` are the base64 auth entries the simulation of
     * challengeTransaction(...) returned, `latestLedger` its latest ledger.
     * They must be exactly the challenge asked for: one entry each for the
     * client, the server and (when asked) the client domain account, each
     * unsigned, each with address credentials authorizing the web_auth_verify
     * call built from these arguments and nothing under it. Anything else is
     * refused rather than signed: py signs whatever entry the RPC names the
     * server in. The server's entry is then signed with `serverSigner`, valid
     * until latestLedger + expireInLedgers, and the entries are returned, in
     * the simulation's order, as the base64 SorobanAuthorizationEntries a
     * challenge endpoint answers with.
     */
    static QString signSimulatedChallengeEntries(const QStringList& simulatedEntries,
                                                 quint32 latestLedger,
                                                 KeyPair* serverSigner,
                                                 const QString& clientAccountId,
                                                 const QString& homeDomain,
                                                 const QString& webAuthDomain,
                                                 const QString& webAuthContract,
                                                 Network* network,
                                                 const QString& nonce,
                                                 quint32 expireInLedgers = DEFAULT_EXPIRE_IN_LEDGERS,
                                                 const QString& clientDomain = QString(),
                                                 const QString& clientDomainAccount = QString());

    /**
     * Builds a SEP-45 challenge, as py's build_challenge_authorization_entries:
     * simulates challengeTransaction(...) with `sorobanServer` and hands the
     * answer to signSimulatedChallengeEntries(). An empty `nonce` is 48 random
     * bytes in base64. Throws on a failed simulation and on one that does not
     * return exactly the entries asked for.
     */
    static QString buildChallengeAuthorizationEntries(SorobanServer* sorobanServer,
                                                      KeyPair* serverSigner,
                                                      const QString& clientAccountId,
                                                      const QString& homeDomain,
                                                      const QString& webAuthDomain,
                                                      const QString& webAuthContract,
                                                      Network* network,
                                                      const QString& nonce = QString(),
                                                      quint32 expireInLedgers = DEFAULT_EXPIRE_IN_LEDGERS,
                                                      const QString& clientDomain = QString(),
                                                      const QString& clientDomainAccount = QString(),
                                                      const QString& simulateTxAccount = NULL_ACCOUNT);

    /**
     * The same challenge without a round trip to an RPC: the entries the
     * reference web-auth contract's simulation returns (the client's, the
     * server's and, when asked, the client domain's, each with a fresh random
     * credential nonce) are built here and signed as above, with the server's
     * signature valid until latestLedger + expireInLedgers. For a server that
     * already knows the latest ledger; a contract whose web_auth_verify asks
     * for other authorizations than the reference one's needs the simulating
     * overload.
     */
    static QString buildChallengeAuthorizationEntries(KeyPair* serverSigner,
                                                      const QString& clientAccountId,
                                                      const QString& homeDomain,
                                                      const QString& webAuthDomain,
                                                      const QString& webAuthContract,
                                                      Network* network,
                                                      quint32 latestLedger,
                                                      const QString& nonce = QString(),
                                                      quint32 expireInLedgers = DEFAULT_EXPIRE_IN_LEDGERS,
                                                      const QString& clientDomain = QString(),
                                                      const QString& clientDomainAccount = QString());

    /**
     * Reads a SEP-45 challenge the way a client must before signing it, and a
     * server before verifying it — py's read_challenge_authorization_entries,
     * and also the server's signature, which the SEP asks the client to check
     * and py leaves to the final simulation. Throws
     * `InvalidSep45ChallengeException` on anything but a challenge:
     *   - input that is not base64 XDR SorobanAuthorizationEntries, whole;
     *   - entries that do not all authorize the same call, a call with
     *     sub-invocations, or a call other than web_auth_verify on
     *     `webAuthContract`;
     *   - an argument that is not one map of symbols to strings, sorted and
     *     without repeats, with every key the SEP requires, no key it does not
     *     define, and client_domain with client_domain_account or neither;
     *   - an account that is not a contract (C...), a web_auth_domain_account
     *     that is not `serverAccountId`, a web_auth_domain that is not
     *     `webAuthDomain`, a home_domain not in `homeDomains` (each compared
     *     exactly);
     *   - an entry whose credentials are not address credentials, an entry for
     *     anybody but the server, the client and the client domain account, a
     *     missing or repeated one;
     *   - a server entry whose signature is not exactly one ed25519 signature
     *     by `serverAccountId` over that entry on `network`.
     * The client's and the client domain's entries are not required to be
     * signed: they are what the wallet signs next.
     */
    static ChallengeAuthorizationEntries readChallengeAuthorizationEntries(
        const QString& challengeAuthorizationEntries,
        const QString& serverAccountId,
        const QStringList& homeDomains,
        const QString& webAuthDomain,
        const QString& webAuthContract,
        Network* network);

    /**
     * The wallet's half: reads the challenge exactly as
     * readChallengeAuthorizationEntries() does, requires it to be for
     * `clientAccountId`, and only then signs, as py's authorize_entry does with
     * a keypair: the client's entry with `clientSigner` and, when given, the
     * client domain account's entry with `clientDomainSigner`, each valid until
     * `validUntilLedger` (a near ledger: the SEP suggests the current one plus
     * one). Returns the entries, the others unchanged, for the token endpoint.
     *
     * The client's signature takes the default account shape, a vec of one
     * {public_key, signature} map: what a contract account whose __check_auth
     * expects an ed25519 key accepts. Either signer may be null, not both.
     *
     * `clientDomain` and `clientDomainAccount` (together or neither) are the
     * client domain the wallet asked for and the SIGNING_KEY of its
     * stellar.toml; the challenge must then carry exactly those. Not given, a
     * client domain in the challenge is accepted as issued and its entry left
     * for its own key to sign. `clientDomainSigner` requires them and must be
     * the key of `clientDomainAccount`.
     */
    static QString signChallengeAuthorizationEntries(const QString& challengeAuthorizationEntries,
                                                     const QString& serverAccountId,
                                                     const QStringList& homeDomains,
                                                     const QString& webAuthDomain,
                                                     const QString& webAuthContract,
                                                     const QString& clientAccountId,
                                                     KeyPair* clientSigner,
                                                     quint32 validUntilLedger,
                                                     Network* network,
                                                     const QString& clientDomain = QString(),
                                                     const QString& clientDomainAccount = QString(),
                                                     KeyPair* clientDomainSigner = nullptr);

    /**
     * The wallet's last check, which the SEP asks for and py leaves out: once
     * signed, the entries are simulated in verificationTransaction() with
     * `sorobanServer`, and the call must succeed and have no side effect but
     * using up the challenge's own nonces (checkSimulationFootprint()). The
     * wallet then posts the entries. Throws InvalidSep45ChallengeException
     * otherwise, and what SorobanServer throws when it cannot be reached.
     */
    static ChallengeAuthorizationEntries simulateSignedChallenge(SorobanServer* sorobanServer,
                                                                 const QString& signedEntries,
                                                                 const QString& serverAccountId,
                                                                 const QStringList& homeDomains,
                                                                 const QString& webAuthDomain,
                                                                 const QString& webAuthContract,
                                                                 Network* network,
                                                                 const QString& simulateTxAccount = NULL_ACCOUNT);

    /**
     * The footprint half of simulateSignedChallenge(), for a caller that ran
     * the simulation itself: `transactionData`, the base64
     * SorobanTransactionData it returned, may write only the nonce entries
     * (contract data keyed SCV_LEDGER_KEY_NONCE, temporary) of the challenge's
     * client, server and client domain account, each for the nonce of that
     * party's entry, and the web-auth contract's instance when it had been
     * archived (SEP-45 v0.1.1). Since Protocol 23 the simulation restores an
     * archived contract by itself, and its wasm (CONTRACT_CODE) comes in the
     * write set with it: a CONTRACT_CODE entry is accepted when
     * archivedSorobanEntries lists it (a wasm is immutable, restoring it
     * changes nothing). Anything else written is a side effect the wallet did
     * not ask for: InvalidSep45ChallengeException.
     */
    static void checkSimulationFootprint(const ChallengeAuthorizationEntries& challenge,
                                         const QString& transactionData);

    /**
     * The transaction a server simulates to verify a challenge — py's
     * _build_verify_tx: the web_auth_verify call rebuilt from the arguments
     * read, carrying the entries returned, from `simulateTxAccount` at
     * sequence 1, time bounds {0, 0}, fee 100. A base64 envelope, the same
     * bytes py builds. Exposed so a caller with its own RPC client (the QML
     * layer) can run the simulation.
     */
    static QString verificationTransaction(const ChallengeAuthorizationEntries& challenge,
                                           Network* network,
                                           const QString& simulateTxAccount = NULL_ACCOUNT);

    /**
     * The server's check of a signed challenge — py's
     * verify_challenge_authorization_entries: readChallengeAuthorizationEntries()
     * (which also checks the server's own signature, so the entries are the
     * ones it issued), then a simulation of verificationTransaction(): it
     * succeeds only when the client contract's __check_auth, and the client
     * domain's signature when there is one, accept the entries. Throws
     * `InvalidSep45ChallengeException` on anything else, a simulation that
     * reports an error or returns no result included; an RPC that cannot be
     * reached throws what SorobanServer throws. `sorobanServer` is required.
     *
     * Whether the nonce was issued and not used before is the server's to
     * check: it is returned for that.
     */
    static ChallengeAuthorizationEntries verifyChallengeAuthorizationEntries(
        SorobanServer* sorobanServer,
        const QString& challengeAuthorizationEntries,
        const QString& serverAccountId,
        const QStringList& homeDomains,
        const QString& webAuthDomain,
        const QString& webAuthContract,
        Network* network,
        const QString& simulateTxAccount = NULL_ACCOUNT);

    // ─── This SDK's earlier format (deprecated) ──────────────────────
    //
    // Everything below is the pre-1.0.4, non-standard exchange described in
    // the class comment, kept unchanged for the 1.0 API. It does not
    // interoperate with SEP-45 v0.1.1: use the functions above instead.

    /** Five-minute grace window for a client clock behind the server's:
     *  a challenge is accepted up to this long before its minTime, never
     *  after its maxTime or expiration. */
    static const int GRACE_PERIOD_SECONDS;
    /** The longest challenge newChallenge builds: one day. */
    static const qint64 MAX_TIMEOUT_SECONDS;
    /** The function a challenge calls unless told otherwise. */
    static const QString DEFAULT_WEB_AUTH_FUNCTION_NAME;

    /**
     * Build a challenge in this SDK's earlier format and return it as a base64
     * envelope already signed by `serverSigner`.
     *
     * @deprecated Not SEP-45: no SEP-45 v0.1.1 client can read it. Use
     * buildChallengeAuthorizationEntries().
     *
     * @param serverSigner     server-side signing keypair (the anchor).
     * @param network          target network.
     * @param clientAccountId  client's contract account (C-strkey).
     * @param webAuthContract  contract address (C-strkey) that hosts the
     *                         `webAuthFunctionName` entry point used by
     *                         this anchor.
     * @param webAuthFunctionName  function on `webAuthContract` to invoke,
     *                         usually DEFAULT_WEB_AUTH_FUNCTION_NAME. A
     *                         verifier checks it: another name needs the
     *                         verifyChallenge overload that takes it.
     * @param homeDomain       SEP-1 home domain (not blank).
     * @param webAuthDomain    the host that handles the auth endpoint (not blank).
     * @param timeoutSec       validity window, 1 to MAX_TIMEOUT_SECONDS.
     * @param nonceBytes       optional fixed nonce — pass empty for a
     *                         freshly generated random one (48 bytes).
     *                         Tests pass a fixed nonce for determinism.
     */
    static QString newChallenge(KeyPair* serverSigner,
                                Network* network,
                                const QString& clientAccountId,
                                const QString& webAuthContract,
                                const QString& webAuthFunctionName,
                                const QString& homeDomain,
                                const QString& webAuthDomain,
                                qint64 timeoutSec = 300,
                                const QByteArray& nonceBytes = QByteArray());

    /** Result of a successful `verifyChallenge`. */
    struct VerifiedChallenge
    {
        QString clientAccountId;     // C-strkey of the verified client
        QString matchedHomeDomain;   // which of `homeDomains` matched
        QByteArray nonce;            // the 48-byte nonce
        qint64 expirationUnix = 0;
        QString webAuthContract;     // C-strkey of the contract the challenge calls
        QString webAuthFunctionName; // the function it calls there
    };

    /**
     * Statically verify a client-signed challenge envelope in this SDK's
     * earlier format, then simulate it against `sorobanServer` to confirm the
     * client's auth chain satisfies `__check_auth`. Throws
     * `InvalidSep45ChallengeException` on any failure.
     *
     * @deprecated Not SEP-45: a SEP-45 v0.1.1 challenge is refused. Use
     * readChallengeAuthorizationEntries() (wallet) or
     * verifyChallengeAuthorizationEntries() (server).
     *
     * Refused, among others: a challenge not signed by the server, outside
     * its time bounds (grace before minTime only) or past its expiration;
     * one that does not call DEFAULT_WEB_AUTH_FUNCTION_NAME; one whose auth
     * entry authorizes anything but exactly the challenge's own call, or
     * has sub-invocations; a blank webAuthDomain. This overload does not
     * check WHICH contract is called — a wallet should use the one below,
     * with the web-auth contract it trusts.
     *
     * If `sorobanServer` is nullptr, the simulation step is skipped — the
     * static checks still run. This mode is intended for unit tests that
     * just want to exercise the structural validation.
     */
    static VerifiedChallenge verifyChallenge(const QString& challengeXdr,
                                             const QString& serverAccountId,
                                             Network* network,
                                             SorobanServer* sorobanServer,
                                             const QStringList& homeDomains,
                                             const QString& webAuthDomain);

    /**
     * The same checks, and also that the challenge calls `webAuthFunctionName`
     * on `webAuthContract` (a C-strkey, required: a blank or malformed one is
     * refused, not read as "any contract").
     *
     * @deprecated As the overload above.
     */
    static VerifiedChallenge verifyChallenge(const QString& challengeXdr,
                                             const QString& serverAccountId,
                                             Network* network,
                                             SorobanServer* sorobanServer,
                                             const QStringList& homeDomains,
                                             const QString& webAuthDomain,
                                             const QString& webAuthContract,
                                             const QString& webAuthFunctionName = DEFAULT_WEB_AUTH_FUNCTION_NAME);
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(Sep45Challenge)

#endif // SEP45CHALLENGE_H
