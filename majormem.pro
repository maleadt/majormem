#-------------------------------------------------
#
# Project created by QtCreator 2014-09-02T21:21:39
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = majormem
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp

CONFIG += c++11

QMAKE_CXXFLAGS += -fopenmp
LIBS += -fopenmp

QMAKE_CXXFLAGS += -Wall -Werror
