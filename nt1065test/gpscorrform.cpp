#include "gpscorrform.h"
#include "ui_gpscorrform.h"

GPSCorrForm::GPSCorrForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GPSCorrForm),
    working( false ),
    running( true )
{
    ui->setupUi(this);


    QObject::connect(this, SIGNAL(satInfo(int,float,bool)), this, SLOT(satChanged(int,float,bool)) );

    plotCorrAll = ui->widgetCorrAll;

    gr_vis = plotCorrAll->addGraph();
    gr_vis->setPen(QPen(Qt::green));
    gr_vis->setLineStyle( (QCPGraph::LineStyle) QCPGraph::lsImpulse);

    gr_inv = plotCorrAll->addGraph();
    gr_inv->setPen(QPen(Qt::red));
    gr_inv->setLineStyle( (QCPGraph::LineStyle) QCPGraph::lsImpulse);

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
        if ( sv.FindMaxCorr( freq, tshift, corrval ) ) {
            emit satInfo( PRN_IN_OPER[ si ], corrval, true );
            sv.PreciseFreq( freq, tshift, corrval );
            emit satInfo( PRN_IN_OPER[ si ], corrval, true );

//            file_dump( NULL, 0, "xcorr", "flt", PRN_IN_OPER[ si ], true );
//            std::vector< std::vector<float> > cm;
//            std::vector<double> freqs;
//            sv.GetCorrMatrix( cm, freqs);
//            for ( unsigned int fi = 0; fi < cm.size(); fi++ ) {
//                file_dump( &cm[ fi ][ 0 ], cm[ fi ].size()*4, "xcorr", "flt", PRN_IN_OPER[ si ], true );
//            }
//            file_dump( &freqs[ 0 ], freqs.size()*8, "freqs", "flt", PRN_IN_OPER[ si ], false );
//            fprintf( stderr, "[%2d] size %d x %d\n", PRN_IN_OPER[ si ], cm.size(), cm[0].size() );

        } else {
            emit satInfo( PRN_IN_OPER[ si ], corrval, false );
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

void GPSCorrForm::satChanged(int prn, float corr, bool is_visible) {
    if ( is_visible ) {
        visible_corrs[ prn ]   = corr;
        invisible_corrs[ prn ] = 0;
    } else {
        visible_corrs[ prn ]   = 0;
        invisible_corrs[ prn ] = corr;
    }
    redrawVisGraph();
}
