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
    // Horizon sends only the flags the operation changed (true set, false
    // cleared). An absent one read as false, "cleared": has...() tells.
    Q_PROPERTY(bool auth_required_flag READ getAuthRequiredFlag WRITE setAuthRequiredFlag)
    Q_PROPERTY(bool auth_revokable_flag READ getAuthRevokableFlag WRITE setAuthRevokableFlag)
    bool m_authRequiredFlag;
    bool m_authRevokableFlag;
    bool m_hasAuthRequiredFlag = false;
    bool m_hasAuthRevokableFlag = false;
public:
    AccountFlagsUpdatedEffectResponse(QNetworkReply * reply = nullptr);
    virtual ~AccountFlagsUpdatedEffectResponse();
    /** false when not in the record: see hasAuthRequiredFlag(). */
    bool getAuthRequiredFlag() const;
    bool getAuthRevokableFlag() const;
    /** Whether the record says the operation changed the flag (js: not undefined). */
    bool hasAuthRequiredFlag() const { return m_hasAuthRequiredFlag; }
    /**
     * Whether the record has auth_revokable_flag. Current Horizon never sends
     * it (it stores the change as auth_revocable_flag, which its effect
     * resource does not read), nor the immutable and clawback flags.
     */
    bool hasAuthRevokableFlag() const { return m_hasAuthRevokableFlag; }
    void setAuthRequiredFlag(bool v) { m_authRequiredFlag = v; m_hasAuthRequiredFlag = true; }
    void setAuthRevokableFlag(bool v) { m_authRevokableFlag = v; m_hasAuthRevokableFlag = true; }
protected:
    void reset() override { EffectResponse::reset(); m_hasAuthRequiredFlag = m_hasAuthRevokableFlag = false; }

};

QSTELLAR_END_NS

QSTELLAR_ALIAS(AccountFlagsUpdatedEffectResponse)



Q_DECLARE_METATYPE(qstellar::AccountFlagsUpdatedEffectResponse*)
#endif // ACCOUNTFLAGSUPDATEDEFFECTRESPONSE_H

