#include "mainwindow.h"
#include <QApplication>
#include <QtGlobal>
#include <qt_windows.h>
#include "Cy3Thread.h"
#include "qcustomplot.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    CyThread* cythrd = new CyThread();
    w.connect(cythrd, SIGNAL(resultReady(const QVector<double>*, int)), &w, SLOT(getSpc(const QVector<double>*, int)));
    w.show();
    cythrd->start();
    return a.exec();
}
