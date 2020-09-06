#include "sep10challenge.h"
#include "util.h"
#include "account.h"
#include <QDateTime>
#include <QRandomGenerator>
#include "strkey.h"
#define NONCE_SIZE 48

Transaction* Sep10Challenge::buildChallengeTx(KeyPair *serverSignerSecret, QString clientAccountID, QString domainName, qint64 timebound,Network* network)
{
    QByteArray randomNonce = Util::generateRandomNonce(NONCE_SIZE);//48 random bytes converted to base64 is 64 bytes
    randomNonce = randomNonce.toBase64(QByteArray::Base64Option::Base64UrlEncoding| QByteArray::OmitTrailingEquals);

    // represent server signing account
    Account *sa = new Account(new KeyPair(*serverSignerSecret),-1);//as is a temporal account, we use a keypair copy.
    TimeBounds *timeBounds;
    if(timebound>0)
    {
        qint64 now = QDateTime::currentMSecsSinceEpoch()/ 1000L;
        qint64 timeoutTimestamp = now + timebound;
        timeBounds = new TimeBounds(now,timeoutTimestamp);
    }
    else
        timeBounds= new TimeBounds(0,0);
    ManageDataOperation* dataOp = new ManageDataOperation(domainName + " auth",randomNonce);
    dataOp->setSourceAccount(clientAccountID);
    Transaction *tx = Transaction::Builder(sa,network).addOperation(dataOp).addTimeBounds(timeBounds).setBaseFee(Transaction::Builder::BASE_FEE).build();
    tx->sign(serverSignerSecret);
    return tx;
}

Sep10Challenge::ChallengeTransaction * Sep10Challenge::readChallengeTransaction(QString challengeXdr, QString serverAccountId, QString domainName, Network *network)  {
    // decode the received input as a base64-urlencoded XDR representation of Stellar transaction envelope
    AbstractTransaction* parsed = Transaction::fromEnvelopeXdr(challengeXdr, network);
    Transaction* transaction = dynamic_cast<Transaction*>(parsed);
    if (!transaction) {
        throw std::runtime_error("Transaction cannot be a fee bump transaction");
    }


    if (StrKey::decodeVersionByte(serverAccountId) != StrKey::VersionByte::ACCOUNT_ID) {
        throw  std::runtime_error("serverAccountId is not a valid account id");
    }

    // verify that transaction source account is equal to the server's signing key
    if (serverAccountId != (transaction->getSourceAccount())) {
        throw std::runtime_error("Transaction source account is not equal to server's account.");
    }

    // verify that transaction sequenceNumber is equal to zero
    if (transaction->getSequenceNumber() != 0L) {
        throw std::runtime_error("The transaction sequence number should be zero.");
    }

    // verify that transaction has time bounds set, and that current time is between the minimum and maximum bounds.
    if (transaction->getTimeBounds() == nullptr) {
        throw std::runtime_error("Transaction requires timebounds.");
    }

    long maxTime = transaction->getTimeBounds()->getMaxTime();
    long minTime = transaction->getTimeBounds()->getMinTime();
    if (maxTime == 0L) {
        throw std::runtime_error("Transaction requires non-infinite timebounds.");
    }

    long currentTime = QDateTime::currentMSecsSinceEpoch() / 1000L;
    if (currentTime < minTime || currentTime > maxTime) {
        throw std::runtime_error("Transaction is not within range of the specified timebounds.");
    }

    // verify that transaction contains a single Manage Data operation and its source account is not null
    if (transaction->getOperations().length() != 1) {
        throw std::runtime_error("Transaction requires a single ManageData operation.");
    }
    Operation* operation = transaction->getOperations()[0];
    ManageDataOperation* manageDataOperation = dynamic_cast<ManageDataOperation*>(operation);

    if (!operation) {
        throw std::runtime_error("Operation type should be ManageData.");
    }

    // verify that transaction envelope has a correct signature by server's signing key
    QString clientAccountId = manageDataOperation->getSourceAccount();
    if (clientAccountId == nullptr) {
        throw std::runtime_error("Operation should have a source account.");
    }

    if (QString("%1 auth").arg(domainName) !=  manageDataOperation->getName()){
        throw std::runtime_error("The transaction's operation key name does not include the expected home domain.");
    }

    if (StrKey::decodeVersionByte(clientAccountId) != StrKey::VersionByte::ACCOUNT_ID) {
        throw std::runtime_error("clientAccountId is not a valid account id");
    }

    // verify manage data value
    if (manageDataOperation->getValue().length() != 64) {
        throw std::runtime_error("Random nonce encoded as base64 should be 64 bytes long.");
    }

    QByteArray nonce= QByteArray::fromBase64(manageDataOperation->getValue());
    if(nonce.isEmpty())
    {
        throw std::runtime_error("Failed to decode random nonce provided in ManageData operation.");
    }

    if (nonce.length() != 48) {
        throw std::runtime_error("Random nonce before encoding as base64 should be 48 bytes long.");
    }

    if (!verifyTransactionSignature(transaction, serverAccountId, domainName)) {
        throw std::runtime_error("Transaction not signed by server");
    }

    return new ChallengeTransaction(transaction, clientAccountId);
}

