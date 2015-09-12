TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

QMAKE_CXXFLAGS += -std=c++11
DEFINES += _GLIBCXX_DEBUG

include(deployment.pri)
qtcAddDeployment()

HEADERS += \
    bimap.h

