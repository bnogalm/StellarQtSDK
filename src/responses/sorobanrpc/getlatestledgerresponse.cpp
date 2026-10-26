#include "getlatestledgerresponse.h"
#include "gethealthresponse.h"

#include <limits>

QSTELLAR_BEGIN_NS


GetLatestLedgerResponse GetLatestLedgerResponse::fromJson(const QJsonObject& r)
{
    // The sequence used to be read from a string too (text as 0, a number
    // past 32 bits cut to 32), and a missing one read as ledger 0.
    const sorobanrpcdetail::RpcFields fields(r, QStringLiteral("getLatestLedger"));
    GetLatestLedgerResponse out;
    out.m_id              = fields.hash(QStringLiteral("id"));
    out.m_protocolVersion = fields.int32(QStringLiteral("protocolVersion"));
    out.m_sequence        = fields.uint32(QStringLiteral("sequence"));
    // closeTime, headerXdr and metadataXdr used to be dropped; js and py read
    // them (py requires all three, js both XDRs).
    out.m_closeTime       = fields.decimal(QStringLiteral("closeTime"),
                                           std::numeric_limits<qint64>::max());
    out.m_headerXdr       = fields.xdr(QStringLiteral("headerXdr"));
    out.m_metadataXdr     = fields.xdr(QStringLiteral("metadataXdr"));
    return out;
}
QSTELLAR_END_NS
