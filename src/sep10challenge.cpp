#include "sep10challenge.h"
#include "util.h"
#include "account.h"
#include <QDateTime>
#include <QRandomGenerator>
#include <memory>
#include "strkey.h"
#include "memo.h"


#define NONCE_SIZE 48

const int Sep10Challenge::GRACE_PERIOD_SECONDS = 5 * 60;
const QString Sep10Challenge::CLIENT_DOMAIN_DATA_NAME = "client_domain";
const QString Sep10Challenge::HOME_DOMAIN_MANAGER_DATA_NAME_FLAG = "auth";
const QString Sep10Challenge::WEB_AUTH_DOMAIN_MANAGER_DATA_NAME = "web_auth_domain";

Transaction* Sep10Challenge::buildChallengeTx(KeyPair *serverSignerSecret, QString clientAccountID, QString domainName, QString webAuthDomain, qint64 timebound, Network* network
                                              , QString clientDomain, QString clientSigningKey, Memo *memo)
{
    // FIX §5.1: timebound<=0 used to build TimeBounds(0,0), which
    // readChallengeTransaction itself rejects ("non-infinite timebounds").
    // SEP-10 forbids expiring-never challenges: fail fast.
    if (timebound <= 0)
        throw std::runtime_error("timebound must be > 0 (SEP-10 requires non-infinite timebounds)");
    qint64 now = QDateTime::currentMSecsSinceEpoch() / 1000L;
    qint64 timeoutTimestamp = now + timebound;
    TimeBounds *timeBounds = new TimeBounds(now, timeoutTimestamp);
    return buildChallengeTx(serverSignerSecret, clientAccountID, domainName, webAuthDomain, timeBounds, network, clientDomain, clientSigningKey, memo);
}


Transaction* Sep10Challenge::buildChallengeTx(KeyPair* serverSignerSecret, QString clientAccountID, QString domainName, QString webAuthDomain, TimeBounds* timeBounds, Network *network
                                         , QString clientDomain, QString clientSigningKey, Memo *memo)
{
    // FIX §1.6: previously, any exception after creating `sa`, `timeBounds`,
    // or the ManageDataOperation instances leaked them. Now they live under
    // std::unique_ptr until release() transfers ownership to the Builder.
    if (clientDomain.isEmpty() != clientSigningKey.isEmpty()) {
        throw std::runtime_error("clientDomain is required if clientSigningKey is provided");
    }
    if (StrKey::decodeVersionByte(clientAccountID) != StrKey::VersionByte::ACCOUNT_ID)
        throw std::runtime_error("Version byte is invalid");
    // Validate clientSigningKey early so we don't have to clean up later.
    if (!clientSigningKey.isEmpty() &&
        StrKey::decodeVersionByte(clientSigningKey) != StrKey::VersionByte::ACCOUNT_ID) {
        throw std::runtime_error("clientSigningKey is not a valid account id");
    }
    // Validate memo (only MemoId allowed) before allocating.
    if (memo && !dynamic_cast<MemoId*>(memo)) {
        throw std::runtime_error("only memo type `id` is supported");
    }

    QByteArray randomNonce = Util::generateRandomNonce(NONCE_SIZE); // 48 bytes → base64 64 chars
    randomNonce = randomNonce.toBase64(QByteArray::Base64Option::Base64UrlEncoding | QByteArray::OmitTrailingEquals);

    // Temporary server account (sequence -1; build() bumps it to 0).
    std::unique_ptr<Account> sa(new Account(new KeyPair(*serverSignerSecret), -1));

    std::unique_ptr<ManageDataOperation> domainNameOperation(
        new ManageDataOperation(domainName + " " + HOME_DOMAIN_MANAGER_DATA_NAME_FLAG, randomNonce));
    domainNameOperation->setSourceAccount(clientAccountID);

    std::unique_ptr<ManageDataOperation> webAuthDomainOperation(
        new ManageDataOperation(WEB_AUTH_DOMAIN_MANAGER_DATA_NAME, webAuthDomain.toUtf8()));
    webAuthDomainOperation->setSourceAccount(serverSignerSecret->getAccountId());

    auto builder = Transaction::Builder(AccountConverter().enableMuxed(), sa.get(), network)
        .addOperation(domainNameOperation.release())
        .addOperation(webAuthDomainOperation.release())
        .addTimeBounds(timeBounds)
        .setBaseFee(Transaction::Builder::BASE_FEE);

    if (!clientSigningKey.isEmpty()) {
        std::unique_ptr<ManageDataOperation> clientDomainOp(
            new ManageDataOperation(CLIENT_DOMAIN_DATA_NAME, clientDomain.toLatin1()));
        clientDomainOp->setSourceAccount(clientSigningKey);
        builder.addOperation(clientDomainOp.release());
    }

    if (memo) {
        builder.addMemo(memo);
    }

    Transaction *tx = builder.build();
    tx->sign(serverSignerSecret);
    // `sa` released by unique_ptr at scope exit (Builder doesn't own Account).
    return tx;
}

