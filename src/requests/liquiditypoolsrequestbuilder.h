#ifndef LIQUIDITYPOOLSREQUESTBUILDER_H
#define LIQUIDITYPOOLSREQUESTBUILDER_H

#include "requestbuilder.h"
#include "../responses/liquiditypoolresponse.h"
#include "../responses/page.h"
#include <QStringList>

#include "../keypair.h"
#include "../qstellar_namespace.h"
QSTELLAR_FWD(Asset)
QSTELLAR_BEGIN_NS


/**
 * Builds requests for the `/liquidity_pools` Horizon endpoint.
 *
 * @see https://developers.stellar.org/docs/data/apis/horizon/api-reference/resources/liquiditypools
 */
class LiquidityPoolsRequestBuilder : public RequestBuilder
{
public:
    LiquidityPoolsRequestBuilder(Server* server);

    /**
     * Requests `GET /liquidity_pools/{id}` and returns a single LiquidityPoolResponse.
     */
    LiquidityPoolResponse* liquidityPool(const QString& id);

    /**
     * Filter pools by exact reserve assets (multi-asset filter).
     * Horizon expects assets joined by commas, in canonical form.
     */
    LiquidityPoolsRequestBuilder& forReserves(const QList<Asset*>& reserves);

    /**
     * Pools that include the given account as a trustline / participant.
     */
    LiquidityPoolsRequestBuilder& forAccount(const QString& accountId);
    LiquidityPoolsRequestBuilder& forAccount(KeyPair* account);

    LiquidityPoolsRequestBuilder& cursor(QString token);
    LiquidityPoolsRequestBuilder& limit(int number);
    LiquidityPoolsRequestBuilder& order(Order direction);
    LiquidityPoolsRequestBuilder& stream();

    /** Build and execute. Returns a Page; caller owns it. */
    Page<LiquidityPoolResponse>* execute();
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(LiquidityPoolsRequestBuilder)

#endif // LIQUIDITYPOOLSREQUESTBUILDER_H
