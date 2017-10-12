#ifndef PAYMENTSREQUESTBUILDER_H
#define PAYMENTSREQUESTBUILDER_H
#include "requestbuilder.h"
#include "../responses/page.h"
#include "../responses/operations/operationresponse.h"

class Server;
class KeyPair;
class OperationPage;
/**
 * Builds requests connected to payments.
 */
class PaymentsRequestBuilder : public RequestBuilder
{
public:
    PaymentsRequestBuilder(Server* server);


    /**
     * Builds request to <code>GET /accounts/{account}/payments</code>
     * @see <a href="https://www.stellar.org/developers/horizon/reference/payments-for-account.html">Payments for Account</a>
     * @param account Account for which to get payments
     */
    PaymentsRequestBuilder& forAccount(KeyPair* account);

    /**
     * Builds request to <code>GET /ledgers/{ledgerSeq}/payments</code>
     * @see <a href="https://www.stellar.org/developers/horizon/reference/payments-for-ledger.html">Payments for Ledger</a>
     * @param ledgerSeq Ledger for which to get payments
     */
    PaymentsRequestBuilder &forLedger(qint64 ledgerSeq);

    /**
     * Builds request to <code>GET /transactions/{transactionId}/payments</code>
     * @see <a href="https://www.stellar.org/developers/horizon/reference/payments-for-transaction.html">Payments for Transaction</a>
     * @param transactionId Transaction ID for which to get payments
     */
    PaymentsRequestBuilder &forTransaction(QString transactionId);

    /**
     * Requests specific <code>uri</code> and returns {@link Page} of {@link OperationResponse}.
     * This method is helpful for getting the next set of results.
     * @return {@link Page} of {@link OperationResponse}
     * @throws TooManyRequestsException when too many requests were sent to the Horizon server.
     * @throws IOException
     */
    OperationPage* execute(QUrl uri);

    /**
     * Allows to stream SSE events from horizon.
     * Certain endpoints in Horizon can be called in streaming mode using Server-Sent Events.
     * This mode will keep the connection to horizon open and horizon will continue to return
     * responses as ledgers close.
     * @see <a href="http://www.w3.org/TR/eventsource/" target="_blank">Server-Sent Events</a>
     * @see <a href="https://www.stellar.org/developers/horizon/learn/responses.html" target="_blank">Response Format documentation</a>
     * @param listener {@link EventListener} implementation with {@link OperationResponse} type
     * @return EventSource object, so you can <code>close()</code> connection when not needed anymore
     */

    PaymentsRequestBuilder& stream();

    /**
     * Build and execute request.
     * @return {@link Page} of {@link OperationResponse}
     * @throws TooManyRequestsException when too many requests were sent to the Horizon server.
     * @throws IOException
     */
    OperationPage *execute();
    PaymentsRequestBuilder& cursor(QString token) {
      RequestBuilder::cursor(token);
      return *this;
    }


    PaymentsRequestBuilder& limit(int number) {
      RequestBuilder::limit(number);
      return *this;
    }


    PaymentsRequestBuilder& order(Order direction) {
      RequestBuilder::order(direction);
      return *this;
  }
};

#endif // PAYMENTSREQUESTBUILDER_H
