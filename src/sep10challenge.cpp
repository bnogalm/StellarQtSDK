#include "sep10challenge.h"
#include <limits>
#include "util.h"
#include "account.h"
#include <QDateTime>
#include <QUrl>
#include <QRandomGenerator>
#include <memory>
#include "strkey.h"
#include "memo.h"

QSTELLAR_BEGIN_NS



#define NONCE_SIZE 48

const int Sep10Challenge::GRACE_PERIOD_SECONDS = 5 * 60;
const QString Sep10Challenge::CLIENT_DOMAIN_DATA_NAME = "client_domain";
const QString Sep10Challenge::HOME_DOMAIN_MANAGER_DATA_NAME_FLAG = "auth";
const QString Sep10Challenge::WEB_AUTH_DOMAIN_MANAGER_DATA_NAME = "web_auth_domain";

namespace {

/**
 * "Transaction not signed by server", and, when the server's signature is
 * good for another network the SDK knows, which one: a challenge read with
 * the wrong network fails at the signature, and said nothing more.
 */
std::string notSignedByServer(Transaction* transaction, const QString& serverAccountId)
{
    const QString message = QStringLiteral("Transaction not signed by server");
    const QString given = transaction->getNetwork()->getNetworkPassphrase();
    const struct { const char* name; QString passphrase; } known[] = {
        {"the public network", Network::PUBLIC_NETWORK_PASSPHRASE()},
        {"testnet", Network::TESTNET_NETWORK_PASSPHRASE()},
        {"futurenet", Network::FUTURENET_NETWORK_PASSPHRASE()},
    };
    for (const auto& network : known) {
        if (given == network.passphrase)
            continue;
        Network other{network.passphrase};
        try {
            std::unique_ptr<AbstractTransaction> again(
                AbstractTransaction::fromEnvelopeXdr(transaction->toEnvelopeXdrBase64(), &other));
            auto* signedFor = dynamic_cast<Transaction*>(again.get());
            if (signedFor && Sep10Challenge::verifyTransactionSignature(signedFor, serverAccountId)) {
                return (message + QStringLiteral(": it is signed for %1, not for the network given (\"%2\")")
                                      .arg(QLatin1String(network.name), given)).toStdString();
            }
        } catch (const std::exception&) {
        }
    }
    return message.toStdString();
}

// A G address exactly as it encodes: checksum, 32 bytes, canonical spelling.
bool isAccountIdStrKey(const QString& value)
{
    try {
        if (value.size() != StrKey::ACCOUNT_ID_ADDRESS_LENGTH
                || StrKey::decodeVersionByte(value) != StrKey::VersionByte::ACCOUNT_ID)
            return false;
        const QByteArray raw = StrKey::decodeStellarAccountId(value);
        return raw.size() == 32 && StrKey::encodeStellarAccountId(raw) == value;
    } catch (const std::exception&) {
        return false;
    }
}

// The same for an M address.
bool isMuxedAccountStrKey(const QString& value)
{
    try {
        if (value.size() != StrKey::MUXED_ACCOUNT_ADDRESS_LENGTH
                || StrKey::decodeVersionByte(value) != StrKey::VersionByte::MUXED_ACCOUNT)
            return false;
        return StrKey::encodeStellarMuxedAccount(StrKey::encodeToXDRMuxedAccount(value)) == value;
    } catch (const std::exception&) {
        return false;
    }
}

} // namespace

