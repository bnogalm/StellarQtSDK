#include "sorobandatabuilder.h"

#include <QDataStream>

QSTELLAR_BEGIN_NS


SorobanDataBuilder& SorobanDataBuilder::setReadOnly(const QList<stellar::SorobanLedgerKey>& keys)
{
    m_data.resources.footprint.readOnly = keys;
    return *this;
}

SorobanDataBuilder& SorobanDataBuilder::setReadWrite(const QList<stellar::SorobanLedgerKey>& keys)
{
    m_data.resources.footprint.readWrite = keys;
    return *this;
}

SorobanDataBuilder& SorobanDataBuilder::setResources(quint32 instructions,
                                                     quint32 diskReadBytes,
                                                     quint32 writeBytes)
{
    m_data.resources.instructions = instructions;
    m_data.resources.diskReadBytes = diskReadBytes;
    m_data.resources.writeBytes = writeBytes;
    return *this;
}

SorobanDataBuilder& SorobanDataBuilder::setResourceFee(qint64 resourceFee)
{
    m_data.resourceFee = resourceFee;
    return *this;
}

QByteArray SorobanDataBuilder::toXdr() const
{
    QByteArray bytes;
    QDataStream s(&bytes, QIODevice::WriteOnly);
    s << m_data;
    return bytes;
}
QSTELLAR_END_NS
