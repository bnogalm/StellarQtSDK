# Changelog

## 1.0.2

### Fixed

- **`CreateClaimableBalance` transactions could not be serialized at all.**
  `stellar::Operation`'s body union documents that its non-trivial members must
  be placement-constructed, and ships a `fill*Op()` helper for each —
  `CreateClaimableBalanceOp`, which holds an `Array<Claimant>` (a `QVector`),
  was the one that had none. `fillOperationBody` assigned straight into the
  unconstructed union member, the copy constructor and `operator=` did the
  same, `clear()` never destroyed it, and the `QDataStream` reader read into it
  without constructing it. Any `toEnvelopeXdr()` on a transaction carrying this
  operation faulted inside the container's refcount, so the operation could
  never be submitted. Added `Operation::fillCreateClaimableBalanceOp()` and the
  missing lifetime handling at all four sites.
- **`AllowTrustOperation` left `authorize` indeterminate when both flags were
  false.** `stellar::AllowTrustOp` has no default member initializer and the
  constructor's `if` / `else if` chain had no final `else`, so a de-authorising
  AllowTrust shipped whatever was on the heap — and any garbage with bit 0 set
  reads back as `AUTHORIZED`, turning a de-authorize into an authorize.
- **`Txrep::fromTxrep` leaked the `Asset` it allocated for `PAYMENT`** — the
  only one of thirteen branches missing the `delete`.
- **`StellarToml::parse()` silently swallowed the rest of the document on an
  unterminated multi-line string** instead of throwing, because
  `QString::mid` clamps a negative length to "everything from position". Every
  key after the malformed value was lost with no error raised.
- **`RevokeTrustlineSponsorshipOperation` crashed when destroyed or
  re-serialized.** Its `(accountId, asset)` constructor left the `m_asset`
  lazy-cache pointer uninitialized, so `getAsset()` returned garbage and the
  destructor did `delete` on a wild pointer (heap corruption / crash). Now
  initialized to `nullptr`. Surfaced by the new txrep round-trip test.
- **`trustline_sponsorship_updated` dropped its `new_sponsor` field.** The
  property was declared `newSponsor` (camelCase), so Horizon's snake_case
  `new_sponsor` key never bound during deserialization and
  `getNewSponsor()` always returned empty. Renamed the property to
  `new_sponsor` (getter/setter names unchanged). Found via the new
  sponsorship-effect coverage.

### Added

- **Protocol 28 (Adapter) — CAP-85 externally managed contract executables.**
  `ContractExecutable` gained the `CONTRACT_EXECUTABLE_EXTERNAL_REF` arm and its
  payload `ContractExecutableExternalRef` (`executableOwner` + `tag`), so a
  contract instance that points at a shared, updatable code reference decodes
  correctly. This is not just a missing field: contract instances arrive as XDR
  produced by Core, and the old reader consumed only the discriminant for every
  non-WASM executable — so an external-ref payload stayed in the stream and
  silently corrupted everything decoded after it (the instance's storage map,
  the rest of the ledger entry), with no error raised. Unknown executable arms
  now throw instead of desyncing. Testnet votes on Protocol 28 on 2026-08-27,
  Mainnet on 2026-09-16.
- **BIP-39 mnemonic generation & validation.** `Util::generateMnemonic()`,
  `Util::entropyToMnemonic()` and `Util::validateMnemonic()` complete the
  mnemonic surface (the SDK already had word→seed derivation). Verified against
  the canonical BIP-39 test vectors.
- **SEP-1 — `stellar.toml` parsing.** `StellarToml::parse()` reads the TOML
  subset real `stellar.toml` files use (scalars, arrays, `[Table]`,
  `[[ArrayOfTables]]`, multi-line basic strings, comments) into a nested
  `QVariantMap` whose shape matches py-stellar-sdk / `tomllib`. (Network
  resolution from a domain is a follow-up; this is the parse half.)
- **SEP-7 — URI scheme (`web+stellar:`)** for delegated signing. `Sep7Pay`
  (`pay` requests) and `Sep7Tx` (`tx` requests, with txrep `replace` fields)
  build and parse URIs and sign / verify them with the origin domain's
  `URI_REQUEST_SIGNING_KEY`. The URI encoding and signing payload were verified
  byte-for-byte against py-stellar-sdk, so the URIs interoperate with other
  SDKs and wallets.
