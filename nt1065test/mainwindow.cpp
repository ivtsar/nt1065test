#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    n = 5300;
    expAve = 1;
    ui->setupUi(this);
    customPlot = ui->widget;
    customPlot->addGraph();
    customPlot->graph(0)->setPen(QPen(Qt::blue));
    customPlot->addGraph();
    customPlot->graph(1)->setPen(QPen(Qt::red));
    customPlot->addGraph();
    customPlot->graph(2)->setPen(QPen(Qt::black));
    customPlot->addGraph();
    customPlot->graph(3)->setPen(QPen(Qt::green));
    x = new QVector<double>(n/2);
    for (int i = 0; i < n/2; i++)
        (*x)[i] = (double)i;
    customPlot->xAxis->setRange(0, n/2-1);
    customPlot->yAxis->setRange(0, 100);
    expAved = new QVector<double>(n/2);
    expAved->fill(0);
    expAveN = 10;
    expAveC = 1.0/(double)expAveN;
    firstExpAve = 1;

    QObject::connect(ui->btnShowGCAC, SIGNAL(clicked(bool)), this, SLOT(onShowGPS(bool)) );

}

void MainWindow::getSpc(const QVector<double>* spc, int cn)
{
    if ( !ui->checkSpectrum->isChecked() ) {
        return;
    }

    if (expAve)
    {
        if (firstExpAve){
            firstExpAve = 0;
            for ( int i = 0; i < n/2; i++)
                (*expAved)[i] = (*spc)[i];
        }else{
            for ( int i = 0; i < n/2; i++)
            {
                (*expAved)[i] -= (*expAved)[i]*expAveC;
                (*expAved)[i] += (*spc)[i]*expAveC;
            }
        }
    }
    customPlot->graph(cn-1)->setData(*x, *expAved);//spc);
    //if ( cn == 1)
        customPlot->replot();
}

MainWindow::~MainWindow()
{
    delete expAved;
    delete x;
    delete ui;
}

void MainWindow::onShowGPS(bool)
{
    this->gpsform->setVisible( true );
}
