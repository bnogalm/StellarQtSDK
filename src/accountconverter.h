#ifndef ACCOUNTCONVERTER_H
#define ACCOUNTCONVERTER_H
#include "keypair.h"
#include "strkey.h"

class AccountConverter
{
    bool m_enableMuxed;
public:

    AccountConverter(bool enabled=true);

    /**
     * Returns an AccountConverter which supports muxed accounts.
     **/
    AccountConverter& enableMuxed();

    /**
     * Returns an AccountConverter which does not support muxed accounts. When trying to encode or decode
     * a muxed account the AccountConverter will first convert the muxed account into an ED25519 account id.
     **/
    AccountConverter& disableMuxed();


    /**
     * Encodes an account string into its XDR MuxedAccount representation.
     *
     * @param account the string representation of an account
     */
    stellar::MuxedAccount encode(QString account);

    /**
     * Decodes an XDR MuxedAccount into its string representation.
     *
     * @param account the XDR MuxedAccount representation of an account
     */
    QString decode(stellar::MuxedAccount account);

    /**
     * Filter muxed account
     *
     * @param account the XDR MuxedAccount representation of an account
     */
    stellar::MuxedAccount filter(stellar::MuxedAccount account);


    /**
     * Filter muxed account
     *
     * @param account representation of an account M -> G (if muxed is disabled)
     */
    QString filter(QString account);

};

#endif // ACCOUNTCONVERTER_H
