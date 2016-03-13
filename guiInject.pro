#-------------------------------------------------
#
# Project created by QtCreator 2016-02-25T20:39:31
#
#-------------------------------------------------

QT       += widgets network concurrent xml

QT       -= gui

TARGET = guiInject
TEMPLATE = lib

INCLUDEPATH += libmaia
LIBS += libmaia/libmaia.a

DEFINES += GUIINJECT_LIBRARY

SOURCES += guiinject.cpp \
    startuphelper.cpp

HEADERS += guiinject.h\
        guiinject_global.h \
    startuphelper.h

CONFIG += c++11

sunix {
    target.path = /usr/lib
    INSTALLS += target
}
