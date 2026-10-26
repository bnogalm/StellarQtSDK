#ifndef FEDERATION_H
#define FEDERATION_H

#include "federationresponse.h"
#include "../qstellar_namespace.h"
QSTELLAR_BEGIN_NS

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
     * given Account ID, once it has checked that it is one (G... or M...):
     * json() is {"account_id": value}, the record js hands back (js takes a
     * G address only).</li>
     * </ul>
     * Throws, before asking anybody, whatever validate() refuses.
     * @param value Stellar address or account id
     * @return FederationResponse, you take ownership of the object so delete it when you don't need anymore.
     */
    static FederationResponse* resolve(QString value);

    /**
     * Throws, with the reason, on a value resolve() would refuse: a blank one,
     * more than one '*', an empty name, a domain that is not a host name (an
     * optional port aside, as js and py take one; see
     * StellarTomlResolver::isHostName), or a value without '*' that is not an
     * account id. Asks nobody.
     */
    static void validate(QString value);
signals:

public slots:
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(Federation)

#endif // FEDERATION_H
