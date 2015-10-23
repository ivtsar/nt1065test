#include <windows.h>
#include "CyAPIProc.h"
#include <QTextStream>
#include <QFile>
#include <QDebug>

CyAPIProc::CyAPIProc(void){
    StartParams.USBDevice = new CCyFX3Device;
}

int CyAPIProc::LoadRAM(char* fwFileName)
{
    if ((NULL != StartParams.USBDevice) && (StartParams.USBDevice->IsBootLoaderRunning()))
    {
        int retCode  = StartParams.USBDevice->DownloadFw(fwFileName, FX3_FWDWNLOAD_MEDIA_TYPE::RAM);
        if (FX3_FWDWNLOAD_ERROR_CODE::SUCCESS == retCode)
        {
            Sleep(5000);
            return retCode;
        }
        else return -1;
    }
    return 0;
}

int CyAPIProc::load1065Ctrlfile(char* fileName)
{
    QFile cf(fileName);
    if (!cf.open(QFile::ReadOnly)) return -1;
    QTextStream st(&cf);
    QString line;
    do {
        line = st.readLine();
        if (line[0] != ';')
            if (line.startsWith("Reg"))
            {
                int addr;
                int data;
                bool ok;
                line.remove(0, 3);
                QString dataStr = line.right(2);
                line.remove(line.length()-3, 3);
                addr = line.toInt(&ok, 10);
                data = dataStr.toInt(&ok, 16);
                Sleep(100);
                Send16bitSPI(data, addr);
            }
    } while (!line.isNull());
    return 0;
}

int CyAPIProc::Send16bitSPI(unsigned char data, unsigned char addr)
{
    UCHAR buf[16];
    buf[0] = (UCHAR)(data);
    buf[1] = (UCHAR)(addr);

    qDebug() << "Reg" << addr << " " << hex << data;

    CCyControlEndPoint* CtrlEndPt;
    CtrlEndPt = StartParams.USBDevice->ControlEndPt;
    CtrlEndPt->Target = TGT_DEVICE;
    CtrlEndPt->ReqType = REQ_VENDOR;
    CtrlEndPt->Direction = DIR_TO_DEVICE;
    CtrlEndPt->ReqCode = 0xB3;
    CtrlEndPt->Value = 0;
    CtrlEndPt->Index = 1;
    long len = 16;
    int success = CtrlEndPt->XferData(&buf[0], len);

    return success;
}

int CyAPIProc::DeviceReset()
{
    UCHAR buf[16];
    buf[2] = (UCHAR)(0xFF);

    CCyControlEndPoint* CtrlEndPt;
    CtrlEndPt = StartParams.USBDevice->ControlEndPt;
    CtrlEndPt->Target = TGT_DEVICE;
    CtrlEndPt->ReqType = REQ_VENDOR;
    CtrlEndPt->Direction = DIR_TO_DEVICE;
    CtrlEndPt->ReqCode = 0xB3;
    CtrlEndPt->Value = 0;
    CtrlEndPt->Index = 1;
    long len = 16;
    int success = CtrlEndPt->XferData(&buf[0], len);

    return success;
}

