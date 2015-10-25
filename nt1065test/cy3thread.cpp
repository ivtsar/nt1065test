#include "cy3thread.h"
#include <qdebug.h>
#include <qfile.h>
#include "..\fftw3.h"

char *tmpDataContainer = NULL;
long DataSize = 0;
int packetsize = 0;
#define MAX_DATA_SIZE 10000000

//speed calc
unsigned int count_size = 0;
double PCFreq = 0.0;
LONGLONG perc;
double ptime = -1;
//

int testDataSpeed(int size)
{
    if (ptime == -1)
    {
        LARGE_INTEGER li;
        QueryPerformanceFrequency(&li);
        PCFreq = double(li.QuadPart)/1000.0;
        QueryPerformanceCounter(&li);
        perc = li.QuadPart;
    }

    if (count_size >= 64*1024*1024)
    {
        LARGE_INTEGER li;
        QueryPerformanceCounter(&li);
        ptime = double(li.QuadPart-perc)/PCFreq;
        perc = li.QuadPart;
        qDebug() << "time: " << ptime << ", size: " << count_size << ", speed: " << double(count_size)/ptime/1000.0;
        //fprintf(stderr, "t:%f, s:%i", ptime, count_size);
        count_size = 0;
    }else
        count_size += size;

    return ptime;
}

unsigned short mask = 0;

int testDataBits16(unsigned short* data, int size)
{
    for ( int scount = 0; scount < size/2; scount++ )
        mask |= data[scount];
    if (count_size >= 64*1024*1024)
    {
        qDebug() << "mask: " << hex << mask;
        //mask = 0;
    }
    return mask;
}

int histo1[4] = {0, 0, 0, 0};
int histo2[4] = {0, 0, 0, 0};
int histo3[4] = {0, 0, 0, 0};
int histo4[4] = {0, 0, 0, 0};

double histoOut(int* histo1)
{
    double histo1max = max(max(histo1[0], histo1[1]), max(histo1[2], histo1[3]));
    qDebug() << "histo1: " << histo1[1] << " " << histo1[0] << " " << histo1[2] << " " << histo1[3];
    qDebug() << "histo1norm: " << histo1[1]/histo1max << " " << histo1[0]/histo1max << " " << histo1[2]/histo1max << " " << histo1[3]/histo1max;
    histo1[0] = 0; histo1[1] = 0; histo1[2] = 0; histo1[3] = 0;

    return histo1max;
}
int testADCBits(unsigned short* data, int size)
{
    for ( int scount = 0; scount < size/2; scount++ )
    {
        histo1[(data[scount]&0x03)>>0]++;
        histo2[(data[scount]&0x0C)>>2]++;
        histo3[(data[scount]&0x30)>>4]++;
        histo4[(data[scount]&0xC0)>>6]++;
    }
    if (count_size >= 64*1024*1024)
    {
        histoOut(&histo1[0]);
    }
    return mask;
}

int acc[4] = {0, 0, 0, 0};
int decode_samples[4] = {1, 3, -1, -3};

int testADCoffset(unsigned short* data, int size)
{
    for ( int scount = 0; scount < size/2; scount++ )
    {
        acc[0] += decode_samples[(data[scount]&0x03)>>0];
        acc[1] += decode_samples[(data[scount]&0x0C)>>2];
        acc[2] += decode_samples[(data[scount]&0x30)>>4];
        acc[3] += decode_samples[(data[scount]&0xC0)>>6];
    }
    if (count_size >= 64*1024*1024)
    {
        qDebug() << "offset: " << double(acc[0])/size/2 << " " << double(acc[1])/size/2 << " " << double(acc[2])/size/2 << " " << double(acc[3])/size/2;
        acc[0] = 0; acc[1] = 0; acc[2] = 0; acc[3] = 0;
    }
    return mask;
}

float* fftw_in;
fftwf_complex* fftw_out;
fftwf_plan pl;
int fftw_n;
QVector<double>* spc;
int spc_counter = 100;

