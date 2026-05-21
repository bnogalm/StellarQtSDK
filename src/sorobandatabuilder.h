#ifndef SOROBANDATABUILDER_H
#define SOROBANDATABUILDER_H

#include <QList>
#include <QtGlobal>

#include "xdr/sorobantxdata.h"

/**
 * CAP-46 — fluent composer for `stellar::SorobanTransactionData`.
 *
 * Usage:
 *   SorobanTransactionData data = SorobanDataBuilder()
 *       .setReadOnly(roKeys)
 *       .setReadWrite(rwKeys)
 *       .setResources(1_000_000, 4096, 4096)
 *       .setResourceFee(50_000)
 *       .build();
 *
 * The builder is value-typed and copyable. Mirrors the Java SDK
 * `SorobanDataBuilder`. Operations that carry Soroban data (Invoke /
 * Extend / Restore) are added in 0.10.0; for 0.9.0 the builder lets you
 * produce a `SorobanTransactionData` you can serialize / inspect.
 */
class SorobanDataBuilder
{
    stellar::SorobanTransactionData m_data;
public:
    SorobanDataBuilder() = default;
    /** Initialize from an existing payload (e.g. to amend a parsed one). */
    explicit SorobanDataBuilder(const stellar::SorobanTransactionData& data) : m_data(data) {}

    SorobanDataBuilder& setReadOnly(const QList<stellar::SorobanLedgerKey>& keys);
    SorobanDataBuilder& setReadWrite(const QList<stellar::SorobanLedgerKey>& keys);

    /**
     * Sets instruction count and IO budget.
     * @param instructions  CPU instructions budget.
     * @param diskReadBytes maximum bytes read from disk.
     * @param writeBytes    maximum bytes written to disk.
     */
    SorobanDataBuilder& setResources(quint32 instructions,
                                     quint32 diskReadBytes,
                                     quint32 writeBytes);

    /** Sets the inclusion fee portion paid for Soroban resources (stroops). */
    SorobanDataBuilder& setResourceFee(qint64 resourceFee);

    /** Current snapshot (copy). */
    stellar::SorobanTransactionData build() const { return m_data; }

    /** Convenience XDR serializer for tests / inspection. */
    QByteArray toXdr() const;
};

#endif // SOROBANDATABUILDER_H
