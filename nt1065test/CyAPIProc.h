#pragma once

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

class CyAPIProc
{
protected:	
	StartDataTransferParams StartParams; 
	std::vector<EndPointParams> EndPointsParams;

public:
	CyAPIProc(void);
	~CyAPIProc(void);

	int GetEndPointsCount();
	void GetEndPointParamsByInd(int EndPointInd, int* Attr, bool* In, int* MaxPktSize, int* MaxBurst, int* Interface, int* Address);
	void StartTransferData(int EndPointInd, int PPX, int QueueSize, int TimeOut, DataProcessorFunc DataProc);
	void StopTransferData();

    int LoadFPGA(char* fwFileName);
    int LoadRAM(char* fwFileName);
    void GetStreamerDevice();
    int Send16bitSPI(unsigned char data, unsigned char addr);
    int DeviceReset();
    int load1065Ctrlfile(char* fwFileName);

};

