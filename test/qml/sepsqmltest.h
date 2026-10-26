#ifndef SEPSQMLTEST_H
#define SEPSQMLTEST_H

#include <QObject>
#include <QtTest>
#include <QDateTime>
#include <QScopedPointer>
#include "../testcollector.h"

#include "../../src/qml/sepsqml.h"
#include "../../src/sep10challenge.h"
#include "../../src/keypair.h"
#include "../../src/network.h"
#include "../../src/account.h"
#include "../../src/transaction.h"
#include "../../src/transactionbuilder.h"
#include "../../src/paymentoperation.h"
#include "../../src/managedataoperation.h"
#include "../../src/memo.h"
#include "../../src/assettypenative.h"
#include "../../src/timebounds.h"
#include "../../src/strkey.h"
#include "../../src/sep7.h"
#include "../../src/muxedaccount.h"
#include "../../xdr/sorobanops.h"
#include <QDataStream>
#include <QRegularExpression>
#include <algorithm>

/**
 * Seps.signChallenge signs only what is provably a SEP-10 challenge.
 *
 * It used to sign whatever envelope it was given. An app that logs in with
 * "fetch challenge, signChallenge, post it back" would then hand a malicious
 * or compromised auth server a signed payment from the user's account at its
 * next sequence number — the one thing a login must never produce. The other
 * SDKs validate before they sign (js-stellar-sdk's readChallengeTx, the
 * Flutter, iOS and Kotlin jwtToken), and so does this now.
 */
class SepsQmlTest: public QObject
{
    Q_OBJECT

    static QString HOME()     { return QStringLiteral("example.com"); }
    static QString WEB_AUTH() { return QStringLiteral("auth.example.com"); }

    /** A challenge built by the SDK's server side, signed by `server`. */
    static QString challengeFor(KeyPair* server, const QString& client,
                                TimeBounds* bounds = nullptr)
    {
        QScopedPointer<Transaction> tx(bounds
            ? Sep10Challenge::buildChallengeTx(server, client, HOME(), WEB_AUTH(), bounds,
                                               Network::testnetNetwork())
            : Sep10Challenge::buildChallengeTx(server, client, HOME(), WEB_AUTH(), 300,
                                               Network::testnetNetwork()));
        return tx->toEnvelopeXdrBase64();
    }

    static QVariantMap argsFor(const QString& xdr, KeyPair* server, KeyPair* signer)
    {
        QVariantMap args;
        args.insert(QStringLiteral("xdr"), xdr);
        args.insert(QStringLiteral("secretSeed"), signer->getSecretSeed());
        args.insert(QStringLiteral("serverAccountId"), server->getAccountId());
        args.insert(QStringLiteral("homeDomain"), HOME());
        args.insert(QStringLiteral("webAuthDomain"), WEB_AUTH());
        args.insert(QStringLiteral("network"), QStringLiteral("testnet"));
        return args;
    }

    static bool refused(const QVariantMap& out)
    {
        return out.contains(QStringLiteral("error")) && !out.contains(QStringLiteral("xdr"));
    }

    /**
     * A challenge assembled by hand, the way a server on another SDK would:
     * server-sourced at `sequence`, the client's "<home> auth" nonce, the
     * server's web_auth_domain, optionally one more operation, signed by the
     * server.
     */
    static QString handMade(KeyPair* server, const QString& client, const QByteArray& encodedNonce,
                            qint64 sequence = 0, Operation* extra = nullptr,
                            TimeBounds* bounds = nullptr)
    {
        const qint64 now = QDateTime::currentMSecsSinceEpoch() / 1000;
        Account account(KeyPair::fromAccountId(server->getAccountId()), sequence - 1);
        ManageDataOperation* auth = ManageDataOperation::create(HOME() + QStringLiteral(" auth"),
                                                                encodedNonce);
        auth->setSourceAccount(client);
        ManageDataOperation* web = ManageDataOperation::create(QStringLiteral("web_auth_domain"),
                                                               WEB_AUTH().toUtf8());
        web->setSourceAccount(server->getAccountId());
        TransactionBuilder builder(&account, Network::testnetNetwork());
        builder.addOperation(auth).addOperation(web);
        if (extra)
            builder.addOperation(extra);
        builder.addTimeBounds(bounds ? bounds : new TimeBounds(now, now + 300)).setBaseFee(100);
        QScopedPointer<Transaction> tx(builder.build());
        tx->sign(server);
        return tx->toEnvelopeXdrBase64();
    }

    /** 48 bytes whose standard base64 is "++++" x8 then "////" x8. */
    static QByteArray nonceWithPlusAndSlash()
    {
        QByteArray raw;
        for (int i = 0; i < 8; ++i) raw.append("\xFB\xEF\xBE", 3);
        for (int i = 0; i < 8; ++i) raw.append("\xFF\xFF\xFF", 3);
        return raw;
    }

private slots:

    /**
     * parseToml hands JavaScript an integer it cannot hold exactly (past
     * ±2^53) as its decimal text: as a number it was rounded. Inside tables
     * and arrays too; 2^53 itself, and smaller ones, stay numbers.
     */
    void testParseTomlKeepsBigIntegersExact()
    {
        SepsQml seps;
        const QVariantMap values = seps.parseToml(QStringLiteral(
            "A = 9007199254740993\n"
            "B = 9007199254740992\n"
            "C = [1, 9223372036854775807]\n"
            "[t]\n"
            "D = -9007199254740993\n"
            "E = { f = 12345678901234567 }\n")).value(QStringLiteral("values")).toMap();
        QCOMPARE(values.value(QStringLiteral("A")).userType(), int(QMetaType::QString));
        QCOMPARE(values.value(QStringLiteral("A")).toString(), QStringLiteral("9007199254740993"));
        QCOMPARE(values.value(QStringLiteral("B")).userType(), int(QMetaType::LongLong));
        const QVariantList c = values.value(QStringLiteral("C")).toList();
        QCOMPARE(c.at(0).userType(), int(QMetaType::LongLong));
        QCOMPARE(c.at(1).toString(), QStringLiteral("9223372036854775807"));
        const QVariantMap t = values.value(QStringLiteral("t")).toMap();
        QCOMPARE(t.value(QStringLiteral("D")).toString(), QStringLiteral("-9007199254740993"));
        QCOMPARE(t.value(QStringLiteral("E")).toMap().value(QStringLiteral("f")).toString(),
                 QStringLiteral("12345678901234567"));
    }

    /**
     * tomlSigningKeys checks the two keys it hands on: an account id each,
     * or { error } naming the one that is not (a secret seed, a damaged key,
     * a number). It handed on whatever text the toml held.
     */
    void testTomlSigningKeysAreAccountIds()
    {
        SepsQml seps;
        const QString good = QStringLiteral("GBVOL67TMUQBGL4TZYNMY3ZQ5WGQYFPFD5VJRWXR72VA33VFNL225PL5");
        QCOMPARE(seps.tomlSigningKeys(QStringLiteral("SIGNING_KEY = \"%1\"\n").arg(good))
                     .value(QStringLiteral("signingKey")).toString(), good);
        QVERIFY(seps.tomlSigningKeys(QStringLiteral("VERSION = \"2.0.0\"\n")).isEmpty());

        QString damaged = good;
        damaged[20] = damaged[20] == QLatin1Char('A') ? QLatin1Char('B') : QLatin1Char('A');
        for (const QString& line : {QStringLiteral("SIGNING_KEY = \"%1\"").arg(damaged),
                                    QStringLiteral("SIGNING_KEY = \"SDJHRQF4GCMIIKAAAQ6IHY42X73FQFLHUULAPSKKD4DFDM7UXWWCRHBE\""),
                                    QStringLiteral("SIGNING_KEY = 7"),
                                    QStringLiteral("URI_REQUEST_SIGNING_KEY = \"%1\"").arg(good.toLower())}) {
            const QVariantMap out = seps.tomlSigningKeys(line + QLatin1Char('\n'));
            const QString error = out.value(QStringLiteral("error")).toString();
            QVERIFY2(error.contains(line.section(QLatin1Char(' '), 0, 0)), qPrintable(line + QStringLiteral(" -> ") + error));
            QVERIFY(!error.contains(QStringLiteral("SDJHRQF4")));
        }
    }

    /**
     * tomlSigningKeys fails whole, as its header says: one damaged key, an
     * empty URI_REQUEST_SIGNING_KEY included, is { error } and nothing else,
     * not even a good SIGNING_KEY. The header still said an empty string
     * came back as a value distinct from an absent key.
     */
    void testTomlSigningKeysFailsWholeOnOneBadKey()
    {
        SepsQml seps;
        const QString good = QStringLiteral("GBVOL67TMUQBGL4TZYNMY3ZQ5WGQYFPFD5VJRWXR72VA33VFNL225PL5");
        const QString toml = QStringLiteral("SIGNING_KEY = \"%1\"\n"
                                            "WEB_AUTH_ENDPOINT = \"https://auth.example\"\n"
                                            "URI_REQUEST_SIGNING_KEY = \"\"\n").arg(good);
        const QVariantMap out = seps.tomlSigningKeys(toml);
        QCOMPARE(out.size(), 1);
        QVERIFY(out.value(QStringLiteral("error")).toString().contains(QStringLiteral("URI_REQUEST_SIGNING_KEY")));
        // Not published is absent, not empty.
        const QVariantMap alone = seps.tomlSigningKeys(QStringLiteral("SIGNING_KEY = \"%1\"\n").arg(good));
        QCOMPARE(alone.value(QStringLiteral("signingKey")).toString(), good);
        QVERIFY(!alone.contains(QStringLiteral("uriRequestSigningKey")));
    }

    void initTestCase() {}
    void cleanupTestCase() {}

    /**
     * verifyUriAgainstToml is Sep7::verifyOrigin against the pasted toml:
     * the key is an account id spelled exactly (a padded one was trimmed and
     * passed, which tomlSigningKeys refuses; a number got a base32 error),
     * the URI names an origin_domain that is a fully qualified domain name
     * (none passed), and the URI is signed by that key.
     */
    void testVerifyUriAgainstTomlIsTheOriginCheck()
    {
        SepsQml seps;
        QScopedPointer<KeyPair> origin(KeyPair::random());
        const QString key = origin->getAccountId();
        const QString toml = QStringLiteral("URI_REQUEST_SIGNING_KEY = \"%1\"\n").arg(key);
        const QString destination = QStringLiteral("GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7");
        Sep7Pay pay(destination);
        pay.originDomain = QStringLiteral("example.com");
        pay.sign(origin.data());
        const QVariantMap out = seps.verifyUriAgainstToml(pay.toUri(), toml);
        QVERIFY2(out.value(QStringLiteral("valid")).toBool(), qPrintable(out.value(QStringLiteral("error")).toString()));
        QCOMPARE(out.value(QStringLiteral("keyUsed")).toString(), key);
        QCOMPARE(out.value(QStringLiteral("originDomain")).toString(), QStringLiteral("example.com"));
        QCOMPARE(out.value(QStringLiteral("fetched")).toBool(), false);
        // An internationalised origin in its ACE form (example.com with a
        // Cyrillic a looked like example.com).
        Sep7Pay homograph(destination);
        homograph.originDomain = QStringLiteral("ex") + QChar(ushort(0x0430)) + QStringLiteral("mple.com");
        homograph.sign(origin.data());
        QCOMPARE(seps.verifyUriAgainstToml(homograph.toUri(), toml).value(QStringLiteral("originDomain")).toString(),
                 QStringLiteral("xn--exmple-4nf.com"));

        auto error = [&](const QString& uri, const QString& text) {
            return seps.verifyUriAgainstToml(uri, text).value(QStringLiteral("error")).toString();
        };
        QVERIFY(error(pay.toUri(), QStringLiteral("URI_REQUEST_SIGNING_KEY = \" %1 \"\n").arg(key))
                    .contains(QStringLiteral("URI_REQUEST_SIGNING_KEY")));
        QVERIFY(error(pay.toUri(), QStringLiteral("URI_REQUEST_SIGNING_KEY = 7\n"))
                    .contains(QStringLiteral("URI_REQUEST_SIGNING_KEY")));

        Sep7Pay noOrigin(destination);
        noOrigin.sign(origin.data());
        QVERIFY(error(noOrigin.toUri(), toml).contains(QStringLiteral("no origin_domain")));
        Sep7Pay local(destination);
        local.originDomain = QStringLiteral("localhost");
        local.sign(origin.data());
        QVERIFY(error(local.toUri(), toml).contains(QStringLiteral("fully qualified")));
        QScopedPointer<KeyPair> other(KeyPair::random());
        pay.sign(other.data());
        QVERIFY(error(pay.toUri(), toml).contains(QStringLiteral("not signed by")));

        const QVariantMap none = seps.verifyUriAgainstToml(pay.toUri(), QStringLiteral("VERSION = \"2.0.0\"\n"));
        QCOMPARE(none.value(QStringLiteral("hasUriRequestSigningKey")).toBool(), false);
        QVERIFY(none.contains(QStringLiteral("error")));
    }

    void testSignsAValidChallenge()
    {
        QScopedPointer<KeyPair> server(KeyPair::random());
        QScopedPointer<KeyPair> client(KeyPair::random());
        SepsQml seps;

        const QVariantMap out = seps.signChallenge(
            argsFor(challengeFor(server.data(), client->getAccountId()),
                    server.data(), client.data()));
        QVERIFY2(!out.contains(QStringLiteral("error")),
                 qPrintable(out.value(QStringLiteral("error")).toString()));
        QCOMPARE(out.value(QStringLiteral("signatureCount")).toInt(), 2);
        QCOMPARE(out.value(QStringLiteral("signedBy")).toString(), client->getAccountId());
        QCOMPARE(out.value(QStringLiteral("clientAccountId")).toString(), client->getAccountId());
        QCOMPARE(out.value(QStringLiteral("matchedHomeDomain")).toString(), HOME());

        // And the server side accepts what came back.
        const QSet<QString> signers = Sep10Challenge::verifyChallengeTransactionSigners(
            out.value(QStringLiteral("xdr")).toString(), server->getAccountId(), HOME(),
            WEB_AUTH(), QSet<QString>{client->getAccountId()}, Network::testnetNetwork());
        QCOMPARE(signers, QSet<QString>{client->getAccountId()});
    }

    /** The attack: a live payment presented as a challenge. */
    void testRefusesATransactionThatIsNotAChallenge()
    {
        QScopedPointer<KeyPair> server(KeyPair::random());
        QScopedPointer<KeyPair> client(KeyPair::random());
        SepsQml seps;

        Account account(KeyPair::fromAccountId(client->getAccountId()), 41);
        QScopedPointer<Transaction> payment(
            TransactionBuilder(&account, Network::testnetNetwork())
                .addOperation(new PaymentOperation(server->getAccountId(),
                                                   new AssetTypeNative(), "1000"))
                .setTimeout(300)
                .setBaseFee(100)
                .build());
        payment->sign(server.data());   // even carrying the "server's" signature

        QVERIFY(refused(seps.signChallenge(
            argsFor(payment->toEnvelopeXdrBase64(), server.data(), client.data()))));
    }

    /** No expectations, no signature: there is nothing to check against. */
    void testRefusesWithoutTheExpectations()
    {
        QScopedPointer<KeyPair> server(KeyPair::random());
        QScopedPointer<KeyPair> client(KeyPair::random());
        SepsQml seps;
        const QVariantMap args = argsFor(challengeFor(server.data(), client->getAccountId()),
                                         server.data(), client.data());

        for (const char* key : {"serverAccountId", "homeDomain", "webAuthDomain",
                                "secretSeed", "xdr"}) {
            QVariantMap missing = args;
            missing.remove(QString::fromLatin1(key));
            QVERIFY2(refused(seps.signChallenge(missing)), key);
        }
    }

    void testRefusesTheWrongServerDomainOrNetwork()
    {
        QScopedPointer<KeyPair> server(KeyPair::random());
        QScopedPointer<KeyPair> client(KeyPair::random());
        QScopedPointer<KeyPair> impostor(KeyPair::random());
        SepsQml seps;
        const QVariantMap args = argsFor(challengeFor(server.data(), client->getAccountId()),
                                         server.data(), client.data());

        QVariantMap wrong = args;
        wrong.insert(QStringLiteral("serverAccountId"), impostor->getAccountId());
        QVERIFY(refused(seps.signChallenge(wrong)));

        wrong = args;
        wrong.insert(QStringLiteral("homeDomain"), QStringLiteral("evil.example"));
        QVERIFY(refused(seps.signChallenge(wrong)));

        wrong = args;
        wrong.insert(QStringLiteral("webAuthDomain"), QStringLiteral("evil.example"));
        QVERIFY(refused(seps.signChallenge(wrong)));

        // The passphrase is hashed into the server's signature.
        wrong = args;
        wrong.insert(QStringLiteral("network"), QStringLiteral("public"));
        QVERIFY(refused(seps.signChallenge(wrong)));
    }

