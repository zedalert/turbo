#-------------------------------------------------
#
# Project created by QtCreator 2012-07-19T00:43:42
#
#-------------------------------------------------

QT       += core gui network

TARGET = turbo
TEMPLATE = app


SOURCES += main.cpp\
    turbo.cpp \
    settings.cpp \
    warning.cpp \
    about.cpp \
    critical.cpp

HEADERS  += turbo.h \
    settings.h \
    warning.h \
    HoverButton.h \
    version.h \
    about.h \
    critical.h

FORMS    += turbo.ui \
    settings.ui \
    warning.ui \
    about.ui \
    critical.ui

RESOURCES = images.qrc
RC_FILE = turbo.rc

OTHER_FILES += \
    turbo.rc
