#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#include <iostream>
#include <sstream>
#include <fstream>		// ifstream
#include <algorithm>	// remove_if
#include <chrono>
#include <signal.h>
#include <fcntl.h>

#include "CameraControl.h"

using namespace std;
using namespace chrono;

#define LOG_NAME	"[GigEGrab]"
#define ETH1_STATUS "/sys/class/net/eth1/operstate"

bool g_bExit = false;

//Use SDK
int                 g_hCamera;          // device handler
unsigned char*      g_pFrameBuffer;	    // data buffer
tSdkFrameHead       g_tFrameHead;       // header info of image frame
tSdkCameraCapbility g_tCapability;      // device information
tSdkFrameStatistic  g_tFrameStatistic;
tSdkImageResolution g_tResolution;

string              msg;
int                 g_read_fps;        // fps
int                 g_SaveImage_type;  // save format

static CameraControl *pCamCtrl = NULL;
int signalHandler(int sig, void *ptr) 
{
	if(sig == SIGINT || sig == SIGQUIT){
		SAFE_DELETE(pCamCtrl);

        if(g_hCamera > 0) {
            // release resources
            CameraUnInit(g_hCamera);
            g_hCamera = -1;
        }

        if(g_pFrameBuffer != NULL) {
            free(g_pFrameBuffer);
            g_pFrameBuffer = NULL;
        }
        
		exit(0);
	}

	return 0;
}

/*
typedef struct
{
    char acProductSeries[32];   // 
    char acProductName[32];     // 
    char acFriendlyName[32];    // 
    char acLinkName[32];        // 
    char acDriverVersion[32];   // 
    char acSensorType[32];      // 
    char acPortType[32];        // 
    char acSn[32];              // 
    UINT uInstance;             // 
} tSdkCameraDevInfo;
*/
bool PrintDeviceInfo(tSdkCameraDevInfo* pstDevInfo)
{
    if (NULL == pstDevInfo)
    {
        printf("The Pointer of pstDevInfo is NULL!\n");
        msg = "The Pointer of pstDevInfo is NULL";
        ERR_LOG(msg);

        return false;
    } 

    printf("Device Product Series: %s\n",   pstDevInfo->acProductSeries);
    printf("Device Model Name: %s\n",       pstDevInfo->acProductName);
    printf("Device Friendly Name: %s\n",    pstDevInfo->acFriendlyName);
    printf("Device Link Name: %s\n",        pstDevInfo->acLinkName);
    printf("Device Driver Version: %s\n",   pstDevInfo->acDriverVersion);
    printf("Device Sensor Type: %s\n",      pstDevInfo->acSensorType);
    printf("Device Port Type: %s\n",        pstDevInfo->acPortType);
    printf("Device S/N: %s\n",              pstDevInfo->acSn);


    return true;
}

void PrintFrameStatisticInfo()
{
    printf("=========== tSdkCameraCapbility - tSdkFrameSpeed ================\n");
    printf("g_tCapability.tSdkFrameSpeed: %d, %s\n", g_tCapability.pFrameSpeedDesc->iIndex, g_tCapability.pFrameSpeedDesc->acDescription);
    printf("================================================\n");

    printf("=========== tSdkCameraCapbility - tSdkIspCapacity ================\n");
    printf("g_tCapability.sIspCapacity.bMonoSensor: %d\n", g_tCapability.sIspCapacity.bMonoSensor);
    printf("g_tCapability.sIspCapacity.bWbOnce: %d\n", g_tCapability.sIspCapacity.bWbOnce);
    printf("g_tCapability.sIspCapacity.bAutoWb: %d\n", g_tCapability.sIspCapacity.bAutoWb);
    printf("g_tCapability.sIspCapacity.bAutoExposure: %d\n", g_tCapability.sIspCapacity.bAutoExposure);
    printf("g_tCapability.sIspCapacity.bManualExposure: %d\n", g_tCapability.sIspCapacity.bManualExposure);
    printf("g_tCapability.sIspCapacity.bAntiFlick: %d\n", g_tCapability.sIspCapacity.bAntiFlick);
    printf("g_tCapability.sIspCapacity.bDeviceIsp: %d\n", g_tCapability.sIspCapacity.bDeviceIsp);
    printf("g_tCapability.sIspCapacity.bForceUseDeviceIsp: %d\n", g_tCapability.sIspCapacity.bForceUseDeviceIsp);
    printf("g_tCapability.sIspCapacity.bZoomHD: %d\n", g_tCapability.sIspCapacity.bZoomHD);
    printf("==================================================================\n");

    printf("=========== tSdkCameraCapbility - tSdkExpose ================\n");
    printf("g_tCapability.sExposeDesc.uiTargetMin: %d\n", g_tCapability.sExposeDesc.uiTargetMin);
    printf("g_tCapability.sExposeDesc.uiTargetMax: %d\n", g_tCapability.sExposeDesc.uiTargetMax);
    printf("g_tCapability.sExposeDesc.uiAnalogGainMin: %d\n", g_tCapability.sExposeDesc.uiAnalogGainMin);
    printf("g_tCapability.sExposeDesc.uiAnalogGainMax: %d\n", g_tCapability.sExposeDesc.uiAnalogGainMax);
    printf("g_tCapability.sExposeDesc.fAnalogGainStep: %f\n", g_tCapability.sExposeDesc.fAnalogGainStep);
    printf("g_tCapability.sExposeDesc.uiExposeTimeMin: %d\n", g_tCapability.sExposeDesc.uiExposeTimeMin);
    printf("g_tCapability.sExposeDesc.uiExposeTimeMax: %d\n", g_tCapability.sExposeDesc.uiExposeTimeMax);
    printf("=============================================================\n");
}