int testSpectrRectInit(int n)
{
    //customPlot =
    //customPlot->addGraph();
    spc = new QVector<double>(n/2);
    fftw_in = (float*) fftwf_malloc(sizeof(float) * n);
    fftw_out = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * n);
    pl = fftwf_plan_dft_r2c_1d(n, fftw_in, fftw_out, FFTW_ESTIMATE);
    fftw_n = n;
    return 0;
}

int CyThread::testSpectrRect(unsigned short* data, int size)
{
    if (spc_counter)
    {
        spc_counter--;
        return 0;
    }
    spc_counter = 20;

    FILE* f = fopen( "dump.bin", "wb" );
    fwrite( data, sizeof( short ), size / 2, f );
    fclose( f );
#if 1
    for (int i = 0; i < fftw_n; i++) {
        fftw_in[i] = decode_samples[(data[i]&0x03)>>0];
        //qDebug() << (data[i]&0x03) << " " << decode_samples[(data[i]&0x03)>>0];
    }
    fftwf_execute(pl);
    for (int i = 0; i < fftw_n/2; i++)
        (*spc)[i] = (double)(10.0*log10(fftw_out[i][0]*fftw_out[i][0]+fftw_out[i][1]*fftw_out[i][1]));
    emit resultReady(spc, 1);
#endif
#if 1
    for (int i = 0; i < fftw_n; i++)
        fftw_in[i] = decode_samples[(data[i]&0x0C)>>2];
    fftwf_execute(pl);
    for (int i = 0; i < fftw_n/2; i++)
        (*spc)[i] = (double)(10.0*log10(fftw_out[i][0]*fftw_out[i][0]+fftw_out[i][1]*fftw_out[i][1]));
    emit resultReady(spc, 2);
#endif
#if 1
    for (int i = 0; i < fftw_n; i++)
        fftw_in[i] = decode_samples[(data[i]&0x30)>>4];
    fftwf_execute(pl);
    for (int i = 0; i < fftw_n/2; i++)
        (*spc)[i] = (double)(10.0*log10(fftw_out[i][0]*fftw_out[i][0]+fftw_out[i][1]*fftw_out[i][1]));
    emit resultReady(spc, 3);
#endif
#if 1
    for (int i = 0; i < fftw_n; i++)
        fftw_in[i] = decode_samples[(data[i]&0xC0)>>6];
    fftwf_execute(pl);
    for (int i = 0; i < fftw_n/2; i++)
        (*spc)[i] = (double)(10.0*log10(fftw_out[i][0]*fftw_out[i][0]+fftw_out[i][1]*fftw_out[i][1]));
    emit resultReady(spc, 4);
#endif
    return 0;
}


int testSpectrRectFree()
{
    delete spc;
    fftwf_free(fftw_in);
    fftwf_free(fftw_out);
    fftwf_destroy_plan(pl);
    return 0;
}

//void __stdcall DataProcessor(char* Data, int size)
void CyThread::DataProcessor(char* Data, int size)
{
    testDataSpeed(size);
    //testDataBits16((unsigned short*) Data, size);
    //testADCBits((unsigned short*) Data, size);
    //testADCoffset((unsigned short*) Data, size);
    testSpectrRect((unsigned short*) Data, size);
    //qDebug() << "data received";
    if(DataSize + size > MAX_DATA_SIZE)
        return;
    //memcpy(tmpDataContainer + DataSize, Data, size);
    DataSize += size;
    packetsize = size;
}


