#-------------------------------------------------
#
# Project created by QtCreator 2015-12-09T17:49:19
#
#-------------------------------------------------

QT       += core gui network
CONFIG   += static

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = e-ProtocolNotification
TEMPLATE = app


SOURCES += main.cpp\
        epn_dialog.cpp \
    popup.cpp \
    filedownloader.cpp

HEADERS  += epn_dialog.h \
    popup.h \
    version.h \
    filedownloader.h

FORMS    += epn_dialog.ui \
    popup.ui

DISTFILES +=

RESOURCES += \
    epnresources.qrc
