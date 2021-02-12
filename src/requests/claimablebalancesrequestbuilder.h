#ifndef CLAIMABLEBALANCESREQUESTBUILDER_H
#define CLAIMABLEBALANCESREQUESTBUILDER_H

#include "requestbuilder.h"
#include "../responses/claimablebalanceresponse.h"
#include "../responses/page.h"

/**
 * Builds requests connected to claimable balances.
 */
class ClaimableBalancesRequestBuilder : public RequestBuilder
{

public:
    ClaimableBalancesRequestBuilder(Server *server);
    /**
       * Requests specific <code>uri</code> and returns {@link ClaimableBalanceResponse}.
       * This method is helpful for getting the links.
       */
      ClaimableBalanceResponse* claimableBalance(QUrl uri);

      /**
       * The claimable balance details endpoint provides information on a claimable balance.
       * @param id specifies which claimable balance to load.
       * @return The claimable balance details.
       */
      ClaimableBalanceResponse* claimableBalance(QString id);

      /**
       * Returns all claimable balances sponsored by a given account.
       *
       * @param sponsor Account ID of the sponsor.
       * @return current {@link ClaimableBalancesRequestBuilder} instance
       */
      ClaimableBalancesRequestBuilder& forSponsor(QString sponsor);

      /**
       * Returns all claimable balances which hold a given asset.
       *
       * @param asset The Asset held by the claimable balance.
       * @return current {@link ClaimableBalancesRequestBuilder} instance
       */
      ClaimableBalancesRequestBuilder& forAsset(Asset* asset);

      /**
       * Returns all claimable balances which can be claimed by a given account id.
       *
       * @param claimant Account ID of the address which can claim the claimable balance.
       * @return current {@link ClaimableBalancesRequestBuilder} instance
       */
      ClaimableBalancesRequestBuilder& forClaimant(QString claimant);

      /**
       * Requests specific <code>uri</code> and returns {@link Page} of {@link ClaimableBalanceResponse}.
       * This method is helpful for getting the next set of results.
       * @return {@link Page} of {@link ClaimableBalanceResponse}
       */
      Page<ClaimableBalanceResponse>* execute(QUrl uri);

      /**
       * Build and execute request.
       * @return {@link Page} of {@link ClaimableBalanceResponse}
       */
      Page<ClaimableBalanceResponse>* execute();


      ClaimableBalancesRequestBuilder& cursor(QString token);


      ClaimableBalancesRequestBuilder& limit(int number);

      ClaimableBalancesRequestBuilder& order(Order direction);
};

#endif // CLAIMABLEBALANCESREQUESTBUILDER_H
