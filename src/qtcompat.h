#ifndef QTCOMPAT_H
#define QTCOMPAT_H

// Central Qt 5 / Qt 6 source-compatibility shims for the SDK.
//
// Keep every "this API differs between Qt versions" workaround here, behind a
// single QT_VERSION_CHECK, so call sites stay clean and there is one place to
// look (and to delete from, when the minimum Qt version is raised).

#include <QtGlobal>
#include <QJsonValue>
#include <QVariant>
#include <QMetaType>
#include <QByteArray>
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * Read a JSON number as a 64-bit integer, portably across Qt versions.
 *
 * QJsonValue::toInteger() was added in Qt 6.0 and is exact for integral values.
 * Qt 5 has no integer accessor — it stores every JSON number as a double — so
 * there we fall back to toDouble(); that is the best precision Qt 5 can offer
 * (values above 2^53 cannot be represented exactly there regardless).
 *
 * Strings are NOT parsed (this mirrors toInteger): callers that also accept
 * string-encoded numbers must check isString() themselves first.
 */
inline qint64 jsonToInt64(const QJsonValue& v, qint64 defaultValue = 0)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return v.toInteger(defaultValue);
#else
    return v.isDouble()
        ? static_cast<qint64>(v.toDouble(static_cast<double>(defaultValue)))
        : defaultValue;
#endif
}

/**
 * Build a typed-but-null QVariant of the given metatype: QVariant::isNull()
 * stays true until the variant is assigned, while its type is already known.
 * The SDK uses this so the reflection-based JSON filler populates the field
 * (an untyped null QVariant would be left untouched).
 *
 * QVariant(QMetaType) is Qt 6.0+; Qt 5 uses the QVariant(QMetaType::Type) ctor.
 */
inline QVariant nullVariant(QMetaType::Type type)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return QVariant(QMetaType(type));
#else
    // Qt 5.15 explicitly DELETES QVariant(QMetaType::Type); the live overload
    // that builds a null variant of a type is QVariant(QVariant::Type). For the
    // core metatypes the SDK uses here (Bool), QVariant::Type and QMetaType::Type
    // share enum values, so the cast is valid.
    return QVariant(static_cast<QVariant::Type>(type));
#endif
}

/**
 * Register T as a Qt metatype and, on Qt 5, ALSO alias it under the name moc
 * stores for a property of type T — i.e. with the enclosing "qstellar::"
 * namespace stripped (including inside QList<...>). Qt 5 resolves a property's
 * type by string name in QMetaProperty::read, and moc records that name without
 * the namespace, while Q_DECLARE_METATYPE/qRegisterMetaType register the fully
 * qualified name — so without the alias the lookup fails and the property is
 * left default. On Qt 6 this is just qRegisterMetaType<T>() (types resolve by
 * value, and the string-name qRegisterMetaType overload was removed).
 *
 * NOTE: for a type referenced by its BARE class name inside its own namespace
 * (e.g. TimeBounds/LedgerBounds inside Preconditions, both in
 * TransactionResponseAttach), moc stores just "TimeBounds"; register that bare
 * alias explicitly with qRegisterMetaType<T>("TimeBounds") in addition to this.
 */
template <typename T>
inline void regType()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    qRegisterMetaType<T>();
#else
    const int id = qRegisterMetaType<T>();
    QByteArray mocName(QMetaType::typeName(id));
    mocName.replace("qstellar::", "");
    if (!mocName.isEmpty() && QMetaType::type(mocName.constData()) == QMetaType::UnknownType)
        qRegisterMetaType<T>(mocName.constData());
#endif
}

QSTELLAR_END_NS

#endif // QTCOMPAT_H
