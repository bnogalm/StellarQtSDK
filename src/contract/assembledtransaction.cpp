#include "assembledtransaction.h"

#include <QByteArray>
#include <QDataStream>
#include <stdexcept>

#include <QSet>
#include <cstring>
#include <memory>

#include "abstracttransaction.h"
#include "account.h"
#include "invokehostfunctionoperation.h"
#include "keypair.h"
#include "network.h"
#include "restorefootprintoperation.h"
#include "sorobanserver.h"
#include "sorobandatabuilder.h"
#include "stellaraddress.h"
#include "transaction.h"
#include "transactionbuilder.h"
#include "util.h"
#include "contract/sorobancredentialssigner.h"
#include "exception/assembledtransactionexception.h"
#include "exception/notyetsimulatedexception.h"
#include "exception/sendfailedexception.h"
#include "exception/simulationfailedexception.h"

QSTELLAR_BEGIN_NS

using qstellar::exception::AssembledTransactionException;
using qstellar::exception::ExpiredStateException;
using qstellar::exception::NeedsMoreSignaturesException;
using qstellar::exception::NoSignatureNeededException;
using qstellar::exception::NotYetSimulatedException;
using qstellar::exception::SendFailedException;
using qstellar::exception::SendOutcomeUnknownException;
using qstellar::exception::SimulationFailedException;
using qstellar::exception::SorobanTransportException;

// The soroban data, the fee and the auth entries are in the signed body:
// changed once the transaction is signed, the signature is over a body that
// is not sent. signAndSend then signed the new body as well and kept the
// stale signature, which core refuses (txBAD_AUTH_EXTRA). Refused, as py's
// prepare() and sign_auth_entries() refuse it.
static void refuseOnceSigned(const Transaction* tx, const char* method)
{
    if (!tx->getSignatures().isEmpty()) {
        throw AssembledTransactionException(
            QString("AssembledTransaction::%1: the transaction is already signed; simulate it and "
                    "sign its auth entries before signing it").arg(QString::fromLatin1(method)));
    }
}

AssembledTransaction::AssembledTransaction(Transaction* tx, SorobanServer* server, Network* network)
    : m_tx(tx), m_server(server), m_network(network)
{
    if (!m_tx)      throw AssembledTransactionException("AssembledTransaction: null transaction");
    if (!m_server)  throw AssembledTransactionException("AssembledTransaction: null SorobanServer");
    if (!m_network) throw AssembledTransactionException("AssembledTransaction: null Network");
}

AssembledTransaction::~AssembledTransaction()
{
    delete m_tx;
}

AssembledTransaction& AssembledTransaction::simulate()
{
    refuseOnceSigned(m_tx, "simulate");
    // The one-argument overload, which a SorobanServer subclass may override.
    return applySimulation(m_server->simulateTransactionXdr(m_tx->toEnvelopeXdrBase64()));
}

AssembledTransaction& AssembledTransaction::simulate(const SimulateTransactionOptions& options)
{
    refuseOnceSigned(m_tx, "simulate");
    return applySimulation(m_server->simulateTransactionXdr(m_tx->toEnvelopeXdrBase64(), options));
}

AssembledTransaction& AssembledTransaction::applySimulation(const SimulateTransactionResponse& sim)
{
    // Not simulated until this simulation is applied. One that fails, or
    // cannot be applied, leaves m_tx with the earlier one's soroban data, fee
    // and auth, and sign() and signAndSend() then signed and sent that stale
    // transaction. js drops the earlier simulation's data as well.
    m_simulated = false;
    m_sim = sim;
    if (m_sim.isError()) {
        throw SimulationFailedException(
            QString("AssembledTransaction::simulate: %1").arg(m_sim.getError()));
    }
    SorobanServer::applyTransactionSimulation(m_tx, m_sim);
    m_simulated = true;
    return *this;
}

stellar::SCVal AssembledTransaction::result() const
{
    if (!m_simulated) {
        throw NotYetSimulatedException("AssembledTransaction::result: simulate() not called yet");
    }
    if (m_sim.getResults().isEmpty()) {
        throw AssembledTransactionException("AssembledTransaction::result: simulation produced no result");
    }
    const QString xdrB64 = m_sim.getResults().first().xdr;
    if (xdrB64.isEmpty()) {
        throw AssembledTransactionException("AssembledTransaction::result: empty result xdr");
    }
    const auto decoded = QByteArray::fromBase64Encoding(
        xdrB64.toLatin1(), XDR_BASE64ENCODING | QByteArray::AbortOnBase64DecodingErrors);
    if (!decoded) {
        throw AssembledTransactionException("AssembledTransaction::result: result xdr is not base64");
    }
    QByteArray raw = *decoded;
    QDataStream s(&raw, QIODevice::ReadOnly);
    stellar::SCVal v;
    s >> v;
    // A result that does not decode in full is not a result: a truncated one
    // used to come back as a default (void or zero) value.
    if (s.status() != QDataStream::Ok || !s.atEnd()) {
        throw AssembledTransactionException("AssembledTransaction::result: result xdr is not an SCVal");
    }
    return v;
}

