# Changelog

## 1.0.1

Patch release: correctness/precision fixes, response-field parity with
`java-stellar-sdk`, and two crash fixes. **No breaking API changes** — only
additive getters, fields and classes. (Pulls forward several roadmap-1.0.2
P1 items.)

### Fixed

- **Exact rational price (`price_r`) was silently dropped.** Offer / LP
  responses kept only the rounded decimal `price` and discarded the exact
  `{n,d}`. Added `getPriceR()` to `ManageSellOffer` / `ManageBuyOffer` /
  `CreatePassiveSellOffer` operation responses and `OfferResponse`;
  `getMinPriceR()` / `getMaxPriceR()` to `LiquidityPoolDepositOperationResponse`;
  `getHighR()` / `getLowR()` / `getOpenR()` / `getCloseR()` to
  `TradeAggregationResponse`.
- **Crash on liquidity-pool-share balances** — `Balance::getAsset()` /
  `AssetResponse::getAsset()` threw for `asset_type == "liquidity_pool_shares"`;
  they now return `nullptr` (use `getLiquidityPoolId()`).
- **Crash on Soroban contract effects** — the effect dispatcher threw on
  `contract_credited` (96) / `contract_debited` (97); added
  `ContractCreditedEffectResponse` / `ContractDebitedEffectResponse`.
- `TradeResponse`: added `liquidity_pool_fee_bp`, `trade_type`,
  `base_liquidity_pool_id`, `counter_liquidity_pool_id`.
- **`Price(QString)` silently corrupted out-of-range prices** — the
  numerator was computed in `qint64` and narrowed to `qint32` without a
  check, so a price that did not fit an int32 fraction produced a wrong
  offer price. It now throws instead of truncating.
- **`Operation::toXdrAmount` mis-parsed amounts under some locales** — it did
  a blind `','→'.'` replace (breaking thousands-grouped input), skipped
  validation, and could overflow `qint64` silently. Rewritten with strict
  decimal parsing, MSVC-safe overflow checks, and correct sign handling.
- **Build fix:** `Predicate` used `QTimeZone::UTC` (Qt 6.5+) unconditionally;
  now guarded for Qt < 6.5.

### Added (response-field parity)

- **CAP-27 muxed accounts** (`*_muxed` / `*_muxed_id`, as `QString` to preserve
  the full uint64 id): `OperationResponse` (source), `TransactionResponse`
  (account + fee), Payment, PathPayment, AccountMerge, CreateAccount and
  Clawback operations, and `TradeEffectResponse` (seller).
- `AccountResponse`: `Flags.auth_clawback_enabled`, `Balance.is_clawback_enabled`,
  `sequence_ledger`, `sequence_time`.
- `TransactionResponse`: `memo_bytes`.
- `AssetResponse`: `num_liquidity_pools`, `liquidity_pools_amount`,
  `num_contracts`, `contracts_amount`, `num_archived_contracts`,
  `archived_contracts_amount`, `contract_id`.
- Soroban `getEvents`: `GetEventsResponse` `oldestLedger` /
  `latestLedgerCloseTime` / `oldestLedgerCloseTime`; `EventInfo` `operationIndex`
  / `transactionIndex`.
- `SubmitTransactionResponse`: `diagnostic_events_xdr` (failed Soroban submits).
- `OffersRequestBuilder::offer(id)` — `GET /offers/{id}`.

### Security / robustness

- **Secret material is now wiped on `KeyPair` teardown.** The custom `memset_s`
  only touched the first byte through a `volatile` cast, leaving the optimizer
  free to drop the wipe of the expanded private key; it now writes the whole
  buffer through a `volatile` pointer. The `m_secretSeed` `QByteArray` (never
  cleared before) is now best-effort wiped in the destructor and on assignment,
  COW-guarded so a shared buffer is not silently deep-copied. (No changes to the
  ed25519 curve-derivation library.)
- **Safer resubmit after a timeout.** `SubmitTransactionResponse::isTimeout()`
  (HTTP 504) and `getStatusCode()`, plus `Account::decrementSequenceNumber()`,
  to support idempotent resend of the *same* signed envelope without risking a
  double-spend or a `tx_bad_seq`.