void CyThread::run()
{
    StartParams.USBDevice = new CCyFX3Device;

    if ( ReviewDevices() ) {
        qFatal( "Hardware or driver problem" );
        return;
    }

    if ( LoadRAM("..\\SlaveFifoSync.img") ) {
        qFatal( "LoadRAM() error" );
        return;
    }
    GetStreamerDevice();

    //cy.load1065Ctrlfile("..\\default_fix_ADC_OUTCLK.txt");
    if ( load1065Ctrlfile("..\\singleLO_L1_10MHz_ADC_CLKOUT_RE_noIFAGC.hex", 112) ) {
        qFatal( "load1065Ctrlfile() error" );
        return;
    }
    if ( load1065Ctrlfile("..\\singleLO_L1_10MHz_ADC_CLKOUT_RE_noIFAGC.hex", 48) ) {
        qFatal( "load1065Ctrlfile() error" );
        return;
    }
#if 0
    {
        int res;
        Sleep(100);
        res = Send16bitSPI(0x03, 0x0C);
        Sleep(100);
        int Reg15 = 0x0B;
        res = Send16bitSPI(Reg15, 0x0F);
        Sleep(100);
        res = Send16bitSPI(Reg15, 0x16);
        Sleep(100);
        res = Send16bitSPI(Reg15, 0x1D);
        Sleep(100);
        res = Send16bitSPI(Reg15, 0x24);
    }
#endif
    testSpectrRectInit(5300);

    int Attr;
    bool In;
    int MaxPktSize;
    int MaxBurst;
    int Interface;
    int Address;
    int EndPointsCount = GetEndPointsCount();
    for(int i = 0; i < EndPointsCount; i++){
        GetEndPointParamsByInd(i, &Attr, &In, &MaxPktSize, &MaxBurst, &Interface, &Address);
        printf("EndPoint index %d, Attr = %d, In = %d, MaxPktSize = %d, MaxBurst = %d, Interface = %d, Address = %d\n", i, Attr, In, MaxPktSize, MaxBurst, Interface, Address);
        fflush(stdout);
    }
    printf("start transfering data\n");
    fflush(stdout);
    tmpDataContainer = new char[MAX_DATA_SIZE];
    StartTransferData(0, 128, 4, 1500);
    XferLoop(&StartParams);
    //for (;;) Sleep(5000);
    StopTransferData();
    testSpectrRectFree();
    DeviceReset();

    //emit resultReady("FX3 is not connected");
}

int CyThread::LoadRAM(const char* fwFileName)
{
    qDebug() << "LoadRAM( '" << fwFileName << "' )";
    QFile cf(fwFileName);
    if (!cf.open(QFile::ReadOnly)) {
        qFatal( "LoadRAM file '%s' io error!", fwFileName );
        return -1;
    } else {
        cf.close();
    }

    if (NULL != StartParams.USBDevice) {
        //qDebug() << "StartParams.USBDevice->Open(0)" << StartParams.USBDevice->Open(0);
        if (StartParams.USBDevice->IsBootLoaderRunning())
        {
            int retCode  = StartParams.USBDevice->DownloadFw((char*)fwFileName, FX3_FWDWNLOAD_MEDIA_TYPE::RAM);
            if (FX3_FWDWNLOAD_ERROR_CODE::SUCCESS == retCode)
            {
                Sleep(6000);
                return retCode;
            }
            else return -1;
        }
    } else {
        qDebug() << "LoadRAM(): USBDevice is NULL";
        return -2;
    }
    return 0;
}

int CyThread::load1065Ctrlfile(const char* fileName, int lastaddr)
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
                Sleep(20);
                Send16bitSPI(data, addr);
                if (addr == lastaddr) break;
            }
    } while (!line.isNull());
    return 0;
}

int CyThread::Send16bitSPI(unsigned char data, unsigned char addr)
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

int CyThread::DeviceReset()
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

int CyThread::LoadFPGA(char* fwFileName)
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

int CyThread::GetEndPointsCount(){
    return EndPointsParams.size();
}

