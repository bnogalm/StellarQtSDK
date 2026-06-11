#ifndef OFFERSREQUESTBUILDER_H
#define OFFERSREQUESTBUILDER_H
#include "requestbuilder.h"
#include "../responses/page.h"
#include "../responses/offerresponse.h"
#include "../keypair.h"
#include "../qstellar_namespace.h"
QSTELLAR_BEGIN_NS

/**
 * Builds requests connected to offers.
 */
class OffersRequestBuilder : public RequestBuilder
{
public:
    OffersRequestBuilder(Server* server);

     /**
      * Requests <code>GET /offers/{offer_id}</code> — a single offer.
      * @see <a href="https://developers.stellar.org/api/horizon/resources/get-offer-by-offer-id/">Offer Details</a>
      * @param offerId the offer id
      */
     OfferResponse* offer(qint64 offerId);
     /** Requests a specific offer <code>uri</code> and returns the {@link OfferResponse}. */
     OfferResponse* offer(QUrl uri);

     /**
      * Builds request to <code>GET /accounts/{account}/offers</code>
      * @see <a href="https://www.stellar.org/developers/horizon/reference/offers-for-account.html">Offers for Account</a>
      * @param account Account for which to get offers
      */
     OffersRequestBuilder& forAccount(KeyPair* account);

     /**
      * Returns all offers sponsored by a given account.
      *
      * @param sponsor Account ID of the sponsor.
      * @return current {@link OffersRequestBuilder} instance
      * @see <a href="https://www.stellar.org/developers/horizon/reference/endpoints/offers.html">Offers</a>
      */
private:
     OffersRequestBuilder& forSponsor(QString sponsor);
public:
     OffersRequestBuilder& forSponsor(KeyPair* sponsor);

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

     OffersRequestBuilder& seller(KeyPair *accountId);
     /** Java-aligned alias for `seller`. */
     OffersRequestBuilder& forSeller(const QString& accountId);
     OffersRequestBuilder& selling(Asset* asset);
     OffersRequestBuilder& buying(Asset* asset);
     /**
      * Allows to stream SSE events from horizon.
      * Certain endpoints in Horizon can be called in streaming mode using Server-Sent Events.
      * This mode will keep the connection to horizon open and horizon will continue to return
      * responses as ledgers close.
      */
     OffersRequestBuilder& stream();
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(OffersRequestBuilder)

#endif // OFFERSREQUESTBUILDER_H
