#include "liquiditypoolresponse.h"
#include <QCoreApplication>

static void registerLiquidityPoolResponseTypes()
{
    qRegisterMetaType<LiquidityPoolResponseAttach::Links>();
}

Q_COREAPP_STARTUP_FUNCTION(registerLiquidityPoolResponseTypes)
