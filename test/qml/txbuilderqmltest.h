#ifndef TXBUILDERQMLTEST_H
#define TXBUILDERQMLTEST_H

#include <QObject>
#include <QtTest>
#include "../testcollector.h"

#include "../../src/qml/stellarqml.h"
#include "../../src/qml/txbuilderqml.h"
#include "../../src/qml/sepsqml.h"
#include "../../src/stellaraddress.h"
#include "../../src/scval/scv.h"
#include "../../src/invokehostfunctionoperation.h"
#include "../../src/setoptionsoperation.h"
#include "../../src/managedataoperation.h"
#include <QJSEngine>
#include <QQmlEngine>
#include <limits>
#include "../../src/account.h"
#include "../../src/keypair.h"
#include "../../src/network.h"
#include "../../src/memo.h"
#include "../../src/transaction.h"
#include "../../src/abstracttransaction.h"
#include "../../src/transactionbuilder.h"
#include "../../src/bumpsequenceoperation.h"

/**
 * The build / sign / inspect contract of the QML layer.
 *
 * These are pinned here rather than left to the app because two of them are
 * invisible from inside the app: a transaction that carries the wrong sequence
 * number and a signature made for the wrong network both look perfectly fine
 * until a real network rejects them.
 */
class TxBuilderQmlTest: public QObject
{
    Q_OBJECT

    static const char* SOURCE() { return "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"; }
    static const char* DEST()   { return "GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"; }
    static const char* SECRET() { return "SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"; }

    /** A minimal valid payment spec at the given sequence. */
    static QVariantMap paymentSpec(const QString& sequence)
    {
        QVariantMap op;
        op.insert(QStringLiteral("type"), QStringLiteral("payment"));
        op.insert(QStringLiteral("destination"), QString::fromLatin1(DEST()));
        op.insert(QStringLiteral("asset"), QStringLiteral("native"));
        op.insert(QStringLiteral("amount"), QStringLiteral("10.5"));

        QVariantMap spec;
        spec.insert(QStringLiteral("sourceAccount"), QString::fromLatin1(SOURCE()));
        spec.insert(QStringLiteral("sequence"), sequence);
        spec.insert(QStringLiteral("operations"), QVariantList{op});
        return spec;
    }

private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    // ─── Sequence numbers ────────────────────────────────────────────

