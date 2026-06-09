#ifndef EFFECTSPAGE_H
#define EFFECTSPAGE_H

#include <QObject>
#include "page.h"
#include "effects/effectresponse.h"
#include "qstellar_namespace.h"


QSTELLAR_BEGIN_NS

class EffectsPage : public Page<EffectResponse>
{
public:
    EffectsPage(QNetworkReply *reply=nullptr);

    // PageBase interface
public:
    void processRecords(const QJsonArray &records);
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(EffectsPage)




#endif // EFFECTSPAGE_H
