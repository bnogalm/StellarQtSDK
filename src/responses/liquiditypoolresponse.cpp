#include "liquiditypoolresponse.h"
#include <QCoreApplication>

QSTELLAR_BEGIN_NS


static void registerLiquidityPoolResponseTypes()
{
    qRegisterMetaType<LiquidityPoolResponseAttach::Links>();
}

Q_COREAPP_STARTUP_FUNCTION(registerLiquidityPoolResponseTypes)
QSTELLAR_END_NS