int CyAPIProc::LoadFPGA(char* fwFileName)
{
    int retCode;

    FILE* fpga = fopen(fwFileName, "rb");
    fseek(fpga, 0L, SEEK_END);
    long sz = ftell(fpga);
    fseek(fpga, 0L, SEEK_SET);
    UCHAR* buff = (UCHAR*) new UCHAR[sz];
    int rd = fread((void*)buff, 1, sz, fpga);
    fclose(fpga);

    //if ((StartParams.USBDevice->VendorID == VENDOR_ID) && (StartParams.USBDevice->ProductID == PRODUCT_ID))
    {
        //CCyBulkEndPoint* bept = (CCyBulkEndPoint*)StartParams.USBDevice->EndPointOf(0x02);
        CCyBulkEndPoint* bept = StartParams.USBDevice->BulkOutEndPt;
        UCHAR buf[16];
        buf[0] = (UCHAR)(sz & 0x000000FF);
        buf[1] = (UCHAR)((sz & 0x0000FF00) >> 8);
        buf[2] = (UCHAR)((sz & 0x00FF0000) >> 16);
        buf[3] = (UCHAR)((sz & 0xFF000000) >> 24);

        CCyControlEndPoint* CtrlEndPt;
        CtrlEndPt = StartParams.USBDevice->ControlEndPt;
        CtrlEndPt->Target = TGT_DEVICE;
        CtrlEndPt->ReqType = REQ_VENDOR;
        CtrlEndPt->Direction = DIR_TO_DEVICE;
        CtrlEndPt->ReqCode = 0xB2;
        CtrlEndPt->Value = 0;
        CtrlEndPt->Index = 1;
        long len = 16;
        int success = CtrlEndPt->XferData(&buf[0], len);//send vendor command to start configuration
        // myDevice.BulkOutEndPt.TimeOut = 100000;
        if (success == 1)
        {

            bept->SetXferSize(4096);//set transfer size as 4096
            //myDevice.BulkOutEndPt.TimeOut = 1000;

            success = bept->XferData(buff, sz); //check if transfer successful
            if (success == 0)
            {
                int st = bept->NtStatus;
                char sstr[1000];
                StartParams.USBDevice->UsbdStatusString(bept->UsbdStatus, &sstr[0]);
                delete [] buff;
                return 0;
            }
            else
            {
                Sleep(1000);
                CtrlEndPt->Target = TGT_DEVICE;
                CtrlEndPt->ReqType = REQ_VENDOR;
                CtrlEndPt->Direction = DIR_FROM_DEVICE;
                CtrlEndPt->ReqCode = 0xB1;
                CtrlEndPt->Value = 0;
                CtrlEndPt->Index = 1;
                long len = 16;
                int success = CtrlEndPt->XferData(&buf[0], len);//send vendor command to start configuration
                // myDevice.BulkOutEndPt.TimeOut = 100000;
                if (success == 1)
                {
                    if (1 == buf[0])
                    {
                        //Success
                        retCode = 1;
                    }
                    else
                    {
                        //Fail
                        retCode = 0;
                    }
                }

            }
        }

    }
    delete [] buff;

    return retCode;
}

CyAPIProc::~CyAPIProc(void){
}

int CyAPIProc::GetEndPointsCount(){
	return EndPointsParams.size();
}

void CyAPIProc::GetEndPointParamsByInd(int EndPointInd, int* Attr, bool* In, int* MaxPktSize, int* MaxBurst, int* Interface, int* Address){
	if(EndPointInd >= EndPointsParams.size())
		return;
	*Attr = EndPointsParams[EndPointInd].Attr;
	*In = EndPointsParams[EndPointInd].In;
	*MaxPktSize = EndPointsParams[EndPointInd].MaxPktSize;
	*MaxBurst = EndPointsParams[EndPointInd].MaxBurst;
	*Interface = EndPointsParams[EndPointInd].Interface;
	*Address = EndPointsParams[EndPointInd].Address;
}

void CyAPIProc::GetStreamerDevice(){

    //StartParams.USBDevice = new CCyFX3Device;//(NULL, CYUSBDRV_GUID,true);

    if (StartParams.USBDevice == NULL) return;

    int n = StartParams.USBDevice->DeviceCount();

    // Walk through all devices looking for VENDOR_ID/PRODUCT_ID
    for (int i=0; i<n; i++)
    {
        if ((StartParams.USBDevice->VendorID == VENDOR_ID) && (StartParams.USBDevice->ProductID == PRODUCT_ID)) 
            break;

        StartParams.USBDevice->Open(i);
    }

    if ((StartParams.USBDevice->VendorID == VENDOR_ID) && (StartParams.USBDevice->ProductID == PRODUCT_ID)) 
    {
        int interfaces = StartParams.USBDevice->AltIntfcCount()+1;

        StartParams.bHighSpeedDevice = StartParams.USBDevice->bHighSpeed;
        StartParams.bSuperSpeedDevice = StartParams.USBDevice->bSuperSpeed;

        for (int i=0; i< interfaces; i++)
        {
            StartParams.USBDevice->SetAltIntfc(i);

            int eptCnt = StartParams.USBDevice->EndPointCount();

            // Fill the EndPointsBox
            for (int e=1; e<eptCnt; e++)
            {
                CCyUSBEndPoint *ept = StartParams.USBDevice->EndPoints[e];
                // INTR, BULK and ISO endpoints are supported.
                if ((ept->Attributes >= 1) && (ept->Attributes <= 3))
                {
					EndPointParams Params;
					Params.Attr = ept->Attributes;
					Params.In = ept->bIn;
					Params.MaxPktSize = ept->MaxPktSize;
					Params.MaxBurst = StartParams.USBDevice->BcdUSB == 0x0300 ? ept->ssmaxburst : 0;
					Params.Interface = i;
					Params.Address = ept->Address;

					EndPointsParams.push_back(Params);
                }
            }
        }
    }
}

