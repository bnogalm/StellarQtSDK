#include "effectspage.h"
#include <QJsonObject>
#include <QJsonArray>
#include "../../src/responses/effects/effectresponse.h"
#include "../../src/responses/effects/accountcreatedeffectresponse.h"
#include "../../src/responses/effects/accountcreditedeffectresponse.h"
#include "../../src/responses/effects/accountdebitedeffectresponse.h"
#include "../../src/responses/effects/accountflagsupdatedeffectresponse.h"
#include "../../src/responses/effects/accounthomedomainupdatedeffectresponse.h"
#include "../../src/responses/effects/accountremovedeffectresponse.h"
#include "../../src/responses/effects/accountthresholdsupdatedeffectresponse.h"
#include "../../src/responses/effects/offercreatedeffectresponse.h"
#include "../../src/responses/effects/offerremovedeffectresponse.h"
#include "../../src/responses/effects/offerupdatedeffectresponse.h"
#include "../../src/responses/effects/signercreatedeffectresponse.h"
#include "../../src/responses/effects/signereffectresponse.h"
#include "../../src/responses/effects/signerremovedeffectresponse.h"
#include "../../src/responses/effects/signerupdatedeffectresponse.h"
#include "../../src/responses/effects/trustlineauthorizationresponse.h"
#include "../../src/responses/effects/trustlineauthorizedeffectresponse.h"
#include "../../src/responses/effects/trustlinecreatedeffectresponse.h"
#include "../../src/responses/effects/trustlinecudresponse.h"
#include "../../src/responses/effects/trustlinedeauthorizedeffectresponse.h"
#include "../../src/responses/effects/trustlineremovedeffectresponse.h"
#include "../../src/responses/effects/trustlineupdatedeffectresponse.h"
#include "../../src/responses/effects/tradeeffectresponse.h"
#include "../../src/responses/effects/accountinflationdestinationupdatedeffectresponse.h"
#include "../../src/responses/effects/datacreatedeffectresponse.h"
#include "../../src/responses/effects/dataremovedeffectresponse.h"
#include "../../src/responses/effects/dataupdatedeffectresponse.h"

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
        default:
            throw std::runtime_error("Invalid operation type");
        }
        if(effect){
            effect->setParent(this);
            Response::fillObject(effect->metaObject(),effect,obj,true);
            this->m_records.append(effect);
        }
    }
}
