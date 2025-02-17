cmake_minimum_required(VERSION 3.16)
project(StellarQtSDK VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Set up AUTOMOC and some sensible defaults for runtime execution
# When using Qt 6.3, you can replace the code block below with
# qt_standard_project_setup()
set(CMAKE_AUTOMOC ON)
include(GNUInstallDirs)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR}
    REQUIRED
    COMPONENTS Core5Compat Gui Network Test
)

qt_add_library(StellarQtSDK STATIC
    ${HEADERS}
    external/cyoencode/src/CyoDecode.cpp external/cyoencode/src/CyoDecode.h
    external/cyoencode/src/CyoEncode.cpp external/cyoencode/src/CyoEncode.h
    external/ed25519/src/add_scalar.c
    external/ed25519/src/ed25519.h
    external/ed25519/src/ed25519_keypair.c
    external/ed25519/src/fe.c external/ed25519/src/fe.h
    external/ed25519/src/fixedint.h
    external/ed25519/src/ge.c external/ed25519/src/ge.h
    external/ed25519/src/key_exchange.c
    external/ed25519/src/precomp_data.h
    external/ed25519/src/sc.c external/ed25519/src/sc.h
    external/ed25519/src/seed.c
    external/ed25519/src/sha512.c external/ed25519/src/sha512.h
    external/ed25519/src/sign.c
    external/ed25519/src/verify.c
    src/abstracttransaction.cpp
    src/abstracttransaction.h
    src/account.cpp
    src/account.h
    src/accountconverter.cpp
    src/accountconverter.h
    src/accountmergeoperation.cpp
    src/accountmergeoperation.h
    src/allowtrustoperation.cpp
    src/allowtrustoperation.h
    src/asset.cpp
    src/asset.h
    src/assettypecreditalphanum.cpp
    src/assettypecreditalphanum.h
    src/assettypecreditalphanum12.cpp
    src/assettypecreditalphanum12.h
    src/assettypecreditalphanum4.cpp
    src/assettypecreditalphanum4.h
    src/assettypenative.cpp
    src/assettypenative.h
    src/beginsponsoringfuturereservesoperation.cpp
    src/beginsponsoringfuturereservesoperation.h
    src/bumpsequenceoperation.cpp
    src/bumpsequenceoperation.h
    src/changetrustoperation.cpp
    src/changetrustoperation.h
    src/checkaccountrequiresmemo.cpp
    src/checkaccountrequiresmemo.h
    src/claimant.cpp
    src/claimant.h
    src/claimclaimablebalanceoperation.cpp
    src/claimclaimablebalanceoperation.h
    src/clawbackclaimablebalanceoperation.cpp
    src/clawbackclaimablebalanceoperation.h
    src/clawbackoperation.cpp
    src/clawbackoperation.h
    src/createaccountoperation.cpp
    src/createaccountoperation.h
    src/createclaimablebalanceoperation.cpp
    src/createclaimablebalanceoperation.h
    src/createpassivesellofferoperation.cpp
    src/createpassivesellofferoperation.h
    src/endsponsoringfuturereservesoperation.cpp
    src/endsponsoringfuturereservesoperation.h
    src/federation/federation.cpp
    src/federation/federation.h
    src/federation/federationresponse.cpp
    src/federation/federationresponse.h
    src/federation/federationserver.cpp
    src/federation/federationserver.h
    src/feebumptransaction.cpp
    src/feebumptransaction.h
    src/inflationoperation.cpp
    src/inflationoperation.h
    src/keypair.cpp
    src/keypair.h
    src/managebuyofferoperation.cpp
    src/managebuyofferoperation.h
    src/managedataoperation.cpp
    src/managedataoperation.h
    src/managesellofferoperation.cpp
    src/managesellofferoperation.h
    src/memo.cpp
    src/memo.h
    src/network.cpp
    src/network.h
    src/operation.cpp
    src/operation.h
    src/pathpaymentoperation.cpp
    src/pathpaymentoperation.h
    src/pathpaymentstrictreceiveoperation.cpp
    src/pathpaymentstrictreceiveoperation.h
    src/pathpaymentstrictsendoperation.cpp
    src/pathpaymentstrictsendoperation.h
    src/paymentoperation.cpp
    src/paymentoperation.h
    src/predicate.cpp
    src/predicate.h
    src/price.cpp
    src/price.h
    src/requests/accountsrequestbuilder.cpp
    src/requests/accountsrequestbuilder.h
    src/requests/assetsrequestbuilder.cpp
    src/requests/assetsrequestbuilder.h
    src/requests/claimablebalancesrequestbuilder.cpp
    src/requests/claimablebalancesrequestbuilder.h
    src/requests/effectsrequestbuilder.cpp
    src/requests/effectsrequestbuilder.h
    src/requests/eventlistener.cpp
    src/requests/eventlistener.h
    src/requests/feestatsrequestbuilder.cpp
    src/requests/feestatsrequestbuilder.h
    src/requests/ledgersrequestbuilder.cpp
    src/requests/ledgersrequestbuilder.h
    src/requests/offersrequestbuilder.cpp
    src/requests/offersrequestbuilder.h
    src/requests/operationsrequestbuilder.cpp
    src/requests/operationsrequestbuilder.h
    src/requests/orderbookrequestbuilder.cpp
    src/requests/orderbookrequestbuilder.h
    src/requests/paymentsrequestbuilder.cpp
    src/requests/paymentsrequestbuilder.h
    src/requests/requestbuilder.cpp
    src/requests/requestbuilder.h
    src/requests/rootrequestbuilder.cpp
    src/requests/rootrequestbuilder.h
    src/requests/strictreceivepathsrequestbuilder.cpp
    src/requests/strictreceivepathsrequestbuilder.h
    src/requests/strictsendpathsrequestbuilder.cpp
    src/requests/strictsendpathsrequestbuilder.h
    src/requests/tradeaggregationsrequestbuilder.cpp
    src/requests/tradeaggregationsrequestbuilder.h
    src/requests/tradesrequestbuilder.cpp
    src/requests/tradesrequestbuilder.h
    src/requests/transactionsrequestbuilder.cpp
    src/requests/transactionsrequestbuilder.h
    src/responses/accountresponse.cpp
    src/responses/accountresponse.h
    src/responses/assetresponse.cpp
    src/responses/assetresponse.h
    src/responses/claimablebalanceresponse.cpp
    src/responses/claimablebalanceresponse.h
    src/responses/common.h
    src/responses/effects/accountcreatedeffectresponse.cpp
    src/responses/effects/accountcreatedeffectresponse.h
    src/responses/effects/accountcreditedeffectresponse.cpp
    src/responses/effects/accountcreditedeffectresponse.h
    src/responses/effects/accountdebitedeffectresponse.cpp
    src/responses/effects/accountdebitedeffectresponse.h
    src/responses/effects/accountflagsupdatedeffectresponse.cpp
    src/responses/effects/accountflagsupdatedeffectresponse.h
    src/responses/effects/accounthomedomainupdatedeffectresponse.cpp
    src/responses/effects/accounthomedomainupdatedeffectresponse.h
    src/responses/effects/accountinflationdestinationupdatedeffectresponse.cpp
    src/responses/effects/accountinflationdestinationupdatedeffectresponse.h
    src/responses/effects/accountremovedeffectresponse.cpp
    src/responses/effects/accountremovedeffectresponse.h
    src/responses/effects/accountsponsorshipcreatedeffectresponse.cpp
    src/responses/effects/accountsponsorshipcreatedeffectresponse.h
    src/responses/effects/accountsponsorshipremovedeffectresponse.cpp
    src/responses/effects/accountsponsorshipremovedeffectresponse.h
    src/responses/effects/accountsponsorshipupdatedeffectresponse.cpp
    src/responses/effects/accountsponsorshipupdatedeffectresponse.h
    src/responses/effects/accountthresholdsupdatedeffectresponse.cpp
    src/responses/effects/accountthresholdsupdatedeffectresponse.h
    src/responses/effects/claimablebalanceclaimantcreatedeffectresponse.cpp
    src/responses/effects/claimablebalanceclaimantcreatedeffectresponse.h
    src/responses/effects/claimablebalanceclaimedeffectresponse.cpp
    src/responses/effects/claimablebalanceclaimedeffectresponse.h
    src/responses/effects/claimablebalanceclawedbackeffectresponse.cpp
    src/responses/effects/claimablebalanceclawedbackeffectresponse.h
    src/responses/effects/claimablebalancecreatedeffectresponse.cpp
    src/responses/effects/claimablebalancecreatedeffectresponse.h
    src/responses/effects/claimablebalancesponsorshipcreatedeffectresponse.cpp
    src/responses/effects/claimablebalancesponsorshipcreatedeffectresponse.h
    src/responses/effects/claimablebalancesponsorshipremovedeffectresponse.cpp
    src/responses/effects/claimablebalancesponsorshipremovedeffectresponse.h
    src/responses/effects/claimablebalancesponsorshipupdatedeffectresponse.cpp
    src/responses/effects/claimablebalancesponsorshipupdatedeffectresponse.h
    src/responses/effects/datacreatedeffectresponse.cpp
    src/responses/effects/datacreatedeffectresponse.h
    src/responses/effects/dataremovedeffectresponse.cpp
    src/responses/effects/dataremovedeffectresponse.h
    src/responses/effects/datasponsorshipcreatedeffectresponse.cpp
    src/responses/effects/datasponsorshipcreatedeffectresponse.h
    src/responses/effects/datasponsorshipremovedeffectresponse.cpp
    src/responses/effects/datasponsorshipremovedeffectresponse.h
    src/responses/effects/datasponsorshipupdatedeffectresponse.cpp
    src/responses/effects/datasponsorshipupdatedeffectresponse.h
    src/responses/effects/dataupdatedeffectresponse.cpp
    src/responses/effects/dataupdatedeffectresponse.h
    src/responses/effects/effectresponse.cpp
    src/responses/effects/effectresponse.h
    src/responses/effects/offercreatedeffectresponse.cpp
    src/responses/effects/offercreatedeffectresponse.h
    src/responses/effects/offerremovedeffectresponse.cpp
    src/responses/effects/offerremovedeffectresponse.h
    src/responses/effects/offerupdatedeffectresponse.cpp
    src/responses/effects/offerupdatedeffectresponse.h
    src/responses/effects/sequencebumpedeffectresponse.cpp
    src/responses/effects/sequencebumpedeffectresponse.h
    src/responses/effects/signercreatedeffectresponse.cpp
    src/responses/effects/signercreatedeffectresponse.h
    src/responses/effects/signereffectresponse.cpp
    src/responses/effects/signereffectresponse.h
    src/responses/effects/signerremovedeffectresponse.cpp
    src/responses/effects/signerremovedeffectresponse.h
    src/responses/effects/signersponsorshipcreatedeffectresponse.cpp
    src/responses/effects/signersponsorshipcreatedeffectresponse.h
    src/responses/effects/signersponsorshipremovedeffectresponse.cpp
    src/responses/effects/signersponsorshipremovedeffectresponse.h
    src/responses/effects/signersponsorshipupdatedeffectresponse.cpp
    src/responses/effects/signersponsorshipupdatedeffectresponse.h
    src/responses/effects/signerupdatedeffectresponse.cpp
    src/responses/effects/signerupdatedeffectresponse.h
    src/responses/effects/tradeeffectresponse.cpp
    src/responses/effects/tradeeffectresponse.h
    src/responses/effects/trustlineauthorizationresponse.cpp
    src/responses/effects/trustlineauthorizationresponse.h
    src/responses/effects/trustlineauthorizedeffectresponse.cpp
    src/responses/effects/trustlineauthorizedeffectresponse.h
    src/responses/effects/trustlineauthorizedtomaintainliabilitieseffectresponse.cpp
    src/responses/effects/trustlineauthorizedtomaintainliabilitieseffectresponse.h
    src/responses/effects/trustlinecreatedeffectresponse.cpp
    src/responses/effects/trustlinecreatedeffectresponse.h
    src/responses/effects/trustlinecudresponse.cpp
    src/responses/effects/trustlinecudresponse.h
    src/responses/effects/trustlinedeauthorizedeffectresponse.cpp
    src/responses/effects/trustlinedeauthorizedeffectresponse.h
    src/responses/effects/trustlineflagsupdatedeffectresponse.cpp
    src/responses/effects/trustlineflagsupdatedeffectresponse.h
    src/responses/effects/trustlineremovedeffectresponse.cpp
    src/responses/effects/trustlineremovedeffectresponse.h
    src/responses/effects/trustlinesponsorshipcreatedeffectresponse.cpp
    src/responses/effects/trustlinesponsorshipcreatedeffectresponse.h
    src/responses/effects/trustlinesponsorshipremovedeffectresponse.cpp
    src/responses/effects/trustlinesponsorshipremovedeffectresponse.h
    src/responses/effects/trustlinesponsorshipupdatedeffectresponse.cpp
    src/responses/effects/trustlinesponsorshipupdatedeffectresponse.h
    src/responses/effects/trustlineupdatedeffectresponse.cpp
    src/responses/effects/trustlineupdatedeffectresponse.h
    src/responses/effectspage.cpp
    src/responses/effectspage.h
    src/responses/feestatsresponse.cpp
    src/responses/feestatsresponse.h
    src/responses/genericoperation.cpp
    src/responses/genericoperation.h
    src/responses/ledgerresponse.cpp
    src/responses/ledgerresponse.h
    src/responses/link.cpp
    src/responses/link.h
    src/responses/offerresponse.cpp
    src/responses/offerresponse.h
    src/responses/operationpage.cpp
    src/responses/operationpage.h
    src/responses/operations/accountmergeoperationresponse.cpp
    src/responses/operations/accountmergeoperationresponse.h
    src/responses/operations/allowtrustoperationresponse.cpp
    src/responses/operations/allowtrustoperationresponse.h
    src/responses/operations/beginsponsoringfuturereservesoperationresponse.cpp
    src/responses/operations/beginsponsoringfuturereservesoperationresponse.h
    src/responses/operations/bumpsequenceoperationresponse.cpp
    src/responses/operations/bumpsequenceoperationresponse.h
    src/responses/operations/changetrustoperationresponse.cpp
    src/responses/operations/changetrustoperationresponse.h
    src/responses/operations/claimclaimablebalanceoperationresponse.cpp
    src/responses/operations/claimclaimablebalanceoperationresponse.h
    src/responses/operations/clawbackclaimablebalanceoperationresponse.cpp
    src/responses/operations/clawbackclaimablebalanceoperationresponse.h
    src/responses/operations/clawbackoperationresponse.cpp
    src/responses/operations/clawbackoperationresponse.h
    src/responses/operations/createaccountoperationresponse.cpp
    src/responses/operations/createaccountoperationresponse.h
    src/responses/operations/createclaimablebalanceoperationresponse.cpp
    src/responses/operations/createclaimablebalanceoperationresponse.h
    src/responses/operations/createpassiveofferoperationresponse.cpp
    src/responses/operations/createpassiveofferoperationresponse.h
    src/responses/operations/endsponsoringfuturereservesoperationresponse.cpp
    src/responses/operations/endsponsoringfuturereservesoperationresponse.h
    src/responses/operations/inflationoperationresponse.cpp
    src/responses/operations/inflationoperationresponse.h
    src/responses/operations/managebuyofferoperationresponse.cpp
    src/responses/operations/managebuyofferoperationresponse.h
    src/responses/operations/managedataoperationresponse.cpp
    src/responses/operations/managedataoperationresponse.h
    src/responses/operations/managesellofferoperationresponse.cpp
    src/responses/operations/managesellofferoperationresponse.h
    src/responses/operations/operationresponse.cpp
    src/responses/operations/operationresponse.h
    src/responses/operations/pathpaymentbaseoperationresponse.cpp
    src/responses/operations/pathpaymentbaseoperationresponse.h
    src/responses/operations/pathpaymentoperationresponse.cpp
    src/responses/operations/pathpaymentoperationresponse.h
    src/responses/operations/pathpaymentstrictreceiveoperationresponse.cpp
    src/responses/operations/pathpaymentstrictreceiveoperationresponse.h
    src/responses/operations/pathpaymentstrictsendoperationresponse.cpp
    src/responses/operations/pathpaymentstrictsendoperationresponse.h
    src/responses/operations/paymentoperationresponse.cpp
    src/responses/operations/paymentoperationresponse.h
    src/responses/operations/revokesponsorshipoperationresponse.cpp
    src/responses/operations/revokesponsorshipoperationresponse.h
    src/responses/operations/setoptionsoperationresponse.cpp
    src/responses/operations/setoptionsoperationresponse.h
    src/responses/operations/settrustlineflagsoperationresponse.cpp
    src/responses/operations/settrustlineflagsoperationresponse.h
    src/responses/orderbookresponse.cpp
    src/responses/orderbookresponse.h
    src/responses/page.cpp
    src/responses/page.h
    src/responses/pathresponse.cpp
    src/responses/pathresponse.h
    src/responses/response.cpp
    src/responses/response.h
    src/responses/rootresponse.cpp
    src/responses/rootresponse.h
    src/responses/submittransactionresponse.cpp
    src/responses/submittransactionresponse.h
    src/responses/tradeaggregationresponse.cpp
    src/responses/tradeaggregationresponse.h
    src/responses/traderesponse.cpp
    src/responses/traderesponse.h
    src/responses/transactionresponse.cpp
    src/responses/transactionresponse.h
    src/revokeaccountsponsorshipoperation.cpp
    src/revokeaccountsponsorshipoperation.h
    src/revokeclaimablebalancesponsorshipoperation.cpp
    src/revokeclaimablebalancesponsorshipoperation.h
    src/revokedatasponsorshipoperation.cpp
    src/revokedatasponsorshipoperation.h
    src/revokeoffersponsorshipoperation.cpp
    src/revokeoffersponsorshipoperation.h
    src/revokesignersponsorshipoperation.cpp
    src/revokesignersponsorshipoperation.h
    src/revoketrustlinesponsorshipoperation.cpp
    src/revoketrustlinesponsorshipoperation.h
    src/sep10challenge.cpp
    src/sep10challenge.h
    src/server.cpp
    src/server.h
    src/setoptionsoperation.cpp
    src/setoptionsoperation.h
    src/settrustlineflagsoperation.cpp
    src/settrustlineflagsoperation.h
    src/signer.cpp
    src/signer.h
    src/slip10.cpp
    src/slip10.h
    src/strkey.cpp
    src/strkey.h
    src/timebounds.cpp
    src/timebounds.h
    src/transaction.cpp
    src/transaction.h
    src/transactionbuilderaccount.h
    src/util.cpp
    src/util.h
    xdr/stellarledger.h
    xdr/stellarledgerentries.h
    xdr/stellartransaction.cpp xdr/stellartransaction.h
    xdr/stellartypes.h
    xdr/xdrhelper.cpp xdr/xdrhelper.h
)
target_include_directories(StellarQtSDK PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} src)

