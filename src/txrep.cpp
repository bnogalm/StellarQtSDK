#include "txrep.h"

#include <QHash>
#include <QStringList>
#include <QTextStream>
#include <stdexcept>

#include "transaction.h"
#include "transactionbuilder.h"
#include "network.h"
#include "keypair.h"
#include "account.h"
#include "memo.h"
#include "managedataoperation.h"
#include "paymentoperation.h"
#include "asset.h"
#include "assettypenative.h"
#include "assettypecreditalphanum4.h"
#include "assettypecreditalphanum12.h"
#include "timebounds.h"

QSTELLAR_BEGIN_NS


namespace {

void appendLine(QString& out, const QString& key, const QString& value)
{
    out.append(key).append(": ").append(value).append('\n');
}

QString hex(const QByteArray& bytes) { return QString::fromLatin1(bytes.toHex()); }

QString quoted(const QString& s)
{
    QString escaped = s;
    escaped.replace('\\', QStringLiteral("\\\\"));
    escaped.replace('"', QStringLiteral("\\\""));
    return QString("\"%1\"").arg(escaped);
}

QString assetToTxrep(Asset* a)
{
    if (!a) throw std::runtime_error("asset cannot be null");
    if (dynamic_cast<AssetTypeNative*>(a)) return QStringLiteral("XLM");
    auto* alpha = dynamic_cast<AssetTypeCreditAlphaNum*>(a);
    if (!alpha) throw std::runtime_error("unsupported asset type");
    return alpha->getCode() + ":" + alpha->getIssuer().getAccountId();
}

Asset* assetFromTxrep(const QString& s)
{
    if (s == "XLM" || s == "native") return new AssetTypeNative();
    int colon = s.indexOf(':');
    if (colon < 0) throw std::runtime_error("malformed asset in txrep");
    return Asset::create(s.left(colon), s.mid(colon + 1));
}

void emitOperation(QString& out, Operation* op, int i)
{
    const QString prefix = QString("tx.operations[%1]").arg(i);
    if (op->getSourceAccount().isEmpty()) {
        appendLine(out, prefix + ".sourceAccount._present", "false");
    } else {
        appendLine(out, prefix + ".sourceAccount._present", "true");
        appendLine(out, prefix + ".sourceAccount", op->getSourceAccount());
    }

    if (auto* md = dynamic_cast<ManageDataOperation*>(op)) {
        appendLine(out, prefix + ".body.type", "MANAGE_DATA");
        appendLine(out, prefix + ".body.manageDataOp.dataName", quoted(md->getName()));
        const QByteArray value = md->getValue();
        if (value.isEmpty()) {
            appendLine(out, prefix + ".body.manageDataOp.dataValue._present", "false");
        } else {
            appendLine(out, prefix + ".body.manageDataOp.dataValue._present", "true");
            appendLine(out, prefix + ".body.manageDataOp.dataValue", hex(value));
        }
    } else if (auto* pay = dynamic_cast<PaymentOperation*>(op)) {
        appendLine(out, prefix + ".body.type", "PAYMENT");
        appendLine(out, prefix + ".body.paymentOp.destination", pay->getDestination());
        appendLine(out, prefix + ".body.paymentOp.asset", assetToTxrep(pay->getAsset()));
        appendLine(out, prefix + ".body.paymentOp.amount",
                   QString::number(Operation::toXdrAmount(pay->getAmount())));
    } else {
        throw std::runtime_error("Txrep emitter: unsupported operation type");
    }
}

QString unquote(const QString& s)
{
    if (s.size() < 2 || s.at(0) != '"' || s.at(s.size() - 1) != '"') {
        throw std::runtime_error("malformed quoted string in txrep");
    }
    QString body = s.mid(1, s.size() - 2);
    body.replace(QStringLiteral("\\\""), QStringLiteral("\""));
    body.replace(QStringLiteral("\\\\"), QStringLiteral("\\"));
    return body;
}

const QString& need(const QHash<QString, QString>& kv, const QString& key)
{
    auto it = kv.constFind(key);
    if (it == kv.constEnd()) {
        throw std::runtime_error(("missing txrep key: " + key).toStdString());
    }
    return it.value();
}

Operation* parseOperation(const QHash<QString, QString>& kv, int i)
{
    const QString prefix = QString("tx.operations[%1]").arg(i);
    const QString type = need(kv, prefix + ".body.type");

    Operation* op = nullptr;
    if (type == "MANAGE_DATA") {
        QString name = unquote(need(kv, prefix + ".body.manageDataOp.dataName"));
        const bool present = need(kv, prefix + ".body.manageDataOp.dataValue._present") == "true";
        QByteArray value;
        if (present) {
            value = QByteArray::fromHex(need(kv, prefix + ".body.manageDataOp.dataValue").toLatin1());
        }
        op = new ManageDataOperation(name, value);
    } else if (type == "PAYMENT") {
        QString dest   = need(kv, prefix + ".body.paymentOp.destination");
        Asset* asset   = assetFromTxrep(need(kv, prefix + ".body.paymentOp.asset"));
        qint64 stroops = need(kv, prefix + ".body.paymentOp.amount").toLongLong();
        op = new PaymentOperation(dest, asset, Operation::fromXdrAmount(stroops));
    } else {
        throw std::runtime_error(("Txrep parser: unsupported operation type: " + type).toStdString());
    }

    if (kv.value(prefix + ".sourceAccount._present") == "true") {
        op->setSourceAccount(need(kv, prefix + ".sourceAccount"));
    }
    return op;
}

} // namespace

