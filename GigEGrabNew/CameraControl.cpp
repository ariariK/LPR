/*
 *	CameraMan.cpp
 *
 * 
 *
 *
 *
 */

#include <iostream>
#include <sstream>
#include <chrono>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include "CameraControl.h"

using namespace std;
using namespace chrono;

extern int                  g_hCamera;          // device handler
extern unsigned char*       g_pFrameBuffer;	    // data buffer
extern tSdkFrameHead        g_tFrameHead;       // header info of image frame
extern tSdkCameraCapbility  g_tCapability;      // device information
extern tSdkFrameStatistic   g_tFrameStatistic;

#define FD_GPIO_ST2	        "/sys/class/gpio/gpio158/value" // 카메라 동작상태 토그 표시
#define FD_GPIO_CAM_SW	    "/sys/class/gpio/gpio146/value"	// GPIO4_C2, 스위치 신호상태(input, H:AUTO, LOW:Force OFF)
#define FD_GPIO_CAM_STATE	  "/sys/class/gpio/gpio52/value"	// GPIO1_C4, (output, 0 : Forec-OFF, 1 : AUTO)

CameraControl::CameraControl()
{ 
  fd_cam_st = -1;

  msq.msg_type = 1;
	msq.data.capWidth = 0;
	msq.data.capHeight = 0;

  msq_img.msg_type = 1;
	msq_img.data.capWidth = 0;
	msq_img.data.capHeight = 0;

  // shared memory for grab image
	Sm_Grab = new Ipcs(KEY_NUM_SM, MEM_SIZE_SM);
	Sm_Grab->SharedMemoryCreate();

	Sm_Res = new Ipcs(KEY_NUM_SM_RES, MEM_SIZE_SM_RES);
	Sm_Res->SharedMemoryCreate();

	Sm_Cam = new Ipcs(KEY_NUM_SM_CAM, MEM_SIZE_SM_CAM);
	Sm_Cam->SharedMemoryCreate();

	Mq_Grab = new Ipcs(KEY_NUM_MQ_GRAB, 0);
	Mq_Grab->MessageQueueCreate();
	msq.msg_type = 1;

	Mq_GrabImg = new Ipcs(KEY_NUM_MQ_GRAB_IMG, 0);
	Mq_GrabImg->MessageQueueCreate();
	msq_img.msg_type = 1;  
}

CameraControl::~CameraControl()
{
  close(fd_cam_st);

  Sm_Grab->SharedMemoryFree();
	Sm_Res->SharedMemoryFree();
	Sm_Cam->SharedMemoryFree();
	Mq_Grab->MessageQueueFree();
	Mq_GrabImg->MessageQueueFree();

	SAFE_DELETE(Sm_Grab);
	SAFE_DELETE(Sm_Res);
	SAFE_DELETE(Sm_Cam);
	SAFE_DELETE(Mq_Grab);
	SAFE_DELETE(Mq_GrabImg);
}

int CameraControl::Init()
{
  fd_cam_st = open(FD_GPIO_ST2, O_WRONLY);
  if (fd_cam_st == -1) {
		ERR_LOG(string("Unable to open /sys/class/gpio/gpio158/value"));
    return -1;
	}

  CameraGetAeState(g_hCamera, &st_cam2.bAeState);
  CameraGetLightFrequency(g_hCamera, &st_cam2.iFrequencySel);
  //CameraGetAeExposureRange(g_hCamera, &st_cam.expMin, &st_cam.expMax);
  CameraGetExposureTimeRange(g_hCamera, &st_cam.expMin, &st_cam.expMax, &st_cam2.expStep);

  return 0;
}

