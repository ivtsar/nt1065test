#include "gpscorrform.h"
#include "ui_gpscorrform.h"

GPSCorrForm::GPSCorrForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GPSCorrForm),
    working( false ),
    running( true )
{
    ui->setupUi(this);


    QObject::connect(this, SIGNAL(satInfo(int,float,int,double,bool)),
                     this, SLOT(satChanged(int,float,int,double,bool)) );

    QObject::connect(ui->tableRes, SIGNAL( cellDoubleClicked (int, int) ),
                     this, SLOT( cellSelected( int, int ) ) );

    plotCorrAll   = ui->widgetCorrAll;
    plotCorrGraph = ui->widgetCorrGraph;

    gr_vis = plotCorrAll->addGraph();
    gr_vis->setPen(QPen(Qt::green));
    gr_vis->setLineStyle( (QCPGraph::LineStyle) QCPGraph::lsImpulse);

    gr_inv = plotCorrAll->addGraph();
    gr_inv->setPen(QPen(Qt::red));
    gr_inv->setLineStyle( (QCPGraph::LineStyle) QCPGraph::lsImpulse);

    cdata.resize(PRN_MAX+1);

    visible_sats.resize( PRN_MAX );
    visible_corrs.resize( PRN_MAX );

    invisible_sats.resize( PRN_MAX );
    invisible_corrs.resize( PRN_MAX );

    for ( int i = 0; i < PRN_MAX; i++ ) {
        invisible_sats[ i ] = i;
        invisible_corrs[ i ] = 1000.0;

        visible_sats[ i ] = i;
        visible_corrs[ i ] = 0.0;
    }

    gr_vis->setData( visible_sats, visible_corrs );
    gr_vis->rescaleAxes(true);

    gr_inv->setData( invisible_sats, invisible_corrs );
    gr_inv->rescaleAxes(true);

    plotCorrAll->yAxis->setRange( 0, 38000 );

    ui->tableRes->setRowCount( PRN_MAX );
    ui->tableRes->setColumnCount( 4 );

    QStringList heads;
    heads << "Stat" << "Freq" << "Shift" << "Val";
    ui->tableRes->setHorizontalHeaderLabels( heads );
    ui->tableRes->setColumnWidth( 0, 40 );
    ui->tableRes->setColumnWidth( 1, 40 );
    ui->tableRes->setColumnWidth( 2, 40 );
    ui->tableRes->setColumnWidth( 3, 40 );

    for ( int i = 0; i < PRN_MAX; ++i ) {
        ui->tableRes->setRowHeight( i, 18 );
    }

    set_tmp_dir( "M:\\tmp" );

    calc_thread = std::thread(&GPSCorrForm::calcLoop, this);
}

GPSCorrForm::~GPSCorrForm()
{
    delete ui;
    qDebug( "GPSCorrForm::~GPSCorrForm() will wait for thread\n" );
    running = false;
    if ( calc_thread.joinable() ) {
        calc_thread.join();
    }
    qDebug( "GPSCorrForm::~GPSCorrForm() finished!\n" );
}

void GPSCorrForm::calcSats()
{

    for ( int si = 0; si < PRN_CNT; si++ ) {
        double freq;
        int tshift;
        float corrval;

        GPSVis sv( PRN_IN_OPER[ si ], 7000.0, 1000.0, RCV_SR, GPS_FREQ_IN_RCV );
        sv.SetSignal( &sigs );
        sv.CalcCorrMatrix();

        plot_data_t& p = cdata[ PRN_IN_OPER[ si ] ];
        if ( sv.FindMaxCorr( freq, tshift, corrval ) ) {
            emit satInfo( PRN_IN_OPER[ si ], corrval, tshift, freq, true );

            sv.PreciseFreq( freq, tshift, corrval );
            emit satInfo( PRN_IN_OPER[ si ], corrval, tshift, freq, true );

            p.mutex->lock();
            sv.GetCorrMatrix( p.cors, p.freqs_vals );
            p.center = tshift;
            p.inited = true;
            p.mutex->unlock();

            //file_dump( NULL, 0, "xcorr", "flt", PRN_IN_OPER[ si ], true );
            //for ( unsigned int fi = 0; fi < cm.size(); fi++ ) {
            //    file_dump( &cm[ fi ][ 0 ], cm[ fi ].size()*4, "xcorr", "flt", PRN_IN_OPER[ si ], true );
            //}
            //file_dump( &freqs[ 0 ], freqs.size()*8, "freqs", "flt", PRN_IN_OPER[ si ], false );
            //fprintf( stderr, "[%2d] size %d x %d\n", PRN_IN_OPER[ si ], cm.size(), cm[0].size() );

        } else {
            p.mutex->lock();

            //p.inited = false;

            sv.GetCorrMatrix( p.cors, p.freqs_vals );
            p.center = tshift;
            p.inited = true;

            p.mutex->unlock();
            emit satInfo( PRN_IN_OPER[ si ], corrval, tshift, freq, false );
        }

        if ( !running ) {
            break;
        }
    }

    for ( uint32_t i = 0; i < sigs.size(); i++ ) {
        delete sigs[ i ];
    }
    sigs.clear();
}

