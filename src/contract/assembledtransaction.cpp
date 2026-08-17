#include "assembledtransaction.h"

#include <QByteArray>
#include <QDataStream>
#include <stdexcept>

#include <QSet>
#include <cstring>

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
using qstellar::exception::NotYetSimulatedException;
using qstellar::exception::SendFailedException;
using qstellar::exception::SimulationFailedException;


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
    m_sim = m_server->simulateTransactionXdr(m_tx->toEnvelopeXdrBase64());
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
    QByteArray raw = QByteArray::fromBase64(xdrB64.toLatin1(), XDR_BASE64ENCODING);
    QDataStream s(&raw, QIODevice::ReadOnly);
    stellar::SCVal v;
    s >> v;
    return v;
}

AssembledTransaction& AssembledTransaction::sign(KeyPair* signer)
{
    if (!signer) throw AssembledTransactionException("AssembledTransaction::sign: null signer");
    m_tx->sign(signer);
    return *this;
}

GetTransactionResponse AssembledTransaction::signAndSend(KeyPair* signer, int timeoutMs, int intervalMs)
{
    if (signer) m_tx->sign(signer);

    m_sendResponse = m_server->sendTransactionXdr(m_tx->toEnvelopeXdrBase64());
    if (m_sendResponse.getStatus() == SendTransactionResponse::Status::ERROR_) {
        throw SendFailedException(
            QString("AssembledTransaction::signAndSend: sendTransaction returned ERROR (%1)")
                .arg(m_sendResponse.getErrorResultXdr()));
    }

    m_finalResponse = m_server->pollTransaction(m_sendResponse.getHash(), timeoutMs, intervalMs);
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
            if (e.credentials.type != stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS)
                continue;
            // Only ACCOUNT-type addresses can be signed with a KeyPair.
            if (e.credentials.address.address.type != stellar::SCAddressType::SC_ADDRESS_TYPE_ACCOUNT)
                continue;
            // Empty signature == SCV_VOID placeholder.
            if (e.credentials.address.signature.type != stellar::SCValType::SCV_VOID)
                continue;
            QByteArray pub(reinterpret_cast<const char*>(e.credentials.address.address.accountId), 32);
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
            if (e.credentials.type != stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS)
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
                                                              int baseFee,
                                                              int timeoutMs,
                                                              int intervalMs)
{
    if (!m_simulated) throw NotYetSimulatedException("restoreFootprint: simulate() not called yet");
    if (!m_sim.needsRestore()) {
        throw AssembledTransactionException("restoreFootprint: simulation has no restorePreamble");
    }
    if (!sourceAccount) throw AssembledTransactionException("restoreFootprint: null sourceAccount");
    if (!signer)        throw AssembledTransactionException("restoreFootprint: null signer");

    // Decode the restorePreamble's SorobanTransactionData and build a tx
    // whose only op is RestoreFootprint, carrying that data.
    QByteArray raw = QByteArray::fromBase64(m_sim.getRestorePreamble().transactionData.toLatin1(),
                                            XDR_BASE64ENCODING);
    QDataStream s(&raw, QIODevice::ReadOnly);
    stellar::SorobanTransactionData data;
    s >> data;

    TransactionBuilder b(sourceAccount, m_network);
    b.addOperation(RestoreFootprintOperation::create());
    b.setBaseFee(baseFee);
    b.setTimeout(TransactionBuilder::TIMEOUT_INFINITE);
    b.setSorobanData(data);

    // TransactionBuilder already sums data.resourceFee with baseFee*numOps,
    // so no extra fee math is needed here — restorePreamble.minResourceFee
    // is just a sanity hint that is already baked into the SorobanTransactionData.
    Transaction* restoreTx = b.build();
    restoreTx->sign(signer);

    SendTransactionResponse sendResp = m_server->sendTransactionXdr(restoreTx->toEnvelopeXdrBase64());
    delete restoreTx;

    if (sendResp.getStatus() == SendTransactionResponse::Status::ERROR_) {
        throw SendFailedException(
            QString("restoreFootprint: sendTransaction returned ERROR (%1)")
                .arg(sendResp.getErrorResultXdr()));
    }
    return m_server->pollTransaction(sendResp.getHash(), timeoutMs, intervalMs);
}
QSTELLAR_END_NS
