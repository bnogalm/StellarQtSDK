#ifndef ACCOUNTSREQUESTBUILDER_H
#define ACCOUNTSREQUESTBUILDER_H

#include "requestbuilder.h"
#include "../responses/accountresponse.h"
#include "../responses/page.h"
#include <QUrl>


/**
 * Builds requests connected to accounts.
 */
class AccountsRequestBuilder : public RequestBuilder
{
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
        * Build and execute request. <strong>Warning!</strong> {@link AccountResponse}s in {@link Page} will contain only <code>keypair</code> field.
        * @return {@link Page} of {@link AccountResponse}
        * @throws TooManyRequestsException when too many requests were sent to the Horizon server.
        * @throws IOException
        */
      Page<AccountResponse>* execute();

};

#endif // ACCOUNTSREQUESTBUILDER_H