stellar::SCVal AssembledTransaction::landedResult() const
{
    if (m_finalResponse.getStatus() != GetTransactionResponse::Status::SUCCESS)
        throw AssembledTransactionException("AssembledTransaction::landedResult: no transaction landed "
                                            "successfully (call signAndSend() first)");
    try {
        return m_finalResponse.getReturnValue();
    } catch (const std::exception& e) {
        throw AssembledTransactionException(std::string("AssembledTransaction::landedResult: ") + e.what());
    }
}

bool AssembledTransaction::isReadCall() const
{
    if (!m_simulated) {
        throw NotYetSimulatedException("isReadCall: simulate() not called yet");
    }
    if (m_tx->getOperations().size() != 1
            || !dynamic_cast<InvokeHostFunctionOperation*>(m_tx->getOperations().first()))
        return false;
    // As js reads it: the auth of the simulation's result, and the readWrite
    // footprint of its transactionData, which simulate() put on m_tx.
    const bool authorizes = !m_sim.getResults().isEmpty() && !m_sim.getResults().first().auth.isEmpty();
    const QSharedPointer<stellar::SorobanTransactionData> data = m_tx->getSorobanData();
    const bool writes = !data.isNull() && !data->resources.footprint.readWrite.isEmpty();
    return !authorizes && !writes;
}

void AssembledTransaction::refuseToSign(const char* method, bool force) const
{
    const QString where = QString::fromLatin1(method);
    // An unsimulated Soroban transaction carries no footprint and fails; js
    // and py refuse to sign one too. sign() used to sign it.
    if (!m_simulated) {
        // The last simulation failed: what the transaction carries is an
        // earlier one's. js refuses it (SimulationFailed), forced or not; py
        // refuses it unless forced.
        if (m_sim.isError()) {
            throw SimulationFailedException(
                QString("AssembledTransaction::%1: the last simulation failed (%2); simulate() "
                        "again").arg(where, m_sim.getError()));
        }
        throw NotYetSimulatedException(
            QString("AssembledTransaction::%1: simulate() not called yet").arg(where));
    }
    // The call needs archived entries: sent, it fails on chain and the fee is
    // paid. js and py refuse it (ExpiredState), forced or not.
    if (m_sim.needsRestore()) {
        throw ExpiredStateException(
            QString("AssembledTransaction::%1: the simulation asks for archived entries to be "
                    "restored first (restorePreamble); run restoreFootprint(), then simulate the "
                    "call again").arg(where));
    }
    // It changes nothing, and sent, it only pays the fee. js and py refuse it
    // unless forced (NoSignatureNeeded). It used to be signed and sent.
    if (!force && isReadCall()) {
        throw NoSignatureNeededException(
            QString("AssembledTransaction::%1: this is a read call: it changes nothing, and "
                    "sending it only pays the fee. Its answer is result(); pass force = true to "
                    "sign and send it anyway").arg(where));
    }
    // Sent so, the host refuses the authorization when the transaction is
    // applied, and the fee is paid. js and py refuse it, forced or not
    // (NeedsMoreSignatures). js leaves C... addresses out of this check, as
    // needsNonInvokerSigningBy() does; py counts them.
    const QStringList unsignedBy = needsNonInvokerSigningBy();
    if (!unsignedBy.isEmpty()) {
        throw NeedsMoreSignaturesException(
            QString("AssembledTransaction::%1: the auth entries of %2 are not signed yet; sign "
                    "them with signAuthEntries() first (see needsNonInvokerSigningBy())")
                .arg(where, unsignedBy.join(QStringLiteral(", "))),
            unsignedBy);
    }
}

AssembledTransaction& AssembledTransaction::sign(KeyPair* signer, bool force)
{
    if (!signer) throw AssembledTransactionException("AssembledTransaction::sign: null signer");
    refuseToSign("sign", force);
    m_tx->sign(signer);
    return *this;
}