void AbortXferLoop(StartDataTransferParams* Params, int pending, PUCHAR *buffers, CCyIsoPktInfo **isoPktInfos, PUCHAR *contexts, OVERLAPPED *inOvLap)
{
    //EndPt->Abort(); - This is disabled to make sure that while application is doing IO and user unplug the device, this function hang the app.
    long len = Params->EndPt->MaxPktSize * Params->PPX;

    for (int j=0; j< Params->QueueSize; j++) 
    { 
        if (j<pending)
        {
            if (!Params->EndPt->WaitForXfer(&inOvLap[j], Params->TimeOut)) 
            {
                Params->EndPt->Abort();
                if (Params->EndPt->LastError == ERROR_IO_PENDING)
                    WaitForSingleObject(inOvLap[j].hEvent,2000);
            }

            Params->EndPt->FinishDataXfer(buffers[j], len, &inOvLap[j], contexts[j]);
        }

        CloseHandle(inOvLap[j].hEvent);

        delete [] buffers[j];
        delete [] isoPktInfos[j];
    }

    delete [] buffers;
    delete [] isoPktInfos;
    delete [] contexts;

    Params->bStreaming = false;
	Params->ThreadAlreadyStopped = true;
}

void XferLoop(LPVOID pParams){
	StartDataTransferParams *Params;
	Params = (StartDataTransferParams*)pParams;

	if(Params->EndPt->MaxPktSize==0)
		return;

    // Limit total transfer length to 4MByte
    long len = ((Params->EndPt->MaxPktSize) * Params->PPX);

    int maxLen = 0x400000;  //4MByte
    if (len > maxLen){
        Params->PPX = maxLen / (Params->EndPt->MaxPktSize);
        if((Params->PPX%8)!=0)
            Params->PPX -= (Params->PPX%8);
    }

    if ((Params->bSuperSpeedDevice || Params->bHighSpeedDevice) && (Params->EndPt->Attributes == 1)){  // HS/SS ISOC Xfers must use PPX >= 8
		Params->PPX = max(Params->PPX, 8);
        Params->PPX = (Params->PPX / 8) * 8;
        if(Params->bHighSpeedDevice)
			Params->PPX = max(Params->PPX, 128);
    }

	long BytesXferred = 0;
    unsigned long Successes = 0;
    unsigned long Failures = 0;
    int i = 0;

    // Allocate the arrays needed for queueing
    PUCHAR			*buffers		= new PUCHAR[Params->QueueSize];
    CCyIsoPktInfo	**isoPktInfos	= new CCyIsoPktInfo*[Params->QueueSize];
    PUCHAR			*contexts		= new PUCHAR[Params->QueueSize];
    OVERLAPPED		inOvLap[MAX_QUEUE_SZ];

    Params->EndPt->SetXferSize(len);

    // Allocate all the buffers for the queues
    for (i=0; i< Params->QueueSize; i++) 
    { 
        buffers[i]        = new UCHAR[len];
        isoPktInfos[i]    = new CCyIsoPktInfo[Params->PPX];
        inOvLap[i].hEvent = CreateEvent(NULL, false, false, NULL);

        memset(buffers[i],0xEF,len);
    }

    // Queue-up the first batch of transfer requests
    for (i=0; i< Params->QueueSize; i++)	
    {
        contexts[i] = Params->EndPt->BeginDataXfer(buffers[i], len, &inOvLap[i]);
        if (Params->EndPt->NtStatus || Params->EndPt->UsbdStatus) // BeginDataXfer failed
        {
            AbortXferLoop(Params, i+1, buffers,isoPktInfos,contexts,inOvLap);
            return;
        }
    }

    i=0;	

    // The infinite xfer loop.
    for (;Params->bStreaming;)		
    {
        long rLen = len;	// Reset this each time through because
        // FinishDataXfer may modify it

        if (!Params->EndPt->WaitForXfer(&inOvLap[i], Params->TimeOut))
        {
            Params->EndPt->Abort();
            if (Params->EndPt->LastError == ERROR_IO_PENDING)
                WaitForSingleObject(inOvLap[i].hEvent,2000);
        }

        if (Params->EndPt->Attributes == 1) // ISOC Endpoint
        {	
            if (Params->EndPt->FinishDataXfer(buffers[i], rLen, &inOvLap[i], contexts[i], isoPktInfos[i])) 
            {			
                CCyIsoPktInfo *pkts = isoPktInfos[i];
                for (int j=0; j< Params->PPX; j++) 
                {
					if ((pkts[j].Status == 0) && (pkts[j].Length<=Params->EndPt->MaxPktSize)) 
                    {
                        BytesXferred += pkts[j].Length;
                        Successes++;
                    }
                    else
                        Failures++;
                    pkts[j].Length = 0;	// Reset to zero for re-use.
					pkts[j].Status = 0;
                }
            } 
            else
                Failures++; 
        } 
        else // BULK Endpoint
        {
            if (Params->EndPt->FinishDataXfer(buffers[i], rLen, &inOvLap[i], contexts[i])) 
            {			
                Successes++;
                BytesXferred += len;
            } 
            else
                Failures++; 
        }

		BytesXferred = max(BytesXferred, 0); 

		Params->DataProc((char*)buffers[i], len);

        // Re-submit this queue element to keep the queue full
        contexts[i] = Params->EndPt->BeginDataXfer(buffers[i], len, &inOvLap[i]);
        if (Params->EndPt->NtStatus || Params->EndPt->UsbdStatus) // BeginDataXfer failed
        {
            AbortXferLoop(Params, Params->QueueSize,buffers,isoPktInfos,contexts,inOvLap);
            return;
        }

		i = (i + 1) % Params->QueueSize;
    }  // End of the infinite loop

    // Memory clean-up
    AbortXferLoop(Params, Params->QueueSize,buffers,isoPktInfos,contexts,inOvLap);
}

