#ifndef ACCOUNTFLAGSUPDATEDEFFECTRESPONSE_H
#define ACCOUNTFLAGSUPDATEDEFFECTRESPONSE_H

#include "effectresponse.h"

/**
 * Represents account_flags_updated effect response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
 * @see org.stellar.sdk.requests.EffectsRequestBuilder
 * @see org.stellar.sdk.Server#effects()
 */
class AccountFlagsUpdatedEffectResponse : public EffectResponse
{
    Q_OBJECT
    Q_PROPERTY(bool auth_required_flag MEMBER m_authRequiredFlag)
    Q_PROPERTY(bool auth_revokable_flag MEMBER m_authRevokableFlag)
    bool m_authRequiredFlag;
    bool m_authRevokableFlag;
public:
    AccountFlagsUpdatedEffectResponse(QNetworkReply * reply = nullptr);
    virtual ~AccountFlagsUpdatedEffectResponse();
    bool getAuthRequiredFlag() const;
    bool getAuthRevokableFlag() const;

};
Q_DECLARE_METATYPE(AccountFlagsUpdatedEffectResponse*)
#endif // ACCOUNTFLAGSUPDATEDEFFECTRESPONSE_H

