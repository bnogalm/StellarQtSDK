#ifndef SOROBANRPCEXCEPTION_H
#define SOROBANRPCEXCEPTION_H

#include <QString>
#include <QVariant>

#include "sdkexception.h"

namespace qstellar { namespace exception {

/**
 * Wraps a JSON-RPC 2.0 error object returned by a Soroban RPC node:
 *   { "code": -32601, "message": "Method not found", "data": ... }
 */
class SorobanRpcException : public SdkException
{
    int m_code = 0;
    QString m_rpcMessage;
    QVariant m_data;
public:
    SorobanRpcException(int code, const QString& message, const QVariant& data = QVariant())
        : SdkException(QString("Soroban RPC error %1: %2").arg(code).arg(message))
        , m_code(code)
        , m_rpcMessage(message)
        , m_data(data) {}

    int getCode() const { return m_code; }
    QString getRpcMessage() const { return m_rpcMessage; }
    QVariant getData() const { return m_data; }
};

} } // namespaces

#endif // SOROBANRPCEXCEPTION_H
