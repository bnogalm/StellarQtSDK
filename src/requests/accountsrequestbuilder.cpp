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

AccountDataResponse* AccountsRequestBuilder::accountData(QUrl uri)
{
    this->setRequestUri(uri);//we overwrite the uri
    return server()->get<AccountDataResponse>(this);
}

AccountDataResponse* AccountsRequestBuilder::accountData(KeyPair* account, const QString& key)
{
    if (!account)
        throw std::runtime_error("accountData: account cannot be null");
    // stellar-core's rule for a data name (ManageDataOpFrame: not empty,
    // isStringValid): 1 to 64 bytes, each printable ASCII. QString's UTF-16
    // units are checked, so a character outside ASCII is refused whatever
    // its UTF-8 length.
    if (key.isEmpty())
        throw std::runtime_error("accountData: the data name cannot be empty");
    if (key.toUtf8().size() > 64)
        throw std::runtime_error("accountData: the data name exceeds 64 bytes when UTF-8 encoded");
    for (const QChar c : key) {
        if (c.unicode() < 0x20 || c.unicode() > 0x7E)
            throw std::runtime_error(QStringLiteral("accountData: the data name holds U+%1, but stellar-core "
                                                    "takes only printable ASCII (0x20 to 0x7E) in one")
                                         .arg(QString::number(c.unicode(), 16).toUpper()
                                                  .rightJustified(4, QLatin1Char('0')))
                                         .toStdString());
    }
    // "." and ".." are dot-segments: Qt normalizes them out of the path it
    // sends (and %2E is decoded back to '.'), so no URL addresses them.
    if (key == QLatin1String(".") || key == QLatin1String(".."))
        throw std::runtime_error(QStringLiteral("accountData: the data name \"%1\" cannot be asked for over "
                                                "HTTP: it is a dot-segment, which the path loses")
                                     .arg(key).toStdString());
    // One path segment: every byte but the unreserved ones percent-encoded,
    // so '/', '?', '#', '%' and spaces stay inside the name.
    this->setSegments(QStringList() << "accounts" << account->getAccountId() << "data"
                                    << QString::fromLatin1(QUrl::toPercentEncoding(key)));
    return this->accountData(this->buildUri());
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
