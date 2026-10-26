#ifndef QSTELLAR_SEPSQML_H
#define QSTELLAR_SEPSQML_H

#include <QHash>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QStringList>
#include <QVariantMap>

#include "qstellar_namespace.h"

class QNetworkReply;

QSTELLAR_BEGIN_NS

/**
 * QML facade over the SEPs this SDK implements.
 *
 * A SEP is a Stellar Ecosystem Proposal: a convention agreed between wallets,
 * exchanges and anchors for something the protocol deliberately does not
 * specify. The ledger does not care how you discover an anchor's endpoints, how
 * a payment request travels in a link, or how a website checks that you hold a
 * key; the SEPs are how everybody agreed to do it anyway. **Nothing here is
 * enforced by a validator** — skip every one of these conventions and the
 * network will still apply your transaction.
 *
 * NEARLY EVERYTHING HERE IS OFFLINE AND SYNCHRONOUS, which is why most of it
 * needs no thread and no request ids. That is not this wrapper being cautious —
 * it is what the SDK implements, and where a SEP is half-implemented the page
 * says which half. SEP-1 parses a stellar.toml and does not fetch one; SEP-10
 * and SEP-45 have all of the challenge cryptography and none of the HTTP that
 * makes it a login.
 *
 * The one exception is SEP-2, at the bottom: federation is two chained HTTPS
 * round trips and there is no offline form of the answer. It uses the same
 * request-id-plus-signal shape as the Horizon facade, so no SDK object ever
 * crosses into QML and a stale reply cannot be mistaken for the current one.
 * It needs no worker thread — the SDK's federation classes are built on
 * QNetworkAccessManager and never block — which is what separates it from the
 * Soroban facade.
 *
 * Same rules as the rest of the layer: nothing throws into QML, no SDK pointer
 * crosses the boundary, and every failure comes back as a map with an "error"
 * key carrying the SDK's own message — because those messages ("`memo` requires
 * `memo_type`", "unterminated string", "Malformed address") are the most
 * instructive part of the whole exercise.
 *
 * Every int64 crosses as a STRING. Sequence numbers, TOIDs, muxed ids and
 * stroop amounts all reach values a JavaScript number cannot hold exactly, and
 * a silently rounded id is worse than no id.
 *
 * Registered as the `Seps` singleton by QStellarQml::registerTypes().
 */
class SepsQml : public QObject
{
    Q_OBJECT
public:
    explicit SepsQml(QObject* parent = nullptr);
    ~SepsQml() override;

    // ─── SEP-1: stellar.toml ─────────────────────────────────────────

    /**
     * Parses a stellar.toml into { values } or { error }.
     *
     * Parse only: no fetch. All of TOML 1.0 is read (StellarToml::parse:
     * tables, arrays of tables, inline tables, dotted keys; dates come back as
     * their text), and anything else is { error } naming the line.
     *
     * An integer JavaScript cannot hold exactly (beyond ±2^53) comes back as
     * its decimal text: as a number it was rounded, a balance or an id
     * included, without a word.
     */
    Q_INVOKABLE QVariantMap parseToml(const QString& text) const;

    /** The keys a reader usually wants, in the order worth showing them. */
    Q_INVOKABLE QStringList wellKnownTomlKeys() const;

    /**
     * The same parse, flattened to { rows: [{ path, value, kind }] }.
     *
     * `values` is a nested map, and a nested map does not render: the page was
     * showing eight well-known scalars and silently dropping `[DOCUMENTATION]`
     * and every `[[CURRENCIES]]` entry — which is most of what a real
     * stellar.toml contains. Paths read `DOCUMENTATION.ORG_NAME` and
     * `CURRENCIES[0].code`, so what came out of the file is visible whole.
     */
    Q_INVOKABLE QVariantMap parseTomlRows(const QString& text) const;

