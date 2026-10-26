#ifndef QSTELLAR_QSTELLARQML_H
#define QSTELLAR_QSTELLARQML_H

#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * Entry point of the optional QML layer.
 *
 * The whole layer is compiled only when STELLAR_QT_QML is defined, because it
 * pulls in QtQml, which a plain C++ consumer of the SDK has no reason to link:
 *
 *     DEFINES += STELLAR_QT_QML
 *     include(StellarQtSDK/StellarQtSDK.pri)
 *
 * Then, once a QQmlEngine exists:
 *
 *     QStellarQml::registerTypes();
 *
 * and QML sees a `Stellar` singleton:
 *
 *     import QStellar 1.0
 *     TextField {
 *         onTextChanged: error = Stellar.validateAmount(text)
 *     }
 *
 * What this layer is, and is not: it makes the SDK's types USABLE from QML —
 * invokable methods, string-keyed factories, no pointers and no exceptions
 * crossing the boundary. It deliberately holds no application logic: submission
 * state machines, wizards and the like belong to the app, not here.
 */
class QStellarQml
{
public:
    /**
     * Registers the QML types under `uri`. Safe to call more than once.
     * Must be called after a QCoreApplication exists and before the QML that
     * uses it is loaded.
     */
    static void registerTypes(const char* uri = "QStellar",
                              int versionMajor = 1,
                              int versionMinor = 0);
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(QStellarQml)

#endif // QSTELLAR_QSTELLARQML_H