QSet<QString> Sep10Challenge::verifyChallengeTransactionSigners(QString challengeXdr, QString serverAccountId, QString domainName, QSet<QString> signers, Network *network) {
    if (signers.isEmpty()) {
        throw std::runtime_error("No verifiable signers provided, at least one G... address must be provided.");
    }

    // Read the transaction which validates its structure.
    ChallengeTransaction* parsedChallengeTransaction = readChallengeTransaction(challengeXdr, serverAccountId, domainName, network);
    Transaction* transaction = parsedChallengeTransaction->getTransaction();

    // Ensure the server account ID is an address and not a seed.
    KeyPair* serverKeyPair = KeyPair::fromAccountId(serverAccountId);

    // Deduplicate the client signers and ensure the server is not included
    // anywhere we check or output the list of signers.
    QSet<QString> clientSigners;
    for (QString signer : signers) {
        // Ignore non-G... account/address signers.
        StrKey::VersionByte versionByte;
        try {
            versionByte = StrKey::decodeVersionByte(signer);
        } catch (std::runtime_error e) {
            continue;
        }

        if (StrKey::VersionByte::ACCOUNT_ID != versionByte) {
            continue;
        }

        // Ignore the server signer if it is in the signers list. It's
        // important when verifying signers of a challenge transaction that we
        // only verify and return client signers. If an account has the server
        // as a signer the server should not play a part in the authentication
        // of the client.
        if (serverKeyPair->getAccountId() == signer) {
            continue;
        }
        clientSigners.insert(signer);
    }

    // Don't continue if none of the signers provided are in the final list.
    if (clientSigners.isEmpty()) {
        throw std::runtime_error("No verifiable signers provided, at least one G... address must be provided.");
    }

    // Verify all the transaction's signers (server and client) in one
    // hit. We do this in one hit here even though the server signature was
    // checked in the readChallengeTx to ensure that every signature and signer
    // are consumed only once on the transaction.
    QSet<QString> allSigners = QSet<QString>(clientSigners);
    allSigners.insert(serverKeyPair->getAccountId());
    QSet<QString> signersFound = verifyTransactionSignatures(transaction, domainName, allSigners);

    // Confirm the server is in the list of signers found and remove it.
    bool serverSignerFound = signersFound.remove(serverKeyPair->getAccountId());

    // Confirm we matched a signature to the server signer.
    if (!serverSignerFound) {
        throw std::runtime_error("Transaction not signed by server.");
    }

    // Confirm we matched signatures to the client signers.
    if (signersFound.isEmpty()) {
        throw std::runtime_error("Transaction not signed by any client signer.");
    }

    // Confirm all signatures were consumed by a signer.
   if (signersFound.size() != transaction->getSignatures().size() - 1) {
        throw std::runtime_error("Transaction has unrecognized signatures.");
    }

    return signersFound;
}

