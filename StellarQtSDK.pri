

DEFINES += STELLAR_QT_REPLY_TIMEOUT=30000

DEFINES *= ED25519_NO_SEED

DEFINES += STELLAR_QT_SDK_CLIENT_NAME=\"\\\"qtcpp-stellar-sdk\\\"\"
DEFINES += STELLAR_QT_SDK_CLIENT_VERSION=\"\\\"0.3.19\\\"\"

#DEFINES += STELLAR_QT_AUTOSET_BASE_FEE

#If you have problems with network connections, uncomment this define to print some traces that may help you.
#DEFINES += STELLAR_QT_DEBUG_NETWORK_REQUESTS

QT *= core network
CONFIG *= c++11

TEMPLATE = app


greaterThan(QT_MAJOR_VERSION, 5) {
    DEFINES +=  QIODeviceEnums=QIODeviceBase
} else {
    DEFINES +=  QIODeviceEnums=QIODevice
}

INCLUDEPATH *=  $$PWD
INCLUDEPATH *=  $$PWD/src/


SOURCES += \
    $$PWD/src/abstracttransaction.cpp \
    $$PWD/src/account.cpp \
    $$PWD/src/accountconverter.cpp \
    $$PWD/src/accountmergeoperation.cpp \
    $$PWD/src/beginsponsoringfuturereservesoperation.cpp \
    $$PWD/src/claimant.cpp \
    $$PWD/src/claimclaimablebalanceoperation.cpp \
    $$PWD/src/clawbackclaimablebalanceoperation.cpp \
    $$PWD/src/clawbackoperation.cpp \
    $$PWD/src/createclaimablebalanceoperation.cpp \
    $$PWD/src/createpassivesellofferoperation.cpp \
    $$PWD/src/endsponsoringfuturereservesoperation.cpp \
    $$PWD/src/feebumptransaction.cpp \
    $$PWD/src/managebuyofferoperation.cpp \
    $$PWD/src/managesellofferoperation.cpp \
    $$PWD/src/operation.cpp \
    $$PWD/src/keypair.cpp \
    $$PWD/src/allowtrustoperation.cpp \
    $$PWD/src/asset.cpp \
    $$PWD/src/assettypecreditalphanum.cpp \
    $$PWD/src/assettypecreditalphanum12.cpp \
    $$PWD/src/assettypecreditalphanum4.cpp \
    $$PWD/src/assettypenative.cpp \
    $$PWD/src/changetrustoperation.cpp \
    $$PWD/src/createaccountoperation.cpp \
    $$PWD/src/pathpaymentstrictreceiveoperation.cpp \
    $$PWD/src/pathpaymentstrictsendoperation.cpp \
    $$PWD/src/predicate.cpp \
    $$PWD/src/requests/claimablebalancesrequestbuilder.cpp \
    $$PWD/src/requests/feestatsrequestbuilder.cpp \
    $$PWD/src/requests/strictreceivepathsrequestbuilder.cpp \
    $$PWD/src/requests/strictsendpathsrequestbuilder.cpp \
    $$PWD/src/responses/claimablebalanceresponse.cpp \
    $$PWD/src/responses/effects/accountsponsorshipcreatedeffectresponse.cpp \
    $$PWD/src/responses/effects/accountsponsorshipremovedeffectresponse.cpp \
    $$PWD/src/responses/effects/accountsponsorshipupdatedeffectresponse.cpp \
    $$PWD/src/responses/effects/claimablebalanceclaimantcreatedeffectresponse.cpp \
    $$PWD/src/responses/effects/claimablebalanceclaimedeffectresponse.cpp \
    $$PWD/src/responses/effects/claimablebalanceclawedbackeffectresponse.cpp \
    $$PWD/src/responses/effects/claimablebalancecreatedeffectresponse.cpp \
    $$PWD/src/responses/effects/claimablebalancesponsorshipcreatedeffectresponse.cpp \
    $$PWD/src/responses/effects/claimablebalancesponsorshipremovedeffectresponse.cpp \
    $$PWD/src/responses/effects/claimablebalancesponsorshipupdatedeffectresponse.cpp \
    $$PWD/src/responses/effects/datasponsorshipcreatedeffectresponse.cpp \
    $$PWD/src/responses/effects/datasponsorshipremovedeffectresponse.cpp \
    $$PWD/src/responses/effects/datasponsorshipupdatedeffectresponse.cpp \
    $$PWD/src/responses/effects/sequencebumpedeffectresponse.cpp \
    $$PWD/src/responses/effects/signersponsorshipcreatedeffectresponse.cpp \
    $$PWD/src/responses/effects/signersponsorshipremovedeffectresponse.cpp \
    $$PWD/src/responses/effects/signersponsorshipupdatedeffectresponse.cpp \
    $$PWD/src/responses/effects/trustlineauthorizedtomaintainliabilitieseffectresponse.cpp \
    $$PWD/src/responses/effects/trustlineflagsupdatedeffectresponse.cpp \
    $$PWD/src/responses/effects/trustlinesponsorshipcreatedeffectresponse.cpp \
    $$PWD/src/responses/effects/trustlinesponsorshipremovedeffectresponse.cpp \
    $$PWD/src/responses/effects/trustlinesponsorshipupdatedeffectresponse.cpp \
    $$PWD/src/responses/feestatsresponse.cpp \
    $$PWD/src/responses/operations/beginsponsoringfuturereservesoperationresponse.cpp \
    $$PWD/src/responses/operations/bumpsequenceoperationresponse.cpp \
    $$PWD/src/responses/operations/claimclaimablebalanceoperationresponse.cpp \
    $$PWD/src/responses/operations/clawbackclaimablebalanceoperationresponse.cpp \
    $$PWD/src/responses/operations/clawbackoperationresponse.cpp \
    $$PWD/src/responses/operations/createclaimablebalanceoperationresponse.cpp \
    $$PWD/src/responses/operations/endsponsoringfuturereservesoperationresponse.cpp \
    $$PWD/src/responses/operations/managebuyofferoperationresponse.cpp \
    $$PWD/src/responses/operations/managesellofferoperationresponse.cpp \
    $$PWD/src/responses/operations/pathpaymentbaseoperationresponse.cpp \
    $$PWD/src/responses/operations/pathpaymentstrictreceiveoperationresponse.cpp \
    $$PWD/src/responses/operations/pathpaymentstrictsendoperationresponse.cpp \
    $$PWD/src/responses/operations/revokesponsorshipoperationresponse.cpp \
    $$PWD/src/responses/operations/settrustlineflagsoperationresponse.cpp \
    $$PWD/src/revokeaccountsponsorshipoperation.cpp \
    $$PWD/src/revokeclaimablebalancesponsorshipoperation.cpp \
    $$PWD/src/revokedatasponsorshipoperation.cpp \
    $$PWD/src/revokeoffersponsorshipoperation.cpp \
    $$PWD/src/revokesignersponsorshipoperation.cpp \
    $$PWD/src/revoketrustlinesponsorshipoperation.cpp \
    $$PWD/src/sep10challenge.cpp \
    $$PWD/src/settrustlineflagsoperation.cpp \
    $$PWD/src/strkey.cpp \
    $$PWD/src/inflationoperation.cpp \
    $$PWD/src/util.cpp \
    $$PWD/src/network.cpp \
    $$PWD/src/pathpaymentoperation.cpp \
    $$PWD/src/paymentoperation.cpp \
    $$PWD/src/price.cpp \
    $$PWD/src/server.cpp \
    $$PWD/src/setoptionsoperation.cpp \
    $$PWD/src/signer.cpp \
    $$PWD/src/transaction.cpp \
    $$PWD/src/memo.cpp \
    $$PWD/xdr/xdrhelper.cpp \
    $$PWD/external/ed25519/src/add_scalar.c \
    $$PWD/external/ed25519/src/ed25519_keypair.c \
    $$PWD/external/ed25519/src/fe.c \
    $$PWD/external/ed25519/src/ge.c \
    $$PWD/external/ed25519/src/key_exchange.c \
    $$PWD/external/ed25519/src/sc.c \
    $$PWD/external/ed25519/src/seed.c \
    $$PWD/external/ed25519/src/sha512.c \
    $$PWD/external/ed25519/src/sign.c \
    $$PWD/external/ed25519/src/verify.c \
    $$PWD/external/cyoencode/src/CyoDecode.cpp \
    $$PWD/external/cyoencode/src/CyoEncode.cpp \
    $$PWD/src/requests/effectsrequestbuilder.cpp \
    $$PWD/src/requests/eventlistener.cpp \
    $$PWD/src/requests/ledgersrequestbuilder.cpp \
    $$PWD/src/requests/offersrequestbuilder.cpp \
    $$PWD/src/requests/operationsrequestbuilder.cpp \
    $$PWD/src/requests/orderbookrequestbuilder.cpp \
    $$PWD/src/requests/paymentsrequestbuilder.cpp \
    $$PWD/src/requests/requestbuilder.cpp \
    $$PWD/src/requests/transactionsrequestbuilder.cpp \
    $$PWD/src/requests/accountsrequestbuilder.cpp \
    $$PWD/src/responses/accountresponse.cpp \
    $$PWD/src/responses/response.cpp \
    $$PWD/src/responses/submittransactionresponse.cpp \
    $$PWD/src/responses/link.cpp \
    $$PWD/src/responses/transactionresponse.cpp \
    $$PWD/src/responses/page.cpp \
    $$PWD/src/responses/operations/operationresponse.cpp \
    $$PWD/src/responses/operationpage.cpp \
    $$PWD/src/responses/operations/paymentoperationresponse.cpp \
    $$PWD/src/responses/operations/createaccountoperationresponse.cpp \
    $$PWD/src/federation/federationserver.cpp \
    $$PWD/src/federation/federationresponse.cpp \
    $$PWD/src/federation/federation.cpp \
    $$PWD/src/responses/operations/accountmergeoperationresponse.cpp \
    $$PWD/src/responses/operations/pathpaymentoperationresponse.cpp \
    $$PWD/src/responses/operations/inflationoperationresponse.cpp \
    $$PWD/src/responses/operations/allowtrustoperationresponse.cpp \
    $$PWD/src/responses/operations/changetrustoperationresponse.cpp \
    $$PWD/src/responses/operations/createpassiveofferoperationresponse.cpp \
    $$PWD/src/managedataoperation.cpp \
    $$PWD/src/responses/operations/managedataoperationresponse.cpp \
    $$PWD/src/responses/operations/setoptionsoperationresponse.cpp \
    $$PWD/xdr/stellartransaction.cpp \
    $$PWD/src/requests/tradesrequestbuilder.cpp \
    $$PWD/src/responses/traderesponse.cpp \
    $$PWD/src/responses/pathresponse.cpp \
    $$PWD/src/responses/orderbookresponse.cpp \
    $$PWD/src/responses/offerresponse.cpp \
    $$PWD/src/responses/ledgerresponse.cpp \
    $$PWD/src/responses/genericoperation.cpp \
    $$PWD/src/responses/effects/effectresponse.cpp \
    $$PWD/src/responses/effects/accountcreatedeffectresponse.cpp \
    $$PWD/src/responses/effects/accountcreditedeffectresponse.cpp \
    $$PWD/src/responses/effects/accountdebitedeffectresponse.cpp \
    $$PWD/src/responses/effects/accountflagsupdatedeffectresponse.cpp \
    $$PWD/src/responses/effects/accounthomedomainupdatedeffectresponse.cpp \
    $$PWD/src/responses/effects/accountremovedeffectresponse.cpp \
    $$PWD/src/responses/effects/accountthresholdsupdatedeffectresponse.cpp \
    $$PWD/src/responses/effects/offercreatedeffectresponse.cpp \
    $$PWD/src/responses/effects/offerremovedeffectresponse.cpp \
    $$PWD/src/responses/effects/offerupdatedeffectresponse.cpp \
    $$PWD/src/responses/effects/signereffectresponse.cpp \
    $$PWD/src/responses/effects/signercreatedeffectresponse.cpp \
    $$PWD/src/responses/effects/signerremovedeffectresponse.cpp \
    $$PWD/src/responses/effects/signerupdatedeffectresponse.cpp \
    $$PWD/src/responses/effects/tradeeffectresponse.cpp \
    $$PWD/src/responses/effects/trustlineauthorizationresponse.cpp \
    $$PWD/src/responses/effects/trustlineauthorizedeffectresponse.cpp \
    $$PWD/src/responses/effects/trustlinedeauthorizedeffectresponse.cpp \
    $$PWD/src/responses/effects/trustlinecudresponse.cpp \
    $$PWD/src/responses/effects/trustlinecreatedeffectresponse.cpp \
    $$PWD/src/responses/effects/trustlineremovedeffectresponse.cpp \
    $$PWD/src/responses/effects/trustlineupdatedeffectresponse.cpp \
    $$PWD/src/responses/effectspage.cpp \
    $$PWD/src/requests/assetsrequestbuilder.cpp \
    $$PWD/src/responses/assetresponse.cpp \
    $$PWD/src/requests/tradeaggregationsrequestbuilder.cpp \
    $$PWD/src/responses/tradeaggregationresponse.cpp \
    $$PWD/src/responses/effects/accountinflationdestinationupdatedeffectresponse.cpp \
    $$PWD/src/responses/effects/datacreatedeffectresponse.cpp \
    $$PWD/src/responses/effects/dataremovedeffectresponse.cpp \
    $$PWD/src/responses/effects/dataupdatedeffectresponse.cpp \
    $$PWD/src/responses/rootresponse.cpp \
    $$PWD/src/requests/rootrequestbuilder.cpp \
    $$PWD/src/bumpsequenceoperation.cpp \
    $$PWD/src/timebounds.cpp \
    $$PWD/src/slip10.cpp\
    $$PWD/src/checkaccountrequiresmemo.cpp

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

