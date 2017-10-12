#ifndef FEDERATION_H
#define FEDERATION_H

#include "federationresponse.h"
/**
 * Helper class for resolving Stellar addresses.
 *
 * @see <a href="https://www.stellar.org/developers/learn/concepts/federation.html" target="_blank">Federation docs</a>
 */
class Federation
{
private:
    Federation();
public:
    /**
     * This method is a helper method for handling user inputs that contain `destination` value.
     * It accepts two types of values:
     * <ul>
     * <li>For Stellar address (ex. <code>bob*stellar.org`</code>) it splits Stellar address and then tries to find information about
     * federation server in <code>stellar.toml</code> file for a given domain.</li>
     * <li>For account ID (ex. <code>GB5XVAABEQMY63WTHDQ5RXADGYF345VWMNPTN2GFUDZT57D57ZQTJ7PS</code>) it simply returns the
     * given Account ID.</li>
     * </ul>
     * @param value Stellar address or account id
     * @return FederationResponse, you take ownership of the object so delete it when you don't need anymore.
     */
    static FederationResponse* resolve(QString value);
signals:

public slots:
};

#endif // FEDERATION_H