QSet<QString> Sep10Challenge::verifyChallengeTransactionThreshold(QString challengeXdr, QString serverAccountId, QString domainName, int threshold, QSet<Sep10Challenge::Signer> signers, Network *network)
{
    if (signers.isEmpty()) {
        throw std::runtime_error("No verifiable signers provided, at least one G... address must be provided.");
    }

    QMap<QString, int> weightsForSigner;
    QSet<QString> signersSet;
    for (Sep10Challenge::Signer signer : signers) {
        weightsForSigner.insert(signer.getKey(), signer.getWeight());
        signersSet.insert(signer.getKey());
    }
    QSet<QString> signersFound = verifyChallengeTransactionSigners(challengeXdr, serverAccountId, domainName, signersSet, network);

    int sum = 0;
    for (QString signer : signersFound) {
        Integer weight = weightsForSigner.value(signer);
        if (!weight) {
            sum += weight;
        }
    }

    if (sum < threshold) {
        throw std::runtime_error("Signers weight do not meet threshold.");
    }

    return signersFound;
}

QSet<QString> Sep10Challenge::verifyTransactionSignatures(Transaction *transaction, QString domainName, QSet<QString> signers){
    Q_UNUSED(domainName)
    if (transaction->getSignatures().isEmpty()) {
        throw std::runtime_error("Transaction has no signatures.");
    }

    QByteArray txHash = transaction->hash();

    // find and verify signatures
    QSet<QString> signersFound;

    QMultiMap<stellar::SignatureHint,  QByteArray> signatures;
    for (stellar::DecoratedSignature& decoratedSignature : transaction->getSignatures()) {
        signatures.insert(decoratedSignature.hint, decoratedSignature.signature.binary());
    }

    for (QString signer : signers) {
        KeyPair* keyPair = KeyPair::fromAccountId(signer);
        stellar::SignatureHint hint = keyPair->getSignatureHint();

        for (QByteArray& signature : signatures.values(hint)) {
            if (keyPair->verify(txHash, signature)) {
                signersFound.insert(signer);
                // explicitly ensure that a transaction signature cannot be
                // mapped to more than one signer
                signatures.remove(hint, signature);
                break;
            }
        }
    }

    return signersFound;
}

bool Sep10Challenge::verifyTransactionSignature(Transaction *transaction, QString accountId, QString domainName) {
    return !verifyTransactionSignatures(transaction, domainName, QSet<QString>() << accountId).isEmpty();
}

Sep10Challenge::ChallengeTransaction::ChallengeTransaction(Transaction *transaction, QString clientAccountId):m_transaction(transaction),m_clientAccountId(clientAccountId)
{
}

Transaction *Sep10Challenge::ChallengeTransaction::getTransaction() const
{
    return m_transaction;
}

QString Sep10Challenge::ChallengeTransaction::getClientAccountId() const
{
    return m_clientAccountId;
}

int Sep10Challenge::ChallengeTransaction::hashCode() const
{
    return qHash(m_transaction->hash()) ^qHash(m_clientAccountId);
}

bool Sep10Challenge::ChallengeTransaction::equals(const Sep10Challenge::ChallengeTransaction *other) const
{
    return m_transaction->hash()==other->m_transaction->hash()
            && m_clientAccountId == other->m_clientAccountId;
}

Sep10Challenge::Signer::Signer(QString key, int weight):m_key(key),m_weight(weight) {
}

QString Sep10Challenge::Signer::getKey() {
    return m_key;
}

int Sep10Challenge::Signer::getWeight() const {
    return m_weight;
}

int Sep10Challenge::Signer::hashCode() const {
    return qHash(m_key) ^ qHash(m_weight);
}

bool Sep10Challenge::Signer::equals(const Sep10Challenge::Signer *other) const {
    return m_key==other->m_key && m_weight == other->m_weight;
}