- **SEP-11 Txrep now supports all memo types and `PRECOND_V2`.**
  `toTxrep`/`fromTxrep` previously handled only `MEMO_NONE` and threw on any
  V2 precondition; they now round-trip every memo type (`TEXT`/`ID`/`HASH`/
  `RETURN`) and the full V2 condition set (timeBounds, ledgerBounds,
  minSeqNum, minSeqAge, minSeqLedgerGap, extraSigners). Operation coverage
  grew from ManageData/Payment to also include `CREATE_ACCOUNT`,
  `ACCOUNT_MERGE`, `BUMP_SEQUENCE`, `SET_OPTIONS`, `CHANGE_TRUST` (non-pool),
  `ALLOW_TRUST`, `MANAGE_SELL_OFFER`, `MANAGE_BUY_OFFER`,
  `CREATE_PASSIVE_SELL_OFFER`, `PATH_PAYMENT_STRICT_RECEIVE`,
  `PATH_PAYMENT_STRICT_SEND`, `BEGIN_SPONSORING_FUTURE_RESERVES`,
  `END_SPONSORING_FUTURE_RESERVES`, `CLAWBACK`, `CLAWBACK_CLAIMABLE_BALANCE`,
  `SET_TRUST_LINE_FLAGS`, `CLAIM_CLAIMABLE_BALANCE`, `LIQUIDITY_POOL_DEPOSIT`,
  `LIQUIDITY_POOL_WITHDRAW`, `CREATE_CLAIMABLE_BALANCE` (with full nested
  claim-predicate trees) and `REVOKE_SPONSORSHIP` (all 7 ledger-key / signer
  variants). **Interoperable with py-stellar-sdk / stc:** every field path and
  value encoding was diffed against py-stellar-sdk's canonical `to_txrep`
  output and aligned — native asset as `native`; `changeTrustOp.line.type`
  discriminator; `allowTrustOp.authorize` as a bool; `PRECOND_V2` fields under
  `tx.cond.*` (not `tx.cond.v2.*`) with decomposed `extraSigners[i].type` +
  strkey; RevokeSponsorship `ledgerKey.account` collapsed and `ledgerKey.data.
  dataName` unquoted. The parser also tolerates the human-readable ` (...)`
  comments those tools append to typed values (e.g. `amount: 105000000 (10.5)`).
  The Soroban operations remain unsupported (txrep is a pre-Soroban format;
  no mainstream SDK encodes them).
- **Typed Soroban contract exceptions.** `AssembledTransaction` previously
  threw plain `std::runtime_error` for every failure, so callers could not
  distinguish a simulation error from a misuse or a send rejection. It now
  throws `AssembledTransactionException` (base) and the specific
  `SimulationFailedException`, `NotYetSimulatedException` and
  `SendFailedException` (in `qstellar::exception`). All derive from
  `SdkException` → `std::runtime_error`, so existing
  `catch (std::runtime_error&)` / `catch (std::exception&)` code is unaffected.

### Tests

- `getEvents` request construction is now tested (the response side already
  was): a `startLedger`/`endLedger`/`filters`/`cursor`/`limit` call maps to
  the right JSON-RPC params, and defaults are OMITTED rather than sent as
  `0`/empty. `SorobanServer::sendRequest` was made `virtual` so a test can
  capture the assembled params (same pattern as `sendTransactionXdr`).
- SSE keep-alive (`data: "hello"`) frames are covered: the stream parser
  skips them without emitting an event, surfacing only real payloads.
- Signer-type deserialization coverage: the `sha256_hash`, `preauth_tx` and
  `ed25519_signed_payload` (CAP-40) signer key types are now tested
  (previously only `ed25519_public_key` was).
- Sponsorship-effect deserialization coverage: all 15 CAP-33 sponsorship
  effect classes (account / trustline / data / claimable-balance / signer ×
  created/updated/removed, `type_i` 60-74) were entirely untested; each now
  has a deserialization test. (One of them surfaced the `new_sponsor` bug
  above.)
- Single-resource endpoint coverage (`offers().offer(id)`, `ledgers().ledger(seq)`,
  `transactions().transaction(hash)`, `liquidityPools().liquidityPool(id)`):
  these now drive a full GET round-trip against the in-process `FakeServer`,
  not just `buildUri()`. Catches regressions in the requested URL and in the
  parse of the single (non-paged) resource body.

### Security

- **SEP-45 now cryptographically verifies the server's challenge signature.**
  `Sep45Challenge::verifyChallenge` previously deferred all crypto to
  `simulateTransaction`, which validates only the *client's* Soroban auth
  chain — it never confirmed the challenge envelope was actually signed by the
  server's signing key. A forged challenge (the server's address as source,
  signed with the attacker's own key) would have passed. It now verifies a
  valid server signature over the transaction hash locally, mirroring
  `Sep10Challenge`, and throws `InvalidSep45ChallengeException` otherwise.
  The signature is checked against the challenge's *as-issued* form: the server
  signs before the client fills in the auth entry's
  `signatureExpirationLedger` and `signature`, and those fields live inside the
  operation body, so hashing the envelope the client returns would reject every
  genuine response. Only those two client-owned fields are reset before
  hashing — source account, sequence, timebounds, the host function and its
  args, and each auth entry's address, nonce and `rootInvocation` all stay
  covered.
- **SEP-7: a signed `pay` URI accepted an injected `memo` that `verify()` did
  not cover.** `verify()` checks the signature against the canonical
  re-serialisation, and `toUri()` emits `memo` only alongside `memo_type` (and
  `asset_issuer` only alongside `asset_code`), while `fromUri()` populated both
  unconditionally. Appending `&memo=…` to a legitimately signed URI therefore
  reproduced the signed base byte-for-byte: `verify()` returned `true` while the
  parsed request carried attacker-controlled data that a wallet would attach to
  the payment. `fromUri()` now rejects a `memo` without `memo_type` and an
  `asset_issuer` without `asset_code`.

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

### Build / CI

- **GitHub Actions matrix is green on Qt 5.15.2, 6.4.3 and 6.8.1** (build +
  full test suite). Qt 5.15 needed three compatibility fixes that newer Qt
  headers mask: force-include `<limits>` on Qt 5 / GCC (works around
  QTBUG-90395, where QtCore specializes `std::numeric_limits` without including
  it), and add the standard headers (`<cstring>`, `<algorithm>`, `<cmath>`)
  that Qt 5.15's headers no longer pull in transitively. The CI build uses
  `make -k` so a single run reports every compile error at once.

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