QString Txrep::toTxrep(Transaction* transaction)
{
    if (!transaction) {
        throw std::runtime_error("transaction cannot be null");
    }

    QString out;
    appendLine(out, "type", "ENVELOPE_TYPE_TX");
    appendLine(out, "tx.sourceAccount", transaction->getSourceAccount());
    appendLine(out, "tx.fee", QString::number(transaction->getFee()));
    appendLine(out, "tx.seqNum", QString::number(transaction->getSequenceNumber()));

    // Preconditions: PRECOND_NONE / PRECOND_TIME only in this release.
    const TransactionPreconditions& pre = transaction->getPreconditions();
    if (pre.hasV2()) {
        throw std::runtime_error("Txrep emitter: PRECOND_V2 not yet supported");
    }
    TimeBounds* tb = pre.getTimeBounds();
    if (tb) {
        appendLine(out, "tx.cond.type", "PRECOND_TIME");
        appendLine(out, "tx.cond.timeBounds.minTime", QString::number(tb->getMinTime()));
        appendLine(out, "tx.cond.timeBounds.maxTime", QString::number(tb->getMaxTime()));
    } else {
        appendLine(out, "tx.cond.type", "PRECOND_NONE");
    }

    // Memo: NONE only.
    Memo* memo = transaction->getMemo();
    if (memo && !dynamic_cast<MemoNone*>(memo)) {
        throw std::runtime_error("Txrep emitter: only MEMO_NONE is supported in this release");
    }
    appendLine(out, "tx.memo.type", "MEMO_NONE");

    QVector<Operation*> ops = transaction->getOperations();
    appendLine(out, "tx.operations.len", QString::number(ops.size()));
    for (int i = 0; i < ops.size(); ++i) {
        emitOperation(out, ops.at(i), i);
    }

    appendLine(out, "tx.ext.v", "0");

    appendLine(out, "signatures.len", "0");
    return out;
}

Transaction* Txrep::fromTxrep(const QString& txrep, Network* network)
{
    if (!network) throw std::runtime_error("network cannot be null");

    // Parse the dotted-path key/value lines into a flat map.
    QHash<QString, QString> kv;
    for (const QString& rawLine : txrep.split('\n')) {
        QString line = rawLine.trimmed();
        if (line.isEmpty() || line.startsWith('#')) continue;
        int sep = line.indexOf(':');
        if (sep < 0) continue;
        kv.insert(line.left(sep).trimmed(), line.mid(sep + 1).trimmed());
    }

    if (kv.value("type") != "ENVELOPE_TYPE_TX") {
        throw std::runtime_error("Txrep parser: only ENVELOPE_TYPE_TX is supported");
    }
    if (kv.value("tx.memo.type") != "MEMO_NONE") {
        throw std::runtime_error("Txrep parser: only MEMO_NONE is supported");
    }

    QString sourceAccount = need(kv, "tx.sourceAccount");
    qint64 fee = need(kv, "tx.fee").toLongLong();
    qint64 seqNum = need(kv, "tx.seqNum").toLongLong();

    // Account starts at seqNum-1 so that build() increments it to seqNum.
    KeyPair* kp = KeyPair::fromAccountId(sourceAccount);
    Account* account = new Account(kp, seqNum - 1);

    TransactionBuilder builder(AccountConverter().enableMuxed(), account, network);
    const qint64 opCount = need(kv, "tx.operations.len").toLongLong();
    builder.setBaseFee(opCount > 0 ? static_cast<quint32>(fee / opCount) : TransactionBuilder::BASE_FEE);

    QString condType = kv.value("tx.cond.type", "PRECOND_NONE");
    if (condType == "PRECOND_TIME") {
        qint64 minT = need(kv, "tx.cond.timeBounds.minTime").toLongLong();
        qint64 maxT = need(kv, "tx.cond.timeBounds.maxTime").toLongLong();
        builder.addTimeBounds(new TimeBounds(minT, maxT));
    } else if (condType == "PRECOND_NONE") {
        builder.setTimeout(TransactionBuilder::TIMEOUT_INFINITE);
    } else {
        delete account;
        throw std::runtime_error(("Txrep parser: unsupported cond type: " + condType).toStdString());
    }

    builder.addMemo(Memo::none());
    for (int i = 0; i < opCount; ++i) {
        builder.addOperation(parseOperation(kv, i));
    }

    Transaction* tx = builder.build();
    delete account; // builder doesn't own it
    return tx;
}
QSTELLAR_END_NS