Sep10Challenge::ChallengeTransaction * Sep10Challenge::readChallengeTransaction(QString challengeXdr, QString serverAccountId, QStringList domainNames, QString webAuthDomain, Network *network)  {
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

    Memo* memo = transaction->getMemo();
    if (memo && !(dynamic_cast<MemoNone*>(memo) || dynamic_cast<MemoId*>(memo))) {
        throw std::runtime_error("only memo type `id` is supported");
    }

    // verify that transaction has time bounds set, and that current time is between the minimum and maximum bounds.
    if (transaction->getTimeBounds() == nullptr) {
        throw std::runtime_error("Transaction requires timebounds.");
    }

    // FIX §5.4: TimeBounds returns qint64; `long` is 32-bit on MSVC (LLP64)
    // and truncates post-2038 on Windows.
    qint64 maxTime = transaction->getTimeBounds()->getMaxTime();
    qint64 minTime = transaction->getTimeBounds()->getMinTime();
    if (maxTime == 0) {
        throw std::runtime_error("Transaction requires non-infinite timebounds.");
    }

    qint64 currentTime = QDateTime::currentMSecsSinceEpoch() / 1000L;
    if ((currentTime + GRACE_PERIOD_SECONDS) < minTime || currentTime > maxTime) {
        throw std::runtime_error("Transaction is not within range of the specified timebounds.");
    }

    // verify that transaction contains  at least one Manage Data operation and its source account is not null
    if (transaction->getOperations().length() < 1) {
        throw std::runtime_error("Transaction requires at least one ManageData operation.");
    }
    Operation* operation = transaction->getOperations().at(0);
    ManageDataOperation* manageDataOperation = dynamic_cast<ManageDataOperation*>(operation);

    if (!manageDataOperation) {
        throw std::runtime_error("Operation type should be ManageData.");
    }

    // verify that transaction envelope has a correct signature by server's signing key
    QString clientAccountId = manageDataOperation->getSourceAccount();
    // FIX §5.2 + §5.3: QString == nullptr is always false (QString constructs
    // an empty string from nullptr). The check was silently skipped — an
    // attacker could pass an op with no source account. Use isEmpty() to
    // match the subsequent-ops check below.
    if (clientAccountId.isEmpty()) {
        throw std::runtime_error("Operation should have a source account.");
    }
    QString matchedDomainName;
    for (const QString& homeDomain : domainNames) {
        if ((homeDomain + " " + HOME_DOMAIN_MANAGER_DATA_NAME_FLAG)==manageDataOperation->getName()) {
            matchedDomainName = homeDomain;
            break;
        }
    }
    if (matchedDomainName.isNull()){
        throw std::runtime_error("The transaction's operation key name does not include one of the expected home domains.");
    }

    if (StrKey::decodeVersionByte(clientAccountId) != StrKey::VersionByte::ACCOUNT_ID) {
        throw std::runtime_error("clientAccountId is not a valid account id");
    }

    // verify manage data value
    QByteArray nonce = manageDataOperation->getValue();
    if (nonce.length() != 64) {
        throw std::runtime_error("Random nonce encoded as base64 should be 64 bytes long.");
    }

    nonce= QByteArray::fromBase64(nonce,QByteArray::Base64Option::Base64UrlEncoding| QByteArray::OmitTrailingEquals);
    if(nonce.isEmpty())
    {
        throw std::runtime_error("Failed to decode random nonce provided in ManageData operation.");
    }

    if (nonce.length() != 48) {
        throw std::runtime_error("Random nonce before encoding as base64 should be 48 bytes long.");
    }


    QByteArray webAuthDomaidUtf8 = webAuthDomain.toUtf8();
    // verify subsequent operations are manage data ops with source account set to server account
    for(int i=1;i<transaction->getOperations().length();i++)
    {
        auto subsequentOp = transaction->getOperations().at(i);
        ManageDataOperation* subsequentManageDataOperation = dynamic_cast<ManageDataOperation*>(subsequentOp);
        if (!subsequentManageDataOperation)
            throw std::runtime_error("Operation type should be ManageData.");
        // verify that transaction envelope has a correct signature by server's signing key
        QString subsequentClientAccountId = subsequentManageDataOperation->getSourceAccount();
        if (subsequentClientAccountId.isEmpty())
            throw std::runtime_error("Operation should have a source account.");
        if ((subsequentManageDataOperation->getName() != CLIENT_DOMAIN_DATA_NAME) && subsequentClientAccountId != serverAccountId)
            throw std::runtime_error("subsequent operations are unrecognized");

        if (WEB_AUTH_DOMAIN_MANAGER_DATA_NAME == subsequentManageDataOperation->getName()) {
           if (subsequentManageDataOperation->getValue().isNull()) {
             throw std::runtime_error("'web_auth_domain' operation value should not be null.");
           }
           if (webAuthDomaidUtf8 !=  subsequentManageDataOperation->getValue()) {
             throw std::runtime_error("'web_auth_domain' operation value does not match.");
           }
         }
    }

    if (!verifyTransactionSignature(transaction, serverAccountId)) {
        throw std::runtime_error("Transaction not signed by server");
    }

    return new ChallengeTransaction(transaction, clientAccountId, matchedDomainName);
}

