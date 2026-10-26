#include "sep45challenge.h"
#include <limits>

#include <QDataStream>
#include <QDateTime>
#include <QIODevice>
#include <cstring>
#include <memory>
#include <stdexcept>

#include "abstracttransaction.h"
#include "account.h"
#include "contract/sorobancredentialssigner.h"
#include "exception/invalidsep45challengeexception.h"
#include "invokehostfunctionoperation.h"
#include "keypair.h"
#include "network.h"
#include "responses/sorobanrpc/simulatetransactionresponse.h"
#include "scval/scv.h"
#include "sorobanserver.h"
#include "stellaraddress.h"
#include "strkey.h"
#include "transaction.h"
#include "transactionbuilder.h"
#include "util.h"
#include "xdr/scval.h"
#include "xdr/sorobanops.h"

QSTELLAR_BEGIN_NS


const int Sep45Challenge::GRACE_PERIOD_SECONDS = 5 * 60;
const qint64 Sep45Challenge::MAX_TIMEOUT_SECONDS = 86400;
const QString Sep45Challenge::DEFAULT_WEB_AUTH_FUNCTION_NAME = QStringLiteral("web_auth_verify");
const QString Sep45Challenge::NULL_ACCOUNT =
    QStringLiteral("GAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAWHF");
const quint32 Sep45Challenge::DEFAULT_EXPIRE_IN_LEDGERS = 12 * 15;

namespace {

constexpr int NONCE_SIZE = 48;

/** The XDR encoding of a value: two invocations are the same call exactly
 *  when these bytes are equal. */
template <typename T>
QByteArray xdrBytes(const T& value)
{
    QByteArray bytes;
    QDataStream out(&bytes, QIODevice::WriteOnly);
    out << value;
    return bytes;
}

/** Validates a strkey is a C-strkey (contract address). */
void requireContract(const QString& strkey, const char* what)
{
    if (StrKey::decodeVersionByte(strkey) != StrKey::VersionByte::CONTRACT) {
        throw std::runtime_error(QString("%1 must be a contract (C-strkey)").arg(what).toStdString());
    }
}

/** Build the args SCVec used both by the host function call and by the
 *  matching SorobanAuthorizedInvocation. */
QList<stellar::SCVal> buildArgs(const QString& clientAccountId,
                                const QString& homeDomain,
                                const QString& webAuthDomain,
                                const QByteArray& nonce,
                                qint64 expirationUnix)
{
    QList<stellar::SCVal> args;
    args.append(Scv::toAddress(clientAccountId));
    args.append(Scv::toString(homeDomain));
    args.append(Scv::toString(webAuthDomain));
    args.append(Scv::toBytes(nonce));
    args.append(Scv::toUint64(static_cast<quint64>(expirationUnix)));
    return args;
}

/** Deterministic Soroban auth-replay nonce derived from the SEP-45
 *  challenge nonce. Different namespace than the SCVal arg nonce. */
qint64 authNonceFromBytes(const QByteArray& nonce48)
{
    QByteArray h = Util::hash(nonce48);
    qint64 v = 0;
    for (int i = 0; i < 8 && i < h.size(); ++i) {
        v = (v << 8) | static_cast<quint8>(h.at(i));
    }
    return v;
}

/** True iff the challenge envelope carries a valid ed25519 signature from
 *  the server's signing key over the transaction hash. This is the
 *  server-side half of SEP-45 (mirrors Sep10Challenge::verifyTransactionSignature):
 *  the client's proof of control is the Soroban auth entry validated by
 *  `simulateTransaction`, but the envelope itself is signed ONLY by the
 *  server (see Sep45Challenge::newChallenge → `tx->sign(serverSigner)`), so
 *  this confirms the verifier is looking at a challenge it actually issued
 *  — not one forged with a different (or no) server key. Filter by the
 *  signature hint first, exactly as SEP-10 does. */
/**
 * Verify the server's own signature on a challenge envelope.
 *
 * The server signs BEFORE the client completes the challenge, and the client
 * completes it by filling in the auth entry's `signatureExpirationLedger` and
 * `signature` — fields that live inside the InvokeHostFunction operation body,
 * i.e. inside the transaction signature payload. Hashing the envelope the
 * client returns therefore produces a different hash than the one the server
 * signed, so the signature must be checked against the *as-issued* form:
 * re-parse the envelope and reset every ADDRESS-credentials auth entry to its
 * unsigned state before hashing.
 *
 * Only those two client-owned fields are excluded. Everything the client must
 * not touch — source account, sequence, timebounds, the host function with its
 * client / homeDomain / webAuthDomain / nonce / expiration args, and each auth
 * entry's address, nonce and rootInvocation — is still covered.
 */
bool serverSignedChallenge(const QString& challengeXdr, Network* network,
                           const QString& serverAccountId)
{
    QScopedPointer<AbstractTransaction> reissued(
        AbstractTransaction::fromEnvelopeXdr(challengeXdr, network));
    auto* asIssued = dynamic_cast<Transaction*>(reissued.data());
    if (!asIssued || asIssued->getOperations().isEmpty()) {
        return false;
    }
    auto* op = dynamic_cast<InvokeHostFunctionOperation*>(asIssued->getOperations().at(0));
    if (!op) {
        return false;
    }

    QList<stellar::SorobanAuthorizationEntry> auth = op->getAuth();
    for (stellar::SorobanAuthorizationEntry& e : auth) {
        switch (e.credentials.type) {
        case stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS:
        case stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS_V2:
            e.credentials.address.signatureExpirationLedger = 0;
            e.credentials.address.signature = stellar::SCVal();   // default is SCV_VOID
            break;
        case stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS_WITH_DELEGATES:
            e.credentials.addressWithDelegates.addressCredentials.signatureExpirationLedger = 0;
            e.credentials.addressWithDelegates.addressCredentials.signature = stellar::SCVal();
            e.credentials.addressWithDelegates.delegates.clear();
            break;
        default:
            break;  // SOURCE_ACCOUNT carries nothing the client fills in
        }
    }
    op->setAuth(auth);

    QScopedPointer<KeyPair> serverKey(KeyPair::fromAccountId(serverAccountId));
    const QByteArray txHash = asIssued->hash();
    const stellar::SignatureHint serverHint = serverKey->getSignatureHint();
    const QVector<stellar::DecoratedSignature> sigs = asIssued->getSignatures();
    for (const stellar::DecoratedSignature& ds : sigs) {
        if (!(ds.hint == serverHint)) {
            continue;
        }
        if (serverKey->verify(txHash, ds.signature.binary())) {
            return true;
        }
    }
    return false;
}

} // namespace

