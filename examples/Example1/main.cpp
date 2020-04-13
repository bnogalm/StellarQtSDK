#include <QCoreApplication>
#include <QDebug>
#include <QTimer>

#include "network.h"
#include "server.h"
#include "keypair.h"
#include "responses/operations/paymentoperationresponse.h"
#include "responses/operations/createaccountoperationresponse.h"



int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);


    Network::useTestNetwork();//select the network to use
    Server * server = new Server("https://horizon-testnet.stellar.org");//choose a horizon host
    //Qt removes child QObjects before removing parent, Server is a QObject, so we can use it to remove it before application ends.
    //responses requested to a server object are child of the used server, so they will be removed with the server if you didn't before.
    server->setParent(qApp);

    KeyPair * keypair = KeyPair::random();

    qDebug() << "Created keypair";
    qDebug() << "Public key: "<<keypair->getAccountId();
    qDebug() << "Private key: "<<keypair->getSecretSeed();


    //check if the account is already created, if not, we alert the user to create it
    AccountResponse * accountCheckResponse = server->accounts().account(keypair);

    //here we connect the signal ready of the response to a lambda function, that will be executed as soon response is ready
    QObject::connect(accountCheckResponse,&AccountResponse::ready,[accountCheckResponse,keypair](){
        if(accountCheckResponse->accountID().isEmpty()){
            //account not funded yet
            qDebug() << "Account not funded yet, fund it using friendbot loading the next link:";
            qDebug() << QString("https://friendbot.stellar.org?addr=%1").arg(keypair->getAccountId());
        }
        //response contains account data
        //we remove the response as we don't need anymore
        // note that as we are using a lambda, this pointer is now deleted but not set to nullptr
        // we could get this pointer by reference, to be able to set it to nullptr
        delete accountCheckResponse; //we don't need it anymore. It would be deleted anyway by server destruction, but normally you will have to remove it here

        //we dont need it anymore, note this lambda will not be executed until Qt event loop starts, that is why we can use it the next lines
        delete keypair;
    });

    //stream the all the payment for our account
    OperationPage* paymentsStream =  server->payments().forAccount(keypair).order(RequestBuilder::Order::ASC)
            .stream().execute();



    QObject::connect(paymentsStream,&OperationResponse::ready,[paymentsStream](){
        QList<Response*> records = paymentsStream->getRecords();
        for(Response * r : records)
        {
            if(PaymentOperationResponse * payment = dynamic_cast<PaymentOperationResponse*>(r))
            {
                qDebug() << "Payment detected! Amount: "<< payment->getAmount()<< " Asset: "<< payment->getAsset()->toString()<< " From: "<< payment->getFrom().getAccountId() << " Created at: "<< payment->getCreatedAt();
            }
            else if(CreateAccountOperationResponse * createAccount = dynamic_cast<CreateAccountOperationResponse*>(r))
            {
                qDebug() << "Create account detected! Amount: "<< createAccount->getStartingBalance()<< " From: "<< createAccount->getFunder().getAccountId()<< " Created at: "<<createAccount->getCreatedAt();

            }
        }
        //as is a streaming request, objects will be automatically removed with the next update.
        //you should take this in consideration, as you can't store them to read values later.
    });


    return a.exec();
}
