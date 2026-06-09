# StellarQtSDK

Stellar SDK for Qt / C++. Builds against Qt 6 (Qt 5.15 still works).
Interface to drive the Horizon REST API and the Soroban RPC API.

## Adding to your project

In your `.pro`:

```pro
include($$PWD/StellarQtSDK/StellarQtSDK.pri)
```

The SDK requires C++17 (`CONFIG += c++17`).

## Namespace

Every public class lives in `qstellar::`. New code should use the qualified
names:

```cpp
#include <keypair.h>
#include <server.h>

qstellar::KeyPair* kp = qstellar::KeyPair::random();
qstellar::Server* srv = new qstellar::Server("https://horizon-testnet.stellar.org");
```

If your project was written against the 0.x unqualified API and you don't
want to qualify every call site, define `STELLAR_QT_LEGACY_GLOBAL_NS` and
the SDK will additionally emit global `using` aliases for every public
class:

```pro
# In your .pro:
DEFINES += STELLAR_QT_LEGACY_GLOBAL_NS
```

With the flag, both `KeyPair` and `qstellar::KeyPair` resolve. Without
it, only the qualified spelling is exported.

The bundled test suite and the two `examples/` projects build with the
flag defined (their code uses the unqualified API). New consumers should
not define it.

## Examples

Two runnable examples are included under `examples/`. The
[wiki](https://github.com/bnogalm/StellarQtSDK/wiki) walks through setting
up the environment and writing a first app.

## Changelog

See [CHANGELOG.md](CHANGELOG.md).