    void testRefusesAnExpiredChallenge()
    {
        QScopedPointer<KeyPair> server(KeyPair::random());
        QScopedPointer<KeyPair> client(KeyPair::random());
        SepsQml seps;
        const qint64 now = QDateTime::currentMSecsSinceEpoch() / 1000;

        // Built by hand: this SDK's own builder now refuses expired bounds.
        QVERIFY(refused(seps.signChallenge(argsFor(
            handMade(server.data(), client->getAccountId(), nonceWithPlusAndSlash().toBase64(),
                     0, nullptr, new TimeBounds(now - 900, now - 600)),
            server.data(), client.data()))));
    }

    /**
     * A valid challenge for another account is still not ours to sign, unless
     * the caller says it signs FOR that account — a signer of a multisig one.
     */
    void testSignsOnlyForTheAccountItIsTold()
    {
        QScopedPointer<KeyPair> server(KeyPair::random());
        QScopedPointer<KeyPair> account(KeyPair::random());
        QScopedPointer<KeyPair> cosigner(KeyPair::random());
        SepsQml seps;
        QVariantMap args = argsFor(challengeFor(server.data(), account->getAccountId()),
                                   server.data(), cosigner.data());

        QVERIFY(refused(seps.signChallenge(args)));

        args.insert(QStringLiteral("clientAccountId"), account->getAccountId());
        const QVariantMap out = seps.signChallenge(args);
        QVERIFY2(!out.contains(QStringLiteral("error")),
                 qPrintable(out.value(QStringLiteral("error")).toString()));
        QCOMPARE(out.value(QStringLiteral("signedBy")).toString(), cosigner->getAccountId());
        QCOMPARE(out.value(QStringLiteral("clientAccountId")).toString(), account->getAccountId());
    }

