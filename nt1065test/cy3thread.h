#ifndef CYTHREAD_H
#define CYTHREAD_H

#include <QThread>
#include <windows.h>
//#include "CyAPIProc.h"
#include "library\inc\CyAPI.h"
#include <vector>

#define MAX_QUEUE_SZ  64
#define VENDOR_ID  0x04B4
#define PRODUCT_ID  0x00F1

typedef void (__stdcall * DataProcessorFunc)(char*, int);//data pointer, data size (bytes);

struct EndPointParams{
    int Attr;
    bool In;
    int MaxPktSize;
    int MaxBurst;
    int Interface;
    int Address;
};

struct StartDataTransferParams{
    CCyFX3Device	*USBDevice;
    CCyUSBEndPoint  *EndPt;
    int				PPX;
    int				QueueSize;
    int				TimeOut;
    bool			bHighSpeedDevice;
    bool			bSuperSpeedDevice;
    bool			bStreaming;
    bool			ThreadAlreadyStopped;
    DataProcessorFunc DataProc;
    EndPointParams  CurEndPoint;
};

class CyThread: public QThread
{
//public:
    Q_OBJECT
    void run();

    void DataProcessor(char* Data, int size);

//protected:
    StartDataTransferParams StartParams;
    std::vector<EndPointParams> EndPointsParams;

public:
    //CyAPIProc(void);
    //~CyAPIProc(void);

    int GetEndPointsCount();
    void GetEndPointParamsByInd(int EndPointInd, int* Attr, bool* In, int* MaxPktSize, int* MaxBurst, int* Interface, int* Address);
    void StartTransferData(int EndPointInd, int PPX, int QueueSize, int TimeOut);
    void StopTransferData();

    int LoadFPGA(char* fwFileName);
    int LoadRAM(const char* fwFileName);
    void GetStreamerDevice();
    int Send16bitSPI(unsigned char data, unsigned char addr);
    int DeviceReset();
    int load1065Ctrlfile(const char* fwFileName, int lastaddr);
    void XferLoop(StartDataTransferParams* Params);
    int testSpectrRect(unsigned short* Data, int size);

signals:
    void resultReady( const QVector<double>* spc, int);
};

#endif // CYTHREAD_H