GetTransactionResponse AssembledTransaction::signAndSend(KeyPair* signer, int timeoutMs, int intervalMs,
                                                         bool force)
{
    refuseToSign("signAndSend", force);
    if (signer) {
        // Sending again after TRY_AGAIN_LATER sends the same envelope: a
        // signer whose signature the transaction already carries is not added
        // twice. The second copy is an extra signature, and the network
        // refuses the transaction for it (txBAD_AUTH_EXTRA). The hint as well
        // as the signature: core matches a signature to its signer by it.
        const QByteArray hash = m_tx->hash();
        const stellar::SignatureHint hint = signer->getSignatureHint();
        bool already = false;
        for (const stellar::DecoratedSignature& s : m_tx->getSignatures()) {
            if (std::memcmp(s.hint.signatureHint, hint.signatureHint, 4) == 0
                    && signer->verify(hash, s.signature.binary())) {
                already = true;
                break;
            }
        }
        if (!already)
            m_tx->sign(signer);
    }

    try {
        m_sendResponse = m_server->sendTransactionXdr(m_tx->toEnvelopeXdrBase64());
    } catch (const SorobanTransportException& e) {
        // Qt sent it again, by itself, after the connection that carried it
        // whole closed with no answer: what ended the send came of a later
        // one — an ERROR or TRY_AGAIN_LATER answer among them, which
        // SorobanServer refuses as the answer to this send — while the RPC may
        // have taken in, and applied, the first. Said with the hash, as the
        // outcome it is. (The answer came back as this send's, and an ERROR
        // or TRY_AGAIN_LATER threw SendFailedException: "not taken in".)
        if (!e.resent())
            throw;
        const QString hash = m_tx->hashHex();
        throw SendOutcomeUnknownException(
            QString("AssembledTransaction::signAndSend: the outcome of sending transaction %1 is "
                    "not known (%2). It may still be applied: look it up by its hash "
                    "(getTransaction) before sending it again or handing its sequence number "
                    "back").arg(hash, QString::fromUtf8(e.what())),
            hash);
    }
    if (m_sendResponse.getStatus() == SendTransactionResponse::Status::ERROR_) {
        throw SendFailedException(
            QString("AssembledTransaction::signAndSend: sendTransaction returned ERROR (%1)")
                .arg(m_sendResponse.getErrorResultXdr()));
    }
    // Not taken in: there is nothing to poll for, as restoreFootprint already
    // decides. It used to be polled until the timeout, as if the RPC had said
    // PENDING. The transaction keeps its sequence number (see the header).
    if (m_sendResponse.getStatus() == SendTransactionResponse::Status::TRY_AGAIN_LATER) {
        throw SendFailedException(
            QString("AssembledTransaction::signAndSend: sendTransaction returned TRY_AGAIN_LATER"));
    }

    // Taken in: whatever the polling says, the transaction may land. A poll
    // whose own connection never opened threw SorobanTransportException with
    // mayHaveReachedServer() false, which says of a send that it never left
    // — and a caller acting on that could send the payment again.
    try {
        m_finalResponse = m_server->pollTransaction(m_sendResponse.getHash(), timeoutMs, intervalMs);
    } catch (const SorobanTransportException& e) {
        // Always named by its hash and "may still be applied", however the
        // poll failed: a poll that did reach its server was rethrown as it
        // was, and the caller then had no hash in the message to look up.
        throw SorobanTransportException(
            QString("AssembledTransaction::signAndSend: the RPC took transaction %1 in, and "
                    "polling for its result failed (%2); it may still be applied: look it up "
                    "by its hash before sending it again").arg(m_tx->hashHex(),
                                                                QString::fromUtf8(e.what())),
            true, e.resent());
    }
    return m_finalResponse;
}

QStringList AssembledTransaction::needsNonInvokerSigningBy() const
{
    if (!m_simulated) {
        throw NotYetSimulatedException("needsNonInvokerSigningBy: simulate() not called yet");
    }
    QSet<QString> seen;
    QStringList out;
    for (Operation* op : m_tx->getOperations()) {
        auto* invoke = dynamic_cast<InvokeHostFunctionOperation*>(op);
        if (!invoke) continue;
        for (const stellar::SorobanAuthorizationEntry& e : invoke->getAuth()) {
            // ADDRESS_V2 carries the same SorobanAddressCredentials payload and
            // is equally signable; only its signature preimage differs.
            // ADDRESS_WITH_DELEGATES carries it as its top-level credentials,
            // which js reads as an ADDRESS entry's (getAddressCredentials):
            // listed while that signature is missing, its delegates not looked
            // at. It was skipped, and sign() signed and sent a call the host
            // then refused.
            const stellar::SorobanAddressCredentials* creds = nullptr;
            if (e.credentials.type == stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS
                || e.credentials.type == stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS_V2)
                creds = &e.credentials.address;
            else if (e.credentials.type
                     == stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS_WITH_DELEGATES)
                creds = &e.credentials.addressWithDelegates.addressCredentials;
            else
                continue;
            // Only ACCOUNT-type addresses can be signed with a KeyPair.
            if (creds->address.type != stellar::SCAddressType::SC_ADDRESS_TYPE_ACCOUNT)
                continue;
            // Empty signature == SCV_VOID placeholder.
            if (creds->signature.type != stellar::SCValType::SCV_VOID)
                continue;
            QByteArray pub(reinterpret_cast<const char*>(creds->address.accountId), 32);
            StellarAddress addr(StellarAddress::Type::ACCOUNT, pub);
            const QString strkey = addr.toString();
            if (!seen.contains(strkey)) {
                seen.insert(strkey);
                out.append(strkey);
            }
        }
    }
    return out;
}

