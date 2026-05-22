#include "invokehostfunctionoperationresponse.h"

#include <QCoreApplication>
#include "../response.h"

static void registerInvokeHostFnTypes()
{
    qRegisterMetaType<InvokeHostFunctionOperationResponseAttach::Parameter>();
    qRegisterMetaType<InvokeHostFunctionOperationResponseAttach::AssetBalanceChange>();
    QMetaType::registerConverter<QVariantList,
        QList<InvokeHostFunctionOperationResponseAttach::Parameter>>(
        &ResponseConverters::convert<InvokeHostFunctionOperationResponseAttach::Parameter>);
    QMetaType::registerConverter<QVariantList,
        QList<InvokeHostFunctionOperationResponseAttach::AssetBalanceChange>>(
        &ResponseConverters::convert<InvokeHostFunctionOperationResponseAttach::AssetBalanceChange>);
}

Q_COREAPP_STARTUP_FUNCTION(registerInvokeHostFnTypes)

// ─── Parameter ──────────────────────────────────────────────────────

InvokeHostFunctionOperationResponseAttach::Parameter::Parameter() {}
InvokeHostFunctionOperationResponseAttach::Parameter::~Parameter() {}

QString InvokeHostFunctionOperationResponseAttach::Parameter::getType() const { return m_type; }
QString InvokeHostFunctionOperationResponseAttach::Parameter::getValue() const { return m_value; }

bool InvokeHostFunctionOperationResponseAttach::Parameter::operator==(const Parameter& p) const
{
    return m_type == p.m_type && m_value == p.m_value;
}
bool InvokeHostFunctionOperationResponseAttach::Parameter::operator!=(const Parameter& p) const
{
    return !(*this == p);
}

// ─── AssetBalanceChange ─────────────────────────────────────────────

InvokeHostFunctionOperationResponseAttach::AssetBalanceChange::AssetBalanceChange() {}
InvokeHostFunctionOperationResponseAttach::AssetBalanceChange::~AssetBalanceChange() {}

QString InvokeHostFunctionOperationResponseAttach::AssetBalanceChange::getType() const { return m_type; }
QString InvokeHostFunctionOperationResponseAttach::AssetBalanceChange::getFrom() const { return m_from; }
QString InvokeHostFunctionOperationResponseAttach::AssetBalanceChange::getTo() const { return m_to; }
QString InvokeHostFunctionOperationResponseAttach::AssetBalanceChange::getAmount() const { return m_amount; }
QString InvokeHostFunctionOperationResponseAttach::AssetBalanceChange::getAssetType() const { return m_assetType; }
QString InvokeHostFunctionOperationResponseAttach::AssetBalanceChange::getAssetCode() const { return m_assetCode; }
QString InvokeHostFunctionOperationResponseAttach::AssetBalanceChange::getAssetIssuer() const { return m_assetIssuer; }
QString InvokeHostFunctionOperationResponseAttach::AssetBalanceChange::getDestinationMuxed() const { return m_destinationMuxed; }
QString InvokeHostFunctionOperationResponseAttach::AssetBalanceChange::getDestinationMuxedId() const { return m_destinationMuxedId; }

bool InvokeHostFunctionOperationResponseAttach::AssetBalanceChange::operator==(const AssetBalanceChange& b) const
{
    return m_type == b.m_type && m_from == b.m_from && m_to == b.m_to
        && m_amount == b.m_amount && m_assetType == b.m_assetType
        && m_assetCode == b.m_assetCode && m_assetIssuer == b.m_assetIssuer
        && m_destinationMuxed == b.m_destinationMuxed
        && m_destinationMuxedId == b.m_destinationMuxedId;
}
bool InvokeHostFunctionOperationResponseAttach::AssetBalanceChange::operator!=(const AssetBalanceChange& b) const
{
    return !(*this == b);
}

// ─── InvokeHostFunctionOperationResponse ────────────────────────────

InvokeHostFunctionOperationResponse::InvokeHostFunctionOperationResponse(QNetworkReply* reply)
    : OperationResponse(reply)
{
}

InvokeHostFunctionOperationResponse::~InvokeHostFunctionOperationResponse() {}

QString InvokeHostFunctionOperationResponse::getFunction() const { return m_function; }
QString InvokeHostFunctionOperationResponse::getAddress() const { return m_address; }
QString InvokeHostFunctionOperationResponse::getSalt() const { return m_salt; }
QList<InvokeHostFunctionOperationResponseAttach::Parameter>
InvokeHostFunctionOperationResponse::getParameters() const { return m_parameters; }
QList<InvokeHostFunctionOperationResponseAttach::AssetBalanceChange>
InvokeHostFunctionOperationResponse::getAssetBalanceChanges() const { return m_assetBalanceChanges; }
