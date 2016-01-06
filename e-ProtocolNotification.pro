#-------------------------------------------------
#
# Project created by QtCreator 2015-12-09T17:49:19
#
#-------------------------------------------------

QT       += core gui
CONFIG   += static

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = e-ProtocolNotification
TEMPLATE = app


SOURCES += main.cpp\
        epn_dialog.cpp \
    popup.cpp

HEADERS  += epn_dialog.h \
    popup.h

FORMS    += epn_dialog.ui \
    popup.ui