Transaction* Sep10Challenge::buildChallengeTx(KeyPair *serverSignerSecret, QString clientAccountID, QString domainName, QString webAuthDomain, qint64 timebound, Network* network
                                              , QString clientDomain, QString clientSigningKey, Memo *memo)
{
    // Owned here too, so a refusal before the hand-over does not leak it.
    std::unique_ptr<Memo> ownedMemo(memo);
    // FIX §5.1: timebound<=0 used to build TimeBounds(0,0), which
    // readChallengeTransaction itself rejects ("non-infinite timebounds").
    // SEP-10 forbids expiring-never challenges: fail fast.
    if (timebound <= 0)
        throw std::runtime_error("timebound must be > 0 (SEP-10 requires non-infinite timebounds)");
    qint64 now = QDateTime::currentMSecsSinceEpoch() / 1000L;
    // now + timebound overflowed for a huge timebound (INT64_MAX, or Infinity
    // read as one): undefined behaviour, and a challenge valid for ever.
    if (timebound > std::numeric_limits<qint64>::max() - now)
        throw std::runtime_error("timebound is too large");
    qint64 timeoutTimestamp = now + timebound;
    TimeBounds *timeBounds = new TimeBounds(now, timeoutTimestamp);
    // The overload below owns both from its first line.
    return buildChallengeTx(serverSignerSecret, clientAccountID, domainName, webAuthDomain, timeBounds, network, clientDomain, clientSigningKey, ownedMemo.release());
}


Transaction* Sep10Challenge::buildChallengeTx(KeyPair* serverSignerSecret, QString clientAccountID, QString domainName, QString webAuthDomain, TimeBounds* timeBounds, Network *network
                                         , QString clientDomain, QString clientSigningKey, Memo *memo)
{
    // FIX §1.6: previously, any exception after creating `sa`, `timeBounds`,
    // or the ManageDataOperation instances leaked them. Now they live under
    // std::unique_ptr until release() transfers ownership to the Builder.
    // The time bounds and the memo are owned from the first line, so a refusal
    // below frees them too instead of leaking the caller's objects.
    std::unique_ptr<TimeBounds> ownedTimeBounds(timeBounds);
    std::unique_ptr<Memo> ownedMemo(memo);
    if (!serverSignerSecret)
        throw std::runtime_error("serverSignerSecret cannot be null");
    // These used to be signed as given: TimeBounds(now, 0) made a challenge
    // that never expires and past bounds one that already had, both refused
    // by every reader, this SDK's included. Same limits as the reader's.
    if (!ownedTimeBounds)
        throw std::runtime_error("timeBounds cannot be null");
    const qint64 now = QDateTime::currentMSecsSinceEpoch() / 1000L;
    if (ownedTimeBounds->getMaxTime() == 0)
        throw std::runtime_error("timeBounds must have a maxTime (SEP-10 requires non-infinite timebounds)");
    if (ownedTimeBounds->getMaxTime() < now)
        throw std::runtime_error("timeBounds have already expired");
    if (ownedTimeBounds->getMinTime() > now + GRACE_PERIOD_SECONDS)
        throw std::runtime_error("timeBounds are not valid yet");
    // A blank domain gave the key " auth", which no reader matches; a null
    // webAuthDomain a web_auth_domain operation with no value, which every
    // reader refuses.
    if (domainName.trimmed().isEmpty())
        throw std::runtime_error("domainName is required");
    if (webAuthDomain.isNull())
        throw std::runtime_error("webAuthDomain is required");
    if (clientDomain.isEmpty() != clientSigningKey.isEmpty()) {
        throw std::runtime_error("clientDomain is required if clientSigningKey is provided");
    }
    // A muxed client (M...) used to be refused; js and py build its challenge
    // with the M address as the first operation's source, and so does this.
    const StrKey::VersionByte clientVersion = StrKey::decodeVersionByte(clientAccountID);
    const bool muxedClient = clientVersion == StrKey::VersionByte::MUXED_ACCOUNT;
    if (clientVersion != StrKey::VersionByte::ACCOUNT_ID && !muxedClient)
        throw std::runtime_error("Version byte is invalid");
    // The version byte alone let any G-prefixed text through, to fail only
    // when the challenge was signed. Decoding checks the checksum as well.
    if (muxedClient ? !isMuxedAccountStrKey(clientAccountID)
                    : StrKey::decodeStellarAccountId(clientAccountID).size() != 32)
        throw std::runtime_error("clientAccountID is not a valid account id");
    // The M address already names the sub-account; a memo would name another.
    if (muxedClient && ownedMemo)
        throw std::runtime_error("memo cannot be used if clientAccountID is a muxed account");
    // Validate clientSigningKey early so we don't have to clean up later.
    if (!clientSigningKey.isEmpty() &&
        (StrKey::decodeVersionByte(clientSigningKey) != StrKey::VersionByte::ACCOUNT_ID
         || StrKey::decodeStellarAccountId(clientSigningKey).size() != 32)) {
        throw std::runtime_error("clientSigningKey is not a valid account id");
    }
    // Validate memo (only MemoId allowed) before allocating.
    if (ownedMemo && !dynamic_cast<MemoId*>(ownedMemo.get())) {
        throw std::runtime_error("only memo type `id` is supported");
    }

    // 48 random bytes, 64 characters in STANDARD base64 — what the js, Java
    // and Python SDKs and the Anchor Platform emit. It used to be the URL-safe
    // alphabet, which java-stellar-sdk refuses outright and py-stellar-sdk
    // mis-decodes, so a wallet built on either rejected most challenges from a
    // server built on this SDK. 48 bytes need no padding.
    QByteArray randomNonce = Util::generateRandomNonce(NONCE_SIZE);
    randomNonce = randomNonce.toBase64();

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
        .addTimeBounds(ownedTimeBounds.release())
        .setBaseFee(Transaction::Builder::BASE_FEE);

    if (!clientSigningKey.isEmpty()) {
        // UTF-8, as web_auth_domain and the js, java and py SDKs write it:
        // Latin-1 sent "bücher.example" as an invalid byte and "例.jp" as "?.jp".
        std::unique_ptr<ManageDataOperation> clientDomainOp(
            new ManageDataOperation(CLIENT_DOMAIN_DATA_NAME, clientDomain.toUtf8()));
        clientDomainOp->setSourceAccount(clientSigningKey);
        builder.addOperation(clientDomainOp.release());
    }

    if (ownedMemo) {
        builder.addMemo(ownedMemo.release());
    }

    // Owned until returned: a failure to sign used to leak the whole challenge.
    std::unique_ptr<Transaction> tx(builder.build());
    tx->sign(serverSignerSecret);
    // `sa` released by unique_ptr at scope exit (Builder doesn't own Account).
    return tx.release();
}

