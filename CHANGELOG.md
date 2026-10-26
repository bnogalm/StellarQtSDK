# Changelog

## 1.0.5

The Horizon utilities js-stellar-sdk 17.1.0 and py-stellar-sdk 14.1.0 have
and this SDK lacked: the base fee, time bounds by the server's clock, an
account's data entry and friendbot. Each reads its answer exactly and says
where it differs from js and py, and why. New source and header files
(`src/httpdate.h`, `src/responses/accountdataresponse.h`,
`src/responses/friendbotresponse.h`): re-run qmake / re-configure CMake.

### Added

- **`Server::fetchBaseFee()`**, as js's `fetchBaseFee`: GET `/fee_stats` and
  its `last_ledger_base_fee`, in a `BaseFeeResponse` (a `FeeStatsResponse`,
  read exactly, with `baseFee()`) that the caller owns. js answers 100 when
  the fee does not parse or is 0 (and returns a negative one as it is), py
  (`fetch_base_fee`, from the latest ledger's `base_fee_in_stroops`) when
  there is no ledger. Here no fee is made up: an
  answer without a positive base fee (missing, null, 0, negative, not a
  whole number) emits `error()` with an `UnknownResponseException`, and a
  caller that wants js's default takes 100 itself. A failed request keeps its
  own kind, as everywhere.
- **`Server::fetchTimebounds(seconds)`**, as js's `fetchTimebounds`:
  `{minTime 0, maxTime: the server's time + seconds}` in a caller-owned
  `TimeboundsResponse` (`minTime()`, `maxTime()`, `toTimeBounds()` for
  `TransactionBuilder::addTimeBounds`). The server's time is the HTTP `Date`
  of the last answer from the Server's own host and port, if it came within
  5 minutes (js's rule), moved on by the time since; otherwise the Horizon
  root is asked, and an answer without a readable `Date` (or with one at or
  before 1970) gives the local time, as js's retry does. The `Date` is taken
  when the answer's headers arrive, not when its body ends, and streams do
  not count (a stream's headers come when it opens, maybe hours before it
  ends; js's EventSource streams are not recorded either). `ready()` never
  gives a maxTime of 0 (no upper bound) or a negative one: with no usable
  `Date` and a local clock at or before 1970 it is an `error()`. Where it
  differs from js, and why: each Server
  keeps its own time (js keeps one table for the whole process); the age is
  measured on a monotonic clock (js's wall clock moves when the clock is
  set); every HTTP answer's `Date` counts, an error's too (js records 2xx
  only), and so the root's `Date` is used whatever its status (js fails on
  an HTTP error there); an answer without a `Date` leaves the last time
  (js forgets it); a redirected answer's `Date` counts only for the host that
  answered (js records it under the host it asked, so a friendbot's clock
  would be taken for Horizon's). A root that is not answered at all is an `error()`, as js
  propagates it. Negative seconds throw `std::invalid_argument` (js does not
  check, and gives a maxTime in the past). py has no `fetch_timebounds`.
  `HttpDate::parse` reads the three HTTP-date forms (IMF-fixdate, RFC 850,
  asctime) exactly, the same on Qt 5.15 and Qt 6, where
  `QDateTime::fromString(..., Qt::RFC2822Date)` is not; an RFC 850 two-digit
  year is read by RFC 7231's rule (the year with those digits at most 50
  years ahead and at most 49 behind: in 2051, "00" is 2100).
- **`AccountsRequestBuilder::accountData(account, key)`** (and
  `accountData(QUrl)`): GET `/accounts/{id}/data/{key}`, as py's
  `Server.data(account_id, data_name)` and js's `data({key})` link on an
  account record. `AccountDataResponse`
  decodes the value exactly (`value()` the bytes, `valueBase64()` the text,
  `sponsor()`): text that is not canonical base64 of at most 64 bytes is an
  `error()`; py hands the text back undecoded. The key is percent-encoded as
  one path segment: py writes it into the URL raw, so a name with `/`, `?`
  or `#` asks for another path and one with a `%XX` sequence for another
  name (`a%20b` is read as `a b`; requests encodes a space or a lone `%`). A
  name with `/` is answered 404 by Horizon however it is encoded (js too):
  read it from the account's data map. A
  key no entry can have is refused with `std::runtime_error` by
  stellar-core's rule for data names (1 to 64 bytes, printable ASCII), and
  so are `.` and `..`: core takes them, but they are dot-segments that Qt
  normalizes out of the path, so HTTP cannot ask for them (py's `..` asks
  for the account). After `stream()` the entry is streamed,
  as py's `DataCallBuilder.stream()`.
- **`Server::friendbot(account)`** (and `friendbot(account, url)`), as js's
  `Horizon.Server.friendbot(addr)`: GET `<horizon>/friendbot?addr=G...`,
  following redirects (SDF's Horizons answer 307 to friendbot.stellar.org;
  Qt 5.15 follows none unless told), never from https to http: js (node's
  follow-redirects) follows that one, dropping only confidential headers;
  it is refused here so that a funding request is never downgraded. The explicit
  URL keeps its query and gets `addr` set, as QML's `Horizon.fundAccount()`
  does. `FriendbotResponse` (the Server's, as every `get()`) is ready only
  for a 200 saying `"successful": true` with a 64-hex `hash`, read as
  `fundAccount()` reads it (`getHash()`, `getLedger()`, `getEnvelopeXdr()`,
  `getResultXdr()`); js returns any 2xx body. HTTP errors are the usual
  ones, with friendbot's problem document. py's Horizon Server has no
  friendbot.

### Changed

- The client version sent to Horizon (`X-Client-Version`) is 1.0.5.

## 1.0.4

What is signed is what was asked for. 1.0.3 fixed the Soroban wire format;
this release closes the places where the bytes signed, or the value shown,
could differ from what the caller gave or the server sent — prices, fees,
integers, time bounds, sequence numbers, TxRep documents, SEP-10 and SEP-45
challenges, and the numbers read back from Horizon and Soroban RPC. Every fix
ships with a test that fails when it is reverted. Adds an optional QML layer,
SEP-45 v0.1.1, contract interfaces read from the `.wasm`, and a CMake package.
New source and header files: re-run qmake / re-configure CMake.

### Fixed — values signed other than the ones asked for

- **Prices.** `Price(QString)` read `"1.234567e-06"` as 1.234567 (it kept 11
  characters and went through a double); it now reads the whole decimal
  exactly, and a value that is not an int32 fraction gets exactly the fraction
  java and js sign (identical to js-stellar-sdk on ~250,000 decimals).
  `Price(n, d)` compiles only with `qint32` terms; offers built from a 0/0, 1/0
  or negative price are refused. Horizon trade prices (int64) are no longer
  wrapped into int32 (~40% of real pool trades read as another price).
- **Fees.** `setBaseFee` took a `quint32` (-5 became 4294967291 stroops); fee
  totals above uint32 were signed truncated; a 101st operation was silently
  dropped; fee bumps over Soroban counted the resource fee per operation; a
  negative resource fee in a transaction's own soroban data was signed. All
  refused or exact now. A simulated transaction's fee is the bid plus the
  simulated `resourceFee`, as in js-stellar-sdk (re-simulating no longer moves
  it).
- **Transactions.** `signatureBase()` failures made `sign()` sign
  `sha256("")`; a transaction built with muxing disabled signed the hash of
  its M... form while sending the G... envelope; a set timeout was lost after
  `addPreconditions`, a second `build()` or a failed one; `fromEnvelopeXdr`
  signed truncated envelopes with the missing fields as zeros (it must now
  re-encode byte for byte; 22,541 real envelopes checked); weights and
  thresholds above 255 were signed; copying builders and operations freed
  memory twice. `SetOptionsOperation`'s full constructor signed the home
  domain lower-cased and punycoded, and EMPTY — which clears the account's —
  for what Qt takes for no host name (`"stellar.org/"`) or a null QString; it
  is signed as given now, like `setHomeDomain()` and js/py.
- **Sequence numbers.** `ContractClient::read` and a failed `invoke` used up
  the account's sequence number. `restoreFootprint` hands its number back only
  when nothing was submitted; when the send may have reached the server it
  keeps it and throws `SendOutcomeUnknownException` with the hash to look up.
- **Contract integers.** `Scv::toUint32` and friends narrowed silently
  (`4294967296` was signed as u32 0); `toInt128Parts(0, -100)` was signed as
  2^64-100. Range-checked now. Soroban maps are in the host's order, so the
  auth entries a G account signs are accepted on chain (they were refused).
- **TxRep (SEP-11).** A comment must state its line's value (an amount in units
  or with `e7` in stroops, a time as a date in UTC or at an offset, the key a
  signature verifies for) or the document is refused: `(99.5)` next to 10.5
  units, or `tx.seqNum: 77 (78)`, was signed unchecked. The fee is kept exactly
  (1000 over three operations came back as 999); every integer is
  range-checked; missing `_present`/type lines, duplicate keys and invisible
  characters are refused — anywhere, comment lines included: a lone CR (not
  part of a CRLF), which editors show as a line break, and every format
  character that draws nothing (soft hyphen, Arabic letter mark, fillers,
  language tags beyond the BMP...). The writer escapes them. Fee bumps, V0 envelopes, muxed sources, pool-share
  revokes and INFLATION have forms; py-stellar-sdk and stc documents are read
  byte for byte. A date at a zone abbreviation is read only when the
  abbreviation names one offset in every IANA tz release and in common use
  (CET/CEST yes; IST, CST, PST, EST... no).
- **SEP-10.** Nonces were decoded URL-safe only (87% of real challenges were
  refused); the server side accepted a challenge the client never signed, and
  `verifyChallengeTransactionThreshold` refused signers that met the threshold
  when a weight was above 255 (pre-protocol-10 signers; counted as 255 now, as
  stellar-core does). Muxed clients and binding the `client_domain` operation
  to its signing key are supported.
- **SEP-45.** The published standard, v0.1.1, is implemented, byte-compatible
  with py-stellar-sdk; its QML verdict requires the simulation of the signed
  entries (the server's own unsigned simulation used to pass). The wallet's
  footprint check accepts the wasm a Protocol 23 simulation restores with an
  archived web-auth contract (testanchor's, today). The earlier format checked
  who signed, not what was authorized; it is fixed and deprecated.
- **BIP-39.** The passphrase is NFKD-normalised as the standard requires.
  Wallets made with 1.0.0-1.0.3 and an accented passphrase derived other
  accounts: `Util::mnemonicToBIP39SeedLegacyPassphrase` (and the QML
  equivalents) recovers them. `Seps.deriveAccountRange` refuses a phrase
  whose checksum fails (a typo that derived someone else's wallet), and every
  QML account index is read exactly (`2^31` gave account 0's keys).
- **SEP-2, SEP-29.** Federation answers are checked before use, https only,
  and its cache works (it cached nothing and leaked); the name is sent
  percent-encoded (`+`, `#` and `%` went raw, so another name was looked up);
  memo-required checks read the envelope actually submitted and report
  failures as what they were.

### Fixed — reading Horizon and Soroban RPC

- **Horizon responses are read exactly.** A value that does not fit its field
  (text or a fraction in a number, a number past its range or above 2^53, the
  wrong JSON type), or a member Horizon always sends that is missing, fails the
  response naming the field; it used to read as 0 or empty — an account's
  sequence and balances included. About 1,400 real documents of every type
  from Horizon 28 and 29 read, failed liquidity-pool operations and signers
  heavier than 255 included; a pool-share trustline's sponsorship effects
  (asset_type `liquidity_pool`, read with the new `getLiquidityPoolId()`)
  load. `operations().operation(id)` returned every
  operation as a CreateAccount; pages with clawback operations were refused
  whole; SSE streams stalled on an unreadable event.
- **Asset and predicate factories.** `Asset::create(type, code, issuer)` built
  whichever asset the code's length made; claim predicates read from JSON
  turned an unreadable time into another one; and/or equality ignored operand
  order, so a reordered predicate was signed in its old order. All exact now.
- **Soroban RPC answers are read exactly** (numbers, statuses, hashes, XDR),
  or refused naming the field. A `getTransaction` without a known status read
  as NOT_FOUND and `pollTransaction` polled to its timeout; an answer about
  another transaction was taken for this one. `getSACBalance` threw on every
  real balance (it read a layout of its own); new `getAssetBalance` reads an
  account's trustline or XLM, or a contract's SAC balance.
- **Timeouts.** Every `SorobanServer` request is bounded and cancellable, and
  says whether it may have reached the server (`SorobanTransportException`).
  The SDK's own Horizon reply timeout is `ClientTimeoutException` (still a
  `ConnectionErrorException`). The QML `Soroban.send` asks the RPC which
  network it serves before posting, so a send whose outcome is unknown always
  reports the hash it can land under, and a passphrase for another network is
  refused before anything is posted. `Soroban.cancelAll()` and an endpoint
  change no longer drop a send that had already started: it is reported once,
  as cancelled (with the hash to look up if it may have been posted) or with
  its own result; only sends that never started are dropped.
- **Qt 5 resends.** Qt 5 sends a POST again by itself when the connection
  closes after the request was written. An answer to a `sendTransaction` it
  sent again is taken only when PENDING or DUPLICATE; anything else throws
  `SorobanTransportException` with the new `resent()` (the first send may have
  landed). `restoreFootprint` then keeps the sequence number, `signAndSend`
  throws `SendOutcomeUnknownException` with the hash for any failure of a
  resent send (ERROR and TRY_AGAIN_LATER used to be `SendFailedException`,
  timeouts and cancels `SorobanTransportException`), and `Soroban.send` reports
  it with `resent`.
- **`Horizon.submit` (QML).** It checks the passphrase against the network the
  endpoint serves (Horizon's root) before anything is posted. `cancelAll()`, an
  endpoint change or any failure of a submit that may have reached Horizon
  (the SDK's timeout, a 504, a connection lost, a cancel) is reported with
  `mayHaveReachedServer` and the hash to look up; the outcome used to be
  dropped, and a submit cancelled during its SEP-29 lookup was posted anyway.
  Only a submit that never sent a byte is reported as not sent, and a refusal
  that answers a POST Qt 5 resent by itself carries `resent: true`: it is not
  final, since the first send may have landed.

### Fixed — operation by operation, as js-stellar-sdk 17.1.0 and py-stellar-sdk 14.1.0

Every operation was built, signed, decoded and read from Horizon side by side
with js and py. Where they sign the same bytes, so does this SDK now; where
stellar-core refuses what they sign, this SDK refuses it too.

- **Signed differently.** An `Account` made from an M... address signed from
  its plain G account (the muxed id dropped); `createContract` without
  constructor arguments was signed as v1 in QML (js/py sign v2);
  `setMinSeqAge(0)` / `setMinSeqLedgerGap(0)` gave PRECOND_TIME (js/py V2); a
  custom network passphrase outside ASCII was hashed from Latin-1, not UTF-8;
  QML trimmed names, home domains, data names, function names and symbols, and
  signed a manageData value "" as a delete.
- **Signed what core refuses.** Asset codes outside `[A-Za-z0-9]`, zero
  amounts (payments, path payments, passive offers, claimable balances,
  clawbacks, pool withdrawals), pool fees other than 30, ledger bounds with
  min > max, a negative `bumpTo` (QML), B.../L... addresses as contract
  arguments, Soroban data applied to a classic or multi-operation
  transaction, memo hashes shorter than 32 bytes (padded with zeros), a QML
  `allowTrust` without `authorize` (it de-authorized). All refused now.
- **Refused what js/py accept.** Amounts written `.5`, `5.`, `+1` or `1e2`;
  a base fee below 100 (the fee-0 inner transaction of a fee bump); the pool
  address `L...` in QML `revokeLiquidityPoolSponsorship`; a pool-share
  trustline in `revokeTrustlineSponsorship` (QML `liquidityPoolId`).
- **Decoded or read differently.** A decoded strict-receive path payment is a
  `PathPaymentStrictReceiveOperation`; offer prices read as js writes them;
  `SetOptionsOperation::getHomeDomain` returns the bytes signed; lower-case
  strkeys are refused; a short alphanum-12 code throws with a message; a
  default `Claimant` is unconditional instead of crashing. From Horizon:
  pool-share trustline effects and revoke-sponsorship operations name their
  pool; `*_muxed` / `*_muxed_id` fields, data effects' name and value, and
  set-options fields left out (no longer read as 0) are exact; flag effects
  tell a flag not sent from false; a native balance has no `authorized` in
  QML (it read false); an SSE stream that reconnects drops the half event of
  the dropped connection.
- **TxRep.** A leading byte order mark is text, as py writes it.
- **Found by the live test against js-stellar-sdk** (`interop-live`, two apps
  running one 34-step scenario on testnet): a failed submission (Horizon's
  400) left `SubmitTransactionResponse::getExtras()` and `getResultXdr()`
  empty, so the result codes were only reachable through the exception's
  Problem; they hold Horizon's extras now, `inner_transaction` included.
  `CreateClaimableBalanceResult` was a union whose code shared storage with
  the balance id's type (setting an id turned UNDERFUNDED into SUCCESS).
  `KeyPair::verify` accepted a signature longer than 64 bytes, its tail
  ignored, so a padded blob verified (SEP-53 messages, SEP-7, SEP-10 and
  every other caller); it requires exactly 64 bytes now.
  An account balance's `is_authorized`, `is_authorized_to_maintain_liabilities`
  and `is_clawback_enabled` left out by Horizon (XLM, pool shares) read as a
  definite false: `Balance::hasAuthorized()` / `hasAuthorizedToMaintainLiabilities()`
  / `hasClawbackEnabled()` tell them apart. A set-options operation's integer
  `set_flags` / `clear_flags` were not read (`getSetFlagValues()`,
  `getClearFlagValues()`).

- **Found reviewing the lab's notes about the SDK** (each checked against js 17.1.0):
  - StrKey decoding accepted a strkey with a good checksum but the wrong body (a G of 5 bytes, an M of 32, a C of 20), a B whose type byte is not 0, and '=' inside a valid G (decoded as the same key). The body length per kind and the canonical spelling are enforced now, as js does.
  - Util::mnemonicToBIP39Seed did not check the BIP-39 checksum: two words swapped derived another, valid-looking wallet (only the QML layer checked). It refuses a bad checksum now, as py's to_bip39_seed; KeyPair::fromMnemonic(words, passphrase, account) goes from a phrase to an account, as py's from_mnemonic_phrase. Words may be separated by any whitespace (tabs, line breaks), as the QML layer already read them; py-stellar-sdk 14.1.0 refuses such a phrase (it splits on single spaces), and the seed is the single-spaced phrase's either way. The legacy recovery derivation still opens a phrase with a bad checksum.
  - SEP-7 verify() checked its own re-serialisation, not the URI received (a signer ordering its parameters otherwise failed); fromUri() did not apply toUri()'s rules, so verify() could throw; a parameter after signature was read unsigned; replace parts could hold their own delimiters. A received URI is verified as received now, signature must be last, duplicate parameters and invalid requests are refused, the 300-character message limit counts code points (as py).
  - SEP-2: a lookup that failed at the domain's stellar.toml emitted a bare error() (no toml, no FEDERATION_SERVER, a non-https one: all alike); the response's lastException() is a FederationException with the reason now. Federation::resolve(G... or M...) never emitted ready(); it does, queued, like any other lookup.
  - BumpSequenceOperation (C++) built and signed a negative bumpTo, which core refuses (BUMP_SEQUENCE_BAD_SEQ; the QML layer already refused it). The constructor refuses it now; decoding an existing operation, TxRep included, still reads it.
  - TOID took a ledger above 2147483647 and encoded a negative id that sorts before every real one; the constructor refuses it now, as stellar/go does. Decoding (fromInt64) still takes any 64 bits.
  - QML Horizon.fundAccount decided by "testnet" appearing in the endpoint URL: futurenet and a local quickstart were refused, and any URL with "testnet" in it was sent to SDF's friendbot. The friendbot is now the one of the endpoint's host (testnet, futurenet), or the new public friendbotUrl property; any other Horizon is asked at its own /friendbot (see the release review below).
  - StellarToml::parse misread TOML silently: \u escapes corrupted ("Caf\u00e9" read "Cafu00e9"), 'literal' strings kept their quotes, duplicate keys and tables overwrote, "[DOC" was accepted, inline tables, dotted keys, [a.b] and datetimes came back as strings, an int beyond int64 was rounded. It is a TOML 1.0 reader now, refusing invalid TOML with the line; it agrees with Python's tomllib on 24 real stellar.toml files and 80,000 generated cases (datetimes are returned as their text; an integer outside int64 is refused).
  - Base64 from Horizon, SEP-45 simulations and the QML XDR reader is decoded strictly (Qt's lenient decoder skipped unknown characters, so a URL-safe '-' / '_' or a damaged value became other bytes): a memo hash / return that is not base64 is refused, `Data::getDecoded` returns empty, the XDR reader says what is wrong.
  - SEP-10 `verifyChallengeTransactionSigners` refuses a secret seed (S...) among the signers, without repeating it, instead of skipping it as js does (py refuses it too, with a message that repeats the seed): it is the caller's mistake, and skipping it left the account's own signer out of the check.
  - New `StellarTomlResolver` fetches a domain's stellar.toml as js's `StellarToml.Resolver` does: https only, no redirect followed, a timeout, refused over 100 KB, a domain that is not a host name alone refused. `FederationServer` reads its toml through it (its errors now say why: the request failed, HTTP status, too large, not TOML), and a federation answer over 100 KB is refused (js `FEDERATION_RESPONSE_MAX_SIZE`). The unused cpptoml branch is gone, and `external/cpptoml` with it.
  - SEP-2 reverse and transaction lookups: `FederationServer::resolveAccountId` (type=id) and `resolveTransactionId` (type=txid), as js has them (py has `resolve_account_id`, and no transaction lookup); an argument that is not an account id / a 64-hex transaction id is refused before any request.
  - SEP-7 `Sep7Tx`: the xdr has to decode as a whole transaction envelope (it was carried as any text), `fromUri(uri, expectedNetworkPassphrase)` reads the network as py's `from_uri` does (a request for another network is refused, one that names none is read for the expected network, and with none expected the URI has to name one), and `transaction(network)` returns the decoded envelope, refusing a network other than the request's. The test fixture is a real envelope whose URI matches py's byte for byte.
  - SEP-29 on its own: `Server::checkMemoRequired(transaction)` is public, as js's `checkMemoRequired`, and submits nothing; its `MemoRequiredCheck` emits ready(), or error() with the AccountRequiresMemoException / failed lookup `submitTransaction` would have delivered. The destination list it used to name is now the private `memoCheckedDestinations`.
  - TxRep: the emitter writes the glosses py-stellar-sdk writes: each amount followed by its units (`125000000 (12.5)`, `1 (1E-7)` as py has it from an envelope) and each time bound by its UTC date (`1700000000 (2023-11-14 22:13:20.000000+00:00 (UTC))`). A document is now py's `to_txrep` of the same envelope line for line (py leaves out the final newline; a NOT predicate is written with SEP-11's `notPredicate._present` line, which py leaves out), and the reader still checks every gloss against its value.
  - SEP-7 origin check: `Sep7::verifyOrigin(uri, originToml)` requires an origin_domain that is a fully qualified domain name, a signature, and that signature to be the toml's URI_REQUEST_SIGNING_KEY's; `Sep7OriginCheck::verify(uri)` fetches the origin's stellar.toml (`StellarTomlResolver`) and runs it. SEP-10: `Sep10Challenge::webAuthFromToml(toml)` reads WEB_AUTH_ENDPOINT (https) and SIGNING_KEY (an account id), or says which is missing. Neither js nor py has these; they are what SEP-7 and SEP-10 ask a client to do.
  - SEP-23 signed-payload (P...) keys are tested against the spec's literal vectors (32- and 29-byte payloads both ways, as py-stellar-sdk reads them; the spec's two invalid keys, whose declared length does not match the data, refused as js and py refuse them), and against a key whose padding is not zero, refused as js and core refuse it (py 14.1.0 accepts it).
  - SEP-35 TOID: `afterLedger`, `ledgerRangeInclusive` and `incOperationOrder`, as stellar/go's toid package has them (including go's roll-over into the next ledger), refusing where go would overflow; tested against a real testnet Horizon operation id.
  - SEP-1: `StellarToml::currency(toml, code, issuer)` finds an asset's [[CURRENCIES]] entry by issuer and code (or `code_template` with `?` wildcards), empty when the domain does not list it: the lookup a wallet makes to check an asset against its issuer's home_domain.
  - BIP-39: the seven other wordlists py-stellar-sdk has (Japanese, French, Spanish, Italian, Korean, Chinese simplified and traditional), through a language parameter on `entropyToMnemonic`, `generateMnemonic`, `validateMnemonic`, `mnemonicToBIP39Seed` and `KeyPair::fromMnemonic` (English by default). Phrases and accounts match py's byte for byte, U+3000 between Japanese words included.
  - `KeyPair::random()` is documented as what it is, the operating system's CSPRNG (as js's and py's Keypair.random), not "DON'T USE IT"; `random(QByteArray)` as XORing caller entropy into it. The temporary seed buffers are wiped, and a keypair keeps its own copy of the seed bytes.
  - `KeyPair::fromSecretSeed`: the Latin-1 and decoded copies of a seed are wiped; a strkey given to the bytes overload is refused saying to use `fromSecretSeed(QString)` (it said "you seems using an user encoded string as bytearray seed"); no message repeats a seed; java's "insecure" label is replaced by what the risk is.
  - QML `Seps.tomlSigningKeys`: SIGNING_KEY and URI_REQUEST_SIGNING_KEY must be account ids (G..., checksum and all); a present key that is not one (a seed, a damaged key, a number) is { error } naming it, without repeating it.
  - SEP-2: a federation answer's members the SDK does not read (SEP-2 lets servers add their own) are kept, through `FederationResponse::json()`; now documented and tested.
  - `Util::mnemonicProblem(phrase, language)` says why `validateMnemonic` refuses a phrase (not valid Unicode, the word count, the position of a word not in the list, the checksum), empty when it does not; `mnemonicToBIP39Seed` throws the same reason. A word is named by its position, never repeated.
  - `KeyPair::fromBip39Seed(seed, path)` derives the key at a SLIP-10 path ("m/44'/148'/0'", n' or nH), for derivations other than SEP-5's; an index that is not hardened, or not a number below 2^31, is refused.
  - SEP-10: a challenge read with the wrong network failed as "Transaction not signed by server"; when the server's signature is good for another known network (public, testnet, futurenet), the error now names it.
  - `StrKey::decodeAny(text)` decodes any strkey whole (checksum, length, canonical spelling) and returns its kind and payload; `decodeVersionByte` is documented as reading the first byte only, and reads Latin-1 instead of the local 8-bit codec.
  - SEP-29's data entry name and value are public constants in `sep29.h` (`Sep29::DATA_NAME`, `DATA_VALUE`, `DATA_VALUE_BASE64`), used by the server check and the QML layer instead of copies in each .cpp.
  - TOID: the 1-based transaction and operation numbering is documented; `operator<`, `qHash`, `toUInt64`, `toString` and `fromString` (Horizon's decimal ids and paging tokens, refusing a sign, spaces or a number past int64).
  - SEP-53: `KeyPair::signMessage` / `verifyMessage` (as js's and py's Keypair), QString and const char* overloads signing the UTF-8 bytes, and `MessageAuth::PREFIX`; signatures match py-stellar-sdk's.
  - QML `Seps.parseToml` hands an integer JavaScript cannot hold exactly (beyond ±2^53) to QML as its decimal text instead of a rounded number; the stale doc about the subset of TOML read is corrected.
- **Found in the release review** (each checked against js 17.1.0 and py 14.1.0):
  - Ed25519 verification accepted a signature with L added to its S (only S < 2^253 was checked), so a valid signature had a second, malleated form that stellar-core, js and py refuse; `KeyPair::verify` (and through it addSignature, SEP-10, SEP-7 and SEP-53) refuses S >= L now.
  - `KeyPair::fromMnemonic` took a negative account number and derived another account with it (-2^31 gave account 0, -1 account 2^31 - 1); it is refused now. `fromBip39Seed(seed, n)` documents the Java rule it keeps: a number with the top bit set is taken as already hardened.
  - Secret copies the SDK said it wiped were only freed: `StrKey::decodeCheck`'s re-encoding of a seed (the S... strkey again), the account key `fromBip39Seed(seed, n)` derives and SLIP-10's intermediate keys, `mnemonicProblem`'s entropy on an unknown word, and the BIP-39 PBKDF2 password; they are wiped now.
  - `StrKey::decodeCheck(SIGNED_PAYLOAD, ...)` (and `decodeAny`) accepted a P... key whose declared payload length did not match its data or whose padding was not zero (only `decodeSignedPayload` checked); it refuses them now, as js's decodeCheck does (py accepts non-zero padding, which core refuses).
  - `src/bip39wordlists.cpp` carries the MIT notice of python-mnemonic, whose wordlist files it was generated from.
  - SEP-7 `fromUri` read a raw '+' in a value as a '+' (QUrlQuery's rule), where py's parse_qsl and URLSearchParams read a space: "msg=Pay+me" showed "Pay+me", and a testnet request written with '+' was refused for testnet. It is a space now (%2B stays a '+'). Base64 values (xdr, signature, a hash or return memo) keep a raw '+', which py breaks; toUri() still writes %20, as py.
  - SEP-7 pay: `asset_code=XLM` with no issuer was refused ("asset_code without asset_issuer"); it is the native asset, as py's Asset reads it: fromUri() leaves assetCode empty and toUri() writes no asset, as py's to_uri().
  - SEP-7 pay: the asset was checked only as a code/issuer pair, so `asset_issuer=garbage` or a 13-character code was read as a request; asset_code is 1 to 12 ASCII letters or digits and asset_issuer an account id (G...) now, on the way in and out, as py's Asset checks them.
  - SEP-7 pay: a `memo_type` with no `memo` was read as an empty text memo; it is refused now, as py refuses it. An empty `memo=` stays an empty text memo (core takes it; toUri() writes it as py's to_uri() does, which py's own reader cannot read back).
  - SEP-7 `Sep7Tx::fromUri(uri, expected)` refused a URI naming no network_passphrase unless expected was the public network, though documented as py's from_uri; it reads such a URI for the expected network now, as py (networkPassphrase holds it, toUri() writes it, verify() still checks the URI received), and with an empty expected refuses a URI naming none, as py with None. fromUri(uri) still reads an absent one as SEP-7's public network.
  - SEP-7 origin check: `Sep7::verifyOrigin` skipped SEP-7's step 3 and `Sep7OriginCheck` took an IP address or a single label ("localhost"); both refuse an origin_domain that is not a fully qualified domain name now (`Sep7::isFullyQualifiedDomainName`), Sep7OriginCheck before any request.
  - SEP-7: the documentation claimed field-for-field parity with py's stellar_uri.py and py's network check for `Sep7Tx::fromUri(uri, expected)`; sep7.h now lists where it departs from py and why (verify() over the URI as received, where py has no verify; raw '+' kept in base64 values; `memo=` read back; destination and amount checked; an absent network_passphrase read as public by Sep7Tx::fromUri(uri)).
  - QML `Seps.verifyUriAgainstToml`: its doc said the SDK cannot fetch the origin's stellar.toml, which `Sep7OriginCheck` does since 1.0.4; corrected.
  - QML `Seps.verifyUriAgainstToml` trimmed URI_REQUEST_SIGNING_KEY and skipped the account-id and origin checks (a padded key, or a URI with no origin_domain, came back valid); it runs `Sep7::verifyOrigin` on the pasted toml now, so it accepts and refuses what tomlSigningKeys and Sep7OriginCheck do.
  - SorobanServer::simulateTransaction could not send an instruction leeway or an auth mode (js and py can): new SimulateTransactionOptions (instructionLeeway, authMode, useUpgradedAuth) on new simulateTransaction / prepareTransaction overloads and AssembledTransaction::simulate(options), sent under js's names; the defaults still send the transaction alone. useUpgradedAuth is sent only when asked for (js sends true by default, py never): py 14.1.0 cannot sign ADDRESS_V2 entries and SEP-45 challenges take ADDRESS ones only.
  - AssembledTransaction::simulate() and signAuthEntries() went ahead after sign(), keeping a signature over the old body; signAndSend then added a second one and core refused the transaction (txBAD_AUTH_EXTRA). Both refuse once the transaction is signed, as py's prepare() and sign_auth_entries() do.
  - AssembledTransaction::sign() / signAndSend() signed and sent a call whose simulation asked for a restore (restorePreamble), which fails on chain with the fee paid, and signed one never simulated. They throw ExpiredStateException / NotYetSimulatedException now, as js and py refuse (ExpiredState); automatic restore is not done.
  - AssembledTransaction::sign() / signAndSend() signed and sent a read-only call, paying the fee for nothing. New isReadCall() (no authorization asked for, nothing written, as js and py decide it); a read call throws NoSignatureNeededException unless the new `force` argument is true, as js's and py's sign({force}).
  - AssembledTransaction::sign() / signAndSend() signed and sent a call whose auth entries for other G... addresses were still unsigned; the host refused it on chain and the fee was paid. They throw NeedsMoreSignaturesException (with the addresses) now, force or not, as js and py.
  - AssembledTransaction::signAuthEntries() with nothing to sign leaves the transaction as it was without an error (py does not throw either but re-signs an entry already signed; js throws); documented, with the custom-signer forms js and py have that are not here.
  - AssembledTransaction::needsNonInvokerSigningBy() documents its differences from js and py: C... addresses are not listed (js and py list them), and there is no includeAlreadySigned.
  - The ContractClient comment said soroban-rpc cannot supply an account; js and py read it with getLedgerEntries, and only the helper is missing here (deferred).
  - Comments: getLedgers / getTransactions take startLedger 0 with a cursor (it said the cursor took precedence; both were sent, and the RPC refuses the two); the SDK does derive an asset contract's id (Util::getContractAddressFromAsset, Asset::toSAC).
  - Memo::parse("id", ...) (Horizon's memo included) read text that is not a uint64 ("abc", "", 2^64, "1.5") as memo id 0 and took a sign or spaces; it takes decimal digits only now and throws otherwise, as js Memo.id does.
  - TimeBounds(-1, 0) was signed as minTime 2^64 - 1 (never valid) and a negative maxTime as one that never expires; TransactionBuilder::setMinSeqAge / setMinSeqLedgerGap got a negative value already wrapped. They are refused now, as js (and py for the bounds) refuse them; the setters take a qint64, and decoding goes through TimeBounds::fromTimePoints, which still reads TimePoints of 2^63 and up.
  - setTimeout(TIMEOUT_INFINITE) after addTimeBounds(min, 0) kept {min, 0}, where js writes {0, 0} (dropping min) and py refuses; it is refused now, as py does. setTimeout(n) still keeps the minTime, as js does.
  - RevokeDataSponsorshipOperation::getDataName and SetOptionsOperation::getHomeDomain dropped a leading UTF-8 BOM (QString::fromUtf8), so a revoke of BOM + "k" read as a revoke of "k"; the BOM is kept as U+FEFF now, as js and py keep it (ManageData's name since G24). TxRep, which refused that revoke only at its read-back check and a memo with two BOMs, refuses the first for its U+FEFF and writes the second.
  - With muxing off (AccountConverter(false)) the transaction source and a fee bump's fee source were still written as M... while everything else went out as G...; both go through the converter now, as in java. js and py have no switch and write M..., as muxing on does.
  - ChangeTrustAsset::fromXdr leaked both assets of a pool share it refused (out of order, a fee other than 30); they are freed now. The constructor still leaves refused assets with the caller, as TxRep expects; the header says so.
  - Amounts with '_' between digits ("1_000.5", read by js and py) and js's 0x/0b/0o forms (py refuses them) are refused on purpose, as easy to misread in an amount about to be signed; Operation::toXdrAmount now says what it reads and refuses.
  - Network::FUTURENET_NETWORK_PASSPHRASE(), STANDALONE_NETWORK_PASSPHRASE() and SANDBOX_NETWORK_PASSPHRASE(), as js Networks and py Network name them; the QML layer, TxRep and SEP-10 used their own copies of the strings.
  - Price(QString) refuses a price no int32 fraction holds (2^31 and up, 1/2^31 and below), as py does; js 17.1 signs it as 2147483647/1 or 1/2147483647 instead. The header claimed js parity there.
  - RequestBuilder: a query parameter set twice (cursor, limit, order, includeFailed, a filter such as forSigner) was sent twice; the second value now replaces the first where it was set, as js's searchParams.set and py's params dict (`limit(10).limit(20)` asks for 20).
  - `accounts()`: signer, sponsor, asset and liquidity_pool are refused together in either order; `forLiquidityPool(id).forSigner(...)` used to go out for Horizon to answer 400 (only the reverse order threw). js and py check neither order.
  - Trade aggregations: a resolution other than 60000/300000/900000/3600000/86400000/604800000 ms, or an offset that is not whole hours, above the resolution or a day or more, throws before the request, as js and py refuse them (a negative offset too, which js and py let through); it used to be Horizon's 400.
  - `effects().forLiquidityPool(id)` and `transactions().forLiquidityPool(id)` (GET /liquidity_pools/{id}/effects and /transactions), as js's forLiquidityPool and py's for_liquidity_pool; only accounts, operations and trades had it.
  - `payments().includeFailed(bool)`, as js's includeFailed and py's include_failed (operations and transactions had it); `join("transactions")` is not there yet.
  - `trades().stream()`, as js's and py's trades().stream(); cursor/limit/order now return the trades builder so `trades().cursor("now").stream()` chains.
  - `StrictReceivePathsRequestBuilder::destinationAccount` is documented as deprecated: /paths/strict-receive does not document destination_account (java legacy; js and py never send it). Kept for 1.0.x code.
  - SEP-29: a MEMO_ID 0 or an empty MEMO_TEXT counted as no memo, so `submitTransaction`/`checkMemoRequired` refused a payment js and py submit (both skip the check for any memo); any memo counts now, in QML `Seps.sep29Inspect` too.
  - `Server::submitTransactionAsync(tx, skipMemoRequiredCheck)`: POST /transactions_async, as js's submitAsyncTransaction and py's submit_transaction_async. Its `SubmitTransactionAsyncResponse` is ready() on PENDING and error() on DUPLICATE / ERROR / TRY_AGAIN_LATER (as js and py reject them) with `getTxStatus()`, `getHash()` and `getErrorResultXdr()` read either way; SEP-29 is checked first unless skipped.
  - SEP-2: a federation answer with memo_type text and an empty memo was refused as "a memo_type without a memo"; core takes MEMO_TEXT "" and js and py hand it back, so it is kept now (an empty id or hash memo is still refused).
  - SEP-2: the federation request set no redirect policy, so a Qt 5.15 build failed on a 301/302 that Qt 6, js and py follow; it follows redirects on both now (never https to http). The stellar.toml request still follows none, as js.
  - StellarToml::parse made up to three regex matches per value, each re-validating the document from there to its end (O(n^2): an estimated few seconds for a 100 KB toml of ints, on the GUI thread of a federation or SEP-7 lookup); the document is validated once now.
  - SEP-2 reverse lookup (resolveAccountId): an answer without account_id was refused and one without stellar_address accepted; it is read as py reads it now (stellar_address required, a missing account_id is the asked one), and an answer about another account is refused (py replaces it silently, js passes it through).
  - SEP-2: FederationServer::resolveAddress("bob") threw even when the server knew its domain; it looks up bob*<domain> now, as js does (a server with no domain still refuses it, saying why, as js does).
  - A stellar.toml or federation domain with a port ("anchor.example:8443", "bob*anchor.example:8443") was refused where js and py take it; an optional port is accepted now, and StellarTomlResolver drops surrounding whitespace and a trailing '/' as js does. IPv4 and IDN hosts stay accepted (as py and js's Resolver); js's federation check (validateDomain) refuses an IPv4 address, an IDN written in Unicode (it takes the same name in its xn-- form) and any label starting with a digit, which is not copied.
  - SEP-2: a query the FEDERATION_SERVER URL already carries is kept and type/q are added to it, as py does; js drops it. Documented (no change).
  - Federation::resolve(G... or M...) left json() empty; it is {"account_id": value} now, the record js's resolve() returns (js takes a G address only).
  - StellarToml::currency does not follow a [[CURRENCIES]] entry that links out with toml="..." nor match one listed by contract alone; documented, since an empty result then does not prove the asset is unlisted.
  - Docs: StellarTomlResolver matches js's Resolver, not py (py's fetch_stellar_toml follows redirects, reads 512 KB, refuses only a 404), and its timeout is an inactivity timeout; resolveTransactionId has no py equivalent and returns the record of the transaction's sender.
  - Docs: FederationException also reports a federation answer stopped past 100 KB, not only stellar.toml failures; an answer refused once read is an UnknownResponseException.
  - SEP-10 `verifyChallengeTransactionSigners` refuses a signer that starts with G but is not a valid account id, and names it. A typo in the second character (an unknown version byte) used to be skipped without a word, so the signer meant was left out of the check. py refuses it in either order; js refuses it when it reaches it. Other non-G entries are still skipped, as js skips them.
  - TxRep: claim predicate times carry py's gloss too. absBefore is followed by its UTC date, and relBefore by the date that many seconds after 1970, as py writes it (`3600 (1970-01-01 01:00:00.000000+00:00 (UTC))`). Before 0 or after year 9999, the number is written alone. A document with time predicates is now py's `to_txrep` line for line, except for the `notPredicate._present` line SEP-11 asks for.
  - QML Seps.resolveFederation reported every failure that was not a refused answer as one fixed "the lookup failed: ..." text, so the reason K5 gives (a stellar.toml that could not be read, no or a non-https FEDERATION_SERVER) and the federation server's HTTP status were lost; the message carries them now (kind still "connection"), as js and py tell these apart.
  - QML Horizon.fundAccount still refused a local quickstart and any Horizon other than SDF's testnet/futurenet unless friendbotUrl was set; it asks <endpoint>/friendbot now, as js-stellar-sdk's Horizon.Server.friendbot() does (horizon.stellar.org is still refused), and keeps a query friendbotUrl carries (setQuery() dropped it). A contract address (C...) is refused, documented: js funds one only through rpc.Server.fundAddress, which this layer has no counterpart of.
  - QML Xdr.decode (and looksLikeBase64) refused base64 with a space, tab or line break inside it, so line-wrapped XDR pasted from an email or a 76-column dump was "not valid base64"; those are skipped now, as js and py decode such text, and any other character outside the alphabet is still refused (as js refuses it).
  - QML Seps.tomlSigningKeys: its header said an empty key came back distinct from an absent one; since K23 a SIGNING_KEY or URI_REQUEST_SIGNING_KEY that is not an account id, an empty string included, makes the whole answer { error } (a good SIGNING_KEY is not handed on either), which is now documented as the intended fail-closed behaviour (js and py have no such helper and return the toml unchecked).
  - QML help texts said what the builder does not do: changeTrust's asset offered "native" (stellar-core refuses a native trustline as malformed) and createClaimableBalance's amount said "1e7" is rejected (it is ten million units since G12, as in js and py); a stale "-1 when invalid" comment sat above amountToStroops.
  - Ed25519 verification accepted public keys of small order (GAAQ...HV4, GAAAA...WHF and the other 12 encodings of libsodium's blocklist), so anybody could make a signature that verified for those account ids without a secret key, and non-canonical ones (y >= p, a second account id for the same key); `KeyPair::verify` (and through it SEP-10, SEP-7, SEP-45 and SEP-53) refuses both now, with libsodium's ge25519_is_canonical / ge25519_has_small_order checks, as stellar-core, js and py.
  - Ed25519 verification accepted a signature whose R is of small order (R = the identity, S = h * a), which only the key holder can make and which stellar-core (libsodium) and py refuse; `KeyPair::verify` refuses it now. js 17.1.0 accepts it, and core wins.
  - operations().operation(id) requested /operation/{id}, which Horizon answers with 404 (since the first version). It requests /operations/{id} now, as js and py; the other single-record calls (account, ledger, transaction, offer, claimable balance, pool) were already right.
  - An effect or operation of a type the SDK does not know (a newer Horizon's) made the whole page, operations().operation(id) and each stream event of that type fail. It is kept now as a plain EffectResponse / OperationResponse (getType() names it, json() has every member), as js and py keep the record; type_i must still be a whole number and the common members are still required.
  - Page::getNextPage() crashed on a page with a next link but no network reply (read with loadFromJson, or after its QNetworkAccessManager was deleted); it returns null then. getPrevPage() follows _links.prev, as js's and py's prev().
  - A stream answered 429 read Retry-After (seconds) as milliseconds, so it reconnected after 1 s whatever Horizon asked and kept hitting the limit; it now waits the Retry-After for that reconnect (js stops until its 15 s reconnect timer, py ends the stream).
  - A stream that could not connect (refused, DNS, TLS, no answer) or got an HTTP error with no body never emitted error() and retried every second forever; one with an error body emitted error() per piece of it, and a 2xx web page read as a stream without events. Each failed connection now emits error() once with its exception (a 2xx that is not text/event-stream an UnknownResponseException) and the stream reconnects, as js's EventSource reports them (js then waits its 15 s reconnect timer, py ends the stream).
  - Deleting a stream (or its Server) from the stream's ready()/error() deleted the network reply while Qt was still inside its readyRead, a use-after-free; the reply, and a deleted Server's network manager, are now deleted later from the event loop.
  - The SSE reader ignored the event: field (a stream's "event: error" was reported as "JSON parse failed", its message lost, and an event of another type was read as a record) and knew only LF and CRLF line ends (an event written with CRs never ended); it now reads them as js's EventSource does, and Page::streamedElement() returns nullptr instead of indexing an empty list before the first record or after an unreadable event.
  - Horizon's problem document (title, detail, extras.result_codes) was attached to the exception only when the answer said application/problem+json, so behind a proxy that rewrote the Content-Type getProblem() was null; it is read from any JSON object body now, as js and py read it.
  - Deleting a Server while a submitTransaction / submitTransactionAsync / checkMemoRequired of its was in flight (the POST, a SEP-29 lookup, or before the check started) left the caller's response waiting forever; it now emits error() with a ConnectionErrorException saying the Server was deleted (a submission may still have reached Horizon).
  - `getLatestLedger` dropped closeTime, headerXdr and metadataXdr, which js and py both read. GetLatestLedgerResponse now has getCloseTime() (decimal text), getHeaderXdr() and getMetadataXdr() (base64). All three are required: py requires all three and js the two XDRs.
  - XDR: an SCVal's bytes, string, symbol or executable tag, a ContractExecutable external-ref tag, an invocation's function name and an uploaded wasm were read with their padding unchecked, so non-zero padding was accepted and written back as zeros. It is refused now, as core and js refuse it (py-stellar-sdk 14.1.0 accepts it). A Soroban footprint no longer reserves memory for the key count it claims before reading the keys (with Qt 6, a short hostile simulation answer allocated ~240 times its size).
  - The network manager StellarTomlResolver::resolve (without one given) and FederationServer use was one per process, made by whichever thread called first: made from a worker thread, it served the GUI thread from the wrong thread, and none at all once the worker ended. Each thread now has its own (new StellarTomlResolver::defaultNetworkAccessManager()), and the federation server cache is the calling thread's.
  - AssembledTransaction: after a simulate() that failed (an earlier one having succeeded), sign() and signAndSend() signed and sent the transaction with the earlier simulation's footprint, fee and auth, which fails on chain with the fee paid. A failed simulation leaves it unsimulated now (isSimulated() false), and sign() refuses it, forced or not (SimulationFailedException), as js does; py refuses unless forced.
  - AssembledTransaction::needsNonInvokerSigningBy skipped ADDRESS_WITH_DELEGATES auth entries, so sign() signed and sent a call whose delegated G... entry was unsigned, for the host to refuse with the fee paid. Such an entry is listed now while its top-level signature is missing (its delegates are not looked at), and sign() refuses it, as js 17.1.0 does; py has no delegate credentials. signAuthEntries() still cannot sign one.
  - QML Horizon: a HorizonQml deleted with a submit on the wire could be written to after it was freed, by its POST's upload progress (the reply has outlived its Server since I8); the watch's connection now goes with the HorizonQml.
  - QML Xdr.decode / looksLikeBase64 refused a form feed that js and py skip, and took a vertical tab or a Unicode space at the ends that js refuses: they skip exactly space, tab, LF, form feed and CR now, as js does (py skips any character outside the alphabet).
  - StrKey::decodeCheck still left the decoded copies of a seed freed but not zeroed when it refused one for its checksum (a single mistyped character: 32 bytes that are almost the seed), its version byte or unused bits; a scope guard wipes them on every throw now, as it already did on the length and canonical failures (js and py wipe nothing).
  - keypair.h: KeyPair::verify and verifyMessage now say what they refuse (a small-order or non-canonical public key, a small-order R, an S not below the order, a signature not of 64 bytes) and that js 17.1.0 accepts a small-order R where stellar-core and py refuse it, core winning; the old text promised an exception verify never throws.
  - `FederationServer::createForDomain(" acme.com/")` read acme.com's stellar.toml but kept the domain as given: a name alone was asked for as "bob* acme.com/", and the cache held it apart from "acme.com" (the toml was read twice). The domain is trimmed once, as the stellar.toml request reads it, and that is the server's domain and cache key (js refuses such a domain in createForDomain and trims it in its Resolver).
  - The per-thread federation cache (I13) was a thread_local pointer nothing freed: a worker thread that turned it on and ended without `activeCache(false)` leaked the cache and every server in it. It is held in a QThreadStorage now, and deleted with its servers when the thread ends, as the thread's network manager is.
  - A SEP-2 reverse lookup answered with `"account_id":""` was refused as "about another account"; it is the record of the account asked for now, as a missing or null one already was and as py reads it (js hands back "").
  - A stream answered with an HTTP error whose body kept coming (a slow drip, or no end) never emitted error() and buffered the body without bound: its bytes restarted the timeout. Only event-stream data restarts it now; the error has replyTimeout() from the request, then its status is reported (without the body) and the stream reconnects. js and py report on the status alone (J8).
  - A stream answered 2xx with a body that is not an event stream and never ends (a proxy relabelling Horizon's stream) said so once and then stayed on that connection forever, deaf: the connection is aborted now, so it reconnects and reports once per connection, as js and py give it up (J9).
  - A transaction whose id memo is "-0" passed the response check and then made getMemo() throw: the check refuses it now, as Memo::parse() and js's Memo.id do (J11).
  - SEP-7 pay: `memo=+5&memo_type=MEMO_ID` (a raw '+' is a space) and `memo=%205` were read as the id " 5", kept with its space, and Memo::parse("id", ...) then refused it; a MEMO_ID is decimal digits only now, as js Memo.id (py's int() takes "+5", " 5", "-0" and "1_0"; sep7.h lists the departure).
  - SEP-7 origin: an internationalised origin_domain was checked in its ACE form, but `Sep7::originDomain` (QML `Seps.verifyUriAgainstToml`'s `originDomain`) handed back the Unicode one, so example.com spelled with a Cyrillic a verified as xn--exmple-4nf.com and was shown as "example.com"; it comes back in its ACE form now, the one to show and compare (the parsed `originDomain` field still holds what the URI says, as py).
  - getVersionInfo is checked against what js and py read of a real answer. Only the camelCase names are read, as py reads them: snake_case names next to them are ignored, and an answer with only snake_case names is refused (J17).

### Fixed — wire format and hostile input

- Result codes were byte-reversed (19 enums, 9 structs); INFLATION wrote an
  uninitialised MuxedAccount; inner fee-bump results lost 4 bytes; ClaimAtom and
  the Soroban results had no arm; every closed union read or wrote an unknown
  arm as nothing; XDR flags accepted any non-zero value; a contract data
  key's durability and a config setting id took any int32; ledger entries
  lacked their extensions. All match stellar-xdr `curr` now.
- A 5 KB envelope overflowed the stack and 12 bytes made the decoder reserve
  3 GB; nesting and every count are bounded.

### Added

- **Optional QML layer** (`DEFINES += STELLAR_QT_QML`, or `-DSTELLAR_QT_QML=ON`):
  `Stellar`, `Tx`, `Horizon`, `Soroban`, `Seps` and `Xdr` singletons. No
  pointers or exceptions cross into QML, and nothing the caller gives is
  replaced by a nearby value: integers (account indexes and counts included)
  are read exactly, an amount given as a JS number only while it is exact
  (text otherwise), contract arguments include u128/i128/u256/i256 as decimal
  text, and every call that takes a map refuses a key it does not read.
- **ContractSpec / WasmSections**: a contract's interface from its own `.wasm`.
- SEP-45 v0.1.1; `Txrep::toTxrep(FeeBumpTransaction*)` and
  `fromTxrepEnvelope()`; `SorobanServer::getAssetBalance`, `setRequestTimeout`,
  `cancelRequests`; BIP-39 legacy recovery; `Scv::toInt128`/`toUint128`/
  `decimalText`; `RevokeTrustlineSponsorshipOperation::create(account, poolId)`.
- `AbstractTransaction::addSignature` (a signature made elsewhere: a wallet,
  an HSM, a CAP-40 signed payload; or a public key and a base64 signature,
  verified against the hash, as js). `TimeBounds::getMinTimePoint` /
  `getMaxTimePoint` (uint64). From Horizon: an account's
  `last_modified_ledger` / `last_modified_time`, an asset's
  `auth_immutable` / `auth_clawback_enabled`, a claimable balance's
  `flags.clawback_enabled`, the root's `ingest_latest_ledger`,
  `history_latest_ledger_closed_at` and `supported_protocol_version`;
  `Response::json()` (the record as Horizon sent it).
  **The value a Soroban call returned on chain**: `TransactionMeta` v0-v4
  (`xdr/stellarledgermeta.h`: every ledger-entry change, contract data and
  code, TTLs, contract/diagnostic/transaction events), decoded exactly by
  `stellar::decodeTransactionMeta`; `GetTransactionResponse::getTransactionMeta()`,
  `hasReturnValue()`, `getReturnValue()` (js `getTransaction().returnValue`)
  and `AssembledTransaction::landedResult()` (js `SentTransaction.result`;
  `result()` stays the simulation's).
  QML `Stellar.strKeyKind()` names the signer-key strkeys too
  ("preAuthTx" T, "sha256Hash" X, "signedPayload" P; they read as none).
  `AccountResponseAttach::Data::keys()`, `ChangeTrustOperation::MAX_LIMIT`
  (js's default limit), `ResultCodes::getInnerTransactionResultCode()`.
- **stellar.toml, SEP-2, SEP-7, SEP-10** (`stellartomlresolver.h`,
  `sep7origincheck.h`): `StellarTomlResolver::resolve` / `request` /
  `read` (a domain's stellar.toml, as js's `StellarToml.Resolver`) and
  `StellarTomlResolver::isHostName`; `StellarToml::currency(toml, code,
  issuer)`; `FederationServer::resolveAccountId` / `resolveTransactionId`
  and `FederationException`;
  `Sep7Tx::fromUri(uri, expectedNetworkPassphrase)`,
  `Sep7Tx::effectiveNetworkPassphrase()` and
  `Sep7Tx::transaction(network)`; `Sep7::verifyOrigin`,
  `Sep7::originDomain` (in its ACE form, xn--... for an internationalised
  name), `Sep7::isFullyQualifiedDomainName` and
  `Sep7OriginCheck::verify(uri)`; `Sep10Challenge::webAuthFromToml(toml)`.
- **Horizon**: `Server::checkMemoRequired(transaction)` (a
  `MemoRequiredCheck`, nothing submitted) and
  `Server::submitTransactionAsync(transaction, skipMemoRequiredCheck)`
  (POST /transactions_async, `SubmitTransactionAsyncResponse`);
  `effects().forLiquidityPool(id)`, `transactions().forLiquidityPool(id)`,
  `payments().includeFailed(bool)`, `trades().stream()`; `Sep29::DATA_NAME`,
  `DATA_VALUE`, `DATA_VALUE_BASE64` (`sep29.h`);
  `Network::FUTURENET_NETWORK_PASSPHRASE()`,
  `STANDALONE_NETWORK_PASSPHRASE()`, `SANDBOX_NETWORK_PASSPHRASE()`.
- **Soroban**: `SimulateTransactionOptions` (`instructionLeeway`,
  `authMode`, `useUpgradedAuth`) on new `SorobanServer::simulateTransaction`,
  `simulateTransactionXdr` and `prepareTransaction` overloads and
  `AssembledTransaction::simulate(options)`;
  `AssembledTransaction::isReadCall()`, a `force` argument on `sign()` and
  `signAndSend()`, and `ExpiredStateException`,
  `NoSignatureNeededException`, `NeedsMoreSignaturesException`
  (`getAddresses()`).
- **Keys**: `KeyPair::fromMnemonic(words, passphrase, account, language)`,
  `KeyPair::fromBip39Seed(seed, path)` (a SLIP-10 path),
  `KeyPair::signMessage` / `verifyMessage` (SEP-53), `MessageAuth::PREFIX`
  and QString / const char* overloads of `MessageAuth::signMessage` /
  `verifyMessage`; the BIP-39 wordlists py has
  (`Util::SupportedMnemonicLanguages` JA, FR, ES, IT, KO, ZH_HANS, ZH_HANT)
  on `entropyToMnemonic`, `generateMnemonic`, `validateMnemonic` and
  `mnemonicToBIP39Seed`; `Util::mnemonicProblem`;
  `StrKey::decodeAny` (`StrKey::Decoded`).
- **TOID** (SEP-35): `MAX_LEDGER`, `afterLedger`, `ledgerRangeInclusive`,
  `incOperationOrder`, `operator<`, `qHash`, `toUInt64`, `toString`,
  `fromString`. `TimeBounds::fromTimePoints` (any uint64, for decoding).
- **QML**: `Horizon.friendbotUrl` (property) and
  `Horizon.friendbotUrlFor(endpoint)`.
- **From the release review**: `PageBase::getPrevPage()` (follows
  _links.prev, as js's and py's prev());
  `GetLatestLedgerResponse::getCloseTime()` (decimal text),
  `getHeaderXdr()` and `getMetadataXdr()` (base64);
  `StellarTomlResolver::defaultNetworkAccessManager()` (the
  calling thread's own manager, which resolve() without one and
  FederationServer use). `AssembledTransaction::sign()` /
  `signAndSend()` throw `SimulationFailedException` after a failed
  simulation.

### Build

- CMake reads its file lists and the version from the qmake files, installs a
  `find_package(StellarQtSDK)` package with the whole-archive target
  `StellarQtSDK::StellarQtSDK` (Debug as `StellarQtSDKd`; RelWithDebInfo and
  MinSizeRel consumers get the release archive), defaults `STELLAR_QT_QML` to
  OFF, and CI builds it (QML on and off, Qt 5.15 and 6) and links a consumer
  through the installed package.

### Changed (read before upgrading)

- `TIMEOUT_INFINITE` writes time bounds {0, 0} (as js and java): such
  transactions hash differently from 1.0.3.
- A BIP-39 passphrase whose NFKD form differs from what was typed derives
  different (standard) accounts; recover old ones with the legacy functions.
- Operations and `FeeBumpTransactionBuilder` are not copyable;
  `TransactionBuilder` is not assignable; `setBaseFee` takes a `qint64`.
  Classes gained members and virtual functions (`Response`, several
  responses, `MemoText`...) and XDR types changed: rebuild everything that
  links a prebuilt 1.0.x library.
- `ContractClient::invoke`/`read` and `restoreFootprint` sign with time bounds
  of now + 300 s by default (1.0.3: none); pass 0 (`timeoutSeconds`,
  `txTimeoutSeconds`) for a transaction that never expires.
  A flow that gathers signatures for longer gets `tx_too_late`.
- Every `SorobanServer` request times out after 30 s by default
  (`setRequestTimeout`).
- `build()` needs `setTimeout` (or time bounds) again after
  `addPreconditions` and for every build; a builder copied from another is
  checked the same way.
- `Price(n, d)` compiles only with `qint32` terms, and `Scv::to*` only with
  integers of their range (float, bool and enum arguments no longer compile;
  out-of-range values throw `std::out_of_range`).
- `Util::hexToBytes` throws on text that is not hex of even length.
- Offer and path-payment results hold `ClaimAtom`s (`offers`,
  `offersClaimed`): `.sellerID` is now `.orderBook.sellerID`.
- `Sep45Challenge::verifyChallenge` (six arguments) requires the
  `web_auth_verify` function, and `newChallenge` a timeout of at most 86400 s.
- `Operation::fromXdrAmount` writes a negative value as `-1.5` (it wrote
  `-1.-5`).
- Many inputs that used to be read as 0, truncated or signed are refused:
  malformed prices, fees and amounts, TxRep documents whose comments do not
  match, Horizon and RPC answers that do not fit their fields, SEP-10/45
  challenges that are not exactly a login.
- SEP-10 nonces are written in standard base64; SEP-45's QML calls speak
  v0.1.1; TxRep writes signatures, bare revoke-data names and py's muxed form.
- `Memo::hash` / `Memo::returnHash` take exactly 32 bytes (64 hex
  characters); shorter input was padded with zeros.
- Refused now as js/core refuse them: asset codes outside `[A-Za-z0-9]`,
  lower-case strkeys, zero amounts in the operations above, pool fees other
  than 30, ledger bounds with min > max. `setBaseFee` accepts 0..UINT32_MAX.
- A custom network passphrase outside ASCII has another network id (UTF-8,
  as core); an `Account` made from an M... address signs as that M account.
- `Price::toString` writes the exact decimal js writes (20 digits, rounded
  half up): decoded offer prices read differently from 1.0.3.
- QML: `manageData` deletes only when `value` is left out; names, domains and
  symbols are not trimmed; `allowTrust` needs `authorize`; `createContract`
  is always v2.
- `AssetCodeLengthInvalidException` is a `std::runtime_error` with a message.
- CMake consumers link `StellarQtSDK::StellarQtSDK`; headers install under
  `include/StellarQtSDK/`.
- Keys and signatures: `KeyPair::verify` (and addSignature, SEP-7, SEP-10,
  SEP-45, SEP-53 through it) is false for a signature that is not exactly
  64 bytes, whose S is not below the group order or whose R is of small
  order, and for a public key of small order (the 14 encodings of
  libsodium's blocklist, GAAQ...HV4 among them) or not canonical, as
  stellar-core and py refuse them (js 17.1.0 accepts the small-order R).
  `StrKey::decodeCheck` refuses a body of the wrong length for its kind, a
  non-canonical spelling, a B... of another type and a P... whose
  declared length or padding is wrong.
  `StrKey::decodeVersionByte` reads Latin-1.
- BIP-39: `Util::mnemonicToBIP39Seed` throws on a phrase whose checksum
  fails, with `Util::mnemonicProblem`'s reason as its message (the legacy
  recovery functions still open one). `fromSecretSeed(QByteArray)` still
  refuses anything but 32 bytes, with a new message (an S... strkey is
  told to go to `fromSecretSeed(QString)`).
- Base64 is decoded strictly: a Horizon memo hash or return that is not
  base64 fails the response, `Data::getDecoded` returns empty, SEP-45
  simulation auth entries are refused; QML `Xdr.decode` and
  `looksLikeBase64` skip exactly space, tab, LF, form feed and CR, at the
  ends or inside, and refuse any other character outside the alphabet, a
  vertical tab or a Unicode space at the ends included, as js does.
- `StellarToml::parse` is a TOML 1.0 reader: invalid TOML throws (naming the
  line), inline tables, dotted keys and `[a.b]` are nested maps, an integer
  outside int64 throws; QML `Seps.parseToml` gives an integer past 2^53 as
  its decimal text.
- SEP-2: a failed stellar.toml step fails the response with a
  `FederationException` saying why (it was a bare error()), the toml is
  fetched by `StellarTomlResolver` (https, no redirect, 100 KB) and an
  answer over 100 KB is refused. A reverse lookup's answer needs
  stellar_address, and one about another account is refused.
  `resolveAddress("bob")` asks for bob*<domain>; a domain with a port is
  accepted; an empty text memo is kept; the federation request follows
  redirects on Qt 5 too (never https to http). `Federation::resolve(G...)`
  emits ready() (queued) and its json() is `{"account_id": ...}`.
  `FederationServer::createForDomain` trims the domain and drops a
  trailing '/' or '\' as the stellar.toml request does: that form is
  `getDomain()`, where a name alone is looked up, and the cache key. A
  reverse lookup answered with an empty `account_id` is the record of the
  account asked for (it was refused). The default network manager
  (`StellarTomlResolver::defaultNetworkAccessManager()`) and the
  federation cache are the calling thread's: `activeCache(true)` turns
  it on for that thread only, and a thread's cache and manager are
  deleted when it ends.
- SEP-7: `verify()` checks the URI as received; `fromUri()` refuses what
  `toUri()` refuses (destination, amount, asset code and issuer, a
  memo_type with no memo), a parameter after `signature` and a parameter
  given twice. A raw '+' in a value reads as a space (except in the base64
  values), and `asset_code=XLM` with no issuer is the native asset. A
  MEMO_ID is decimal digits only, at most 2^64 - 1 (`memo=+5`, read as
  " 5", is refused; py takes it as 5). `Sep7::originDomain` and QML
  `Seps.verifyUriAgainstToml`'s `originDomain` give an internationalised
  origin in its ACE form (xn--...), the form checked.
  `Sep7Tx` refuses an xdr that is not a transaction envelope;
  `Sep7Tx::fromUri(uri, expected)` reads a URI naming no network for
  `expected`, and with an empty `expected` refuses it. QML
  `Seps.verifyUriAgainstToml` runs `Sep7::verifyOrigin` and answers
  `{ error }` for a URI with no or a bad origin_domain, no signature or a
  key that is not an account id (it answered `{ valid: false }` or passed).
- SEP-10: `verifyChallengeTransactionSigners` throws for a secret seed
  among the signers and for a signer starting with G that is not an
  account id (both were skipped). A challenge read with the wrong network
  says so in its error when the server signed it for another known one.
- SEP-29: any memo satisfies the memo-required check, MEMO_ID 0 and an
  empty MEMO_TEXT included (they were refused with
  `AccountRequiresMemoException`); in QML `Seps.sep29Inspect` too.
  `Server::checkMemoRequired` is now the public check
  (`MemoRequiredCheck*`); the private destination list it named is
  `memoCheckedDestinations`.
- TxRep writes py's glosses: amounts with their units, time bounds and claim
  predicate times with their UTC date. Code that compares or parses the
  text the emitter writes must expect them.
- Transactions: `Memo::parse("id", ...)` (Horizon's memo included) throws
  on text that is not a uint64 (it read 0), and a Horizon transaction
  whose id memo is "-0" fails the response (its getMemo() threw).
  `TimeBounds` refuses negative times, and `setMinSeqAge` /
  `setMinSeqLedgerGap` take a `qint64` and
  refuse a negative value. `setTimeout(TIMEOUT_INFINITE)` after a minTime
  is set throws. A C++ `BumpSequenceOperation` with a negative bumpTo and a
  `TOID` with a ledger past 2^31 - 1 throw when built.
  `RevokeDataSponsorshipOperation::getDataName` and
  `SetOptionsOperation::getHomeDomain` keep a leading BOM as U+FEFF. With
  muxing off (`AccountConverter(false)`) the transaction and fee-bump
  sources are written as G...: such transactions hash differently.
- `AssembledTransaction::sign()` / `signAndSend()` throw
  `NotYetSimulatedException` before `simulate()`,
  `SimulationFailedException` after a simulate() that failed, forced or
  not and even after an earlier successful one (`isSimulated()` is false
  then; `NotYetSimulatedException` when its answer could not be
  applied), `ExpiredStateException` while the simulation asks for
  a restore, `NeedsMoreSignaturesException` while other G... addresses
  have unsigned auth entries (an ADDRESS_WITH_DELEGATES entry whose
  top-level signature is missing included), and
  `NoSignatureNeededException` for a read call unless `force` is true;
  `simulate()` and `signAuthEntries()` throw once the transaction is
  signed.
- Horizon requests: a parameter set twice (`limit`, `cursor`, a filter...)
  is sent once, with the last value; `accounts()` refuses a second, other
  filter in either order; trade aggregations throw before the request for a
  resolution or an offset Horizon refuses.
- QML: `Seps.tomlSigningKeys` answers `{ error }` when SIGNING_KEY or
  URI_REQUEST_SIGNING_KEY is present but not an account id;
  `Horizon.fundAccount` picks the friendbot by the endpoint (SDF's testnet
  and futurenet ones, `<endpoint>/friendbot` for any other Horizon but the
  public network's, or `friendbotUrl`),
  `Seps.resolveFederation` messages carry the reason of the failure, and
  `Stellar.strKeyKind()` names T..., X... and P... keys (they read as none).
- Horizon and RPC answers: an effect or operation of a type the SDK does
  not know is a plain `EffectResponse` / `OperationResponse` (getType()
  names it, json() holds it) where the page, `operations().operation(id)`
  or the stream event failed: code that casts every record to a known
  subclass must expect the base class. `getProblem()` is read from any
  JSON object body, whatever its Content-Type. `getLatestLedger` refuses
  an answer without closeTime, headerXdr or metadataXdr. XDR with
  non-zero padding after an SCVal's bytes, string, symbol or executable
  tag, an external-ref tag, a function name or an uploaded wasm is
  refused, as core and js refuse it (py accepts it). `getNextPage()`
  returns null (it crashed) for a page with no reply to ask through: one
  read with loadFromJson(), or whose reply or network manager is gone.
- Streams: a connection that fails (refused, DNS, TLS, no answer, an
  HTTP error, a 2xx that is not text/event-stream) emits error() once
  with its exception and the stream reconnects; it retried in silence,
  or emitted error() per piece of an error body. An HTTP error has the
  request's replyTimeout() to finish and is reported by its status, and
  a 2xx that is not an event stream is aborted after its error(). A 429
  waits its Retry-After seconds before that reconnect. An `event: error`
  is reported with its message (`UnknownResponseException`), an event of
  another type than message is not read as a record, and lone CR line
  ends are read. `Page<T>::streamedElement()` returns nullptr before the
  first record and after an event that could not be read: check it.
- A stream or its `Server` may be deleted from the stream's ready() or
  error(): the reply and the Server's network manager are deleted later,
  from the event loop. Deleting a `Server` ends the caller's pending
  `submitTransaction`, `submitTransactionAsync` and `checkMemoRequired`
  with error() (a `ConnectionErrorException`; a submission may still
  have reached Horizon), where they waited forever.
- Exception messages changed in many places (BIP-39, seeds, federation,
  SEP-7, SEP-10): code that matches on their text must be checked.

## 1.0.3

Protocol-correctness release: finishes the Protocol 28 work started in 1.0.2 by
fixing the record that *contains* the new CAP-85 arm, and closes the Protocol 27
(CAP-71) gap. No new public classes.

### Fixed

- **Every `SCV_VEC` and `SCV_MAP` was 4 bytes short of Core.** The canonical
  union declares them as XDR *optionals* — `SCVec *vec;` / `SCMap *map;`, with
  the comment "Vec and Map are recursive so need to live behind an option" — so
  each carries a 4-byte present flag before the array (RFC 4506 §4.19). The SDK
  wrote only the length. Consequences: every container the SDK produced was
  short, every container Core produced was misparsed (the present flag was read
  as the element count), the auth signature built by
  `SorobanCredentialsSigner` (a vec wrapping a map) went into submitted
  transactions 8 bytes short, and the SAC balance ledger key built by
  `SorobanServer` could never match an entry. Writer and reader were wrong
  together, so every round-trip test passed.
- **`SCAddress`'s muxed arm had its two fields in the wrong order.** Canonical
  is `struct MuxedEd25519Account { uint64 id; uint256 ed25519; }` — id first.
  The SDK emitted the SEP-23 strkey order (ed25519 then id). Reversed in both
  directions, so the round-trip test passed while every muxed `SCAddress` on
  the wire was wrong. The classic `MuxedAccount` in `stellartransaction.h`
  already had it right.
- **Contract instances did not interoperate with Core.** Canonical XDR is
  `SCContractInstance { ContractExecutable executable; SCMap* storage; }` — a
  4-byte present flag followed by a *bare* length-prefixed `SCMapEntry` array.
  The SDK serialised `storage` as a whole `SCVal`, which prepends its `SCV_MAP`
  discriminant, so every instance the SDK wrote was 4 bytes longer than Core's
  and every instance Core produced was misparsed (the map's element count was
  read as an `SCValType` tag). Writer and reader agreed with each other, which
  is why the round-trip tests passed. This undercut 1.0.2's CAP-85 support: the
  new external-ref arm decoded correctly but the record carrying it did not.
  The wire form is now canonical, pinned by a test that decodes a hand-assembled
  byte stream and asserts the exact encoded size. Note this fix only becomes
  complete together with the `SCV_VEC`/`SCV_MAP` fix above: a real instance's
  storage entries are usually themselves maps or vectors.
- **`SCValType` was missing `SCV_EXECUTABLE_TAG = 22`**, CAP-85's companion
  value (it carries an `SCString`). Protocol 28 decoding was incomplete without
  it.
- **`SCAddress` silently desynced on an unrecognised address type** — the same
  defect 1.0.2 fixed one level up in `ContractExecutable`. Both stream
  operators now throw on an unknown arm instead of consuming the discriminant
  alone and leaving the payload in the stream.

### Added

- **Protocol 27 (CAP-71) Soroban credentials.** `SorobanCredentialsType` gains
  `SOROBAN_CREDENTIALS_ADDRESS_V2` and `SOROBAN_CREDENTIALS_ADDRESS_WITH_DELEGATES`,
  with the new `SorobanAddressCredentialsWithDelegates` and the self-recursive
  `SorobanDelegateSignature` (nested delegate chains round-trip). `EnvelopeType`
  gains `ENVELOPE_TYPE_SOROBAN_AUTHORIZATION_WITH_ADDRESS = 10`. An unknown
  credentials arm now throws rather than desyncing — previously an entry
  produced by any SDK already emitting V2 would have corrupted everything
  decoded after it.
- **Signing for `ADDRESS_V2`.** `SorobanCredentialsSigner::hashedSignaturePayload`
  builds the CAP-71 preimage (the CAP-46 layout with the signer's `SCAddress`
  inserted between `signatureExpirationLedger` and `invocation`), and
  `authorizeEntry` signs V2 entries. Binding the address is what stops a
  signature being replayed under a different account sharing the same private
  key. `AssembledTransaction` now treats V2 entries as signable rather than
  ignoring them. `ADDRESS_WITH_DELEGATES` throws from `authorizeEntry`: a
  delegate chain cannot be signed by a single keypair, and returning the entry
  unsigned would look like success and fail only at simulate.

### Fixed (pre-freeze defect sweep)

Nine defects surfaced while scripting the guided tour for the lab, plus the
siblings found by sweeping the whole repo for each defect class. **None was
covered by a test**: the suite passed with every one of them present. Each fix
now ships with a regression that was verified to fail — or crash — when the fix
is reverted.

Memory corruption:

- **`PathPaymentStrictSend/Receive/PathPaymentOperation::setPath()`:
  use-after-free and double-free.** All three deleted the `Asset*` entries of the
  lazy cache `m_path` without clearing it, so the next `getPath()` handed back
  freed pointers and the destructor deleted them again. The triggering sequence
  — `getPath()` to render the path, `setPath()` to change it — is what any UI
  does. With the fix reverted the test crashes with an access violation in the
  destructor.
- **`Claimant` and `Predicate::Not/Or/And` violated the rule of three.** Each had
  a copy constructor and a destructor but no assignment operator, so the implicit
  one copied the pointers and both objects deleted them. Copying a
  default-constructed `Claimant` or `Not` also dereferenced a null predicate.
  These are types that live in `QList` and are exposed to QML, where copying and
  assigning is routine.
- **`AccountResponseAttach::Balance` had the same defect and leaked on top.** It
  is a `Q_GADGET` holding two owned pointers, returned by value inside a `QList`
  from `AccountResponse::getBalances()`, and its destructor freed only one of the
  two. Copies now start with empty caches (they rebuild themselves from the other
  members) and both pointers are freed.
- **`KeyPair::fromPublicKey` read past the end of the buffer.** The constructor
  `memcpy`s 32 bytes and nothing checked the size, so a shorter `QByteArray` read
  adjacent memory and produced a half-formed key, silently.

Signing something other than what was approved:

- **`ManageDataOperation` silently truncated** both the name (a trimming loop)
  and the value (`Array::set` with a `qMin`). `setHomeDomain` already threw in
  the same situation; now this does too.
- **`SetOptionsOperation` masked the signer weight with `& 0xFF`**, in the
  constructor and in `setSigner`. A weight of 256 became 0 — and weight 0
  **deletes** the signer, so a range error could lock the user out of their own
  account.
- **`AllowTrustOperation` truncated the asset code** with a fixed 12-byte
  `memcpy`, signing an authorization over a different asset.
- **`AssetTypeCreditAlphaNum4/12` validated in characters and copied in bytes.** A
  4-character non-ASCII code takes more than 4 bytes and was trimmed.
- **`Predicate::And/Or` dropped the third operand** (`Array::append` silently
  ignores anything past its maximum), so the signed claim condition was weaker
  than the one the user built.
- **Operations, signatures and claimants past their maximum were dropped at
  serialization time.** A transaction with more than `MAX_OPS_PER_TX` operations,
  more than 20 signatures or more than 10 claimants was signed truncated.
- **A HASH_X preimage longer than 64 bytes was truncated**, producing a signature
  that cannot satisfy the hash.

Other:

- **`Operation::fromXdr` had no `INFLATION` case** and threw "Unknown operation
  body", so any historical envelope containing it was undecodable and the XDR
  Inspector died there. `InflationOperation::create()` was also not `static`,
  unlike every other operation, so it did not even compile.
- **`OrderBookResponseAttach::Row::operator!=` always returned `true`**, breaking
  the equality contract of a `Q_GADGET` used inside `QList` from QML.
- **`Predicate::fromXdr` read the wrong union member** in the AND and OR cases
  (union aliasing: the same bytes today, latent UB).

### Added (exact offer prices)

A Stellar price **is** an `n/d` fraction in the XDR, but the three offer classes
only let you build and read it as a decimal string. That string goes through
`Price(QString)`, which truncates to 11 characters and re-derives the fraction
over 1e9, so any price without a short decimal representation was silently
corrupted: `7/9` came back as `777777777/1000000000`. The **response** classes
already exposed `getPriceR()`; the asymmetry was on the construction side only.

- **`getPriceR()`** on `ManageSellOfferOperation`, `ManageBuyOfferOperation` and
  `CreatePassiveSellOfferOperation`, returning the exact XDR fraction — the
  authoritative value. `getPrice()` still returns the decimal string.
- **Constructor and `create()` overloads taking `const Price&`**, alongside the
  `QString` variants, which are kept for compatibility and still approximate (now
  documented in the header rather than being a surprise). Same shape
  `LiquidityPoolDepositOperation` already used.
- **Txrep no longer loses the fraction in either direction.** Emitting used
  `Price(op->getPrice())`, i.e. it re-parsed its own decimal string; parsing
  converted the document's exact `n`/`d` to decimal only to approximate it again.
  A txrep with `price.n: 7 / price.d: 9` produced a transaction with a different
  price and **a different hash** than the document the user reviewed.
- `Price::toXdr()` is now `const` (it never mutated), and `getNumerator()`,
  `getDenominator()` and `toString()` are `Q_INVOKABLE` so prices can be
  formatted from QML without dropping into C++.

### Fixed (test harness)

- **A test class that threw was counted as green.** In `TestCollector`, the
  `catch` around `QTest::qExec` added the class to the failed list but never
  incremented the counter, so the summary printed "Passed: everything,
  Failed: 0" and `RunAllTests` returned 0: **CI would have gone green over a
  suite that had blown up**. This surfaced while verifying the fixes above — a
  throwing test also aborts its whole class and takes the other verdicts with it,
  which is how four failures stayed hidden on the first pass. An exception now
  counts as a failure of the whole class and the exit code reflects it.

### Security / robustness

- **The new CAP-71 delegate readers were two denial-of-service primitives.**
  The 4-byte array count was attacker-controlled up to 0x7FFFFFFF and went
  straight into `reserve()` (~800 GB for a ~400-byte struct), and `QDataStream`
  never throws on exhaustion — it flags `ReadPastEnd` and returns defaults — so
  a truncated stream appended empty delegates until the process died. Separately
  `SorobanDelegateSignature` recursed with no depth limit: ~110 KB of hostile
  XDR overflows a 1 MB stack, a crash no `catch` can intercept. Now bounded
  (`MAX_SOROBAN_DELEGATES`), depth-limited (`MAX_SOROBAN_DELEGATE_DEPTH`), and
  the readers check `QDataStream::status()` each iteration.
- **Delegate arrays are validated against CAP-71-01's ordering rule.** Every
  delegate array, at every nesting level, must be sorted by ascending address
  with no duplicates or Core rejects the whole invocation before the contract
  runs. The encoder wrote whatever order the caller appended. Both the encoder
  and the decoder now reject unsorted or duplicated arrays, so an invalid
  transaction fails locally with a clear message instead of on-chain.
- **`hashedSignaturePayload` could sign an expiration the entry does not
  carry.** Core rebuilds the preimage from the entry's own
  `signatureExpirationLedger`, so the documented external-signer flow — hash for
  ledger Y, stamp the returned signature in, ship an entry still saying 0 —
  produced a signature that fails auth on-chain with nothing local to catch it.
  The builder now requires the entry to already carry the expiration it is being
  hashed for. `authorizeEntry` stamps it internally, so that path is unaffected.

### Changed

- The XDR the SDK produces for a contract instance with storage is 4 bytes
  shorter (and now correct). Anything that stored or golden-tested
  SDK-generated contract-instance XDR will differ — it was never valid against
  Core.

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
