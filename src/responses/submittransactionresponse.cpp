#include "submittransactionresponse.h"
#include "xdr/stellartransaction.h"
#include "../transaction.h"
SubmitTransactionResponse::SubmitTransactionResponse(QNetworkReply *reply,Transaction * transactionRequest):Response(reply)
  ,m_ledger(0)
  ,m_transactionRequest(transactionRequest)
{
    connect(this,&Response::ready,this,&SubmitTransactionResponse::responseReady);
}

SubmitTransactionResponse::~SubmitTransactionResponse()
{
    if(m_transactionRequest)
        delete m_transactionRequest;
}

Transaction *SubmitTransactionResponse::transactionRequest() const
{
    return m_transactionRequest;
}

QString SubmitTransactionResponse::getHash() const
{
    return m_hash;
}

qint64 SubmitTransactionResponse::getLedger() const
{
    return m_ledger;
}

QString SubmitTransactionResponse::getResultXdr()
{
    if (this->isSuccess()) {
        return this->m_result_xdr;
    } else {
        return this->getExtras().getResultXdr();
    }
}

SubmitTransactionResponseAttach::Extras &SubmitTransactionResponse::getExtras()
{
    return m_extra;
}

bool SubmitTransactionResponse::isSuccess() const
{
    return m_ledger != 0;
}

QString SubmitTransactionResponse::getEnvelopeXdr() {
    if (this->isSuccess()) {
        return this->m_envelope_xdr;
    } else {
        return this->getExtras().getTransactionResultCode();
    }
}

qint64 SubmitTransactionResponse::getOfferIdFromResult(int position) {
    if (!this->isSuccess()) {
        return 0;
    }

    QByteArray data = QByteArray::fromBase64(this->getResultXdr().toLatin1());
    QDataStream stream(&data,QIODevice::ReadOnly);
    stellar::TransactionResult result;
    stream >> result;

    if (result.results.length()<=position) {
        return 0;
    }
    auto& operationResult = result.results.at(position);
    auto operationType = operationResult.type;

    if (operationType == stellar::OperationType::MANAGE_SELL_OFFER) {
        auto& effect = operationResult.manageSellOfferResult.success.effect;
        if (effect == stellar::ManageOfferEffect::MANAGE_OFFER_CREATED
                ||effect == stellar::ManageOfferEffect::MANAGE_OFFER_UPDATED) {
            return operationResult.manageSellOfferResult.success.offer.offerID;
        }
    }
    if (operationType == stellar::OperationType::MANAGE_BUY_OFFER) {
        auto& effect = operationResult.manageBuyOfferResult.success.effect;
        if (effect == stellar::ManageOfferEffect::MANAGE_OFFER_CREATED
                ||effect == stellar::ManageOfferEffect::MANAGE_OFFER_UPDATED) {
            return operationResult.manageBuyOfferResult.success.offer.offerID;
        }
    }
    return 0;
}
