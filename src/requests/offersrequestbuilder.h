#ifndef OFFERSREQUESTBUILDER_H
#define OFFERSREQUESTBUILDER_H
#include "requestbuilder.h"
#include "../responses/page.h"
#include "../responses/offerresponse.h"

class KeyPair;
/**
 * Builds requests connected to offers.
 */
class OffersRequestBuilder : public RequestBuilder
{
public:
    OffersRequestBuilder(Server* server);
     /**
      * Builds request to <code>GET /accounts/{account}/offers</code>
      * @see <a href="https://www.stellar.org/developers/horizon/reference/offers-for-account.html">Offers for Account</a>
      * @param account Account for which to get offers
      */
     OffersRequestBuilder& forAccount(KeyPair* account);

     /**
      * Requests specific <code>uri</code> and returns {@link Page} of {@link OfferResponse}.
      * This method is helpful for getting the next set of results.
      * @return {@link Page} of {@link OfferResponse}
      * @throws TooManyRequestsException when too many requests were sent to the Horizon server.
      * @throws IOException
      */
     Page<OfferResponse>* execute(QUrl uri);

     /**
      * Build and execute request.
      * @return {@link Page} of {@link OfferResponse}
      * @throws TooManyRequestsException when too many requests were sent to the Horizon server.
      * @throws IOException
      */
     Page<OfferResponse>* execute();

     OffersRequestBuilder& cursor(QString token);

     OffersRequestBuilder& limit(int number);
     OffersRequestBuilder& order(RequestBuilder::Order direction);
};

#endif // OFFERSREQUESTBUILDER_H