    /**
     * Just the keys that identify somebody, for wiring SEP-1 into SEP-7 and
     * SEP-10: { signingKey, uriRequestSigningKey, networkPassphrase,
     * webAuthEndpoint, federationServer }.
     *
     * SIGNING_KEY and URI_REQUEST_SIGNING_KEY are the two accounts a toml
     * exists to publish. A key the toml does not publish is absent from the
     * map. One of the two that is present but is not an account id (G...,
     * spelt exactly, checksum and all; an empty string included) makes the
     * whole answer { error } naming it, and the other values, a good
     * SIGNING_KEY too, are not handed on: the call fails closed rather than
     * pass on half of a damaged toml. js-stellar-sdk and py-stellar-sdk have
     * no such call (their SEP-1 resolvers return the toml unchecked, and
     * their SEP-10 and SEP-7 code checks the key it uses, as this SDK's
     * does); read one value regardless with parseToml().
     */
    Q_INVOKABLE QVariantMap tomlSigningKeys(const QString& text) const;

    // ─── SEP-7: the URI scheme ───────────────────────────────────────

    /**
     * Builds a `web+stellar:pay` URI from { destination, amount, assetCode,
     * assetIssuer, memo, memoType, callback, msg, networkPassphrase,
     * originDomain }, returning { uri } or { error }.
     */
    Q_INVOKABLE QVariantMap buildPayUri(const QVariantMap& args) const;

    /** Builds a `web+stellar:tx` URI around an envelope. */
    Q_INVOKABLE QVariantMap buildTxUri(const QVariantMap& args) const;

    /**
     * Reads either kind back, returning every field it carried plus `kind`.
     *
     * The rejections are the interesting part and are passed through verbatim:
     * a `memo` with no `memo_type` is refused, for instance, because toUri()
     * would not re-emit it — so it would ride along outside the signed form,
     * which is exactly the trick a signature is meant to prevent.
     */
    Q_INVOKABLE QVariantMap parseUri(const QString& uri) const;

    /** Signs a URI with a secret seed, returning { uri, signature } or { error }. */
    Q_INVOKABLE QVariantMap signUri(const QString& uri, const QString& secretSeed) const;

    /**
     * Checks a URI's signature against an account id.
     *
     * Returns { valid: true|false } — a wrong signature is an ANSWER, not an
     * error. { error } is reserved for a URI that could not be read at all.
     */
    Q_INVOKABLE QVariantMap verifyUri(const QString& uri, const QString& accountId) const;

    /**
     * The check a wallet really performs: verify the URI against the key a
     * pasted stellar.toml publishes, rather than against a key typed by hand.
     *
     * It is Sep7::verifyOrigin: the URI names an origin_domain that is a
     * fully qualified domain name, it is signed, and by the toml's
     * URI_REQUEST_SIGNING_KEY, an account id spelled exactly (as
     * tomlSigningKeys checks it). Returns { valid: true, signed: true,
     * originDomain, keyUsed, fetched: false } (originDomain in its ACE
     * form, xn--... for an internationalised name, as Sep7::originDomain
     * returns it), or { error } saying which
     * check failed; a toml with no URI_REQUEST_SIGNING_KEY at all is
     * { hasUriRequestSigningKey: false, valid: false, fetched: false, error }.
     * The `fetched: false` is not padding: a wallet reads the toml from the
     * URI's own `origin_domain` (Sep7OriginCheck does, in C++), and here it
     * was pasted, so nothing ties the key to that domain.
     */
    Q_INVOKABLE QVariantMap verifyUriAgainstToml(const QString& uri,
                                                 const QString& tomlText) const;

    // ─── SEP-5: keys from a mnemonic ─────────────────────────────────

    /**
     * Says what is wrong with a phrase, rather than just that something is.
     *
     * Returns { valid, wordCount, unknownWords, reason }. `validateMnemonic`
     * answers a bare bool, which cannot distinguish a typo in one word from a
     * phrase of the wrong length from a correct-looking phrase whose checksum
     * fails — and those have completely different fixes.
     */
    Q_INVOKABLE QVariantMap inspectMnemonic(const QString& mnemonic) const;

