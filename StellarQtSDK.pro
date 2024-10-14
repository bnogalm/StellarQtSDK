#example to add the Stellar Qt SDK to your project. It will not link correctly because it doesn't have a main().

QT += core
QT -= gui

CONFIG += c++11

TARGET = StellarQtSDK
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

include(StellarQtSDK.pri)