Sep10Challenge::ChallengeTransaction *Sep10Challenge::readChallengeTransaction(QString challengeXdr, QString serverAccountId, QString domainName, QString webAuthDomain, Network *network)
{
    return readChallengeTransaction(challengeXdr, serverAccountId, QStringList()<< domainName, webAuthDomain, network);
}

QSet<QString> Sep10Challenge::verifyChallengeTransactionSigners(QString challengeXdr, QString serverAccountId, QStringList domainNames, QString webAuthDomain, QSet<QString> signers, Network *network) {
    if (signers.isEmpty()) {
        throw std::runtime_error("No verifiable signers provided, at least one G... address must be provided.");
    }

    // Read the transaction which validates its structure.
    // FIX §1.5: parsedChallengeTransaction and serverKeyPair used to leak.
    // unique_ptr frees both on any return / exception path. ChallengeTransaction
    // owns its inner Transaction; getTransaction() returns a raw observer.
    std::unique_ptr<ChallengeTransaction> parsedChallengeTransaction(
        readChallengeTransaction(challengeXdr, serverAccountId, domainNames, webAuthDomain, network));
    Transaction* transaction = parsedChallengeTransaction->getTransaction();

    // Ensure the server account ID is an address and not a seed.
    std::unique_ptr<KeyPair> serverKeyPair(KeyPair::fromAccountId(serverAccountId));

    // Deduplicate the client signers and ensure the server is not included
    // anywhere we check or output the list of signers.
    QSet<QString> clientSigners;
    for (const QString& signer : signers) {
        // Ignore non-G... account/address signers.
        StrKey::VersionByte versionByte;
        try {
            versionByte = StrKey::decodeVersionByte(signer);
        } catch (const std::runtime_error& e) {
            Q_UNUSED(e)
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


    QString clientDomainSigner;

    for (Operation* op : transaction->getOperations()) {
        if (!(dynamic_cast<ManageDataOperation*>(op))) {
            throw std::runtime_error("Operation type should be ManageData.");
        }
        ManageDataOperation* manageDataOp = (ManageDataOperation*) op;
        if (manageDataOp->getSourceAccount().isNull()) {
            throw std::runtime_error("Operation should have a source account.");
        }
        if (manageDataOp->getName() == (CLIENT_DOMAIN_DATA_NAME)) {
            allSigners.insert(manageDataOp->getSourceAccount());
            clientDomainSigner = manageDataOp->getSourceAccount();
            break;
        }
    }


    QSet<QString> signersFound = verifyTransactionSignatures(transaction, allSigners);

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
    int expectedSignaturesLength = transaction->getSignatures().size() - 1;

    if (!clientDomainSigner.isNull()) {
        // Confirm the client_domain signer is in the list of signers found and remove it.
        bool clientSignerFound = signersFound.remove(clientDomainSigner);

        // Confirm we matched a signature to the client_domain signer.
        if (!clientSignerFound) {
            throw std::runtime_error("Transaction not signed by by the source account of the 'client_domain'.");
        }
        expectedSignaturesLength--;
    }
    // Confirm all signatures were consumed by a signer.
   if (signersFound.size() != expectedSignaturesLength) {
        throw std::runtime_error("Transaction has unrecognized signatures.");
    }

    return signersFound;
}

QSet<QString> Sep10Challenge::verifyChallengeTransactionThreshold(QString challengeXdr, QString serverAccountId, QStringList domainNames, QString webAuthDomain, int threshold, QSet<Sep10Challenge::Signer> signers, Network *network)
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
    QSet<QString> signersFound = verifyChallengeTransactionSigners(challengeXdr, serverAccountId, domainNames,webAuthDomain, signersSet, network);

    int sum = 0;
    for (const QString& signer : signersFound) {
        Integer weight = weightsForSigner.value(signer);
        if (weight.filled) {
            sum += weight.value;
        }
    }

    if (sum < threshold) {
        throw std::runtime_error("Signers weight do not meet threshold.");
    }

    return signersFound;
}

QSet<QString> Sep10Challenge::verifyChallengeTransactionThreshold(QString challengeXdr, QString serverAccountId, QString domainName, QString webAuthDomain, int threshold, QSet<Sep10Challenge::Signer> signers, Network *network)
{
    return verifyChallengeTransactionThreshold(challengeXdr,serverAccountId,QStringList() << domainName,webAuthDomain,threshold,signers,network);
}

QSet<QString> Sep10Challenge::verifyTransactionSignatures(Transaction *transaction, QSet<QString> signers){
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

    for (const QString& signer : signers) {
        // FIX §1.4: leaked one KeyPair per signer (DoS via public SEP-10
        // endpoint). std::unique_ptr frees it at scope exit.
        std::unique_ptr<KeyPair> keyPair(KeyPair::fromAccountId(signer));
        stellar::SignatureHint hint = keyPair->getSignatureHint();

        auto signaturesFiltered = signatures.values(hint);
        for (const QByteArray& signature : signaturesFiltered) {
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

bool Sep10Challenge::verifyTransactionSignature(Transaction *transaction, QString accountId) {
    return !verifyTransactionSignatures(transaction, QSet<QString>() << accountId).isEmpty();
}

Sep10Challenge::ChallengeTransaction::ChallengeTransaction(Transaction *transaction, QString clientAccountId, QString matchedHomeDomain)
    :m_transaction(transaction),m_clientAccountId(clientAccountId),m_matchedHomeDomain(matchedHomeDomain)
{
}

Sep10Challenge::ChallengeTransaction::~ChallengeTransaction()
{
    // Takes real ownership of m_transaction (used to leak whenever the
    // caller didn't manually delete getTransaction()).
    delete m_transaction;
}

Transaction *Sep10Challenge::ChallengeTransaction::getTransaction() const
{
    return m_transaction;
}

QString Sep10Challenge::ChallengeTransaction::getClientAccountId() const
{
    return m_clientAccountId;
}

QString Sep10Challenge::ChallengeTransaction::getMatchedHomeDomain() const
{
    return m_matchedHomeDomain;
}

int Sep10Challenge::ChallengeTransaction::hashCode() const
{
    return static_cast<int>(qHash(m_transaction->hash()) ^ qHash(m_clientAccountId) ^ qHash(m_matchedHomeDomain));
}

bool Sep10Challenge::ChallengeTransaction::equals(const Sep10Challenge::ChallengeTransaction *other) const
{
    return m_transaction->hash()==other->m_transaction->hash()
            && m_clientAccountId == other->m_clientAccountId
            && m_matchedHomeDomain == other->m_matchedHomeDomain;
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

bool Sep10Challenge::Signer::operator==(const Sep10Challenge::Signer &other) const
{
    return m_key==other.m_key && m_weight == other.m_weight;
}