QString Sep45Challenge::newChallenge(KeyPair* serverSigner,
                                     Network* network,
                                     const QString& clientAccountId,
                                     const QString& webAuthContract,
                                     const QString& webAuthFunctionName,
                                     const QString& homeDomain,
                                     const QString& webAuthDomain,
                                     qint64 timeoutSec,
                                     const QByteArray& nonceBytes)
{
    if (!serverSigner) throw std::runtime_error("Sep45Challenge::newChallenge: null serverSigner");
    if (!network)      throw std::runtime_error("Sep45Challenge::newChallenge: null network");
    if (timeoutSec <= 0) {
        throw std::runtime_error("timeoutSec must be > 0 (SEP-45 requires non-infinite timebounds)");
    }
    // The same ceiling as the QML layer: a day. Past it a challenge stops
    // being a login and becomes a long-lived credential.
    if (timeoutSec > MAX_TIMEOUT_SECONDS) {
        throw std::runtime_error("timeoutSec must be at most 86400 (one day)");
    }
    // verifyChallenge never matches a blank domain, so a challenge carrying
    // one could only ever be refused.
    if (homeDomain.trimmed().isEmpty()) throw std::runtime_error("homeDomain is required");
    if (webAuthDomain.trimmed().isEmpty()) throw std::runtime_error("webAuthDomain is required");
    requireContract(clientAccountId, "clientAccountId");
    requireContract(webAuthContract, "webAuthContract");

    QByteArray nonce = nonceBytes.isEmpty()
        ? Util::generateRandomNonce(NONCE_SIZE)
        : nonceBytes;
    if (nonce.size() != NONCE_SIZE) {
        throw std::runtime_error("nonceBytes must be exactly 48 bytes");
    }

    const qint64 now = QDateTime::currentMSecsSinceEpoch() / 1000LL;
    // Overflowing now + timeoutSec made a challenge that never expires.
    if (timeoutSec > std::numeric_limits<qint64>::max() - now)
        throw std::runtime_error("timeoutSec is too large");
    const qint64 expiration = now + timeoutSec;

    QList<stellar::SCVal> args = buildArgs(
        clientAccountId, homeDomain, webAuthDomain, nonce, expiration);

    StellarAddress webAuth = StellarAddress::fromString(webAuthContract);
    StellarAddress client = StellarAddress::fromString(clientAccountId);
    // Owned until the builder takes it, so a refusal below does not leak it.
    std::unique_ptr<InvokeHostFunctionOperation> op(
        InvokeHostFunctionOperation::invokeContractFunctionOperationBuilder(
            webAuth, webAuthFunctionName, args));

    // Single unsigned auth entry whose credentials reference the client
    // contract and whose invocation matches the host call exactly.
    stellar::SorobanAuthorizationEntry entry;
    entry.credentials.type = stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS;
    entry.credentials.address.address.type = stellar::SCAddressType::SC_ADDRESS_TYPE_CONTRACT;
    std::memcpy(entry.credentials.address.address.contractId,
                client.getBytes().constData(), 32);
    entry.credentials.address.nonce = authNonceFromBytes(nonce);
    entry.credentials.address.signatureExpirationLedger = 0;
    // signature stays SCV_VOID until the client signs.

    entry.rootInvocation.function.type =
        stellar::SorobanAuthorizedFunctionType::SOROBAN_AUTHORIZED_FUNCTION_TYPE_CONTRACT_FN;
    entry.rootInvocation.function.contractFn.contractAddress = op->getHostFunction().invokeContract.contractAddress;
    entry.rootInvocation.function.contractFn.functionName = op->getHostFunction().invokeContract.functionName;
    entry.rootInvocation.function.contractFn.args = args;

    op->setAuth({entry});

    // Sequence -1 → 0 after build() (Java SEP-10 pattern).
    Account sa(new KeyPair(*serverSigner), -1);
    TransactionBuilder b(&sa, network);
    b.addOperation(op.release());
    b.setBaseFee(100);
    // Time-bounded validity window encoded in the tx as well.
    b.addTimeBounds(new TimeBounds(now, expiration));
    std::unique_ptr<Transaction> tx(b.build());
    tx->sign(serverSigner);
    return tx->toEnvelopeXdrBase64();
}

namespace {

/**
 * The checks behind both verifyChallenge overloads. `expectedContract` null
 * means the caller did not name the web-auth contract, so any contract passes;
 * the function name is always checked.
 */
Sep45Challenge::VerifiedChallenge verifyChallengeImpl(const QString& challengeXdr,
                                                      const QString& serverAccountId,
                                                      Network* network,
                                                      SorobanServer* sorobanServer,
                                                      const QStringList& homeDomains,
                                                      const QString& webAuthDomain,
                                                      const StellarAddress* expectedContract,
                                                      const QString& webAuthFunctionName)
{
    using qstellar::exception::InvalidSep45ChallengeException;

    // A blank expectation matched a challenge carrying the same blank field.
    if (webAuthDomain.trimmed().isEmpty()) {
        throw InvalidSep45ChallengeException("webAuthDomain is required");
    }
    if (webAuthFunctionName.isEmpty()) {
        throw InvalidSep45ChallengeException("webAuthFunctionName is required");
    }

    AbstractTransaction* atx = AbstractTransaction::fromEnvelopeXdr(challengeXdr, network);
    auto* tx = dynamic_cast<Transaction*>(atx);
    if (!tx) {
        delete atx;
        throw InvalidSep45ChallengeException("challenge is not a v1 Transaction");
    }
    QScopedPointer<Transaction> guard(tx);

    if (tx->getSourceAccount() != serverAccountId) {
        throw InvalidSep45ChallengeException("server source account mismatch");
    }
    if (tx->getOperations().size() != 1) {
        throw InvalidSep45ChallengeException("challenge must have exactly one operation");
    }
    auto* op = dynamic_cast<InvokeHostFunctionOperation*>(tx->getOperations().first());
    if (!op) {
        throw InvalidSep45ChallengeException("challenge op must be InvokeHostFunction");
    }
    const stellar::HostFunction& hf = op->getHostFunction();
    if (hf.type != stellar::HostFunctionType::HOST_FUNCTION_TYPE_INVOKE_CONTRACT) {
        throw InvalidSep45ChallengeException("host function must be INVOKE_CONTRACT");
    }
    // Which contract and function the challenge calls was never looked at, so
    // a "challenge" calling a token's transfer verified like a login.
    if (hf.invokeContract.contractAddress.type != stellar::SCAddressType::SC_ADDRESS_TYPE_CONTRACT) {
        throw InvalidSep45ChallengeException("host function must call a contract");
    }
    const StellarAddress hostContract(
        StellarAddress::Type::CONTRACT,
        QByteArray(reinterpret_cast<const char*>(hf.invokeContract.contractAddress.contractId), 32));
    if (expectedContract && hostContract != *expectedContract) {
        throw InvalidSep45ChallengeException("challenge does not call the expected web-auth contract");
    }
    if (hf.invokeContract.functionName != webAuthFunctionName.toUtf8()) {
        throw InvalidSep45ChallengeException(
            QStringLiteral("challenge does not call %1").arg(webAuthFunctionName).toStdString());
    }
    if (hf.invokeContract.args.size() != 5) {
        throw InvalidSep45ChallengeException("expected 5 args (client, homeDomain, webAuthDomain, nonce, expiration)");
    }

    // Extract args.
    StellarAddress client;
    QString homeDomain, webAuthDomainArg;
    QByteArray nonce;
    quint64 expirationUnix = 0;
    try {
        client            = Scv::fromAddress(hf.invokeContract.args.at(0));
        homeDomain        = Scv::fromString(hf.invokeContract.args.at(1));
        webAuthDomainArg  = Scv::fromString(hf.invokeContract.args.at(2));
        nonce             = Scv::fromBytes(hf.invokeContract.args.at(3));
        expirationUnix    = Scv::fromUint64(hf.invokeContract.args.at(4));
    } catch (const std::exception&) {
        throw InvalidSep45ChallengeException("challenge args have wrong SCVal types");
    }

    if (client.getType() != StellarAddress::Type::CONTRACT) {
        throw InvalidSep45ChallengeException("client account in args must be a contract address");
    }
    if (nonce.size() != NONCE_SIZE) {
        throw InvalidSep45ChallengeException("nonce must be 48 bytes");
    }
    if (webAuthDomainArg != webAuthDomain) {
        throw InvalidSep45ChallengeException("webAuthDomain mismatch");
    }
    // A blank entry in the allowed list matched a challenge with no home domain.
    if (homeDomain.trimmed().isEmpty() || !homeDomains.contains(homeDomain)) {
        throw InvalidSep45ChallengeException("homeDomain not in allowed set");
    }

    // The transaction's time bounds, checked as SEP-10 checks them, and the
    // expiration argument. The grace is for a client clock behind the
    // server's, so it widens the not-yet-valid side only: it used to be added
    // after expiry, as signed arithmetic that overflowed near INT64_MAX, and
    // the time bounds were not read at all.
    const qint64 now = QDateTime::currentMSecsSinceEpoch() / 1000LL;
    TimeBounds* bounds = tx->getTimeBounds();
    if (!bounds) {
        throw InvalidSep45ChallengeException("challenge requires timebounds");
    }
    if (bounds->getMaxTime() == 0) {
        throw InvalidSep45ChallengeException("challenge requires non-infinite timebounds");
    }
    if (now + Sep45Challenge::GRACE_PERIOD_SECONDS < bounds->getMinTime() || now > bounds->getMaxTime()) {
        throw InvalidSep45ChallengeException("challenge is not within its timebounds");
    }
    if (expirationUnix < static_cast<quint64>(now)) {
        throw InvalidSep45ChallengeException("challenge expired");
    }
    // Reported as a qint64: past INT64_MAX it would come out negative.
    if (expirationUnix > static_cast<quint64>(std::numeric_limits<qint64>::max())) {
        throw InvalidSep45ChallengeException("challenge expiration is out of range");
    }

    // Auth entry must reference the same client contract and use the
    // matching invocation. (The signature itself is verified by the
    // simulate step below — we can't re-run __check_auth locally.)
    if (op->getAuth().size() != 1) {
        throw InvalidSep45ChallengeException("auth list must have exactly one entry");
    }
    const stellar::SorobanAuthorizationEntry& entry = op->getAuth().first();
    if (entry.credentials.type != stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS) {
        throw InvalidSep45ChallengeException("auth entry must use ADDRESS credentials");
    }
    if (entry.credentials.address.address.type != stellar::SCAddressType::SC_ADDRESS_TYPE_CONTRACT) {
        throw InvalidSep45ChallengeException("auth entry address must be a contract");
    }
    if (std::memcmp(entry.credentials.address.address.contractId,
                    client.getBytes().constData(), 32) != 0) {
        throw InvalidSep45ChallengeException("auth entry contract does not match args client");
    }
    // The entry is what the client signs, and it says what that signature
    // authorizes: exactly this challenge's own call, nothing nested under it.
    // Unchecked, a wallet using this as its gate signed a token transfer, or a
    // transfer under web_auth_verify, as a "login" (py refuses both).
    const stellar::SorobanAuthorizedInvocation& root = entry.rootInvocation;
    if (root.subInvocations && !root.subInvocations->isEmpty()) {
        throw InvalidSep45ChallengeException("auth entry must not have sub-invocations");
    }
    if (root.function.type != stellar::SorobanAuthorizedFunctionType::SOROBAN_AUTHORIZED_FUNCTION_TYPE_CONTRACT_FN
            || xdrBytes(root.function.contractFn) != xdrBytes(hf.invokeContract)) {
        throw InvalidSep45ChallengeException("auth entry must authorize exactly the challenge's own call");
    }

    // The challenge envelope MUST be signed by the server's signing key.
    // simulateTransaction validates only the client-side auth chain (the
    // contract's __check_auth), NOT who issued the envelope — so without
    // this check a verifier would accept a challenge it never signed
    // (or an unsigned one), defeating SEP-45's mutual-auth guarantee.
    // Verified locally and cheaply, exactly as SEP-10 does.
    if (!serverSignedChallenge(challengeXdr, network, serverAccountId)) {
        throw InvalidSep45ChallengeException(
            "challenge transaction not signed by the server's signing key");
    }

    // Live verification: only meaningful with a real `sorobanServer`. The
    // simulation either succeeds (contract's __check_auth accepts the
    // client's signature) or returns an error.
    if (sorobanServer) {
        SimulateTransactionResponse sim = sorobanServer->simulateTransactionXdr(challengeXdr);
        if (sim.isError()) {
            throw InvalidSep45ChallengeException(
                ("simulation failed: " + sim.getError()).toStdString());
        }
    }

    Sep45Challenge::VerifiedChallenge out;
    out.clientAccountId = client.toString();
    out.matchedHomeDomain = homeDomain;
    out.nonce = nonce;
    out.expirationUnix = static_cast<qint64>(expirationUnix);
    out.webAuthContract = hostContract.toString();
    out.webAuthFunctionName = QString::fromUtf8(hf.invokeContract.functionName);
    return out;
}

} // namespace