    /**
     * The js, Java and Python SDKs and the Anchor Platform write the nonce in
     * STANDARD base64. Decoding it as URL-safe only silently skipped '+' and
     * '/', the length check failed, and 87% of real challenges were refused.
     * Either alphabet is accepted, strictly; a mixture is not base64.
     */
    void testAcceptsEitherBase64AlphabetForTheNonce()
    {
        QScopedPointer<KeyPair> server(KeyPair::random());
        QScopedPointer<KeyPair> client(KeyPair::random());
        SepsQml seps;

        const QByteArray standard = nonceWithPlusAndSlash().toBase64();
        QVERIFY(standard.contains('+') && standard.contains('/'));
        const QByteArray urlSafe = nonceWithPlusAndSlash().toBase64(
            QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
        const QByteArray mixed = standard.left(32) + urlSafe.mid(32);

        for (const QByteArray& nonce : {standard, urlSafe}) {
            const QVariantMap out = seps.signChallenge(argsFor(
                handMade(server.data(), client->getAccountId(), nonce), server.data(), client.data()));
            QVERIFY2(!out.contains(QStringLiteral("error")),
                     qPrintable(QString::fromLatin1(nonce) + QLatin1Char(' ')
                                + out.value(QStringLiteral("error")).toString()));
        }
        QVERIFY(refused(seps.signChallenge(argsFor(
            handMade(server.data(), client->getAccountId(), mixed), server.data(), client.data()))));

        // What this SDK's own server side now emits reads back too.
        QVERIFY(!seps.signChallenge(argsFor(challengeFor(server.data(), client->getAccountId()),
                                            server.data(), client.data()))
                     .contains(QStringLiteral("error")));
    }

    /**
     * The attack in its real shape: sourced at the server, at a live sequence
     * number, looking like a challenge, with the client's payment inside. And
     * the variant at sequence 0 that smuggles a payment in after the nonce.
     */
    void testRefusesALiveTransactionDressedAsAChallenge()
    {
        QScopedPointer<KeyPair> server(KeyPair::random());
        QScopedPointer<KeyPair> client(KeyPair::random());
        SepsQml seps;
        const QByteArray nonce = nonceWithPlusAndSlash().toBase64();

        auto payment = [&]() {
            auto* pay = new PaymentOperation(server->getAccountId(), new AssetTypeNative(), "1000");
            pay->setSourceAccount(client->getAccountId());
            return pay;
        };

        QVERIFY(refused(seps.signChallenge(argsFor(
            handMade(server.data(), client->getAccountId(), nonce, 5000000001LL, payment()),
            server.data(), client.data()))));
        QVERIFY(refused(seps.signChallenge(argsFor(
            handMade(server.data(), client->getAccountId(), nonce, 0, payment()),
            server.data(), client.data()))));
    }

    /** A single domain given as a string is that domain — not its letters. */
    void testHomeDomainsGivenAsAString()
    {
        QScopedPointer<KeyPair> server(KeyPair::random());
        QScopedPointer<KeyPair> client(KeyPair::random());
        SepsQml seps;

        QVariantMap args = argsFor(challengeFor(server.data(), client->getAccountId()),
                                   server.data(), client.data());
        args.remove(QStringLiteral("homeDomain"));
        args.insert(QStringLiteral("homeDomains"), HOME());
        QVERIFY(!seps.signChallenge(args).contains(QStringLiteral("error")));

        args.insert(QStringLiteral("homeDomains"), QStringLiteral("e"));
        QVERIFY(refused(seps.signChallenge(args)));
    }

    /**
     * What this SDK's server side emits is STANDARD base64 — the alphabet
     * java-stellar-sdk requires and py-stellar-sdk expects. A regression to
     * URL-safe would still read back here, so the alphabet is checked itself.
     */
    void testChallengesAreBuiltWithStandardBase64Nonces()
    {
        QScopedPointer<KeyPair> server(KeyPair::random());
        QScopedPointer<KeyPair> client(KeyPair::random());
        for (int i = 0; i < 20; ++i) {
            QScopedPointer<Transaction> tx(Sep10Challenge::buildChallengeTx(
                server.data(), client->getAccountId(), HOME(), WEB_AUTH(), 300,
                Network::testnetNetwork()));
            auto* auth = dynamic_cast<ManageDataOperation*>(tx->getOperations().at(0));
            QVERIFY(auth != nullptr);
            const QByteArray nonce = auth->getValue();
            QCOMPARE(nonce.size(), 64);
            QVERIFY(!nonce.contains('-') && !nonce.contains('_') && !nonce.contains('='));
            const auto decoded = QByteArray::fromBase64Encoding(
                nonce, QByteArray::Base64Encoding | QByteArray::AbortOnBase64DecodingErrors);
            QVERIFY(decoded.decodingStatus == QByteArray::Base64DecodingStatus::Ok);
            QCOMPARE(decoded.decoded.size(), 48);
        }
    }

    /** The memo names the custodial sub-account the token is for, so both
     *  steps report it. */
    void testTheMemoIsReported()
    {
        QScopedPointer<KeyPair> server(KeyPair::random());
        QScopedPointer<KeyPair> client(KeyPair::random());
        SepsQml seps;
        QScopedPointer<Transaction> tx(Sep10Challenge::buildChallengeTx(
            server.data(), client->getAccountId(), HOME(), WEB_AUTH(), 300,
            Network::testnetNetwork(), QString(), QString(), Memo::id(18446744073709551615ULL)));
        const QVariantMap args = argsFor(tx->toEnvelopeXdrBase64(), server.data(), client.data());

        QCOMPARE(seps.readChallenge(args).value(QStringLiteral("memoId")).toString(),
                 QStringLiteral("18446744073709551615"));
        QCOMPARE(seps.signChallenge(args).value(QStringLiteral("memoId")).toString(),
                 QStringLiteral("18446744073709551615"));
    }

    /** A second signature by the same key would make the server refuse the
     *  whole challenge; it is refused here instead, with a reason. */
    void testRefusesToSignTheSameChallengeTwice()
    {
        QScopedPointer<KeyPair> server(KeyPair::random());
        QScopedPointer<KeyPair> client(KeyPair::random());
        SepsQml seps;

        const QVariantMap once = seps.signChallenge(
            argsFor(challengeFor(server.data(), client->getAccountId()), server.data(), client.data()));
        QVERIFY(!once.contains(QStringLiteral("error")));
        QVERIFY(refused(seps.signChallenge(
            argsFor(once.value(QStringLiteral("xdr")).toString(), server.data(), client.data()))));
    }

    /**
     * A challenge's lifetime is read exactly and bounded: 1e300 or Infinity
     * became INT64_MAX, and now + INT64_MAX overflowed into a challenge valid
     * for ever; 300.7 became 301, true became 1. An empty webAuthDomain built
     * a challenge every reader refuses; an unknown network name meant testnet.
     */
    void testChallengeParametersAreReadExactly()
    {
        SepsQml seps;
        QScopedPointer<KeyPair> server(KeyPair::random());
        QScopedPointer<KeyPair> client(KeyPair::random());
        QVariantMap args;
        args.insert(QStringLiteral("serverSeed"), server->getSecretSeed());
        args.insert(QStringLiteral("clientAccountId"), client->getAccountId());
        args.insert(QStringLiteral("homeDomain"), HOME());
        args.insert(QStringLiteral("webAuthDomain"), WEB_AUTH());

        const QVariantMap ok = seps.buildChallenge(args);
        QVERIFY2(!ok.contains(QStringLiteral("error")), qPrintable(ok.value(QStringLiteral("error")).toString()));

        for (const QVariant& bad : {QVariant(1e300), QVariant(qInf()), QVariant(true), QVariant(300.7),
                                    QVariant(0), QVariant(-5), QVariant(86401), QVariant(QStringLiteral("abc")),
                                    QVariant(qlonglong(9223372036854775807LL))}) {
            QVariantMap withTimeout = args;
            withTimeout.insert(QStringLiteral("timeoutSeconds"), bad);
            QVERIFY2(seps.buildChallenge(withTimeout).contains(QStringLiteral("error")),
                     qPrintable(bad.toString()));
        }
        QVariantMap noWebAuth = args;
        noWebAuth.insert(QStringLiteral("webAuthDomain"), QString());
        QVERIFY(seps.buildChallenge(noWebAuth).contains(QStringLiteral("error")));
        QVariantMap typo = args;
        typo.insert(QStringLiteral("network"), QStringLiteral("pubnet"));
        QVERIFY(seps.buildChallenge(typo).contains(QStringLiteral("error")));
    }

    /** A message given as hex is signed as those bytes: "0xab" signed 00 ab. */
    void testAMessageInHexIsReadStrictly()
    {
        SepsQml seps;
        QScopedPointer<KeyPair> signer(KeyPair::random());
        auto sign = [&](const QString& hex) {
            QVariantMap args;
            args.insert(QStringLiteral("message"), hex);
            args.insert(QStringLiteral("encoding"), QStringLiteral("hex"));
            args.insert(QStringLiteral("secretSeed"), signer->getSecretSeed());
            args.insert(QStringLiteral("seed"), signer->getSecretSeed());
            return seps.signMessage(args);
        };
        QVERIFY(sign(QStringLiteral("0xab")).contains(QStringLiteral("error")));
        QVERIFY(sign(QStringLiteral("abc")).contains(QStringLiteral("error")));
    }


    /** A payment request says what was asked for, or is not written: a memo
     *  without its type, and SEP-7's own key names, were dropped in silence. */
    void testAPaymentRequestIsWhatWasAsked()
    {
        SepsQml seps;
        QScopedPointer<KeyPair> dest(KeyPair::random());
        QScopedPointer<KeyPair> issuer(KeyPair::random());
        QVariantMap args;
        args.insert(QStringLiteral("destination"), dest->getAccountId());
        args.insert(QStringLiteral("amount"), QStringLiteral("100"));
        args.insert(QStringLiteral("assetCode"), QStringLiteral("USDC"));
        args.insert(QStringLiteral("assetIssuer"), issuer->getAccountId());
        args.insert(QStringLiteral("memo"), QStringLiteral("12345"));
        QVERIFY(seps.buildPayUri(args).contains(QStringLiteral("error")));      // memo, no type
        args.insert(QStringLiteral("memoType"), QStringLiteral("MEMO_ID"));
        const QVariantMap ok = seps.buildPayUri(args);
        QVERIFY2(!ok.contains(QStringLiteral("error")), qPrintable(ok.value(QStringLiteral("error")).toString()));
        QVERIFY(ok.value(QStringLiteral("uri")).toString().contains(QStringLiteral("memo_type=MEMO_ID")));

        QVariantMap wire = args;
        wire.insert(QStringLiteral("asset_code"), QStringLiteral("USDC"));
        QVERIFY(seps.buildPayUri(wire).contains(QStringLiteral("error")));      // unknown key
        QVariantMap negative = args;
        negative.insert(QStringLiteral("amount"), QStringLiteral("-5"));
        QVERIFY(seps.buildPayUri(negative).contains(QStringLiteral("error")));
        QVariantMap badId = args;
        badId.insert(QStringLiteral("memo"), QStringLiteral("abc"));
        QVERIFY(seps.buildPayUri(badId).contains(QStringLiteral("error")));
    }

    /**
     * A client_domain challenge signed by the server and the client_domain key
     * only — the client never signed — came back { valid: true, count: 0 }.
     */
    void testVerifySignersNeverReportsALoginNoClientSigned()
    {
        QScopedPointer<KeyPair> server(KeyPair::random());
        QScopedPointer<KeyPair> client(KeyPair::random());
        QScopedPointer<KeyPair> clientDomainKey(KeyPair::random());
        SepsQml seps;
        QScopedPointer<Transaction> tx(Sep10Challenge::buildChallengeTx(
            server.data(), client->getAccountId(), HOME(), WEB_AUTH(), 300,
            Network::testnetNetwork(), QStringLiteral("wallet.example"),
            clientDomainKey->getAccountId()));
        tx->sign(clientDomainKey.data());

        QVariantMap args;
        args.insert(QStringLiteral("xdr"), tx->toEnvelopeXdrBase64());
        args.insert(QStringLiteral("serverAccountId"), server->getAccountId());
        args.insert(QStringLiteral("homeDomain"), HOME());
        args.insert(QStringLiteral("webAuthDomain"), WEB_AUTH());
        args.insert(QStringLiteral("network"), QStringLiteral("testnet"));
        args.insert(QStringLiteral("signers"), QVariantList{client->getAccountId()});
        const QVariantMap out = seps.verifySigners(args);
        QVERIFY(out.contains(QStringLiteral("error")));
        QVERIFY(!out.value(QStringLiteral("valid")).toBool());

        // With the client's signature it is a login, and only the client counts.
        tx->sign(client.data());
        args.insert(QStringLiteral("xdr"), tx->toEnvelopeXdrBase64());
        const QVariantMap ok = seps.verifySigners(args);
        QVERIFY2(ok.value(QStringLiteral("valid")).toBool(), qPrintable(ok.value(QStringLiteral("error")).toString()));
        QCOMPARE(ok.value(QStringLiteral("count")).toInt(), 1);
    }

    /** A "client_domain" name with a UTF-8 BOM in front, from a key that is
     *  not the server's, read as client_domain and was signed. */
    void testSignChallengeRefusesANameThatMatchesOnlyWithoutItsBom()
    {
        QScopedPointer<KeyPair> server(KeyPair::random());
        QScopedPointer<KeyPair> client(KeyPair::random());
        QScopedPointer<KeyPair> other(KeyPair::random());
        SepsQml seps;
        const QByteArray name = QByteArray("\xEF\xBB\xBF", 3) + "client_domain";
        const QByteArray value("wallet.example");
        stellar::ManageDataOp raw;
        raw.dataName.set(const_cast<char*>(name.data()), static_cast<int>(name.size()));
        raw.dataValue.value.set(reinterpret_cast<uchar*>(const_cast<char*>(value.data())),
                                static_cast<int>(value.size()));
        raw.dataValue.filled = 1;
        auto* bomClientDomain = new ManageDataOperation(raw);
        bomClientDomain->setSourceAccount(other->getAccountId());

        QVERIFY(refused(seps.signChallenge(argsFor(
            handMade(server.data(), client->getAccountId(), nonceWithPlusAndSlash().toBase64(),
                     0, bomClientDomain),
            server.data(), client.data()))));
    }

    /** A blank homeDomain built a challenge keyed " auth", which no reader
     *  accepts, signed by the server. */
    void testBuildChallengeRequiresAHomeDomain()
    {
        SepsQml seps;
        QScopedPointer<KeyPair> server(KeyPair::random());
        QScopedPointer<KeyPair> client(KeyPair::random());
        QVariantMap args;
        args.insert(QStringLiteral("serverSeed"), server->getSecretSeed());
        args.insert(QStringLiteral("clientAccountId"), client->getAccountId());
        args.insert(QStringLiteral("webAuthDomain"), WEB_AUTH());
        for (const QString& blank : {QString(), QStringLiteral(" "), QStringLiteral("\t")}) {
            args.insert(QStringLiteral("homeDomain"), blank);
            QVERIFY2(seps.buildChallenge(args).contains(QStringLiteral("error")), qPrintable(blank));
        }
        args.remove(QStringLiteral("homeDomain"));
        QVERIFY(seps.buildChallenge(args).contains(QStringLiteral("error")));
    }

    /**
     * A challenge for a muxed M account was refused at every step. It is
     * built, read and signed now, as js does. The M address has to be named:
     * the G's owner signing without naming it is not asking to log in one of
     * its sub-accounts. Its signers are those of its G account.
     */
    void testSignChallengeForAMuxedClient()
    {
        QScopedPointer<KeyPair> server(KeyPair::random());
        QScopedPointer<KeyPair> client(KeyPair::random());
        SepsQml seps;
        const QString muxed =
            MuxedAccount::fromAccountIdAndMuxedId(client->getAccountId(), 42).toAddress();

        QVariantMap build;
        build.insert(QStringLiteral("serverSeed"), server->getSecretSeed());
        build.insert(QStringLiteral("clientAccountId"), muxed);
        build.insert(QStringLiteral("homeDomain"), HOME());
        build.insert(QStringLiteral("webAuthDomain"), WEB_AUTH());
        build.insert(QStringLiteral("network"), QStringLiteral("testnet"));
        const QVariantMap built = seps.buildChallenge(build);
        QVERIFY2(!built.contains(QStringLiteral("error")),
                 qPrintable(built.value(QStringLiteral("error")).toString()));
        QCOMPARE(built.value(QStringLiteral("clientBaseAccountId")).toString(), client->getAccountId());

        QVariantMap args = argsFor(built.value(QStringLiteral("xdr")).toString(),
                                   server.data(), client.data());
        const QVariantMap read = seps.readChallenge(args);
        QVERIFY2(!read.contains(QStringLiteral("error")),
                 qPrintable(read.value(QStringLiteral("error")).toString()));
        QCOMPARE(read.value(QStringLiteral("clientAccountId")).toString(), muxed);
        QCOMPARE(read.value(QStringLiteral("clientBaseAccountId")).toString(), client->getAccountId());
        QCOMPARE(read.value(QStringLiteral("clientMuxedId")).toString(), QStringLiteral("42"));

        QVERIFY(refused(seps.signChallenge(args)));
        args.insert(QStringLiteral("clientAccountId"), muxed);
        const QVariantMap out = seps.signChallenge(args);
        QVERIFY2(!out.contains(QStringLiteral("error")),
                 qPrintable(out.value(QStringLiteral("error")).toString()));
        QCOMPARE(out.value(QStringLiteral("clientAccountId")).toString(), muxed);
        QCOMPARE(out.value(QStringLiteral("signedBy")).toString(), client->getAccountId());

        QCOMPARE(Sep10Challenge::verifyChallengeTransactionSigners(
                     out.value(QStringLiteral("xdr")).toString(), server->getAccountId(), HOME(),
                     WEB_AUTH(), QSet<QString>{client->getAccountId()}, Network::testnetNetwork()),
                 QSet<QString>{client->getAccountId()});
        QVariantMap verify = args;
        verify.insert(QStringLiteral("xdr"), out.value(QStringLiteral("xdr")));
        verify.insert(QStringLiteral("signers"), QVariantList{client->getAccountId()});
        QVERIFY(seps.verifySigners(verify).value(QStringLiteral("valid")).toBool());
    }

    /**
     * A challenge that already carried a signature other than the server's
     * was signed, and the server then refused the result for a signature it
     * could not attribute. It is refused before signing now, unless the caller
     * names the key that signed before, as the next co-signer of a multisig
     * account does.
     */
    void testSignChallengeRefusesSignaturesOtherThanTheServers()
    {
        QScopedPointer<KeyPair> server(KeyPair::random());
        QScopedPointer<KeyPair> client(KeyPair::random());
        QScopedPointer<KeyPair> stranger(KeyPair::random());
        SepsQml seps;

        QScopedPointer<Transaction> tx(Sep10Challenge::buildChallengeTx(
            server.data(), client->getAccountId(), HOME(), WEB_AUTH(), 300, Network::testnetNetwork()));
        tx->sign(stranger.data());
        QVariantMap args = argsFor(tx->toEnvelopeXdrBase64(), server.data(), client.data());
        const QVariantMap out = seps.signChallenge(args);
        QVERIFY(refused(out));
        QVERIFY(out.value(QStringLiteral("error")).toString().contains(QStringLiteral("not the server's")));

        // Named, it is expected there, and the client signs next to it.
        args.insert(QStringLiteral("alreadySignedBy"), QVariantList{stranger->getAccountId()});
        const QVariantMap named = seps.signChallenge(args);
        QVERIFY2(!named.contains(QStringLiteral("error")),
                 qPrintable(named.value(QStringLiteral("error")).toString()));
        QCOMPARE(named.value(QStringLiteral("signatureCount")).toInt(), 3);

        // The server's signature twice is one signature the server cannot
        // place; so is a hash(x) preimage.
        QScopedPointer<Transaction> twice(Sep10Challenge::buildChallengeTx(
            server.data(), client->getAccountId(), HOME(), WEB_AUTH(), 300, Network::testnetNetwork()));
        twice->sign(server.data());
        QCOMPARE(twice->getSignatures().size(), 2);
        QVERIFY(refused(seps.signChallenge(
            argsFor(twice->toEnvelopeXdrBase64(), server.data(), client.data()))));
        QScopedPointer<Transaction> preimage(Sep10Challenge::buildChallengeTx(
            server.data(), client->getAccountId(), HOME(), WEB_AUTH(), 300, Network::testnetNetwork()));
        preimage->sign(QByteArray("a preimage"));
        QVERIFY(refused(seps.signChallenge(
            argsFor(preimage->toEnvelopeXdrBase64(), server.data(), client.data()))));

        // Named but not there, the server named, or not an account: refused.
        QVariantMap wrongly = argsFor(challengeFor(server.data(), client->getAccountId()),
                                      server.data(), client.data());
        for (const QVariant& listed : {QVariant(stranger->getAccountId()),
                                       QVariant(QVariantList{server->getAccountId()}),
                                       QVariant(QVariantList{QStringLiteral("GSTRANGER")}),
                                       QVariant(42)}) {
            wrongly.insert(QStringLiteral("alreadySignedBy"), listed);
            QVERIFY2(refused(seps.signChallenge(wrongly)), qPrintable(listed.toString()));
        }

        // The multisig flow: one co-signer signs, the next names it and signs.
        QScopedPointer<KeyPair> account(KeyPair::random());
        QScopedPointer<KeyPair> second(KeyPair::random());
        QVariantMap first = argsFor(challengeFor(server.data(), account->getAccountId()),
                                    server.data(), client.data());
        first.insert(QStringLiteral("clientAccountId"), account->getAccountId());
        const QVariantMap once = seps.signChallenge(first);
        QVERIFY2(!once.contains(QStringLiteral("error")),
                 qPrintable(once.value(QStringLiteral("error")).toString()));
        QVariantMap next = argsFor(once.value(QStringLiteral("xdr")).toString(),
                                   server.data(), second.data());
        next.insert(QStringLiteral("clientAccountId"), account->getAccountId());
        QVERIFY(refused(seps.signChallenge(next)));
        next.insert(QStringLiteral("alreadySignedBy"), client->getAccountId());
        const QVariantMap both = seps.signChallenge(next);
        QVERIFY2(!both.contains(QStringLiteral("error")),
                 qPrintable(both.value(QStringLiteral("error")).toString()));
        QCOMPARE(Sep10Challenge::verifyChallengeTransactionSigners(
                     both.value(QStringLiteral("xdr")).toString(), server->getAccountId(), HOME(),
                     WEB_AUTH(), QSet<QString>{client->getAccountId(), second->getAccountId()},
                     Network::testnetNetwork()),
                 (QSet<QString>{client->getAccountId(), second->getAccountId()}));
    }

    /**
     * The client_domain operation was never checked: a client that asked to
     * log in through wallet.example signed a challenge naming another domain,
     * or wallet.example from a key that is not its SIGNING_KEY, vouching for
     * whatever the server wrote. Told the domain and its key, readChallenge
     * and signChallenge check both; asked to sign for the domain as well, the
     * key signing must be the operation's source.
     */
    void testSignChallengeBindsTheClientDomainToItsKey()
    {
        QScopedPointer<KeyPair> server(KeyPair::random());
        QScopedPointer<KeyPair> client(KeyPair::random());
        QScopedPointer<KeyPair> wallet(KeyPair::random());
        QScopedPointer<KeyPair> other(KeyPair::random());
        SepsQml seps;
        auto naming = [&](const QString& domain, KeyPair* key) {
            QScopedPointer<Transaction> tx(Sep10Challenge::buildChallengeTx(
                server.data(), client->getAccountId(), HOME(), WEB_AUTH(), 300,
                Network::testnetNetwork(), domain, key->getAccountId()));
            return tx->toEnvelopeXdrBase64();
        };
        const QString good = naming(QStringLiteral("wallet.example"), wallet.data());

        // Not told: signed as before, and the operation is reported.
        QVariantMap args = argsFor(good, server.data(), client.data());
        QVariantMap out = seps.signChallenge(args);
        QVERIFY2(!out.contains(QStringLiteral("error")), qPrintable(out.value(QStringLiteral("error")).toString()));
        QCOMPARE(out.value(QStringLiteral("clientDomain")).toString(), QStringLiteral("wallet.example"));
        QCOMPARE(out.value(QStringLiteral("clientDomainSigningKey")).toString(), wallet->getAccountId());
        QCOMPARE(out.value(QStringLiteral("clientDomainChecked")).toBool(), false);

        // Told: the domain alone is not enough, the two together are checked.
        args.insert(QStringLiteral("clientDomain"), QStringLiteral("wallet.example"));
        QVERIFY(refused(seps.signChallenge(args)));
        QVERIFY(seps.readChallenge(args).contains(QStringLiteral("error")));
        args.insert(QStringLiteral("clientDomainSigningKey"), wallet->getAccountId());
        out = seps.signChallenge(args);
        QVERIFY2(!out.contains(QStringLiteral("error")), qPrintable(out.value(QStringLiteral("error")).toString()));
        QCOMPARE(out.value(QStringLiteral("clientDomainChecked")).toBool(), true);
        QCOMPARE(seps.readChallenge(args).value(QStringLiteral("clientDomainChecked")).toBool(), true);

        // Another domain, the domain from another key, or none at all.
        for (const QString& xdr : {naming(QStringLiteral("evil.example"), wallet.data()),
                                   naming(QStringLiteral("wallet.example"), other.data()),
                                   challengeFor(server.data(), client->getAccountId())}) {
            QVariantMap wrong = args;
            wrong.insert(QStringLiteral("xdr"), xdr);
            QVERIFY(refused(seps.signChallenge(wrong)));
            QVERIFY(seps.readChallenge(wrong).contains(QStringLiteral("error")));
        }
        // A misspelt expectation is refused rather than skipped.
        QVariantMap typo = argsFor(naming(QStringLiteral("evil.example"), other.data()),
                                   server.data(), client.data());
        typo.insert(QStringLiteral("client_domain"), QStringLiteral("wallet.example"));
        QVERIFY(refused(seps.signChallenge(typo)));

        // Signing for the domain too: the seed's account must be the source.
        QVariantMap both = argsFor(good, server.data(), client.data());
        both.insert(QStringLiteral("clientDomain"), QStringLiteral("wallet.example"));
        both.insert(QStringLiteral("clientDomainSeed"), wallet->getSecretSeed());
        const QVariantMap signedBoth = seps.signChallenge(both);
        QVERIFY2(!signedBoth.contains(QStringLiteral("error")),
                 qPrintable(signedBoth.value(QStringLiteral("error")).toString()));
        QCOMPARE(signedBoth.value(QStringLiteral("signatureCount")).toInt(), 3);
        QCOMPARE(signedBoth.value(QStringLiteral("clientDomainSignedBy")).toString(), wallet->getAccountId());
        QCOMPARE(Sep10Challenge::verifyChallengeTransactionSigners(
                     signedBoth.value(QStringLiteral("xdr")).toString(), server->getAccountId(), HOME(),
                     WEB_AUTH(), QSet<QString>{client->getAccountId()}, Network::testnetNetwork()),
                 QSet<QString>{client->getAccountId()});
        QVariantMap wrongSeed = both;
        wrongSeed.insert(QStringLiteral("clientDomainSeed"), other->getSecretSeed());
        QVERIFY(refused(seps.signChallenge(wrongSeed)));
        QVariantMap otherKey = both;
        otherKey.insert(QStringLiteral("clientDomainSigningKey"), other->getAccountId());
        QVERIFY(refused(seps.signChallenge(otherKey)));
        QVariantMap noDomain = both;
        noDomain.remove(QStringLiteral("clientDomain"));
        QVERIFY(refused(seps.signChallenge(noDomain)));
        QVariantMap sameKey = both;
        sameKey.insert(QStringLiteral("clientDomainSeed"), client->getSecretSeed());
        QVERIFY(refused(seps.signChallenge(sameKey)));

        // Signed by the wallet's backend first: named, then signed by the client.
        QScopedPointer<Transaction> backend(Sep10Challenge::buildChallengeTx(
            server.data(), client->getAccountId(), HOME(), WEB_AUTH(), 300, Network::testnetNetwork(),
            QStringLiteral("wallet.example"), wallet->getAccountId()));
        backend->sign(wallet.data());
        QVariantMap after = argsFor(backend->toEnvelopeXdrBase64(), server.data(), client.data());
        after.insert(QStringLiteral("clientDomain"), QStringLiteral("wallet.example"));
        after.insert(QStringLiteral("clientDomainSigningKey"), wallet->getAccountId());
        QVERIFY(refused(seps.signChallenge(after)));
        after.insert(QStringLiteral("alreadySignedBy"), wallet->getAccountId());
        const QVariantMap done = seps.signChallenge(after);
        QVERIFY2(!done.contains(QStringLiteral("error")), qPrintable(done.value(QStringLiteral("error")).toString()));
        QCOMPARE(Sep10Challenge::verifyChallengeTransactionSigners(
                     done.value(QStringLiteral("xdr")).toString(), server->getAccountId(), HOME(),
                     WEB_AUTH(), QSet<QString>{client->getAccountId()}, Network::testnetNetwork()),
                 QSet<QString>{client->getAccountId()});
    }

    /** Two client_domain operations were read and signed; one was shown. */
    void testSignChallengeRefusesTwoClientDomainOperations()
    {
        QScopedPointer<KeyPair> server(KeyPair::random());
        QScopedPointer<KeyPair> client(KeyPair::random());
        QScopedPointer<KeyPair> wallet(KeyPair::random());
        QScopedPointer<KeyPair> other(KeyPair::random());
        SepsQml seps;
        const qint64 now = QDateTime::currentMSecsSinceEpoch() / 1000;
        Account account(KeyPair::fromAccountId(server->getAccountId()), -1);
        ManageDataOperation* auth = ManageDataOperation::create(HOME() + QStringLiteral(" auth"),
                                                                nonceWithPlusAndSlash().toBase64());
        auth->setSourceAccount(client->getAccountId());
        ManageDataOperation* web = ManageDataOperation::create(QStringLiteral("web_auth_domain"),
                                                               WEB_AUTH().toUtf8());
        web->setSourceAccount(server->getAccountId());
        ManageDataOperation* first = ManageDataOperation::create(QStringLiteral("client_domain"),
                                                                 QByteArray("wallet.example"));
        first->setSourceAccount(wallet->getAccountId());
        ManageDataOperation* second = ManageDataOperation::create(QStringLiteral("client_domain"),
                                                                  QByteArray("other.example"));
        second->setSourceAccount(other->getAccountId());
        QScopedPointer<Transaction> tx(TransactionBuilder(&account, Network::testnetNetwork())
                                           .addOperation(auth).addOperation(web)
                                           .addOperation(first).addOperation(second)
                                           .addTimeBounds(new TimeBounds(now, now + 300))
                                           .setBaseFee(100)
                                           .build());
        tx->sign(server.data());

        QVariantMap args = argsFor(tx->toEnvelopeXdrBase64(), server.data(), client.data());
        QVERIFY(seps.readChallenge(args).contains(QStringLiteral("error")));
        QVERIFY(refused(seps.signChallenge(args)));
        args.insert(QStringLiteral("clientDomain"), QStringLiteral("wallet.example"));
        args.insert(QStringLiteral("clientDomainSigningKey"), wallet->getAccountId());
        QVERIFY(refused(seps.signChallenge(args)));
    }

    /**
     * resolveFederation("") never answered: the SDK handed "" back as an
     * account id with no reply behind it, so neither resolved() nor failed()
     * came. A value refused before any request goes out now answers at once,
     * in the returned map, and on failed() as well.
     */
    void testResolveFederationAnswersAMalformedAddressAtOnce()
    {
        SepsQml seps;
        QSignalSpy failed(&seps, &SepsQml::failed);
        QSignalSpy resolved(&seps, &SepsQml::resolved);
        QScopedPointer<KeyPair> key(KeyPair::random());
        QString corrupt = key->getAccountId();
        corrupt[20] = corrupt.at(20) == QLatin1Char('A') ? QLatin1Char('B') : QLatin1Char('A');

        const QStringList bad{QString(), QStringLiteral("   "), QStringLiteral("a*b*c"),
                              QStringLiteral("*example.com"), QStringLiteral("bob*"),
                              QStringLiteral("hello"), QStringLiteral("bob*good.example@evil.example"),
                              QStringLiteral("bob*example.com/x?"), QStringLiteral("bob*exa mple.com"),
                              key->getSecretSeed(), corrupt, key->getAccountId().toLower()};
        QList<int> ids;
        for (const QString& value : bad) {
            const QVariantMap out = seps.resolveFederation(value);
            QVERIFY2(out.contains(QStringLiteral("error")), qPrintable(value));
            // A seed pasted by mistake is not repeated back.
            QVERIFY(!out.value(QStringLiteral("error")).toString().contains(key->getSecretSeed()));
            ids.append(out.value(QStringLiteral("requestId")).toInt());
        }
        QCOMPARE(seps.pending(), 0);
        QTRY_COMPARE(failed.count(), bad.size());
        for (int i = 0; i < failed.count(); ++i) {
            QCOMPARE(failed.at(i).at(0).toInt(), ids.at(i));
            QCOMPARE(failed.at(i).at(1).toMap().value(QStringLiteral("kind")).toString(),
                     QStringLiteral("validation"));
        }
        QCOMPARE(resolved.count(), 0);
        QCOMPARE(seps.describeAddress(QString()).value(QStringLiteral("kind")).toString(),
                 QStringLiteral("malformed"));
        QCOMPARE(seps.describeAddress(QStringLiteral("*example.com")).value(QStringLiteral("kind")).toString(),
                 QStringLiteral("malformed"));

        // An account id, G or M, is its own answer, as before.
        const QString muxed = MuxedAccount::fromAccountIdAndMuxedId(key->getAccountId(), 3).toAddress();
        QVERIFY(!seps.resolveFederation(key->getAccountId()).contains(QStringLiteral("error")));
        QVERIFY(!seps.resolveFederation(muxed).contains(QStringLiteral("error")));
        QTRY_COMPARE(resolved.count(), 2);
        QCOMPARE(resolved.at(0).at(1).toMap().value(QStringLiteral("accountId")).toString(),
                 key->getAccountId());
        QCOMPARE(resolved.at(1).at(1).toMap().value(QStringLiteral("accountId")).toString(), muxed);
        QCOMPARE(failed.count(), bad.size());
    }

    /**
     * newSep45Challenge dropped what it did not know in silence: `timeout: 30`
     * signed a 300-second challenge, a misspelt `netwrok` a testnet one, a
     * `nonceHex` a random one. Unknown keys are refused, as buildChallenge
     * does. (Since 1.0.4 the call issues SEP-45 v0.1.1 challenges, whose
     * nonce is a string named `nonce`: the keys of the earlier format,
     * timeoutSeconds, nonceHex and webAuthFunctionName among them, are now
     * the unknown ones.)
     */
    void testNewSep45ChallengeRefusesUnknownArguments()
    {
        SepsQml seps;
        QScopedPointer<KeyPair> server(KeyPair::random());
        const QVariantMap args = sep45Args(server.data());
        const QVariantMap ok = seps.newSep45Challenge(args);
        QVERIFY2(!ok.contains(QStringLiteral("error")), qPrintable(ok.value(QStringLiteral("error")).toString()));
        QVERIFY(!ok.value(QStringLiteral("authorizationEntries")).toString().isEmpty());

        for (const char* key : {"timeout", "timeoutSeconds", "TimeoutSeconds", "nonceHex", "netwrok",
                                "webAuthFunctionName", "homedomain", "expireInLedger", "simulated"}) {
            QVariantMap withKey = args;
            withKey.insert(QString::fromLatin1(key), QStringLiteral("30"));
            QVERIFY2(seps.newSep45Challenge(withKey).contains(QStringLiteral("error")), key);
        }
        for (const char* key : {"homeDomain", "webAuthDomain", "webAuthContract", "latestLedger",
                                "clientAccountId", "serverSeed"}) {
            QVariantMap without = args;
            without.remove(QString::fromLatin1(key));
            QVERIFY2(seps.newSep45Challenge(without).contains(QStringLiteral("error")), key);
        }
        // A ledger is a whole number, read exactly.
        for (const QVariant& ledger : {QVariant(0), QVariant(1.5), QVariant(true), QVariant(QStringLiteral("x")),
                                       QVariant(-1), QVariant(qint64(4294967296LL))}) {
            QVariantMap bad = args;
            bad.insert(QStringLiteral("latestLedger"), ledger);
            QVERIFY2(seps.newSep45Challenge(bad).contains(QStringLiteral("error")), qPrintable(ledger.toString()));
        }
    }

    /**
     * inspectSep45Challenge read `homeDomains` as a list, so on Qt 6 the string
     * "example.com" became its letters: a challenge for "e" passed and the one
     * for example.com did not. Blank domains passed a challenge with none.
     */
    void testInspectSep45ChallengeReadsDomainsAsSep10Does()
    {
        SepsQml seps;
        QScopedPointer<KeyPair> server(KeyPair::random());
        const QString forHome = seps.newSep45Challenge(sep45Args(server.data()))
                                    .value(QStringLiteral("authorizationEntries")).toString();
        const QString forE = seps.newSep45Challenge(sep45Args(server.data(), QStringLiteral("e")))
                                 .value(QStringLiteral("authorizationEntries")).toString();
        QVERIFY(!forHome.isEmpty() && !forE.isEmpty());

        QVariantMap args = inspectArgs(forHome, server.data());
        args.remove(QStringLiteral("homeDomain"));
        args.insert(QStringLiteral("homeDomains"), HOME());
        const QVariantMap ok = seps.inspectSep45Challenge(args);
        QVERIFY2(ok.value(QStringLiteral("structurallyValid")).toBool(),
                 qPrintable(ok.value(QStringLiteral("error")).toString()));

        args.insert(QStringLiteral("authorizationEntries"), forE);
        QVERIFY(seps.inspectSep45Challenge(args).contains(QStringLiteral("error")));

        QVariantMap blank = inspectArgs(forHome, server.data());
        blank.remove(QStringLiteral("homeDomain"));
        blank.insert(QStringLiteral("homeDomains"), QVariantList{QString(), QStringLiteral(" ")});
        QVERIFY(seps.inspectSep45Challenge(blank).contains(QStringLiteral("error")));
        QVariantMap noWebAuth = inspectArgs(forHome, server.data());
        noWebAuth.insert(QStringLiteral("webAuthDomain"), QString());
        QVERIFY(seps.inspectSep45Challenge(noWebAuth).contains(QStringLiteral("error")));
    }

    /**
     * inspectSep45Challenge said structurallyValid without saying, or checking,
     * what the client would authorize: the contract and function it calls went
     * unreported, a webAuthContract argument was ignored, and so was a typo.
     * It now also reports every entry, and checks the client and the client
     * domain when told them.
     */
    void testInspectSep45ChallengeShowsAndChecksWhatWouldBeAuthorized()
    {
        SepsQml seps;
        QScopedPointer<KeyPair> server(KeyPair::random());
        QScopedPointer<KeyPair> domain(KeyPair::random());
        QVariantMap issue = sep45Args(server.data());
        issue.insert(QStringLiteral("clientDomain"), QStringLiteral("wallet.example"));
        issue.insert(QStringLiteral("clientDomainAccount"), domain->getAccountId());
        const QString entries = seps.newSep45Challenge(issue).value(QStringLiteral("authorizationEntries")).toString();
        QVERIFY(!entries.isEmpty());

        QVariantMap args = inspectArgs(entries, server.data());
        const QVariantMap checked = seps.inspectSep45Challenge(args);
        QVERIFY2(checked.value(QStringLiteral("structurallyValid")).toBool(),
                 qPrintable(checked.value(QStringLiteral("error")).toString()));
        QCOMPARE(checked.value(QStringLiteral("contract")).toString(), WEB_AUTH_CONTRACT());
        QCOMPARE(checked.value(QStringLiteral("function")).toString(), QStringLiteral("web_auth_verify"));
        QCOMPARE(checked.value(QStringLiteral("clientAccountId")).toString(), CLIENT_CONTRACT());
        QCOMPARE(checked.value(QStringLiteral("serverSignatureVerified")).toBool(), true);
        QCOMPARE(checked.value(QStringLiteral("clientProven")).toBool(), false);
        QCOMPARE(checked.value(QStringLiteral("clientDomain")).toString(), QStringLiteral("wallet.example"));
        QCOMPARE(checked.value(QStringLiteral("clientDomainChecked")).toBool(), false);
        const QVariantList rows = checked.value(QStringLiteral("entries")).toList();
        QCOMPARE(rows.size(), 3);
        QStringList roles;
        for (const QVariant& row : rows) {
            roles.append(row.toMap().value(QStringLiteral("role")).toString());
            QCOMPARE(row.toMap().value(QStringLiteral("signed")).toBool(),
                     row.toMap().value(QStringLiteral("role")).toString() == QStringLiteral("server"));
        }
        QCOMPARE(roles, (QStringList{QStringLiteral("client"), QStringLiteral("server"),
                                     QStringLiteral("clientDomain")}));

        // Without the contract, any contract's web_auth_verify passed.
        QVariantMap noContract = inspectArgs(entries, server.data());
        noContract.remove(QStringLiteral("webAuthContract"));
        QVERIFY(seps.inspectSep45Challenge(noContract).contains(QStringLiteral("error")));
        args.insert(QStringLiteral("webAuthContract"), StrKey::encodeContract(QByteArray(32, '\x33')));
        QVERIFY(seps.inspectSep45Challenge(args).contains(QStringLiteral("error")));

        QVariantMap typo = inspectArgs(entries, server.data());
        typo.insert(QStringLiteral("webAuthContrat"), WEB_AUTH_CONTRACT());
        QVERIFY(seps.inspectSep45Challenge(typo).contains(QStringLiteral("error")));
        // The function is not the anchor's to choose any more: web_auth_verify.
        QVariantMap named = inspectArgs(entries, server.data());
        named.insert(QStringLiteral("webAuthFunctionName"), QStringLiteral("web_auth_verify"));
        QVERIFY(seps.inspectSep45Challenge(named).contains(QStringLiteral("error")));

        // A challenge calling some other function is not a login challenge.
        QByteArray raw = QByteArray::fromBase64(entries.toLatin1());
        QDataStream in(&raw, QIODevice::ReadOnly);
        qint32 count = 0;
        in >> count;
        QByteArray edited;
        QDataStream out(&edited, QIODevice::WriteOnly);
        out << count;
        for (qint32 i = 0; i < count; ++i) {
            stellar::SorobanAuthorizationEntry entry;
            in >> entry;
            entry.rootInvocation.function.contractFn.functionName = "transfer";
            out << entry;
        }
        const QString transfer = seps.inspectSep45Challenge(
            inspectArgs(QString::fromLatin1(edited.toBase64()), server.data())).value(QStringLiteral("error")).toString();
        QVERIFY2(transfer.contains(QStringLiteral("web_auth_verify")), qPrintable(transfer));

        // Told the client or the client domain, it checks them.
        QVariantMap mine = inspectArgs(entries, server.data());
        mine.insert(QStringLiteral("clientAccountId"), CLIENT_CONTRACT());
        mine.insert(QStringLiteral("clientDomain"), QStringLiteral("wallet.example"));
        mine.insert(QStringLiteral("clientDomainAccount"), domain->getAccountId());
        QCOMPARE(seps.inspectSep45Challenge(mine).value(QStringLiteral("clientDomainChecked")).toBool(), true);
        QVariantMap notMine = mine;
        notMine.insert(QStringLiteral("clientAccountId"), StrKey::encodeContract(QByteArray(32, '\x44')));
        QVERIFY(seps.inspectSep45Challenge(notMine).contains(QStringLiteral("error")));
        QVariantMap otherDomain = mine;
        otherDomain.insert(QStringLiteral("clientDomain"), QStringLiteral("evil.example"));
        QVERIFY(seps.inspectSep45Challenge(otherDomain).contains(QStringLiteral("error")));
        QVariantMap half = mine;
        half.remove(QStringLiteral("clientDomainAccount"));
        QVERIFY(seps.inspectSep45Challenge(half).contains(QStringLiteral("error")));
    }

    /**
     * The QML calls speak SEP-45 v0.1.1 as py does, byte for byte (fixtures
     * from py-stellar-sdk 14.1.0, see Sep45ChallengeTest): the transaction an
     * anchor simulates, the challenge it issues from the simulation, the
     * wallet's signature and the transaction the anchor simulates to verify.
     * Until 1.0.4 they spoke this SDK's own format, which nothing else reads.
     */
    void testSep45CallsInteroperateWithPy()
    {
        SepsQml seps;
        QVariantMap tx = pyIssueArgs();
        tx.remove(QStringLiteral("serverSeed"));
        tx.remove(QStringLiteral("latestLedger"));
        tx.insert(QStringLiteral("serverAccountId"), PY_SERVER());
        const QVariantMap simulate = seps.sep45ChallengeTransaction(tx);
        QCOMPARE(simulate.value(QStringLiteral("xdr")).toString(), pyChallengeTx());
        QCOMPARE(simulate.value(QStringLiteral("nonce")).toString(), PY_NONCE());

        QVariantMap issue = pyIssueArgs();
        issue.insert(QStringLiteral("simulatedAuth"), QVariant(pySimulatedAuth()));
        const QVariantMap issued = seps.newSep45Challenge(issue);
        QVERIFY2(!issued.contains(QStringLiteral("error")), qPrintable(issued.value(QStringLiteral("error")).toString()));
        QCOMPARE(issued.value(QStringLiteral("authorizationEntries")).toString(), pyBuilt());
        QCOMPARE(issued.value(QStringLiteral("simulated")).toBool(), true);
        QCOMPARE(issued.value(QStringLiteral("serverSignatureExpirationLedger")).toLongLong(), 82106 + 180);

        QVariantMap sign = pyReadArgs(pyBuilt());
        sign.insert(QStringLiteral("clientAccountId"), PY_CLIENT());
        sign.insert(QStringLiteral("secretSeed"), QStringLiteral("SARCEIRCEIRCEIRCEIRCEIRCEIRCEIRCEIRCEIRCEIRCEIRCEIRCEVGI"));
        sign.insert(QStringLiteral("validUntilLedger"), 82200);
        const QVariantMap signedOut = seps.signSep45Challenge(sign);
        QVERIFY2(!signedOut.contains(QStringLiteral("error")), qPrintable(signedOut.value(QStringLiteral("error")).toString()));
        QCOMPARE(signedOut.value(QStringLiteral("authorizationEntries")).toString(), pySigned());

        const QVariantMap verifyTx = seps.sep45VerificationTransaction(pyReadArgs(pySigned()));
        QCOMPARE(verifyTx.value(QStringLiteral("xdr")).toString(), pyVerifyTx());

        QVariantMap verify = pyReadArgs(pySigned());
        QVariantMap simulation;
        simulation.insert(QStringLiteral("ok"), true);
        simulation.insert(QStringLiteral("error"), QString());
        simulation.insert(QStringLiteral("returnValueXdr"), QStringLiteral("AAAAAQ=="));
        simulation.insert(QStringLiteral("auth"), authOf(pySigned()));
        verify.insert(QStringLiteral("simulation"), simulation);
        const QVariantMap verified = seps.verifySep45Challenge(verify);
        QVERIFY2(verified.value(QStringLiteral("verified")).toBool(),
                 qPrintable(verified.value(QStringLiteral("error")).toString()));
        QCOMPARE(verified.value(QStringLiteral("clientAccountId")).toString(), PY_CLIENT());
        QCOMPARE(verified.value(QStringLiteral("nonce")).toString(), PY_NONCE());

        // A simulation that failed, or ran nothing, verifies nothing.
        simulation.insert(QStringLiteral("ok"), false);
        simulation.insert(QStringLiteral("error"), QStringLiteral("HostError: Error(Auth, InvalidAction)"));
        verify.insert(QStringLiteral("simulation"), simulation);
        QVERIFY(seps.verifySep45Challenge(verify).contains(QStringLiteral("error")));
        simulation.insert(QStringLiteral("ok"), true);
        simulation.insert(QStringLiteral("error"), QString());
        simulation.remove(QStringLiteral("returnValueXdr"));
        verify.insert(QStringLiteral("simulation"), simulation);
        QVERIFY(seps.verifySep45Challenge(verify).contains(QStringLiteral("error")));
        verify.remove(QStringLiteral("simulation"));
        QVERIFY(seps.verifySep45Challenge(verify).contains(QStringLiteral("error")));

        // The simulated entries must be the challenge asked for: another nonce
        // than the call carried, and nothing is signed.
        issue.insert(QStringLiteral("nonce"), QStringLiteral("another"));
        QVERIFY(seps.newSep45Challenge(issue).contains(QStringLiteral("error")));
        issue.insert(QStringLiteral("nonce"), PY_NONCE());
        issue.insert(QStringLiteral("simulatedAuth"), pyBuilt());
        QVERIFY(seps.newSep45Challenge(issue).contains(QStringLiteral("error")));
    }

    /** The wallet's half end to end offline: issued, read, signed by the
     *  client and the client domain, read again, and handed to the anchor. */
    void testSignSep45ChallengeSignsOnlyWhatItRead()
    {
        SepsQml seps;
        QScopedPointer<KeyPair> server(KeyPair::random());
        QScopedPointer<KeyPair> client(KeyPair::random());
        QScopedPointer<KeyPair> domain(KeyPair::random());
        QVariantMap issue = sep45Args(server.data());
        issue.insert(QStringLiteral("clientDomain"), QStringLiteral("wallet.example"));
        issue.insert(QStringLiteral("clientDomainAccount"), domain->getAccountId());
        const QString entries = seps.newSep45Challenge(issue).value(QStringLiteral("authorizationEntries")).toString();

        QVariantMap sign = inspectArgs(entries, server.data());
        sign.insert(QStringLiteral("clientAccountId"), CLIENT_CONTRACT());
        sign.insert(QStringLiteral("secretSeed"), client->getSecretSeed());
        sign.insert(QStringLiteral("validUntilLedger"), QStringLiteral("1001"));
        sign.insert(QStringLiteral("clientDomain"), QStringLiteral("wallet.example"));
        sign.insert(QStringLiteral("clientDomainSeed"), domain->getSecretSeed());
        const QVariantMap out = seps.signSep45Challenge(sign);
        QVERIFY2(!out.contains(QStringLiteral("error")), qPrintable(out.value(QStringLiteral("error")).toString()));
        QCOMPARE(out.value(QStringLiteral("signedBy")).toString(), client->getAccountId());
        QCOMPARE(out.value(QStringLiteral("clientDomainSignedBy")).toString(), domain->getAccountId());
        QCOMPARE(out.value(QStringLiteral("validUntilLedger")).toLongLong(), 1001);
        for (const QVariant& row : out.value(QStringLiteral("entries")).toList())
            QVERIFY(row.toMap().value(QStringLiteral("signed")).toBool());
        const QString signedEntries = out.value(QStringLiteral("authorizationEntries")).toString();
        QVERIFY(seps.sep45VerificationTransaction(inspectArgs(signedEntries, server.data()))
                    .value(QStringLiteral("xdr")).toString().size() > 0);

        // Refused, with nothing signed: no key, no ledger (or not a ledger),
        // another client, a client domain key without its domain, a key that
        // is not the client domain account's, an argument it does not read.
        auto refusedSep45 = [&](const QVariantMap& args) {
            const QVariantMap result = seps.signSep45Challenge(args);
            return result.contains(QStringLiteral("error"))
                   && !result.contains(QStringLiteral("authorizationEntries"));
        };
        QVariantMap noKey = sign;
        noKey.remove(QStringLiteral("secretSeed"));
        noKey.remove(QStringLiteral("clientDomainSeed"));
        QVERIFY(refusedSep45(noKey));
        for (const QVariant& ledger : {QVariant(), QVariant(0), QVariant(1.5), QVariant(true), QVariant(QStringLiteral("soon"))}) {
            QVariantMap bad = sign;
            if (ledger.isValid())
                bad.insert(QStringLiteral("validUntilLedger"), ledger);
            else
                bad.remove(QStringLiteral("validUntilLedger"));
            QVERIFY2(refusedSep45(bad), qPrintable(ledger.toString()));
        }
        QVariantMap other = sign;
        other.insert(QStringLiteral("clientAccountId"), StrKey::encodeContract(QByteArray(32, '\x44')));
        QVERIFY(refusedSep45(other));
        QVariantMap noClient = sign;
        noClient.remove(QStringLiteral("clientAccountId"));
        QVERIFY(refusedSep45(noClient));
        QVariantMap noDomain = sign;
        noDomain.remove(QStringLiteral("clientDomain"));
        QVERIFY(refusedSep45(noDomain));
        QVariantMap wrongKey = sign;
        wrongKey.insert(QStringLiteral("clientDomainAccount"), server->getAccountId());
        QVERIFY(refusedSep45(wrongKey));
        QVariantMap typo = sign;
        typo.insert(QStringLiteral("validUntil"), 1001);
        QVERIFY(refusedSep45(typo));
        QVariantMap noContract = sign;
        noContract.remove(QStringLiteral("webAuthContract"));
        QVERIFY(refusedSep45(noContract));
        // Another anchor's key: the server's signature does not check out.
        QScopedPointer<KeyPair> impostor(KeyPair::random());
        QVariantMap impostorSigned = sign;
        impostorSigned.insert(QStringLiteral("serverAccountId"), impostor->getAccountId());
        QVERIFY(refusedSep45(impostorSigned));
    }

    /**
     * signChallenge's refusal of an alreadySignedBy entry that is not an
     * account id quoted the entry, so a secret seed pasted into the wrong
     * field came back in the error message, to be shown and logged. It is
     * named by its place now.
     */
    void testAlreadySignedByIsNamedByPlaceNotValue()
    {
        QScopedPointer<KeyPair> server(KeyPair::random());
        QScopedPointer<KeyPair> client(KeyPair::random());
        QScopedPointer<KeyPair> cosigner(KeyPair::random());
        SepsQml seps;
        QVariantMap args = argsFor(challengeFor(server.data(), client->getAccountId()), server.data(), client.data());
        args.insert(QStringLiteral("alreadySignedBy"),
                    QVariantList{cosigner->getAccountId(), cosigner->getSecretSeed()});
        QVariantMap out = seps.signChallenge(args);
        QVERIFY(refused(out));
        QString error = out.value(QStringLiteral("error")).toString();
        QVERIFY2(!error.contains(cosigner->getSecretSeed()), qPrintable(error));
        QVERIFY2(error.contains(QStringLiteral("alreadySignedBy[1]")), qPrintable(error));

        args.insert(QStringLiteral("alreadySignedBy"), cosigner->getSecretSeed());
        out = seps.signChallenge(args);
        QVERIFY(refused(out));
        error = out.value(QStringLiteral("error")).toString();
        QVERIFY2(!error.contains(cosigner->getSecretSeed()), qPrintable(error));
        QVERIFY2(error.contains(QStringLiteral("alreadySignedBy")), qPrintable(error));
    }

    /**
     * A federation answer was passed through as it came: an account_id that is
     * not an address, a memo invalid for its type, a numeric id memo rounded on
     * its way into a string. What would be paid is checked, here as in
     * resolveFederation.
     */
    void testParseFederationJsonRefusesWhatCannotBePaid()
    {
        SepsQml seps;
        QScopedPointer<KeyPair> key(KeyPair::random());
        const QString account = key->getAccountId();
        const QString muxed = MuxedAccount::fromAccountIdAndMuxedId(account, 7).toAddress();
        auto answer = [&](const QString& fields) {
            return seps.parseFederationJson(QStringLiteral("{\"stellar_address\":\"bob*example.com\",%1}").arg(fields));
        };
        const QString hash = QString::fromLatin1(QByteArray(32, '\x07').toBase64());
        for (const QString& fields : {QStringLiteral("\"account_id\":\"%1\"").arg(account),
                                      QStringLiteral("\"account_id\":\"%1\"").arg(muxed),
                                      QStringLiteral("\"account_id\":\"%1\",\"memo_type\":\"text\",\"memo\":\"1234567890123456789012345678\"").arg(account),
                                      QStringLiteral("\"account_id\":\"%1\",\"memo_type\":\"id\",\"memo\":\"18446744073709551615\"").arg(account),
                                      QStringLiteral("\"account_id\":\"%1\",\"memo_type\":\"hash\",\"memo\":\"%2\"").arg(account, hash),
                                      QStringLiteral("\"account_id\":\"%1\",\"memo_type\":null,\"memo\":null").arg(account)}) {
            const QVariantMap out = answer(fields);
            QVERIFY2(!out.contains(QStringLiteral("error")), qPrintable(fields + QStringLiteral(" -> ")
                                                                        + out.value(QStringLiteral("error")).toString()));
        }
        QCOMPARE(answer(QStringLiteral("\"account_id\":\"%1\",\"memo_type\":\"id\",\"memo\":\"42\"").arg(account))
                     .value(QStringLiteral("memo")).toString(), QStringLiteral("42"));

        QString corrupt = account;
        corrupt[20] = corrupt.at(20) == QLatin1Char('A') ? QLatin1Char('B') : QLatin1Char('A');
        for (const QString& fields : {QStringLiteral("\"memo\":\"1\",\"memo_type\":\"id\""),
                                      QStringLiteral("\"account_id\":\"%1\"").arg(corrupt),
                                      QStringLiteral("\"account_id\":\"%1\"").arg(key->getSecretSeed()),
                                      QStringLiteral("\"account_id\":\"%1\"").arg(StrKey::encodeContract(QByteArray(32, '\x01'))),
                                      QStringLiteral("\"account_id\":42"),
                                      QStringLiteral("\"account_id\":\"%1\",\"memo_type\":\"text\",\"memo\":\"12345678901234567890123456789\"").arg(account),
                                      QStringLiteral("\"account_id\":\"%1\",\"memo_type\":\"text\",\"memo\":\"ééééééééééééééé\"").arg(account),
                                      QStringLiteral("\"account_id\":\"%1\",\"memo_type\":\"id\",\"memo\":\"18446744073709551616\"").arg(account),
                                      QStringLiteral("\"account_id\":\"%1\",\"memo_type\":\"id\",\"memo\":\"-1\"").arg(account),
                                      QStringLiteral("\"account_id\":\"%1\",\"memo_type\":\"id\",\"memo\":\"+1\"").arg(account),
                                      QStringLiteral("\"account_id\":\"%1\",\"memo_type\":\"id\",\"memo\":\"abc\"").arg(account),
                                      QStringLiteral("\"account_id\":\"%1\",\"memo_type\":\"id\",\"memo\":12345678901234567").arg(account),
                                      QStringLiteral("\"account_id\":\"%1\",\"memo_type\":\"hash\",\"memo\":\"%2\"").arg(account, QString::fromLatin1(QByteArray(31, '\x07').toBase64())),
                                      QStringLiteral("\"account_id\":\"%1\",\"memo_type\":\"hash\",\"memo\":\"not base64!\"").arg(account),
                                      QStringLiteral("\"account_id\":\"%1\",\"memo_type\":\"return\",\"memo\":\"%2\"").arg(account, hash),
                                      QStringLiteral("\"account_id\":\"%1\",\"memo\":\"42\"").arg(account),
                                      QStringLiteral("\"account_id\":\"%1\",\"memo_type\":\"id\"").arg(account)}) {
            const QVariantMap out = answer(fields);
            QVERIFY2(out.contains(QStringLiteral("error")), qPrintable(fields));
            QVERIFY(!out.value(QStringLiteral("error")).toString().contains(key->getSecretSeed()));
        }
    }

    /**
     * The wallet's last step, which the SEP asks for and nothing here did:
     * simulate the signed entries and refuse a call that writes anything but
     * the challenge's own nonces before posting them back.
     */
    void testCheckSep45SimulationRefusesSideEffects()
    {
        SepsQml seps;
        QVariantMap args = pyReadArgs(pySigned());
        QVariantMap simulation;
        simulation.insert(QStringLiteral("ok"), true);
        simulation.insert(QStringLiteral("error"), QString());
        simulation.insert(QStringLiteral("returnValueXdr"), QStringLiteral("AAAAAQ=="));
        simulation.insert(QStringLiteral("auth"), authOf(pySigned()));
        // The nonces of the client's and the server's entries, and nothing else.
        simulation.insert(QStringLiteral("transactionData"), QStringLiteral(
            "AAAAAAAAAAAAAAACAAAABgAAAAFoJLDhyeGyXhpbJFrmlhPP8IpiwZ0PLQh2nUqqm7jG5wAAABXu3e8LghZ+6wAAAAAAAAAG"
            "AAAAAAAAAACk2IqM8QZFSkGDUOf7zmoZrxaiw/Zj4MPjY7Grhfb7nQAAABUAAAAAOt5osQAAAAAAD0JAAAAD6AAAAGQAAAAA"
            "AADDUA=="));
        args.insert(QStringLiteral("simulation"), simulation);
        const QVariantMap clean = seps.checkSep45Simulation(args);
        QVERIFY2(clean.value(QStringLiteral("sideEffectFree")).toBool(),
                 qPrintable(clean.value(QStringLiteral("error")).toString()));

        // The same, and a token balance written as well.
        simulation.insert(QStringLiteral("transactionData"), QStringLiteral(
            "AAAAAAAAAAAAAAADAAAABgAAAAFoJLDhyeGyXhpbJFrmlhPP8IpiwZ0PLQh2nUqqm7jG5wAAABXu3e8LghZ+6wAAAAAAAAAG"
            "AAAAAAAAAACk2IqM8QZFSkGDUOf7zmoZrxaiw/Zj4MPjY7Grhfb7nQAAABUAAAAAOt5osQAAAAAAAAAGAAAAASW0/NhZrsL6"
            "Y0hDjEibPDwQyYttIb5P08swy2iVPvl3AAAAEAAAAAEAAAACAAAADwAAAAdCYWxhbmNlAAAAABIAAAABaCSw4cnhsl4aWyRa"
            "5pYTz/CKYsGdDy0Idp1Kqpu4xucAAAABAA9CQAAAA+gAAABkAAAAAAAAw1A="));
        args.insert(QStringLiteral("simulation"), simulation);
        QVERIFY(seps.checkSep45Simulation(args).contains(QStringLiteral("error")));
        // Nothing to read is no proof of nothing written.
        simulation.remove(QStringLiteral("transactionData"));
        args.insert(QStringLiteral("simulation"), simulation);
        QVERIFY(seps.checkSep45Simulation(args).contains(QStringLiteral("error")));
        simulation.insert(QStringLiteral("ok"), false);
        simulation.insert(QStringLiteral("error"), QStringLiteral("HostError"));
        args.insert(QStringLiteral("simulation"), simulation);
        QVERIFY(seps.checkSep45Simulation(args).contains(QStringLiteral("error")));
        args.insert(QStringLiteral("simulations"), simulation);
        QVERIFY(seps.checkSep45Simulation(args).contains(QStringLiteral("error")));
    }

    /**
     * verifySep45Challenge took any successful simulation as the verdict on
     * the signed entries, the anchor's own first one included: the simulation
     * of the challenge before anybody signed it succeeds too, so a page that
     * handed that one over logged in a client that never signed. The
     * simulation must now report these very entries as its auth (a call that
     * carries its own entries is simulated with them), in any order.
     */
    void testSep45VerdictIsOnTheSimulationOfTheSignedEntries()
    {
        SepsQml seps;
        QVariantMap simulation;
        simulation.insert(QStringLiteral("ok"), true);
        simulation.insert(QStringLiteral("error"), QString());
        simulation.insert(QStringLiteral("returnValueXdr"), QStringLiteral("AAAAAQ=="));
        simulation.insert(QStringLiteral("transactionData"), QStringLiteral(
            "AAAAAAAAAAAAAAACAAAABgAAAAFoJLDhyeGyXhpbJFrmlhPP8IpiwZ0PLQh2nUqqm7jG5wAAABXu3e8LghZ+6wAAAAAAAAAG"
            "AAAAAAAAAACk2IqM8QZFSkGDUOf7zmoZrxaiw/Zj4MPjY7Grhfb7nQAAABUAAAAAOt5osQAAAAAAD0JAAAAD6AAAAGQAAAAA"
            "AADDUA=="));
        auto verdict = [&](const QVariant& auth, bool wallet) {
            QVariantMap sim = simulation;
            if (auth.isValid())
                sim.insert(QStringLiteral("auth"), auth);
            QVariantMap args = pyReadArgs(pySigned());
            args.insert(QStringLiteral("simulation"), sim);
            const QVariantMap out = wallet ? seps.checkSep45Simulation(args) : seps.verifySep45Challenge(args);
            return out.contains(QStringLiteral("error")) ? out.value(QStringLiteral("error")).toString()
                                                         : QString();
        };
        QVariantList reversed = authOf(pySigned());
        std::reverse(reversed.begin(), reversed.end());
        for (bool wallet : {false, true}) {
            QCOMPARE(verdict(authOf(pySigned()), wallet), QString());
            QCOMPARE(verdict(reversed, wallet), QString());
            QCOMPARE(verdict(QVariant(QStringList{authOf(pySigned()).at(0).toString(),
                                                  authOf(pySigned()).at(1).toString()}), wallet),
                     QString());
            // The anchor's first simulation: the challenge's entries unsigned.
            QVERIFY(!verdict(QVariant(pySimulatedAuth()), wallet).isEmpty());
            // The challenge as issued: the server's entry signed, the client's not.
            QVERIFY(!verdict(authOf(pyBuilt()), wallet).isEmpty());
            // One entry short, one too many, none reported, not a list.
            QVERIFY(!verdict(authOf(pySigned()).mid(1), wallet).isEmpty());
            QVERIFY(!verdict(authOf(pySigned()) + authOf(pySigned()).mid(0, 1), wallet).isEmpty());
            QVERIFY(!verdict(QVariantList(), wallet).isEmpty());
            QVERIFY(!verdict(QVariant(), wallet).isEmpty());
            QVERIFY(!verdict(QVariant(pySigned()), wallet).isEmpty());
        }
    }

    /**
     * deriveAccountRange normalises the passphrase to NFKD, as BIP-39 asks,
     * so a wallet created with 1.0.0-1.0.3 and a passphrase whose NFKD form
     * differs (here precomposed accents and compatibility characters) is at
     * accounts nothing in SepsQml reached. deriveAccountRangeLegacyPassphrase
     * derives them; for an ASCII passphrase, or one typed in NFKD already,
     * both give the same accounts. Expected: python hashlib
     * PBKDF2-HMAC-SHA512 over the passphrase as typed (what the 1.0.3 code
     * computes) and py stellar_sdk 14.1.0's SEP-5 keys.
     */
    void testDeriveAccountRangeLegacyPassphraseRecoversOldWallets()
    {
        SepsQml seps;
        const QString words = QStringLiteral("abandon abandon abandon abandon abandon abandon "
                                             "abandon abandon abandon abandon abandon about");
        const QString typed = QStringLiteral("P") + QChar(0x00E4) + QStringLiteral("ssw")
                              + QChar(0x00F6) + QStringLiteral("rd ") + QChar(0x334D)
                              + QStringLiteral(" ") + QChar(0xFB01) + QStringLiteral(" ")
                              + QChar(0x212B);
        const auto accountAt = [](const QVariantMap& range, int i) {
            return range.value(QStringLiteral("accounts")).toList().value(i).toMap()
                .value(QStringLiteral("accountId")).toString();
        };

        const QVariantMap legacy = seps.deriveAccountRangeLegacyPassphrase(words, 0, 2, typed);
        QCOMPARE(legacy.value(QStringLiteral("accounts")).toList().size(), 2);
        QCOMPARE(legacy.value(QStringLiteral("bip39SeedHex")).toString(),
                 QStringLiteral("8a2d3630411eef1fd1e6de429f04cb50cf0bc505bfa66275dfe5a0ed09f75117"
                                "fcb7d35c1e7a7f499b9d5af834885894bfa87f749350df4c7fc84ff9ddf67ea8"));
        QCOMPARE(accountAt(legacy, 0), QStringLiteral("GB5OYYOSOUO52LZI6TOMCJIZ726BNNZZFI3RKISAUZWFZ25MQFZLHKYZ"));
        QCOMPARE(accountAt(legacy, 1), QStringLiteral("GA4VE6QLOCX4BMGCH6JRNVNNK2BVNWTM7OGKOTITTCVYGHGOLUWEALL4"));
        QVERIFY(legacy.value(QStringLiteral("legacy")).toBool());
        QVERIFY(legacy.value(QStringLiteral("legacyPassphraseDiffers")).toBool());

        const QVariantMap bip39 = seps.deriveAccountRange(words, 0, 2, typed);
        QCOMPARE(accountAt(bip39, 0), QStringLiteral("GCKHOYVAHPJFPAE54WVDN6K42HG7ZG3AU7P46DOKRGUQYWBVHE66ARHZ"));
        QCOMPARE(accountAt(bip39, 1), QStringLiteral("GD6YKLEGPGY4GD2HC3IEDLNXLA5B7ZYVYOGBP55CJICTI7PQRAZ7OFOF"));
        QVERIFY(bip39.value(QStringLiteral("legacyPassphraseDiffers")).toBool());
        QVERIFY(!bip39.contains(QStringLiteral("legacy")));

        // Typed in NFKD already, the passphrase was read the same way then.
        const QVariantMap decomposed = seps.deriveAccountRangeLegacyPassphrase(
            words, 0, 1, typed.normalized(QString::NormalizationForm_KD));
        QCOMPARE(accountAt(decomposed, 0), accountAt(bip39, 0));
        QVERIFY(!decomposed.value(QStringLiteral("legacyPassphraseDiffers")).toBool());

        const QVariantMap ascii = seps.deriveAccountRangeLegacyPassphrase(words, 0, 1, QStringLiteral("TREZOR"));
        QCOMPARE(accountAt(ascii, 0), QStringLiteral("GBXISQLLM6OGWWZU7SCNF2GOJSQLHJU3I76P23J6IDSJGKEEGM2AJYG5"));
        QCOMPARE(accountAt(seps.deriveAccountRange(words, 0, 1, QStringLiteral("TREZOR")), 0),
                 accountAt(ascii, 0));
        QVERIFY(!ascii.value(QStringLiteral("legacyPassphraseDiffers")).toBool());

        // Failures come back as values, as everywhere in this layer.
        QVERIFY(seps.deriveAccountRangeLegacyPassphrase(QStringLiteral("bad"), 0, 1)
                    .contains(QStringLiteral("error")));
        QVERIFY(seps.deriveAccountRangeLegacyPassphrase(words, 0, 1, QStringLiteral("x") + QChar(0xD800))
                    .contains(QStringLiteral("error")));
    }

    /**
     * A phrase whose checksum fails (two words swapped, a word mistyped into
     * another one) was derived all the same: a valid-looking wallet that is
     * not the user's, shown with its secrets. py-stellar-sdk, js and
     * Stellar.keyPairFromMnemonic refuse it. The range is also read exactly:
     * the QML engine narrowed an int with ToInt32 (2^32 + 100 was 100).
     */
    void testDeriveAccountRangeRefusesABadChecksumAndNarrowedNumbers()
    {
        SepsQml seps;
        const QString words = QStringLiteral("abandon abandon abandon abandon abandon abandon "
                                             "abandon abandon abandon abandon abandon about");
        const QString badChecksum = QStringLiteral("abandon abandon abandon abandon abandon abandon "
                                                   "abandon abandon abandon abandon abandon abandon");
        QVERIFY(!seps.deriveAccountRange(words, 0, 1).contains(QStringLiteral("error")));
        const QVariantMap refused = seps.deriveAccountRange(badChecksum, 0, 2);
        QVERIFY(refused.contains(QStringLiteral("error")));
        QVERIFY(!refused.contains(QStringLiteral("accounts")));
        QVERIFY(seps.deriveAccountRangeLegacyPassphrase(badChecksum, 0, 2).contains(QStringLiteral("error")));
        QCOMPARE(seps.inspectMnemonic(badChecksum).value(QStringLiteral("valid")).toBool(), false);

        const QVariantMap hundred = seps.deriveAccountRange(words, QVariant(100.0), QVariant(1.0));
        QCOMPARE(hundred.value(QStringLiteral("accounts")).toList().value(0).toMap()
                     .value(QStringLiteral("index")).toInt(), 100);
        for (const auto& range : {qMakePair(QVariant(4294967396.0), QVariant(1)),
                                  qMakePair(QVariant(1.5), QVariant(1)),
                                  qMakePair(QVariant(0), QVariant(4294967297.0)),
                                  qMakePair(QVariant(0), QVariant(2.5)),
                                  qMakePair(QVariant(QStringLiteral("x")), QVariant(1))}) {
            QVERIFY(seps.deriveAccountRange(words, range.first, range.second).contains(QStringLiteral("error")));
            QVERIFY(seps.deriveAccountRangeLegacyPassphrase(words, range.first, range.second)
                        .contains(QStringLiteral("error")));
        }
    }

    /**
     * The index is hardened, so 2^31 - 1 is the last account. A range past
     * it overflowed the int, and its entry -2147483648 came back with
     * account 0's keys: an index with its top bit set is taken as already
     * hardened. Such a range is refused; the last account itself is derived
     * as py stellar_sdk 14.1.0 derives it.
     */
    void testDeriveAccountRangeStopsAtTheLastHardenedIndex()
    {
        SepsQml seps;
        const QString words = QStringLiteral("abandon abandon abandon abandon abandon abandon "
                                             "abandon abandon abandon abandon abandon about");
        const QVariantList last = seps.deriveAccountRange(words, 2147483646, 2)
                                      .value(QStringLiteral("accounts")).toList();
        QCOMPARE(last.size(), 2);
        QCOMPARE(last.at(0).toMap().value(QStringLiteral("accountId")).toString(),
                 QStringLiteral("GC4RJH2G3VUUQVQ4Y5AXHI442AKY5CKOVD3TLGALXJDOUHZCX5BJJ3QY"));
        QCOMPARE(last.at(1).toMap().value(QStringLiteral("accountId")).toString(),
                 QStringLiteral("GAYW4AVOGT2533ITZT6SUNUVCIQFIRFONQZN2GY3FX27LSF4K2DMNYGU"));
        QCOMPARE(last.at(1).toMap().value(QStringLiteral("path")).toString(),
                 QStringLiteral("m/44'/148'/2147483647'"));
        for (const auto& range : {qMakePair(2147483647, 2), qMakePair(2147483630, 20)}) {
            QVERIFY(seps.deriveAccountRange(words, range.first, range.second)
                        .contains(QStringLiteral("error")));
            QVERIFY(seps.deriveAccountRangeLegacyPassphrase(words, range.first, range.second)
                        .contains(QStringLiteral("error")));
        }
    }

    /**
     * Seps.txrepFromXdr refused a fee bump ("this SDK's TxRep emitter
     * handles plain transactions"), xdrFromTxrep read plain documents only
     * and handed them back unsigned, and txrepRoundTrip turned every signed
     * envelope into an unsigned one. They go through Txrep::toTxrep and
     * Txrep::fromTxrepEnvelope now: a fee bump as the fee bump (its M... fee
     * source and fee, never its inner transaction as what is signed), a V0
     * envelope as V0, signatures carried both ways and read strictly. The
     * envelopes and documents are py-stellar-sdk 14.1.0's.
     */
    void testTxrepCoversFeeBumpsAndV0Envelopes()
    {
        SepsQml seps;
        const auto call = [](const char* key, const QString& value) {
            QVariantMap args;
            args.insert(QString::fromLatin1(key), value);
            args.insert(QStringLiteral("network"), QStringLiteral("testnet"));
            return args;
        };
        const auto failure = [](const QVariantMap& out) {
            return out.value(QStringLiteral("error")).toString();
        };

        const QVariantMap shown = seps.txrepFromXdr(call("xdr", pyFeeBumpXdr()));
        QVERIFY2(!shown.contains(QStringLiteral("error")), qPrintable(failure(shown)));
        // The fee bump's own document, both signature lists in it, and it
        // reads back as this very envelope.
        const QString feeBumpTxrep = shown.value(QStringLiteral("txrep")).toString();
        QVERIFY2(feeBumpTxrep.startsWith(pyFeeBumpTxrep().section(QLatin1Char('\n'), 0, 3)),
                 qPrintable(feeBumpTxrep));
        QVERIFY(feeBumpTxrep.endsWith(pyFeeBumpTxrep().section(QLatin1Char('\n'), -8)));
        // Line for line py's document, but for the glosses py adds after a
        // time and an amount, which this emitter leaves out.
        QCOMPARE(withoutGlosses(feeBumpTxrep), withoutGlosses(pyFeeBumpTxrep()));
        QCOMPARE(seps.xdrFromTxrep(call("txrep", feeBumpTxrep)).value(QStringLiteral("xdr")).toString(),
                 pyFeeBumpXdr());
        QCOMPARE(shown.value(QStringLiteral("envelopeType")).toString(),
                 QStringLiteral("ENVELOPE_TYPE_TX_FEE_BUMP"));
        QVERIFY(shown.value(QStringLiteral("isFeeBump")).toBool());
        QCOMPARE(shown.value(QStringLiteral("hash")).toString(),
                 QStringLiteral("5b0ae1961848750e3cdd6e2aad7453832c502a8b6c589c937ac9552f4bdcf828"));
        QCOMPARE(shown.value(QStringLiteral("feeSource")).toString(),
                 QStringLiteral("MCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZIAAAAAAAAAAAA42ZW"));
        QCOMPARE(shown.value(QStringLiteral("fee")).toString(), QStringLiteral("400"));
        QCOMPARE(shown.value(QStringLiteral("signatureCount")).toInt(), 1);
        QCOMPARE(shown.value(QStringLiteral("innerHash")).toString(),
                 QStringLiteral("94d9731476424e3d6ed9cf22be801da0ef8c40961741db0fc836084acbedd8d5"));
        QCOMPARE(shown.value(QStringLiteral("innerSourceAccount")).toString(),
                 QStringLiteral("GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR"));
        QCOMPARE(shown.value(QStringLiteral("innerFee")).toString(), QStringLiteral("100"));
        QCOMPARE(shown.value(QStringLiteral("innerSignatureCount")).toInt(), 1);
        // The inner source is not the envelope's: under that name a reader
        // takes it for the account paying.
        QVERIFY(!shown.contains(QStringLiteral("sourceAccount")));

        const QVariantMap read = seps.xdrFromTxrep(call("txrep", pyFeeBumpTxrep()));
        QVERIFY2(!read.contains(QStringLiteral("error")), qPrintable(failure(read)));
        QCOMPARE(read.value(QStringLiteral("xdr")).toString(), pyFeeBumpXdr());
        QCOMPARE(read.value(QStringLiteral("hash")), shown.value(QStringLiteral("hash")));
        QCOMPARE(read.value(QStringLiteral("feeSource")), shown.value(QStringLiteral("feeSource")));
        QCOMPARE(read.value(QStringLiteral("innerSignatureCount")).toInt(), 1);

        const QVariantMap trip = seps.txrepRoundTrip(call("xdr", pyFeeBumpXdr()));
        QVERIFY(trip.value(QStringLiteral("matched")).toBool());
        QVERIFY(trip.value(QStringLiteral("bytesMatched")).toBool());
        QCOMPARE(trip.value(QStringLiteral("signaturesOut")).toInt(), 1);
        QCOMPARE(trip.value(QStringLiteral("signaturesDropped")).toInt(), 0);

        const QVariantMap v0 = seps.txrepFromXdr(call("xdr", pyV0Xdr()));
        QVERIFY2(!v0.contains(QStringLiteral("error")), qPrintable(failure(v0)));
        const QString v0Txrep = v0.value(QStringLiteral("txrep")).toString();
        QVERIFY2(v0Txrep.startsWith(pyV0Txrep().section(QLatin1Char('\n'), 0, 3)), qPrintable(v0Txrep));
        QVERIFY(v0Txrep.endsWith(pyV0Txrep().section(QLatin1Char('\n'), -5)));
        QCOMPARE(withoutGlosses(v0Txrep), withoutGlosses(pyV0Txrep()));
        QCOMPARE(seps.xdrFromTxrep(call("txrep", v0Txrep)).value(QStringLiteral("xdr")).toString(),
                 pyV0Xdr());
        QCOMPARE(v0.value(QStringLiteral("envelopeType")).toString(), QStringLiteral("ENVELOPE_TYPE_TX_V0"));
        QVERIFY(!v0.value(QStringLiteral("isFeeBump")).toBool());
        QCOMPARE(v0.value(QStringLiteral("hash")).toString(),
                 QStringLiteral("3715eaccd49cb1c4aedd626b31c1daf23197f3d2327b3cb2bf68ee8f5b4838a3"));
        QCOMPARE(v0.value(QStringLiteral("sourceAccount")).toString(),
                 QStringLiteral("GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR"));
        QCOMPARE(v0.value(QStringLiteral("sequence")).toString(), QStringLiteral("42"));
        const QVariantMap v0read = seps.xdrFromTxrep(call("txrep", pyV0Txrep()));
        QCOMPARE(v0read.value(QStringLiteral("xdr")).toString(), pyV0Xdr());
        QCOMPARE(v0read.value(QStringLiteral("signatureCount")).toInt(), 1);
        const QVariantMap v0trip = seps.txrepRoundTrip(call("xdr", pyV0Xdr()));
        QVERIFY(v0trip.value(QStringLiteral("bytesMatched")).toBool());
        QCOMPARE(v0trip.value(QStringLiteral("signaturesDropped")).toInt(), 0);

        // A signed V1 transaction comes back signed as well.
        QScopedPointer<KeyPair> signer(KeyPair::fromSecretSeed(QByteArray(32, '\x01')));
        Account account(signer->getAccountId(), 41);
        QScopedPointer<Transaction> tx(TransactionBuilder(&account, Network::testnetNetwork())
            .addOperation(PaymentOperation::create(
                QStringLiteral("GDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435DIAG"),
                new AssetTypeNative(), QStringLiteral("10.5")))
            .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
            .setBaseFee(100)
            .build());
        tx->sign(signer.data());
        const QVariantMap signedTrip = seps.txrepRoundTrip(call("xdr", tx->toEnvelopeXdrBase64()));
        QVERIFY(signedTrip.value(QStringLiteral("bytesMatched")).toBool());
        QCOMPARE(signedTrip.value(QStringLiteral("signaturesIn")).toInt(), 1);
        QCOMPARE(signedTrip.value(QStringLiteral("signaturesDropped")).toInt(), 0);

        // What the documents say of their signatures is read, not taken on
        // trust: a list longer than its entries, a hint that is not 4 bytes.
        QString longer = pyFeeBumpTxrep();
        longer.replace(QStringLiteral("feeBump.signatures.len: 1\n"), QStringLiteral("feeBump.signatures.len: 2\n"));
        QVERIFY(longer != pyFeeBumpTxrep());
        QVERIFY(seps.xdrFromTxrep(call("txrep", longer)).contains(QStringLiteral("error")));
        QString shortHint = pyV0Txrep();
        shortHint.replace(QStringLiteral("signatures[0].hint: b40f6f5c\n"), QStringLiteral("signatures[0].hint: b40f6f\n"));
        QVERIFY(shortHint != pyV0Txrep());
        QVERIFY(seps.xdrFromTxrep(call("txrep", shortHint)).contains(QStringLiteral("error")));
    }

private:
    /** A txrep document without the comments after its values: py's
     *  " (10.5)" and " (1970-01-01 00:00:00.000000+00:00 (UTC))", and the
     *  " (accountID: G..., accountMuxedID: N)" both write. */
    static QString withoutGlosses(QString txrep)
    {
        return txrep.remove(QRegularExpression(QStringLiteral(" \\(.*\\)$"),
                                               QRegularExpression::MultilineOption));
    }

    /** py-stellar-sdk 14.1.0: a payment of 10.5 XLM from
     *  GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR (seed 32 x 0x01)
     *  at sequence 42, signed by it, wrapped in a fee bump whose fee source is
     *  MuxedAccount(GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U
     *  (seed 32 x 0x02), 7) at a base fee of 200, signed by that key; testnet. */
    static QString pyFeeBumpXdr()
    {
        return QStringLiteral(
            "AAAABQAAAQAAAAAAAAAAB4E5dw6ofRdfVqNUZsNMfszLjYqRtO43ol32D1uPybOUAAAAAAAAAZAAAAACAAAAAIqI4910CfGV"
            "/VLbLTy6XXLKZwm/HZQSG/N0iAG0D29cAAAAZAAAAAAAAAAqAAAAAQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAQAAAAAAAAAB"
            "AAAAAO1JKMYo0cLG6ukDOJBZlWEpWSc6XGP5NjbBRhSshzfRAAAAAAAAAAAGQixAAAAAAAAAAAG0D29cAAAAQI2/3jIjD1H5"
            "ce57vcEOy3hOtWHXiQestw8B0DJ8m9efXeM5DdW9yUUNGrQ2O90HQN6v5DqVpjVYWoms5YsDSgwAAAAAAAAAAY/Js5QAAABA"
            "3f2Fe9cqiDtzQAeb84wyJ6MxeVbBu7J/qt69awr9Dpyr7AT6qQ2iZJSdRfhOg1DJQfvLpiv7rfaMrAV4VS7cAA==");
    }

    /** py-stellar-sdk 14.1.0's to_txrep of pyFeeBumpXdr(). */
    static QString pyFeeBumpTxrep()
    {
        return QStringLiteral(
            "type: ENVELOPE_TYPE_TX_FEE_BUMP\n"
            "feeBump.tx.feeSource: MCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZIAAAAAAAAAAAA42ZW "
            "(accountID: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U, accountMuxedID: 7)\n"
            "feeBump.tx.fee: 400\n"
            "feeBump.tx.innerTx.type: ENVELOPE_TYPE_TX\n"
            "feeBump.tx.innerTx.tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
            "feeBump.tx.innerTx.tx.fee: 100\n"
            "feeBump.tx.innerTx.tx.seqNum: 42\n"
            "feeBump.tx.innerTx.tx.cond.type: PRECOND_TIME\n"
            "feeBump.tx.innerTx.tx.cond.timeBounds.minTime: 0 (1970-01-01 00:00:00.000000+00:00 (UTC))\n"
            "feeBump.tx.innerTx.tx.cond.timeBounds.maxTime: 0 (1970-01-01 00:00:00.000000+00:00 (UTC))\n"
            "feeBump.tx.innerTx.tx.memo.type: MEMO_NONE\n"
            "feeBump.tx.innerTx.tx.operations.len: 1\n"
            "feeBump.tx.innerTx.tx.operations[0].sourceAccount._present: false\n"
            "feeBump.tx.innerTx.tx.operations[0].body.type: PAYMENT\n"
            "feeBump.tx.innerTx.tx.operations[0].body.paymentOp.destination: "
            "GDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435DIAG\n"
            "feeBump.tx.innerTx.tx.operations[0].body.paymentOp.asset: native\n"
            "feeBump.tx.innerTx.tx.operations[0].body.paymentOp.amount: 105000000 (10.5)\n"
            "feeBump.tx.innerTx.tx.ext.v: 0\n"
            "feeBump.tx.innerTx.signatures.len: 1\n"
            "feeBump.tx.innerTx.signatures[0].hint: b40f6f5c\n"
            "feeBump.tx.innerTx.signatures[0].signature: 8dbfde32230f51f971ee7bbdc10ecb784eb561d78907acb70f01"
            "d0327c9bd79f5de3390dd5bdc9450d1ab4363bdd0740deafe43a95a635585a89ace58b034a0c\n"
            "feeBump.tx.ext.v: 0\n"
            "feeBump.signatures.len: 1\n"
            "feeBump.signatures[0].hint: 8fc9b394\n"
            "feeBump.signatures[0].signature: ddfd857bd72a883b7340079bf38c3227a3317956c1bbb27faadebd6b0afd0e9c"
            "abec04faa90da264949d45f84e8350c941fbcba62bfbadf68cac0578552edc00\n");
    }

    /** py-stellar-sdk 14.1.0, built with v1=False: the same payment as a V0
     *  envelope, time bounds 0 to 1700000000, text memo "v0 memo", signed. */
    static QString pyV0Xdr()
    {
        return QStringLiteral(
            "AAAAAIqI4910CfGV/VLbLTy6XXLKZwm/HZQSG/N0iAG0D29cAAAAZAAAAAAAAAAqAAAAAQAAAAAAAAAAAAAAAGVT8QAAAAAB"
            "AAAAB3YwIG1lbW8AAAAAAQAAAAAAAAABAAAAAO1JKMYo0cLG6ukDOJBZlWEpWSc6XGP5NjbBRhSshzfRAAAAAAAAAAAGQixA"
            "AAAAAAAAAAG0D29cAAAAQAA0JBydFq75kf//2uWARGv7Mc4va3iifTCOf8v52Yjp73eMZ/eyk3/o0TiWmoiddDGtGydVyttN"
            "HDuv9ULWlQs=");
    }

    /** py-stellar-sdk 14.1.0's to_txrep of pyV0Xdr(). */
    static QString pyV0Txrep()
    {
        return QStringLiteral(
            "type: ENVELOPE_TYPE_TX_V0\n"
            "tx.sourceAccount: GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR\n"
            "tx.fee: 100\n"
            "tx.seqNum: 42\n"
            "tx.cond.type: PRECOND_TIME\n"
            "tx.cond.timeBounds.minTime: 0 (1970-01-01 00:00:00.000000+00:00 (UTC))\n"
            "tx.cond.timeBounds.maxTime: 1700000000 (2023-11-14 22:13:20.000000+00:00 (UTC))\n"
            "tx.memo.type: MEMO_TEXT\n"
            "tx.memo.text: \"v0 memo\"\n"
            "tx.operations.len: 1\n"
            "tx.operations[0].sourceAccount._present: false\n"
            "tx.operations[0].body.type: PAYMENT\n"
            "tx.operations[0].body.paymentOp.destination: GDWUSKGGFDI4FRXK5EBTRECZSVQSSWJHHJOGH6JWG3AUMFFMQ435DIAG\n"
            "tx.operations[0].body.paymentOp.asset: native\n"
            "tx.operations[0].body.paymentOp.amount: 105000000 (10.5)\n"
            "tx.ext.v: 0\n"
            "signatures.len: 1\n"
            "signatures[0].hint: b40f6f5c\n"
            "signatures[0].signature: 0034241c9d16aef991ffffdae580446bfb31ce2f6b78a27d308e7fcbf9d988e9"
            "ef778c67f7b2937fe8d138969a889d7431ad1b2755cadb4d1c3baff542d6950b\n");
    }

    /** A challenge's entries one by one, in base64: how Soroban.simulate
     *  reports the authorizations a call ran with. */
    static QVariantList authOf(const QString& entries)
    {
        QByteArray data = QByteArray::fromBase64(entries.toLatin1());
        QDataStream in(&data, QIODevice::ReadOnly);
        qint32 count = 0;
        in >> count;
        QVariantList out;
        for (qint32 i = 0; i < count; ++i) {
            stellar::SorobanAuthorizationEntry entry;
            in >> entry;
            QByteArray one;
            QDataStream write(&one, QIODevice::WriteOnly);
            write << entry;
            out.append(QString::fromLatin1(one.toBase64()));
        }
        return out;
    }

    static QString CLIENT_CONTRACT()   { return QStringLiteral("CDLZFC3SYJYDZT7K67VZ75HPJVIEUVNIXF47ZG2FB2RMQQVU2HHGCYSC"); }
    static QString WEB_AUTH_CONTRACT() { return QStringLiteral("CAS3J7GYLGXMF6TDJBBYYSE3HQ6BBSMLNUQ34T6TZMYMW2EVH34XOWMA"); }

    static QVariantMap sep45Args(KeyPair* server, const QString& homeDomain = HOME())
    {
        QVariantMap args;
        args.insert(QStringLiteral("serverSeed"), server->getSecretSeed());
        args.insert(QStringLiteral("clientAccountId"), CLIENT_CONTRACT());
        args.insert(QStringLiteral("webAuthContract"), WEB_AUTH_CONTRACT());
        args.insert(QStringLiteral("homeDomain"), homeDomain);
        args.insert(QStringLiteral("webAuthDomain"), WEB_AUTH());
        args.insert(QStringLiteral("latestLedger"), 1000);
        args.insert(QStringLiteral("network"), QStringLiteral("testnet"));
        return args;
    }

    static QVariantMap inspectArgs(const QString& entries, KeyPair* server)
    {
        QVariantMap args;
        args.insert(QStringLiteral("authorizationEntries"), entries);
        args.insert(QStringLiteral("serverAccountId"), server->getAccountId());
        args.insert(QStringLiteral("homeDomain"), HOME());
        args.insert(QStringLiteral("webAuthDomain"), WEB_AUTH());
        args.insert(QStringLiteral("webAuthContract"), WEB_AUTH_CONTRACT());
        args.insert(QStringLiteral("network"), QStringLiteral("testnet"));
        return args;
    }

    // py-stellar-sdk 14.1.0's SEP-45 v0.1.1, without a client domain: the
    // keys, nonce and ledger fixed, the simulation stubbed (Sep45ChallengeTest
    // has the full set and how it was made).
    static QString PY_SERVER() { return QStringLiteral("GCSNRCUM6EDEKSSBQNIOP66ONIM26FVCYP3GHYGD4NR3DK4F635Z32WQ"); }
    static QString PY_CLIENT() { return QStringLiteral("CBUCJMHBZHQ3EXQ2LMSFVZUWCPH7BCTCYGOQ6LIIO2OUVKU3XDDOO2HN"); }
    static QString PY_WEB_AUTH() { return QStringLiteral("CCSZMK2C2B7UVP3Q4JRWLS3JY7ZB4YTTRSWQXGWQVS24W3PPZXOUHH4R"); }
    static QString PY_NONCE() { return QStringLiteral("0Q7tFfp5hL2oJv1n9mY3sR8aZc6WkX4bUe0dGiT/+ypoQ1lNwVrE5Ks7jHuMf2Ax"); }

    static QVariantMap pyIssueArgs()
    {
        QVariantMap args;
        args.insert(QStringLiteral("serverSeed"), QStringLiteral("SC7UDDRCWZQXXCJS6I44TK54WYA4X4Z7FXNMV3D66P4TC2VX7GYVKP36"));
        args.insert(QStringLiteral("clientAccountId"), PY_CLIENT());
        args.insert(QStringLiteral("webAuthContract"), PY_WEB_AUTH());
        args.insert(QStringLiteral("homeDomain"), HOME());
        args.insert(QStringLiteral("webAuthDomain"), WEB_AUTH());
        args.insert(QStringLiteral("nonce"), PY_NONCE());
        args.insert(QStringLiteral("latestLedger"), 82106);
        args.insert(QStringLiteral("network"), QStringLiteral("testnet"));
        return args;
    }

    static QVariantMap pyReadArgs(const QString& entries)
    {
        QVariantMap args;
        args.insert(QStringLiteral("authorizationEntries"), entries);
        args.insert(QStringLiteral("serverAccountId"), PY_SERVER());
        args.insert(QStringLiteral("homeDomains"), QVariantList{HOME()});
        args.insert(QStringLiteral("webAuthDomain"), WEB_AUTH());
        args.insert(QStringLiteral("webAuthContract"), PY_WEB_AUTH());
        args.insert(QStringLiteral("network"), QStringLiteral("testnet"));
        return args;
    }

    static QString pyChallengeTx() {
        return QStringLiteral(
            "AAAAAgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAGQAAAAAAAAAAQAAAAEAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAEAAAAAAAAAGAAAAAAAAAABpZYrQtB/Sr9w4mNly2nH8h5ic4ytC5rQrLXLbe/N3UMAAAAPd2ViX2F1dGhfdmVy"
            "aWZ5AAAAAAEAAAARAAAAAQAAAAUAAAAPAAAAB2FjY291bnQAAAAADgAAADhDQlVDSk1IQlpIUTNFWFEyTE1TRlZaVVdDUEg3"
            "QkNUQ1lHT1E2TElJTzJPVVZLVTNYRERPTzJITgAAAA8AAAALaG9tZV9kb21haW4AAAAADgAAAAtleGFtcGxlLmNvbQAAAAAP"
            "AAAABW5vbmNlAAAAAAAADgAAAEAwUTd0RmZwNWhMMm9KdjFuOW1ZM3NSOGFaYzZXa1g0YlVlMGRHaVQvK3lwb1ExbE53VnJF"
            "NUtzN2pIdU1mMkF4AAAADwAAAA93ZWJfYXV0aF9kb21haW4AAAAADgAAABBhdXRoLmV4YW1wbGUuY29tAAAADwAAABd3ZWJf"
            "YXV0aF9kb21haW5fYWNjb3VudAAAAAAOAAAAOEdDU05SQ1VNNkVERUtTU0JRTklPUDY2T05JTTI2RlZDWVAzR0hZR0Q0TlIz"
            "REs0RjYzNVozMldRAAAAAAAAAAAAAAAA");
    }

    static QStringList pySimulatedAuth() {
        return QStringList{
            QStringLiteral("AAAAAQAAAAFoJLDhyeGyXhpbJFrmlhPP8IpiwZ0PLQh2nUqqm7jG5+7d7wuCFn7rAAAAAAAAAAEAAAAAAAAAAaWWK0LQf0q/"
                           "cOJjZctpx/IeYnOMrQua0Ky1y23vzd1DAAAAD3dlYl9hdXRoX3ZlcmlmeQAAAAABAAAAEQAAAAEAAAAFAAAADwAAAAdhY2Nv"
                           "dW50AAAAAA4AAAA4Q0JVQ0pNSEJaSFEzRVhRMkxNU0ZWWlVXQ1BIN0JDVENZR09RNkxJSU8yT1VWS1UzWERET08ySE4AAAAP"
                           "AAAAC2hvbWVfZG9tYWluAAAAAA4AAAALZXhhbXBsZS5jb20AAAAADwAAAAVub25jZQAAAAAAAA4AAABAMFE3dEZmcDVoTDJv"
                           "SnYxbjltWTNzUjhhWmM2V2tYNGJVZTBkR2lULyt5cG9RMWxOd1ZyRTVLczdqSHVNZjJBeAAAAA8AAAAPd2ViX2F1dGhfZG9t"
                           "YWluAAAAAA4AAAAQYXV0aC5leGFtcGxlLmNvbQAAAA8AAAAXd2ViX2F1dGhfZG9tYWluX2FjY291bnQAAAAADgAAADhHQ1NO"
                           "UkNVTTZFREVLU1NCUU5JT1A2Nk9OSU0yNkZWQ1lQM0dIWUdENE5SM0RLNEY2MzVaMzJXUQAAAAA="),
            QStringLiteral("AAAAAQAAAAAAAAAApNiKjPEGRUpBg1Dn+85qGa8WosP2Y+DD42Oxq4X2+50AAAAAOt5osQAAAAAAAAABAAAAAAAAAAGllitC"
                           "0H9Kv3DiY2XLacfyHmJzjK0LmtCstctt783dQwAAAA93ZWJfYXV0aF92ZXJpZnkAAAAAAQAAABEAAAABAAAABQAAAA8AAAAH"
                           "YWNjb3VudAAAAAAOAAAAOENCVUNKTUhCWkhRM0VYUTJMTVNGVlpVV0NQSDdCQ1RDWUdPUTZMSUlPMk9VVktVM1hERE9PMkhO"
                           "AAAADwAAAAtob21lX2RvbWFpbgAAAAAOAAAAC2V4YW1wbGUuY29tAAAAAA8AAAAFbm9uY2UAAAAAAAAOAAAAQDBRN3RGZnA1"
                           "aEwyb0p2MW45bVkzc1I4YVpjNldrWDRiVWUwZEdpVC8reXBvUTFsTndWckU1S3M3akh1TWYyQXgAAAAPAAAAD3dlYl9hdXRo"
                           "X2RvbWFpbgAAAAAOAAAAEGF1dGguZXhhbXBsZS5jb20AAAAPAAAAF3dlYl9hdXRoX2RvbWFpbl9hY2NvdW50AAAAAA4AAAA4"
                           "R0NTTlJDVU02RURFS1NTQlFOSU9QNjZPTklNMjZGVkNZUDNHSFlHRDROUjNESzRGNjM1WjMyV1EAAAAA"),
        };
    }

    static QString pyBuilt() {
        return QStringLiteral(
            "AAAAAgAAAAEAAAABaCSw4cnhsl4aWyRa5pYTz/CKYsGdDy0Idp1Kqpu4xufu3e8LghZ+6wAAAAAAAAABAAAAAAAAAAGllitC"
            "0H9Kv3DiY2XLacfyHmJzjK0LmtCstctt783dQwAAAA93ZWJfYXV0aF92ZXJpZnkAAAAAAQAAABEAAAABAAAABQAAAA8AAAAH"
            "YWNjb3VudAAAAAAOAAAAOENCVUNKTUhCWkhRM0VYUTJMTVNGVlpVV0NQSDdCQ1RDWUdPUTZMSUlPMk9VVktVM1hERE9PMkhO"
            "AAAADwAAAAtob21lX2RvbWFpbgAAAAAOAAAAC2V4YW1wbGUuY29tAAAAAA8AAAAFbm9uY2UAAAAAAAAOAAAAQDBRN3RGZnA1"
            "aEwyb0p2MW45bVkzc1I4YVpjNldrWDRiVWUwZEdpVC8reXBvUTFsTndWckU1S3M3akh1TWYyQXgAAAAPAAAAD3dlYl9hdXRo"
            "X2RvbWFpbgAAAAAOAAAAEGF1dGguZXhhbXBsZS5jb20AAAAPAAAAF3dlYl9hdXRoX2RvbWFpbl9hY2NvdW50AAAAAA4AAAA4"
            "R0NTTlJDVU02RURFS1NTQlFOSU9QNjZPTklNMjZGVkNZUDNHSFlHRDROUjNESzRGNjM1WjMyV1EAAAAAAAAAAQAAAAAAAAAA"
            "pNiKjPEGRUpBg1Dn+85qGa8WosP2Y+DD42Oxq4X2+50AAAAAOt5osQABQW4AAAAQAAAAAQAAAAEAAAARAAAAAQAAAAIAAAAP"
            "AAAACnB1YmxpY19rZXkAAAAAAA0AAAAgpNiKjPEGRUpBg1Dn+85qGa8WosP2Y+DD42Oxq4X2+50AAAAPAAAACXNpZ25hdHVy"
            "ZQAAAAAAAA0AAABApp9+7UOIFrH7urEsTG/+SoWMQdqnpbLRffMDGHfA/nj6w5En34oDbGoWCPCJUQrYgpaTuYnU0H3mTCi7"
            "mgxzCgAAAAAAAAABpZYrQtB/Sr9w4mNly2nH8h5ic4ytC5rQrLXLbe/N3UMAAAAPd2ViX2F1dGhfdmVyaWZ5AAAAAAEAAAAR"
            "AAAAAQAAAAUAAAAPAAAAB2FjY291bnQAAAAADgAAADhDQlVDSk1IQlpIUTNFWFEyTE1TRlZaVVdDUEg3QkNUQ1lHT1E2TElJ"
            "TzJPVVZLVTNYRERPTzJITgAAAA8AAAALaG9tZV9kb21haW4AAAAADgAAAAtleGFtcGxlLmNvbQAAAAAPAAAABW5vbmNlAAAA"
            "AAAADgAAAEAwUTd0RmZwNWhMMm9KdjFuOW1ZM3NSOGFaYzZXa1g0YlVlMGRHaVQvK3lwb1ExbE53VnJFNUtzN2pIdU1mMkF4"
            "AAAADwAAAA93ZWJfYXV0aF9kb21haW4AAAAADgAAABBhdXRoLmV4YW1wbGUuY29tAAAADwAAABd3ZWJfYXV0aF9kb21haW5f"
            "YWNjb3VudAAAAAAOAAAAOEdDU05SQ1VNNkVERUtTU0JRTklPUDY2T05JTTI2RlZDWVAzR0hZR0Q0TlIzREs0RjYzNVozMldR"
            "AAAAAA==");
    }

    static QString pySigned() {
        return QStringLiteral(
            "AAAAAgAAAAEAAAABaCSw4cnhsl4aWyRa5pYTz/CKYsGdDy0Idp1Kqpu4xufu3e8LghZ+6wABQRgAAAAQAAAAAQAAAAEAAAAR"
            "AAAAAQAAAAIAAAAPAAAACnB1YmxpY19rZXkAAAAAAA0AAAAgoJql9HpnWYAv+VX43C0qFKXJnSO+l/hkEn/5ODRVpPAAAAAP"
            "AAAACXNpZ25hdHVyZQAAAAAAAA0AAABAaH0lmbS7DL5PMT1Snr2KDd4yRDVqvAywAgPaDBrcM6CHHYztxCSWuu5eiNP7Gkb5"
            "xIhDpRMG7Sa+PmgFXtBoDAAAAAAAAAABpZYrQtB/Sr9w4mNly2nH8h5ic4ytC5rQrLXLbe/N3UMAAAAPd2ViX2F1dGhfdmVy"
            "aWZ5AAAAAAEAAAARAAAAAQAAAAUAAAAPAAAAB2FjY291bnQAAAAADgAAADhDQlVDSk1IQlpIUTNFWFEyTE1TRlZaVVdDUEg3"
            "QkNUQ1lHT1E2TElJTzJPVVZLVTNYRERPTzJITgAAAA8AAAALaG9tZV9kb21haW4AAAAADgAAAAtleGFtcGxlLmNvbQAAAAAP"
            "AAAABW5vbmNlAAAAAAAADgAAAEAwUTd0RmZwNWhMMm9KdjFuOW1ZM3NSOGFaYzZXa1g0YlVlMGRHaVQvK3lwb1ExbE53VnJF"
            "NUtzN2pIdU1mMkF4AAAADwAAAA93ZWJfYXV0aF9kb21haW4AAAAADgAAABBhdXRoLmV4YW1wbGUuY29tAAAADwAAABd3ZWJf"
            "YXV0aF9kb21haW5fYWNjb3VudAAAAAAOAAAAOEdDU05SQ1VNNkVERUtTU0JRTklPUDY2T05JTTI2RlZDWVAzR0hZR0Q0TlIz"
            "REs0RjYzNVozMldRAAAAAAAAAAEAAAAAAAAAAKTYiozxBkVKQYNQ5/vOahmvFqLD9mPgw+NjsauF9vudAAAAADreaLEAAUFu"
            "AAAAEAAAAAEAAAABAAAAEQAAAAEAAAACAAAADwAAAApwdWJsaWNfa2V5AAAAAAANAAAAIKTYiozxBkVKQYNQ5/vOahmvFqLD"
            "9mPgw+NjsauF9vudAAAADwAAAAlzaWduYXR1cmUAAAAAAAANAAAAQKaffu1DiBax+7qxLExv/kqFjEHap6Wy0X3zAxh3wP54"
            "+sORJ9+KA2xqFgjwiVEK2IKWk7mJ1NB95kwou5oMcwoAAAAAAAAAAaWWK0LQf0q/cOJjZctpx/IeYnOMrQua0Ky1y23vzd1D"
            "AAAAD3dlYl9hdXRoX3ZlcmlmeQAAAAABAAAAEQAAAAEAAAAFAAAADwAAAAdhY2NvdW50AAAAAA4AAAA4Q0JVQ0pNSEJaSFEz"
            "RVhRMkxNU0ZWWlVXQ1BIN0JDVENZR09RNkxJSU8yT1VWS1UzWERET08ySE4AAAAPAAAAC2hvbWVfZG9tYWluAAAAAA4AAAAL"
            "ZXhhbXBsZS5jb20AAAAADwAAAAVub25jZQAAAAAAAA4AAABAMFE3dEZmcDVoTDJvSnYxbjltWTNzUjhhWmM2V2tYNGJVZTBk"
            "R2lULyt5cG9RMWxOd1ZyRTVLczdqSHVNZjJBeAAAAA8AAAAPd2ViX2F1dGhfZG9tYWluAAAAAA4AAAAQYXV0aC5leGFtcGxl"
            "LmNvbQAAAA8AAAAXd2ViX2F1dGhfZG9tYWluX2FjY291bnQAAAAADgAAADhHQ1NOUkNVTTZFREVLU1NCUU5JT1A2Nk9OSU0y"
            "NkZWQ1lQM0dIWUdENE5SM0RLNEY2MzVaMzJXUQAAAAA=");
    }

    static QString pyVerifyTx() {
        return QStringLiteral(
            "AAAAAgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAGQAAAAAAAAAAQAAAAEAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAEAAAAAAAAAGAAAAAAAAAABpZYrQtB/Sr9w4mNly2nH8h5ic4ytC5rQrLXLbe/N3UMAAAAPd2ViX2F1dGhfdmVy"
            "aWZ5AAAAAAEAAAARAAAAAQAAAAUAAAAPAAAAB2FjY291bnQAAAAADgAAADhDQlVDSk1IQlpIUTNFWFEyTE1TRlZaVVdDUEg3"
            "QkNUQ1lHT1E2TElJTzJPVVZLVTNYRERPTzJITgAAAA8AAAALaG9tZV9kb21haW4AAAAADgAAAAtleGFtcGxlLmNvbQAAAAAP"
            "AAAABW5vbmNlAAAAAAAADgAAAEAwUTd0RmZwNWhMMm9KdjFuOW1ZM3NSOGFaYzZXa1g0YlVlMGRHaVQvK3lwb1ExbE53VnJF"
            "NUtzN2pIdU1mMkF4AAAADwAAAA93ZWJfYXV0aF9kb21haW4AAAAADgAAABBhdXRoLmV4YW1wbGUuY29tAAAADwAAABd3ZWJf"
            "YXV0aF9kb21haW5fYWNjb3VudAAAAAAOAAAAOEdDU05SQ1VNNkVERUtTU0JRTklPUDY2T05JTTI2RlZDWVAzR0hZR0Q0TlIz"
            "REs0RjYzNVozMldRAAAAAgAAAAEAAAABaCSw4cnhsl4aWyRa5pYTz/CKYsGdDy0Idp1Kqpu4xufu3e8LghZ+6wABQRgAAAAQ"
            "AAAAAQAAAAEAAAARAAAAAQAAAAIAAAAPAAAACnB1YmxpY19rZXkAAAAAAA0AAAAgoJql9HpnWYAv+VX43C0qFKXJnSO+l/hk"
            "En/5ODRVpPAAAAAPAAAACXNpZ25hdHVyZQAAAAAAAA0AAABAaH0lmbS7DL5PMT1Snr2KDd4yRDVqvAywAgPaDBrcM6CHHYzt"
            "xCSWuu5eiNP7Gkb5xIhDpRMG7Sa+PmgFXtBoDAAAAAAAAAABpZYrQtB/Sr9w4mNly2nH8h5ic4ytC5rQrLXLbe/N3UMAAAAP"
            "d2ViX2F1dGhfdmVyaWZ5AAAAAAEAAAARAAAAAQAAAAUAAAAPAAAAB2FjY291bnQAAAAADgAAADhDQlVDSk1IQlpIUTNFWFEy"
            "TE1TRlZaVVdDUEg3QkNUQ1lHT1E2TElJTzJPVVZLVTNYRERPTzJITgAAAA8AAAALaG9tZV9kb21haW4AAAAADgAAAAtleGFt"
            "cGxlLmNvbQAAAAAPAAAABW5vbmNlAAAAAAAADgAAAEAwUTd0RmZwNWhMMm9KdjFuOW1ZM3NSOGFaYzZXa1g0YlVlMGRHaVQv"
            "K3lwb1ExbE53VnJFNUtzN2pIdU1mMkF4AAAADwAAAA93ZWJfYXV0aF9kb21haW4AAAAADgAAABBhdXRoLmV4YW1wbGUuY29t"
            "AAAADwAAABd3ZWJfYXV0aF9kb21haW5fYWNjb3VudAAAAAAOAAAAOEdDU05SQ1VNNkVERUtTU0JRTklPUDY2T05JTTI2RlZD"
            "WVAzR0hZR0Q0TlIzREs0RjYzNVozMldRAAAAAAAAAAEAAAAAAAAAAKTYiozxBkVKQYNQ5/vOahmvFqLD9mPgw+NjsauF9vud"
            "AAAAADreaLEAAUFuAAAAEAAAAAEAAAABAAAAEQAAAAEAAAACAAAADwAAAApwdWJsaWNfa2V5AAAAAAANAAAAIKTYiozxBkVK"
            "QYNQ5/vOahmvFqLD9mPgw+NjsauF9vudAAAADwAAAAlzaWduYXR1cmUAAAAAAAANAAAAQKaffu1DiBax+7qxLExv/kqFjEHa"
            "p6Wy0X3zAxh3wP54+sORJ9+KA2xqFgjwiVEK2IKWk7mJ1NB95kwou5oMcwoAAAAAAAAAAaWWK0LQf0q/cOJjZctpx/IeYnOM"
            "rQua0Ky1y23vzd1DAAAAD3dlYl9hdXRoX3ZlcmlmeQAAAAABAAAAEQAAAAEAAAAFAAAADwAAAAdhY2NvdW50AAAAAA4AAAA4"
            "Q0JVQ0pNSEJaSFEzRVhRMkxNU0ZWWlVXQ1BIN0JDVENZR09RNkxJSU8yT1VWS1UzWERET08ySE4AAAAPAAAAC2hvbWVfZG9t"
            "YWluAAAAAA4AAAALZXhhbXBsZS5jb20AAAAADwAAAAVub25jZQAAAAAAAA4AAABAMFE3dEZmcDVoTDJvSnYxbjltWTNzUjhh"
            "WmM2V2tYNGJVZTBkR2lULyt5cG9RMWxOd1ZyRTVLczdqSHVNZjJBeAAAAA8AAAAPd2ViX2F1dGhfZG9tYWluAAAAAA4AAAAQ"
            "YXV0aC5leGFtcGxlLmNvbQAAAA8AAAAXd2ViX2F1dGhfZG9tYWluX2FjY291bnQAAAAADgAAADhHQ1NOUkNVTTZFREVLU1NC"
            "UU5JT1A2Nk9OSU0yNkZWQ1lQM0dIWUdENE5SM0RLNEY2MzVaMzJXUQAAAAAAAAAAAAAAAA==");
    }

private slots:
    /**
     * Seps.sep29Inspect counts any memo as Server does, id 0 and an empty
     * text included (js and py: memo type not none). It showed both as not
     * satisfying the check.
     */
    void testSep29InspectAnyMemoCounts()
    {
        SepsQml seps;
        const QString passphrase = Network::testnetNetwork()->getNetworkPassphrase();
        const auto satisfies = [&](Memo* memo) {
            Account account(KeyPair::fromAccountId(QStringLiteral("GDSBCQO34HWPGUGQSP3QBFEXVTSR2PW46UIGTHVWGWJGQKH3AFNHXHXN")), 1);
            TransactionBuilder builder(&account, Network::testnetNetwork());
            builder.addOperation(PaymentOperation::create(
                       KeyPair::fromAccountId(QStringLiteral("GCMDQXJJGQE6TJ5XUHJMJUUIWECC5S6VANRAOWIQMMV4ALW43JOY2SEB")),
                       new AssetTypeNative(), QStringLiteral("10")))
                   .setTimeout(TransactionBuilder::TIMEOUT_INFINITE).setBaseFee(100);
            if (memo)
                builder.addMemo(memo);
            QScopedPointer<Transaction> tx(builder.build());
            const QVariantMap out = seps.sep29Inspect(tx->toEnvelopeXdrBase64(), passphrase);
            return out.value(QStringLiteral("memoSatisfies")).toBool();
        };
        QVERIFY(!satisfies(nullptr));
        QVERIFY(satisfies(Memo::id(0)));
        QVERIFY(satisfies(Memo::text("")));
        QVERIFY(satisfies(Memo::id(7)));
    }
};

ADD_TEST(SepsQmlTest)
#endif // SEPSQMLTEST_H
