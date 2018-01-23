# StellarQtSDK
Stellar Java SDK ported to Qt/C++. Interface to deal with Horizon API

To include the SDK, just add it in your .pro file
include($$PWD/StellarQtSDK/StellarQtSDK.pri)


Create an account address:
Randoms in some platforms are not true randoms, so i suggest to mix some "random" numbers with human noise (sensors, mouse movements, etc...)

```c++
KeyPair* accountAddress= KeyPair::random(userGenerated32randomBytes);
```
            
If you trust std::random_device, define DEFINES += STELLAR_ALLOW_UNSECURE_RANDOM so you can use KeyPair::random();
            
Choose network to use and build a server object

```c++
Network::useTestNetwork();//select the network to use
Server * server = new Server("https://horizon-testnet.stellar.org");//choose a horizon host
```
            
Request account data:
```c++
        AccountResponse * response = server->accounts().account(accountAddress);

        connect(response,&AccountResponse::ready,[response](){
            if(response->accountID().isEmpty()){
                //accountNotFundedYet
            }
            //response contains account data
        });
```

Make a transaction:
```c++
    //First you should refresh the account data to get the last secuence number.
    AccountResponse * selfAccountResponse= server->accounts().account(accountAddress);

    connect(response,&AccountResponse::ready,[server,selfAccountResponse,targetAddress,amount,asset, memo](){
        //you should also check that the dest account exists
        KeyPair *destination = KeyPair::fromAccountId(targetAddress);
        AccountResponse * destAccountResponse= server->accounts().account(destination);
        connect(responseDestination,&AccountResponse::ready,[server,selfAccountResponse,destAccountResponse,destination,amount,asset, memo](){
            if(destAccountResponse->keypair()){
                //build the transaction
                KeyPair *source = KeyPair::fromAccountId(accountAddress->getAccountId());//we require a copy, i will improve the API
                qint64 sequenceNumber= selfAccountResponse->getSequenceNumber();
                Account* account = new Account(source, sequenceNumber);
                Transaction::Builder *builder = new Transaction::Builder(account);
                builder->addOperation(new PaymentOperation(destination,new AssetTypeNative(),amount));
                if(!memo.isEmpty()){
                    builder->addMemo(Memo::text(memo));//max 28
                }
                Transaction *transaction = builder->build();
                transaction->sign(m_keypair);
                server->submitTransaction(transaction);
                delete builder;
                responseDestination->deleteLater();//there are more objects to delete in this example.
            }
            else{
                //target account is empty
            }
        });
    });
```
    
Stream transactions: We request to horizon to stream the next transaction from the last we know. This way when there is a new one, horizon will send to us the new transaction we update the cursor and refresh the request again.

```c++
        auto builder =  server->payments().forAccount(accountAddress).order(RequestBuilder::Order::ASC).limit(1)
                .stream();

        builder.cursor(QString::number(lastTransactionCursor));
        OperationPage * response = builder.execute();

        connect(response,&OperationResponse::ready,this,[](){
            auto r = m_streamingOperation->streamedElement();
            quint64 token = ((OperationResponse*)r)->getPagingToken().toULongLong();
            //update lastTransactionCursor with token
            //restart all this function to wait for the next transaction.
        });
        connect(response,&OperationResponse::error,this,/*restart all this function*/);
```