    /**
     * Derives several accounts from one phrase at once:
     * { bip39SeedHex, accounts: [{ index, path, accountId, secretSeed }],
     *   passphraseUsed, legacyPassphraseDiffers }.
     *
     * One phrase is a whole wallet, not one key — that is the entire point of
     * SEP-5 and it is invisible when you can only see index 0. The path is
     * `m/44'/148'/N'`, hardened all the way down, which is why there is no way
     * to derive a public key without the secret, and why N stops at 2^31 - 1.
     *
     * The passphrase is read as BIP-39 reads it, in NFKD. 1.0.3 and earlier
     * read it as typed: `legacyPassphraseDiffers` is true when that makes a
     * difference, and deriveAccountRangeLegacyPassphrase then finds the
     * accounts they derived.
     *
     * The phrase must be a valid BIP-39 phrase, checksum included, as
     * Stellar.keyPairFromMnemonic, py and js require: a phrase with a typo
     * is refused (inspectMnemonic() says why), not derived into someone
     * else's wallet. `first` and `count` are read exactly (a JS number is
     * not narrowed).
     */
    Q_INVOKABLE QVariantMap deriveAccountRange(const QString& mnemonic,
                                               const QVariant& first, const QVariant& count,
                                               const QString& passphrase = QString()) const;

    /**
     * deriveAccountRange as 1.0.0-1.0.3 derived it, for recovering wallets
     * created with those versions; NOT BIP-39. They used the passphrase as
     * typed rather than in NFKD, so for a passphrase whose NFKD form differs
     * (a precomposed accented letter, a compatibility character such as a
     * ligature) the accounts are others than deriveAccountRange, or any
     * other BIP-39 wallet, finds (see
     * Util::mnemonicToBIP39SeedLegacyPassphrase); for an ASCII one they are
     * the same. The phrase is read as deriveAccountRange reads it. Returns
     * deriveAccountRange's map plus `legacy: true`; `legacyPassphraseDiffers`
     * says whether the two derivations give different accounts. Offer it to
     * find and move old funds, never to create a wallet.
     */
    Q_INVOKABLE QVariantMap deriveAccountRangeLegacyPassphrase(const QString& mnemonic,
                                                               const QVariant& first, const QVariant& count,
                                                               const QString& passphrase = QString()) const;

    // ─── SEP-53: signed messages ─────────────────────────────────────

    /**
     * Signs an arbitrary message with a Stellar secret seed.
     *
     * Returns { accountId, hash, signature, signatureHex, messageBytes }.
     * `encoding` is "utf8" (default), "hex" or "base64" — a message is bytes,
     * and the SEP hashes bytes, so a page that could only sign text would be
     * unable to reproduce half the published test vectors.
     */
    Q_INVOKABLE QVariantMap signMessage(const QVariantMap& args) const;

    /** { valid, hash } — a false is an answer, an { error } is a bad input. */
    Q_INVOKABLE QVariantMap verifyMessage(const QVariantMap& args) const;

    /**
     * The domain-separated hash on its own: { hash, prefix, preimageHex }.
     *
     * Showing the preimage is the whole lesson. The prefix "Stellar Signed
     * Message:\n" is what stops a website from handing you a "message" that is
     * really a transaction hash and collecting a signature that spends money.
     */
    Q_INVOKABLE QVariantMap messageHash(const QString& message,
                                        const QString& encoding = QString()) const;

    // ─── SEP-11: TxRep ───────────────────────────────────────────────

    /**
     * Envelope to TxRep: { txrep, hash, envelopeType, isFeeBump,
     * signatureCount, fee, operationCount, ... }, for every envelope type
     * (Txrep::toTxrep): a V0 document says ENVELOPE_TYPE_TX_V0, and a fee
     * bump's is the fee bump's own, signatures included. A plain transaction
     * adds { sourceAccount, sequence }; a fee bump adds { feeSource } (fee
     * and signatureCount being its own) and the transaction it wraps under
     * inner names only: { innerHash, innerSourceAccount, innerSequence,
     * innerFee, innerSignatureCount }. Fees and sequences are strings.
     */
    Q_INVOKABLE QVariantMap txrepFromXdr(const QVariantMap& args) const;

    /**
     * TxRep back to the envelope it describes (Txrep::fromTxrepEnvelope),
     * fee bump and V0 included, with the signatures the document carries:
     * { xdr } plus what txrepFromXdr returns about it, or { error }.
     */
    Q_INVOKABLE QVariantMap xdrFromTxrep(const QVariantMap& args) const;