void GPSCorrForm::processRawData(const std::vector<short> *data) {
    if ( working || !ui->checkRefresh->isChecked() ) {
        delete data;
        return;
    } else {
        sigs.resize( 8 );

        for ( uint32_t i = 0; i < sigs.size(); i++ ) {
            sigs[ i ] = new RawSignal( DATA_SIZE, RCV_SR );
            sigs[ i ]->LoadData( (void*) &( data->at( i ) ), ADC_DATA_TYPE, i*DATA_SIZE );
        }

        delete data;
        working = true;
    }
}

void GPSCorrForm::calcLoop() {
    qDebug( "GPSCorrForm::calcLoop() STARTED\n" );
    while ( running ) {
        Sleep( 500 );
        if ( working ) {
            calcSats();
            working = false;
        }
    }
    qDebug( "GPSCorrForm::calcLoop() FINISH\n" );
}

void GPSCorrForm::redrawVisGraph() {
    gr_vis->setData( visible_sats, visible_corrs );
    gr_vis->rescaleAxes(true);

    gr_inv->setData( invisible_sats, invisible_corrs );
    gr_inv->rescaleAxes(true);

    plotCorrAll->replot();
}

QTableWidgetItem* MakeTableItem( QString& str, bool grey ) {
    QTableWidgetItem* item = new QTableWidgetItem( str );
    item->setTextAlignment( Qt::AlignRight );
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    if ( grey ) {
        item->setForeground(QColor::fromRgb(128,128,128));
    }
    return item;
}

void GPSCorrForm::satChanged(int prn, float corr, int shift, double freq, bool is_visible) {

    int tidx = prn - 1;


    if ( is_visible ) {
        visible_corrs[ prn ]   = corr;
        invisible_corrs[ prn ] = 0;

        ui->tableRes->setItem( tidx, 0, MakeTableItem( QString("VIS"), !is_visible ) );
    } else {
        visible_corrs[ prn ]   = 0;
        invisible_corrs[ prn ] = corr;

        ui->tableRes->setItem( tidx, 0, MakeTableItem(QString("-"), !is_visible ) );
    }

    ui->tableRes->setItem( tidx, 1, MakeTableItem( QString::number( freq, 'f', 0  ), !is_visible ) );
    ui->tableRes->setItem( tidx, 2, MakeTableItem( QString::number( shift ), !is_visible ) );
    ui->tableRes->setItem( tidx, 3, MakeTableItem( QString::number( corr, 'f', 0 ), !is_visible ) );

    redrawVisGraph();
}

void GPSCorrForm::cellSelected(int x, int) {
    int idx = x + 1;
    plot_data_t& p = cdata[ idx ];
    p.mutex->lock();
    qDebug( "PRN %d, init = %d", idx, p.inited );

    plotCorrGraph->clearGraphs();
    if ( p.inited ) {
        int N = p.cors[ 0 ].size();

        QVector< double > times;
        times.resize( N );

        for ( int i = 0; i < N; i++ ) {
            times[ i ] = p.center - N/2 + i;
        }

        for ( int i = 0; i < p.cors.size(); i++ ) {
            QCPGraph* g = plotCorrGraph->addGraph();

            QVector< double > vals;
            std::vector<float> src = p.cors[ i ];
            vals.resize( src.size() );

            for ( int i = 0; i < src.size(); i++ ) {
                vals[ i ] = src[ i ];
            }

            g->setData( times, vals );
            g->rescaleAxes( true );
        }
        plotCorrGraph->xAxis->setRange(p.center - N/2, p.center + N/2);
        plotCorrGraph->yAxis->setRange(0, 35000);
    }
    p.mutex->unlock();
    plotCorrGraph->replot();
}