Sep45Challenge::VerifiedChallenge Sep45Challenge::verifyChallenge(
    const QString& challengeXdr,
    const QString& serverAccountId,
    Network* network,
    SorobanServer* sorobanServer,
    const QStringList& homeDomains,
    const QString& webAuthDomain)
{
    return verifyChallengeImpl(challengeXdr, serverAccountId, network, sorobanServer,
                               homeDomains, webAuthDomain, nullptr,
                               DEFAULT_WEB_AUTH_FUNCTION_NAME);
}

Sep45Challenge::VerifiedChallenge Sep45Challenge::verifyChallenge(
    const QString& challengeXdr,
    const QString& serverAccountId,
    Network* network,
    SorobanServer* sorobanServer,
    const QStringList& homeDomains,
    const QString& webAuthDomain,
    const QString& webAuthContract,
    const QString& webAuthFunctionName)
{
    using qstellar::exception::InvalidSep45ChallengeException;
    // Named, it is checked: a blank or malformed one is refused rather than
    // read as "any contract".
    StellarAddress expected;
    try {
        expected = StellarAddress::fromString(webAuthContract);
    } catch (const std::exception&) {
        throw InvalidSep45ChallengeException("webAuthContract is not a contract address (C...)");
    }
    if (expected.getType() != StellarAddress::Type::CONTRACT) {
        throw InvalidSep45ChallengeException("webAuthContract is not a contract address (C...)");
    }
    return verifyChallengeImpl(challengeXdr, serverAccountId, network, sorobanServer,
                               homeDomains, webAuthDomain, &expected, webAuthFunctionName);
}

// ─── SEP-45 v0.1.1 ───────────────────────────────────────────────────
//
// py-stellar-sdk's sep/stellar_soroban_web_authentication.py is the reference:
// the same transactions, the same entries and the same signatures, byte for
// byte, with the checks it leaves out added where leaving them out would let
// somebody sign what was not asked.