    /**
     * The sequence handed to build() is the one in the envelope.
     *
     * TransactionBuilder takes an Account and calls
     * getIncrementedSequenceNumber() on it, so the obvious implementation
     * produces a transaction one ahead of what the caller asked for. Every
     * submission then dies on tx_bad_seq, and nothing offline reveals it:
     * the XDR is well-formed, the hash is real, the TxRep reads correctly.
     */
    void testSequenceIsUsedVerbatim()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());

        const QStringList sequences = {
            QStringLiteral("1"), QStringLiteral("42"),
            // Beyond 2^53: the value only survives because it travels as text.
            QStringLiteral("103720918407103"), QStringLiteral("18145399306977281")
        };
        for (const QString& sequence : sequences) {
            const QVariantMap built = builder.build(paymentSpec(sequence));
            QVERIFY2(!built.contains(QStringLiteral("error")),
                     qPrintable(built.value(QStringLiteral("error")).toString()));

            QVariantMap args;
            args.insert(QStringLiteral("xdr"), built.value(QStringLiteral("xdr")));
            const QVariantMap seen = builder.inspect(args);
            QCOMPARE(seen.value(QStringLiteral("sequence")).toString(), sequence);
        }
    }

    /** A sequence below 1 cannot appear on the ledger, so it is refused here. */
    void testSequenceBelowOneIsRejected()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());

        QVERIFY(builder.build(paymentSpec(QStringLiteral("0")))
                    .contains(QStringLiteral("error")));
        QVERIFY(builder.build(paymentSpec(QStringLiteral("-5")))
                    .contains(QStringLiteral("error")));
        QVERIFY(builder.build(paymentSpec(QStringLiteral("not a number")))
                    .contains(QStringLiteral("error")));
    }

    // ─── Signing ─────────────────────────────────────────────────────

    void testSignAddsASignatureWithoutMovingTheHash()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());
        const QVariantMap built = builder.build(paymentSpec(QStringLiteral("1")));

        QVariantMap args;
        args.insert(QStringLiteral("xdr"), built.value(QStringLiteral("xdr")));
        args.insert(QStringLiteral("seeds"), QVariantList{QString::fromLatin1(SECRET())});
        const QVariantMap signed_ = builder.sign(args);

        QVERIFY(!signed_.contains(QStringLiteral("error")));
        QCOMPARE(signed_.value(QStringLiteral("signatureCount")).toInt(), 1);
        QCOMPARE(signed_.value(QStringLiteral("added")).toInt(), 1);
        QVERIFY(signed_.value(QStringLiteral("xdr")).toString()
                != built.value(QStringLiteral("xdr")).toString());

        // The hash covers the transaction, not the envelope. If adding a
        // signature moved it, a second signer would be endorsing different
        // bytes than the first one did.
        QCOMPARE(signed_.value(QStringLiteral("hash")).toString(),
                 built.value(QStringLiteral("hash")).toString());
    }

    /**
     * sign() and inspect() read only the keys they know, and ignored the
     * rest: a misspelt "netwrok" or "networkPasphrase" left the default,
     * testnet, so a pubnet envelope was signed (and its hash shown) for
     * testnet without a word. build() already refused unknown keys; now every
     * call that takes a map does.
     */
    void testSignAndInspectRefuseUnknownKeys()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());
        QVariantMap spec = paymentSpec(QStringLiteral("1"));
        spec.insert(QStringLiteral("network"), QStringLiteral("public"));
        const QVariantMap built = builder.build(spec);
        QVERIFY2(!built.contains(QStringLiteral("error")), qPrintable(built.value(QStringLiteral("error")).toString()));

        QVariantMap args;
        args.insert(QStringLiteral("xdr"), built.value(QStringLiteral("xdr")));
        args.insert(QStringLiteral("seeds"), QVariantList{QString::fromLatin1(SECRET())});
        args.insert(QStringLiteral("network"), QStringLiteral("public"));
        const QVariantMap good = builder.sign(args);
        QVERIFY(!good.contains(QStringLiteral("error")));
        QCOMPARE(good.value(QStringLiteral("hash")), built.value(QStringLiteral("hash")));

        for (const QString& typo : {QStringLiteral("netwrok"), QStringLiteral("networkPasphrase"),
                                    QStringLiteral("seed")}) {
            QVariantMap misspelt = args;
            misspelt.remove(QStringLiteral("network"));
            misspelt.insert(typo, QStringLiteral("public"));
            const QVariantMap refused = builder.sign(misspelt);
            QVERIFY2(refused.contains(QStringLiteral("error")), qPrintable(typo));
            QVERIFY(refused.value(QStringLiteral("error")).toString().contains(typo));
            QVERIFY(!refused.contains(QStringLiteral("hash")));
            misspelt.remove(QStringLiteral("seeds"));
            QVERIFY2(builder.inspect(misspelt).contains(QStringLiteral("error")), qPrintable(typo));
        }

        SepsQml seps;
        QVariantMap toTxrep;
        toTxrep.insert(QStringLiteral("xdr"), built.value(QStringLiteral("xdr")));
        toTxrep.insert(QStringLiteral("network"), QStringLiteral("public"));
        QVERIFY(!seps.txrepFromXdr(toTxrep).contains(QStringLiteral("error")));
        QVERIFY(!seps.txrepRoundTrip(toTxrep).contains(QStringLiteral("error")));
        toTxrep.insert(QStringLiteral("netwrok"), QStringLiteral("public"));
        QVERIFY(seps.txrepFromXdr(toTxrep).contains(QStringLiteral("error")));
        QVERIFY(seps.txrepRoundTrip(toTxrep).contains(QStringLiteral("error")));

        // toidEncode reads its numbers exactly: 1000000 as a JS number (which
        // Qt 6 printed as "1e+06" and then refused) is 1000000.
        QVariantMap toid;
        toid.insert(QStringLiteral("ledger"), QVariant(1000000.0));
        toid.insert(QStringLiteral("txIndex"), QVariant(1.0));
        toid.insert(QStringLiteral("opIndex"), QStringLiteral("1"));
        const QVariantMap packed = seps.toidEncode(toid);
        QVERIFY2(!packed.contains(QStringLiteral("error")), qPrintable(packed.value(QStringLiteral("error")).toString()));
        toid.insert(QStringLiteral("opIndex"), QVariant(1.5));
        QVERIFY(seps.toidEncode(toid).contains(QStringLiteral("error")));
        toid.insert(QStringLiteral("opIndex"), 1);
        toid.insert(QStringLiteral("ledgr"), 1);
        QVERIFY(seps.toidEncode(toid).contains(QStringLiteral("error")));
    }

    /**
     * An amount given as a JS number was read with QVariant::toString(): from
     * 2^29 units on, where one double stands for several seven-decimal
     * amounts, a nearby amount was signed (536870912.1234005 as
     * 536870912.1234004), and 1e11 or 1e-7 were refused as "1e+11" /
     * "1e-07". A number is read now when it is exact, and gives the same
     * transaction as the text; otherwise it is refused, asking for text.
     */
    void testAmountsGivenAsNumbersAreExactOrRefused()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());
        auto payment = [&](const QVariant& amount) {
            QVariantMap spec = paymentSpec(QStringLiteral("1"));
            QVariantList ops = spec.value(QStringLiteral("operations")).toList();
            QVariantMap op = ops.at(0).toMap();
            op.insert(QStringLiteral("amount"), amount);
            spec.insert(QStringLiteral("operations"), QVariantList{op});
            // No time bound from the clock: two builds a second apart had
            // different hashes, and the comparison below failed now and then.
            spec.insert(QStringLiteral("timeout"), 0);
            return builder.build(spec);
        };
        const QList<QPair<double, QString>> same = {
            {10.5, QStringLiteral("10.5")},
            {0.1, QStringLiteral("0.1")},
            {1e-7, QStringLiteral("0.0000001")},
            {100.0000001, QStringLiteral("100.0000001")},
            {536870911.1234567, QStringLiteral("536870911.1234567")},
            {536870912.0, QStringLiteral("536870912")},
            {1e11, QStringLiteral("100000000000")},
        };
        for (const auto& pair : same) {
            const QVariantMap fromNumber = payment(QVariant(pair.first));
            const QVariantMap fromText = payment(pair.second);
            QVERIFY2(!fromNumber.contains(QStringLiteral("error")),
                     qPrintable(pair.second + QStringLiteral(": ") + fromNumber.value(QStringLiteral("error")).toString()));
            QVERIFY2(!fromText.contains(QStringLiteral("error")), qPrintable(pair.second));
            QCOMPARE(fromNumber.value(QStringLiteral("hash")), fromText.value(QStringLiteral("hash")));
        }
        for (const double refused : {536870912.1234005, 700000000.1234011, 922337203685.4775807,
                                     9007199254740992.0, std::numeric_limits<double>::infinity(),
                                     std::numeric_limits<double>::quiet_NaN()}) {
            QVERIFY2(payment(QVariant(refused)).contains(QStringLiteral("error")),
                     qPrintable(QString::number(refused, 'g', 17)));
        }
        // As text, the largest amount is exact.
        QVERIFY(!payment(QStringLiteral("922337203685.4775807")).contains(QStringLiteral("error")));
    }

    /**
     * invokeContract had no u128/i128/u256/i256 argument: a token transfer
     * (amount: i128), the commonest Soroban call, could not be built, and the
     * "raw JSON" way out the documentation offered read the same types. They
     * are read now from exact decimal text (or an exact integer), and the
     * transaction is the one js-stellar-sdk 17.1.0 builds with
     * nativeToScVal(BigInt(v), {type}). Anything that does not fit is refused.
     */
    void testInvokeContractTakesBigIntegers()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());
        const QString source = QStringLiteral("GBO5RIS4HALBCS7BQGCGOO5KIJDGLHYASOOKSYXKJPISKEO7SDDWJYK5");
        const QString sac = QStringLiteral("CDLZFC3SYJYDZT7K67VZ75HPJVIEUVNIXF47ZG2FB2RMQQVU2HHGCYSC");
        auto argument = [](const QString& type, const QVariant& value) {
            QVariantMap a;
            a.insert(QStringLiteral("type"), type);
            if (value.isValid())
                a.insert(QStringLiteral("value"), value);
            return QVariant(a);
        };
        auto invoke = [&](const QString& function, const QVariant& args) {
            QVariantMap op;
            op.insert(QStringLiteral("type"), QStringLiteral("invokeContract"));
            op.insert(QStringLiteral("contractId"), sac);
            op.insert(QStringLiteral("functionName"), function);
            op.insert(QStringLiteral("args"), args);
            QVariantMap spec;
            spec.insert(QStringLiteral("network"), QStringLiteral("testnet"));
            spec.insert(QStringLiteral("sourceAccount"), source);
            spec.insert(QStringLiteral("sequence"), QStringLiteral("4294967297"));
            spec.insert(QStringLiteral("timeout"), 0);
            spec.insert(QStringLiteral("operations"), QVariantList{op});
            return builder.build(spec);
        };
        const QVariantMap transfer = invoke(QStringLiteral("transfer"), QVariantList{
            argument(QStringLiteral("address"), source), argument(QStringLiteral("address"), sac),
            argument(QStringLiteral("i128"), QStringLiteral("170141183460469231731687303715884105727"))});
        QVERIFY2(!transfer.contains(QStringLiteral("error")), qPrintable(transfer.value(QStringLiteral("error")).toString()));
        QCOMPARE(transfer.value(QStringLiteral("hash")).toString(),
                 QStringLiteral("0e63b1eb3130f689ff005670744ff50fc09a3bb85bb72258780bfadda58ad249"));

        const QVariantMap numbers = invoke(QStringLiteral("f"), QVariantList{
            argument(QStringLiteral("i128"), QVariant(100.0)),
            argument(QStringLiteral("u64"), QVariant(9007199254740991.0)),
            argument(QStringLiteral("i256"), QStringLiteral("-1")),
            argument(QStringLiteral("u256"), QStringLiteral("1")),
            argument(QStringLiteral("string"), QStringLiteral(" x ")),
            argument(QStringLiteral("bool"), true),
            argument(QStringLiteral("void"), QVariant())});
        QVERIFY2(!numbers.contains(QStringLiteral("error")), qPrintable(numbers.value(QStringLiteral("error")).toString()));
        QCOMPARE(numbers.value(QStringLiteral("hash")).toString(),
                 QStringLiteral("a90067717be9191762e76e01f1499c336a043e1494812417ec894450024f6ab9"));

        const QVariantList bounds{
            argument(QStringLiteral("i128"), QStringLiteral("-170141183460469231731687303715884105728")),
            argument(QStringLiteral("u128"), QStringLiteral("340282366920938463463374607431768211455")),
            argument(QStringLiteral("i256"),
                     QStringLiteral("-57896044618658097711785492504343953926634992332820282019728792003956564819968")),
            argument(QStringLiteral("u256"),
                     QStringLiteral("115792089237316195423570985008687907853269984665640564039457584007913129639935")),
            argument(QStringLiteral("i128"), QStringLiteral("-1")),
            argument(QStringLiteral("u128"), QStringLiteral("0"))};
        const QVariantMap edges = invoke(QStringLiteral("f"), bounds);
        QVERIFY2(!edges.contains(QStringLiteral("error")), qPrintable(edges.value(QStringLiteral("error")).toString()));
        QCOMPARE(edges.value(QStringLiteral("hash")).toString(),
                 QStringLiteral("76c9ab36fb6fa7c61dcc3357a353c4460cb80fdc9f0b23c3a9962d90b71a8659"));
        // The same list as JSON text reads the same.
        QVariantList jsonArgs;
        QString json = QStringLiteral("[");
        for (int i = 0; i < bounds.size(); ++i) {
            const QVariantMap a = bounds.at(i).toMap();
            json += QStringLiteral("%1{\"type\":\"%2\",\"value\":\"%3\"}")
                        .arg(i ? QStringLiteral(",") : QString(), a.value(QStringLiteral("type")).toString(),
                             a.value(QStringLiteral("value")).toString());
        }
        json += QStringLiteral("]");
        QCOMPARE(invoke(QStringLiteral("f"), json).value(QStringLiteral("hash")), edges.value(QStringLiteral("hash")));

        const QList<QPair<QString, QVariant>> refused = {
            {QStringLiteral("i128"), QStringLiteral("170141183460469231731687303715884105728")},
            {QStringLiteral("i128"), QStringLiteral("-170141183460469231731687303715884105729")},
            {QStringLiteral("u128"), QStringLiteral("340282366920938463463374607431768211456")},
            {QStringLiteral("u128"), QStringLiteral("-1")},
            {QStringLiteral("u256"), QStringLiteral("-1")},
            {QStringLiteral("i128"), QStringLiteral("-0")},
            {QStringLiteral("i128"), QStringLiteral("+1")},
            {QStringLiteral("i128"), QStringLiteral("1.5")},
            {QStringLiteral("i128"), QStringLiteral("1e3")},
            {QStringLiteral("i128"), QStringLiteral("0x10")},
            {QStringLiteral("i128"), QStringLiteral("")},
            {QStringLiteral("i128"), QVariant(1.5)},
            {QStringLiteral("i128"), QVariant(9007199254740992.0)},
            {QStringLiteral("i128"), QVariant(true)},
        };
        for (const auto& bad : refused) {
            const QVariantMap out = invoke(QStringLiteral("f"), QVariantList{argument(bad.first, bad.second)});
            QVERIFY2(out.contains(QStringLiteral("error")), qPrintable(bad.first + QStringLiteral(" ") + bad.second.toString()));
        }
    }

    /**
     * manageData with value "" (text, hex or base64) signed a DELETE of the
     * entry: only a value left out deletes it now, and "" is an empty value,
     * as js and py sign it (op XDR AAAAAAAAAAoAAAABawAAAAAAAAEAAAAA from
     * js-stellar-sdk 17.1.0) and core keeps it. Payload text is not trimmed
     * any more: " lead" named the entry "lead", and a home domain "  " was
     * signed as "", which clears the account's.
     */
    void testManageDataAndPayloadTextAreSignedAsGiven()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());
        auto firstOp = [&](const QVariantMap& op) {
            QVariantMap spec = paymentSpec(QStringLiteral("1"));
            spec.insert(QStringLiteral("operations"), QVariantList{op});
            const QVariantMap built = builder.build(spec);
            if (built.contains(QStringLiteral("error")))
                return QStringLiteral("error: ") + built.value(QStringLiteral("error")).toString();
            QScopedPointer<AbstractTransaction> tx(AbstractTransaction::fromEnvelopeXdr(
                built.value(QStringLiteral("xdr")).toString(), Network::testnetNetwork()));
            return dynamic_cast<Transaction*>(tx.data())->getOperations().first()->toXdrBase64();
        };
        auto manageData = [](const QVariant& name, const QVariant& value, const QString& format = QString()) {
            QVariantMap op;
            op.insert(QStringLiteral("type"), QStringLiteral("manageData"));
            op.insert(QStringLiteral("name"), name);
            if (value.isValid())
                op.insert(QStringLiteral("value"), value);
            if (!format.isEmpty())
                op.insert(QStringLiteral("valueFormat"), format);
            return op;
        };
        auto decoded = [](const QString& base64) {
            QByteArray raw = QByteArray::fromBase64(base64.toLatin1());
            QDataStream in(&raw, QIODevice::ReadOnly);
            stellar::Operation xdr;
            in >> xdr;
            return Operation::fromXdr(xdr);
        };
        const QString emptyValue = QStringLiteral("AAAAAAAAAAoAAAABawAAAAAAAAEAAAAA");
        QCOMPARE(firstOp(manageData(QStringLiteral("k"), QStringLiteral(""))), emptyValue);
        QCOMPARE(firstOp(manageData(QStringLiteral("k"), QStringLiteral(""), QStringLiteral("hex"))), emptyValue);
        QCOMPARE(firstOp(manageData(QStringLiteral("k"), QStringLiteral(""), QStringLiteral("base64"))), emptyValue);
        // Left out: the delete (dataValue absent).
        QCOMPARE(firstOp(manageData(QStringLiteral("k"), QVariant())), QStringLiteral("AAAAAAAAAAoAAAABawAAAAAAAAA="));

        // " lead" is its own entry: 5 bytes, not "lead".
        QScopedPointer<Operation> lead(decoded(firstOp(manageData(QStringLiteral(" lead"), QStringLiteral("v")))));
        QCOMPARE(dynamic_cast<ManageDataOperation*>(lead.data())->getName(), QStringLiteral(" lead"));
        QScopedPointer<Operation> spaces(decoded(firstOp(manageData(QStringLiteral("   "), QVariant()))));
        QCOMPARE(dynamic_cast<ManageDataOperation*>(spaces.data())->getName(), QStringLiteral("   "));
        QVERIFY(firstOp(manageData(QStringLiteral(""), QStringLiteral("v"))).startsWith(QStringLiteral("error: ")));

        QVariantMap options;
        options.insert(QStringLiteral("type"), QStringLiteral("setOptions"));
        options.insert(QStringLiteral("homeDomain"), QStringLiteral("  "));
        QScopedPointer<Operation> domain(decoded(firstOp(options)));
        QCOMPARE(dynamic_cast<SetOptionsOperation*>(domain.data())->getHomeDomainBytes(), QByteArray("  "));

        QVariantMap revoke;
        revoke.insert(QStringLiteral("type"), QStringLiteral("revokeDataSponsorship"));
        revoke.insert(QStringLiteral("accountId"), QString::fromLatin1(SOURCE()));
        revoke.insert(QStringLiteral("dataName"), QStringLiteral("trail "));
        const QString revoked = firstOp(revoke);
        QVERIFY2(!revoked.startsWith(QStringLiteral("error")), qPrintable(revoked));
        QVERIFY(QByteArray::fromBase64(revoked.toLatin1()).contains(QByteArray("trail ")));

        // A symbol or function name with spaces is refused, not trimmed.
        QVariantMap invoke;
        invoke.insert(QStringLiteral("type"), QStringLiteral("invokeContract"));
        invoke.insert(QStringLiteral("contractId"), QStringLiteral("CDLZFC3SYJYDZT7K67VZ75HPJVIEUVNIXF47ZG2FB2RMQQVU2HHGCYSC"));
        invoke.insert(QStringLiteral("functionName"), QStringLiteral(" balance"));
        invoke.insert(QStringLiteral("args"), QVariantList{});
        QVERIFY(firstOp(invoke).startsWith(QStringLiteral("error: ")));
        QVariantMap symbol;
        symbol.insert(QStringLiteral("type"), QStringLiteral("symbol"));
        symbol.insert(QStringLiteral("value"), QStringLiteral("abc "));
        invoke.insert(QStringLiteral("functionName"), QStringLiteral("balance"));
        invoke.insert(QStringLiteral("args"), QVariantList{symbol});
        QVERIFY(firstOp(invoke).startsWith(QStringLiteral("error: ")));
    }

    /**
     * revokeTrustlineSponsorship could not name a pool-share trustline (js
     * takes a LiquidityPoolId as the asset, py a LiquidityPoolId, the C++ API
     * create(account, poolId)). liquidityPoolId names it now; the operation is
     * js-stellar-sdk 17.1.0's, byte for byte. Exactly one of asset and
     * liquidityPoolId.
     */
    void testRevokePoolShareTrustlineSponsorship()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());
        auto build = [&](const QVariantMap& fields) {
            QVariantMap op = fields;
            op.insert(QStringLiteral("type"), QStringLiteral("revokeTrustlineSponsorship"));
            op.insert(QStringLiteral("accountId"), QStringLiteral("GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H"));
            QVariantMap spec = paymentSpec(QStringLiteral("1"));
            spec.insert(QStringLiteral("operations"), QVariantList{op});
            return builder.build(spec);
        };
        const QString pool = QStringLiteral("e590400b7fdb37ac6eb79fa6ddf4f7842b1d679a8b9d515f47c724def2bac1b3");
        QVariantMap byPool;
        byPool.insert(QStringLiteral("liquidityPoolId"), pool);
        byPool.insert(QStringLiteral("asset"), QStringLiteral(""));   // a blank form field is not set
        const QVariantMap built = build(byPool);
        QVERIFY2(!built.contains(QStringLiteral("error")), qPrintable(built.value(QStringLiteral("error")).toString()));
        QScopedPointer<AbstractTransaction> tx(AbstractTransaction::fromEnvelopeXdr(
            built.value(QStringLiteral("xdr")).toString(), Network::testnetNetwork()));
        QCOMPARE(dynamic_cast<Transaction*>(tx.data())->getOperations().first()->toXdrBase64(),
                 QStringLiteral("AAAAAAAAABIAAAAAAAAAAQAAAABi/B0L0JGythwN1lY0aypo19NHxvLCyO5tBEcCVvwF9wAAAAPlkEALf9s3rG63n6bd9PeEKx1nmoudUV9HxyTe8rrBsw=="));

        QVariantMap both = byPool;
        both.insert(QStringLiteral("asset"), QStringLiteral("USDC:GBBD47IF6LWK7P7MDEVSCWR7DPUWV3NY3DTQEVFL4NAT4AQH3ZLLFLA5"));
        QVERIFY(build(both).contains(QStringLiteral("error")));
        QVERIFY(build(QVariantMap()).contains(QStringLiteral("error")));
        QVariantMap credit;
        credit.insert(QStringLiteral("asset"), QStringLiteral("USDC:GBBD47IF6LWK7P7MDEVSCWR7DPUWV3NY3DTQEVFL4NAT4AQH3ZLLFLA5"));
        QVERIFY(!build(credit).contains(QStringLiteral("error")));
        QVariantMap strkey;
        strkey.insert(QStringLiteral("liquidityPoolId"), QStringLiteral("LAFHVLFOTNB7SNCJQGCVM3JKQZPPSPKPJRCIRRQNBBPVWJUMSSMCL6QH"));
        QVERIFY(build(strkey).contains(QStringLiteral("error")));   // as js: hex only
    }

    /**
     * createContract without constructor arguments signed the v1 host
     * function, where js (createCustomContract) and py always sign
     * CREATE_CONTRACT_V2 with an empty list: another hash and another auth
     * tree for the same deploy. It is v2 now, js-stellar-sdk 17.1.0's
     * operation byte for byte, with or without an empty constructorArgs.
     */
    void testCreateContractIsV2AsJsSignsIt()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());
        auto opOf = [&](const QVariant& ctor) {
            QVariantMap op;
            op.insert(QStringLiteral("type"), QStringLiteral("createContract"));
            op.insert(QStringLiteral("fromAddress"), QStringLiteral("GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H"));
            op.insert(QStringLiteral("wasmHash"), QString::fromLatin1(QByteArray(32, char(0xab)).toHex()));
            op.insert(QStringLiteral("salt"), QString(64, QLatin1Char('0')));
            if (ctor.isValid())
                op.insert(QStringLiteral("constructorArgs"), ctor);
            QVariantMap spec = paymentSpec(QStringLiteral("1"));
            spec.insert(QStringLiteral("operations"), QVariantList{op});
            const QVariantMap built = builder.build(spec);
            if (built.contains(QStringLiteral("error")))
                return QStringLiteral("error: ") + built.value(QStringLiteral("error")).toString();
            QScopedPointer<AbstractTransaction> tx(AbstractTransaction::fromEnvelopeXdr(
                built.value(QStringLiteral("xdr")).toString(), Network::testnetNetwork()));
            return dynamic_cast<Transaction*>(tx.data())->getOperations().first()->toXdrBase64();
        };
        const QString js = QStringLiteral(
            "AAAAAAAAABgAAAADAAAAAAAAAAAAAAAAYvwdC9CRsrYcDdZWNGsqaNfTR8bywsjubQRHAlb8BfcAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAACrq6urq6urq6urq6urq6urq6urq6urq6urq6urq6urqwAAAAAAAAAA");
        QCOMPARE(opOf(QVariant()), js);
        QCOMPARE(opOf(QVariantList{}), js);
        QCOMPARE(opOf(QStringLiteral("[]")), js);
    }

    /** A negative bumpTo (which core refuses) was signed, although the field
     *  says "not negative". */
    void testBumpSequenceIsNotNegative()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());
        auto build = [&](const QVariant& to) {
            QVariantMap op;
            op.insert(QStringLiteral("type"), QStringLiteral("bumpSequence"));
            op.insert(QStringLiteral("bumpTo"), to);
            QVariantMap spec = paymentSpec(QStringLiteral("1"));
            spec.insert(QStringLiteral("operations"), QVariantList{op});
            return builder.build(spec);
        };
        QVERIFY(fails(build(QStringLiteral("-1"))));
        QVERIFY(fails(build(QStringLiteral("-9223372036854775808"))));
        QVERIFY(!fails(build(QStringLiteral("0"))));
        QVERIFY(!fails(build(QStringLiteral("9223372036854775807"))));
    }

    /**
     * A claimable-balance (B...) or liquidity-pool (L...) address as a
     * contract argument was signed; the host refuses them as input (CAP-67
     * made them event-only), so the transaction failed on chain with its fee
     * paid. js refuses them; so does this, at any depth, in the C++ builder
     * and in Tx.build.
     */
    void testEventOnlyAddressesAreNoContractArguments()
    {
        const QString b = QStringLiteral("BAABAPLCKSTNSS5MZAXIEKEFDBPVNRU4W6M6YUJEYCVEAXRYNF2RKG6D2A");
        const QString l = QStringLiteral("LAFHVLFOTNB7SNCJQGCVM3JKQZPPSPKPJRCIRRQNBBPVWJUMSSMCL6QH");
        const StellarAddress sac = StellarAddress::fromString(QStringLiteral("CDLZFC3SYJYDZT7K67VZ75HPJVIEUVNIXF47ZG2FB2RMQQVU2HHGCYSC"));
        for (const QString& address : {b, l}) {
            QVERIFY_EXCEPTION_THROWN(delete InvokeHostFunctionOperation::invokeContractFunctionOperationBuilder(
                sac, QStringLiteral("f"), {Scv::toAddress(address)}), std::runtime_error);
            QVERIFY_EXCEPTION_THROWN(delete InvokeHostFunctionOperation::invokeContractFunctionOperationBuilder(
                sac, QStringLiteral("f"), {Scv::toVec({Scv::toUint32(quint32(1)), Scv::toAddress(address)})}), std::runtime_error);
        }
        delete InvokeHostFunctionOperation::invokeContractFunctionOperationBuilder(
            sac, QStringLiteral("f"), {Scv::toAddress(QString::fromLatin1(SOURCE())), Scv::toAddress(sac.toString())});

        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());
        auto invoke = [&](const QString& address) {
            QVariantMap arg;
            arg.insert(QStringLiteral("type"), QStringLiteral("address"));
            arg.insert(QStringLiteral("value"), address);
            QVariantMap op;
            op.insert(QStringLiteral("type"), QStringLiteral("invokeContract"));
            op.insert(QStringLiteral("contractId"), sac.toString());
            op.insert(QStringLiteral("functionName"), QStringLiteral("balance"));
            op.insert(QStringLiteral("args"), QVariantList{arg});
            QVariantMap spec = paymentSpec(QStringLiteral("1"));
            spec.insert(QStringLiteral("operations"), QVariantList{op});
            return builder.build(spec);
        };
        QVERIFY(invoke(b).contains(QStringLiteral("error")));
        QVERIFY(invoke(l).contains(QStringLiteral("error")));
        QVERIFY(!invoke(QString::fromLatin1(SOURCE())).contains(QStringLiteral("error")));
    }

    /**
     * The same key signing again adds nothing: a second copy of its signature
     * is an extra signature, and the network refuses the transaction for it
     * (txBAD_AUTH_EXTRA). It used to be appended.
     */
    void testSigningTwiceWithTheSameKeyAddsNothing()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());
        const QVariantMap built = builder.build(paymentSpec(QStringLiteral("1")));

        QVariantMap first;
        first.insert(QStringLiteral("xdr"), built.value(QStringLiteral("xdr")));
        first.insert(QStringLiteral("seeds"), QVariantList{QString::fromLatin1(SECRET())});
        const QVariantMap once = builder.sign(first);

        QVariantMap second;
        second.insert(QStringLiteral("xdr"), once.value(QStringLiteral("xdr")));
        second.insert(QStringLiteral("seeds"), QVariantList{QString::fromLatin1(SECRET())});
        const QVariantMap twice = builder.sign(second);

        QCOMPARE(once.value(QStringLiteral("signatureCount")).toInt(), 1);
        QCOMPARE(twice.value(QStringLiteral("signatureCount")).toInt(), 1);
        QCOMPARE(twice.value(QStringLiteral("added")).toInt(), 0);
        QCOMPARE(twice.value(QStringLiteral("xdr")).toString(),
                 once.value(QStringLiteral("xdr")).toString());
    }

    /** A bad seed comes back as a value; QML cannot catch an exception. */
    void testSignReportsBadInputAsAValue()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());
        const QVariantMap built = builder.build(paymentSpec(QStringLiteral("1")));

        QVariantMap badSeed;
        badSeed.insert(QStringLiteral("xdr"), built.value(QStringLiteral("xdr")));
        badSeed.insert(QStringLiteral("seeds"), QVariantList{QStringLiteral("not a seed")});
        QVERIFY(builder.sign(badSeed).contains(QStringLiteral("error")));

        QVariantMap noSeeds;
        noSeeds.insert(QStringLiteral("xdr"), built.value(QStringLiteral("xdr")));
        noSeeds.insert(QStringLiteral("seeds"), QVariantList{});
        QVERIFY(builder.sign(noSeeds).contains(QStringLiteral("error")));

        QVariantMap badXdr;
        badXdr.insert(QStringLiteral("xdr"), QStringLiteral("not base64 xdr"));
        badXdr.insert(QStringLiteral("seeds"), QVariantList{QString::fromLatin1(SECRET())});
        QVERIFY(builder.sign(badXdr).contains(QStringLiteral("error")));
    }

    /**
     * A signature is bound to a network, because the signing payload begins
     * with the hash of the network passphrase. The same bytes therefore hash
     * differently depending on which network they are read as — which is why
     * an envelope signed for testnet is worthless on the public network rather
     * than dangerous.
     */
    void testTheSameBytesHashDifferentlyPerNetwork()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());
        const QVariantMap built = builder.build(paymentSpec(QStringLiteral("1")));

        QVariantMap asPublic;
        asPublic.insert(QStringLiteral("xdr"), built.value(QStringLiteral("xdr")));
        asPublic.insert(QStringLiteral("network"), QStringLiteral("public"));

        QVERIFY(builder.inspect(asPublic).value(QStringLiteral("hash")).toString()
                != built.value(QStringLiteral("hash")).toString());
    }

    // ─── Inspecting ──────────────────────────────────────────────────

    void testInspectRecoversWhatBuildProduced()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());
        const QVariantMap built = builder.build(paymentSpec(QStringLiteral("77")));

        QVariantMap args;
        args.insert(QStringLiteral("xdr"), built.value(QStringLiteral("xdr")));
        const QVariantMap seen = builder.inspect(args);

        QCOMPARE(seen.value(QStringLiteral("hash")).toString(),
                 built.value(QStringLiteral("hash")).toString());
        QCOMPARE(seen.value(QStringLiteral("sourceAccount")).toString(),
                 QString::fromLatin1(SOURCE()));
        QCOMPARE(seen.value(QStringLiteral("sequence")).toString(), QStringLiteral("77"));
        QCOMPARE(seen.value(QStringLiteral("operationCount")).toInt(), 1);
        QCOMPARE(seen.value(QStringLiteral("signatureCount")).toInt(), 0);
        QCOMPARE(seen.value(QStringLiteral("isFeeBump")).toBool(), false);
        QVERIFY(seen.value(QStringLiteral("txrep")).toString()
                    .contains(QStringLiteral("PAYMENT")));
    }

    // ─── Money fields: what was typed, or an error ───────────────────
    //
    // Every value below used to be converted with QVariant::toUInt(),
    // toLongLong() or toInt() and the failure flag dropped. Each wrong one
    // built, signed and reported no error.

    static QVariantMap withKey(QVariantMap spec, const char* key, const QVariant& value)
    {
        spec.insert(QString::fromLatin1(key), value);
        return spec;
    }

    static bool fails(const QVariantMap& result)
    {
        return result.contains(QStringLiteral("error"))
               && !result.contains(QStringLiteral("xdr"));
    }

    /** A negative base fee wrapped to 4294967291 stroops per operation. */
    void testBaseFeeIsAWholeNumberOfStroopsOrAnError()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());
        const QVariantMap spec = paymentSpec(QStringLiteral("5"));

        const QVariantList bad = {
            -5, QStringLiteral("-5"), 1.5, QStringLiteral("abc"), QStringLiteral(""),
            QStringLiteral("12abc"), qlonglong(4294967296LL), 1e10
        };
        for (const QVariant& fee : bad) {
            const QVariantMap r = builder.build(withKey(spec, "baseFee", fee));
            QVERIFY2(fails(r), qPrintable(QStringLiteral("baseFee accepted: ") + fee.toString()));
        }

        QCOMPARE(builder.build(spec).value(QStringLiteral("fee")).toLongLong(), 100LL);
        // Under the network minimum is what a fee bump's inner transaction
        // carries (0 commonly); js and py build it. It was refused below 100.
        QCOMPARE(builder.build(withKey(spec, "baseFee", 0)).value(QStringLiteral("fee")).toLongLong(), 0LL);
        QCOMPARE(builder.build(withKey(spec, "baseFee", 99)).value(QStringLiteral("fee")).toLongLong(), 99LL);
        QCOMPARE(builder.build(withKey(spec, "baseFee", 250))
                     .value(QStringLiteral("fee")).toLongLong(), 250LL);
        QCOMPARE(builder.build(withKey(spec, "baseFee", QStringLiteral("4294967295")))
                     .value(QStringLiteral("fee")).toLongLong(), 4294967295LL);
    }

    /**
     * The envelope's fee field is a uint32. A total above it used to be signed
     * truncated while build() reported the full amount.
     */
    void testATotalFeeTheEnvelopeCannotHoldIsRefused()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());
        QVariantMap spec = paymentSpec(QStringLiteral("5"));
        QVariantList ops = spec.value(QStringLiteral("operations")).toList();
        ops.append(ops.first());
        spec.insert(QStringLiteral("operations"), ops);

        QVERIFY(fails(builder.build(withKey(spec, "baseFee", QStringLiteral("4294967295")))));
        QCOMPARE(builder.build(withKey(spec, "baseFee", QStringLiteral("2147483647")))
                     .value(QStringLiteral("fee")).toLongLong(), 4294967294LL);
    }

    /** A blank timeout used to become TIMEOUT_INFINITE: no expiry at all. */
    void testNoTimeoutIsInfiniteUnlessAskedFor()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());
        const QVariantMap spec = paymentSpec(QStringLiteral("5"));

        for (const QVariant& t : QVariantList{QStringLiteral(""), QStringLiteral("3m"),
                                              QStringLiteral("abc"), -1, 1.5}) {
            QVERIFY2(fails(builder.build(withKey(spec, "timeout", t))),
                     qPrintable(QStringLiteral("timeout accepted: ") + t.toString()));
        }

        const auto condition = [&](const QVariantMap& s) {
            const QString txrep = builder.build(s).value(QStringLiteral("txrep")).toString();
            return txrep.contains(QStringLiteral("tx.cond.type: PRECOND_NONE"))
                       ? QStringLiteral("none")
                       : txrep.contains(QStringLiteral("tx.cond.timeBounds.maxTime: 0 ("))
                             ? QStringLiteral("never")
                             : txrep.contains(QStringLiteral("tx.cond.type: PRECOND_TIME"))
                                   ? QStringLiteral("time") : QStringLiteral("?");
        };
        QCOMPARE(condition(spec), QStringLiteral("time"));                     // default 180 s
        QCOMPARE(condition(withKey(spec, "timeout", 60)), QStringLiteral("time"));
        // Asked for, 0 is time bounds {0, 0}, as js-stellar-sdk writes it. It
        // was PRECOND_NONE: the same transaction with a different hash.
        QCOMPARE(condition(withKey(spec, "timeout", 0)), QStringLiteral("never"));
        QCOMPARE(condition(withKey(spec, "timeout", QStringLiteral("0"))), QStringLiteral("never"));
    }

    /** The declared resource fee was read with toLongLong(), which took
     *  "+5000" and "-0" that baseFee refuses. It is read as baseFee is. */
    void testTheResourceFeeIsReadAsTheBaseFeeIs()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());
        const QString contract = QStringLiteral("CDLZFC3SYJYDZT7K67VZ75HPJVIEUVNIXF47ZG2FB2RMQQVU2HHGCYSC");
        const auto fee = [&](const QString& resourceFee) {
            const QVariantMap r = builder.footprintForContract(
                contract, QStringLiteral("persistent"), resourceFee);
            return r.contains(QStringLiteral("error"))
                       ? QStringLiteral("refused")
                       : r.value(QStringLiteral("resourceFee")).toString();
        };
        for (const QString& bad : {QStringLiteral("+5000"), QStringLiteral("+0"),
                                   QStringLiteral("-0"), QStringLiteral("-00"),
                                   QStringLiteral("-1"), QStringLiteral("5000abc"),
                                   QStringLiteral(""), QStringLiteral("4294967296")}) {
            QVERIFY2(fee(bad) == QStringLiteral("refused"), qPrintable(bad));
        }
        QCOMPARE(fee(QStringLiteral("5000")), QStringLiteral("5000"));
        QCOMPARE(fee(QStringLiteral("0")), QStringLiteral("0"));
        QCOMPARE(fee(QStringLiteral("4294967295")), QStringLiteral("4294967295"));
        QCOMPARE(builder.footprintForContract(contract).value(QStringLiteral("resourceFee")).toString(),
                 QStringLiteral("0"));
        // Not only text: a JS number -0 or an array was turned into text
        // ("0", "5000") first and read.
        QVariantList list;
        list << 5000;
        for (const QVariant& bad : {QVariant(-0.0), QVariant(list), QVariant(true), QVariant(1.5)}) {
            QVERIFY2(builder.footprintForContract(contract, QStringLiteral("persistent"), bad)
                         .contains(QStringLiteral("error")), qPrintable(bad.toString()));
        }
        QCOMPARE(builder.footprintForContract(contract, QStringLiteral("persistent"), QVariant(5000))
                     .value(QStringLiteral("resourceFee")).toString(), QStringLiteral("5000"));
    }

    static QVariantMap sellOfferSpec(const QVariantMap& priceFields)
    {
        QVariantMap op = priceFields;
        op.insert(QStringLiteral("type"), QStringLiteral("manageSellOffer"));
        op.insert(QStringLiteral("selling"), QStringLiteral("native"));
        op.insert(QStringLiteral("buying"),
                  QStringLiteral("USD:GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7"));
        op.insert(QStringLiteral("amount"), QStringLiteral("10"));

        QVariantMap spec = paymentSpec(QStringLiteral("5"));
        spec.insert(QStringLiteral("operations"), QVariantList{op});
        return spec;
    }

    static QPair<qint64, qint64> signedPrice(const QVariantMap& built)
    {
        qint64 n = -1, d = -1;
        for (const QString& line : built.value(QStringLiteral("txrep")).toString().split('\n')) {
            if (line.contains(QStringLiteral("manageSellOfferOp.price.n: ")))
                n = line.section(QStringLiteral(": "), 1).toLongLong();
            if (line.contains(QStringLiteral("manageSellOfferOp.price.d: ")))
                d = line.section(QStringLiteral(": "), 1).toLongLong();
        }
        return qMakePair(n, d);
    }

    /** priceR is the exact fraction or an error — 1.9 used to be signed as 2. */
    void testPriceRIsNeverRounded()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());

        const auto withR = [](const QVariant& n, const QVariant& d) {
            QVariantMap r;
            r.insert(QStringLiteral("n"), n);
            r.insert(QStringLiteral("d"), d);
            QVariantMap fields;
            fields.insert(QStringLiteral("priceR"), r);
            return sellOfferSpec(fields);
        };

        const QList<QPair<QVariant, QVariant>> bad = {
            {1.9, 1}, {1, 2.5}, {QStringLiteral("x"), QStringLiteral("y")}, {0, 1}, {1, 0},
            {-7, 9}, {qlonglong(2147483648LL), 1}, {3e9, 1}, {QStringLiteral("7.0"), 9},
            {QVariant(), 9}, {7, QStringLiteral("")}
        };
        for (const auto& p : bad) {
            QVERIFY2(fails(builder.build(withR(p.first, p.second))),
                     qPrintable(QStringLiteral("priceR accepted: %1/%2")
                                    .arg(p.first.toString(), p.second.toString())));
        }
        QVariantMap notAMap;
        notAMap.insert(QStringLiteral("priceR"), QStringLiteral("7/9"));
        QVERIFY(fails(builder.build(sellOfferSpec(notAMap))));

        QCOMPARE(signedPrice(builder.build(withR(7, 9))), qMakePair(qint64(7), qint64(9)));
        QCOMPARE(signedPrice(builder.build(withR(QStringLiteral("7"), QStringLiteral("9")))),
                 qMakePair(qint64(7), qint64(9)));
        QCOMPARE(signedPrice(builder.build(withR(7.0, 2147483647))),
                 qMakePair(qint64(7), qint64(2147483647)));
    }

    /**
     * A decimal price handed over as a JS number arrives as QVariant(double),
     * whose text for small values is scientific: "1.234567e-06". The old
     * parser kept 11 characters of that and signed a price a million times
     * too high.
     */
    void testADecimalPriceIsTheNumberGiven()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());

        QVariantMap fields;
        fields.insert(QStringLiteral("price"), 0.000001234567);
        const QPair<qint64, qint64> p = signedPrice(builder.build(sellOfferSpec(fields)));
        QVERIFY(p.first > 0 && p.second > 0);
        const double got = double(p.first) / double(p.second);
        QVERIFY2(qAbs(got - 0.000001234567) / 0.000001234567 < 1.0 / 2147483647.0,
                 qPrintable(QStringLiteral("%1/%2").arg(p.first).arg(p.second)));

        fields.insert(QStringLiteral("price"), QStringLiteral("0.25"));
        QCOMPARE(signedPrice(builder.build(sellOfferSpec(fields))),
                 qMakePair(qint64(1), qint64(4)));
    }

    static QVariantMap invokeSpec(const QString& type, const QVariant& value)
    {
        QVariantMap arg;
        arg.insert(QStringLiteral("type"), type);
        arg.insert(QStringLiteral("value"), value);

        QVariantMap op;
        op.insert(QStringLiteral("type"), QStringLiteral("invokeContract"));
        op.insert(QStringLiteral("contractId"),
                  QStringLiteral("CDLZFC3SYJYDZT7K67VZ75HPJVIEUVNIXF47ZG2FB2RMQQVU2HHGCYSC"));
        op.insert(QStringLiteral("functionName"), QStringLiteral("f"));
        op.insert(QStringLiteral("args"), QVariantList{arg});

        QVariantMap spec = paymentSpec(QStringLiteral("5"));
        spec.insert(QStringLiteral("operations"), QVariantList{op});
        return spec;
    }

    /**
     * Contract integers are range-checked, never narrowed: "4294967296" used
     * to be signed as u32 0 and "2147483648" as i32 -2147483648, and TxRep
     * cannot show a Soroban argument, so nothing on screen said so.
     */
    void testContractIntegersAreNeverNarrowed()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());

        const QList<QPair<QString, QVariant>> bad = {
            {"u32", QStringLiteral("4294967296")}, {"u32", QStringLiteral("-1")},
            {"u32", 1.5}, {"i32", QStringLiteral("2147483648")},
            {"i32", QStringLiteral("-2147483649")}, {"i32", QStringLiteral("1e3")},
            {"bool", QStringLiteral("yes")}, {"bool", QStringLiteral("1")},
            {"bool", QStringLiteral("treu")}
        };
        for (const auto& a : bad) {
            QVERIFY2(fails(builder.build(invokeSpec(a.first, a.second))),
                     qPrintable(a.first + QStringLiteral(" accepted: ") + a.second.toString()));
        }

        const QList<QPair<QString, QVariant>> good = {
            {"u32", QStringLiteral("4294967295")}, {"u32", 0}, {"i32", QStringLiteral("2147483647")},
            {"i32", QStringLiteral("-2147483648")}, {"bool", QStringLiteral("TRUE")},
            {"bool", false}
        };
        for (const auto& a : good) {
            const QVariantMap r = builder.build(invokeSpec(a.first, a.second));
            QVERIFY2(!r.contains(QStringLiteral("error")),
                     qPrintable(a.first + QStringLiteral(" refused: ") + a.second.toString()
                                + QStringLiteral(" - ") + r.value(QStringLiteral("error")).toString()));
        }
    }

    /**
     * What QML actually sends. A JS number that went through parseInt(),
     * Number() or Math.round() is a double, and Qt 6 prints 100000.0 as
     * "1e+05" — so reading these fields as text refused a perfectly good fee.
     * Whole doubles are accepted up to 2^53, where a JS number stops being
     * exact; past it, only text is.
     */
    void testWholeJavaScriptNumbersAreAcceptedExactlyAndOnlyWhileExact()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());
        const QVariantMap spec = paymentSpec(QStringLiteral("5"));

        QCOMPARE(builder.build(withKey(spec, "baseFee", 100000.0))
                     .value(QStringLiteral("fee")).toLongLong(), 100000LL);
        QCOMPARE(builder.build(withKey(spec, "baseFee", 3e9))
                     .value(QStringLiteral("fee")).toLongLong(), 3000000000LL);
        QVERIFY(!builder.build(withKey(spec, "timeout", 300000.0)).contains(QStringLiteral("error")));
        QVERIFY(!builder.build(invokeSpec(QStringLiteral("u32"), 3e9)).contains(QStringLiteral("error")));

        QVariantMap r;
        r.insert(QStringLiteral("n"), 1.0);
        r.insert(QStringLiteral("d"), 1000000.0);
        QVariantMap fields;
        fields.insert(QStringLiteral("priceR"), r);
        QCOMPARE(signedPrice(builder.build(sellOfferSpec(fields))),
                 qMakePair(qint64(1), qint64(1000000)));

        // Past 2^53 a JS number is already a different integer. Refused as a
        // number, exact as text.
        const auto memo = [](const QVariant& id) {
            QVariantMap m;
            m.insert(QStringLiteral("type"), QStringLiteral("id"));
            m.insert(QStringLiteral("value"), id);
            return m;
        };
        QVERIFY(fails(builder.build(withKey(spec, "memo", memo(1234567890123456789.0)))));
        const QVariantMap byText = builder.build(
            withKey(spec, "memo", memo(QStringLiteral("1234567890123456789"))));
        QVERIFY(byText.value(QStringLiteral("txrep")).toString()
                    .contains(QStringLiteral("tx.memo.id: 1234567890123456789")));
        QVERIFY(fails(builder.build(withKey(spec, "sequence", 216172782113783809.0))));
        QVERIFY(fails(builder.build(invokeSpec(QStringLiteral("u64"), 12345678901234567890.0))));
        QVERIFY(!builder.build(invokeSpec(QStringLiteral("u64"), QStringLiteral("12345678901234567890")))
                     .contains(QStringLiteral("error")));

        // The JSON-text form of the arguments: QJsonDocument keeps an integer
        // that fits int64 exact and turns a larger one into a double.
        QVariantMap json = invokeSpec(QStringLiteral("u64"), 0);
        QVariantMap op = json.value(QStringLiteral("operations")).toList().first().toMap();
        op.insert(QStringLiteral("args"),
                  QStringLiteral("[{\"type\":\"u64\",\"value\":12345678901234567890}]"));
        json.insert(QStringLiteral("operations"), QVariantList{op});
        QVERIFY(fails(builder.build(json)));
    }

    /**
     * Left out means the default; given means read. On Qt 5 a JS empty string
     * arrives as a null QString, which QVariant::isNull() called "missing", so
     * a blank fee silently became 100 there and an error on Qt 6.
     */
    void testBlankIsGivenOnEveryQtVersion()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());
        const QVariantMap spec = paymentSpec(QStringLiteral("5"));

        QVERIFY(fails(builder.build(withKey(spec, "baseFee", QVariant(QString())))));
        QVERIFY(fails(builder.build(withKey(spec, "timeout", QVariant(QString())))));
        // Left out, or null, is the default. A key holding an invalid QVariant
        // is refused: that is what Qt 5 hands over for a JS function (and for
        // undefined — the two cannot be told apart there).
        QCOMPARE(builder.build(spec).value(QStringLiteral("fee")).toLongLong(), 100LL);
        QCOMPARE(builder.build(withKey(spec, "baseFee", QVariant::fromValue(nullptr)))
                     .value(QStringLiteral("fee")).toLongLong(), 100LL);
        QVERIFY(fails(builder.build(withKey(spec, "baseFee", QVariant()))));
        // JS -0 is not an explicit 0: it would mean "never expires".
        QVERIFY(fails(builder.build(withKey(spec, "timeout", -0.0))));
    }

    /** The pool fee is an int32 in the XDR: 4294967295 wrapped to -1. */
    void testPoolShareFeeIsRangeChecked()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());

        const auto lpTrust = [](const QVariant& fee) {
            QVariantMap op;
            op.insert(QStringLiteral("type"), QStringLiteral("changeTrust"));
            op.insert(QStringLiteral("poolShareAssetA"), QStringLiteral("native"));
            op.insert(QStringLiteral("poolShareAssetB"),
                      QStringLiteral("USD:GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7"));
            op.insert(QStringLiteral("limit"), QStringLiteral("1000"));
            if (fee.isValid())
                op.insert(QStringLiteral("poolShareFee"), fee);
            QVariantMap spec = paymentSpec(QStringLiteral("5"));
            spec.insert(QStringLiteral("operations"), QVariantList{op});
            return spec;
        };
        for (const QVariant& bad : QVariantList{QStringLiteral("2147483648"),
                                                QStringLiteral("4294967295"),
                                                QStringLiteral("-1"), 30.5}) {
            QVERIFY2(fails(builder.build(lpTrust(bad))),
                     qPrintable(QStringLiteral("poolShareFee accepted: ") + bad.toString()));
        }
        const QVariantMap ok = builder.build(lpTrust(QVariant()));
        QVERIFY2(!ok.contains(QStringLiteral("error")),
                 qPrintable(ok.value(QStringLiteral("error")).toString()));
    }

    /**
     * The edges of "exact". 2^53 is where 2^53 + 1 rounds to, so a double
     * holding it is refused. A JSON literal with '.0' or an exponent reaches
     * this code as an integer QJsonDocument already rounded, so it is refused
     * from the text. A blank setOptions weight is left alone rather than read
     * as 0 — which deletes a signer. A text memo given as a number is written
     * out exactly, not as "1e+06".
     */
    void testTheEdgesOfExact()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());
        const QVariantMap spec = paymentSpec(QStringLiteral("5"));
        const auto memo = [](const QString& type, const QVariant& value) {
            QVariantMap m;
            m.insert(QStringLiteral("type"), type);
            m.insert(QStringLiteral("value"), value);
            return m;
        };

        QVERIFY(fails(builder.build(withKey(spec, "memo", memo("id", 9007199254740992.0)))));
        QVERIFY(builder.build(withKey(spec, "memo", memo("id", 9007199254740991.0)))
                    .value(QStringLiteral("txrep")).toString()
                    .contains(QStringLiteral("tx.memo.id: 9007199254740991")));

        QVariantMap json = invokeSpec(QStringLiteral("u64"), 0);
        QVariantMap op = json.value(QStringLiteral("operations")).toList().first().toMap();
        for (const char* literal : {"216172782113783809.0", "2.16172782113783809e17",
                                    "9007199254740993.0"}) {
            op.insert(QStringLiteral("args"),
                      QStringLiteral("[{\"type\":\"u64\",\"value\":%1}]")
                          .arg(QString::fromLatin1(literal)));
            json.insert(QStringLiteral("operations"), QVariantList{op});
            QVERIFY2(fails(builder.build(json)), literal);
        }
        op.insert(QStringLiteral("args"),
                  QStringLiteral("[{\"type\":\"u64\",\"value\":\"216172782113783809\"}]"));
        json.insert(QStringLiteral("operations"), QVariantList{op});
        QVERIFY(!builder.build(json).contains(QStringLiteral("error")));

        const auto setOptions = [](const char* key, const QVariant& value, bool withSigner) {
            QVariantMap o;
            o.insert(QStringLiteral("type"), QStringLiteral("setOptions"));
            o.insert(QString::fromLatin1(key), value);
            if (withSigner)
                o.insert(QStringLiteral("signerKey"),
                         QStringLiteral("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"));
            QVariantMap s = paymentSpec(QStringLiteral("5"));
            s.insert(QStringLiteral("operations"), QVariantList{o});
            return s;
        };
        const QVariantMap blankWeight = builder.build(setOptions("masterWeight", QStringLiteral("   "), false));
        QVERIFY2(!blankWeight.contains(QStringLiteral("error")),
                 qPrintable(blankWeight.value(QStringLiteral("error")).toString()));
        QVERIFY(blankWeight.value(QStringLiteral("txrep")).toString()
                    .contains(QStringLiteral("setOptionsOp.masterWeight._present: false")));
        QVERIFY(fails(builder.build(setOptions("signerWeight", QStringLiteral(" "), true))));
        // Bytes to the network: 256 would be signed and then fail, fee spent.
        for (const char* key : {"masterWeight", "lowThreshold", "medThreshold", "highThreshold"}) {
            QVERIFY2(fails(builder.build(setOptions(key, 256, false))), key);
            QVERIFY2(!builder.build(setOptions(key, 255, false)).contains(QStringLiteral("error")), key);
        }
        QVERIFY(fails(builder.build(setOptions("signerWeight", 256, true))));
        QVERIFY(!builder.build(setOptions("signerWeight", 255, true)).contains(QStringLiteral("error")));

        QVERIFY(builder.build(withKey(spec, "memo", memo("text", 1000000.0)))
                    .value(QStringLiteral("txrep")).toString()
                    .contains(QStringLiteral("tx.memo.text: \"1000000\"")));
        QVERIFY(fails(builder.build(withKey(spec, "memo", memo("text", 1.5)))));

        // extendTo is required: left out used to extend to ledger 0.
        QVariantMap extend;
        extend.insert(QStringLiteral("type"), QStringLiteral("extendFootprintTtl"));
        QVariantMap extendSpec = paymentSpec(QStringLiteral("5"));
        extendSpec.insert(QStringLiteral("operations"), QVariantList{extend});
        QVERIFY(fails(builder.build(extendSpec)));
    }

    /**
     * Shapes and text, read exactly. A claimant predicate in the wrong shape
     * used to become "unconditional" — a time-locked balance claimable now; a
     * vec or an argument list in the wrong shape was signed empty; a number
     * given as text reached the ledger as "1e+06"; a JSON literal such as
     * 7.0000000000000001 was rounded to 7 and signed as an integer.
     */
    void testShapesAndTextAreReadExactly()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());
        const QString dest = QString::fromLatin1(DEST());

        const auto claimable = [&](const QVariant& predicate) {
            QVariantMap claimant;
            claimant.insert(QStringLiteral("destination"), dest);
            if (predicate.isValid())
                claimant.insert(QStringLiteral("predicate"), predicate);
            QVariantMap op;
            op.insert(QStringLiteral("type"), QStringLiteral("createClaimableBalance"));
            op.insert(QStringLiteral("asset"), QStringLiteral("native"));
            op.insert(QStringLiteral("amount"), QStringLiteral("10"));
            op.insert(QStringLiteral("claimants"), QVariantList{claimant});
            QVariantMap spec = paymentSpec(QStringLiteral("5"));
            spec.insert(QStringLiteral("operations"), QVariantList{op});
            return spec;
        };
        QVariantMap absBefore;
        absBefore.insert(QStringLiteral("type"), QStringLiteral("absBefore"));
        absBefore.insert(QStringLiteral("value"), QStringLiteral("1798761600"));
        for (const QVariant& bad : QVariantList{QStringLiteral("absBefore:1798761600"),
                                                QVariantList{absBefore}, QVariantMap()}) {
            QVERIFY2(fails(builder.build(claimable(bad))),
                     qPrintable(QStringLiteral("predicate accepted: ") + bad.toString()));
        }
        QVERIFY(!builder.build(claimable(QVariant())).contains(QStringLiteral("error")));
        QVERIFY(!builder.build(claimable(absBefore)).contains(QStringLiteral("error")));

        QVERIFY(fails(builder.build(invokeSpec(QStringLiteral("vec"), 7))));
        QVariantMap notAList;
        notAList.insert(QStringLiteral("type"), QStringLiteral("u64"));
        notAList.insert(QStringLiteral("value"), 7);
        QVERIFY(fails(builder.build(invokeSpec(QStringLiteral("vec"), notAList))));
        QVariantMap argsAsObject = invokeSpec(QStringLiteral("u64"), 7);
        QVariantMap op = argsAsObject.value(QStringLiteral("operations")).toList().first().toMap();
        op.insert(QStringLiteral("args"), notAList);
        argsAsObject.insert(QStringLiteral("operations"), QVariantList{op});
        QVERIFY(fails(builder.build(argsAsObject)));

        QVERIFY(fails(builder.build(invokeSpec(QStringLiteral("string"), 0.1 + 0.2))));
        QVERIFY(!builder.build(invokeSpec(QStringLiteral("string"), 1000000.0))
                     .contains(QStringLiteral("error")));

        QVariantMap json = invokeSpec(QStringLiteral("u32"), 0);
        op = json.value(QStringLiteral("operations")).toList().first().toMap();
        for (const char* literal : {"7.0000000000000001", "4503599627370496.5", "0.99999999999999999"}) {
            op.insert(QStringLiteral("args"),
                      QStringLiteral("[{\"type\":\"u64\",\"value\":%1}]")
                          .arg(QString::fromLatin1(literal)));
            json.insert(QStringLiteral("operations"), QVariantList{op});
            QVERIFY2(fails(builder.build(json)), literal);
        }
        op.insert(QStringLiteral("args"), QStringLiteral("[{\"type\":\"u32\",\"value\":7.0}]"));
        json.insert(QStringLiteral("operations"), QVariantList{op});
        QVERIFY(!builder.build(json).contains(QStringLiteral("error")));

        // manageData: the value as given, a number written out in full.
        QVariantMap data;
        data.insert(QStringLiteral("type"), QStringLiteral("manageData"));
        data.insert(QStringLiteral("name"), QStringLiteral("ref"));
        data.insert(QStringLiteral("value"), 1000000.0);
        QVariantMap dataSpec = paymentSpec(QStringLiteral("5"));
        dataSpec.insert(QStringLiteral("operations"), QVariantList{data});
        QVERIFY(builder.build(dataSpec).value(QStringLiteral("txrep")).toString()
                    .contains(QStringLiteral("manageDataOp.dataValue: ")
                              + QString::fromLatin1(QByteArray("1000000").toHex())));

        // A signer weight with no signer is a mistake, not a no-op.
        QVariantMap weightOnly;
        weightOnly.insert(QStringLiteral("type"), QStringLiteral("setOptions"));
        weightOnly.insert(QStringLiteral("signerWeight"), 5);
        QVariantMap weightSpec = paymentSpec(QStringLiteral("5"));
        weightSpec.insert(QStringLiteral("operations"), QVariantList{weightOnly});
        QVERIFY(fails(builder.build(weightSpec)));

        // An empty text memo is refused on Qt 5 and Qt 6 alike.
        QVariantMap emptyMemo;
        emptyMemo.insert(QStringLiteral("type"), QStringLiteral("text"));
        emptyMemo.insert(QStringLiteral("value"), QString());
        QVERIFY(fails(builder.build(withKey(paymentSpec(QStringLiteral("5")), "memo", emptyMemo))));
        emptyMemo.insert(QStringLiteral("value"), QStringLiteral(""));
        QVERIFY(fails(builder.build(withKey(paymentSpec(QStringLiteral("5")), "memo", emptyMemo))));
    }

    /**
     * Left out is not the same as unreadable. On Qt 5 a JS function arrives
     * exactly like undefined — an invalid QVariant under a key that is there —
     * and where "left out" silently means something (unconditional, delete
     * the entry, no arguments, clear the domain) the key being present is
     * taken as the caller meaning to give a value, and it is refused. The
     * QVariant() below is what Qt 5 hands over for `predicate: lockUntil`.
     */
    void testLeftOutIsNotTheSameAsUnreadable()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());
        const auto withOp = [](const QVariantMap& op) {
            QVariantMap spec = paymentSpec(QStringLiteral("5"));
            spec.insert(QStringLiteral("operations"), QVariantList{op});
            return spec;
        };

        QVariantMap claimant;
        claimant.insert(QStringLiteral("destination"), QString::fromLatin1(DEST()));
        claimant.insert(QStringLiteral("predicate"), QVariant());
        QVariantMap claimable;
        claimable.insert(QStringLiteral("type"), QStringLiteral("createClaimableBalance"));
        claimable.insert(QStringLiteral("asset"), QStringLiteral("native"));
        claimable.insert(QStringLiteral("amount"), QStringLiteral("10"));
        claimable.insert(QStringLiteral("claimants"), QVariantList{claimant});
        QVERIFY(fails(builder.build(withOp(claimable))));

        QVariantMap data;
        data.insert(QStringLiteral("type"), QStringLiteral("manageData"));
        data.insert(QStringLiteral("name"), QStringLiteral("k"));
        data.insert(QStringLiteral("value"), QVariant());
        QVERIFY(fails(builder.build(withOp(data))));

        QVariantMap noArgs = invokeSpec(QStringLiteral("u32"), 7);
        QVariantMap invoke = noArgs.value(QStringLiteral("operations")).toList().first().toMap();
        invoke.insert(QStringLiteral("args"), QVariant());
        QVERIFY(fails(builder.build(withOp(invoke))));
        QVariantMap stringWithoutValue;
        stringWithoutValue.insert(QStringLiteral("type"), QStringLiteral("string"));
        invoke.insert(QStringLiteral("args"), QVariantList{stringWithoutValue});
        QVERIFY(fails(builder.build(withOp(invoke))));

        // homeDomain: undefined leaves it alone, like every other field; "" clears.
        QVariantMap options;
        options.insert(QStringLiteral("type"), QStringLiteral("setOptions"));
        options.insert(QStringLiteral("homeDomain"), QVariant());
        QVERIFY(builder.build(withOp(options)).value(QStringLiteral("txrep")).toString()
                    .contains(QStringLiteral("setOptionsOp.homeDomain._present: false")));

        // A memo hash is exactly 32 bytes: never zero-padded from less.
        const auto hashMemo = [](const QString& hex) {
            QVariantMap m;
            m.insert(QStringLiteral("type"), QStringLiteral("hash"));
            m.insert(QStringLiteral("value"), hex);
            return m;
        };
        const QString full = QString(64, QLatin1Char('a'));
        QVERIFY(fails(builder.build(withKey(paymentSpec(QStringLiteral("5")), "memo", hashMemo("")))));
        QVERIFY(fails(builder.build(withKey(paymentSpec(QStringLiteral("5")), "memo",
                                            hashMemo(full.left(62))))));
        QVERIFY(!builder.build(withKey(paymentSpec(QStringLiteral("5")), "memo", hashMemo(full)))
                     .contains(QStringLiteral("error")));

        // ".999…" is JSON QJsonDocument accepts and rounds to 1.
        QVariantMap json = invokeSpec(QStringLiteral("u32"), 0);
        QVariantMap op = json.value(QStringLiteral("operations")).toList().first().toMap();
        op.insert(QStringLiteral("args"),
                  QStringLiteral("[{\"type\":\"u32\",\"value\":.99999999999999999}]"));
        json.insert(QStringLiteral("operations"), QVariantList{op});
        QVERIFY(fails(builder.build(json)));
    }

    /**
     * The wrong shape is refused, never read as "nothing". A memo given as
     * "12345" or { id: "12345" } was sent as no memo — an exchange deposit
     * credited to nobody; an operation's sourceAccount given as an object was
     * dropped and the operation debited the transaction's source; allowTrust
     * authorize 2 or "yes" signed a full revocation; offerId given as a JS
     * function (on Qt 5: a present key with an invalid value) became 0 — a new
     * offer instead of a change to the existing one.
     */
    void testTheWrongShapeIsNeverReadAsNothing()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());
        const QVariantMap spec = paymentSpec(QStringLiteral("5"));

        QVariantMap byId;
        byId.insert(QStringLiteral("id"), QStringLiteral("12345"));
        QVariantMap noneWithValue;
        noneWithValue.insert(QStringLiteral("type"), QStringLiteral("none"));
        noneWithValue.insert(QStringLiteral("value"), QStringLiteral("12345"));
        QVariantMap untyped;
        untyped.insert(QStringLiteral("value"), QStringLiteral("12345"));
        for (const QVariant& memo : QVariantList{QStringLiteral("12345"), 12345, byId,
                                                 noneWithValue, untyped, QVariant()}) {
            QVERIFY2(fails(builder.build(withKey(spec, "memo", memo))),
                     qPrintable(QStringLiteral("memo accepted: ") + memo.toString()));
        }
        QVariantMap none;
        none.insert(QStringLiteral("type"), QStringLiteral("none"));
        none.insert(QStringLiteral("value"), QString());
        QVERIFY(!builder.build(withKey(spec, "memo", none)).contains(QStringLiteral("error")));

        const auto withOp = [&](const QVariantMap& op) {
            QVariantMap s = spec;
            s.insert(QStringLiteral("operations"), QVariantList{op});
            return s;
        };
        QVariantMap payment = spec.value(QStringLiteral("operations")).toList().first().toMap();
        QVariantMap account;
        account.insert(QStringLiteral("accountId"), QString::fromLatin1(DEST()));
        payment.insert(QStringLiteral("sourceAccount"), account);
        QVERIFY(fails(builder.build(withOp(payment))));
        payment.insert(QStringLiteral("sourceAccount"), QVariant());
        QVERIFY(fails(builder.build(withOp(payment))));
        payment.insert(QStringLiteral("sourceAccount"), QString::fromLatin1(DEST()));
        QVERIFY(builder.build(withOp(payment)).value(QStringLiteral("txrep")).toString()
                    .contains(QStringLiteral("tx.operations[0].sourceAccount._present: true")));

        QVariantMap allow;
        allow.insert(QStringLiteral("type"), QStringLiteral("allowTrust"));
        allow.insert(QStringLiteral("trustor"), QString::fromLatin1(DEST()));
        allow.insert(QStringLiteral("assetCode"), QStringLiteral("USD"));
        for (const QVariant& bad : QVariantList{2, QStringLiteral("2"), QStringLiteral("yes"),
                                                QVariant()}) {
            allow.insert(QStringLiteral("authorize"), bad);
            QVERIFY2(fails(builder.build(withOp(allow))), qPrintable(bad.toString()));
        }
        allow.insert(QStringLiteral("authorize"), true);
        QVERIFY(builder.build(withOp(allow)).value(QStringLiteral("txrep")).toString()
                    .contains(QStringLiteral("allowTrustOp.authorize: true")));
        // Left out, authorize read as false: a revocation nobody asked for.
        // It is required, as js requires it; false is a revocation asked for.
        allow.remove(QStringLiteral("authorize"));
        QVERIFY(fails(builder.build(withOp(allow))));
        allow.insert(QStringLiteral("authorize"), QStringLiteral(""));
        QVERIFY(fails(builder.build(withOp(allow))));
        allow.insert(QStringLiteral("authorize"), false);
        QVERIFY(builder.build(withOp(allow)).value(QStringLiteral("txrep")).toString()
                    .contains(QStringLiteral("allowTrustOp.authorize: false")));

        QVariantMap offer;
        offer.insert(QStringLiteral("type"), QStringLiteral("manageSellOffer"));
        offer.insert(QStringLiteral("selling"), QStringLiteral("native"));
        offer.insert(QStringLiteral("buying"),
                     QStringLiteral("USD:GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7"));
        offer.insert(QStringLiteral("amount"), QStringLiteral("10"));
        offer.insert(QStringLiteral("price"), QStringLiteral("0.5"));
        offer.insert(QStringLiteral("offerId"), QVariant());
        QVERIFY(fails(builder.build(withOp(offer))));
        QVariantMap weights;
        weights.insert(QStringLiteral("type"), QStringLiteral("setOptions"));
        weights.insert(QStringLiteral("masterWeight"), QVariant());
        QVERIFY(fails(builder.build(withOp(weights))));

        // A cleared home domain is shown as cleared, on Qt 6 as on Qt 5.
        QVariantMap clear;
        clear.insert(QStringLiteral("type"), QStringLiteral("setOptions"));
        clear.insert(QStringLiteral("homeDomain"), QString());
        const QString txrep = builder.build(withOp(clear)).value(QStringLiteral("txrep")).toString();
        QVERIFY2(txrep.contains(QStringLiteral("setOptionsOp.homeDomain._present: true")),
                 qPrintable(txrep));
    }

    /** One price, not two: a stale priceR must not silently win over the
     *  decimal the user just typed. */
    void testPriceAndPriceRTogetherAreRefused()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());
        QVariantMap r;
        r.insert(QStringLiteral("n"), 7);
        r.insert(QStringLiteral("d"), 9);
        QVariantMap fields;
        fields.insert(QStringLiteral("priceR"), r);
        fields.insert(QStringLiteral("price"), QStringLiteral("0.5"));
        QVERIFY(fails(builder.build(sellOfferSpec(fields))));
    }


    // ─── What the second review found ────────────────────────────────

    static QVariantMap withArgsText(const QString& json)
    {
        QVariantMap spec = invokeSpec(QStringLiteral("i64"), QStringLiteral("0"));
        QVariantMap op = spec.value(QStringLiteral("operations")).toList().first().toMap();
        op.insert(QStringLiteral("args"), json);
        spec.insert(QStringLiteral("operations"), QVariantList{op});
        return spec;
    }

    /** A JSON integer below INT64_MIN was signed as INT64_MIN: QJsonDocument
     *  holds it as a double, which is exactly INT64_MIN. */
    void testJsonIntegersOutsideInt64AreRefused()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());
        for (const char* json : {"[{\"type\":\"i64\",\"value\":-9223372036854775809}]",
                                 "[{\"type\":\"i64\",\"value\":9223372036854775808}]",
                                 "[{\"type\":\"u64\",\"value\":18446744073709551616}]"}) {
            QVERIFY2(fails(builder.build(withArgsText(QString::fromLatin1(json)))), json);
        }
        QVERIFY(!fails(builder.build(withArgsText(
            QStringLiteral("[{\"type\":\"i64\",\"value\":-9223372036854775808}]")))));
    }

    /** Hex is read strictly: "0xdeadbeef" signed 00 de ad be ef, "abc" signed
     *  0a bc, a salt one digit short was a different contract address. */
    void testHexIsReadStrictly()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());
        for (const QVariant& bad : {QVariant(QStringLiteral("0xdeadbeef")), QVariant(QStringLiteral("abc")),
                                    QVariant(QStringLiteral("deadbeefzz")), QVariant(QStringLiteral("de ad")),
                                    QVariant(255)}) {
            QVERIFY2(fails(builder.build(invokeSpec(QStringLiteral("bytes"), bad))),
                     qPrintable(bad.toString()));
        }
        QVERIFY(!fails(builder.build(invokeSpec(QStringLiteral("bytes"), QStringLiteral("deadBEEF")))));

        auto create = [&](const QString& salt) {
            QVariantMap op;
            op.insert(QStringLiteral("type"), QStringLiteral("createContract"));
            op.insert(QStringLiteral("fromAddress"), QString::fromLatin1(DEST()));
            op.insert(QStringLiteral("wasmHash"), QString(64, QLatin1Char('b')));
            op.insert(QStringLiteral("salt"), salt);
            QVariantMap spec = paymentSpec(QStringLiteral("5"));
            spec.insert(QStringLiteral("operations"), QVariantList{op});
            return builder.build(spec);
        };
        QVERIFY(fails(create(QString(63, QLatin1Char('a')))));
        QVERIFY(fails(create(QStringLiteral("0x") + QString(62, QLatin1Char('a')))));
        QVERIFY(!fails(create(QString(64, QLatin1Char('a')))));
    }

    /** A value given with void, and constructor arguments on createContract,
     *  were dropped without being read. */
    void testNothingGivenIsDropped()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());
        QVERIFY(fails(builder.build(invokeSpec(QStringLiteral("void"), QStringLiteral("123")))));

        auto create = [&](const QVariant& ctor) {
            QVariantMap op;
            op.insert(QStringLiteral("type"), QStringLiteral("createContract"));
            op.insert(QStringLiteral("fromAddress"), QString::fromLatin1(DEST()));
            op.insert(QStringLiteral("wasmHash"), QString(64, QLatin1Char('b')));
            op.insert(QStringLiteral("salt"), QString(64, QLatin1Char('a')));
            op.insert(QStringLiteral("constructorArgs"), ctor);
            QVariantMap spec = paymentSpec(QStringLiteral("5"));
            spec.insert(QStringLiteral("operations"), QVariantList{op});
            return builder.build(spec);
        };
        QVariantMap bad;
        bad.insert(QStringLiteral("type"), QStringLiteral("u32"));
        bad.insert(QStringLiteral("value"), QStringLiteral("4294967296"));
        QVERIFY(fails(create(QVariantList{bad})));
        QVariantMap good = bad;
        good.insert(QStringLiteral("value"), QStringLiteral("7"));
        QVERIFY(!fails(create(QVariantList{good})));
    }

    /** A price in a shape that is not a decimal read as no price at all, so a
     *  stale priceR beside it was signed; zero is not a price. */
    void testAPriceIsADecimalOrPriceR()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());
        QVariantMap r;
        r.insert(QStringLiteral("n"), 7);
        r.insert(QStringLiteral("d"), 9);
        QVariantMap object;
        object.insert(QStringLiteral("price"), r);
        QVERIFY(fails(builder.build(sellOfferSpec(object))));
        object.insert(QStringLiteral("priceR"), r);
        QVERIFY(fails(builder.build(sellOfferSpec(object))));
        QVariantMap list;
        list.insert(QStringLiteral("price"), QVariantList{QStringLiteral("0.5")});
        list.insert(QStringLiteral("priceR"), r);
        QVERIFY(fails(builder.build(sellOfferSpec(list))));
        for (const QVariant& zero : {QVariant(QStringLiteral("0")), QVariant(0), QVariant(0.0)}) {
            QVariantMap fields;
            fields.insert(QStringLiteral("price"), zero);
            QVERIFY(fails(builder.build(sellOfferSpec(fields))));
        }
    }

    /** A key nothing reads was ignored: a fee under the wrong name left the
     *  default, Horizon's price_r beside a decimal signed the decimal. */
    void testUnknownKeysAreRefused()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());
        QVERIFY(fails(builder.build(withKey(paymentSpec(QStringLiteral("5")), "fee", 1000000))));
        QVERIFY(fails(builder.build(withKey(paymentSpec(QStringLiteral("5")), "base_fee", 1000))));
        QVariantMap fields;
        fields.insert(QStringLiteral("price"), QStringLiteral("0.7777778"));
        QVariantMap r;
        r.insert(QStringLiteral("n"), 7);
        r.insert(QStringLiteral("d"), 9);
        fields.insert(QStringLiteral("price_r"), r);
        QVERIFY(fails(builder.build(sellOfferSpec(fields))));
    }

    /** A muxed source was signed as its G account (it is sent muxed now, as
     *  js sends it); an unknown network name meant testnet; zero and
     *  negative payments were signed. */
    void testWhatCannotBeHonouredIsRefused()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());
        {
            QVariantMap op;
            op.insert(QStringLiteral("type"), QStringLiteral("payment"));
            op.insert(QStringLiteral("destination"), QStringLiteral("GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO"));
            op.insert(QStringLiteral("asset"), QStringLiteral("native"));
            op.insert(QStringLiteral("amount"), QStringLiteral("10"));
            QVariantMap spec;
            spec.insert(QStringLiteral("sourceAccount"),
                        QStringLiteral("MCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6AAAAEPXD6YEZPS2W"));
            spec.insert(QStringLiteral("sequence"), QStringLiteral("101"));
            spec.insert(QStringLiteral("timeout"), 0);
            spec.insert(QStringLiteral("operations"), QVariantList{op});
            const QVariantMap muxed = builder.build(spec);
            QVERIFY2(!fails(muxed), qPrintable(muxed.value(QStringLiteral("error")).toString()));
            // js-stellar-sdk 17.1.0, the same MuxedAccount source.
            QCOMPARE(muxed.value(QStringLiteral("hash")).toString(),
                     QStringLiteral("b0399e01d1d74346e33bbaf1b35b7b72c4f1c319069a5d43c7940f93530ccb1d"));
        }
        QVERIFY(fails(builder.build(withKey(paymentSpec(QStringLiteral("5")), "network", QStringLiteral("pubnet")))));
        QVERIFY(!fails(builder.build(withKey(paymentSpec(QStringLiteral("5")), "network", QStringLiteral("mainnet")))));
        QVariantMap contradiction = withKey(paymentSpec(QStringLiteral("5")), "network", QStringLiteral("public"));
        contradiction.insert(QStringLiteral("networkPassphrase"), QStringLiteral("Test SDF Network ; September 2015"));
        QVERIFY(fails(builder.build(contradiction)));
        QVERIFY(!fails(builder.build(withKey(paymentSpec(QStringLiteral("5")), "network", QStringLiteral("public")))));
        for (const char* amount : {"0", "-5", "0.0000000"}) {
            QVariantMap spec = paymentSpec(QStringLiteral("5"));
            QVariantMap op = spec.value(QStringLiteral("operations")).toList().first().toMap();
            op.insert(QStringLiteral("amount"), QString::fromLatin1(amount));
            spec.insert(QStringLiteral("operations"), QVariantList{op});
            QVERIFY2(fails(builder.build(spec)), amount);
        }
        QVariantMap memo;
        memo.insert(QStringLiteral("type"), QStringLiteral("text"));
        memo.insert(QStringLiteral("value"), QStringLiteral("12345") + QChar(0xD800));
        QVERIFY(fails(builder.build(withKey(paymentSpec(QStringLiteral("5")), "memo", memo))));
    }

    /** Tx.sign signed a truncated envelope with the missing fields as zeros,
     *  and one with bytes after it; a memo that is not UTF-8 was re-encoded,
     *  so the signature covered a different transaction. */
    void testSignSignsExactlyTheEnvelopeGiven()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());
        const QString xdr = builder.build(paymentSpec(QStringLiteral("5"))).value(QStringLiteral("xdr")).toString();
        const QByteArray raw = QByteArray::fromBase64(xdr.toLatin1());
        auto sign = [&](const QString& envelope) {
            QVariantMap args;
            args.insert(QStringLiteral("xdr"), envelope);
            args.insert(QStringLiteral("seeds"), QVariantList{QString::fromLatin1(SECRET())});
            return builder.sign(args);
        };
        QVERIFY(fails(sign(QString::fromLatin1(raw.left(raw.size() - 12).toBase64()))));
        QVERIFY(fails(sign(QString::fromLatin1((raw + QByteArray(8, '\0')).toBase64()))));
        QVERIFY(!fails(sign(xdr)));

        Account account(KeyPair::fromAccountId(QString::fromLatin1(SOURCE())), 4);
        QScopedPointer<Transaction> tx(TransactionBuilder(&account, Network::testnetNetwork())
            .addOperation(BumpSequenceOperation::create(qint64(9)))
            .addMemo(MemoText::fromBytes(QByteArray("12345\xff", 6)))
            .setTimeout(TransactionBuilder::TIMEOUT_INFINITE).setBaseFee(100).build());
        const QVariantMap signedMemo = sign(tx->toEnvelopeXdrBase64());
        QVERIFY(!fails(signedMemo));
        QCOMPARE(signedMemo.value(QStringLiteral("hash")).toString(), tx->hashHex());
        QScopedPointer<AbstractTransaction> back(AbstractTransaction::fromEnvelopeXdr(
            signedMemo.value(QStringLiteral("xdr")).toString(), Network::testnetNetwork()));
        QCOMPARE(dynamic_cast<MemoText*>(dynamic_cast<Transaction*>(back.data())->getMemo())->getBytes(),
                 QByteArray("12345\xff", 6));
    }

    /**
     * The money cases from real JavaScript, through QJSEngine: the boundary
     * where a JS number is a double, "" and undefined differ, and Qt 6 prints
     * 100000 as "1e+05". Everything above calls the facade from C++; this is
     * what a QML page actually sends, and it runs headless in CI.
     */
    void testTheMoneyCasesFromJavaScript()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());
        QJSEngine engine;
        engine.globalObject().setProperty(QStringLiteral("Tx"), engine.newQObject(&builder));
        QQmlEngine::setObjectOwnership(&builder, QQmlEngine::CppOwnership);
        const QString base = QStringLiteral(
            "function spec(extra, op) {"
            "  var o = { type: 'payment', destination: '%1', asset: 'native', amount: '10' };"
            "  for (var k in op) o[k] = op[k];"
            "  var s = { sourceAccount: '%2', sequence: '5', operations: [o] };"
            "  for (var k in extra) s[k] = extra[k];"
            "  return s;"
            "}"
            "function refused(extra, op) { var r = Tx.build(spec(extra, op || {})); return !!r.error && !r.xdr; }"
            "function invoke(args) {"
            "  return { type: 'invokeContract', contractId: 'CDLZFC3SYJYDZT7K67VZ75HPJVIEUVNIXF47ZG2FB2RMQQVU2HHGCYSC',"
            "           functionName: 'f', args: args };"
            "}"
            "function refusedOp(op) { var r = Tx.build({ sourceAccount: '%2', sequence: '5', operations: [op] });"
            "                         return !!r.error && !r.xdr; }")
            .arg(QString::fromLatin1(DEST()), QString::fromLatin1(SOURCE()));
        QVERIFY(!engine.evaluate(base).isError());

        const QStringList mustRefuse = {
            QStringLiteral("refused({ baseFee: -5 })"),
            QStringLiteral("refused({ baseFee: '' })"),
            QStringLiteral("refused({ baseFee: 100.5 })"),
            QStringLiteral("refused({ timeout: '' })"),
            QStringLiteral("refused({ timeout: -1 })"),
            QStringLiteral("refused({ fee: 1000000 })"),
            QStringLiteral("refused({ network: 'pubnet' })"),
            QStringLiteral("refused({}, { amount: '-5' })"),
            QStringLiteral("refused({}, { amount: 0 })"),
            QStringLiteral("refusedOp(invoke([{ type: 'u32', value: '4294967296' }]))"),
            QStringLiteral("refusedOp(invoke([{ type: 'u32', value: 4294967296 }]))"),
            QStringLiteral("refusedOp(invoke([{ type: 'u64', value: 9007199254740993 }]))"),
            QStringLiteral("refusedOp(invoke('[{\"type\":\"i64\",\"value\":-9223372036854775809}]'))"),
            QStringLiteral("refusedOp(invoke([{ type: 'bytes', value: '0xdeadbeef' }]))"),
        };
        for (const QString& js : mustRefuse) {
            const QJSValue v = engine.evaluate(js);
            QVERIFY2(!v.isError() && v.toBool(), qPrintable(js));
        }
        const QStringList mustBuild = {
            QStringLiteral("!refused({ baseFee: 100000 })"),     // "1e+05" on Qt 6
            QStringLiteral("!refused({})"),
            QStringLiteral("!refusedOp(invoke([{ type: 'u32', value: 4294967295 }]))"),
        };
        for (const QString& js : mustBuild) {
            const QJSValue v = engine.evaluate(js);
            QVERIFY2(!v.isError() && v.toBool(), qPrintable(js));
        }
        // No timeout given: 180 seconds, not "never expires".
        const QJSValue built = engine.evaluate(QStringLiteral("Tx.build(spec({}, {})).txrep"));
        QVERIFY(built.toString().contains(QStringLiteral("tx.cond.type: PRECOND_TIME")));
    }

    /** A key nothing reads in a claimant or a predicate was ignored: a
     *  misspelt "predicat" made a one-year lock claimable at once. */
    void testClaimantsAndPredicatesHaveNoStrayKeys()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());
        auto claim = [&](const QString& claimants) {
            QVariantMap op;
            op.insert(QStringLiteral("type"), QStringLiteral("createClaimableBalance"));
            op.insert(QStringLiteral("asset"), QStringLiteral("native"));
            op.insert(QStringLiteral("amount"), QStringLiteral("1000"));
            op.insert(QStringLiteral("claimants"), claimants);
            QVariantMap spec = paymentSpec(QStringLiteral("5"));
            spec.insert(QStringLiteral("operations"), QVariantList{op});
            return builder.build(spec);
        };
        const QString dest = QString::fromLatin1(DEST());
        QVERIFY(!fails(claim(QStringLiteral("[{\"destination\":\"%1\",\"predicate\":{\"type\":\"not\",\"predicate\":{\"type\":\"relBefore\",\"value\":\"31536000\"}}}]").arg(dest))));
        QVERIFY(fails(claim(QStringLiteral("[{\"destination\":\"%1\",\"predicat\":{\"type\":\"not\",\"predicate\":{\"type\":\"relBefore\",\"value\":\"31536000\"}}}]").arg(dest))));
        QVERIFY(fails(claim(QStringLiteral("[{\"destination\":\"%1\",\"predicate\":{\"type\":\"relBefore\",\"value\":\"1\",\"extra\":2}}]").arg(dest))));
        QVERIFY(fails(claim(QStringLiteral("[{\"destination\":\"%1\",\"Destination\":\"%1\"}]").arg(dest))));
    }

    /**
     * The help a form shows says what build() does: changeTrust's offered
     * "native", a trustline stellar-core refuses as malformed, and
     * createClaimableBalance's said "1e7" is rejected, where it is ten
     * million units, as js-stellar-sdk and py-stellar-sdk read it.
     */
    void testHelpTextSaysWhatTheBuilderDoes()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());
        auto note = [&](const QString& type, const QString& name) {
            for (const QVariant& field : builder.fieldsFor(type)) {
                if (field.toMap().value(QStringLiteral("name")).toString() == name)
                    return field.toMap().value(QStringLiteral("note")).toString();
            }
            return QStringLiteral("no such field");
        };
        const QString asset = note(QStringLiteral("changeTrust"), QStringLiteral("asset"));
        QVERIFY2(asset.contains(QStringLiteral("native has no trustline")), qPrintable(asset));
        const QString amount = note(QStringLiteral("createClaimableBalance"), QStringLiteral("amount"));
        QVERIFY2(!amount.contains(QStringLiteral("rejected")), qPrintable(amount));

        QVariantMap op;
        op.insert(QStringLiteral("type"), QStringLiteral("createClaimableBalance"));
        op.insert(QStringLiteral("asset"), QStringLiteral("native"));
        op.insert(QStringLiteral("amount"), QStringLiteral("1e7"));
        op.insert(QStringLiteral("claimants"),
                  QStringLiteral("[{\"destination\":\"%1\"}]").arg(QString::fromLatin1(DEST())));
        QVariantMap spec = paymentSpec(QStringLiteral("5"));
        spec.insert(QStringLiteral("operations"), QVariantList{op});
        const QVariantMap built = builder.build(spec);
        QVERIFY2(!fails(built), qPrintable(built.value(QStringLiteral("error")).toString()));
        const QString txrep = built.value(QStringLiteral("txrep")).toString();
        QVERIFY2(txrep.contains(QStringLiteral("createClaimableBalanceOp.amount: 100000000000000")), qPrintable(txrep));
        QVERIFY(!txrep.contains(QStringLiteral("createClaimableBalanceOp.amount: 1000000000000000")));
    }

    /** changeTrust: the pool was built and the asset dropped when both were
     *  given; a blank limit is the maximum. */
    void testChangeTrustTakesOneLine()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());
        auto trust = [&](const QVariantMap& fields) {
            QVariantMap op = fields;
            op.insert(QStringLiteral("type"), QStringLiteral("changeTrust"));
            QVariantMap spec = paymentSpec(QStringLiteral("5"));
            spec.insert(QStringLiteral("operations"), QVariantList{op});
            return builder.build(spec);
        };
        const QString usd = QStringLiteral("USD:GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7");
        QVariantMap both;
        both.insert(QStringLiteral("asset"), usd);
        both.insert(QStringLiteral("poolShareAssetA"), QStringLiteral("native"));
        both.insert(QStringLiteral("poolShareAssetB"), usd);
        QVERIFY(fails(trust(both)));
        QVariantMap plain;
        plain.insert(QStringLiteral("asset"), usd);
        const QVariantMap built = trust(plain);
        QVERIFY(!fails(built));
        QVERIFY(built.value(QStringLiteral("txrep")).toString().contains(
            QStringLiteral("changeTrustOp.limit: 9223372036854775807")));
    }

    // ─── The contract: hostile input never throws ────────────────────

    void testNothingThrowsOnGarbageSpecs()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());

        const QStringList garbage = {
            QString(), QStringLiteral(""), QStringLiteral("   "),
            QStringLiteral("not xdr"), QStringLiteral("////"),
            QString(4000, QChar('A')), QString::fromUtf8("\xF0\x9F\x92\xA9")
        };
        for (const QString& g : garbage) {
            QVariantMap spec;
            spec.insert(QStringLiteral("sourceAccount"), g);
            spec.insert(QStringLiteral("sequence"), g);
            spec.insert(QStringLiteral("xdr"), g);
            spec.insert(QStringLiteral("network"), g);
            spec.insert(QStringLiteral("networkPassphrase"), g);
            spec.insert(QStringLiteral("seeds"), QVariantList{g});

            builder.build(spec);
            builder.sign(spec);
            builder.inspect(spec);
        }
        // An empty map must be survivable too: it is what a half-filled form
        // sends on the first keystroke.
        builder.build(QVariantMap());
        builder.sign(QVariantMap());
        builder.inspect(QVariantMap());

        QVERIFY(true);   // reaching here without an exception IS the assertion
    }

    /** A claim predicate nested deeper than Core allows (4) was signed and
     *  refused on chain, fee spent; with no bound at all, a hostile tree ran
     *  the stack out. */
    void testClaimPredicatesNestAtMostFourDeep()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());
        const auto nested = [](int depth) {
            QVariantMap p;
            p.insert(QStringLiteral("type"), QStringLiteral("unconditional"));
            for (int i = 1; i < depth; ++i) {
                QVariantMap n;
                n.insert(QStringLiteral("type"), QStringLiteral("not"));
                n.insert(QStringLiteral("predicate"), p);
                p = n;
            }
            return p;
        };
        const auto build = [&](int depth) {
            QVariantMap claimant;
            claimant.insert(QStringLiteral("destination"), QString::fromLatin1(DEST()));
            claimant.insert(QStringLiteral("predicate"), nested(depth));
            QVariantMap op;
            op.insert(QStringLiteral("type"), QStringLiteral("createClaimableBalance"));
            op.insert(QStringLiteral("asset"), QStringLiteral("native"));
            op.insert(QStringLiteral("amount"), QStringLiteral("10"));
            op.insert(QStringLiteral("claimants"), QVariantList{claimant});
            QVariantMap spec = paymentSpec(QStringLiteral("5"));
            spec.insert(QStringLiteral("operations"), QVariantList{op});
            return builder.build(spec);
        };
        const QVariantMap four = build(4);
        QVERIFY2(!four.contains(QStringLiteral("error")), qPrintable(four.value(QStringLiteral("error")).toString()));
        QVERIFY(build(5).value(QStringLiteral("error")).toString().contains(QStringLiteral("4 deep")));
        QVERIFY(build(64).contains(QStringLiteral("error")));
    }

    /**
     * manageData's value is typed dataValue in the table, "up to 64 bytes",
     * but was always signed as the UTF-8 of the text: "deadbeef" meant as
     * four bytes went out as eight characters, and a binary value could not
     * be given at all. valueFormat ("text", the default, "hex" or "base64")
     * says how it is written, hex and base64 are decoded strictly, and the
     * bytes are what is signed.
     */
    void testManageDataValueFormat()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());
        const auto build = [&](const QVariant& value, const QVariant& format, bool withFormat = true) {
            QVariantMap op;
            op.insert(QStringLiteral("type"), QStringLiteral("manageData"));
            op.insert(QStringLiteral("name"), QStringLiteral("blob"));
            op.insert(QStringLiteral("value"), value);
            if (withFormat)
                op.insert(QStringLiteral("valueFormat"), format);
            QVariantMap spec = paymentSpec(QStringLiteral("5"));
            spec.insert(QStringLiteral("operations"), QVariantList{op});
            return builder.build(spec);
        };
        // The value the envelope carries; "<none>" when it deletes the entry.
        const auto signedValue = [](const QVariantMap& built) -> QByteArray {
            if (built.contains(QStringLiteral("error")))
                return "error: " + built.value(QStringLiteral("error")).toString().toUtf8();
            const QByteArray raw = QByteArray::fromBase64(
                built.value(QStringLiteral("xdr")).toString().toLatin1());
            QDataStream in(raw);
            stellar::TransactionEnvelope envelope;
            in >> envelope;
            if (in.status() != QDataStream::Ok
                    || envelope.type != stellar::EnvelopeType::ENVELOPE_TYPE_TX)
                return QByteArray("<unreadable envelope>");
            const stellar::ManageDataOp& op =
                envelope.v1.tx.operations.value.at(0).operationManageData;
            if (!op.dataValue.filled)
                return QByteArray("<none>");
            return QByteArray(reinterpret_cast<const char*>(op.dataValue.value.value.constData()),
                              op.dataValue.value.value.size());
        };
        const QByteArray bytes = QByteArray::fromHex("00ff10");

        QCOMPARE(signedValue(build(QStringLiteral("EUR"), QVariant(), false)), QByteArray("EUR"));
        QCOMPARE(signedValue(build(QStringLiteral("EUR"), QStringLiteral("text"))), QByteArray("EUR"));
        QCOMPARE(signedValue(build(QStringLiteral("00ff10"), QStringLiteral("text"))), QByteArray("00ff10"));
        QCOMPARE(signedValue(build(QStringLiteral("EUR"), QStringLiteral(""))), QByteArray("EUR"));
        QCOMPARE(signedValue(build(QStringLiteral("00ff10"), QStringLiteral("hex"))), bytes);
        QCOMPARE(signedValue(build(QStringLiteral("00FF10"), QStringLiteral("hex"))), bytes);
        QCOMPARE(signedValue(build(QStringLiteral("AP8Q"), QStringLiteral("base64"))), bytes);
        QCOMPARE(signedValue(build(QStringLiteral("AP8="), QStringLiteral("base64"))),
                 QByteArray::fromHex("00ff"));
        // An empty value is an empty value, whatever the format (as js and
        // py sign it); only a value left out deletes the entry (see
        // testManageDataAndPayloadTextAreSignedAsGiven).
        QCOMPARE(signedValue(build(QStringLiteral(""), QStringLiteral("hex"))), QByteArray(""));
        // 64 bytes once decoded is the limit, not 64 characters of hex/base64.
        QCOMPARE(signedValue(build(QString(128, QLatin1Char('a')), QStringLiteral("hex"))).size(), 64);
        QVERIFY(fails(build(QString(130, QLatin1Char('a')), QStringLiteral("hex"))));
        QCOMPARE(signedValue(build(QString::fromLatin1(QByteArray(64, 'x').toBase64()),
                                   QStringLiteral("base64"))).size(), 64);
        QVERIFY(fails(build(QString::fromLatin1(QByteArray(65, 'x').toBase64()), QStringLiteral("base64"))));

        // Strict: never a nearby value.
        for (const char* hex : {"0g", "abc", "0xab", " 00ff", "00ff "})
            QVERIFY2(fails(build(QString::fromLatin1(hex), QStringLiteral("hex"))), hex);
        for (const char* b64 : {"AP8", "AP9=", "AP8=\n", "AP-_", "AP8 Q", "AP8Q=", "A"})
            QVERIFY2(fails(build(QString::fromLatin1(b64), QStringLiteral("base64"))), b64);
        for (const char* format : {"binary", "HEX", "utf8"})
            QVERIFY2(fails(build(QStringLiteral("00"), QString::fromLatin1(format))), format);
        QVERIFY(fails(build(QStringLiteral("00"), 16)));
        QVERIFY(fails(build(QStringLiteral("00"), QVariant())));      // present, unreadable
        QVERIFY(fails(build(QStringLiteral("E") + QChar(0xD800), QStringLiteral("text"))));

        // The table advertises the field, so a form draws it.
        bool listed = false;
        for (const QVariant& field : builder.fieldsFor(QStringLiteral("manageData")))
            listed = listed || field.toMap().value(QStringLiteral("name")) == QStringLiteral("valueFormat");
        QVERIFY(listed);
        QCOMPARE(builder.demoValueFor(QStringLiteral("manageData"), QStringLiteral("valueFormat")),
                 QStringLiteral("text"));
    }

    /**
     * Tx.inspect gave a fee bump no txrep ("SEP-11 covers plain transactions
     * only") and nothing else but its hash and signature count: neither the
     * account paying nor what it pays. It shows the fee bump's own document
     * now, its M... fee source and fee, and the transaction it wraps under
     * inner names only, never as the envelope's; a V0 envelope's document
     * says V0. The envelopes are py-stellar-sdk 14.1.0's (see
     * SepsQmlTest::testTxrepCoversFeeBumpsAndV0Envelopes).
     */
    void testInspectShowsAFeeBumpAsTheFeeBump()
    {
        StellarQml facade;
        TxBuilderQml builder(facade.assetRegistry());
        QVariantMap args;
        args.insert(QStringLiteral("xdr"), QStringLiteral(
            "AAAABQAAAQAAAAAAAAAAB4E5dw6ofRdfVqNUZsNMfszLjYqRtO43ol32D1uPybOUAAAAAAAAAZAAAAACAAAAAIqI4910CfGV"
            "/VLbLTy6XXLKZwm/HZQSG/N0iAG0D29cAAAAZAAAAAAAAAAqAAAAAQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAQAAAAAAAAAB"
            "AAAAAO1JKMYo0cLG6ukDOJBZlWEpWSc6XGP5NjbBRhSshzfRAAAAAAAAAAAGQixAAAAAAAAAAAG0D29cAAAAQI2/3jIjD1H5"
            "ce57vcEOy3hOtWHXiQestw8B0DJ8m9efXeM5DdW9yUUNGrQ2O90HQN6v5DqVpjVYWoms5YsDSgwAAAAAAAAAAY/Js5QAAABA"
            "3f2Fe9cqiDtzQAeb84wyJ6MxeVbBu7J/qt69awr9Dpyr7AT6qQ2iZJSdRfhOg1DJQfvLpiv7rfaMrAV4VS7cAA=="));
        const QVariantMap seen = builder.inspect(args);
        QVERIFY2(!seen.contains(QStringLiteral("error")),
                 qPrintable(seen.value(QStringLiteral("error")).toString()));
        QVERIFY(seen.value(QStringLiteral("isFeeBump")).toBool());
        QCOMPARE(seen.value(QStringLiteral("envelopeType")).toString(),
                 QStringLiteral("ENVELOPE_TYPE_TX_FEE_BUMP"));
        QVERIFY(!seen.contains(QStringLiteral("txrepUnavailable")));
        const QString txrep = seen.value(QStringLiteral("txrep")).toString();
        QVERIFY2(txrep.startsWith(QStringLiteral(
            "type: ENVELOPE_TYPE_TX_FEE_BUMP\n"
            "feeBump.tx.feeSource: MCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZIAAAAAAAAAAAA42ZW "
            "(accountID: GCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZI55U, accountMuxedID: 7)\n"
            "feeBump.tx.fee: 400\n"
            "feeBump.tx.innerTx.type: ENVELOPE_TYPE_TX\n")), qPrintable(txrep));
        QVERIFY(txrep.contains(QStringLiteral("\nfeeBump.tx.innerTx.signatures.len: 1\n")));
        QVERIFY(txrep.contains(QStringLiteral("\nfeeBump.signatures.len: 1\nfeeBump.signatures[0].hint: 8fc9b394\n")));
        QCOMPARE(seen.value(QStringLiteral("hash")).toString(),
                 QStringLiteral("5b0ae1961848750e3cdd6e2aad7453832c502a8b6c589c937ac9552f4bdcf828"));
        QCOMPARE(seen.value(QStringLiteral("feeSource")).toString(),
                 QStringLiteral("MCATS5YOVB6ROX2WUNKGNQ2MP3GMXDMKSG2O4N5CLX3A6W4PZGZZIAAAAAAAAAAAA42ZW"));
        QCOMPARE(seen.value(QStringLiteral("fee")).toLongLong(), qint64(400));
        QCOMPARE(seen.value(QStringLiteral("signatureCount")).toInt(), 1);
        QCOMPARE(seen.value(QStringLiteral("innerHash")).toString(),
                 QStringLiteral("94d9731476424e3d6ed9cf22be801da0ef8c40961741db0fc836084acbedd8d5"));
        QCOMPARE(seen.value(QStringLiteral("innerSourceAccount")).toString(),
                 QStringLiteral("GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR"));
        QCOMPARE(seen.value(QStringLiteral("innerSequence")).toString(), QStringLiteral("42"));
        QCOMPARE(seen.value(QStringLiteral("innerFee")).toLongLong(), qint64(100));
        QCOMPARE(seen.value(QStringLiteral("innerSignatureCount")).toInt(), 1);
        QCOMPARE(seen.value(QStringLiteral("operationCount")).toInt(), 1);
        QVERIFY(!seen.contains(QStringLiteral("sourceAccount")));
        QVERIFY(!seen.contains(QStringLiteral("sequence")));

        args.insert(QStringLiteral("xdr"), QStringLiteral(
            "AAAAAIqI4910CfGV/VLbLTy6XXLKZwm/HZQSG/N0iAG0D29cAAAAZAAAAAAAAAAqAAAAAQAAAAAAAAAAAAAAAGVT8QAAAAAB"
            "AAAAB3YwIG1lbW8AAAAAAQAAAAAAAAABAAAAAO1JKMYo0cLG6ukDOJBZlWEpWSc6XGP5NjbBRhSshzfRAAAAAAAAAAAGQixA"
            "AAAAAAAAAAG0D29cAAAAQAA0JBydFq75kf//2uWARGv7Mc4va3iifTCOf8v52Yjp73eMZ/eyk3/o0TiWmoiddDGtGydVyttN"
            "HDuv9ULWlQs="));
        const QVariantMap v0 = builder.inspect(args);
        QVERIFY(!v0.value(QStringLiteral("isFeeBump")).toBool());
        QCOMPARE(v0.value(QStringLiteral("envelopeType")).toString(), QStringLiteral("ENVELOPE_TYPE_TX_V0"));
        QCOMPARE(v0.value(QStringLiteral("hash")).toString(),
                 QStringLiteral("3715eaccd49cb1c4aedd626b31c1daf23197f3d2327b3cb2bf68ee8f5b4838a3"));
        const QString v0txrep = v0.value(QStringLiteral("txrep")).toString();
        QVERIFY2(v0txrep.startsWith(QStringLiteral("type: ENVELOPE_TYPE_TX_V0\n")), qPrintable(v0txrep));
        QVERIFY(v0txrep.contains(QStringLiteral("\nsignatures.len: 1\nsignatures[0].hint: b40f6f5c\n")));
        QCOMPARE(v0.value(QStringLiteral("sourceAccount")).toString(),
                 QStringLiteral("GCFIRY65OQE7DFP5KLNS2PF2LVZMUZYJX4OZIEQ36N2IQANUB5XVYOJR"));
        QCOMPARE(v0.value(QStringLiteral("sequence")).toString(), QStringLiteral("42"));
        QCOMPARE(v0.value(QStringLiteral("signatureCount")).toInt(), 1);
    }
};

ADD_TEST(TxBuilderQmlTest)
#endif // TXBUILDERQMLTEST_H
