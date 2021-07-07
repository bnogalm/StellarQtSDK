#ifndef SEQUENCEBUMPEDEFFECTRESPONSE_H
#define SEQUENCEBUMPEDEFFECTRESPONSE_H

#include "effectresponse.h"
#include <QObject>

class SequenceBumpedEffectResponse : public EffectResponse
{
    Q_OBJECT
    Q_PROPERTY(qint64 new_seq READ getNewSequence WRITE setNewSeq NOTIFY newSeqChanged)
    qint64 m_new_seq;

public:
    SequenceBumpedEffectResponse(QNetworkReply *reply=nullptr);
    qint64 getNewSequence() const;
public slots:
    void setNewSeq(qint64 new_seq);
signals:
    void newSeqChanged();
};

#endif // SEQUENCEBUMPEDEFFECTRESPONSE_H