    /**
     * Both directions at once, comparing what came back:
     * { matched, bytesMatched, hashIn, hashOut, xdrIn, xdrOut, txrep,
     *   signaturesIn, signaturesOut, signaturesDropped, carries }.
     *
     * A round trip is the only honest way to show what TxRep does and does not
     * carry. `matched` compares hashes, which cover the transaction and not
     * the signatures on it; `bytesMatched` the envelopes, which TxRep carries
     * whole, signatures included.
     */
    Q_INVOKABLE QVariantMap txrepRoundTrip(const QVariantMap& args) const;

    // ─── SEP-10: web authentication ──────────────────────────────────

    /**
     * Builds the challenge a server would send:
     * { xdr, serverAccountId, clientAccountId, homeDomain, webAuthDomain,
     *   sequence, fee, ... }.
     *
     * Takes { serverSeed, clientAccountId, homeDomain, webAuthDomain (all
     * required), timeoutSeconds (1 to 86400, default 300), network }. The
     * client account is a G address or a muxed M one. The sequence is 0 and that is the entire
     * safety argument: an account's sequence starts far above zero and only
     * ever increases, so this envelope is valid nowhere, forever.
     */
    Q_INVOKABLE QVariantMap buildChallenge(const QVariantMap& args) const;

    /**
     * Reads a challenge the way a client must before signing it:
     * { clientAccountId, matchedHomeDomain, sequence, expiresInSeconds, ... }.
     *
     * This is the check never to skip. A challenge whose sequence is not 0, or
     * whose source is not the key the domain's toml published, is not a login —
     * it is somebody asking you to sign a live transaction.
     *
     * For a muxed client the map adds clientBaseAccountId (the G account whose
     * signers log it in) and clientMuxedId. A challenge's client_domain
     * operation is always reported, as { clientDomain, clientDomainSigningKey,
     * clientDomainChecked }. It is checked when the client asked for a client
     * domain and says so with { clientDomain, clientDomainSigningKey } — the
     * domain and the SIGNING_KEY of its stellar.toml, the two together: the
     * operation must then name that domain, from that key. A challenge with
     * two client_domain operations is refused.
     */
    Q_INVOKABLE QVariantMap readChallenge(const QVariantMap& args) const;

    /**
     * Validates a challenge exactly as readChallenge does and only then adds
     * the client's signature:
     * { xdr, signatureCount, signedBy, clientAccountId, matchedHomeDomain, hash,
     *   memoId (when the challenge carries one), and what readChallenge adds
     *   for a muxed client and a client_domain operation },
     * or { error }, in which case nothing was signed.
     *
     * The challenge must carry the server's signature and nothing else: any
     * other signature is one the server would refuse next to the client's, so
     * it is refused here, and a key that has already signed is refused rather
     * than adding a duplicate. Signatures the caller expects there, from a
     * co-signer of a multisig account or the client domain's key, are named
     * in alreadySignedBy; each must then be there, once.
     *
     * Takes { xdr, secretSeed, serverAccountId, homeDomain | homeDomains,
     * webAuthDomain, network | networkPassphrase } - all required except the
     * network - plus, optionally:
     *   clientAccountId: the account being logged in, which defaults to the
     *     signer's own and must be the challenge's. A signer of a multisig
     *     account passes the account it signs for, and a muxed M account is
     *     named here: its signers are those of its G account.
     *   clientDomain, clientDomainSigningKey: as readChallenge checks them.
     *   clientDomainSeed: signs for the client domain as well. Its account
     *     must be the client_domain operation's source, and clientDomain is
     *     required with it.
     *   alreadySignedBy: a list of G addresses, as above.
     * Any other key is refused.
     *
     * The validation cannot be skipped because the signature cannot be taken
     * back: handed an unchecked envelope, this would sign a payment as readily
     * as a login.
     */
    Q_INVOKABLE QVariantMap signChallenge(const QVariantMap& args) const;

    /**
     * Which of the offered signers actually signed: { valid, signers, count }.
     *
     * A signer that did not sign is silently ignored rather than rejected,
     * which is what lets a multisig account present all its keys and have the
     * server work out which ones turned up. A challenge no offered signer
     * signed is { error }, never valid with a count of 0; the client_domain
     * key's signature does not count as a client's. For a muxed client the
     * signers offered are those of its G account.
     */
    Q_INVOKABLE QVariantMap verifySigners(const QVariantMap& args) const;

