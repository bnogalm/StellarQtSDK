#include "wallet.h"
#include "transaction.h"
#include "paymentoperation.h"
#include "createaccountoperation.h"
#include "responses/submittransactionresponse.h"

Wallet::Wallet(QObject *parent) : QObject(parent)
  , m_gateway(nullptr)
  , m_account(nullptr)  
{

}

Wallet::~Wallet()
{
    if(m_account)
        delete m_account;
}

QString Wallet::publicAddress() const
{
    return m_account ? m_account->getKeypair()->getAccountId() : QString();
}

QString Wallet::balance() const
{
    return m_balance;
}

bool Wallet::funded() const
{
    return m_account ? m_account->getSequenceNumber()!=0 : false;
}

void Wallet::createRandom()
{
    //we protect from overwrite the account.
    if(!m_account)
    {
        m_account = new Account(KeyPair::random(),0);
        //we just created m_keypair, it changes publicAddress property and we want to notify everybody about it
        emit publicAddressChanged();
    }
}


void Wallet::fund()
{
    if(m_account && m_gateway){
        QNetworkRequest request(QString("https://friendbot.stellar.org?addr=%1").arg(publicAddress()));
        QNetworkReply * reply = m_gateway->server()->manager().get(request);
        connect(reply, &QNetworkReply::finished, this, &Wallet::update);//if we get an answer, we update the account
        connect(reply, &QNetworkReply::finished, reply, &QObject::deleteLater);//we don't manage errors, it is an example!
    }
}

void Wallet::update()
{
    if(m_account && m_gateway)
    {
        AccountResponse * accountCheckResponse = m_gateway->server()->accounts().account(m_account->getKeypair());
        //here we connect the signal ready of the response to a private slot, that will be executed as soon response is ready
        connect(accountCheckResponse,&AccountResponse::ready
                ,this,&Wallet::processAccountCheckResponse);
    }
}

void Wallet::pay(QString destination, QString amount, QString memo)
{
    try {
        //verify the destination account is valid, building a keypair object, exception will be rised if there is a problem with it.
        KeyPair* destKeypair = KeyPair::fromAccountId(destination);

        Server *server= m_gateway->server();

        Transaction *t = Transaction::Builder(m_account)
                .addOperation(new PaymentOperation(destKeypair,new AssetTypeNative(),amount))
                .addMemo(new MemoText(memo))
                .setTimeout(10000)// we have to set a timeout                
                .build();
        t->sign(m_account->getKeypair());
        SubmitTransactionResponse* response = server->submitTransaction(t);
        connect(response, &SubmitTransactionResponse::ready, this, &Wallet::processPaymentSuccess);
        connect(response, &SubmitTransactionResponse::error, this, &Wallet::processPaymentError);

    } catch (std::exception err) {
        QString lastError(err.what());
        emit error(lastError);
    }

}


void Wallet::create(QString destination, QString amount, QString memo)
{
    try {
        //verify the destination account is valid, building a keypair object, exception will be rised if there is a problem with it.
        KeyPair* destKeypair = KeyPair::fromAccountId(destination);

        Server *server= m_gateway->server();

        Transaction *t = Transaction::Builder(m_account)
                .addOperation(new CreateAccountOperation(destKeypair,amount))
                .addMemo(new MemoText(memo))
                .setTimeout(10000)// we have to set a timeout
                .build();
        t->sign(m_account->getKeypair());
        SubmitTransactionResponse* response = server->submitTransaction(t);
        connect(response, &SubmitTransactionResponse::ready, this, &Wallet::processPaymentSuccess);
        connect(response, &SubmitTransactionResponse::error, this, &Wallet::processPaymentError);

    } catch (std::exception err) {
        QString lastError(err.what());
        emit error(lastError);
    }

}

void Wallet::setPublicAddress(QString publicAddress)
{
    //if is the same, we don't modify anything
    if(m_account && m_account->getKeypair()->getAccountId()==publicAddress)
        return;
    //if it changed, we delete it
    if(m_account)
        delete m_account;

    try {
         KeyPair *key = KeyPair::fromAccountId(publicAddress);
            m_account = new Account(key, 0);
            update();
    } catch (std::exception err) {
        m_account = nullptr;
    }
    emit publicAddressChanged();
}

void Wallet::processAccountCheckResponse()
{
    //sender() returns who calls the slot.
    AccountResponse *accountCheckResponse = static_cast<AccountResponse*>(sender());
    if(accountCheckResponse->accountID().isEmpty()){
        //account not funded yet
    }
    else{
        //response contains account data
        //update the account with the correct sequence number, we have to do a copy of the original keypair because keypair is deleted with the account
        Account* updatedAccount = new Account(new KeyPair(*(m_account->getKeypair())), accountCheckResponse->getSequenceNumber());
        delete m_account;
        m_account = updatedAccount;
        auto balances = accountCheckResponse->getBalances();
        for(auto balance: balances)
        {
            if(balance.getAssetType() == "native")
            {
                m_balance = balance.getBalance();
                //notify balance changed!
                emit balanceChanged();
            }
        }
    }
    //notify we know now if it is funded or not
    emit fundedChanged();
    //remove the response as we don't need anymore
    delete accountCheckResponse; //we don't need it anymore. It would be deleted anyway by server destruction, but normally you will have to remove it here
}

void Wallet::processPaymentSuccess()
{

    if(SubmitTransactionResponse * response = dynamic_cast<SubmitTransactionResponse*>(sender())){

        this->update();//request an update of the account
        delete response;//you have to delete the response
        emit success();
    }
}

void Wallet::processPaymentError()
{
    if(SubmitTransactionResponse * response = dynamic_cast<SubmitTransactionResponse*>(sender())){
        emit error(QString("Payment error"));
        this->update();//request an update of the account
        delete response;//you have to delete the response
    }
}
