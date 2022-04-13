#include "settrustlineflagsoperationresponse.h"
#include "keypair.h"
#include "asset.h"
#include <QCoreApplication>

static void registerTypes()
{
    qRegisterMetaType<QList<qint32> >();
    QMetaType::registerConverter<QVariantList ,QList<qint32> >(&ResponseConverters::convertPlain<qint32>);
}

Q_COREAPP_STARTUP_FUNCTION(registerTypes)

SetTrustLineFlagsOperationResponse::SetTrustLineFlagsOperationResponse(QNetworkReply *reply):OperationResponse(reply), m_asset(nullptr)
{

}

SetTrustLineFlagsOperationResponse::~SetTrustLineFlagsOperationResponse()
{
    if(m_asset)
        delete m_asset;
}

const QString SetTrustLineFlagsOperationResponse::getAssetType() const
{
    return m_assetType;
}

void SetTrustLineFlagsOperationResponse::setAssetType(const QString newAsset_type)
{
    if (m_assetType == newAsset_type)
        return;
    m_assetType = newAsset_type;
    emit assetTypeChanged();
}

const QString SetTrustLineFlagsOperationResponse::getAssetCode() const
{
    return m_assetCode;
}

void SetTrustLineFlagsOperationResponse::setAssetCode(const QString newAsset_code)
{
    if (m_assetCode == newAsset_code)
        return;
    m_assetCode = newAsset_code;
    emit assetCodeChanged();
}

const QString SetTrustLineFlagsOperationResponse::getAssetIssuer() const
{
    return m_assetIssuer;
}

void SetTrustLineFlagsOperationResponse::setAssetIssuer(const QString newAsset_issuer)
{
    if (m_assetIssuer == newAsset_issuer)
        return;
    m_assetIssuer = newAsset_issuer;
    emit assetIssuerChanged();
}

const QList<qint32> SetTrustLineFlagsOperationResponse::getClearFlags() const
{
    return m_clearFlags;
}

void SetTrustLineFlagsOperationResponse::setClearFlags(const QList<qint32> newClear_flags)
{
    if (m_clearFlags == newClear_flags)
        return;
    m_clearFlags = newClear_flags;
    emit clearFlagsChanged();
}

const QStringList SetTrustLineFlagsOperationResponse::getClearFlagStrings() const
{
    return m_clearFlagsS;
}

void SetTrustLineFlagsOperationResponse::setClearFlagStrings(const QStringList newClear_flags_s)
{
    if (m_clearFlagsS == newClear_flags_s)
        return;
    m_clearFlagsS = newClear_flags_s;
    emit clearFlagStringsChanged();
}

const QList<qint32> SetTrustLineFlagsOperationResponse::getSetFlags() const
{
    return m_setFlags;
}

void SetTrustLineFlagsOperationResponse::setSetFlags(const QList<qint32> newSet_flags)
{
    if (m_setFlags == newSet_flags)
        return;
    m_setFlags = newSet_flags;
    emit setFlagsChanged();
}

const QStringList SetTrustLineFlagsOperationResponse::getSetFlagStrings() const
{
    return m_setFlagsS;
}

void SetTrustLineFlagsOperationResponse::setSetFlagStrings(const QStringList newSet_flags_s)
{
    if (m_setFlagsS == newSet_flags_s)
        return;
    m_setFlagsS = newSet_flags_s;
    emit setFlagStringsChanged();
}

const QString SetTrustLineFlagsOperationResponse::getTrustor() const
{
    return m_trustor;
}

void SetTrustLineFlagsOperationResponse::setTrustor(const QString newTrustor)
{
    if (m_trustor == newTrustor)
        return;
    m_trustor = newTrustor;
    emit trustorChanged();
}

Asset *SetTrustLineFlagsOperationResponse::getAsset()
{
    if(!m_asset){
        KeyPair* issuer = KeyPair::fromAccountId(m_assetIssuer);
        m_asset= Asset::createNonNativeAsset(m_assetCode, issuer);
    }
    return m_asset;
}
