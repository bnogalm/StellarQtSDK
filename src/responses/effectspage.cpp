#include "effectspage.h"
#include <QJsonObject>
#include <QJsonArray>
#include "effects/effectresponse.h"
#include "effects/accountcreatedeffectresponse.h"
#include "effects/accountcreditedeffectresponse.h"
#include "effects/accountdebitedeffectresponse.h"
#include "effects/accountflagsupdatedeffectresponse.h"
#include "effects/accounthomedomainupdatedeffectresponse.h"
#include "effects/accountremovedeffectresponse.h"
#include "effects/accountthresholdsupdatedeffectresponse.h"
#include "effects/offercreatedeffectresponse.h"
#include "effects/offerremovedeffectresponse.h"
#include "effects/offerupdatedeffectresponse.h"
#include "effects/signercreatedeffectresponse.h"
#include "effects/signereffectresponse.h"
#include "effects/signerremovedeffectresponse.h"
#include "effects/signerupdatedeffectresponse.h"
#include "effects/trustlineauthorizationresponse.h"
#include "effects/trustlineauthorizedeffectresponse.h"
#include "effects/trustlinecreatedeffectresponse.h"
#include "effects/trustlinecudresponse.h"
#include "effects/trustlinedeauthorizedeffectresponse.h"
#include "effects/trustlineremovedeffectresponse.h"
#include "effects/trustlineupdatedeffectresponse.h"
#include "effects/trustlineauthorizedtomaintainliabilitieseffectresponse.h"
#include "effects/tradeeffectresponse.h"
#include "effects/accountinflationdestinationupdatedeffectresponse.h"
#include "effects/datacreatedeffectresponse.h"
#include "effects/dataremovedeffectresponse.h"
#include "effects/dataupdatedeffectresponse.h"
#include "effects/sequencebumpedeffectresponse.h"
#include "effects/claimablebalanceclaimantcreatedeffectresponse.h"
#include "effects/claimablebalanceclaimedeffectresponse.h"
#include "effects/claimablebalancecreatedeffectresponse.h"
#include "effects/claimablebalancesponsorshipcreatedeffectresponse.h"
#include "effects/claimablebalancesponsorshipremovedeffectresponse.h"
#include "effects/claimablebalancesponsorshipupdatedeffectresponse.h"
#include "effects/accountsponsorshipcreatedeffectresponse.h"
#include "effects/accountsponsorshipremovedeffectresponse.h"
#include "effects/accountsponsorshipupdatedeffectresponse.h"
#include "effects/trustlinesponsorshipcreatedeffectresponse.h"
#include "effects/trustlinesponsorshipremovedeffectresponse.h"
#include "effects/trustlinesponsorshipupdatedeffectresponse.h"
#include "effects/datasponsorshipcreatedeffectresponse.h"
#include "effects/datasponsorshipremovedeffectresponse.h"
#include "effects/datasponsorshipupdatedeffectresponse.h"
#include "effects/signersponsorshipcreatedeffectresponse.h"
#include "effects/signersponsorshipremovedeffectresponse.h"
#include "effects/signersponsorshipupdatedeffectresponse.h"
#include "effects/trustlineflagsupdatedeffectresponse.h"
#include "effects/claimablebalanceclawedbackeffectresponse.h"


EffectsPage::EffectsPage(QNetworkReply *reply)
    :Page<EffectResponse>(reply)
{

}

void EffectsPage::processRecords(const QJsonArray &records)
{
    for(int i=0;i<records.size();i++){
        QJsonObject obj = records.at(i).toObject();
        int type = obj.value("type_i").toInt();
        Response * effect=0;
        switch(type)
        {
        // Account effects
        case 0:
          effect= new AccountCreatedEffectResponse();break;
        case 1:
          effect= new AccountRemovedEffectResponse();break;
        case 2:
          effect= new AccountCreditedEffectResponse();break;
        case 3:
          effect= new AccountDebitedEffectResponse();break;
        case 4:
          effect= new AccountThresholdsUpdatedEffectResponse();break;
        case 5:
          effect= new AccountHomeDomainUpdatedEffectResponse();break;
        case 6:
          effect= new AccountFlagsUpdatedEffectResponse();break;
        case 7:
          effect= new AccountInflationDestinationUpdatedEffectResponse();break;
        // Signer effects
        case 10:
          effect= new SignerCreatedEffectResponse();break;
        case 11:
          effect= new SignerRemovedEffectResponse();break;
        case 12:
          effect= new SignerUpdatedEffectResponse();break;
        // Trustline effects
        case 20:
          effect= new TrustlineCreatedEffectResponse();break;
        case 21:
          effect= new TrustlineRemovedEffectResponse();break;
        case 22:
          effect= new TrustlineUpdatedEffectResponse();break;
        case 23:
          effect= new TrustlineAuthorizedEffectResponse();break;
        case 24:
          effect= new TrustlineDeauthorizedEffectResponse();break;
        case 25:
          effect= new TrustlineAuthorizedToMaintainLiabilitiesEffectResponse();break;
        case 26:
          effect= new TrustlineFlagsUpdatedEffectResponse();break;

        // Trading effects
        case 30:
          effect= new OfferCreatedEffectResponse();break;
        case 31:
          effect= new OfferRemovedEffectResponse();break;
        case 32:
          effect= new OfferUpdatedEffectResponse();break;
        case 33:
          effect= new TradeEffectResponse();break;
        // Data effects
        case 40:
          effect= new DataCreatedEffectResponse();break;
        case 41:
          effect= new DataRemovedEffectResponse();break;
        case 42:
          effect= new DataUpdatedEffectResponse();break;
        case 43:
          effect= new SequenceBumpedEffectResponse();break;
          // claimable balance effects
        case 50:
          effect= new ClaimableBalanceCreatedEffectResponse();break;
        case 51:
          effect= new ClaimableBalanceClaimantCreatedEffectResponse();break;
        case 52:
          effect= new ClaimableBalanceClaimedEffectResponse();break;
        // sponsorship effects
        case 60:
          effect= new AccountSponsorshipCreatedEffectResponse();break;
        case 61:
          effect= new AccountSponsorshipUpdatedEffectResponse();break;
        case 62:
          effect= new AccountSponsorshipRemovedEffectResponse();break;
        case 63:
          effect= new TrustlineSponsorshipCreatedEffectResponse();break;
        case 64:
          effect= new TrustlineSponsorshipUpdatedEffectResponse();break;
        case 65:
          effect= new TrustlineSponsorshipRemovedEffectResponse();break;
        case 66:
          effect= new DataSponsorshipCreatedEffectResponse();break;
        case 67:
          effect= new DataSponsorshipUpdatedEffectResponse();break;
        case 68:
          effect= new DataSponsorshipRemovedEffectResponse();break;
        case 69:
          effect= new ClaimableBalanceSponsorshipCreatedEffectResponse();break;
        case 70:
          effect= new ClaimableBalanceSponsorshipUpdatedEffectResponse();break;
        case 71:
          effect= new ClaimableBalanceSponsorshipRemovedEffectResponse();break;
        case 72:
          effect= new SignerSponsorshipCreatedEffectResponse();break;
        case 73:
          effect= new SignerSponsorshipUpdatedEffectResponse();break;
        case 74:
          effect= new SignerSponsorshipRemovedEffectResponse();break;
        case 80:
          effect= new ClaimableBalanceClawedBackEffectResponse();break;
        default:
            throw std::runtime_error("Invalid effect type");
        }
        if(effect){
            effect->setParent(this);
            Response::fillObject(effect->metaObject(),effect,obj,true);
            this->m_records.append(effect);
        }
    }
}