    /** The constants the SEP fixes, so the page quotes them rather than repeats them. */
    Q_INVOKABLE QVariantMap sep10Constants() const;

    // ─── SEP-23: strkey ──────────────────────────────────────────────

    /**
     * Takes a strkey apart: { kind, versionByte, bodyHex, length, ... }.
     *
     * Every other tab on the page quotes strkeys, and this is where a reader
     * finds out that a valid-looking address is a spell-check result and not
     * proof that anything exists. The CRC16 catches a typo; nothing here says
     * the account was ever funded.
     */
    Q_INVOKABLE QVariantMap decodeStrKey(const QString& value) const;

    /** Builds an M-address from a G-address and an id: { address, ... }. */
    Q_INVOKABLE QVariantMap muxedFromAccountId(const QString& accountId,
                                               const QString& muxedId) const;

    /** Splits an M-address back into { isMuxed, accountId, muxedId }. */
    Q_INVOKABLE QVariantMap splitMuxed(const QString& address) const;

    /** Every version byte the encoding defines, and whether this SDK handles it. */
    Q_INVOKABLE QVariantMap strKeyKinds() const;

    // ─── SEP-35: TOID ────────────────────────────────────────────────

    /** Packs { ledger, txIndex, opIndex } into { id, hex, ... }. */
    Q_INVOKABLE QVariantMap toidEncode(const QVariantMap& args) const;

    /** Unpacks a decimal id back into { ledger, txIndex, opIndex, hex }. */
    Q_INVOKABLE QVariantMap toidDecode(const QString& packed) const;

    /** The first and last id in one ledger — how a cursor pages a ledger. */
    Q_INVOKABLE QVariantMap toidLedgerRange(const QString& ledger) const;

    /** The bit budget: 32 bits of ledger, 20 of transaction, 12 of operation. */
    Q_INVOKABLE QVariantMap toidLimits() const;

    // ─── SEP-29: memo required ───────────────────────────────────────

    /**
     * The offline half of the check: walk an envelope and report what a
     * SEP-29-aware client would have to ask Horizon about.
     *
     * Returns { memoType, memoSatisfies, destinations, skippedMuxed,
     * consideredOperations }. The skipped list matters: a muxed destination
     * carries its own routing id, so the check does not apply and a client that
     * asked anyway would be adding a round trip for nothing.
     */
    Q_INVOKABLE QVariantMap sep29Inspect(const QString& envelopeXdr,
                                         const QString& networkPassphrase) const;

    /** The data entry an account sets to demand a memo, and its base64 value. */
    Q_INVOKABLE QVariantMap sep29Flag() const;

    // ─── SEP-45: Soroban web authentication ──────────────────────────
    //
    // SEP-45 v0.1.1, as py-stellar-sdk and the Anchor Platform speak it: the
    // challenge is `authorizationEntries`, a base64 SorobanAuthorizationEntries
    // in which the server, the client's contract account and (when asked) the
    // client domain each have an entry authorizing the same web_auth_verify
    // call, the server's already signed. (Until 1.0.4 these functions spoke
    // this SDK's own earlier format, which nothing else reads.)
    //
    // Everything here is offline. The two steps that need the network are a
    // simulation each, and the page runs them with Soroban.simulate():
    //   anchor:  sep45ChallengeTransaction → Soroban.simulate →
    //            newSep45Challenge({ ..., simulatedAuth, latestLedger })
    //            (or newSep45Challenge with latestLedger alone, no RPC at all)
    //   wallet:  inspectSep45Challenge → signSep45Challenge →
    //            sep45VerificationTransaction → Soroban.simulate →
    //            checkSep45Simulation
    //   anchor:  sep45VerificationTransaction → Soroban.simulate →
    //            verifySep45Challenge({ ..., simulation })
    // Every call refuses a key it does not read. Home domains are read as the
    // SEP-10 calls read them: `homeDomains` (a list, or one domain as a
    // string) or `homeDomain`.

