#include "claimant.h"
#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <stdexcept>
#include "responses/response.h"
#include "qtcompat.h"
static void registerTypes()
{
    using namespace qstellar;
    regType<QList<Claimant> >();
    QMetaType::registerConverter<QVariantList, QList<Claimant> >(&ClaimantConverter::convertData);
}
Q_COREAPP_STARTUP_FUNCTION(registerTypes)

QSTELLAR_BEGIN_NS

Claimant::Claimant(QString destination, Predicate *predicate): m_destination(destination), m_predicate(predicate)
{
 if(!predicate)
     throw std::runtime_error("predicate can't be null");
}

Claimant::Claimant():m_predicate(nullptr)
{

}

Claimant::Claimant(const Claimant &other)
{
    this->m_destination=other.m_destination;
    // The default ctor leaves m_predicate null; copying such a Claimant
    // (which happens on its own when it goes into a QList or through a
    // QVariant) dereferenced a null pointer.
    this->m_predicate=other.m_predicate ? Predicate::create(*(other.m_predicate)) : nullptr;
}

Claimant &Claimant::operator=(const Claimant &other)
{
    if (this == &other)
        return *this;
    // Copied before what it holds is freed, as And/Or assignment does: a
    // copy throws for a predicate Predicate::create does not know, also
    // nested in and/or, and this deleted first, left m_predicate pointing at
    // the freed one, and the destructor deleted it again. A throw now leaves
    // the claimant as it was.
    Predicate* copy = other.m_predicate ? Predicate::create(*(other.m_predicate)) : nullptr;
    delete m_predicate;
    m_destination = other.m_destination;
    m_predicate = copy;
    return *this;
}

Claimant::~Claimant(){
    if(m_predicate)
        delete m_predicate;
}

QString Claimant::getDestination() const
{
    return m_destination;
}

const Predicate& Claimant::getPredicate() const
{
    // A claimant made with no predicate (the default constructor) can claim
    // unconditionally, as js and py read it. Both of these dereferenced null.
    static const Predicate::Unconditional unconditional;
    if (m_predicate)
        return *m_predicate;
    return unconditional;
}

stellar::Claimant Claimant::toXdr() const
{
    stellar::Claimant c;
    c.type = stellar::ClaimantType::CLAIMANT_TYPE_V0;
    c.v0.destination = StrKey::encodeToXDRAccountId(m_destination);
    c.v0.predicate = getPredicate().toXdr();
    return c;
}

Claimant Claimant::fromXdr(stellar::Claimant c)
{
    return Claimant(StrKey::encodeStellarAccountId(c.v0.destination)
                    ,Predicate::fromXdr(c.v0.predicate));
}

bool Claimant::operator==(const Claimant &other) const
{
    return m_destination==other.m_destination && m_predicate && other.m_predicate && m_predicate->equals(other.m_predicate);
}

bool Claimant::operator!=(const Claimant &other) const
{
    return m_destination!=other.m_destination || !m_predicate || !other.m_predicate || !m_predicate->equals(other.m_predicate);
}

void Claimant::setDestination(QString destination)
{
    if (m_destination == destination)
        return;

    m_destination = destination;
}

void Claimant::setPredicate(Predicate &predicate)
{
    // Only for a predicate with the same XDR: equals() compares and/or
    // operands in order. It used to ignore their order, and this kept (and
    // toXdr() signed) and(a, b) when the caller set and(b, a).
    if (m_predicate && m_predicate->equals(&predicate))
        return;

    // The predicate it replaced was never freed.
    Predicate* copy = Predicate::create(predicate);
    delete m_predicate;
    m_predicate = copy;
}

namespace {

[[noreturn]] void refuse(const QString& field, const QString& why)
{
    throw std::runtime_error((field.isEmpty() ? why : field + QStringLiteral(": ") + why).toStdString());
}

QString member(const QString& path, const QString& key)
{
    return path.isEmpty() ? key : path + QLatin1Char('.') + key;
}

/** A JSON value as an error message shows it (as the Horizon readers do). */
QString describe(const QJsonValue& value)
{
    switch (value.type()) {
    case QJsonValue::Null: return QStringLiteral("null");
    case QJsonValue::Bool: return value.toBool() ? QStringLiteral("true") : QStringLiteral("false");
    case QJsonValue::Double:
        return QStringLiteral("the number ") + QString::number(value.toDouble(), 'g', 17);
    case QJsonValue::String: {
        QString text = value.toString();
        if (text.size() > 40)
            text = text.left(40) + QStringLiteral("...");
        return QStringLiteral("the text \"") + text + QLatin1Char('"');
    }
    case QJsonValue::Array: return QStringLiteral("an array");
    case QJsonValue::Object: return QStringLiteral("an object");
    default: return QStringLiteral("nothing");
    }
}

/** A G... account id, in its one canonical spelling. */
bool isAccountId(const QString& id)
{
    try {
        return id.size() == 56 && StrKey::decodeVersionByte(id) == StrKey::VersionByte::ACCOUNT_ID
               && StrKey::encodeStellarAccountId(StrKey::decodeStellarAccountId(id)) == id;
    } catch (const std::exception&) {
        return false;
    }
}

} // namespace

QList<Claimant> ClaimantConverter::convertData(const QVariantList &source)
{
    // Each item used to be read with toMap() and toString(): an item that
    // was no object, a destination that was no account id and any other key
    // all went through, and the predicate was read leniently.
    return fromJson(QJsonArray::fromVariantList(source));
}

QList<Claimant> ClaimantConverter::fromJson(const QJsonValue &json, const QString &field)
{
    if (!json.isArray())
        refuse(field, QStringLiteral("expected an array, found ") + describe(json));
    QList<Claimant> claimants;
    const QJsonArray array = json.toArray();
    for (int i = 0; i < array.size(); ++i) {
        const QString item = field + QLatin1Char('[') + QString::number(i) + QLatin1Char(']');
        if (!array.at(i).isObject())
            refuse(item, QStringLiteral("expected an object, found ") + describe(array.at(i)));
        const QJsonObject claimant = array.at(i).toObject();
        const QStringList keys = claimant.keys();
        for (const QString& key : keys) {
            if (key != QLatin1String("destination") && key != QLatin1String("predicate"))
                refuse(member(item, key), QStringLiteral("not part of a claimant"));
        }
        const QJsonValue destination = claimant.value(QStringLiteral("destination"));
        const QString destinationField = member(item, QStringLiteral("destination"));
        if (!destination.isString() || destination.toString().isEmpty())
            refuse(destinationField, destination.isUndefined()
                                         ? QStringLiteral("missing")
                                         : QStringLiteral("expected text, found ") + describe(destination));
        if (!isAccountId(destination.toString()))
            refuse(destinationField, QStringLiteral("\"%1\" is not an account id").arg(destination.toString()));
        claimants.append(Claimant(destination.toString(),
                                  Predicate::fromJson(claimant.value(QStringLiteral("predicate")),
                                                      member(item, QStringLiteral("predicate")))));
    }
    return claimants;
}

QList<Claimant> checkNotNull(QList<Claimant> claimants, const char *error)
{
    if(claimants.isEmpty())
        throw std::runtime_error(error);
    for(auto c:claimants)
    {
        if(c.getDestination().isNull())
            throw std::runtime_error(error);
    }
    return claimants;
}

QSTELLAR_END_NS