AssembledTransaction& AssembledTransaction::signAuthEntries(KeyPair* signer, quint32 validUntilLedger)
{
    if (!m_simulated) {
        throw NotYetSimulatedException("signAuthEntries: simulate() not called yet");
    }
    if (!signer) throw AssembledTransactionException("signAuthEntries: null signer");
    refuseOnceSigned(m_tx, "signAuthEntries");
    if (validUntilLedger == 0) {
        validUntilLedger = m_sim.getLatestLedger() + 100;
    }
    const QByteArray signerPub = signer->getPublicKey();
    const QString passphrase = m_network->getNetworkPassphrase();

    for (Operation* op : m_tx->getOperations()) {
        auto* invoke = dynamic_cast<InvokeHostFunctionOperation*>(op);
        if (!invoke) continue;
        QList<stellar::SorobanAuthorizationEntry> updated = invoke->getAuth();
        bool changed = false;
        for (stellar::SorobanAuthorizationEntry& e : updated) {
            // ADDRESS_V2 carries the same SorobanAddressCredentials payload and
            // is equally signable; only its signature preimage differs.
            if (e.credentials.type != stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS
                && e.credentials.type != stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS_V2)
                continue;
            if (e.credentials.address.address.type != stellar::SCAddressType::SC_ADDRESS_TYPE_ACCOUNT)
                continue;
            if (e.credentials.address.signature.type != stellar::SCValType::SCV_VOID)
                continue;
            if (std::memcmp(e.credentials.address.address.accountId, signerPub.constData(), 32) != 0)
                continue;
            e = SorobanCredentialsSigner::authorizeEntry(e, signer, validUntilLedger, passphrase);
            changed = true;
        }
        if (changed) invoke->setAuth(updated);
    }
    return *this;
}

