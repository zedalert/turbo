#-------------------------------------------------
#
# Project created by QtCreator 2012-07-19T00:43:42
#
#-------------------------------------------------

QT += core gui network

TARGET = turbo
TEMPLATE = app


SOURCES += main.cpp\
    turbo.cpp \
    settings.cpp \
    about.cpp \
    sslerror.cpp \
    ask.cpp

HEADERS += turbo.h \
    settings.h \
    about.h \
    version.h \
    hoverbutton.h \
    sslerror.h \
    ask.h

FORMS += turbo.ui \
    settings.ui \
    about.ui \
    sslerror.ui \
    ask.ui

RESOURCES = images.qrc
RC_FILE = turbo.rc

OTHER_FILES +=

