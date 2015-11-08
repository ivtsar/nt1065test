#include "mainwindow.h"
#include <QApplication>
#include <QtGlobal>
#include <qt_windows.h>
#include "Cy3Thread.h"
#include "qcustomplot.h"

int main(int argc, char *argv[])
{
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
    QApplication a(argc, argv);
    MainWindow w;
    GPSCorrForm gpsform;
    w.gpsform = &gpsform;
    CyThread* cythrd = new CyThread();

    QObject::connect(cythrd, SIGNAL(resultReady(const QVector<double>*, int)), &w, SLOT(getSpc(const QVector<double>*, int)));
    QObject::connect(cythrd, SIGNAL(adcData(const std::vector<short>*)), &gpsform, SLOT(processRawData(const std::vector<short>*)));

    w.show();
    cythrd->start();
    return a.exec();
}