GetTransactionResponse AssembledTransaction::restoreFootprint(Account* sourceAccount,
                                                              KeyPair* signer,
                                                              qint64 baseFee,
                                                              int timeoutMs,
                                                              int intervalMs,
                                                              qint64 txTimeoutSeconds)
{
    if (!m_simulated) throw NotYetSimulatedException("restoreFootprint: simulate() not called yet");
    if (!m_sim.needsRestore()) {
        throw AssembledTransactionException("restoreFootprint: simulation has no restorePreamble");
    }
    if (!sourceAccount) throw AssembledTransactionException("restoreFootprint: null sourceAccount");
    if (!signer)        throw AssembledTransactionException("restoreFootprint: null signer");

    // Decode the restorePreamble's SorobanTransactionData and build a tx
    // whose only op is RestoreFootprint, carrying that data. In full or not
    // at all: a truncated or garbled preamble used to decode as an all-default
    // footprint, which was then signed and sent.
    const auto decoded = QByteArray::fromBase64Encoding(
        m_sim.getRestorePreamble().transactionData.toLatin1(),
        XDR_BASE64ENCODING | QByteArray::AbortOnBase64DecodingErrors);
    if (!decoded) {
        throw AssembledTransactionException("restoreFootprint: restorePreamble.transactionData is not base64");
    }
    QByteArray raw = *decoded;
    QDataStream s(&raw, QIODevice::ReadOnly);
    stellar::SorobanTransactionData data;
    s >> data;
    if (raw.isEmpty() || s.status() != QDataStream::Ok || !s.atEnd() || data.resourceFee < 0) {
        throw AssembledTransactionException(
            "restoreFootprint: restorePreamble.transactionData is not a SorobanTransactionData");
    }

    TransactionBuilder b(sourceAccount, m_network);
    b.addOperation(RestoreFootprintOperation::create());
    b.setBaseFee(baseFee);
    b.setTimeout(txTimeoutSeconds);
    b.setSorobanData(data);

    // TransactionBuilder already sums data.resourceFee with baseFee*numOps,
    // so no extra fee math is needed here — restorePreamble.minResourceFee
    // is just a sanity hint that is already baked into the SorobanTransactionData.
    //
    // build() takes the next sequence number from the caller's account. Until
    // the RPC has taken the transaction in, nothing can use that number up, so
    // a restore known to have failed before then — signing or encoding, a
    // connection that never opened, or the one send made answered ERROR or
    // TRY_AGAIN_LATER — hands it back, as ContractClient::invoke does. (An
    // answer to a send Qt made again, by itself, is not the answer to the
    // first: SorobanServer throws SorobanTransportException, resent, for it,
    // which keeps the number below.) Only if
    // the account still holds this call's number: the send waits in a nested
    // event loop, where a later one may have been taken. Any other failure of
    // the send, and an answer of PENDING or DUPLICATE, may be followed by the
    // restore landing, so the number stays taken.
    std::unique_ptr<Transaction> restoreTx(b.build());
    const qint64 reserved = sourceAccount->getSequenceNumber();
    auto handBack = [&]() {
        if (sourceAccount->getSequenceNumber() == reserved)
            sourceAccount->decrementSequenceNumber();
    };

    // Signed and encoded before anything is sent: nothing here can have used
    // the number.
    QString envelope;
    QString hash;
    try {
        restoreTx->sign(signer);
        envelope = restoreTx->toEnvelopeXdrBase64();
        hash = restoreTx->hashHex();
    } catch (...) {
        handBack();
        throw;
    }
    restoreTx.reset();

    // A send that threw was handed its number back whatever the reason, and a
    // timeout or a cancel can come after the RPC took the restore in: the next
    // transaction built on the account then reused the number of a restore
    // that could still land. Only a connection that never opened is known not
    // to have reached it.
    auto outcomeUnknown = [&](const QString& why) {
        return SendOutcomeUnknownException(
            QString("restoreFootprint: sending the restore failed after it may have reached the "
                    "server (%1). It may still be applied, so its sequence number %2 stays "
                    "taken: look transaction %3 up (getTransaction) before building on the "
                    "account again").arg(why, QString::number(reserved), hash),
            hash);
    };
    SendTransactionResponse sendResp;
    try {
        sendResp = m_server->sendTransactionXdr(envelope);
    } catch (const SorobanTransportException& e) {
        if (!e.mayHaveReachedServer()) {
            handBack();
            throw;
        }
        throw outcomeUnknown(QString::fromUtf8(e.what()));
    } catch (const std::exception& e) {
        throw outcomeUnknown(QString::fromUtf8(e.what()));
    } catch (...) {
        throw outcomeUnknown(QStringLiteral("an unknown failure"));
    }

    if (sendResp.getStatus() == SendTransactionResponse::Status::ERROR_) {
        handBack();
        throw SendFailedException(
            QString("restoreFootprint: sendTransaction returned ERROR (%1)")
                .arg(sendResp.getErrorResultXdr()));
    }
    // Not taken in: there is nothing to poll for. It used to be polled until
    // the timeout, as if the RPC had said PENDING.
    if (sendResp.getStatus() == SendTransactionResponse::Status::TRY_AGAIN_LATER) {
        handBack();
        throw SendFailedException(
            QString("restoreFootprint: sendTransaction returned TRY_AGAIN_LATER"));
    }
    // Taken in (PENDING or DUPLICATE): a polling failure leaves the outcome
    // unknown, and says so with the hash, as a send's does. It used to come
    // through as the poll threw it: without the restore's hash, which the
    // caller has no other way to learn, and — for a poll whose connection
    // never opened — as a SorobanTransportException saying the request was
    // not sent, which is what a handed-back number is documented by.
    try {
        return m_server->pollTransaction(sendResp.getHash(), timeoutMs, intervalMs);
    } catch (const std::exception& e) {
        throw SendOutcomeUnknownException(
            QString("restoreFootprint: the RPC took the restore in, and polling for its result "
                    "failed (%1). It may still be applied, so its sequence number %2 stays "
                    "taken: look transaction %3 up (getTransaction) before building on the "
                    "account again").arg(QString::fromUtf8(e.what()), QString::number(reserved), hash),
            hash);
    } catch (...) {
        throw SendOutcomeUnknownException(
            QString("restoreFootprint: the RPC took the restore in, and polling for its result "
                    "failed. It may still be applied, so its sequence number %1 stays taken: "
                    "look transaction %2 up (getTransaction) before building on the account "
                    "again").arg(QString::number(reserved), hash),
            hash);
    }
}
QSTELLAR_END_NS