namespace {

using qstellar::exception::InvalidSep45ChallengeException;

const char* const WEB_AUTH_VERIFY = "web_auth_verify";

// The keys of web_auth_verify's one argument (SEP-45 v0.1.1).
const char* const KEY_ACCOUNT = "account";
const char* const KEY_HOME_DOMAIN = "home_domain";
const char* const KEY_NONCE = "nonce";
const char* const KEY_WEB_AUTH_DOMAIN = "web_auth_domain";
const char* const KEY_WEB_AUTH_DOMAIN_ACCOUNT = "web_auth_domain_account";
const char* const KEY_CLIENT_DOMAIN = "client_domain";
const char* const KEY_CLIENT_DOMAIN_ACCOUNT = "client_domain_account";

/** A G address exactly as it encodes. */
bool isAccountId(const QString& value)
{
    try {
        return value.size() == StrKey::ACCOUNT_ID_ADDRESS_LENGTH
               && StrKey::decodeVersionByte(value) == StrKey::VersionByte::ACCOUNT_ID
               && StrKey::encodeStellarAccountId(StrKey::decodeStellarAccountId(value)) == value;
    } catch (const std::exception&) {
        return false;
    }
}

/** A C address exactly as it encodes. */
bool isContractId(const QString& value)
{
    try {
        return StrKey::decodeVersionByte(value) == StrKey::VersionByte::CONTRACT
               && StrKey::encodeContract(StrKey::decodeContract(value)) == value;
    } catch (const std::exception&) {
        return false;
    }
}

/** The G or C address of an entry's credentials; empty for any other kind,
 *  none of which is a party to a challenge. */
QString partyAddress(const stellar::SCAddress& address)
{
    switch (address.type) {
    case stellar::SCAddressType::SC_ADDRESS_TYPE_ACCOUNT:
        return StrKey::encodeStellarAccountId(
            QByteArray(reinterpret_cast<const char*>(address.accountId), 32));
    case stellar::SCAddressType::SC_ADDRESS_TYPE_CONTRACT:
        return StrKey::encodeContract(
            QByteArray(reinterpret_cast<const char*>(address.contractId), 32));
    default:
        return QString();
    }
}

/** The values of web_auth_verify's argument; the client domain pair is empty
 *  when the challenge has none. */
struct WebAuthArgs
{
    QString account;
    QString homeDomain;
    QString nonce;
    QString webAuthDomain;
    QString webAuthDomainAccount;
    QString clientDomain;
    QString clientDomainAccount;
};

/**
 * What a server may issue a challenge for. A value no reader accepts is
 * refused here rather than signed into a challenge that can only fail.
 */
WebAuthArgs issuedArgs(const QString& serverAccountId, const QString& clientAccountId,
                       const QString& homeDomain, const QString& webAuthDomain,
                       const QString& webAuthContract, const QString& nonce,
                       const QString& clientDomain, const QString& clientDomainAccount)
{
    if (!isContractId(clientAccountId))
        throw std::runtime_error("clientAccountId must be a contract account (C...): SEP-45 "
                                 "authenticates contract accounts, SEP-10 the others");
    if (!isContractId(webAuthContract))
        throw std::runtime_error("webAuthContract must be a contract address (C...): the "
                                 "WEB_AUTH_CONTRACT_ID of the server's stellar.toml");
    if (!isAccountId(serverAccountId))
        throw std::runtime_error("the server's signing key must be an account id (G...)");
    if (homeDomain.trimmed().isEmpty())
        throw std::runtime_error("homeDomain is required");
    if (webAuthDomain.trimmed().isEmpty())
        throw std::runtime_error("webAuthDomain is required");
    if (nonce.isEmpty())
        throw std::runtime_error("nonce is required");
    if (clientDomain.isEmpty() != clientDomainAccount.isEmpty())
        throw std::runtime_error("clientDomain and clientDomainAccount are given together or not at all");
    if (!clientDomain.isEmpty() && clientDomain.trimmed().isEmpty())
        throw std::runtime_error("clientDomain is blank");
    if (!clientDomainAccount.isEmpty() && !isAccountId(clientDomainAccount))
        throw std::runtime_error("clientDomainAccount must be an account id (G...): the "
                                 "SIGNING_KEY of the client domain's stellar.toml");
    // Its entry would be the server's own, and no reader could tell them apart.
    if (clientDomainAccount == serverAccountId)
        throw std::runtime_error("clientDomainAccount is the server's own signing key");
    WebAuthArgs args;
    args.account = clientAccountId;
    args.homeDomain = homeDomain;
    args.nonce = nonce;
    args.webAuthDomain = webAuthDomain;
    args.webAuthDomainAccount = serverAccountId;
    args.clientDomain = clientDomain;
    args.clientDomainAccount = clientDomainAccount;
    return args;
}

/** web_auth_verify's one argument: a map of symbols to strings, in the order
 *  the host requires (Scv::toMap sorts it, as py's to_struct does). */
stellar::SCVal argsMap(const WebAuthArgs& args)
{
    QList<stellar::SCMapEntry> entries;
    auto add = [&entries](const char* key, const QString& value) {
        stellar::SCMapEntry entry;
        entry.key = Scv::toSymbol(QString::fromLatin1(key));
        entry.val = Scv::toString(value);
        entries.append(entry);
    };
    add(KEY_ACCOUNT, args.account);
    add(KEY_HOME_DOMAIN, args.homeDomain);
    add(KEY_NONCE, args.nonce);
    add(KEY_WEB_AUTH_DOMAIN, args.webAuthDomain);
    add(KEY_WEB_AUTH_DOMAIN_ACCOUNT, args.webAuthDomainAccount);
    if (!args.clientDomain.isEmpty() && !args.clientDomainAccount.isEmpty()) {
        add(KEY_CLIENT_DOMAIN, args.clientDomain);
        add(KEY_CLIENT_DOMAIN_ACCOUNT, args.clientDomainAccount);
    }
    return Scv::toMap(entries);
}

/** The web_auth_verify call every entry of a challenge authorizes. */
stellar::InvokeContractArgs webAuthCall(const QString& webAuthContract, const WebAuthArgs& args)
{
    stellar::InvokeContractArgs call;
    call.contractAddress = Scv::toAddress(webAuthContract).address;
    call.functionName = QByteArray(WEB_AUTH_VERIFY);
    call.args.append(argsMap(args));
    return call;
}

stellar::SorobanAuthorizedInvocation invocationOf(const stellar::InvokeContractArgs& call)
{
    stellar::SorobanAuthorizedInvocation root;
    root.function.type = stellar::SorobanAuthorizedFunctionType::SOROBAN_AUTHORIZED_FUNCTION_TYPE_CONTRACT_FN;
    root.function.contractFn = call;
    return root;   // no sub-invocations
}

/**
 * The transaction py builds around a web_auth_verify call to simulate it:
 * from `sourceAccount` at sequence 1 (Account(source, 0), bumped by the
 * build), time bounds {0, 0}, the default base fee of 100, no memo, unsigned.
 */
QString invocationTransaction(const QString& sourceAccount, Network* network,
                              const stellar::InvokeContractArgs& call,
                              const QList<stellar::SorobanAuthorizationEntry>& auth)
{
    if (!network)
        throw std::runtime_error("network is required");
    if (!isAccountId(sourceAccount))
        throw std::runtime_error("the account the call is simulated from must be an account id (G...)");
    const StellarAddress contract(
        StellarAddress::Type::CONTRACT,
        QByteArray(reinterpret_cast<const char*>(call.contractAddress.contractId), 32));
    std::unique_ptr<InvokeHostFunctionOperation> op(
        InvokeHostFunctionOperation::invokeContractFunctionOperationBuilder(
            contract, QString::fromLatin1(call.functionName), call.args));
    op->setAuth(auth);

    Account source(KeyPair::fromAccountId(sourceAccount), 0);
    TransactionBuilder builder(&source, network);
    builder.addOperation(op.release());
    builder.setBaseFee(100);
    builder.setTimeout(TransactionBuilder::TIMEOUT_INFINITE);   // {0, 0}, as py's add_time_bounds(0, 0)
    std::unique_ptr<Transaction> tx(builder.build());
    return tx->toEnvelopeXdrBase64();
}

QByteArray encodeEntryList(const QList<stellar::SorobanAuthorizationEntry>& entries)
{
    QByteArray bytes;
    QDataStream out(&bytes, QIODevice::WriteOnly);
    out << static_cast<qint32>(entries.size());
    for (const stellar::SorobanAuthorizationEntry& entry : entries)
        out << entry;
    if (out.status() != QDataStream::Ok)
        throw std::runtime_error("the authorization entries cannot be encoded as XDR");
    return bytes;
}

QString entriesToBase64(const QList<stellar::SorobanAuthorizationEntry>& entries)
{
    return QString::fromLatin1(encodeEntryList(entries).toBase64(XDR_BASE64ENCODING));
}

/** Base64 read strictly: whitespace (a value wrapped over lines) is dropped,
 *  anything else outside the alphabet refused rather than skipped. */
QByteArray strictBase64(const QString& text, const char* what)
{
    QString compact;
    for (const QChar c : text) {
        if (!c.isSpace())
            compact.append(c);
    }
    const auto decoded = QByteArray::fromBase64Encoding(
        compact.toLatin1(), XDR_BASE64ENCODING | QByteArray::AbortOnBase64DecodingErrors);
    if (!decoded || (*decoded).isEmpty())
        throw InvalidSep45ChallengeException(std::string(what) + " is not base64");
    return *decoded;
}

/**
 * Base64 XDR SorobanAuthorizationEntries, in full or not at all: truncated
 * input, trailing bytes and anything this SDK would write back differently
 * are refused, since what is read here is what gets signed.
 */
QList<stellar::SorobanAuthorizationEntry> decodeEntryList(const QString& base64)
{
    QByteArray data = strictBase64(base64, "the challenge");
    QList<stellar::SorobanAuthorizationEntry> entries;
    QDataStream in(&data, QIODevice::ReadOnly);
    QByteArray again;
    try {
        qint32 count = 0;
        in >> count;
        stellar::checkWireCount(in, count, 12, "authorization entries");
        for (qint32 i = 0; i < count; ++i) {
            stellar::checkWireElement(in, "authorization entries");
            stellar::SorobanAuthorizationEntry entry;
            in >> entry;
            entries.append(entry);
        }
        if (in.status() == QDataStream::Ok && in.atEnd())
            again = encodeEntryList(entries);
    } catch (const std::exception&) {
        throw InvalidSep45ChallengeException("the challenge is not XDR SorobanAuthorizationEntries");
    }
    if (in.status() != QDataStream::Ok || !in.atEnd())
        throw InvalidSep45ChallengeException("the challenge does not decode: it is truncated, "
                                             "malformed, or followed by other bytes");
    // Written back, it must be the challenge given, byte for byte: what this
    // SDK would hold differently would be signed as something else.
    if (again != data)
        throw InvalidSep45ChallengeException("the challenge cannot be represented exactly by this "
                                             "SDK, so it is not read");
    return entries;
}

/** One base64 SorobanAuthorizationEntry, as a simulation returns it, whole. */
stellar::SorobanAuthorizationEntry decodeEntry(const QString& base64)
{
    QByteArray data = strictBase64(base64, "a simulated authorization entry");
    stellar::SorobanAuthorizationEntry entry;
    QDataStream in(&data, QIODevice::ReadOnly);
    try {
        in >> entry;
    } catch (const std::exception&) {
        throw std::runtime_error("a simulated authorization entry is not XDR");
    }
    if (in.status() != QDataStream::Ok || !in.atEnd()
            || encodeEntryList({entry}).mid(4) != data)
        throw std::runtime_error("a simulated authorization entry does not decode whole");
    return entry;
}

qint64 randomCredentialNonce()
{
    const QByteArray bytes = Util::generateRandomNonce(8);
    quint64 value = 0;
    for (const char b : bytes)
        value = (value << 8) | static_cast<quint8>(b);
    return static_cast<qint64>(value);
}

stellar::SorobanAuthorizationEntry unsignedEntry(const QString& address, qint64 nonce,
                                                 const stellar::SorobanAuthorizedInvocation& root)
{
    stellar::SorobanAuthorizationEntry entry;
    entry.credentials.type = stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS;
    entry.credentials.address.address = Scv::toAddress(address).address;
    entry.credentials.address.nonce = nonce;
    entry.credentials.address.signatureExpirationLedger = 0;   // signature stays SCV_VOID
    entry.rootInvocation = root;
    return entry;
}

/** latestLedger + expireInLedgers, refusing what cannot be a live signature. */
quint32 serverExpirationLedger(quint32 latestLedger, quint32 expireInLedgers)
{
    if (latestLedger == 0)
        throw std::runtime_error("latestLedger is required: the server's signature is valid "
                                 "for expireInLedgers past it");
    // Valid only up to the ledger it was read at, it has expired by the time
    // the client sees it.
    if (expireInLedgers == 0)
        throw std::runtime_error("expireInLedgers must be at least 1");
    if (latestLedger > std::numeric_limits<quint32>::max() - expireInLedgers)
        throw std::runtime_error("latestLedger + expireInLedgers is past the last ledger number");
    return latestLedger + expireInLedgers;
}

/**
 * The server's signature on a challenge, after checking the entries are
 * exactly the challenge asked for: one each for the client, the server and
 * the client domain account when there is one, unsigned, with address
 * credentials, authorizing `root` and nothing else. py signs whatever entry
 * the RPC puts the server's address on, so an RPC that answered with an entry
 * authorizing a transfer from the server's account got it signed.
 */
QString issueEntries(QList<stellar::SorobanAuthorizationEntry> entries, const WebAuthArgs& args,
                     const stellar::SorobanAuthorizedInvocation& root, KeyPair* serverSigner,
                     quint32 expirationLedger, Network* network)
{
    const QByteArray rootBytes = xdrBytes(root);
    bool client = false, server = false, clientDomain = false;
    int serverIndex = -1;
    for (int i = 0; i < entries.size(); ++i) {
        const stellar::SorobanAuthorizationEntry& entry = entries.at(i);
        if (entry.credentials.type != stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS)
            throw std::runtime_error("the simulation returned an entry without address credentials");
        if (xdrBytes(entry.rootInvocation) != rootBytes)
            throw std::runtime_error("the simulation returned an entry that authorizes something "
                                     "other than the challenge's web_auth_verify call");
        if (entry.credentials.address.signatureExpirationLedger != 0
                || entry.credentials.address.signature.type != stellar::SCValType::SCV_VOID)
            throw std::runtime_error("the simulation returned an entry that is already signed");
        const QString address = partyAddress(entry.credentials.address.address);
        bool* seen = nullptr;
        if (address == args.webAuthDomainAccount)
            seen = &server;
        else if (address == args.account)
            seen = &client;
        else if (!args.clientDomainAccount.isEmpty() && address == args.clientDomainAccount)
            seen = &clientDomain;
        if (!seen)
            throw std::runtime_error("the simulation returned an entry for an address that is not "
                                     "a party to the challenge");
        if (*seen)
            throw std::runtime_error("the simulation returned two entries for the same address");
        *seen = true;
        if (seen == &server)
            serverIndex = i;
    }
    if (!server)
        throw std::runtime_error("the simulation returned no entry for the server");
    if (!client)
        throw std::runtime_error("the simulation returned no entry for the client");
    if (!args.clientDomainAccount.isEmpty() && !clientDomain)
        throw std::runtime_error("the simulation returned no entry for the client domain account");

    entries[serverIndex] = SorobanCredentialsSigner::authorizeEntry(
        entries.at(serverIndex), serverSigner, expirationLedger, network->getNetworkPassphrase());
    return entriesToBase64(entries);
}

/** web_auth_verify's argument as a challenge carries it, read strictly. */
WebAuthArgs readArgs(const stellar::SCVal& argument)
{
    if (argument.type != stellar::SCValType::SCV_MAP || !argument.map)
        throw InvalidSep45ChallengeException("the web_auth_verify argument must be a map of strings");
    WebAuthArgs args;
    bool hasClientDomain = false, hasClientDomainAccount = false;
    const QList<stellar::SCMapEntry>& entries = *argument.map;
    for (int i = 0; i < entries.size(); ++i) {
        const stellar::SCMapEntry& entry = entries.at(i);
        if (entry.key.type != stellar::SCValType::SCV_SYMBOL)
            throw InvalidSep45ChallengeException("the keys of the web_auth_verify argument must be symbols");
        // The host refuses a map that is not strictly ascending, so a
        // challenge carrying one could never verify; a repeated key would be
        // read one way here and another there.
        if (i > 0 && Scv::compare(entries.at(i - 1).key, entry.key) >= 0)
            throw InvalidSep45ChallengeException("the keys of the web_auth_verify argument must be "
                                                 "sorted and without repeats");
        const QByteArray key = entry.key.symbol;
        if (entry.val.type != stellar::SCValType::SCV_STRING)
            throw InvalidSep45ChallengeException("every value of the web_auth_verify argument must be a string");
        const QString value = QString::fromUtf8(entry.val.str);
        if (value.toUtf8() != entry.val.str)
            throw InvalidSep45ChallengeException("a value of the web_auth_verify argument is not UTF-8");
        if (value.isEmpty())
            throw InvalidSep45ChallengeException("a value of the web_auth_verify argument is empty");
        if (key == KEY_ACCOUNT) {
            args.account = value;
        } else if (key == KEY_HOME_DOMAIN) {
            args.homeDomain = value;
        } else if (key == KEY_NONCE) {
            args.nonce = value;
        } else if (key == KEY_WEB_AUTH_DOMAIN) {
            args.webAuthDomain = value;
        } else if (key == KEY_WEB_AUTH_DOMAIN_ACCOUNT) {
            args.webAuthDomainAccount = value;
        } else if (key == KEY_CLIENT_DOMAIN) {
            args.clientDomain = value;
            hasClientDomain = true;
        } else if (key == KEY_CLIENT_DOMAIN_ACCOUNT) {
            args.clientDomainAccount = value;
            hasClientDomainAccount = true;
        } else {
            // py's verification rebuilds the call from the keys it knows, so a
            // challenge carrying another could never verify.
            throw InvalidSep45ChallengeException("the web_auth_verify argument has a key SEP-45 does not define");
        }
    }
    if (args.account.isEmpty())
        throw InvalidSep45ChallengeException("Missing 'account' in arguments.");
    if (args.homeDomain.isEmpty())
        throw InvalidSep45ChallengeException("Missing 'home_domain' in arguments.");
    if (args.nonce.isEmpty())
        throw InvalidSep45ChallengeException("Missing 'nonce' in arguments.");
    if (args.webAuthDomain.isEmpty())
        throw InvalidSep45ChallengeException("Missing 'web_auth_domain' in arguments.");
    if (args.webAuthDomainAccount.isEmpty())
        throw InvalidSep45ChallengeException("Missing 'web_auth_domain_account' in arguments.");
    if (hasClientDomain != hasClientDomainAccount)
        throw InvalidSep45ChallengeException("'client_domain' and 'client_domain_account' must both "
                                             "be provided or both be absent.");
    return args;
}

/**
 * The server's signature on its own entry: the default account shape, one
 * {public_key, signature} map in a vec, by `serverAccountId`, over the entry
 * as it stands on `network`. It covers the call and so every argument, the
 * nonce included: a verifying signature is the server saying it issued this
 * challenge, unaltered.
 */
void requireServerSignature(const stellar::SorobanAuthorizationEntry& entry,
                            const QString& serverAccountId, Network* network)
{
    QScopedPointer<KeyPair> server(KeyPair::fromAccountId(serverAccountId));
    const stellar::SorobanAddressCredentials& credentials = entry.credentials.address;
    const stellar::SCVal& shape = credentials.signature;
    QByteArray signature;
    if (shape.type == stellar::SCValType::SCV_VEC && shape.vec && shape.vec->size() == 1) {
        const stellar::SCVal& map = shape.vec->first();
        if (map.type == stellar::SCValType::SCV_MAP && map.map && map.map->size() == 2) {
            const stellar::SCMapEntry& key = map.map->at(0);
            const stellar::SCMapEntry& sig = map.map->at(1);
            if (key.key.type == stellar::SCValType::SCV_SYMBOL && key.key.symbol == "public_key"
                    && key.val.type == stellar::SCValType::SCV_BYTES
                    && key.val.bytes == server->getPublicKey()
                    && sig.key.type == stellar::SCValType::SCV_SYMBOL && sig.key.symbol == "signature"
                    && sig.val.type == stellar::SCValType::SCV_BYTES && sig.val.bytes.size() == 64) {
                signature = sig.val.bytes;
            }
        }
    }
    if (signature.isEmpty())
        throw InvalidSep45ChallengeException("the server's entry is not signed by the server's "
                                             "signing key");
    QByteArray payload;
    try {
        payload = SorobanCredentialsSigner::hashedSignaturePayload(
            entry, credentials.signatureExpirationLedger, network->getNetworkPassphrase());
    } catch (const std::exception&) {
        throw InvalidSep45ChallengeException("the server's entry cannot be hashed for its signature");
    }
    if (!server->verify(payload, signature))
        throw InvalidSep45ChallengeException("the server's signature on its entry does not verify: "
                                             "the challenge was not issued by this server, was "
                                             "altered, or is for another network");
}

Sep45Challenge::ChallengeAuthorizationEntries readChallenge(const QString& challengeXdr,
                                                            const QString& serverAccountId,
                                                            const QStringList& homeDomains,
                                                            const QString& webAuthDomain,
                                                            const QString& webAuthContract,
                                                            Network* network)
{
    // The expectations first: a blank or malformed one would match a
    // challenge carrying the same blank.
    if (!network)
        throw InvalidSep45ChallengeException("network is required: the server's signature is made on one");
    if (!isAccountId(serverAccountId))
        throw InvalidSep45ChallengeException("serverAccountId must be an account id (G...): the "
                                             "SIGNING_KEY of the server's stellar.toml");
    if (!isContractId(webAuthContract))
        throw InvalidSep45ChallengeException("webAuthContract must be a contract address (C...): "
                                             "the WEB_AUTH_CONTRACT_ID of the server's stellar.toml");
    if (webAuthDomain.trimmed().isEmpty())
        throw InvalidSep45ChallengeException("webAuthDomain is required");

    const QList<stellar::SorobanAuthorizationEntry> entries = decodeEntryList(challengeXdr);
    if (entries.size() < 2)
        throw InvalidSep45ChallengeException("Challenge must contain at least two authorization entries.");

    // Every entry authorizes the same call, and that call is web_auth_verify
    // on the expected contract with nothing under it: it is what the client's
    // signature will authorize.
    const stellar::SorobanAuthorizedInvocation& root = entries.first().rootInvocation;
    const QByteArray rootBytes = xdrBytes(root);
    for (int i = 1; i < entries.size(); ++i) {
        if (xdrBytes(entries.at(i).rootInvocation) != rootBytes)
            throw InvalidSep45ChallengeException("Inconsistent root_invocation across authorization entries.");
    }
    if (root.subInvocations && !root.subInvocations->isEmpty())
        throw InvalidSep45ChallengeException("Authorization entry must not have sub-invocations.");
    if (root.function.type != stellar::SorobanAuthorizedFunctionType::SOROBAN_AUTHORIZED_FUNCTION_TYPE_CONTRACT_FN)
        throw InvalidSep45ChallengeException("Authorization entry must invoke a contract function.");
    const stellar::InvokeContractArgs& call = root.function.contractFn;
    if (call.contractAddress.type != stellar::SCAddressType::SC_ADDRESS_TYPE_CONTRACT
            || partyAddress(call.contractAddress) != webAuthContract)
        throw InvalidSep45ChallengeException("the challenge does not call the expected web-auth contract");
    if (call.functionName != QByteArray(WEB_AUTH_VERIFY))
        throw InvalidSep45ChallengeException("the challenge does not call web_auth_verify");
    if (call.args.size() != 1)
        throw InvalidSep45ChallengeException("Expected exactly one argument in contract function call.");

    const WebAuthArgs args = readArgs(call.args.first());
    if (!isContractId(args.account))
        throw InvalidSep45ChallengeException("the account being authenticated must be a contract (C...)");
    if (args.webAuthDomainAccount != serverAccountId)
        throw InvalidSep45ChallengeException("web_auth_domain_account is not the server's signing key");
    if (args.webAuthDomain != webAuthDomain)
        throw InvalidSep45ChallengeException("web_auth_domain is not the expected web-auth domain");
    if (!homeDomains.contains(args.homeDomain))
        throw InvalidSep45ChallengeException("home_domain is not one of the expected home domains");
    if (!args.clientDomainAccount.isEmpty()) {
        if (!isAccountId(args.clientDomainAccount))
            throw InvalidSep45ChallengeException("client_domain_account must be an account id (G...)");
        if (args.clientDomainAccount == serverAccountId)
            throw InvalidSep45ChallengeException("client_domain_account is the server's own signing key");
    }

    // One entry each for the server, the client and the client domain account
    // when there is one; nobody else, nobody twice.
    int serverIndex = -1;
    bool client = false, clientDomain = false;
    for (int i = 0; i < entries.size(); ++i) {
        const stellar::SorobanAuthorizationEntry& entry = entries.at(i);
        if (entry.credentials.type != stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS)
            throw InvalidSep45ChallengeException("every entry must carry address credentials");
        const QString address = partyAddress(entry.credentials.address.address);
        if (address == serverAccountId) {
            if (serverIndex >= 0)
                throw InvalidSep45ChallengeException("the challenge carries two entries for the server");
            serverIndex = i;
        } else if (address == args.account) {
            if (client)
                throw InvalidSep45ChallengeException("the challenge carries two entries for the client");
            client = true;
        } else if (!args.clientDomainAccount.isEmpty() && address == args.clientDomainAccount) {
            if (clientDomain)
                throw InvalidSep45ChallengeException("the challenge carries two entries for the client "
                                                     "domain account");
            clientDomain = true;
        } else {
            throw InvalidSep45ChallengeException("the challenge carries an entry for an address that "
                                                 "is not a party to it");
        }
    }
    if (serverIndex < 0)
        throw InvalidSep45ChallengeException("Challenge does not contain an authorization entry for the server.");
    if (!client)
        throw InvalidSep45ChallengeException("Challenge does not contain an authorization entry for the client.");
    if (!args.clientDomainAccount.isEmpty() && !clientDomain)
        throw InvalidSep45ChallengeException("Challenge does not contain an authorization entry for "
                                             "the client domain account.");

    requireServerSignature(entries.at(serverIndex), serverAccountId, network);

    Sep45Challenge::ChallengeAuthorizationEntries out;
    out.authorizationEntries = entries;
    out.clientAccountId = args.account;
    out.matchedHomeDomain = args.homeDomain;
    out.nonce = args.nonce;
    out.webAuthDomain = args.webAuthDomain;
    out.serverAccountId = serverAccountId;
    out.webAuthContract = webAuthContract;
    out.clientDomain = args.clientDomain;
    out.clientDomainAccount = args.clientDomainAccount;
    out.serverSignatureExpirationLedger =
        entries.at(serverIndex).credentials.address.signatureExpirationLedger;
    return out;
}

} // namespace

