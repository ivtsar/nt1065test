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
    cy3thread.cpp \
    gpscorrform.cpp \
    gcacorr/dsp_utils.cpp \
    gcacorr/fftwrapper.cpp \
    gcacorr/gpsvis.cpp \
    gcacorr/matrixstatistic.cpp \
    gcacorr/rawsignal.cpp

#    CyAPIProc.cpp \
#    cythread.cpp \


HEADERS  += mainwindow.h \
    qcustomplot.h \
    cy3thread.h \
    gpscorrform.h \
    gcacorr/cas_codes.h \
    gcacorr/dsp_utils.h \
    gcacorr/fftwrapper.h \
    gcacorr/fir_filter.h \
    gcacorr/gpsvis.h \
    gcacorr/mathTypes.h \
    gcacorr/matrixstatistic.h \
    gcacorr/rawsignal.h \
    gcacorr/stattype.h

#    CyAPIProc.h \
#    cythread.h \

FORMS    += mainwindow.ui \
    gpscorrform.ui