void CyThread::GetEndPointParamsByInd(int EndPointInd, int* Attr, bool* In, int* MaxPktSize, int* MaxBurst, int* Interface, int* Address){
    if(EndPointInd >= EndPointsParams.size())
        return;
    *Attr = EndPointsParams[EndPointInd].Attr;
    *In = EndPointsParams[EndPointInd].In;
    *MaxPktSize = EndPointsParams[EndPointInd].MaxPktSize;
    *MaxBurst = EndPointsParams[EndPointInd].MaxBurst;
    *Interface = EndPointsParams[EndPointInd].Interface;
    *Address = EndPointsParams[EndPointInd].Address;
}

void CyThread::GetStreamerDevice(){

    qDebug() << "CyThread::GetStreamerDevice()";
    //StartParams.USBDevice = new CCyFX3Device;//(NULL, CYUSBDRV_GUID,true);

    if (StartParams.USBDevice == NULL) {
        qDebug() << "Error: CyThread::GetStreamerDevice() USBDevice == NULL";
        return;
    }

    int n = StartParams.USBDevice->DeviceCount();

    // Walk through all devices looking for VENDOR_ID/PRODUCT_ID
    for (int i=0; i<n; i++)
    {
        qDebug( "Device[%2d]: 0x%04X 0x%04X",
                i,
                StartParams.USBDevice->VendorID,
                StartParams.USBDevice->ProductID );
        if ((StartParams.USBDevice->VendorID == VENDOR_ID) && (StartParams.USBDevice->ProductID == PRODUCT_ID)) {
            qDebug( "Matched!" );
            break;
        }

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

int CyThread::ReviewDevices() {
    if (StartParams.USBDevice == NULL) {
        qFatal( "Error: ReviewDevices::GetStreamerDevice() USBDevice == NULL" );
        return -1;
    }
    int dev_cnt = StartParams.USBDevice->DeviceCount();
    qDebug( "ReviewDevices(): found %d devices", dev_cnt );

    int suitable_dev_cnt = 0;
    for (int i=0; i<dev_cnt; i++)
    {
        unsigned short product = StartParams.USBDevice->ProductID;
        unsigned short vendor  = StartParams.USBDevice->VendorID;
        bool suitable = ( vendor == VENDOR_ID ) && ( (product == PRODUCT_ID) || (product == PRODUCT_ID2) );
        qDebug( "Device[%2d]: 0x%04X 0x%04X %s", i, vendor, product, suitable ? "***" : "" );
        if ( suitable ) {
            suitable_dev_cnt++;
        }
    }
    if (suitable_dev_cnt == 0) {
        return -2;
    } else {
        return 0;
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

void CyThread::XferLoop(StartDataTransferParams* Params){
    //StartDataTransferParams *Params;
    //Params = (StartDataTransferParams*)pParams;

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

        //Params->DataProc((char*)buffers[i], len);
        DataProcessor((char*)buffers[i], len);

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

void CyThread::StartTransferData(int EndPointInd, int PPX, int QueueSize, int TimeOut){
    if(EndPointInd >= EndPointsParams.size())
        return;
    StartParams.CurEndPoint = EndPointsParams[EndPointInd];
    StartParams.PPX = PPX;
    StartParams.QueueSize = QueueSize;
    StartParams.TimeOut = TimeOut;
    StartParams.bStreaming = true;
    StartParams.ThreadAlreadyStopped = false;
    //StartParams.DataProc = DataProc;

    int alt = StartParams.CurEndPoint.Interface;
    int eptAddr = StartParams.CurEndPoint.Address;
    int clrAlt = (StartParams.USBDevice->AltIntfc() == 0) ? 1 : 0;
    if (! StartParams.USBDevice->SetAltIntfc(alt))
    {
        StartParams.USBDevice->SetAltIntfc(clrAlt); // Cleans-up
        return;
    }

    StartParams.EndPt = StartParams.USBDevice->EndPointOf((UCHAR)eptAddr);


    //CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)XferLoop, &StartParams, 0, NULL);
}

void CyThread::StopTransferData(){
    if(!StartParams.bStreaming)
        return;
    StartParams.bStreaming = false;
    while(!StartParams.ThreadAlreadyStopped)
        Sleep(0);
}