    /**
     * The transaction an anchor simulates to learn a challenge's entries (py's
     * _build_challenge_tx): { xdr, nonce, sourceAccount, sequence, fee }.
     *
     * Takes { serverAccountId, clientAccountId (C...), homeDomain,
     * webAuthDomain, webAuthContract (C...), nonce (48 random bytes in base64
     * when left out, returned either way), clientDomain and
     * clientDomainAccount (together), network | networkPassphrase }. Simulate
     * `xdr` and hand what comes back to newSep45Challenge with the same
     * arguments and this nonce.
     */
    Q_INVOKABLE QVariantMap sep45ChallengeTransaction(const QVariantMap& args) const;

    /**
     * Issues a challenge: the entries, checked, with the server's signed:
     * { authorizationEntries, serverAccountId, clientAccountId, homeDomain,
     *   webAuthDomain, webAuthContract, nonce, clientDomain,
     *   clientDomainAccount, serverSignatureExpirationLedger, entries,
     *   entryCount, simulated }.
     *
     * Takes { serverSeed, clientAccountId, homeDomain, webAuthDomain,
     * webAuthContract, latestLedger (all required), expireInLedgers (default
     * 180), nonce, clientDomain and clientDomainAccount (together), network |
     * networkPassphrase } and, optionally, simulatedAuth: the `auth` list
     * Soroban.simulate returned for sep45ChallengeTransaction's xdr, with its
     * latestLedger and the same nonce. Those entries must be exactly the
     * challenge asked for, or nothing is signed. Without simulatedAuth the
     * entries are built here, as the reference web-auth contract's simulation
     * returns them, with fresh credential nonces. The server's signature is
     * valid until latestLedger + expireInLedgers.
     */
    Q_INVOKABLE QVariantMap newSep45Challenge(const QVariantMap& args) const;

    /**
     * Reads a challenge the way a wallet must before signing it, including the
     * server's signature on its entry: { contract, function, clientAccountId,
     * matchedHomeDomain, webAuthDomain, serverAccountId, nonce, clientDomain,
     * clientDomainAccount, clientDomainChecked, serverSignatureExpirationLedger,
     * entries: [{ address, role, nonce, signatureExpirationLedger, signed }],
     * entryCount, structurallyValid, serverSignatureVerified, clientProven:
     * false, simulated: false }.
     *
     * Takes { authorizationEntries, serverAccountId, homeDomain | homeDomains,
     * webAuthDomain, webAuthContract (all required: webAuthContract is the
     * WEB_AUTH_CONTRACT_ID the domain's stellar.toml names), network |
     * networkPassphrase } and, checked when given, clientAccountId and the
     * pair clientDomain, clientDomainAccount.
     */
    Q_INVOKABLE QVariantMap inspectSep45Challenge(const QVariantMap& args) const;

    /**
     * Validates a challenge exactly as inspectSep45Challenge does and only
     * then signs, as py's authorize_entry does with a keypair: the client's
     * entry with secretSeed, the client domain's with clientDomainSeed, each
     * valid until validUntilLedger. Returns what inspectSep45Challenge does
     * for the signed challenge, plus { authorizationEntries, signedBy,
     * clientDomainSignedBy, validUntilLedger }, or { error }, in which case
     * nothing was signed.
     *
     * Takes { authorizationEntries, serverAccountId, homeDomain | homeDomains,
     * webAuthDomain, webAuthContract, clientAccountId (the contract logging
     * in), validUntilLedger (a near ledger: the SEP suggests the current one
     * plus one), network | networkPassphrase } and secretSeed,
     * clientDomainSeed or both; clientDomain and clientDomainAccount as
     * inspectSep45Challenge checks them (clientDomainSeed needs clientDomain,
     * and its account is clientDomainAccount).
     */
    Q_INVOKABLE QVariantMap signSep45Challenge(const QVariantMap& args) const;

    /**
     * The transaction an anchor simulates to verify a signed challenge (py's
     * _build_verify_tx), after reading it as inspectSep45Challenge does:
     * { xdr } plus what inspectSep45Challenge returns. Takes the same
     * arguments, without the expectations only a wallet has. A wallet
     * simulates the same transaction before posting the entries back.
     */
    Q_INVOKABLE QVariantMap sep45VerificationTransaction(const QVariantMap& args) const;

