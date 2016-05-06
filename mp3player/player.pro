#-------------------------------------------------
#
# Project created by QtCreator 2016-02-20T11:20:35
#
#-------------------------------------------------

QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = player
TEMPLATE = app
CONFIG += c++11


SOURCES += main.cpp\
        widget.cpp

HEADERS  += widget.h

FORMS    += widget.ui

RESOURCES += \
    icon.qrc
