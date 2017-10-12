#ifndef EFFECTSPAGE_H
#define EFFECTSPAGE_H

#include <QObject>
#include "page.h"
#include "effects/effectresponse.h"


class EffectsPage : public Page<EffectResponse>
{
public:
    EffectsPage(QNetworkReply *reply=nullptr);

    // PageBase interface
public:
    void processRecords(const QJsonArray &records);
};

#endif // EFFECTSPAGE_H
