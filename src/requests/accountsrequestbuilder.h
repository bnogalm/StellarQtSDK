#ifndef ACCOUNTSREQUESTBUILDER_H
#define ACCOUNTSREQUESTBUILDER_H

#include "requestbuilder.h"
#include "../responses/accountresponse.h"
#include "../responses/accountdataresponse.h"
#include "../responses/page.h"
#include <QUrl>
#include "../qstellar_namespace.h"
QSTELLAR_BEGIN_NS



/**
 * Builds requests connected to accounts.
 */
class AccountsRequestBuilder : public RequestBuilder
{
     /**
      * Horizon's /accounts takes one of signer, sponsor, asset and
      * liquidity_pool: throws std::runtime_error when one other than
      * `filter` is set already. Only forLiquidityPool checked all three,
      * so forLiquidityPool(id).forSigner(...) went out and Horizon
      * answered 400. js and py check none.
      */
     void checkOneFilter(const QString& filter) const;
public:
     AccountsRequestBuilder(Server* server);

     /**
      * Requests specific <code>uri</code> and returns {@link AccountResponse}.
      * This method is helpful for getting the links.
      * @throws IOException
      */
     AccountResponse *account(QUrl uri);
     //AccountResponse *account(QString) = delete;//to protect someone passing an account without using KeyPair class.

     /**
      * Requests <code>GET /accounts/{account}</code>
      * @see <a href="https://www.stellar.org/developers/horizon/reference/accounts-single.html">Account Details</a>
      * @param account Account to fetch
      * @throws IOException
      */
private:
     AccountResponse *account(QString accountID);
public:
     AccountResponse *account(KeyPair* account);

     /**
      * Requests specific <code>uri</code> and returns {@link AccountDataResponse}:
      * a data entry's own URL, as account(QUrl) for an account.
      */
     AccountDataResponse *accountData(QUrl uri);

     /**
      * Requests <code>GET /accounts/{account}/data/{key}</code>: one data
      * entry of the account, its value decoded (see AccountDataResponse),
      * as py's Server.data(account_id, data_name) and js's data({key}) link
      * on an account record.
      * An entry the account does not have is Horizon's 404, the
      * BadRequestException every request gives. After stream(), the entry
      * is streamed, as py's DataCallBuilder.stream().
      *
      * `key` is sent as one path segment, percent-encoded: a name may hold
      * '/', '?', '#' or a '%XX' sequence, which py writes into the URL as
      * they are, asking for another path (or cutting it at the '?' or '#'),
      * or for another name ("a%20b" asks for "a b"); requests encodes py's
      * spaces and a lone '%'. A name with '/' still cannot be read here:
      * Horizon answers 404 for it however it is encoded (checked on
      * testnet; js gets the same 404). It is in the account's data map
      * (AccountResponse::getData()).
      *
      * Throws std::runtime_error, as the other filters here, for a key no
      * entry can have, by stellar-core's rule for data names (ManageData's
      * MANAGE_DATA_INVALID_NAME): empty, more than 64 bytes, or a byte that
      * is not printable ASCII (0x20 to 0x7E). ManageDataOperation checks the
      * length only; core refuses the others when it validates the
      * transaction. Also for the names "." and "..": core takes them, but
      * HTTP cannot address them, since they are dot-segments that Qt
      * normalizes out of the path (".." would ask for the account itself,
      * as py does), and percent-encoding does not help, %2E being read as
      * '.'. Also for a null account.
      */
     AccountDataResponse *accountData(KeyPair* account, const QString& key);

     /**
      * @brief signer  filters accounts who have a given signer account id
      * @see <a href="https://www.stellar.org/developers/horizon/reference/endpoints/accounts.html">Accounts</a>
      * @param token Account ID
      * @return AccountsRequestBuilder
      */
private:
     AccountsRequestBuilder& forSigner(QString accountID);
public:
     AccountsRequestBuilder& forSigner(KeyPair* accountID);


     /**
      * @brief asset  filters accounts who have a given trustline
      * @see <a href="https://www.stellar.org/developers/horizon/reference/endpoints/accounts.html">Accounts</a>
      * @param asset issued asset to filter with
      * @return AccountsRequestBuilder
      */

     /**
      * Returns all accounts who are sponsored by a given account or have subentries which are sponsored by a given account.
      *
      * @param sponsor Account ID
      * @return current {@link AccountsRequestBuilder} instance
      * @see <a href="https://www.stellar.org/developers/horizon/reference/endpoints/accounts.html">Accounts</a>
      */
private:
     AccountsRequestBuilder& forSponsor(QString sponsor);
public:
     AccountsRequestBuilder& forSponsor(KeyPair *sponsor);


     AccountsRequestBuilder& forAsset(Asset* asset);

     /**
      * Returns all accounts holding shares of the given liquidity pool.
      * @param liquidityPoolId 64-char hex liquidity pool id.
      */
     AccountsRequestBuilder& forLiquidityPool(const QString& liquidityPoolId);


     AccountsRequestBuilder& cursor(QString token);


     AccountsRequestBuilder& limit(int number);


     AccountsRequestBuilder& order(Order direction);
     /**
           * Allows to stream SSE events from horizon.
           * Certain endpoints in Horizon can be called in streaming mode using Server-Sent Events.
           * This mode will keep the connection to horizon open and horizon will continue to return
           * responses as ledgers close.
           */
     AccountsRequestBuilder &stream();

     /**
       * Requests specific <code>uri</code> and returns {@link Page} of {@link AccountResponse}.
       * This method is helpful for getting the next set of results.
       * @return {@link Page} of {@link AccountResponse}
       * @throws TooManyRequestsException when too many requests were sent to the Horizon server.
       * @throws IOException
       */
      Page<AccountResponse>* execute(QUrl uri);
     /**
        * Build and execute request. Each {@link AccountResponse} in the {@link Page} is a whole account, read exactly (see Response::fillObject): a record Horizon sent without its sequence, balances, thresholds, flags or signers makes the page emit error().
        * @return {@link Page} of {@link AccountResponse}
        * @throws TooManyRequestsException when too many requests were sent to the Horizon server.
        * @throws IOException
        */
      Page<AccountResponse>* execute();

};

QSTELLAR_END_NS

QSTELLAR_ALIAS(AccountsRequestBuilder)

#endif // ACCOUNTSREQUESTBUILDER_H
