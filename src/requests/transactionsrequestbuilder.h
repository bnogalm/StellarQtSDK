#ifndef TRANSACTIONSREQUESTBUILDER_H
#define TRANSACTIONSREQUESTBUILDER_H
#include "requestbuilder.h"
#include "../responses/page.h"
#include "../responses/transactionresponse.h"
class KeyPair;

class TransactionsRequestBuilder : public RequestBuilder
{
public:
    /**
     * Builds requests connected to transactions.
     */
    TransactionsRequestBuilder(Server* server);

    /**
     * Adds a parameter defining whether to include failed transactions. By default only successful transactions are
     * returned.
     * @param value Set to <code>true</code> to include failed transactions.
     */
    TransactionsRequestBuilder& includeFailed(bool value);

    /**
     * Requests specific <code>uri</code> and returns {@link TransactionResponse}.
     * This method is helpful for getting the links.
     * @throws IOException
     */
    TransactionResponse* transaction(QUrl uri);

    /**
     * Requests <code>GET /transactions/{transactionId}</code>
     * @see <a href="https://www.stellar.org/developers/horizon/reference/transactions-single.html">Transaction Details</a>
     * @param transactionId Transaction to fetch
     * @throws IOException
     */
    TransactionResponse* transaction(QString transactionId);

    /**
     * Builds request to <code>GET /accounts/{account}/transactions</code>
     * @see <a href="https://www.stellar.org/developers/horizon/reference/transactions-for-account.html">Transactions for Account</a>
     * @param account Account for which to get transactions
     */
    TransactionsRequestBuilder& forAccount(KeyPair* account);

    /**
     * Builds request to <code>GET /ledgers/{ledgerSeq}/transactions</code>
     * @see <a href="https://www.stellar.org/developers/horizon/reference/transactions-for-ledger.html">Transactions for Ledger</a>
     * @param ledgerSeq Ledger for which to get transactions
     */
    TransactionsRequestBuilder& forLedger(qint64 ledgerSeq);

    /**
     * Requests specific <code>uri</code> and returns {@link Page} of {@link TransactionResponse}.
     * This method is helpful for getting the next set of results.
     * @return {@link Page} of {@link TransactionResponse}
     * @throws TooManyRequestsException when too many requests were sent to the Horizon server.
     * @throws IOException
     */

    /**
     * Builds request to <code>GET /claimable_balances/{claimable_balance_id}/transactions</code>
     * @see <a href="https://www.stellar.org/developers/horizon/reference/transactions-for-claimable-balance.html">Transactions for ClaimableBalance</a>
     * @param claimableBalance Claimable Balance for which to get transactions
     */
    TransactionsRequestBuilder& forClaimableBalance(QString claimableBalance);


    Page<TransactionResponse> *execute(QUrl uri);

    //    /**
    //     * Allows to stream SSE events from horizon.
    //     * Certain endpoints in Horizon can be called in streaming mode using Server-Sent Events.
//     * This mode will keep the connection to horizon open and horizon will continue to return
//     * responses as ledgers close.
//     * @see <a href="http://www.w3.org/TR/eventsource/" target="_blank">Server-Sent Events</a>
//     * @see <a href="https://www.stellar.org/developers/horizon/learn/responses.html" target="_blank">Response Format documentation</a>
//     * @param listener {@link EventListener} implementation with {@link TransactionResponse} type
//     * @return EventSource object, so you can <code>close()</code> connection when not needed anymore
//     */
    TransactionsRequestBuilder& stream();

    /**
     * Build and execute request.
     * @return {@link Page} of {@link TransactionResponse}
     * @throws TooManyRequestsException when too many requests were sent to the Horizon server.
     * @throws IOException
     */
    Page<TransactionResponse> *execute();


    TransactionsRequestBuilder& cursor(QString token);


    TransactionsRequestBuilder& limit(int number);


    TransactionsRequestBuilder& order(Order direction);
};

#endif // TRANSACTIONSREQUESTBUILDER_H



