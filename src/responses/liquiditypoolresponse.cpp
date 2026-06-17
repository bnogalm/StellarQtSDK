#include "liquiditypoolresponse.h"
#include <QCoreApplication>
#include "qtcompat.h"

QSTELLAR_BEGIN_NS


static void registerLiquidityPoolResponseTypes()
{
    regType<LiquidityPoolResponseAttach::Links>();
}
Q_COREAPP_STARTUP_FUNCTION(registerLiquidityPoolResponseTypes)
QSTELLAR_END_NS
