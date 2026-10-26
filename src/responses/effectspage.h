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

    /**
     * A new, unfilled record of the class for the effect type in `record`'s
     * type_i, which must be a whole number in [0, 2^31); throws
     * std::runtime_error naming `field`.type_i otherwise. A type this SDK
     * does not know gives a plain EffectResponse: getType() names it and
     * json() holds all its members, as js and py keep such a record. The
     * caller owns it.
     */
    static EffectResponse* createRecord(const QJsonObject& record, const QString& field);
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(EffectsPage)




#endif // EFFECTSPAGE_H
