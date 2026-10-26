#include "qstellarqml.h"

#include <QQmlEngine>
#include <QtGlobal>

#include "horizonqml.h"
#include "sepsqml.h"
#include "xdrqml.h"
#include "sorobanqml.h"
#include "stellarqml.h"
#include "txbuilderqml.h"

QSTELLAR_BEGIN_NS

void QStellarQml::registerTypes(const char* uri, int versionMajor, int versionMinor)
{
    static bool registered = false;
    if (registered)
        return;
    registered = true;

    // One instance owned by the SDK, handed to QML as a singleton. It holds the
    // AssetRegistry, so asset pointers stay valid for the process lifetime and
    // QML never has to think about ownership.
    static StellarQml* facade = new StellarQml();
    qmlRegisterSingletonInstance(uri, versionMajor, versionMinor, "Stellar", facade);

    // Transaction building is a distinct concern from the stateless helpers,
    // and it shares the facade's AssetRegistry so an asset built here is the
    // same instance the rest of the app resolved.
    static TxBuilderQml* builder = new TxBuilderQml(facade->assetRegistry());
    qmlRegisterSingletonInstance(uri, versionMajor, versionMinor, "Tx", builder);

    // Everything that touches a network. Kept apart from the other two on
    // purpose: Stellar and Tx are pure and instant, this one is neither, and a
    // UI should be able to see at a glance which calls can fail or block.
    static HorizonQml* horizon = new HorizonQml(facade->assetRegistry());
    qmlRegisterSingletonInstance(uri, versionMajor, versionMinor, "Horizon", horizon);

    // Soroban speaks a different protocol to a different server, and unlike the
    // other three it does its work on a thread of its own: the SDK's RPC client
    // is synchronous, and calling it from a QML handler would freeze the window.
    static SorobanQml* soroban = new SorobanQml();
    qmlRegisterSingletonInstance(uri, versionMajor, versionMinor, "Soroban", soroban);

    // The SEPs the SDK implements. All of them offline, so no thread and no
    // request ids: SEP-1 parses a stellar.toml it is handed, SEP-7 builds,
    // reads, signs and verifies a URI, and neither fetches anything.
    static SepsQml* seps = new SepsQml();
    qmlRegisterSingletonInstance(uri, versionMajor, versionMinor, "Seps", seps);

    // Reading raw XDR. Offline like the SEPs, and deliberately a short menu:
    // several XDR types in this SDK have a reader that silently skips the arms
    // it does not know, and offering those would look like it worked.
    static XdrQml* xdr = new XdrQml();
    qmlRegisterSingletonInstance(uri, versionMajor, versionMinor, "Xdr", xdr);
}

QSTELLAR_END_NS
