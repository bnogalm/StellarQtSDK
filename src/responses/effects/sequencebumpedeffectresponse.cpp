#include "sequencebumpedeffectresponse.h"

QSTELLAR_BEGIN_NS


SequenceBumpedEffectResponse::SequenceBumpedEffectResponse(QNetworkReply *reply):EffectResponse(reply),m_new_seq(0)
{

}

qint64 SequenceBumpedEffectResponse::getNewSequence() const
{
    return m_new_seq;
}

void SequenceBumpedEffectResponse::setNewSeq(qint64 new_seq)
{
    if (m_new_seq == new_seq)
        return;

    m_new_seq = new_seq;
    emit newSeqChanged();
}
QSTELLAR_END_NS
