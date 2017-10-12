#ifndef ACCOUNTHOMEDOMAINUPDATEDEFFECTRESPONSE_H
#define ACCOUNTHOMEDOMAINUPDATEDEFFECTRESPONSE_H

#include "effectresponse.h"
/**
 * Represents account_home_domain_updated effect response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
 * @see org.stellar.sdk.requests.EffectsRequestBuilder
 * @see org.stellar.sdk.Server#effects()
 */
class AccountHomeDomainUpdatedEffectResponse : public EffectResponse
{
    Q_OBJECT
    Q_PROPERTY(QString home_domain MEMBER m_homeDomain)
    QString m_homeDomain;
public:
    AccountHomeDomainUpdatedEffectResponse(QNetworkReply *reply = nullptr);
    virtual ~AccountHomeDomainUpdatedEffectResponse();
    QString getHomeDomain() const;
};
Q_DECLARE_METATYPE(AccountHomeDomainUpdatedEffectResponse*)
#endif // ACCOUNTHOMEDOMAINUPDATEDEFFECTRESPONSE_H



