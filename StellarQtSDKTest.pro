QT += core testlib
QT -= gui

CONFIG += c++11 testcase

DEFINES += STELLAR_ALLOW_UNSECURE_RANDOM
#DEFINES += STELLAR_SKIP_LIVE_TESTS

DEFINES += STELLAR_ENABLE_TEST_METHODS
#DEFINES += STELLAR_QT_DEBUG_NETWORK_REQUESTS

TARGET = StellarQtSDKTest
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

include(StellarQtSDK.pri)



HEADERS += \
    test/claimablebalanceidtest.h \
    test/fakeserver.h \
    test/feebumptransactiontest.h \
    test/keypairtest.h \
    test/requests/claimablebalancesrequestbuildertest.h \
    test/responses/claimablebalancepagedeserializertest.h \
    test/responses/feestatsdeserializertest.h \
    test/sep10challengetest.h \
    test/strkeytest.h \
    test/testcollector.h \
    test/operationtest.h \
    test/transactiontest.h \
    test/requests/accountsrequestbuildertest.h \
    test/servertest.h \
    test/responses/submittransactionresponsetest.h \
    test/networktest.h \
    test/accounttest.h \
    test/responses/accountdeserializertest.h \
    test/responses/transactionpagedeserializertest.h \
    test/responses/operationspagetest.h \
    test/requests/transactionsrequestbuildertest.h \
    test/requests/paymentsrequestbuildertest.h \
    test/federation/federationtest.h \
    test/memotest.h \
    test/responses/operationdeserializertest.h \
    test/pricetest.h \
    test/requests/tradesrequestbuildertest.h \
    test/responses/tradespagedeserializertest.h \
    test/responses/pathspagedeserializertest.h \
    test/requests/pathsrequestbuildertest.h \
    test/responses/orderbookdeserializertest.h \
    test/requests/orderbookrequestbuildertest.h \
    test/responses/offerpagedeserializertest.h \
    test/requests/offersrequestbuildertest.h \
    test/responses/ledgerdeserializertest.h \
    test/responses/ledgerpagedeserializertest.h \
    test/requests/ledgersrequestbuildertest.h \
    test/responses/accountspagedeserializertest.h \
    test/requests/operationsrequestbuildertest.h \
    test/requests/effectsrequestbuildertest.h \
    test/responses/effectdeserializertest.h \
    test/responses/effectspagedeserializertest.h \
    test/assettest.h \
    test/accountflagtest.h \
    test/responses/assetdeserializertest.h \
    test/responses/transactiondeserializertest.h \
    test/requests/assetsrequestbuildertest.h \
    test/responses/assetspagedeserializertest.h \
    test/requests/tradeaggregationsrequestbuildertest.h \
    test/responses/tradeaggregationspagedeserializertest.h \
    test/responses/rootdeserializertest.h \
    test/requests/rootrequestbuildtest.h \
    test/requests/operationfeerequestbuildertest.h \
    test/sep0005keypairforaccountfrombip39seedtest.h \
    test/slip0010ed25519privatekeytest.h \    
    test/utiltest.h

SOURCES += \
    test/main.cpp
