#ifndef ACCOUNTFLAGSUPDATEDEFFECTRESPONSE_H
#define ACCOUNTFLAGSUPDATEDEFFECTRESPONSE_H

#include "effectresponse.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

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

QSTELLAR_END_NS

QSTELLAR_ALIAS(AccountFlagsUpdatedEffectResponse)



Q_DECLARE_METATYPE(qstellar::AccountFlagsUpdatedEffectResponse*)
#endif // ACCOUNTFLAGSUPDATEDEFFECTRESPONSE_H