CameraSdkStatus CameraControl::SetImageResolution(int offsetx, int offsety, int width, int height)
{
    tSdkImageResolution sRoiResolution = { 0 };

    //CameraGetImageResolution(g_hCamera, &sRoiResolution);
    // 设置成0xff表示自定义分辨率，设置成0到N表示选择预设分辨率
    // Set to 0xff for custom resolution, set to 0 to N for select preset resolution
    sRoiResolution.iIndex = 0xff;

    // iWidthFOV表示相机的视场宽度，iWidth表示相机实际输出宽度
    // 大部分情况下iWidthFOV=iWidth。有些特殊的分辨率模式如BIN2X2：iWidthFOV=2*iWidth，表示视场是实际输出宽度的2倍
    // iWidthFOV represents the camera's field of view width, iWidth represents the camera's actual output width
    // In most cases iWidthFOV=iWidth. Some special resolution modes such as BIN2X2:iWidthFOV=2*iWidth indicate that the field of view is twice the actual output width
    sRoiResolution.iWidth = width;
    sRoiResolution.iWidthFOV = width;

    // 高度，参考上面宽度的说明
    // height, refer to the description of the width above
    sRoiResolution.iHeight = height;
    sRoiResolution.iHeightFOV = height;

    // 视场偏移
    // Field of view offset
    sRoiResolution.iHOffsetFOV = offsetx;
    sRoiResolution.iVOffsetFOV = offsety;

    // ISP软件缩放宽高，都为0则表示不缩放
    // ISP software zoom width and height, all 0 means not zoom
    sRoiResolution.iWidthZoomSw = 0;
    sRoiResolution.iHeightZoomSw = 0;

    // BIN SKIP 模式设置（需要相机硬件支持）
    // BIN SKIP mode setting (requires camera hardware support)
    sRoiResolution.uBinAverageMode = 0;
    sRoiResolution.uBinSumMode = 0;
    sRoiResolution.uResampleMask = 0;
    sRoiResolution.uSkipMode = 0;
    return CameraSetImageResolution(g_hCamera, &sRoiResolution);
}

CameraSdkStatus CameraControl::SetMirrorFlip(BOOL mirror, BOOL flip)
{
  CameraSdkStatus status = CAMERA_STATUS_SUCCESS;

  BOOL bMirror  = mirror;
  BOOL bFlip    = flip;

  status = CameraSetHardwareMirror(g_hCamera, 0, bMirror);  // 0: hor, 1: ver
  status = CameraSetHardwareMirror(g_hCamera, 1, bFlip);    // 0: hor, 1: ver
  if(status == CAMERA_STATUS_SUCCESS)
  { 
    CameraGetHardwareMirror(g_hCamera, 0, &bMirror);
    CameraGetHardwareMirror(g_hCamera, 1, &bFlip);
    printf("Set Mirror : %d, Flip : %d\n", bMirror, bFlip);
  }
  
  return status;
}

CameraSdkStatus CameraControl::SetFrameRate(int value)
{
  int nValue = value;

  CameraSdkStatus status = CameraSetFrameRate(g_hCamera, nValue);
  if(status == CAMERA_STATUS_SUCCESS)
  { 
    CameraGetFrameRate(g_hCamera, &nValue);
    printf("Set Frame Rate : %d\n", nValue);
  }
  
  return status;
}

CameraSdkStatus CameraControl::SetFrameSpeed(int value)
{
  int nValue = (g_tCapability.iFrameSpeedDesc < value) ? value : g_tCapability.iFrameSpeedDesc;

  //printf("iFrameSpeedDesc : %d\n", g_tCapability.iFrameSpeedDesc);

  CameraSdkStatus status = CameraSetFrameSpeed(g_hCamera, nValue-1);
  if(status == CAMERA_STATUS_SUCCESS)
  { 
    CameraGetFrameSpeed(g_hCamera, &nValue);
    printf("Set Frame Speed : %d\n", nValue);
  }
  
  return status;
}

int CameraControl::CtrlGPIO(int fd, int value)
{
  if(value)
	{
		if (write(fd, "1", 1) != 1) 
		{
			ERR_LOG(string("Error writing to /sys/class/gpio/gpio158/value"));
		}
	}
	else
	{
		if (write(fd, "0", 1) != 1) 
		{
			ERR_LOG(string("Error writing to /sys/class/gpio/gpio158/value"));
		}
	}

	return 0;
}

