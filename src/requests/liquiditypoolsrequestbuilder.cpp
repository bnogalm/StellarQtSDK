#include "liquiditypoolsrequestbuilder.h"
#include "../server.h"
#include "../asset.h"
#include "../assettypenative.h"
#include "../assettypecreditalphanum.h"
#include "../keypair.h"

LiquidityPoolsRequestBuilder::LiquidityPoolsRequestBuilder(Server* server)
    : RequestBuilder(server, "liquidity_pools")
{
}

LiquidityPoolResponse* LiquidityPoolsRequestBuilder::liquidityPool(const QString& id)
{
    this->setSegments(QStringList() << "liquidity_pools" << id);
    this->buildUri();
    return server()->get<LiquidityPoolResponse>(this);
}

LiquidityPoolsRequestBuilder& LiquidityPoolsRequestBuilder::forReserves(const QList<Asset*>& reserves)
{
    // Horizon expects `reserves=asset1,asset2` where each asset is encoded
    // as `native` or `code:issuer`. Pre-encode the colon/comma so the
    // current RequestBuilder URL-builder leaves them intact.
    setAssetsParameter("reserves", reserves);
    return *this;
}

LiquidityPoolsRequestBuilder& LiquidityPoolsRequestBuilder::forAccount(const QString& accountId)
{
    addParameter("account", accountId);
    return *this;
}

LiquidityPoolsRequestBuilder& LiquidityPoolsRequestBuilder::forAccount(KeyPair* account)
{
    return forAccount(account->getAccountId());
}

LiquidityPoolsRequestBuilder& LiquidityPoolsRequestBuilder::cursor(QString token)
{
    RequestBuilder::cursor(token);
    return *this;
}

LiquidityPoolsRequestBuilder& LiquidityPoolsRequestBuilder::limit(int number)
{
    RequestBuilder::limit(number);
    return *this;
}

LiquidityPoolsRequestBuilder& LiquidityPoolsRequestBuilder::order(RequestBuilder::Order direction)
{
    RequestBuilder::order(direction);
    return *this;
}

LiquidityPoolsRequestBuilder& LiquidityPoolsRequestBuilder::stream()
{
    setStreamMode();
    return *this;
}

Page<LiquidityPoolResponse>* LiquidityPoolsRequestBuilder::execute()
{
    return server()->get<Page<LiquidityPoolResponse>>(this);
}
