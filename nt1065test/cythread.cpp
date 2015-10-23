#include "cythread.h"
#include "qdebug.h"
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


int testSpectrRectInit(int n)
{
    //customPlot =
    //customPlot->addGraph();
    fftw_in = (float*) fftwf_malloc(sizeof(float) * n);
    fftw_out = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * n);
    pl = fftwf_plan_dft_r2c_1d(n, fftw_in, fftw_out, FFTW_ESTIMATE);
    fftw_n = n;
    return 0;
}

float* spc;

int testSpectrRect(unsigned short* Data, int size)
{

    for (int i = 0; i < fftw_n; i++)
        fftw_in[i] = Data[i];
    fftwf_execute(pl);
    //qDebug() << "*";
    fftwf_complex* out = fftw_out;
    for (int i = 0; i < fftw_n; i++)
    {
        spc[i] = 10*log10(out[i][0]*out[i][0]+out[i][1]*out[i][1]);
        //out++;
    }
    emit resultReady(spc);
    return 0;
}

int testSpectrRectFree()
{
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
    CyAPIProc cy;
    cy.LoadRAM("..\\SlaveFifoSync.img");
    cy.GetStreamerDevice();
#if 0
    {
        int res;
        Sleep(100);
        res = cy.Send16bitSPI(0x03, 0x0C);
        Sleep(100);
        int Reg15 = 0x0B;
        res = cy.Send16bitSPI(Reg15, 0x0F);
        Sleep(100);
        res = cy.Send16bitSPI(Reg15, 0x16);
        Sleep(100);
        res = cy.Send16bitSPI(Reg15, 0x1D);
        Sleep(100);
        res = cy.Send16bitSPI(Reg15, 0x24);
        Sleep(100);
        res = cy.Send16bitSPI(0x0B, 0x13);
        Sleep(100);
        res = cy.Send16bitSPI(0x0B, 0x1A);
        Sleep(100);
        res = cy.Send16bitSPI(0x0B, 0x21);
        Sleep(100);
        res = cy.Send16bitSPI(0x0B, 0x28);
        Sleep(100);
        res = cy.Send16bitSPI(0x03, 0x0C);
    }
#endif
    //cy.load1065Ctrlfile("..\\default_fix_ADC_OUTCLK.txt");
    cy.load1065Ctrlfile("..\\singleLO_L1_10MHz.hex");
#if 1
    {
        int res;
        Sleep(100);
        res = cy.Send16bitSPI(0x03, 0x0C);
        Sleep(100);
        int Reg15 = 0x0B;
        res = cy.Send16bitSPI(Reg15, 0x0F);
        Sleep(100);
        res = cy.Send16bitSPI(Reg15, 0x16);
        Sleep(100);
        res = cy.Send16bitSPI(Reg15, 0x1D);
        Sleep(100);
        res = cy.Send16bitSPI(Reg15, 0x24);
    }
#endif
    testSpectrRectInit(5300);

    int Attr;
    bool In;
    int MaxPktSize;
    int MaxBurst;
    int Interface;
    int Address;
    int EndPointsCount = cy.GetEndPointsCount();
    for(int i = 0; i < EndPointsCount; i++){
        cy.GetEndPointParamsByInd(i, &Attr, &In, &MaxPktSize, &MaxBurst, &Interface, &Address);
        printf("EndPoint index %d, Attr = %d, In = %d, MaxPktSize = %d, MaxBurst = %d, Interface = %d, Address = %d\n", i, Attr, In, MaxPktSize, MaxBurst, Interface, Address);
    }
    printf("start transfering data\n");
    tmpDataContainer = new char[MAX_DATA_SIZE];
    cy.StartTransferData(0, 128, 4, 1500, DataProcessor);
    for (;;) Sleep(5000);
    cy.StopTransferData();
    testSpectrRectFree();
    cy.DeviceReset();

    //emit resultReady("FX3 is not connected");
}