QString Sep45Challenge::challengeTransaction(const QString& serverAccountId,
                                             const QString& clientAccountId,
                                             const QString& homeDomain,
                                             const QString& webAuthDomain,
                                             const QString& webAuthContract,
                                             Network* network,
                                             const QString& nonce,
                                             const QString& clientDomain,
                                             const QString& clientDomainAccount,
                                             const QString& simulateTxAccount)
{
    const WebAuthArgs args = issuedArgs(serverAccountId, clientAccountId, homeDomain, webAuthDomain,
                                        webAuthContract, nonce, clientDomain, clientDomainAccount);
    return invocationTransaction(simulateTxAccount, network, webAuthCall(webAuthContract, args), {});
}

QString Sep45Challenge::signSimulatedChallengeEntries(const QStringList& simulatedEntries,
                                                      quint32 latestLedger,
                                                      KeyPair* serverSigner,
                                                      const QString& clientAccountId,
                                                      const QString& homeDomain,
                                                      const QString& webAuthDomain,
                                                      const QString& webAuthContract,
                                                      Network* network,
                                                      const QString& nonce,
                                                      quint32 expireInLedgers,
                                                      const QString& clientDomain,
                                                      const QString& clientDomainAccount)
{
    if (!serverSigner)
        throw std::runtime_error("signSimulatedChallengeEntries: null serverSigner");
    if (!network)
        throw std::runtime_error("signSimulatedChallengeEntries: null network");
    const WebAuthArgs args = issuedArgs(serverSigner->getAccountId(), clientAccountId, homeDomain,
                                        webAuthDomain, webAuthContract, nonce, clientDomain,
                                        clientDomainAccount);
    const quint32 expiration = serverExpirationLedger(latestLedger, expireInLedgers);
    if (simulatedEntries.isEmpty())
        throw std::runtime_error("No authorization entries returned from simulation.");
    QList<stellar::SorobanAuthorizationEntry> entries;
    for (const QString& base64 : simulatedEntries)
        entries.append(decodeEntry(base64));
    return issueEntries(entries, args, invocationOf(webAuthCall(webAuthContract, args)),
                        serverSigner, expiration, network);
}

