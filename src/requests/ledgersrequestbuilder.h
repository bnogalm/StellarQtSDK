#ifndef LEDGERSREQUESTBUILDER_H
#define LEDGERSREQUESTBUILDER_H
#include "requestbuilder.h"
#include "../responses/ledgerresponse.h"
#include "../responses/page.h"
/**
 * Builds requests connected to ledgers.
 */
class LedgersRequestBuilder : public RequestBuilder
{
public:
    LedgersRequestBuilder(Server* server);
     /**
      * Requests specific <code>uri</code> and returns {@link LedgerResponse}.
      * This method is helpful for getting the links.
      * @throws IOException
      */
     LedgerResponse *ledger(QUrl uri);

     /**
      * Requests <code>GET /ledgers/{ledgerSeq}</code>
      * @see <a href="https://www.stellar.org/developers/horizon/reference/ledgers-single.html">Ledger Details</a>
      * @param ledgerSeq Ledger to fetch
      * @throws IOException
      */
     LedgerResponse* ledger(long ledgerSeq);

     /**
      * Requests specific <code>uri</code> and returns {@link Page} of {@link LedgerResponse}.
      * This method is helpful for getting the next set of results.
      * @return {@link Page} of {@link LedgerResponse}
      * @throws TooManyRequestsException when too many requests were sent to the Horizon server.
      * @throws IOException
      */
     Page<LedgerResponse> *execute(QUrl uri);

     /**
      * Allows to stream SSE events from horizon.
      * Certain endpoints in Horizon can be called in streaming mode using Server-Sent Events.
      * This mode will keep the connection to horizon open and horizon will continue to return
      * responses as ledgers close.
      * @see <a href="http://www.w3.org/TR/eventsource/" target="_blank">Server-Sent Events</a>
      * @see <a href="https://www.stellar.org/developers/horizon/learn/responses.html" target="_blank">Response Format documentation</a>
      */
     LedgersRequestBuilder& stream();

     /**
      * Build and execute request.
      * @return {@link Page} of {@link LedgerResponse}
      * @throws TooManyRequestsException when too many requests were sent to the Horizon server.
      * @throws IOException
      */
     Page<LedgerResponse>* execute();

     LedgersRequestBuilder& cursor(QString token);

     LedgersRequestBuilder& limit(int number);

     LedgersRequestBuilder& order(Order direction);
};

#endif // LEDGERSREQUESTBUILDER_H