void CyAPIProc::StartTransferData(int EndPointInd, int PPX, int QueueSize, int TimeOut, DataProcessorFunc DataProc){
	if(EndPointInd >= EndPointsParams.size())
		return;
	StartParams.CurEndPoint = EndPointsParams[EndPointInd];
	StartParams.PPX = PPX;
	StartParams.QueueSize = QueueSize;
	StartParams.TimeOut = TimeOut;
	StartParams.bStreaming = true;
	StartParams.ThreadAlreadyStopped = false;
	StartParams.DataProc = DataProc;

	int alt = StartParams.CurEndPoint.Interface;
	int eptAddr = StartParams.CurEndPoint.Address;
	int clrAlt = (StartParams.USBDevice->AltIntfc() == 0) ? 1 : 0;
	if (! StartParams.USBDevice->SetAltIntfc(alt))
    {
        StartParams.USBDevice->SetAltIntfc(clrAlt); // Cleans-up
        return;
    }

    StartParams.EndPt = StartParams.USBDevice->EndPointOf((UCHAR)eptAddr);


	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)XferLoop, &StartParams, 0, NULL);
}

void CyAPIProc::StopTransferData(){
	if(!StartParams.bStreaming)
		return;
	StartParams.bStreaming = false;
	while(!StartParams.ThreadAlreadyStopped)
		Sleep(0);
}