Sep10Challenge::ChallengeTransaction * Sep10Challenge::readChallengeTransaction(QString challengeXdr, QString serverAccountId, QStringList domainNames, QString webAuthDomain, Network *network)  {
    return readChallengeTransaction(challengeXdr, serverAccountId, domainNames, webAuthDomain, QString(), QString(), network);
}

Sep10Challenge::ChallengeTransaction * Sep10Challenge::readChallengeTransaction(QString challengeXdr, QString serverAccountId, QStringList domainNames, QString webAuthDomain,
                                                                                QString expectedClientDomain, QString expectedClientDomainSigningKey, Network *network)  {
    // A client that asked for a client domain knows both halves: the domain,
    // and the SIGNING_KEY its stellar.toml publishes. Either alone cannot
    // bind the client_domain operation to anything.
    const bool clientDomainExpected = !expectedClientDomain.isEmpty() || !expectedClientDomainSigningKey.isEmpty();
    if (clientDomainExpected) {
        if (expectedClientDomain.isEmpty())
            throw std::runtime_error("expectedClientDomain is required if expectedClientDomainSigningKey is provided");
        if (expectedClientDomainSigningKey.isEmpty())
            throw std::runtime_error("expectedClientDomainSigningKey is required if expectedClientDomain is provided");
        if (!isAccountIdStrKey(expectedClientDomainSigningKey))
            throw std::runtime_error("expectedClientDomainSigningKey is not a valid account id");
    }

    // decode the received input: a base64 XDR transaction envelope
    //
    // Owned here until it is handed to the ChallengeTransaction: every refusal
    // below used to leak the whole parsed transaction, ~1.5 KB a call, on
    // input that is untrusted by definition — a client's POST on the server
    // side, a server's challenge on the client side.
    std::unique_ptr<AbstractTransaction> parsed(Transaction::fromEnvelopeXdr(challengeXdr, network));
    Transaction* transaction = dynamic_cast<Transaction*>(parsed.get());
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
    // Names are compared as the bytes that are signed, not as decoded text:
    // decoding dropped a leading BOM, so "\xEF\xBB\xBFexample.com auth" matched.
    const QByteArray firstName = manageDataOperation->getNameBytes();
    for (const QString& homeDomain : domainNames) {
        // An empty domain would match the key " auth", which no home domain
        // produces; java refuses an empty domain list, and so does this.
        if (homeDomain.trimmed().isEmpty())
            continue;
        if ((homeDomain + " " + HOME_DOMAIN_MANAGER_DATA_NAME_FLAG).toUtf8() == firstName) {
            matchedDomainName = homeDomain;
            break;
        }
    }
    if (matchedDomainName.isNull()){
        throw std::runtime_error("The transaction's operation key name does not include one of the expected home domains.");
    }

    // A muxed client (M...) is read as js and py read it, and returned as the
    // M address; it used to be refused.
    const StrKey::VersionByte clientVersion = StrKey::decodeVersionByte(clientAccountId);
    const bool muxedClient = clientVersion == StrKey::VersionByte::MUXED_ACCOUNT;
    if (clientVersion != StrKey::VersionByte::ACCOUNT_ID && !muxedClient) {
        throw std::runtime_error("clientAccountId is not a valid account id");
    }
    // The M address already names the sub-account; a memo would name another.
    if (muxedClient && memo && !dynamic_cast<MemoNone*>(memo)) {
        throw std::runtime_error("The transaction has a memo but the client account ID is a muxed account");
    }

    // verify manage data value
    const QByteArray encodedNonce = manageDataOperation->getValue();
    if (encodedNonce.length() != 64) {
        throw std::runtime_error("Random nonce encoded as base64 should be 64 bytes long.");
    }

    // Either base64 alphabet, each decoded strictly. The URL-safe decoder
    // alone silently SKIPS '+' and '/', so a standard-base64 nonce — what the
    // js, Java and Python SDKs and the Anchor Platform all send — came out
    // short and was refused: 87% of real challenges, since a random 48-byte
    // nonce almost always contains one of the two.
    QByteArray nonce;
    const auto standard = QByteArray::fromBase64Encoding(
        encodedNonce, QByteArray::Base64Encoding | QByteArray::AbortOnBase64DecodingErrors);
    if (standard) {
        nonce = *standard;
    } else {
        const auto urlSafe = QByteArray::fromBase64Encoding(
            encodedNonce, QByteArray::Base64UrlEncoding | QByteArray::AbortOnBase64DecodingErrors);
        if (urlSafe)
            nonce = *urlSafe;
    }
    if(nonce.isEmpty())
    {
        throw std::runtime_error("Failed to decode random nonce provided in ManageData operation.");
    }

    if (nonce.length() != 48) {
        throw std::runtime_error("Random nonce before encoding as base64 should be 48 bytes long.");
    }


    QByteArray webAuthDomaidUtf8 = webAuthDomain.toUtf8();
    const QByteArray clientDomainName = CLIENT_DOMAIN_DATA_NAME.toUtf8();
    const QByteArray webAuthDomainName = WEB_AUTH_DOMAIN_MANAGER_DATA_NAME.toUtf8();
    // The client_domain operation, if there is one: its value and its source.
    QByteArray clientDomainValue;
    QString clientDomainSource;
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
        const QByteArray subsequentName = subsequentManageDataOperation->getNameBytes();
        if ((subsequentName != clientDomainName) && subsequentClientAccountId != serverAccountId)
            throw std::runtime_error("subsequent operations are unrecognized");

        if (subsequentName == clientDomainName) {
            // Only the verifier refused a second one, so a client read the
            // challenge, was shown one client domain and signed for both.
            if (!clientDomainSource.isEmpty())
                throw std::runtime_error("Found more than one client_domain operation");
            // The key that signs for a domain is its toml's SIGNING_KEY, a G
            // address; any other source no verifier can check a signature of.
            if (!isAccountIdStrKey(subsequentClientAccountId))
                throw std::runtime_error("The client_domain operation's source must be an account id (G...)");
            clientDomainSource = subsequentClientAccountId;
            clientDomainValue = subsequentManageDataOperation->getValue();
        }

        if (webAuthDomainName == subsequentName) {
           if (subsequentManageDataOperation->getValue().isNull()) {
             throw std::runtime_error("'web_auth_domain' operation value should not be null.");
           }
           if (webAuthDomaidUtf8 !=  subsequentManageDataOperation->getValue()) {
             throw std::runtime_error("'web_auth_domain' operation value does not match.");
           }
         }
    }

    // Bound to what the client asked for: the operation used to be accepted
    // from any key and naming any domain, and the client signed it, vouching
    // for a domain it never asked for or with a key that is not the domain's.
    if (clientDomainExpected) {
        if (clientDomainSource.isEmpty())
            throw std::runtime_error(QString("The transaction has no client_domain operation, but %1 was expected")
                                         .arg(expectedClientDomain).toStdString());
        if (clientDomainValue != expectedClientDomain.toUtf8())
            throw std::runtime_error(QString("'client_domain' operation value does not match %1")
                                         .arg(expectedClientDomain).toStdString());
        if (clientDomainSource != expectedClientDomainSigningKey)
            throw std::runtime_error(QString("'client_domain' operation source is not %1, the signing key of %2")
                                         .arg(expectedClientDomainSigningKey, expectedClientDomain).toStdString());
    }

    if (!verifyTransactionSignature(transaction, serverAccountId)) {
        throw std::runtime_error(notSignedByServer(transaction, serverAccountId));
    }

    auto* result = new ChallengeTransaction(transaction, clientAccountId, matchedDomainName,
                                            clientDomainSource.isEmpty() ? QString() : QString::fromUtf8(clientDomainValue),
                                            clientDomainSource);
    parsed.release();   // the same object; the ChallengeTransaction owns it now
    return result;
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
        // A G... that is not an account id is refused, as py refuses it (js
        // too, when it reaches it): only the first byte was read, so a typo in
        // the second character (an unknown version byte) was skipped without a
        // word, leaving the signer meant out of the check. A bad checksum or
        // length was refused further on; it is refused here, saying so.
        if (signer.startsWith(QLatin1Char('G')) && !isAccountIdStrKey(signer)) {
            throw std::runtime_error(QString("a signer starts with G but is not a valid account id: %1")
                                         .arg(signer).toStdString());
        }

        // Ignore non-G... account/address signers.
        StrKey::VersionByte versionByte;
        try {
            versionByte = StrKey::decodeVersionByte(signer);
        } catch (const std::runtime_error& e) {
            Q_UNUSED(e)
            continue;
        }

        // A secret seed is refused, not skipped as js skips it (py refuses it
        // too, with a message that repeats the seed): it is the caller's
        // mistake (a seed where its G... was meant), and skipping it left the
        // account's own signer out of the check without a word. The message
        // does not repeat the seed.
        if (StrKey::VersionByte::SEED == versionByte) {
            throw std::runtime_error("a signer is a secret seed (S...): pass its account id (G...), "
                                     "and keep the seed out of verification code");
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
    const QByteArray clientDomainName = CLIENT_DOMAIN_DATA_NAME.toUtf8();

    for (Operation* op : transaction->getOperations()) {
        if (!(dynamic_cast<ManageDataOperation*>(op))) {
            throw std::runtime_error("Operation type should be ManageData.");
        }
        ManageDataOperation* manageDataOp = (ManageDataOperation*) op;
        if (manageDataOp->getSourceAccount().isNull()) {
            throw std::runtime_error("Operation should have a source account.");
        }
        if (manageDataOp->getNameBytes() == clientDomainName) {
            // Only the first one used to be checked, so a second client_domain
            // key that never signed went through. js refuses it too.
            if (!clientDomainSigner.isNull())
                throw std::runtime_error("Found more than one client_domain operation");
            allSigners.insert(manageDataOp->getSourceAccount());
            clientDomainSigner = manageDataOp->getSourceAccount();
        }
    }


    QSet<QString> signersFound = verifyTransactionSignatures(transaction, allSigners);

    // Confirm the server is in the list of signers found and remove it.
    bool serverSignerFound = signersFound.remove(serverKeyPair->getAccountId());

    // Confirm we matched a signature to the server signer.
    if (!serverSignerFound) {
        throw std::runtime_error("Transaction not signed by server.");
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

    // Confirm we matched signatures to the client signers. Only now, with the
    // client_domain signer gone (py's order): checked before, its signature
    // counted as the client's, and a challenge the client never signed came
    // back as verified with no signers — which threshold 0 then accepted.
    if (signersFound.isEmpty()) {
        throw std::runtime_error("Transaction not signed by any client signer.");
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

    // Counted as stellar-core counts them (SignatureChecker, from protocol
    // 10): a weight past a byte weighs 255. Such a weight is a signer's added
    // before protocol 10, and Horizon sends it as an int32, negative from
    // 2^31 on. Summed as they came, a negative weight went in as 2^32 minus
    // it and several heavy ones overflowed the int: signers that meet the
    // threshold were refused.
    qint64 sum = 0;
    for (const QString& signer : signersFound) {
        const int weight = weightsForSigner.value(signer);
        sum += (weight < 0 || weight > 255) ? 255 : weight;
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

Sep10Challenge::ChallengeTransaction::ChallengeTransaction(Transaction *transaction, QString clientAccountId, QString matchedHomeDomain,
                                                           QString clientDomain, QString clientDomainSigningKey)
    :m_transaction(transaction),m_clientAccountId(clientAccountId),m_matchedHomeDomain(matchedHomeDomain)
    ,m_clientDomain(clientDomain),m_clientDomainSigningKey(clientDomainSigningKey)
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

QString Sep10Challenge::ChallengeTransaction::getClientDomain() const
{
    return m_clientDomain;
}

QString Sep10Challenge::ChallengeTransaction::getClientDomainSigningKey() const
{
    return m_clientDomainSigningKey;
}

int Sep10Challenge::ChallengeTransaction::hashCode() const
{
    return static_cast<int>(::qHash(m_transaction->hash()) ^ ::qHash(m_clientAccountId) ^ ::qHash(m_matchedHomeDomain));
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
    // qHash returns size_t on Qt 6; fold it into the int hashCode explicitly.
    return static_cast<int>(::qHash(m_key) ^ ::qHash(m_weight));
}

bool Sep10Challenge::Signer::equals(const Sep10Challenge::Signer *other) const {
    return m_key==other->m_key && m_weight == other->m_weight;
}

bool Sep10Challenge::Signer::operator==(const Sep10Challenge::Signer &other) const
{
    return m_key==other.m_key && m_weight == other.m_weight;
}
Sep10Challenge::WebAuth Sep10Challenge::webAuthFromToml(const QVariantMap& toml)
{
    const QVariant endpointValue = toml.value(QStringLiteral("WEB_AUTH_ENDPOINT"));
    const QUrl endpoint(endpointValue.toString(), QUrl::StrictMode);
    // https only: the challenge is signed and the token returned over it.
    if (endpointValue.userType() != QMetaType::QString || !endpoint.isValid() || endpoint.host().isEmpty()
        || endpoint.scheme().compare(QLatin1String("https"), Qt::CaseInsensitive) != 0) {
        throw std::runtime_error("SEP-10: the stellar.toml has no WEB_AUTH_ENDPOINT (an https URL)");
    }
    const QVariant keyValue = toml.value(QStringLiteral("SIGNING_KEY"));
    const QString key = keyValue.toString();
    bool account = false;
    if (keyValue.userType() == QMetaType::QString) {
        try {
            account = StrKey::decodeVersionByte(key) == StrKey::VersionByte::ACCOUNT_ID
                      && StrKey::encodeStellarAccountId(StrKey::decodeStellarAccountId(key)) == key;
        } catch (const std::exception&) {
        }
    }
    if (!account)
        throw std::runtime_error("SEP-10: the stellar.toml has no SIGNING_KEY (an account id)");
    return WebAuth{endpointValue.toString(), key};
}

QSTELLAR_END_NS