QString Sep45Challenge::buildChallengeAuthorizationEntries(SorobanServer* sorobanServer,
                                                           KeyPair* serverSigner,
                                                           const QString& clientAccountId,
                                                           const QString& homeDomain,
                                                           const QString& webAuthDomain,
                                                           const QString& webAuthContract,
                                                           Network* network,
                                                           const QString& nonce,
                                                           quint32 expireInLedgers,
                                                           const QString& clientDomain,
                                                           const QString& clientDomainAccount,
                                                           const QString& simulateTxAccount)
{
    if (!sorobanServer)
        throw std::runtime_error("buildChallengeAuthorizationEntries: a SorobanServer is required to "
                                 "simulate the call (the overload taking latestLedger needs none)");
    if (!serverSigner)
        throw std::runtime_error("buildChallengeAuthorizationEntries: null serverSigner");
    // py's default: 48 random bytes, base64.
    const QString challengeNonce = nonce.isEmpty()
        ? QString::fromLatin1(Util::generateRandomNonce(48).toBase64())
        : nonce;
    const QString tx = challengeTransaction(serverSigner->getAccountId(), clientAccountId, homeDomain,
                                            webAuthDomain, webAuthContract, network, challengeNonce,
                                            clientDomain, clientDomainAccount, simulateTxAccount);
    const SimulateTransactionResponse sim = sorobanServer->simulateTransactionXdr(tx);
    if (sim.isError())
        throw std::runtime_error(("the simulation of the challenge failed: " + sim.getError()).toStdString());
    if (sim.getResults().size() != 1 || sim.getResults().first().auth.isEmpty())
        throw std::runtime_error("No authorization entries returned from simulation.");
    return signSimulatedChallengeEntries(sim.getResults().first().auth, sim.getLatestLedger(),
                                         serverSigner, clientAccountId, homeDomain, webAuthDomain,
                                         webAuthContract, network, challengeNonce, expireInLedgers,
                                         clientDomain, clientDomainAccount);
}

