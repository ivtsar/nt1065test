#ifndef CYTHREAD_H
#define CYTHREAD_H

#include <QThread>
#include <windows.h>
#include "CyAPIProc.h"

class CyThread: public QThread
{
//public:
    Q_OBJECT
    void run();
    void CyThread::DataProcessor(char* Data, int size);

signals:
    void resultReady( const float* spc);
};

#endif // CYTHREAD_H