- **`Response::getStatus()` was never populated** — `m_status` defaulted to 0
  and was never set from the reply's HTTP status code, so `getStatus()` /
  `getStatusCode()` / `isTimeout()` always reported 0/false (a Horizon 504 was
  invisible). `preprocessResponse()` now records the real HTTP code, making the
  timeout helpers and the existing `RequestTimeoutException` consistent.

### Tests

- Operation-response deserialization brought to full parity with java-stellar-sdk
  `OperationResponseTest` (27/27 operation types), incl. `claim_claimable_balance`
  and the `change_trust` liquidity-pool variant.
- Added boundary tests for `Price(QString)` and `Operation::toXdrAmount`
  (rejecting non-numeric, locale-grouped and overflowing input) and
  `Account::decrementSequenceNumber`.
- SSE streaming tests against the in-process `FakeServer`: data delivery,
  multi-event parsing, and reconnect with `Last-Event-Id`.
- `testSubmitTimeout504IsDetected`: a 504 submit is delivered on the error path
  and flagged `isTimeout()` / `getStatusCode()==504`, while a 2xx submit reports
  `200` and is not a timeout (regression guard for the `m_status` fix).
- Suite at 662 tests / 91 classes (also revived a test class that a malformed
  `#include` had silently disabled).

## 1.0.0

First stable release. Catches up with `java-stellar-sdk` 3.x on the Soroban /
SEP front and stabilises the public API for downstream consumers.

### Highlights

- **Soroban / Stellar Smart Contracts**
  - `SorobanServer` (CAP-46 JSON-RPC client), `SorobanDataBuilder`,
    `Scv` helpers (primitives, big-integer, vec collections, address coding).
  - `InvokeHostFunctionOperation`, `ExtendFootprintTTLOperation`,
    `RestoreFootprintOperation` and their response counterparts.
  - `AssembledTransaction` lifecycle (simulate → sign auth entries → send →
    poll → result) and `ContractClient` facade. Stellar Asset Contract
    balance queries via `SorobanServer::getSACBalance`.
- **SEPs**
  - SEP-10 (web auth) — full challenge / verification surface.
  - SEP-11 (txrep) round-trip.
  - SEP-29 (memo required) check.
  - SEP-45 (SEP-10 for contract accounts).
- **Horizon protocol 15+**
  - Liquidity pools (request builders, responses, effects, ops).
  - Claimable balances, sponsorship, clawback, trustline flags.
  - `MuxedAccount`, `StellarAddress`, TOID utilities.
- **Public API**
  - All public classes now live in the `qstellar::` namespace. New code
    should use the qualified names (`qstellar::KeyPair`,
    `qstellar::Server`, ...). Projects written against the 0.x
    unqualified API can opt in to source-compatible aliases by defining
    `STELLAR_QT_LEGACY_GLOBAL_NS` before including any SDK header (or
    via `DEFINES += STELLAR_QT_LEGACY_GLOBAL_NS`). With that flag, the
    SDK additionally emits a global `using ::qstellar::Foo;` for every
    public class so unqualified names keep resolving.
  - `Q_INVOKABLE` constructors on `*Response` types and `Q_PROPERTY` on
    payload fields — usable from QML.

### Requirements

- Qt 6 (tested against 6.11), Qt 5 still supported via `QIODeviceEnums`.
- C++17 (bumped from C++11). Adjust your `.pro` / CMake if you pinned an
  older standard.

### Migration notes

- The legacy `MemoId(qint64)` constructor was removed — use the existing
  `MemoId(QString)` factory (`Memo::id(QString::number(n))`).
- `Transaction::Builder` and `FeeBumpTransaction::Builder` remain as
  deprecated aliases for the top-level `TransactionBuilder` /
  `FeeBumpTransactionBuilder` classes; the alias will be retired in 2.0.
- `Network::current()` / `Network::use*` continue to work; the explicit
  per-`Transaction` network pointer is the recommended path going forward.
- Test runners no longer bind to a fixed TCP port — `FakeServer` defaults
  to an OS-allocated ephemeral port.

### Fixes

- Numerous ownership / leak fixes across `Transaction`, `FeeBumpTransaction`,
  `Operation` subclasses and response polling.
- Tighter input validation in `StrKey`, `MuxedAccount`, claimable balance
  IDs, and XDR boundary code paths.
- More consistent error reporting from the Horizon / Soroban RPC layers.