QString Sep45Challenge::buildChallengeAuthorizationEntries(KeyPair* serverSigner,
                                                           const QString& clientAccountId,
                                                           const QString& homeDomain,
                                                           const QString& webAuthDomain,
                                                           const QString& webAuthContract,
                                                           Network* network,
                                                           quint32 latestLedger,
                                                           const QString& nonce,
                                                           quint32 expireInLedgers,
                                                           const QString& clientDomain,
                                                           const QString& clientDomainAccount)
{
    if (!serverSigner)
        throw std::runtime_error("buildChallengeAuthorizationEntries: null serverSigner");
    if (!network)
        throw std::runtime_error("buildChallengeAuthorizationEntries: null network");
    const QString challengeNonce = nonce.isEmpty()
        ? QString::fromLatin1(Util::generateRandomNonce(48).toBase64())
        : nonce;
    const WebAuthArgs args = issuedArgs(serverSigner->getAccountId(), clientAccountId, homeDomain,
                                        webAuthDomain, webAuthContract, challengeNonce, clientDomain,
                                        clientDomainAccount);
    const quint32 expiration = serverExpirationLedger(latestLedger, expireInLedgers);
    // In the order the reference contract asks for them, as its simulation
    // returns them: the client, the server, the client domain.
    const stellar::SorobanAuthorizedInvocation root = invocationOf(webAuthCall(webAuthContract, args));
    QList<stellar::SorobanAuthorizationEntry> entries;
    entries.append(unsignedEntry(args.account, randomCredentialNonce(), root));
    entries.append(unsignedEntry(args.webAuthDomainAccount, randomCredentialNonce(), root));
    if (!args.clientDomainAccount.isEmpty())
        entries.append(unsignedEntry(args.clientDomainAccount, randomCredentialNonce(), root));
    return issueEntries(entries, args, root, serverSigner, expiration, network);
}

Sep45Challenge::ChallengeAuthorizationEntries Sep45Challenge::readChallengeAuthorizationEntries(
    const QString& challengeAuthorizationEntries,
    const QString& serverAccountId,
    const QStringList& homeDomains,
    const QString& webAuthDomain,
    const QString& webAuthContract,
    Network* network)
{
    return readChallenge(challengeAuthorizationEntries, serverAccountId, homeDomains, webAuthDomain,
                         webAuthContract, network);
}

QString Sep45Challenge::signChallengeAuthorizationEntries(const QString& challengeAuthorizationEntries,
                                                          const QString& serverAccountId,
                                                          const QStringList& homeDomains,
                                                          const QString& webAuthDomain,
                                                          const QString& webAuthContract,
                                                          const QString& clientAccountId,
                                                          KeyPair* clientSigner,
                                                          quint32 validUntilLedger,
                                                          Network* network,
                                                          const QString& clientDomain,
                                                          const QString& clientDomainAccount,
                                                          KeyPair* clientDomainSigner)
{
    // What is to be signed, and by whom, before anything is read.
    if (!clientSigner && !clientDomainSigner)
        throw InvalidSep45ChallengeException("nothing to sign with: give the client's signer, the "
                                             "client domain's, or both");
    if (validUntilLedger == 0)
        throw InvalidSep45ChallengeException("validUntilLedger is required: the last ledger the "
                                             "signature is good for");
    if (!isContractId(clientAccountId))
        throw InvalidSep45ChallengeException("clientAccountId must be a contract account (C...)");
    if (clientDomain.isEmpty() != clientDomainAccount.isEmpty())
        throw InvalidSep45ChallengeException("clientDomain and clientDomainAccount are given together "
                                             "or not at all");
    if (clientDomainSigner) {
        if (clientDomainAccount.isEmpty())
            throw InvalidSep45ChallengeException("clientDomainSigner needs clientDomain and "
                                                 "clientDomainAccount: the domain that key signs for");
        if (clientDomainSigner->getAccountId() != clientDomainAccount)
            throw InvalidSep45ChallengeException("clientDomainSigner is not the key of clientDomainAccount");
    }

    // Validate, then sign: never the other way round. The signature cannot be
    // taken back, and handed an unchecked challenge a wallet would authorize
    // whatever call the entries named.
    const ChallengeAuthorizationEntries read = readChallenge(
        challengeAuthorizationEntries, serverAccountId, homeDomains, webAuthDomain, webAuthContract,
        network);
    // A valid challenge for another account is still not this wallet's to sign.
    if (read.clientAccountId != clientAccountId)
        throw InvalidSep45ChallengeException(QStringLiteral("this challenge is for %1, not %2")
                                                 .arg(read.clientAccountId, clientAccountId)
                                                 .toStdString());
    if (!clientDomain.isEmpty()
            && (read.clientDomain != clientDomain || read.clientDomainAccount != clientDomainAccount))
        throw InvalidSep45ChallengeException("the challenge does not carry the client domain asked "
                                             "for, with that domain's signing key");

    const QString passphrase = network->getNetworkPassphrase();
    QList<stellar::SorobanAuthorizationEntry> entries = read.authorizationEntries;
    for (stellar::SorobanAuthorizationEntry& entry : entries) {
        const QString address = partyAddress(entry.credentials.address.address);
        if (clientSigner && address == read.clientAccountId)
            entry = SorobanCredentialsSigner::authorizeEntry(entry, clientSigner, validUntilLedger, passphrase);
        else if (clientDomainSigner && address == read.clientDomainAccount)
            entry = SorobanCredentialsSigner::authorizeEntry(entry, clientDomainSigner, validUntilLedger,
                                                             passphrase);
    }
    return entriesToBase64(entries);
}

