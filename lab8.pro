QT += core
QT -= gui

TARGET = lab8
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    server.cpp

HEADERS += \
    server.h

LIBS += -lprogbase -lssl
