#ifndef INVOKEHOSTFUNCTIONOPERATIONRESPONSE_H
#define INVOKEHOSTFUNCTIONOPERATIONRESPONSE_H

#include <QList>
#include <QObject>
#include <QString>
#include "operationresponse.h"

namespace InvokeHostFunctionOperationResponseAttach
{

/** XDR-base64 encoded SCVal parameter as exposed by Horizon. */
class Parameter {
    Q_GADGET
    Q_PROPERTY(QString type MEMBER m_type)
    Q_PROPERTY(QString value MEMBER m_value)
    QString m_type;
    QString m_value; // base64-encoded SCVal XDR
public:
    Parameter();
    ~Parameter();
    QString getType() const;
    QString getValue() const;
    bool operator !=(const Parameter& p) const;
    bool operator ==(const Parameter& p) const;
};

/** Balance change attached to a host function invocation (e.g. SAC transfers). */
class AssetBalanceChange {
    Q_GADGET
    Q_PROPERTY(QString type MEMBER m_type)
    Q_PROPERTY(QString from MEMBER m_from)
    Q_PROPERTY(QString to MEMBER m_to)
    Q_PROPERTY(QString amount MEMBER m_amount)
    Q_PROPERTY(QString asset_type MEMBER m_assetType)
    Q_PROPERTY(QString asset_code MEMBER m_assetCode)
    Q_PROPERTY(QString asset_issuer MEMBER m_assetIssuer)
    Q_PROPERTY(QString destination_muxed MEMBER m_destinationMuxed)
    Q_PROPERTY(QString destination_muxed_id MEMBER m_destinationMuxedId)
    QString m_type;
    QString m_from;
    QString m_to;
    QString m_amount;
    QString m_assetType;
    QString m_assetCode;
    QString m_assetIssuer;
    QString m_destinationMuxed;
    QString m_destinationMuxedId;
public:
    AssetBalanceChange();
    ~AssetBalanceChange();
    QString getType() const;
    QString getFrom() const;
    QString getTo() const;
    QString getAmount() const;
    QString getAssetType() const;
    QString getAssetCode() const;
    QString getAssetIssuer() const;
    QString getDestinationMuxed() const;
    QString getDestinationMuxedId() const;
    bool operator !=(const AssetBalanceChange& b) const;
    bool operator ==(const AssetBalanceChange& b) const;
};

}

/**
 * Represents InvokeHostFunction operation response (CAP-46).
 */
class InvokeHostFunctionOperationResponse : public OperationResponse
{
    Q_OBJECT
    Q_PROPERTY(QString function MEMBER m_function)
    Q_PROPERTY(QString address MEMBER m_address)
    Q_PROPERTY(QString salt MEMBER m_salt)
    Q_PROPERTY(QList<InvokeHostFunctionOperationResponseAttach::Parameter> parameters MEMBER m_parameters)
    Q_PROPERTY(QList<InvokeHostFunctionOperationResponseAttach::AssetBalanceChange> asset_balance_changes MEMBER m_assetBalanceChanges)

    QString m_function;
    QString m_address;
    QString m_salt;
    QList<InvokeHostFunctionOperationResponseAttach::Parameter> m_parameters;
    QList<InvokeHostFunctionOperationResponseAttach::AssetBalanceChange> m_assetBalanceChanges;

public:
    InvokeHostFunctionOperationResponse(QNetworkReply* reply=nullptr);
    virtual ~InvokeHostFunctionOperationResponse();

    QString getFunction() const;
    QString getAddress() const;
    QString getSalt() const;
    QList<InvokeHostFunctionOperationResponseAttach::Parameter> getParameters() const;
    QList<InvokeHostFunctionOperationResponseAttach::AssetBalanceChange> getAssetBalanceChanges() const;
};
Q_DECLARE_METATYPE(InvokeHostFunctionOperationResponseAttach::Parameter)
Q_DECLARE_METATYPE(InvokeHostFunctionOperationResponseAttach::AssetBalanceChange)
Q_DECLARE_METATYPE(InvokeHostFunctionOperationResponse*)
#endif // INVOKEHOSTFUNCTIONOPERATIONRESPONSE_H
