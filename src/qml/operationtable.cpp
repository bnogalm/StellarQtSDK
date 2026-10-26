#include "operationtable.h"

#include <QHash>

QSTELLAR_BEGIN_NS

const QVector<OpSpec>& operationTable()
{
    // First generated from an operation survey by a script that is not part
    // of this repository; edit this table by hand now. The field NAMES here
    // are the keys makeOperation() reads out of the spec (and the only keys
    // refuseUnknownKeys() lets through), so a rename here must be made there
    // too: one made only here fails at run time, as an operation that
    // silently ignores an input the form kept showing.
    static const QVector<OpSpec> table = {
    { "payment", "Payment",
      "Send an amount of one asset to another account.",
      false, {
        { "destination", "address",
          "Destination account", true,
          "A G address, or a muxed M address. It must already exist and "
          "must already trust the asset.",
          "GA5ZSEJYB37JRC5AVCIA5MOP4RHTM335X2KGX3IHOJAPP5RE34K4KZVN" },
        { "asset", "asset",
          "Asset", true,
          "Type native for XLM, otherwise CODE:ISSUER. The code is 1-12 "
          "characters, the issuer a G address.",
          "USDC:GBBD47IF6LWK7P7MDEVSCWR7DPUWV3NY3DTQEVFL4NAT4AQH3ZLLFLA5" },
        { "amount", "amount",
          "Amount", true,
          "Decimal, up to 7 places, dot not comma. Largest value is "
          "922337203685.4775807.",
          "10.5" },
      } },

    { "createAccount", "Create account",
      "Fund a brand-new account with a starting XLM balance.",
      false, {
        { "destination", "address",
          "New account", true,
          "A G address only, no muxed M addresses. It must NOT already "
          "exist, or you get ALREADY_EXISTS.",
          "GCEZWKCA5VLDNRLN3RPRJMRZOX3Z6G5CHCGSNFHEYVXM3XOJMDS674JZ" },
        { "startingBalance", "amount",
          "Starting balance (XLM)", true,
          "Decimal, up to 7 places. Always XLM. Must be at least 1 XLM "
          "(the base reserve) or it fails.",
          "2.5" },
      } },

    { "accountMerge", "Account merge",
      "Delete an account and move its whole XLM balance to another "
          "account.",
      false, {
        { "destination", "address",
          "Merge into", true,
          "A G address, or a muxed M address. It must already exist; it "
          "receives the entire XLM balance.",
          "GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H" },
      } },

    { "pathPaymentStrictSend", "Path payment strict send",
      "Spend an exact amount, converting it to another asset on the "
          "way.",
      false, {
        { "sendAsset", "asset",
          "Asset you send", true,
          "Type native for XLM, otherwise CODE:ISSUER.",
          "USDC:GBBD47IF6LWK7P7MDEVSCWR7DPUWV3NY3DTQEVFL4NAT4AQH3ZLLFLA5" },
        { "sendAmount", "amount",
          "Amount you send", true,
          "Decimal, up to 7 places. Exactly this much leaves your "
          "account, whatever the conversion returns.",
          "10" },
        { "destination", "address",
          "Destination account", true,
          "A G address, or a muxed M address. It must already exist and "
          "must trust the asset it receives.",
          "GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7" },
        { "destAsset", "asset",
          "Asset they receive", true,
          "Type native for XLM, otherwise CODE:ISSUER.",
          "native" },
        { "destMin", "amount",
          "Least they may receive", true,
          "Decimal, up to 7 places. Your slippage floor: fails with "
          "UNDER_DESTMIN if the path delivers less.",
          "118.7500000" },
        { "path", "assetList",
          "Assets to convert through", false,
          "At most 5, in order. Do not repeat the sent or received asset. "
          "Empty means convert directly.",
          "[]" },
      } },

    { "pathPaymentStrictReceive", "Path payment strict receive",
      "Deliver an exact amount, paying for it with a different asset.",
      false, {
        { "sendAsset", "asset",
          "Asset you send", true,
          "Type native for XLM, otherwise CODE:ISSUER.",
          "native" },
        { "sendMax", "amount",
          "Most you will send", true,
          "Decimal, up to 7 places. Your slippage ceiling: fails with "
          "OVER_SENDMAX if the path costs more.",
          "125.0000000" },
        { "destination", "address",
          "Destination account", true,
          "A G address, or a muxed M address. It must already exist and "
          "must trust the asset it receives.",
          "GA5ZSEJYB37JRC5AVCIA5MOP4RHTM335X2KGX3IHOJAPP5RE34K4KZVN" },
        { "destAsset", "asset",
          "Asset they receive", true,
          "Type native for XLM, otherwise CODE:ISSUER.",
          "USDC:GBBD47IF6LWK7P7MDEVSCWR7DPUWV3NY3DTQEVFL4NAT4AQH3ZLLFLA5" },
        { "destAmount", "amount",
          "Amount they receive", true,
          "Decimal, up to 7 places. Exactly this much is delivered, and "
          "you pay whatever it costs.",
          "10" },
        { "path", "assetList",
          "Assets to convert through", false,
          "At most 5, in order. Do not repeat the sent or received asset. "
          "Empty means convert directly.",
          "[\"BTC:GDUKMGUGDZQK6YHYA5Z6AY2G4XDSZPSZ3SW5UN3ARVMO6QSRDWP5YLEX\"]" },
      } },

    { "changeTrust", "Change trust",
      "Opens, resizes, or removes a trustline so you can hold an "
          "asset.",
      false, {
        { "asset", "asset",
          "Asset", false,
          "Written as CODE:ISSUER; native has no trustline (stellar-core "
          "refuses it as malformed). Leave blank only if you are filling in "
          "the two pool assets instead.",
          "USD:GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG" },
        { "limit", "amount",
          "Limit", false,
          "Decimal, up to 7 places. Blank means the maximum. 0 removes "
          "the trustline, and only works at a zero balance.",
          "1000" },
        // Blank demos: a demo fills one line, the asset above. A pool share
        // line instead is asset left blank and these three filled in.
        { "poolShareAssetA", "asset",
          "Pool asset A", false,
          "Only for a liquidity pool share trustline. Must sort before "
          "asset B; \"native\" always sorts first.",
          "" },
        { "poolShareAssetB", "asset",
          "Pool asset B", false,
          "Required once asset A is filled in, and must sort after it.",
          "" },
        { "poolShareFee", "uint32",
          "Pool fee", false,
          "In basis points. 30 is the only value the network accepts.",
          "" },
      } },

    { "setTrustlineFlags", "Set trustline flags",
      "Authorizes, freezes, or changes clawback on a trustline for "
          "your asset.",
      false, {
        { "trustor", "address",
          "Trustline holder", true,
          "The account holding the trustline. A G address of 56 "
          "characters; M addresses are rejected.",
          "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG" },
        { "asset", "asset",
          "Asset", true,
          "CODE:ISSUER, never native. You must be the issuer for the "
          "network to accept this.",
          "USD:GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7" },
        { "clearFlags", "flags",
          "Clear flags", false,
          "Add the bits to turn off: 1 authorized, 2 maintain "
          "liabilities, 4 clawback enabled. 0 clears nothing.",
          "4" },
        { "setFlags", "flags",
          "Set flags", false,
          "Same bits, to turn on. A bit cannot appear in both boxes, and "
          "4 can only be cleared here, never set.",
          "1" },
      } },

    { "allowTrust", "Allow trust (deprecated)",
      "Authorizes or freezes a trustline for your asset; use Set "
          "trustline flags.",
      true, {
        { "trustor", "address",
          "Trustline holder", true,
          "The account holding the trustline. A G address of 56 "
          "characters; M addresses are rejected.",
          "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG" },
        { "assetCode", "text",
          "Asset code", true,
          "Just the code, such as USD, with no issuer. 1 to 12 "
          "characters; the issuer is this operation's source.",
          "USD" },
        { "authorize", "bool",
          "Authorize", true,
          "On authorizes the trustline, and overrides the box below. Both "
          "off freezes the trustline. Required (as in js): left out it "
          "froze the trustline.",
          "true" },
        { "authorizeToMaintainLiabilities", "bool",
          "Authorize to maintain liabilities", false,
          "Only takes effect when Authorize is off. Lets existing offers "
          "stand while blocking new activity.",
          "false" },
      } },

    { "manageSellOffer", "Manage sell offer",
      "Create, update, or delete an offer to sell one asset for "
          "another.",
      false, {
        { "selling", "asset",
          "Selling", true,
          "The asset you give up. Must be different from the asset you "
          "are buying.",
          "USDC:GA2HGBJIJKI6O4XEM7CZWY5PS6GKSXL6D34ERAJYQSPYA6X6AI7HYW36" },
        { "buying", "asset",
          "Buying", true,
          "The asset you receive. If it is the same as the selling asset, "
          "the offer is rejected as malformed.",
          "native" },
        { "amount", "amount",
          "Amount to sell", true,
          "Decimal, up to 7 places, in the selling asset. Enter 0 "
          "together with an offer ID to delete that offer.",
          "250.5" },
        { "price", "price",
          "Price", true,
          "How much of the buying asset per 1 unit of selling. Decimal, "
          "above zero. Exact when it is a fraction of two 31-bit whole numbers; "
          "otherwise the same approximation the Java and JS SDKs sign. For an "
          "exact fraction, give priceR: { n, d }.",
          "0.115" },
        { "offerId", "int64",
          "Offer ID", false,
          "Leave empty to create a new offer. Enter an existing ID to "
          "update it, or with amount 0 to delete it.",
          "0" },
      } },

    { "manageBuyOffer", "Manage buy offer",
      "Create, update, or delete an offer to buy a set amount of an "
          "asset.",
      false, {
        { "selling", "asset",
          "Paying with", true,
          "The asset you give up. Must be different from the asset you "
          "are buying.",
          "native" },
        { "buying", "asset",
          "Buying", true,
          "The asset you want to acquire. The amount below is counted in "
          "this asset.",
          "USDC:GA2HGBJIJKI6O4XEM7CZWY5PS6GKSXL6D34ERAJYQSPYA6X6AI7HYW36" },
        { "amount", "amount",
          "Amount to buy", true,
          "Decimal, up to 7 places, in the buying asset, not the selling "
          "one. 0 plus an offer ID deletes that offer.",
          "100" },
        { "price", "price",
          "Price", true,
          "How much of the selling asset per 1 unit of buying, the "
          "reverse of a sell offer. Decimal, must be positive.",
          "8.75" },
        { "offerId", "int64",
          "Offer ID", false,
          "Leave empty to create a new offer. Enter an existing ID to "
          "update it, or with amount 0 to delete it.",
          "0" },
      } },

    { "createPassiveSellOffer", "Create passive sell offer",
      "Place a sell offer that never takes an existing offer at the "
          "same price.",
      false, {
        { "selling", "asset",
          "Selling", true,
          "The asset you give up. Must be different from the asset you "
          "are buying.",
          "EURT:GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD" },
        { "buying", "asset",
          "Buying", true,
          "The asset you receive. If it is the same as the selling asset, "
          "the offer is rejected.",
          "USDC:GA2HGBJIJKI6O4XEM7CZWY5PS6GKSXL6D34ERAJYQSPYA6X6AI7HYW36" },
        { "amount", "amount",
          "Amount to sell", true,
          "Decimal, up to 7 places, in the selling asset.",
          "1000" },
        { "price", "price",
          "Price", true,
          "How much of the buying asset per 1 unit of selling. Decimal, "
          "above zero. Exact when it is a fraction of two 31-bit whole numbers; "
          "otherwise the same approximation the Java and JS SDKs sign. For an "
          "exact fraction, give priceR: { n, d }.",
          "1.08" },
      } },

    { "liquidityPoolDeposit", "Liquidity pool deposit",
      "Add both of a pool's assets and receive pool shares in return.",
      false, {
        { "liquidityPoolId", "poolId",
          "Pool ID", true,
          "Hex, exactly 64 characters. Not an asset code, and not a G... "
          "or L... address.",
          "dd7b1ab831c273310ddbec6f97870aa83c2fbd78ce22aded37ecbf4f3380fac7" },
        { "maxAmountA", "amount",
          "Most asset A to put in", true,
          "Decimal, up to 7 places. A is the pool's first asset, which "
          "may not be the order you think of them in.",
          "1000" },
        { "maxAmountB", "amount",
          "Most asset B to put in", true,
          "Decimal, up to 7 places. B is the pool's second asset.",
          "115.5" },
        { "minPrice", "price",
          "Lowest price accepted", true,
          "Lowest A-per-B rate you will deposit at. Required, and must be "
          "above zero.",
          "0.11" },
        { "maxPrice", "price",
          "Highest price accepted", true,
          "Highest A-per-B rate you will deposit at. Required, and must "
          "be at least the lowest price, or it fails.",
          "0.12" },
      } },

    { "liquidityPoolWithdraw", "Liquidity pool withdraw",
      "Burn pool shares to take back your share of both assets in the "
          "pool.",
      false, {
        { "liquidityPoolId", "poolId",
          "Pool ID", true,
          "Hex, exactly 64 characters. Not an asset code, and not a G... "
          "or L... address.",
          "dd7b1ab831c273310ddbec6f97870aa83c2fbd78ce22aded37ecbf4f3380fac7" },
        { "amount", "amount",
          "Pool shares to burn", true,
          "The number of pool shares to redeem, not an amount of either "
          "asset. Decimal, up to 7 places.",
          "50" },
        { "minAmountA", "amount",
          "Least asset A to accept", true,
          "Decimal, up to 7 places. The withdrawal fails if the pool "
          "would return less than this. 0 means no floor.",
          "480" },
        { "minAmountB", "amount",
          "Least asset B to accept", true,
          "Decimal, up to 7 places. The withdrawal fails if the pool "
          "would return less than this. 0 means no floor.",
          "55.25" },
      } },

    { "createClaimableBalance", "Create claimable balance",
      "Sets aside an amount that named accounts can claim once "
          "conditions are met.",
      false, {
        { "asset", "asset",
          "Asset", true,
          "\"native\" for XLM, or CODE:ISSUER. Pool shares cannot be "
          "locked up this way.",
          "native" },
        { "amount", "amount",
          "Amount", true,
          "Decimal with up to 7 places, greater than 0. Use a dot, not a "
          "comma; an exponent is read as a number, so \"1e7\" is ten "
          "million.",
          "10.5" },
        { "claimants", "claimants",
          "Claimants", true,
          "1 to 10 entries. Each needs a G... address (56 characters) and "
          "a condition; and/or take exactly two.",
          "[{\"destination\":\"GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO\",\"predicate\":{\"type\":\"and\",\"predicates\":[{\"type\":\"absBefore\",\"value\":\"1798761600\"},{\"type\":\"not\",\"predicate\":{\"type\":\"relBefore\",\"value\":\"86400\"}}]}},{\"destination\":\"GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7\",\"predicate\":{\"type\":\"unconditional\"}}]" },
      } },

    { "claimClaimableBalance", "Claim claimable balance",
      "Claims a balance someone locked up for you, paying it into "
          "your account.",
      false, {
        { "balanceId", "balanceId",
          "Balance ID", true,
          "Hex, exactly 72 characters, starting 00000000. A wrong one "
          "fails on submission with op_does_not_exist.",
          "00000000fedcba9876543210fedcba9876543210fedcba9876543210fedcba9876543210" },
      } },

    { "clawbackClaimableBalance", "Clawback claimable balance",
      "Pulls back a claimable balance of your asset before anyone "
          "claims it.",
      false, {
        { "balanceId", "balanceId",
          "Balance ID", true,
          "Hex, exactly 72 characters, as Horizon shows it. Not the B... "
          "address, and no 0x prefix.",
          "0000000095001252ab3b4d16adbfa5364ce526dfcda03cb2258b827edbb2e0450087be51" },
      } },

    { "clawback", "Clawback",
      "Pulls an amount of your asset back from an account that holds "
          "it.",
      false, {
        { "from", "address",
          "From account", true,
          "The account you are pulling the asset back from. G and M "
          "addresses both work.",
          "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG" },
        { "asset", "asset",
          "Asset", true,
          "CODE:ISSUER, never native. You must be the issuer, and the "
          "trustline must have clawback enabled.",
          "USD:GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7" },
        { "amount", "amount",
          "Amount", true,
          "Decimal, up to 7 places. Use a dot, not a comma.",
          "10.5" },
      } },

    { "setOptions", "Set options",
      "Changes your account's signers, thresholds, flags, and home "
          "domain.",
      false, {
        { "inflationDest", "address",
          "Inflation destination", false,
          "A G address of 56 characters. Leave blank to leave it "
          "unchanged.",
          "" },
        { "clearFlags", "flags",
          "Clear account flags", false,
          "Bits to turn off: 1 auth required, 2 auth revocable, 4 "
          "immutable, 8 clawback. 4 can never be cleared.",
          "" },
        { "setFlags", "flags",
          "Set account flags", false,
          "Same bits, to turn on. Turning on 8 also needs 2 set, or the "
          "network rejects the operation.",
          "" },
        { "masterWeight", "uint32",
          "Master key weight", false,
          "0 to 255. Blank leaves it alone. 0 retires your master key, so "
          "another signer must cover you.",
          "" },
        { "lowThreshold", "uint32",
          "Low threshold", false,
          "0 to 255. Blank leaves it unchanged. Signature weight needed "
          "for low-risk operations.",
          "" },
        { "medThreshold", "uint32",
          "Medium threshold", false,
          "0 to 255. Blank leaves it unchanged. Covers payments and most "
          "everyday operations.",
          "" },
        { "highThreshold", "uint32",
          "High threshold", false,
          "0 to 255. Blank leaves it unchanged. Covers signer and "
          "threshold changes; keep it reachable.",
          "" },
        { "homeDomain", "text",
          "Home domain", false,
          "Up to 32 characters. Sending it empty clears the domain rather "
          "than leaving it as it is.",
          "example.com" },
        { "signerKey", "signerKey",
          "Signer key", false,
          "An address starting with G, T, X, or P. Only takes effect "
          "together with a weight below.",
          "" },
        { "signerWeight", "uint32",
          "Signer weight", false,
          "0 to 255, and required with a signer key. 0 removes that "
          "signer instead of adding it.",
          "" },
      } },

    { "manageData", "Manage data",
      "Store, change, or remove a named value on an account.",
      false, {
        { "name", "text",
          "Entry name", true,
          "At most 64 bytes: accented and CJK characters count as several "
          "bytes each. Longer names are rejected.",
          "config.currency" },
        { "value", "dataValue",
          "Entry value", false,
          "Leave it out to delete the entry. At most 64 bytes once "
          "decoded: text is signed as its UTF-8 bytes, hex or base64 as "
          "the bytes they spell (see Value format).",
          "EUR" },
        { "valueFormat", "text",
          "Value format", false,
          "text (the default), hex, or base64: how Entry value is "
          "written. hex takes an even number of digits 0-9 and a-f "
          "(either case), base64 the standard alphabet with its = "
          "padding; anything else, spaces included, is refused rather "
          "than guessed at.",
          "text" },
      } },

    { "bumpSequence", "Bump sequence",
      "Raise an account's sequence number to a higher value.",
      false, {
        { "bumpTo", "int64",
          "New sequence number", true,
          "A whole number, no decimals and not negative. A value below "
          "the current sequence does nothing.",
          "1234567890123" },
      } },

    { "beginSponsoringFutureReserves", "Begin sponsoring future reserves",
      "Starts paying another account's reserves until an End "
          "operation closes it.",
      false, {
        { "sponsoredId", "address",
          "Sponsored account", true,
          "A G... address, 56 characters; muxed M... addresses are "
          "rejected. It does not have to exist yet.",
          "GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7" },
      } },

    { "endSponsoringFutureReserves", "End sponsoring future reserves",
      "Closes the sponsorship opened earlier in this transaction.",
      false, {
      } },

    { "revokeAccountSponsorship", "Revoke account sponsorship",
      "Stops sponsoring an account's base reserve, handing the cost "
          "back to it.",
      false, {
        { "accountId", "address",
          "Sponsored account", true,
          "The account being sponsored, not the sponsor. Must be a G... "
          "address, 56 characters; M... is rejected.",
          "GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H" },
      } },

    { "revokeTrustlineSponsorship", "Revoke trustline sponsorship",
      "Stops sponsoring a trustline, handing its reserve back to the "
          "account holding it.",
      false, {
        { "accountId", "address",
          "Trustline owner", true,
          "The account that holds the trustline. Must be a G... address, "
          "56 characters; M... is rejected.",
          "GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H" },
        { "asset", "asset",
          "Asset", false,
          "Written as CODE:ISSUER. Must be a trustline the account "
          "actually holds; native has no trustline. For a pool-share "
          "trustline give liquidityPoolId instead (exactly one of the two).",
          "USDC:GBBD47IF6LWK7P7MDEVSCWR7DPUWV3NY3DTQEVFL4NAT4AQH3ZLLFLA5" },
        { "liquidityPoolId", "poolId",
          "Pool ID (pool-share trustline)", false,
          "Hex, 64 characters: the pool whose shares the trustline holds. "
          "Instead of asset, for a pool-share trustline.",
          "" },
      } },

    { "revokeOfferSponsorship", "Revoke offer sponsorship",
      "Stops sponsoring an open offer, handing its reserve back to "
          "the seller.",
      false, {
        { "seller", "address",
          "Offer seller", true,
          "The account that placed the offer. Must be a G... address, 56 "
          "characters; M... is rejected.",
          "GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H" },
        { "offerId", "int64",
          "Offer ID", true,
          "Whole number, as shown on the offer. 0 is rejected with "
          "\"offerId cannot be null\"; real IDs start at 1.",
          "123456789" },
      } },

    { "revokeDataSponsorship", "Revoke data sponsorship",
      "Stops sponsoring a data entry, handing its reserve back to the "
          "account.",
      false, {
        { "accountId", "address",
          "Data entry owner", true,
          "The account that owns the entry. Must be a G... address, 56 "
          "characters; M... is rejected.",
          "GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H" },
        { "dataName", "text",
          "Entry name", true,
          "The entry's name, not its value. Plain text, up to 64 bytes; "
          "accented characters count as more than one.",
          "config.homedomain" },
      } },

    { "revokeClaimableBalanceSponsorship", "Revoke claimable balance sponsorship",
      "Stops sponsoring a claimable balance, returning its reserve to "
          "whoever created it.",
      false, {
        { "balanceId", "balanceId",
          "Balance ID", true,
          "Hex, 72 characters, exactly as Horizon shows it. A bare "
          "64-character hash or a B... address is rejected.",
          "00000000929b20b72e5890ab51c24f1cc46fa01c4f318d8d33367d24dd614cfdf5491072" },
      } },

    { "revokeLiquidityPoolSponsorship", "Revoke liquidity pool sponsorship",
      "Stops sponsoring a liquidity pool stake, handing its reserve "
          "back to the owner.",
      false, {
        { "liquidityPoolId", "poolId",
          "Pool ID", true,
          "Hex, 64 characters (not an L... address, as in js). No type "
          "prefix here, unlike a claimable balance ID.",
          "dd7b1ab831c273310ddbec6f97870aa83c2fbd78ce22aded37ecbf4f3380fac7" },
      } },

    { "revokeSignerSponsorship", "Revoke signer sponsorship",
      "Stops sponsoring a signer on an account, handing its reserve "
          "back to that account.",
      false, {
        { "accountId", "address",
          "Account with the signer", true,
          "The account the signer is on, not the signer itself. G... "
          "address, 56 characters; M... is rejected.",
          "GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H" },
        { "signerKey", "signerKey",
          "Signer key", true,
          "Must start with G (account), T (pre-auth), X (hash) or P "
          "(signed payload). Anything else is rejected.",
          "GDQNY3PBOJOKYZSRMK2S7LHHGWZIUISD4QORETLMXEWXBI7KFZZMKTL3" },
      } },

    { "invokeContract", "Invoke contract",
      "Call a function on a deployed contract.",
      false, {
        { "contractId", "address",
          "Contract", true,
          "The C... address of a deployed contract. Not a G... account.",
          "CDLZFC3SYJYDZT7K67VZ75HPJVIEUVNIXF47ZG2FB2RMQQVU2HHGCYSC" },
        { "functionName", "text",
          "Function", true,
          "The exact name the contract exports. At most 32 bytes, and it "
          "is case-sensitive.",
          "hello" },
        { "args", "scvalList",
          "Arguments", false,
          "A JSON array of typed values, e.g. "
          "[{\"type\":\"symbol\",\"value\":\"world\"}]. The type must "
          "match what the contract declares or the call fails inside the "
          "host.",
          "[{\"type\":\"symbol\",\"value\":\"world\"}]" },
      } },

    { "uploadContractWasm", "Upload contract WASM",
      "Store compiled contract code on the ledger. This does not "
          "deploy it.",
      false, {
        { "wasmHex", "text",
          "WASM bytes (hex)", true,
          "The compiled .wasm file as hex. Uploading only stores the code "
          "and gives you its hash; creating an instance from it is a "
          "separate step.",
          "0061736d01000000" },
      } },

    { "createContract", "Create contract",
      "Deploy an instance of code that was already uploaded.",
      false, {
        { "fromAddress", "address",
          "Deploying address", true,
          "The account or contract the new contract id is derived from.",
          "GA6U5X6WOPNKKDKQULBR7IDHDBAQKOWPHYEC7WSXHZBFEYFD3XVZAKOO" },
        { "wasmHash", "hex32",
          "WASM hash", true,
          "64 hex characters: the hash returned when the code was "
          "uploaded.",
          "a1b2c3d4e5f60718293a4b5c6d7e8f90a1b2c3d4e5f60718293a4b5c6d7e8f90" },
        { "salt", "hex32",
          "Salt", true,
          "64 hex characters. Address plus salt decides the contract id, "
          "so a different salt deploys a second independent instance of "
          "the same code.",
          "0000000000000000000000000000000000000000000000000000000000000001" },
      } },

    { "createContractFromAsset", "Wrap an asset as a contract",
      "Give a classic asset a contract interface, so contracts can "
          "hold it.",
      false, {
        { "asset", "asset",
          "Asset", true,
          "\"native\" or CODE:ISSUER. Deploys the Stellar Asset Contract "
          "for it, which is how an ordinary asset becomes usable from "
          "Soroban.",
          "native" },
      } },

    { "extendFootprintTtl", "Extend footprint TTL",
      "Keeps contract data alive longer by pushing its expiry further "
          "out.",
      false, {
        { "extendTo", "uint32",
          "Extend to (ledgers)", true,
          "A number of ledgers ahead of the current one, 0 to 4294967295. "
          "Not a date, not an amount.",
          "535679" },
      } },

    { "restoreFootprint", "Restore footprint",
      "Brings archived contract data back so it can be used again.",
      false, {
      } },

    { "inflation", "Inflation (deprecated)",
      "Legacy network payout, switched off since protocol 12; it "
          "always fails.",
      true, {
      } },
    };
    return table;
}

const OpSpec* operationSpec(const QString& type)
{
    static const QHash<QString, const OpSpec*> index = []() {
        QHash<QString, const OpSpec*> map;
        for (const OpSpec& spec : operationTable())
            map.insert(QString::fromLatin1(spec.type), &spec);
        return map;
    }();
    return index.value(type, nullptr);
}

QSTELLAR_END_NS
