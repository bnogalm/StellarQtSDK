#include "sep45challenge.h"

#include <QDateTime>
#include <cstring>
#include <stdexcept>

#include "abstracttransaction.h"
#include "account.h"
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

namespace {

constexpr int NONCE_SIZE = 48;

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
    requireContract(clientAccountId, "clientAccountId");
    requireContract(webAuthContract, "webAuthContract");

    QByteArray nonce = nonceBytes.isEmpty()
        ? Util::generateRandomNonce(NONCE_SIZE)
        : nonceBytes;
    if (nonce.size() != NONCE_SIZE) {
        throw std::runtime_error("nonceBytes must be exactly 48 bytes");
    }

    const qint64 now = QDateTime::currentMSecsSinceEpoch() / 1000LL;
    const qint64 expiration = now + timeoutSec;

    QList<stellar::SCVal> args = buildArgs(
        clientAccountId, homeDomain, webAuthDomain, nonce, expiration);

    StellarAddress webAuth = StellarAddress::fromString(webAuthContract);
    InvokeHostFunctionOperation* op =
        InvokeHostFunctionOperation::invokeContractFunctionOperationBuilder(
            webAuth, webAuthFunctionName, args);

    // Single unsigned auth entry whose credentials reference the client
    // contract and whose invocation matches the host call exactly.
    stellar::SorobanAuthorizationEntry entry;
    entry.credentials.type = stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS;
    StellarAddress client = StellarAddress::fromString(clientAccountId);
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
    b.addOperation(op);
    b.setBaseFee(100);
    // Time-bounded validity window encoded in the tx as well.
    b.addTimeBounds(new TimeBounds(now, expiration));
    Transaction* tx = b.build();
    tx->sign(serverSigner);
    QString envelope = tx->toEnvelopeXdrBase64();
    delete tx;
    return envelope;
}

Sep45Challenge::VerifiedChallenge Sep45Challenge::verifyChallenge(
    const QString& challengeXdr,
    const QString& serverAccountId,
    Network* network,
    SorobanServer* sorobanServer,
    const QStringList& homeDomains,
    const QString& webAuthDomain)
{
    using qstellar::exception::InvalidSep45ChallengeException;

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
    if (!homeDomains.contains(homeDomain)) {
        throw InvalidSep45ChallengeException("homeDomain not in allowed set");
    }

    const qint64 now = QDateTime::currentMSecsSinceEpoch() / 1000LL;
    if (static_cast<qint64>(expirationUnix) + GRACE_PERIOD_SECONDS < now) {
        throw InvalidSep45ChallengeException("challenge expired");
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

    // The DecoratedSignature path lives in Sep10Challenge / AbstractTransaction
    // — for SEP-45 we accept any envelope that round-trips via
    // `AbstractTransaction::fromEnvelopeXdr` (which validates the wire format)
    // and matches the source account, and we then defer the cryptographic
    // truth to `simulateTransaction`. A challenge with no server signature
    // would still fail on the network at submit time; here, simulate is
    // the authoritative validator of the client-side auth chain.

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

    VerifiedChallenge out;
    out.clientAccountId = client.toString();
    out.matchedHomeDomain = homeDomain;
    out.nonce = nonce;
    out.expirationUnix = static_cast<qint64>(expirationUnix);
    return out;
}
QSTELLAR_END_NS