// wait for user to input enter to stop grabbing or end the sample program
void PressEnterToExit(void)
{
    int c;
    while ( (c = getchar()) != '\n' && c != EOF );
    fprintf( stderr, "\nPress enter to exit.\n");
    while( getchar() != '\n');
    g_bExit = true;
    sleep(1);
}

static void* GrabThread(void* pParam)
{
    CameraSdkStatus status;
    BYTE*           pRawData;

    INFO_LOG(string("*** IMAGE ACQUISITION ***"));

    while(1) {
        status = pCamCtrl->GetFrameData(pRawData);
        if(status == CAMERA_STATUS_SUCCESS) {
            // post processing
            pCamCtrl->PostProcess();
            //pCamCtrl->CalcGrabberStat();
        }
        else {
            msg = "Fail to get frame data : " + to_string(status);
	        WARN_LOG(msg);
            usleep(10000);

            if(status == CAMERA_STATUS_TIME_OUT) {
                system("ifconfig eth1 down");
                g_bExit = TRUE;
            }
        }

        if(g_bExit) break;
    }

    return 0;
}

#if false
static void* GrabThread_Test(void* pParam)
{
    g_read_fps = 0; // init

    CameraSdkStatus status;
    BYTE*           pRawData;

    static int64_t runningTime  = 0;
    static int pack_size        = 10;
    while(1)
    {
        // elapsed time : start
        system_clock::time_point start = system_clock::now();

#if true
        if(pCamCtrl->GetFrameData(pRawData) == CAMERA_STATUS_SUCCESS)
        {
            /*
            if(g_tFrameHead.uiMediaType==CAMERA_MEDIA_TYPE_MONO8)
            {
                status = CameraSaveImage(g_hCamera, "/oem/Screen_shot/monitor_.jpg", g_pFrameBuffer, &g_tFrameHead, FILE_JPG, 100);
                if (status == CAMERA_STATUS_SUCCESS)
                {
                    //printf("Save image successfully. image_size = %dX%d\n", g_tFrameHead.iWidth, g_tFrameHead.iHeight);
                }
                else
                {
                    printf("Save image failed. err=%d\n", status);
                }
            }
            */

            g_read_fps++;
            //printf("Captured image : %d\n", g_read_fps);
        }
        else 
        {
            printf("wait for capturing...\n");
            usleep(10000);
        }
#else
        // Take a frame from the camera
        if(CameraGetImageBuffer(g_hCamera, &g_tFrameHead, &g_pFrameBuffer, 200) == CAMERA_STATUS_SUCCESS)
        {
            CameraImageProcess(g_hCamera, pRawData, g_pFrameBuffer, &g_tFrameHead);
            CameraReleaseImageBuffer(g_hCamera, pRawData);

            if(g_tFrameHead.uiMediaType == CAMERA_MEDIA_TYPE_MONO8){
                status = CameraSaveImage(g_hCamera, "/oem/Screen_shot/monitor_.jpg", g_pFrameBuffer, &g_tFrameHead, FILE_JPG, 100);
                if (status == CAMERA_STATUS_SUCCESS)
                {
                    printf("Save image successfully. image_size = %dX%d\n", g_tFrameHead.iWidth, g_tFrameHead.iHeight);
                }
                else
                {
                    printf("Save image failed. err=%d\n", status);
                }
            }

            g_read_fps++;
            //printf("Captured image : %d\n", g_read_fps);
        }
        else 
        {
            printf("wait for capturing...\n");
            usleep(10000);
        }
#endif


        system_clock::time_point end = system_clock::now();
        nanoseconds nano = end - start;
        runningTime += nano.count()/1000000;	// msec
        if (g_read_fps % pack_size == 0)
        {
            double FPS = (double)(1000*pack_size)/runningTime;
            printf("Captured FPS : %f\n", FPS);

            runningTime = 0;
        }

        if(g_bExit) break;
    }

    return 0;
}
#endif