add_compile_definitions(
    STELLAR_QT_SDK_CLIENT_NAME=\"\\\"qtcpp-stellar-sdk\\\"\"
    STELLAR_QT_SDK_CLIENT_VERSION=\"\\\"0.3.17\\\"\"
)

target_compile_definitions(StellarQtSDK PRIVATE
    ED25519_NO_SEED
    QT_DEPRECATED_WARNINGS
    STELLAR_QT_REPLY_TIMEOUT=30000
)

target_link_libraries(StellarQtSDK PRIVATE Qt::Core Qt::Gui Qt::Network)

install(DIRECTORY src/ DESTINATION include     FILES_MATCHING PATTERN "*.h")
install(DIRECTORY xdr/ DESTINATION include/xdr FILES_MATCHING PATTERN "*.h")
install(TARGETS StellarQtSDK
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

enable_testing()
qt_add_executable(StellarQtSDKTest EXCLUDE_FROM_ALL
    test/fakeserver.h
    test/feebumptransactiontest.h
    test/keypairtest.h
    test/requests/claimablebalancesrequestbuildertest.h
    test/responses/claimablebalancepagedeserializertest.h
    test/responses/feestatsdeserializertest.h
    test/sep10challengetest.h
    test/strkeytest.h
    test/testcollector.h
    test/operationtest.h
    test/transactiontest.h
    test/requests/accountsrequestbuildertest.h
    test/servertest.h
    test/responses/submittransactionresponsetest.h
    test/networktest.h
    test/accounttest.h
    test/responses/accountdeserializertest.h
    test/responses/transactionpagedeserializertest.h
    test/responses/operationspagetest.h
    test/requests/transactionsrequestbuildertest.h
    test/requests/paymentsrequestbuildertest.h
    test/federation/federationtest.h
    test/memotest.h
    test/responses/operationdeserializertest.h
    test/pricetest.h
    test/requests/tradesrequestbuildertest.h
    test/responses/tradespagedeserializertest.h
    test/responses/pathspagedeserializertest.h
    test/requests/pathsrequestbuildertest.h
    test/responses/orderbookdeserializertest.h
    test/requests/orderbookrequestbuildertest.h
    test/responses/offerpagedeserializertest.h
    test/requests/offersrequestbuildertest.h
    test/responses/ledgerdeserializertest.h
    test/responses/ledgerpagedeserializertest.h
    test/requests/ledgersrequestbuildertest.h
    test/responses/accountspagedeserializertest.h
    test/requests/operationsrequestbuildertest.h
    test/requests/effectsrequestbuildertest.h
    test/responses/effectdeserializertest.h
    test/responses/effectspagedeserializertest.h
    test/assettest.h
    test/accountflagtest.h
    test/responses/assetdeserializertest.h
    test/responses/transactiondeserializertest.h
    test/requests/assetsrequestbuildertest.h
    test/responses/assetspagedeserializertest.h
    test/requests/tradeaggregationsrequestbuildertest.h
    test/responses/tradeaggregationspagedeserializertest.h
    test/responses/rootdeserializertest.h
    test/requests/rootrequestbuildtest.h
    test/requests/operationfeerequestbuildertest.h
    test/sep0005keypairforaccountfrombip39seedtest.h
    test/slip0010ed25519privatekeytest.h
    test/utiltest.h
    test/main.cpp
)
target_compile_definitions(StellarQtSDKTest PRIVATE
    STELLAR_ALLOW_UNSECURE_RANDOM
    STELLAR_ENABLE_TEST_METHODS
    STELLAR_SKIP_LIVE_TESTS
)

if((QT_VERSION_MAJOR GREATER 5))
    target_compile_definitions(StellarQtSDK PRIVATE
        QIODeviceEnums=QIODeviceBase
    )
endif()

if((QT_VERSION_MAJOR LESS 6))
    target_compile_definitions(StellarQtSDK PRIVATE
        QIODeviceEnums=QIODevice
    )
endif()

target_include_directories(StellarQtSDKTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    src
)
target_link_libraries(StellarQtSDKTest PRIVATE
    Qt::Network Qt::Test StellarQtSDK
)
add_test(NAME StellarQtSDKTest COMMAND StellarQtSDKTest)
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} DEPENDS StellarQtSDKTest)