void CameraControl::PostProcess()
{
  CameraGetAeTarget(g_hCamera, &st_cam2.itarAE);
  CameraGetExposureTime(g_hCamera, &st_cam.expCur);
  CalcGrabberStat();

  // LED - ST2 동작상태표시
  CtrlGPIO(fd_cam_st, (st_cam.capCount>>2)&0x1);

#if 0
  printf("=========== PostProcess ================\n");
  printf("AE Enable State(0:off, 1:on) : %d\n", st_cam2.bAeState);
  printf("LightFrequence(0:50Hz, 1:60Hz) : %d\n", st_cam2.iFrequencySel);
  printf("Target AE : %d\n", st_cam2.itarAE);
  printf("st_cam.expCur : %f [%f ~ %f](step:%f)\n", st_cam.expCur, st_cam.expMin, st_cam.expMax, st_cam2.expStep);
  printf("========================================\n");
#endif
}

void CameraControl::CalcGrabberStat()
{
  // elapsed time : start
  static double pre_time = 0;
  static system_clock::time_point start = system_clock::now();
  system_clock::time_point end = system_clock::now();
  nanoseconds nano = end - start;

  double time_sec = nano.count()/1000000000;

  CameraGetFrameStatistic(g_hCamera, &g_tFrameStatistic);
  st_cam.capCount = g_tFrameStatistic.iCapture;
  st_cam.capFPS   = st_cam.capCount/time_sec;
  
  //if(((int)time_sec%10) == 0) {
  if((time_sec - pre_time) >= 10) {
    //printf("Running TIME : %f[sec]\n", time_sec);
    //printf("Captured FPS : %f\n", st_cam.capFPS);

    msg = "Grabbed image: " + string_format("%09d", st_cam.capCount) + ", width = " + to_string(st_cam.capWidth) + ", height = " + to_string(st_cam.capHeight) + \
					"\tAverage FPS: " + string_format("%.2f", st_cam.capFPS);
    INFO_LOG(msg);          
    pre_time = time_sec;
  }
}

CameraSdkStatus CameraControl::GetFrameData(BYTE *pData)
{
  CameraSdkStatus status = -1;

#if 1
  status = CameraGetImageBuffer(g_hCamera, &g_tFrameHead, &g_pFrameBuffer, 2000);
  if(status == CAMERA_STATUS_SUCCESS)
  {
#if false // for debug
    static system_clock::time_point start = system_clock::now();
    system_clock::time_point end = system_clock::now();
    nanoseconds nano = end - start;
    double time_msec = nano.count()/1000000;
    printf("CameraGetImageBuffer TIME : %f[msec]\n", time_msec);
#endif

    CameraImageProcess(g_hCamera, pData, g_pFrameBuffer, &g_tFrameHead);
    CameraReleaseImageBuffer(g_hCamera, pData);

    // Update
    int width   = g_tFrameHead.iWidth;
    int height  = g_tFrameHead.iHeight;

    // Use message queue
    if (msq.data.capWidth != (int)width || msq.data.capHeight != (int)height || Mq_Grab->MessageQueueQNum() < 1)
    {
      msq.data.capWidth   = width;
      msq.data.capHeight  = height;
      Mq_Grab->MessageQueueWrite((char*)&msq);
    }

    // Use shared memory
    if (st_grab.capWidth != (int)width || st_grab.capHeight != (int)height)
    {
      st_grab.capWidth  = width;
      st_grab.capHeight = height;
      Sm_Res->SharedMemoryWrite((char *)&st_grab, sizeof(struct grab_data));
    }

    // write to shared memory
    Sm_Grab->SharedMemoryWrite((char*)g_pFrameBuffer, width*height);
    Sm_Cam->SharedMemoryWrite((char *)&st_cam, sizeof(struct cam_param));

    //////////////////////////////////////////////////////////////////////////////////////////////
    // Update for lpr
    // Use message queue
    if (Mq_GrabImg->MessageQueueQNum() < 1)
    {
      msq.data.capWidth   = width;
      msq.data.capHeight  = height;
      Mq_GrabImg->MessageQueueWrite((char*)&msq);
    }
    //////////////////////////////////////////////////////////////////////////////////////////////

    st_cam.capWidth 	= st_grab.capWidth;
    st_cam.capHeight	= st_grab.capHeight;
  }
  else 
  {
    printf("Error!!! CameraGetImageBuffer : %d\n", status);
  }
#endif

  return status;
}