    /**
     * The wallet's check before it posts the signed entries back, which the
     * SEP asks for: `simulation` (what Soroban.simulate answered for
     * sep45VerificationTransaction's xdr) must have succeeded, must report
     * these very entries as its `auth` (so it is the simulation of the signed
     * challenge and not of another call), and must write nothing but the
     * challenge's own nonces, and the web-auth contract's instance if it was
     * archived. Returns { sideEffectFree: true } plus what
     * inspectSep45Challenge returns, or { error }. Takes the arguments of
     * sep45VerificationTransaction and `simulation`.
     */
    Q_INVOKABLE QVariantMap checkSep45Simulation(const QVariantMap& args) const;

    /**
     * The anchor's verdict: reads the challenge again and checks `simulation`,
     * the map Soroban.simulate delivered for sep45VerificationTransaction's
     * xdr: { verified: true, clientProven: true, simulated: true } plus what
     * inspectSep45Challenge returns, or { error } for a simulation that failed,
     * returned no result, or does not report these very entries as its `auth`
     * (a call carrying its own entries is simulated with them, and the RPC
     * reports them back; the anchor's first simulation, of the challenge
     * before it was signed, succeeds as well and is refused here). Only then
     * is a token due, and only once per nonce.
     */
    Q_INVOKABLE QVariantMap verifySep45Challenge(const QVariantMap& args) const;

    /** What SEP-45 v0.1.1 fixes: { functionName, argumentKeys, nullAccount,
     *  defaultExpireInLedgers }. */
    Q_INVOKABLE QVariantMap sep45Constants() const;

    // ─── SEP-2: federation ───────────────────────────────────────────

    /**
     * What would happen to this value, without doing it:
     * { kind: "address"|"accountId"|"malformed", name, domain, tomlUrl }.
     *
     * Worth its own call because `Federation::resolve` treats a bare G-address
     * as an answer rather than a question — it hands it straight back, having
     * checked its spelling and asked nobody — and that surprises people who
     * think they just learnt the account exists. "malformed" carries the
     * reason resolveFederation would refuse the value with.
     */
    Q_INVOKABLE QVariantMap describeAddress(const QString& value) const;

    /** Parses a federation server's JSON body offline, so the shape is
     *  visible. An answer resolveFederation would refuse (see there) is
     *  { error } here too. */
    Q_INVOKABLE QVariantMap parseFederationJson(const QString& json) const;

    /**
     * The real lookup: name*domain to an account id, over the network.
     *
     * Returns { requestId } at once; the answer arrives as resolved() or
     * failed(). A value refused before any request goes out — blank, more
     * than one '*', an empty name, a domain that is not a host name, a bare
     * value that is not an account id — returns { requestId, error } and
     * still gets its failed(). Two chained HTTPS round trips — the domain's stellar.toml
     * first, to find out where its federation server is, then the server
     * itself — so SEP-1 does the first half of SEP-2's work. Each is given up
     * on after STELLAR_QT_REPLY_TIMEOUT without an answer, with failed(). An
     * answer that cannot be paid as given — an account_id that is not a G or
     * M address, a memo_type other than text, id or hash, a memo invalid for
     * it — is failed() with kind "badResponse" and the reason. Any other
     * failure is kind "connection", and its message says which: the
     * stellar.toml could not be read (and why), it publishes no
     * FEDERATION_SERVER or one that is not https, the federation server
     * answered an HTTP error (404 for a name it does not know), or nothing
     * answered in time; js-stellar-sdk and py-stellar-sdk tell these apart
     * too. Before 1.0.4 they were all one fixed text.
     */
    Q_INVOKABLE QVariantMap resolveFederation(const QString& address);

    /** How many federation lookups are still in flight. */
    Q_PROPERTY(int pending READ pending NOTIFY pendingChanged)
    int pending() const;

signals:
    /** { stellarAddress, accountId, memoType, memo } for a completed lookup. */
    void resolved(int requestId, const QVariantMap& result);

    /** { kind, message } — same classification vocabulary as the Horizon facade. */
    void failed(int requestId, const QVariantMap& error);

    void pendingChanged();

private:
    int nextRequestId();
    void failWith(int requestId, const QString& kind, const QString& message);

    int m_nextRequestId = 1;
    QHash<int, QPointer<QObject>> m_inFlight;
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(SepsQml)

#endif // QSTELLAR_SEPSQML_H
