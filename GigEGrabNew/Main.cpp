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

#if true
static void* GrabThread(void* pParam)
{
    CameraSdkStatus status;
    //BYTE*           pRawData;

    INFO_LOG(string("*** IMAGE ACQUISITION ***"));

    while(1) {
        status = pCamCtrl->GetFrameData();
        if(status == CAMERA_STATUS_SUCCESS) {
            // post processing
            if(pCamCtrl->PostProcess() < 0) g_bExit = TRUE;
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

        usleep(1000);   // 
    }

    return 0;
}
#endif

#if false
static void* GrabThread(void* pParam)
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

#if false//true
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
/*
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
*/
            g_read_fps++;
            printf("Captured image : %d\n", g_read_fps);
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

int main(int argc, char** argv)
{
    CameraSdkStatus         status = -1;
    tSdkCameraDevInfo       tCameraEnumList[4];
    BOOL                    bMonoCamera = true;
    int                     iCameraCounts = 4;
    //int                     iValue;
    //double                  dwValue;

    signal(SIGINT, (void (*)(int))signalHandler);
    signal(SIGQUIT, (void (*)(int))signalHandler);

    pthread_t nThreadID;
	cpu_set_t mask_grab;

    openlog(LOG_NAME, LOG_PID, LOG_USER);

    msg = "LOG_LEVEL = " + to_string(LOG_LEVEL);
	INFO_LOG(msg);

    //===============================================================================================================
    // User Parameters
    //===============================================================================================================
    int nCapWidth 			= GIGE_CAMERA_CAP_WIDTH_FHD;
	int nCapHeight			= GIGE_CAMERA_CAP_HEIHGT_FHD;
	int nOffsetX			= GIGE_CAMERA_OFFSET_X;
	int nOffsetY			= GIGE_CAMERA_OFFSET_Y;
	//bool bFrameRateMode     = GIGE_CAMERA_FRAME_RATE_ENABLE ? true : false;
	float fFrameRate		= GIGE_CAMERA_FRAME_RATE;
	//bool bSaveEnable		= false;
    float fShutterMin       = 1000.0;   // 1msec
    float fShutterMax       = 33000.0;  // 33msec
    float fGainMin          = 1.0;      // 1db
    float fGainMax          = 8.0;      // 64db
    float fExposureLow	    = 30.0;     // 30% : trun off
	float fExposureHigh	    = 70.0;     // 70% : turn on
    int nTargetAE           = 120;
    int nAE_Win_x           = 320;
    int nAE_Win_y           = 200;
    int nAE_Win_w           = 1280;
    int nAE_Win_h           = 500;

	float fNightGainLow		=  5.0;
	float fNightGainHigh	= 20.0;
	
    // image process
	bool bImgProcEn			= true;
	bool bGammaEn			= true;
	bool bSharpEn  			= true;

	//float fGamma			= 1.5;  
    //int nSharpAuto			= 0;	// 0: off, 2: continuous

    int nGamma              = 100;
	int nSharp				= 0;
    int nContrast           = 100;
    int nSaturation         = 100;
    int nLutMode            = 0;
    int nLutSel             = 0;

    // add. by ariari : 2022.11.22 - begin
    int draw_en             = 0;
    int roi_sx              = nAE_Win_x;
    int roi_sy              = nAE_Win_y;
    int roi_w               = nAE_Win_w;
    int roi_h               = nAE_Win_h;
    // add. by ariari : 2022.11.22 - end

    //===============================================================================================================
    ifstream    cFile;
    string      savePath;
	int res;
	string file_cfg = "/oem/config_lpr.txt";
	while ((res=getopt(argc, argv, "f:h")) != -1) {
		switch(res) {
		case 'f':   // configure file
			file_cfg = optarg;
		  break;

		case 'h':
      std::cout << " [Usage]: " << argv[0] << " [-h]\n"
	              << " [-f configure file]\n";
			break;
		}
	}

	cFile.open(file_cfg);
	if (cFile.is_open())
	{
        string line;
        while(getline(cFile, line)) {
            line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());

			if (line[0] == '#' || line.empty())
			{
				continue;
			}

			auto delimiterPos = line.find("=");
			auto name = line.substr(0, delimiterPos);
			auto value = line.substr(delimiterPos + 1);
			
			cout << name << " " << value << endl;

            /////////////////////////////////////////////////////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////////////////////////////////////
			if (name.compare("capWidth") == 0)
			{
				nCapWidth = stoi(value);
			}
			else if (name.compare("capHeight") == 0)
			{
				nCapHeight = stoi(value);
			}
			else if (name.compare("offsetX") == 0)
			{
				nOffsetX = stoi(value);
			}
			else if (name.compare("offsetY") == 0)
			{
				nOffsetY = stoi(value);
			}
			//else if (name.compare("frameRateMode") == 0)
			//{
			//	bFrameRateMode = stoi(value) ? true : false;
			//}
			else if (name.compare("frameRate") == 0)
			{
				fFrameRate = stof(value);
			}
            else if (name.compare("targetAE") == 0)
			{
				nTargetAE = stoi(value);
			}
			else if (name.compare("shutterMin") == 0)
			{
				fShutterMin = stof(value);
			}
            else if (name.compare("shutterMax") == 0)
			{
				fShutterMax = stof(value);
			}
            else if (name.compare("gainMin") == 0)
			{
				fGainMin = stof(value);
			}
            else if (name.compare("gainMax") == 0)
			{
				fGainMax = stof(value);
			}
			else if (name.compare("exposureLow") == 0)
			{
				fExposureLow = stof(value);
			}
			else if (name.compare("exposureHigh") == 0)
			{
				fExposureHigh = stof(value);
			}
            else if (name.compare("nightGainLow") == 0)
			{
				fNightGainLow = stof(value);
			}
			else if (name.compare("nightGainHigh") == 0)
			{
				fNightGainHigh = stof(value);
			}
            else if (name.compare("ae_win_x") == 0)
			{
				nAE_Win_x = stoi(value);
			}
            else if (name.compare("ae_win_y") == 0)
			{
				nAE_Win_y = stoi(value);
			}
            else if (name.compare("ae_win_w") == 0)
			{
				nAE_Win_w = stoi(value);
			}
            else if (name.compare("ae_win_h") == 0)
			{
				nAE_Win_h = stoi(value);
			}
			//else if (name.compare("saveEnable") == 0)
			//{
			//	bSaveEnable = stoi(value) ? true : false;
			//}
			else if (name.compare("savePath") == 0)
			{
				savePath = value;
			}
			
			else if (name.compare("imgProcEnable") == 0)
			{
				bImgProcEn = stoi(value) ? true : false;
			}
			else if (name.compare("gammaEnable") == 0)
			{
				bGammaEn = stoi(value) ? true : false;
			}
			else if (name.compare("sharpEnable") == 0)
			{
				bSharpEn = stoi(value) ? true : false;
			}
			else if (name.compare("gamma") == 0)
			{
				nGamma = stoi(value);
			}
			//else if (name.compare("sharpAuto") == 0)
			//{
			//	nSharpAuto = stoi(value);
			//}
			else if (name.compare("sharp") == 0)
			{
				nSharp = stoi(value);
			}
            else if (name.compare("contrast") == 0)
			{
				nContrast = stoi(value);
			}
            else if (name.compare("saturation") == 0)
			{
				nSaturation = stoi(value);
			}
            else if (name.compare("lutMode") == 0)
			{
				nLutMode = stoi(value);
			}
            else if (name.compare("lutSel") == 0)
			{
				nLutSel = stoi(value);
			}
            // add. by ariari : 2022.11.22 - begin
            else if (name.compare("draw_en") == 0)
			{
				draw_en = stoi(value);
			}
            else if (name.compare("roi_sx") == 0)
			{
				roi_sx = stoi(value);
			}
            else if (name.compare("roi_sy") == 0)
            {
                roi_sy = stoi(value);
            }
            else if (name.compare("roi_w") == 0)
            {
                roi_w = stoi(value);
            }
            else if (name.compare("roi_h") == 0)
            {
                roi_h = stoi(value);
            }
            // add. by ariari : 2022.11.22 - end
			/////////////////////////////////////////////////////////////////////////////////////////////////////////

        };  // while
        cFile.close();
    }
    else 
	{
		WARN_LOG(string("Couldn't open config file for reading..."));
		EMERG_LOG(string("Run defalut mode"));

		return -1;
	}

    // config params
	INFO_LOG(string("#################################################################################################"));
	INFO_LOG(string("# GigEGrab Configs                                                                              #"));
	INFO_LOG(string("#################################################################################################"));
	msg = ">> capWidth : " + to_string(nCapWidth);
	INFO_LOG(msg);
	msg = ">> nCapHeight : " + to_string(nCapHeight);
	INFO_LOG(msg);
	msg = ">> nOffsetX : " + to_string(nOffsetX);
	INFO_LOG(msg);
	msg = ">> nOffsetY : " + to_string(nOffsetY);
	INFO_LOG(msg);
	msg = ">> fFrameRate : " + to_string(fFrameRate);
	INFO_LOG(msg);
	
    msg = ">> nTargetAE : " + to_string(nTargetAE);
	INFO_LOG(msg);
	msg = ">> fShutterMin : " + to_string(fShutterMin);
	INFO_LOG(msg);
    msg = ">> fShutterMax : " + to_string(fShutterMax);
	INFO_LOG(msg);
    msg = ">> fGainMin : " + to_string(fGainMin);
	INFO_LOG(msg);
    msg = ">> fGainMax : " + to_string(fGainMax);
	INFO_LOG(msg);

    msg = ">> fNightGainLow : " + to_string(fNightGainLow);
	INFO_LOG(msg);
	msg = ">> fNightGainHigh : " + to_string(fNightGainHigh);
	INFO_LOG(msg);
	msg = ">> fExposureLow : " + to_string(fExposureLow);
	INFO_LOG(msg);
	msg = ">> fExposureHigh : " + to_string(fExposureHigh);
	INFO_LOG(msg);
    msg = ">> nAE_Win_x : " + to_string(nAE_Win_x);
	INFO_LOG(msg);
    msg = ">> nAE_Win_y : " + to_string(nAE_Win_y);
	INFO_LOG(msg);
    msg = ">> nAE_Win_w : " + to_string(nAE_Win_w);
	INFO_LOG(msg);
    msg = ">> nAE_Win_h : " + to_string(nAE_Win_h);
	INFO_LOG(msg);
	// add. by ariari : 2022.05.18 - begin
	msg = ">> bImgProcEn : " + to_string(bImgProcEn);
	INFO_LOG(msg);
	msg = ">> bGammaEn : " + to_string(bGammaEn);
	INFO_LOG(msg);
	msg = ">> bSharpEn : " + to_string(bSharpEn);
	INFO_LOG(msg);
	msg = ">> nGamma : " + to_string(nGamma);
	INFO_LOG(msg);
	//msg = ">> nSharpAuto : " + to_string(nSharpAuto);
	//INFO_LOG(msg);
	msg = ">> nSharp : " + to_string(nSharp);
	INFO_LOG(msg);
    msg = ">> nContrast : " + to_string(nContrast);
	INFO_LOG(msg);
    msg = ">> nSaturation : " + to_string(nSaturation);
	INFO_LOG(msg);
    msg = ">> nLutMode : " + to_string(nLutMode);
	INFO_LOG(msg);
    msg = ">> nLutSel : " + to_string(nLutSel);
	INFO_LOG(msg);
	// add. by ariari : 2022.05.18 - end
	INFO_LOG(string("#################################################################################################"));

    //===============================================================================================================


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
    //status = CameraInit(&tCameraEnumList[0], PARAM_MODE_BY_SN, PARAMETER_TEAM_DEFAULT, &g_hCamera);
    //status = CameraInit(&tCameraEnumList[0], -1, PARAMETER_TEAM_DEFAULT, &g_hCamera);
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
    //===============================================================================================================
    // Create Instance...
    //===============================================================================================================
    pCamCtrl= new CameraControl();
    pCamCtrl->st_cam2.stSystem.hoff         = nOffsetX;
    pCamCtrl->st_cam2.stSystem.voff         = nOffsetY;
    pCamCtrl->st_cam2.stSystem.capWidth     = nCapWidth;
    pCamCtrl->st_cam2.stSystem.capHeight    = nCapHeight;
    pCamCtrl->st_cam2.stSystem.bMirror      = FALSE;
    pCamCtrl->st_cam2.stSystem.bFlip        = TRUE;
    pCamCtrl->st_cam2.stSystem.framerate    = fFrameRate;

    pCamCtrl->st_cam2.stAE.itarAE           = nTargetAE;
    pCamCtrl->st_cam2.stAE.expMin           = fShutterMin;
    pCamCtrl->st_cam2.stAE.expMax           = fShutterMax;
    pCamCtrl->st_cam2.stAE.gainStep         = 0.1;
    pCamCtrl->st_cam2.stAE.gainMin          = fGainMin;
    pCamCtrl->st_cam2.stAE.gainMax          = fGainMax;
    pCamCtrl->st_cam2.stAE.againMin         = fGainMin/pCamCtrl->st_cam2.stAE.gainStep;
    pCamCtrl->st_cam2.stAE.againMax         = fGainMax/pCamCtrl->st_cam2.stAE.gainStep;
    pCamCtrl->st_cam2.stAE.irTurnOnValue    = fExposureHigh;
    pCamCtrl->st_cam2.stAE.irTurnOffValue   = fExposureLow;
    pCamCtrl->st_cam2.stAE.ae_win_x         = nAE_Win_x;
    pCamCtrl->st_cam2.stAE.ae_win_y         = nAE_Win_y;
    pCamCtrl->st_cam2.stAE.ae_win_w         = nAE_Win_w;
    pCamCtrl->st_cam2.stAE.ae_win_h         = nAE_Win_h;
    
    pCamCtrl->st_cam2.stEnhance.iGamma      = nGamma;
    pCamCtrl->st_cam2.stEnhance.iSharpness  = nSharp;
    pCamCtrl->st_cam2.stEnhance.iContrast   = nContrast;
    pCamCtrl->st_cam2.stEnhance.iSaturation = nSaturation;
    pCamCtrl->st_cam2.stEnhance.iLutMode    = nLutMode;
    pCamCtrl->st_cam2.stEnhance.iLutSel     = nLutSel;

    // add. by ariari - 2022.11.22 - begin
    pCamCtrl->st_cam.draw_en                = draw_en;
    pCamCtrl->st_cam.roi_sx                 = roi_sx;
    pCamCtrl->st_cam.roi_sy                 = roi_sy;
    pCamCtrl->st_cam.roi_w                  = roi_w;
    pCamCtrl->st_cam.roi_h                  = roi_h;
    // add. by ariari - 2022.11.22 - end

    pCamCtrl->st_cam2.stIO.iIOIndex = 0;
    if(pCamCtrl->Init() < 0) goto GRAB_EXIT;
    //===============================================================================================================

#if false    // add. by ariari : 2022.11.07
    // Set Camera 
    pCamCtrl->SetImageResolution(0, 200, 1920, 800);
    pCamCtrl->SetMirrorFlip(FALSE, TRUE);
    //pCamCtrl->SetFrameRate(0);  // 0:Max, others
    pCamCtrl->SetFrameRate(10);  // 0:Max, others
    pCamCtrl->SetFrameSpeed(0);
#endif

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
