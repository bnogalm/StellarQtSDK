# StellarQtSDK

[![CI](https://github.com/bnogalm/StellarQtSDK/actions/workflows/ci.yml/badge.svg)](https://github.com/bnogalm/StellarQtSDK/actions/workflows/ci.yml)

Stellar SDK for Qt / C++. Builds against Qt 6 (Qt 5.15 still works).
Interface to drive the Horizon REST API and the Soroban RPC API.

## What it covers

- **Horizon** (`Server`): every request builder, pages and streams;
  `submitTransaction`, `submitTransactionAsync` and the SEP-29
  memo-required check on its own (`checkMemoRequired`).
- **Soroban RPC** (`SorobanServer`): simulate (with
  `SimulateTransactionOptions`), prepare, send and poll;
  `ContractClient` / `AssembledTransaction`; a contract's interface read
  from its `.wasm` (`ContractSpec`).
- **Keys**: `KeyPair`, StrKeys of every kind (`StrKey::decodeAny`),
  BIP-39 phrases in eight languages and SEP-5 / SLIP-10 derivation
  (`KeyPair::fromMnemonic`, `fromBip39Seed`), SEP-53 message signing
  (`KeyPair::signMessage` / `verifyMessage`).
- **SEPs**: SEP-1 (`StellarToml`, `StellarTomlResolver`), SEP-2
  (`Federation`, `FederationServer`), SEP-7 (`Sep7Pay`, `Sep7Tx`, the
  origin check `Sep7OriginCheck`), SEP-10, SEP-11 TxRep (`Txrep`),
  SEP-29, SEP-35 (`TOID`), SEP-45.

## Adding to your project

In your `.pro`:

```pro
include($$PWD/StellarQtSDK/StellarQtSDK.pri)
```

The SDK requires C++17 (`CONFIG += c++17`).

With CMake 3.24 or newer, add the source tree or install it, and link the
same target either way:

```cmake
add_subdirectory(StellarQtSDK)        # or: find_package(StellarQtSDK 1.0 REQUIRED)
target_link_libraries(yourapp PRIVATE StellarQtSDK::StellarQtSDK)
```

Link `StellarQtSDK::StellarQtSDK`, not the bare `StellarQtSDK` archive. It
links the archive whole-archive; without that, Horizon responses deserialise
with empty lists. Installed headers live under `include/StellarQtSDK/` and are
included the same way as in the tree (`#include "keypair.h"`). A Debug build
installs as `StellarQtSDKd`.

### QML layer

An optional QML layer (`src/qml/`) exposes the SDK to QML as the `Stellar`,
`Tx`, `Horizon`, `Soroban`, `Seps` and `Xdr` singletons: no raw pointers and
no exceptions cross into QML, failures come back as values. It is off by
default, because it adds a QtQml dependency. Enable it before including the
`.pri`:

```pro
DEFINES += STELLAR_QT_QML
include($$PWD/StellarQtSDK/StellarQtSDK.pri)
```

or configure CMake with `-DSTELLAR_QT_QML=ON`, then call
`QStellarQml::registerTypes()` once before loading QML
(`import QStellar 1.0`). See `src/qml/qstellarqml.h`.

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
