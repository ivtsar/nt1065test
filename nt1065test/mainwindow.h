#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "QCustomPlot.h"
#include "gpscorrform.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    GPSCorrForm* gpsform;

private:
    Ui::MainWindow *ui;
    QCustomPlot* customPlot;
    QVector<double>* x;
    int n;
    QVector<double>* expAved;
    int expAve;
    int expAveN;
    double expAveC;
    int firstExpAve;

private slots:
    void onShowGPS(bool);

public slots:
    void getSpc(const QVector<double>* spc, int cn);
};

#endif // MAINWINDOW_H