QString Sep45Challenge::verificationTransaction(const ChallengeAuthorizationEntries& challenge,
                                                Network* network,
                                                const QString& simulateTxAccount)
{
    if (!isContractId(challenge.webAuthContract))
        throw InvalidSep45ChallengeException("webAuthContract must be a contract address (C...)");
    if (challenge.clientDomain.isEmpty() != challenge.clientDomainAccount.isEmpty())
        throw InvalidSep45ChallengeException("clientDomain and clientDomainAccount go together");
    WebAuthArgs args;
    args.account = challenge.clientAccountId;
    args.homeDomain = challenge.matchedHomeDomain;
    args.nonce = challenge.nonce;
    args.webAuthDomain = challenge.webAuthDomain;
    args.webAuthDomainAccount = challenge.serverAccountId;
    args.clientDomain = challenge.clientDomain;
    args.clientDomainAccount = challenge.clientDomainAccount;
    return invocationTransaction(simulateTxAccount, network,
                                 webAuthCall(challenge.webAuthContract, args),
                                 challenge.authorizationEntries);
}

Sep45Challenge::ChallengeAuthorizationEntries Sep45Challenge::verifyChallengeAuthorizationEntries(
    SorobanServer* sorobanServer,
    const QString& challengeAuthorizationEntries,
    const QString& serverAccountId,
    const QStringList& homeDomains,
    const QString& webAuthDomain,
    const QString& webAuthContract,
    Network* network,
    const QString& simulateTxAccount)
{
    // Without the simulation nothing says the client signed: __check_auth is
    // contract code, and only the host runs it.
    if (!sorobanServer)
        throw InvalidSep45ChallengeException("a SorobanServer is required: the client's signature is "
                                             "checked by simulating the call");
    const ChallengeAuthorizationEntries read = readChallenge(
        challengeAuthorizationEntries, serverAccountId, homeDomains, webAuthDomain, webAuthContract,
        network);
    const QString tx = verificationTransaction(read, network, simulateTxAccount);
    const SimulateTransactionResponse sim = sorobanServer->simulateTransactionXdr(tx);
    if (sim.isError())
        throw InvalidSep45ChallengeException(
            ("Validation failed during simulation: " + sim.getError()).toStdString());
    // An answer with neither an error nor a result did not run the call.
    if (sim.getResults().isEmpty())
        throw InvalidSep45ChallengeException("the simulation returned no result: the call was not run");
    return read;
}

Sep45Challenge::ChallengeAuthorizationEntries Sep45Challenge::simulateSignedChallenge(
    SorobanServer* sorobanServer,
    const QString& signedEntries,
    const QString& serverAccountId,
    const QStringList& homeDomains,
    const QString& webAuthDomain,
    const QString& webAuthContract,
    Network* network,
    const QString& simulateTxAccount)
{
    if (!sorobanServer)
        throw InvalidSep45ChallengeException("a SorobanServer is required to simulate the signed challenge");
    const ChallengeAuthorizationEntries read = readChallenge(
        signedEntries, serverAccountId, homeDomains, webAuthDomain, webAuthContract, network);
    const SimulateTransactionResponse sim =
        sorobanServer->simulateTransactionXdr(verificationTransaction(read, network, simulateTxAccount));
    if (sim.isError())
        throw InvalidSep45ChallengeException(
            ("the signed challenge does not verify in simulation: " + sim.getError()).toStdString());
    if (sim.getResults().isEmpty())
        throw InvalidSep45ChallengeException("the simulation returned no result: the call was not run");
    checkSimulationFootprint(read, sim.getTransactionData());
    return read;
}

void Sep45Challenge::checkSimulationFootprint(const ChallengeAuthorizationEntries& challenge,
                                              const QString& transactionData)
{
    // Nothing to read is no proof of no side effect.
    QByteArray data = strictBase64(transactionData, "the simulation's transactionData");
    stellar::SorobanTransactionData soroban;
    QDataStream in(&data, QIODevice::ReadOnly);
    try {
        in >> soroban;
    } catch (const std::exception&) {
        throw InvalidSep45ChallengeException("the simulation's transactionData is not XDR");
    }
    if (in.status() != QDataStream::Ok || !in.atEnd())
        throw InvalidSep45ChallengeException("the simulation's transactionData does not decode whole");

    // The nonce each party's entry uses up, and nothing else.
    QList<QPair<QString, qint64>> nonces;
    for (const stellar::SorobanAuthorizationEntry& entry : challenge.authorizationEntries) {
        if (entry.credentials.type == stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS)
            nonces.append({partyAddress(entry.credentials.address.address), entry.credentials.address.nonce});
    }
    // Since Protocol 23 the simulation restores what is archived by itself: an
    // archived contract comes in readWrite with its instance AND its wasm,
    // both listed (by index into readWrite) in archivedSorobanEntries. A wasm
    // (CONTRACT_CODE) is immutable, no contract writes it, so restoring one
    // is no side effect. Anything else that is not contract data is.
    const QVector<quint32> archived = soroban.ext.v == 1 ? soroban.ext.archivedSorobanEntries
                                                         : QVector<quint32>();
    const auto& readWrite = soroban.resources.footprint.readWrite;
    for (int i = 0; i < readWrite.size(); ++i) {
        const stellar::SorobanLedgerKey& key = readWrite.at(i);
        if (key.type == stellar::LedgerEntryType::CONTRACT_CODE && archived.contains(quint32(i)))
            continue;
        if (key.type != stellar::LedgerEntryType::CONTRACT_DATA)
            throw InvalidSep45ChallengeException("the simulation writes a ledger entry that is not "
                                                 "contract data: the challenge does more than log in");
        const stellar::LedgerKeyContractData& entry = key.contractData;
        const QString owner = partyAddress(entry.contract);
        if (entry.key.type == stellar::SCValType::SCV_LEDGER_KEY_NONCE
                && entry.durability == stellar::ContractDataDurability::TEMPORARY
                && (owner == challenge.clientAccountId || owner == challenge.serverAccountId
                    || (!challenge.clientDomainAccount.isEmpty() && owner == challenge.clientDomainAccount))
                && nonces.contains({owner, entry.key.nonceKey.nonce})) {
            continue;
        }
        // v0.1.1: the web-auth contract's instance, restored when archived.
        if (entry.key.type == stellar::SCValType::SCV_LEDGER_KEY_CONTRACT_INSTANCE
                && entry.durability == stellar::ContractDataDurability::PERSISTENT
                && owner == challenge.webAuthContract) {
            continue;
        }
        throw InvalidSep45ChallengeException("the simulation writes contract data other than the "
                                             "challenge's nonces: the challenge does more than log in");
    }
}
QSTELLAR_END_NS
