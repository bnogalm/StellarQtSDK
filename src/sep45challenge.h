#ifndef SEP45CHALLENGE_H
#define SEP45CHALLENGE_H

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QtGlobal>

#include "keypair.h"
#include "network.h"
#include "qstellar_namespace.h"
QSTELLAR_FWD(SorobanServer)

QSTELLAR_BEGIN_NS


/**
 * SEP-45 — Stellar Web Authentication for contract accounts.
 *
 * Parallels SEP-10 but the client is a Soroban contract account
 * (C-strkey) rather than a classic Stellar account (G-strkey). The
 * challenge is an unsigned `InvokeHostFunction` transaction with one
 * `SorobanAuthorizationEntry` whose credentials reference the client
 * contract. The client proves control by attaching a signature whose
 * shape its own `__check_auth` implementation accepts.
 *
 * Verification combines static checks (server signature, timebounds,
 * domain encoded in the call args, nonce length) with a simulation
 * against the network: if `simulateTransaction` succeeds, the contract's
 * `__check_auth` has accepted the client's auth chain.
 *
 * Symmetry with `Sep10Challenge`:
 *   - `newChallenge(...)` → base64 envelope (unsigned by the client).
 *   - `verifyChallenge(...)` → `VerifiedChallenge` or throws
 *     `qstellar::exception::InvalidSep45ChallengeException`.
 */
class Sep45Challenge
{
public:
    /** Five-minute grace window for clock skew. */
    static const int GRACE_PERIOD_SECONDS;

    /**
     * Build a SEP-45 challenge tx and return it as a base64 envelope
     * already signed by `serverSigner`.
     *
     * @param serverSigner     server-side signing keypair (the anchor).
     * @param network          target network.
     * @param clientAccountId  client's contract account (C-strkey).
     * @param webAuthContract  contract address (C-strkey) that hosts the
     *                         `webAuthFunctionName` entry point used by
     *                         this anchor.
     * @param webAuthFunctionName  function on `webAuthContract` to invoke
     *                         (typically `"web_auth_verify"` per the
     *                         current SEP-45 reference; left configurable
     *                         so spec churn doesn't break the API).
     * @param homeDomain       SEP-1 home domain.
     * @param webAuthDomain    the host that handles the auth endpoint.
     * @param timeoutSec       validity window (must be > 0).
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
    };

    /**
     * Statically verify a client-signed challenge envelope, then
     * simulate it against `sorobanServer` to confirm the client's auth
     * chain satisfies `__check_auth`. Throws
     * `InvalidSep45ChallengeException` on any failure.
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
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(Sep45Challenge)

#endif // SEP45CHALLENGE_H
