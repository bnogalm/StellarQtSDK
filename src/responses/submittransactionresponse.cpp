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

quint64 SubmitTransactionResponse::getOfferIdFromResult(int position) {
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

    if (result.results.at(position).type != stellar::OperationType::MANAGE_OFFER) {
        return 0;
    }
    if (result.results.at(position).manageOfferResult.success.effect == stellar::ManageOfferEffect::MANAGE_OFFER_CREATED
            ||result.results.at(position).manageOfferResult.success.effect == stellar::ManageOfferEffect::MANAGE_OFFER_UPDATED) {
        return result.results.at(position).manageOfferResult.success.offer.offerID;
    }
    return 0;
}
