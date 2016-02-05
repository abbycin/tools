#-------------------------------------------------
#
# Project created by QtCreator 2016-02-04T18:30:24
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = calculator
TEMPLATE = app
CONFIG += c++11

SOURCES += main.cpp\
        widget.cpp

HEADERS  += widget.h rpn.h

FORMS    += widget.ui