int main()
{
    CameraSdkStatus         status = -1;
    tSdkCameraDevInfo       tCameraEnumList[4];
    BOOL                    bMonoCamera = true;
    int                     iCameraCounts = 4;
    //int                     iValue;
    //double                  dwValue;


    pthread_t nThreadID;
	cpu_set_t mask_grab;

    openlog(LOG_NAME, LOG_PID, LOG_USER);

    msg = "LOG_LEVEL = " + to_string(LOG_LEVEL);
	INFO_LOG(msg);

    // SDK Initialize (0:Eng, 1:Chn)
    CameraSdkInit(0);

    // Create device enumerate
    status = CameraEnumerateDevice(tCameraEnumList, &iCameraCounts);
    if(status != CAMERA_STATUS_SUCCESS) {
        printf("Find No Camera Devices!\n");
        goto GRAB_EXIT;
    }
    else {
        // only one(first)
        printf("Find Devices : %d\n", iCameraCounts);
        PrintDeviceInfo(&tCameraEnumList[0]);
    }

    CameraSetSysOption("NumBuffers", "2");
    // In this example, we only initialize the first camera.
    // (-1,-1) means to load the parameters saved before the last exit. If it is the first time to use the camera, then load the default parameters.
    status = CameraInit(&tCameraEnumList[0], -1, -1, &g_hCamera);
    // Fail Init
    if(status != CAMERA_STATUS_SUCCESS) {
        printf("Failed to init the camera! Error code is [%x]\n", status);
        goto GRAB_EXIT;
    }

    // Get the camera's feature description
    if(CameraGetCapability(g_hCamera, &g_tCapability) != CAMERA_STATUS_SUCCESS) goto GRAB_EXIT;
    if(CameraGetFrameStatistic(g_hCamera, &g_tFrameStatistic) != CAMERA_STATUS_SUCCESS) goto GRAB_EXIT;
    PrintFrameStatisticInfo();

    // Judging whether it is a mono camera or a color camera
    bMonoCamera = g_tCapability.sIspCapacity.bMonoSensor;
    if(bMonoCamera) {
        CameraSetIspOutFormat(g_hCamera, CAMERA_MEDIA_TYPE_MONO8);
    }else {
        CameraSetIspOutFormat(g_hCamera, CAMERA_MEDIA_TYPE_RGB8);
    } 

    //CameraGigeSetIp(&tCameraEnumList[0], "192.168.1.5", "255.255.255.0", "192.168.1.1", TRUE);

    // Init Innstance

    CameraSetLightFrequency(g_hCamera, 1);  // 0:50Hz, 1:60Hz

    pCamCtrl= new CameraControl();
    if(pCamCtrl->Init() < 0) goto GRAB_EXIT;

    // Set Camera 
    pCamCtrl->SetImageResolution(0, 200, 1920, 800);
    pCamCtrl->SetMirrorFlip(FALSE, TRUE);
    //pCamCtrl->SetFrameRate(0);  // 0:Max, others
    pCamCtrl->SetFrameRate(10);  // 0:Max, others
    pCamCtrl->SetFrameSpeed(0);

    g_pFrameBuffer = (unsigned char*)malloc(g_tCapability.sResolutionRange.iHeightMax*g_tCapability.sResolutionRange.iWidthMax*(bMonoCamera ? 1 : 3));

    // Let the SDK internal grab thread start working
    CameraPlay(g_hCamera);    

    // Create thread
	CPU_ZERO(&mask_grab);
	CPU_SET(4, &mask_grab);
    if(pthread_create(&nThreadID, NULL, GrabThread, NULL) != 0)
    {
        printf("thread create failed\n");
        goto GRAB_EXIT;
    }
    if (pthread_setaffinity_np(nThreadID, sizeof(mask_grab), &mask_grab) != 0) {
        printf("warning: could not set CPU ocr affinity, continuing...\n");
    }

    pthread_join(nThreadID, NULL);
    //PressEnterToExit();                                    

GRAB_EXIT:
    SAFE_DELETE(pCamCtrl);

    if(g_hCamera > 0) {
        // release resources
        CameraUnInit(g_hCamera);
        g_hCamera = -1;
    }

    if(g_pFrameBuffer != NULL) {
        free(g_pFrameBuffer);
        g_pFrameBuffer = NULL;
    }

    printf("exit\n");
    return 0;
}
