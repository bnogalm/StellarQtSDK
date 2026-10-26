#include "accountsrequestbuilder.h"
#include "../keypair.h"
#include "../server.h"

QSTELLAR_BEGIN_NS


AccountsRequestBuilder::AccountsRequestBuilder(Server *server):RequestBuilder(server, "accounts") {

}

AccountResponse *AccountsRequestBuilder::account(QUrl uri)  {
    this->setRequestUri(uri);//we overwrite the uri
    return server()->get<AccountResponse>(this);
}

AccountResponse* AccountsRequestBuilder::account(QString accountID)
{
    this->setSegments(QStringList()<< "accounts"<< accountID);
    return this->account(this->buildUri());
}

AccountResponse* AccountsRequestBuilder::account(KeyPair *account) {    
    return this->account(account->getAccountId());
}

void AccountsRequestBuilder::checkOneFilter(const QString& filter) const
{
    const QStringList filters = { QStringLiteral("signer"), QStringLiteral("sponsor"),
                                  QStringLiteral("asset"), QStringLiteral("liquidity_pool") };
    for (const QString& other : filters) {
        if (other != filter && !parameter(other).isNull())
            throw std::runtime_error(QStringLiteral("cannot set both %1 and %2: /accounts takes one of "
                                                    "signer, sponsor, asset and liquidity_pool")
                                         .arg(other, filter).toStdString());
    }
}

AccountsRequestBuilder &AccountsRequestBuilder::forSigner(QString accountID)
{
    checkOneFilter(QStringLiteral("signer"));
    addParameter("signer",accountID);
    return *this;
}

AccountsRequestBuilder &AccountsRequestBuilder::forSigner(KeyPair *accountID)
{    
    return forSigner(accountID->getAccountId());
}

AccountsRequestBuilder &AccountsRequestBuilder::forSponsor(QString sponsor) {
    checkOneFilter(QStringLiteral("sponsor"));
    addParameter("sponsor", sponsor);
    return *this;
}

AccountsRequestBuilder &AccountsRequestBuilder::forSponsor(KeyPair* sponsor) {
    return forSponsor(sponsor->getAccountId());
}

AccountsRequestBuilder &AccountsRequestBuilder::forAsset(Asset *asset)
{
    checkOneFilter(QStringLiteral("asset"));
    RequestBuilder::setAssetParameter("asset",asset);
    return *this;
}

AccountsRequestBuilder &AccountsRequestBuilder::forLiquidityPool(const QString& liquidityPoolId)
{
    checkOneFilter(QStringLiteral("liquidity_pool"));
    addParameter("liquidity_pool", liquidityPoolId);
    return *this;
}

AccountsRequestBuilder &AccountsRequestBuilder::cursor(QString token) {
    RequestBuilder::cursor(token);
    return *this;
}

AccountsRequestBuilder &AccountsRequestBuilder::limit(int number) {
    RequestBuilder::limit(number);
    return *this;
}

AccountsRequestBuilder &AccountsRequestBuilder::order(RequestBuilder::Order direction) {
    RequestBuilder::order(direction);
    return *this;
}

AccountsRequestBuilder &AccountsRequestBuilder::stream()
{
    setStreamMode();
    return *this;
}

Page<AccountResponse>* AccountsRequestBuilder::execute(QUrl uri) {
    this->setRequestUri(uri);//we overwrite the uri
    return server()->get<Page<AccountResponse> >(this);
}

Page<AccountResponse>* AccountsRequestBuilder::execute() {
    return server()->get<Page<AccountResponse> >(this);
}
QSTELLAR_END_NS
