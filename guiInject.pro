QT += widgets network concurrent xml

TARGET = guiInject
TEMPLATE = lib

INCLUDEPATH += libmaia
LIBS += libmaia/libmaia.a

DEFINES += GUIINJECT_LIBRARY

SOURCES += guiinject.cpp \
    startuphelper.cpp \
    directpick.cpp

HEADERS += guiinject.h\
    guiinject_global.h \
    startuphelper.h \
    directpick.h

greaterThan(QT_MAJOR_VERSION, 4){
    CONFIG += c++11
    QT       -= gui
} else {
    QMAKE_CXXFLAGS += -std=c++0x

    OBJECTS_DIR = .tmpinj
    MOC_DIR = .tmpinj
    RCC_DIR = .tmpinj
}

sunix {
    target.path = /usr/lib
    INSTALLS += target
}

