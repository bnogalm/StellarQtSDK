#ifndef EFFECTSREQUESTBUILDER_H
#define EFFECTSREQUESTBUILDER_H
#include "requestbuilder.h"
#include "../responses/effectspage.h"

class KeyPair;
/**
 * Builds requests connected to effects.
 */
class EffectsRequestBuilder : public RequestBuilder
{
public:
    EffectsRequestBuilder(Server* server);


    /**
     * Builds request to <code>GET /accounts/{account}/effects</code>
     * @see <a href="https://www.stellar.org/developers/horizon/reference/effects-for-account.html">Effects for Account</a>
     * @param account Account for which to get effects
     */
    EffectsRequestBuilder& forAccount(KeyPair* account);

    /**
     * Builds request to <code>GET /ledgers/{ledgerSeq}/effects</code>
     * @see <a href="https://www.stellar.org/developers/horizon/reference/effects-for-ledger.html">Effects for Ledger</a>
     * @param ledgerSeq Ledger for which to get effects
     */
    EffectsRequestBuilder& forLedger(qint64 ledgerSeq);

    /**
     * Builds request to <code>GET /transactions/{transactionId}/effects</code>
     * @see <a href="https://www.stellar.org/developers/horizon/reference/effects-for-transaction.html">Effect for Transaction</a>
     * @param transactionId Transaction ID for which to get effects
     */
    EffectsRequestBuilder& forTransaction(QString transactionId);

    /**
     * Builds request to <code>GET /operation/{operationId}/effects</code>
     * @see <a href="https://www.stellar.org/developers/horizon/reference/effects-for-operation.html">Effect for Operation</a>
     * @param operationId Operation ID for which to get effects
     */
    EffectsRequestBuilder& forOperation(qint64 operationId);

    /**
     * Requests specific <code>uri</code> and returns {@link Page} of {@link EffectResponse}.
     * This method is helpful for getting the next set of results.
     * @return {@link Page} of {@link EffectResponse}
     */
    EffectsPage* execute(QUrl uri);

    /**
     * Allows to stream SSE events from horizon.
     * Certain endpoints in Horizon can be called in streaming mode using Server-Sent Events.
     * This mode will keep the connection to horizon open and horizon will continue to return
     * responses as ledgers close.
     */
    EffectsRequestBuilder& stream();


    /**
     * Build and execute request.
     * @return {@link Page} of {@link EffectResponse}
     * @throws TooManyRequestsException when too many requests were sent to the Horizon server.
     * @throws IOException
     */
    EffectsPage* execute();

    EffectsRequestBuilder& cursor(QString token);

    EffectsRequestBuilder& limit(qint32 number);

    EffectsRequestBuilder& order(RequestBuilder::Order direction);
};

#endif // EFFECTSREQUESTBUILDER_H
