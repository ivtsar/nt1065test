#-------------------------------------------------
#
# Project created by QtCreator 2015-01-17T17:02:23
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = nt1065test
TEMPLATE = app

#CONFIG += QWT
#    CyAPIProc.cpp

LIBS += ..\nt1065test\library\lib\x86\CyAPI.lib
LIBS += ..\libfftw3f-3.lib
LIBS += /NODEFAULTLIB:LIBCMT
LIBS += "C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Lib\User32.lib"
LIBS += "C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Lib\setupapi.lib"

SOURCES += main.cpp\
        mainwindow.cpp \
        qcustomplot.cpp \
    cy3thread.cpp

#    CyAPIProc.cpp \
#    cythread.cpp \


HEADERS  += mainwindow.h \
    qcustomplot.h \
    cy3thread.h

#    CyAPIProc.h \
#    cythread.h \

FORMS    += mainwindow.ui
