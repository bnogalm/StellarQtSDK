// Single .cpp for all LP effect response classes — they're header-only
// otherwise. We use this file to register Q_GADGET list converters needed
// by Response::fillObject.

#include "liquiditypoolcreatedeffectresponse.h"
#include "liquiditypooldepositedeffectresponse.h"
#include "liquiditypoolwithdreweffectresponse.h"
#include "liquiditypooltradeeffectresponse.h"
#include "liquiditypoolremovedeffectresponse.h"
#include "liquiditypoolrevokedeffectresponse.h"
#include "../response.h"
#include <QCoreApplication>

static void registerLiquidityPoolEffectTypes()
{
    using namespace qstellar;
    qRegisterMetaType<AssetAmount>();
    qRegisterMetaType<LiquidityPool>();
    qRegisterMetaType<LiquidityPoolClaimableAssetAmount>();
    qRegisterMetaType<QList<AssetAmount>>();
    qRegisterMetaType<QList<LiquidityPoolClaimableAssetAmount>>();

    QMetaType::registerConverter<QVariantList, QList<AssetAmount>>(
        &ResponseConverters::convert<AssetAmount>);
    QMetaType::registerConverter<QVariantList, QList<LiquidityPoolClaimableAssetAmount>>(
        &ResponseConverters::convert<LiquidityPoolClaimableAssetAmount>);
}

Q_COREAPP_STARTUP_FUNCTION(registerLiquidityPoolEffectTypes)