HEADERS += \
    $$PWD/src/abstracttransaction.h \
    $$PWD/src/accountconverter.h \
    $$PWD/src/beginsponsoringfuturereservesoperation.h \
    $$PWD/src/claimant.h \
    $$PWD/src/claimclaimablebalanceoperation.h \
    $$PWD/src/clawbackclaimablebalanceoperation.h \
    $$PWD/src/clawbackoperation.h \
    $$PWD/src/createclaimablebalanceoperation.h \
    $$PWD/src/createpassivesellofferoperation.h \
    $$PWD/src/endsponsoringfuturereservesoperation.h \
    $$PWD/src/feebumptransaction.h \
    $$PWD/src/managebuyofferoperation.h \
    $$PWD/src/managesellofferoperation.h \
    $$PWD/src/pathpaymentstrictreceiveoperation.h \
    $$PWD/src/pathpaymentstrictsendoperation.h \
    $$PWD/src/predicate.h \
    $$PWD/src/requests/claimablebalancesrequestbuilder.h \
    $$PWD/src/requests/feestatsrequestbuilder.h \
    $$PWD/src/requests/strictreceivepathsrequestbuilder.h \
    $$PWD/src/requests/strictsendpathsrequestbuilder.h \
    $$PWD/src/responses/claimablebalanceresponse.h \
    $$PWD/src/responses/effects/accountsponsorshipcreatedeffectresponse.h \
    $$PWD/src/responses/effects/accountsponsorshipremovedeffectresponse.h \
    $$PWD/src/responses/effects/accountsponsorshipupdatedeffectresponse.h \
    $$PWD/src/responses/effects/claimablebalanceclaimantcreatedeffectresponse.h \
    $$PWD/src/responses/effects/claimablebalanceclaimedeffectresponse.h \
    $$PWD/src/responses/effects/claimablebalanceclawedbackeffectresponse.h \
    $$PWD/src/responses/effects/claimablebalancecreatedeffectresponse.h \
    $$PWD/src/responses/effects/claimablebalancesponsorshipcreatedeffectresponse.h \
    $$PWD/src/responses/effects/claimablebalancesponsorshipremovedeffectresponse.h \
    $$PWD/src/responses/effects/claimablebalancesponsorshipupdatedeffectresponse.h \
    $$PWD/src/responses/effects/datasponsorshipcreatedeffectresponse.h \
    $$PWD/src/responses/effects/datasponsorshipremovedeffectresponse.h \
    $$PWD/src/responses/effects/datasponsorshipupdatedeffectresponse.h \
    $$PWD/src/responses/effects/sequencebumpedeffectresponse.h \
    $$PWD/src/responses/effects/signersponsorshipcreatedeffectresponse.h \
    $$PWD/src/responses/effects/signersponsorshipremovedeffectresponse.h \
    $$PWD/src/responses/effects/signersponsorshipupdatedeffectresponse.h \
    $$PWD/src/responses/effects/trustlineauthorizedtomaintainliabilitieseffectresponse.h \
    $$PWD/src/responses/effects/trustlineflagsupdatedeffectresponse.h \
    $$PWD/src/responses/effects/trustlinesponsorshipcreatedeffectresponse.h \
    $$PWD/src/responses/effects/trustlinesponsorshipremovedeffectresponse.h \
    $$PWD/src/responses/effects/trustlinesponsorshipupdatedeffectresponse.h \
    $$PWD/src/responses/feestatsresponse.h \
    $$PWD/src/responses/operations/beginsponsoringfuturereservesoperationresponse.h \
    $$PWD/src/responses/operations/bumpsequenceoperationresponse.h \
    $$PWD/src/responses/operations/claimclaimablebalanceoperationresponse.h \
    $$PWD/src/responses/operations/clawbackclaimablebalanceoperationresponse.h \
    $$PWD/src/responses/operations/clawbackoperationresponse.h \
    $$PWD/src/responses/operations/createclaimablebalanceoperationresponse.h \
    $$PWD/src/responses/operations/endsponsoringfuturereservesoperationresponse.h \
    $$PWD/src/responses/operations/managebuyofferoperationresponse.h \
    $$PWD/src/responses/operations/managesellofferoperationresponse.h \
    $$PWD/src/responses/operations/pathpaymentbaseoperationresponse.h \
    $$PWD/src/responses/operations/pathpaymentstrictreceiveoperationresponse.h \
    $$PWD/src/responses/operations/pathpaymentstrictsendoperationresponse.h \
    $$PWD/src/responses/operations/revokesponsorshipoperationresponse.h \
    $$PWD/src/responses/operations/settrustlineflagsoperationresponse.h \
    $$PWD/src/revokeaccountsponsorshipoperation.h \
    $$PWD/src/revokeclaimablebalancesponsorshipoperation.h \
    $$PWD/src/revokedatasponsorshipoperation.h \
    $$PWD/src/revokeoffersponsorshipoperation.h \
    $$PWD/src/revokesignersponsorshipoperation.h \
    $$PWD/src/revoketrustlinesponsorshipoperation.h \
    $$PWD/src/sep10challenge.h \
    $$PWD/src/settrustlineflagsoperation.h \
    $$PWD/xdr/stellartypes.h \
    $$PWD/src/account.h \
    $$PWD/src/transactionbuilderaccount.h \
    $$PWD/src/accountmergeoperation.h \
    $$PWD/src/operation.h \
    $$PWD/src/keypair.h \
    $$PWD/src/allowtrustoperation.h \
    $$PWD/src/asset.h \
    $$PWD/src/assettypecreditalphanum.h \
    $$PWD/src/assettypecreditalphanum12.h \
    $$PWD/src/assettypecreditalphanum4.h \
    $$PWD/src/assettypenative.h \
    $$PWD/src/changetrustoperation.h \
    $$PWD/src/createaccountoperation.h \
    $$PWD/src/strkey.h \
    $$PWD/src/inflationoperation.h \
    $$PWD/src/util.h \
    $$PWD/src/network.h \
    $$PWD/src/pathpaymentoperation.h \
    $$PWD/src/paymentoperation.h \
    $$PWD/src/price.h \
    $$PWD/src/server.h \
    $$PWD/src/setoptionsoperation.h \
    $$PWD/src/signer.h \
    $$PWD/src/transaction.h \
    $$PWD/xdr/stellartransaction.h \
    $$PWD/xdr/stellarledgerentries.h \
    $$PWD/xdr/xdrhelper.h \
    $$PWD/xdr/stellarledger.h \
    $$PWD/src/memo.h \
    $$PWD/external/ed25519/src/ed25519.h \
    $$PWD/external/ed25519/src/fe.h \
    $$PWD/external/ed25519/src/fixedint.h \
    $$PWD/external/ed25519/src/ge.h \
    $$PWD/external/ed25519/src/precomp_data.h \
    $$PWD/external/ed25519/src/sc.h \
    $$PWD/external/ed25519/src/sha512.h \
    $$PWD/external/cyoencode/src/CyoEncode.h \
    $$PWD/external/cyoencode/src/CyoDecode.h \
    $$PWD/src/requests/effectsrequestbuilder.h \
    $$PWD/src/requests/eventlistener.h \
    $$PWD/src/requests/ledgersrequestbuilder.h \
    $$PWD/src/requests/offersrequestbuilder.h \
    $$PWD/src/requests/operationsrequestbuilder.h \
    $$PWD/src/requests/orderbookrequestbuilder.h \
    $$PWD/src/requests/paymentsrequestbuilder.h \
    $$PWD/src/requests/requestbuilder.h \
    $$PWD/src/requests/transactionsrequestbuilder.h \
    $$PWD/src/requests/accountsrequestbuilder.h \
    $$PWD/src/responses/accountresponse.h \
    $$PWD/src/responses/response.h \
    $$PWD/src/responses/submittransactionresponse.h \
    $$PWD/src/responses/link.h \
    $$PWD/src/responses/transactionresponse.h \
    $$PWD/src/responses/page.h \
    $$PWD/src/responses/operations/operationresponse.h \
    $$PWD/src/responses/operationpage.h \
    $$PWD/src/responses/operations/paymentoperationresponse.h \
    $$PWD/src/responses/operations/createaccountoperationresponse.h \
    $$PWD/src/federation/federationserver.h \
    $$PWD/src/federation/federationresponse.h \
    $$PWD/src/federation/federation.h \
    $$PWD/src/responses/operations/accountmergeoperationresponse.h \
    $$PWD/src/responses/operations/pathpaymentoperationresponse.h \
    $$PWD/src/responses/operations/inflationoperationresponse.h \
    $$PWD/src/responses/operations/allowtrustoperationresponse.h \
    $$PWD/src/responses/operations/changetrustoperationresponse.h \
    $$PWD/src/responses/operations/createpassiveofferoperationresponse.h \
    $$PWD/src/managedataoperation.h \
    $$PWD/src/responses/operations/managedataoperationresponse.h \
    $$PWD/src/responses/operations/setoptionsoperationresponse.h \
    $$PWD/src/requests/tradesrequestbuilder.h \
    $$PWD/src/responses/traderesponse.h \
    $$PWD/src/responses/pathresponse.h \
    $$PWD/src/responses/orderbookresponse.h \
    $$PWD/src/responses/offerresponse.h \
    $$PWD/src/responses/common.h \
    $$PWD/src/responses/ledgerresponse.h \
    $$PWD/src/responses/genericoperation.h \
    $$PWD/src/responses/effects/effectresponse.h \
    $$PWD/src/responses/effects/accountcreatedeffectresponse.h \
    $$PWD/src/responses/effects/accountcreditedeffectresponse.h \
    $$PWD/src/responses/effects/accountdebitedeffectresponse.h \
    $$PWD/src/responses/effects/accountflagsupdatedeffectresponse.h \
    $$PWD/src/responses/effects/accounthomedomainupdatedeffectresponse.h \
    $$PWD/src/responses/effects/accountremovedeffectresponse.h \
    $$PWD/src/responses/effects/accountthresholdsupdatedeffectresponse.h \
    $$PWD/src/responses/effects/offercreatedeffectresponse.h \
    $$PWD/src/responses/effects/offerremovedeffectresponse.h \
    $$PWD/src/responses/effects/offerupdatedeffectresponse.h \
    $$PWD/src/responses/effects/signereffectresponse.h \
    $$PWD/src/responses/effects/signercreatedeffectresponse.h \
    $$PWD/src/responses/effects/signerremovedeffectresponse.h \
    $$PWD/src/responses/effects/signerupdatedeffectresponse.h \
    $$PWD/src/responses/effects/tradeeffectresponse.h \
    $$PWD/src/responses/effects/trustlineauthorizationresponse.h \
    $$PWD/src/responses/effects/trustlineauthorizedeffectresponse.h \
    $$PWD/src/responses/effects/trustlinedeauthorizedeffectresponse.h \
    $$PWD/src/responses/effects/trustlinecudresponse.h \
    $$PWD/src/responses/effects/trustlinecreatedeffectresponse.h \
    $$PWD/src/responses/effects/trustlineremovedeffectresponse.h \
    $$PWD/src/responses/effects/trustlineupdatedeffectresponse.h \
    $$PWD/src/responses/effectspage.h \
    $$PWD/src/requests/assetsrequestbuilder.h \
    $$PWD/src/responses/assetresponse.h \
    $$PWD/src/requests/tradeaggregationsrequestbuilder.h \
    $$PWD/src/responses/tradeaggregationresponse.h \
    $$PWD/src/responses/effects/accountinflationdestinationupdatedeffectresponse.h \
    $$PWD/src/responses/effects/datacreatedeffectresponse.h \
    $$PWD/src/responses/effects/dataremovedeffectresponse.h \
    $$PWD/src/responses/effects/dataupdatedeffectresponse.h \
    $$PWD/src/responses/rootresponse.h \
    $$PWD/src/requests/rootrequestbuilder.h \
    $$PWD/src/bumpsequenceoperation.h \
    $$PWD/src/timebounds.h \
    $$PWD/src/slip10.h \
    $$PWD/src/checkaccountrequiresmemo.h

#to maintain it easier, until we dont support more than read FEDERATION_SERVER address, we dont use CPPTOML
#CPPTOML is not compiling in android caused by some problems in android NDK
#!android{
#    HEADERS+= $$PWD/external/cpptoml/include/cpptoml.h
#    DEFINES+= USE_CPPTOML
#}
