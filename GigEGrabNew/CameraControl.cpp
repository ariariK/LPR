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
  st_cam2 = {0};

  fd_cam_st = -1;

  msq.msg_type            = 1;
	msq.data.capWidth       = 0;
	msq.data.capHeight      = 0;

  msq_img.msg_type        = 1;
	msq_img.data.capWidth   = 0;
	msq_img.data.capHeight  = 0;

  // shared memory for grab image
	Sm_Grab = new Ipcs(KEY_NUM_SM, MEM_SIZE_SM);
	Sm_Grab->SharedMemoryCreate();

  Sm_GrabLpr = new Ipcs(KEY_NUM_SM2, MEM_SIZE_SM2);
	Sm_GrabLpr->SharedMemoryCreate();

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
  Sm_GrabLpr->SharedMemoryFree();
	Sm_Res->SharedMemoryFree();
	Sm_Cam->SharedMemoryFree();
	Mq_Grab->MessageQueueFree();
	Mq_GrabImg->MessageQueueFree();

	SAFE_DELETE(Sm_Grab);
  SAFE_DELETE(Sm_GrabLpr);
	SAFE_DELETE(Sm_Res);
	SAFE_DELETE(Sm_Cam);
	SAFE_DELETE(Mq_Grab);
	SAFE_DELETE(Mq_GrabImg);
}

void CameraControl::PrintDeviceInfo()
{
  printf("=========== tSdkCameraCapbility - tSharpnessRange ================\n");
  printf("sSharpnessRange :  iMin=%d, iMax=%d\n", g_tCapability.sSharpnessRange.iMin, g_tCapability.sSharpnessRange.iMax);

  int iSharpness;
  CameraGetSharpness(g_hCamera, &iSharpness);
  printf("iSharpness :  iSharpness=%d\n", iSharpness);

  int nISP=255;
  CameraSetIspProcessor(g_hCamera, nISP);
  CameraGetIspProcessor(g_hCamera, &nISP);
  printf("iSharpness :  CameraGetIspProcessor=%d\n", nISP);
  printf("================================================\n");

  //CameraSaveParameterToFile(g_hCamera, "/oem/cam.config");
  //CameraReadParameterFromFile(g_hCamera, "/oem/lpr.config");
}

int CameraControl::Init()
{
  fd_cam_st = open(FD_GPIO_ST2, O_WRONLY);
  if (fd_cam_st == -1) {
		ERR_LOG(string("Unable to open /sys/class/gpio/gpio158/value"));
    return -1;
	}

  PrintDeviceInfo();
 
  
  CameraSetGPIOs();
  CameraSetSystem();
  CameraSetAE();
  CameraEnhancement();

  //PrintDeviceInfo();
  
  return 0;
}

CameraSdkStatus CameraControl::SetAeExposureRange(double dwMinValue, double dwMaxValue)
{
  return CameraSetAeExposureRange(g_hCamera, dwMinValue, dwMaxValue);
}

CameraSdkStatus CameraControl::SetImageResolution(int offsetx, int offsety, int width, int height)
{
    tSdkImageResolution sRoiResolution = { 0 };

    //CameraGetImageResolution(g_hCamera, &sRoiResolution);
    // 设置成0xff表示自定义分辨率，设置成0到N表示选择预设分辨率
    // Set to 0xff for custom resolution, set to 0 to N for select preset resolution
    sRoiResolution.iIndex     = 0xff;

    // iWidthFOV表示相机的视场宽度，iWidth表示相机实际输出宽度
    // 大部分情况下iWidthFOV=iWidth。有些特殊的分辨率模式如BIN2X2：iWidthFOV=2*iWidth，表示视场是实际输出宽度的2倍
    // iWidthFOV represents the camera's field of view width, iWidth represents the camera's actual output width
    // In most cases iWidthFOV=iWidth. Some special resolution modes such as BIN2X2:iWidthFOV=2*iWidth indicate that the field of view is twice the actual output width
    sRoiResolution.iWidth     = width;
    sRoiResolution.iWidthFOV  = width;

    // 高度，参考上面宽度的说明
    // height, refer to the description of the width above
    sRoiResolution.iHeight    = height;
    sRoiResolution.iHeightFOV = height;

    // 视场偏移
    // Field of view offset
    sRoiResolution.iHOffsetFOV = offsetx;
    sRoiResolution.iVOffsetFOV = offsety;

    // ISP软件缩放宽高，都为0则表示不缩放
    // ISP software zoom width and height, all 0 means not zoom
    sRoiResolution.iWidthZoomSw   = 0;
    sRoiResolution.iHeightZoomSw  = 0;

    // BIN SKIP 模式设置（需要相机硬件支持）
    // BIN SKIP mode setting (requires camera hardware support)
    sRoiResolution.uBinAverageMode  = 0;
    sRoiResolution.uBinSumMode      = 0;
    sRoiResolution.uResampleMask    = 0;
    sRoiResolution.uSkipMode        = 0;
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

CameraSdkStatus CameraControl::CameraSetSystem()
{
  CameraSdkStatus status = -1;

  // 캡쳐영역 설정(소나타기준 기존 설정값 적용)
  status = SetImageResolution(st_cam2.stSystem.hoff, st_cam2.stSystem.voff, st_cam2.stSystem.capWidth, st_cam2.stSystem.capHeight);
  if(status != CAMERA_STATUS_SUCCESS)
  {
    msg = string_format("SetImageResolution : %d", status);
    ERR_LOG(msg);
    printf("[ERROR]SetImageResolution : %d\n", status);
    return status;
  }
  // Mirror : off, Flip : on
  status = SetMirrorFlip(st_cam2.stSystem.bMirror, st_cam2.stSystem.bFlip);               
  if(status != CAMERA_STATUS_SUCCESS)
  {
    msg = string_format("SetMirrorFlip : %d", status);
    ERR_LOG(msg);
    printf("[ERROR]SetMirrorFlip : %d\n", status);
    return status;
  }

  // Set Frame Rate (0:Max, others:fps)
  status = SetFrameRate(st_cam2.stSystem.framerate);
  if(status != CAMERA_STATUS_SUCCESS)
  {
    msg = string_format("SetFrameRate : %d", status);
    ERR_LOG(msg);
    printf("[ERROR]SetFrameRate : %d\n", status);
    return status;
  }
  status = SetFrameSpeed(0);
  if(status != CAMERA_STATUS_SUCCESS)
  {
    msg = string_format("SetFrameSpeed : %d", status);
    ERR_LOG(msg);
    printf("[ERROR]SetFrameSpeed : %d\n", status);
    return status;
  }
  
  st_cam2.stSystem.iFrequencySel = 1;    // 0:50Hz, 1:60Hz
  status = CameraSetLightFrequency(g_hCamera, st_cam2.stSystem.iFrequencySel);
  if(status != CAMERA_STATUS_SUCCESS)
  {
    msg = string_format("CameraSetLightFrequency : %d", status);
    ERR_LOG(msg);
    printf("[ERROR]CameraSetLightFrequency : %d\n", status);
    return status;
  }
  status = CameraGetLightFrequency(g_hCamera, &st_cam2.stSystem.iFrequencySel);
  if(status != CAMERA_STATUS_SUCCESS)
  {
    msg = string_format("CameraGetLightFrequency : %d", status);
    ERR_LOG(msg);
    printf("[ERROR]CameraGetLightFrequency : %d\n", status);
    return status;
  }

  return status;
}

CameraSdkStatus CameraControl::CameraSetAE()
{
  CameraSdkStatus status = -1;

  // AE Mode : 
  st_cam2.stSystem.bAeState = 1;   // Auto
  status = CameraSetAeState(g_hCamera, st_cam2.stSystem.bAeState);
  if(status != CAMERA_STATUS_SUCCESS)
  {
    msg = string_format("CameraSetAeState : %d", status);
    ERR_LOG(msg);
    printf("[ERROR]CameraSetAeState : %d\n", status);
    return status;
  }
  status = CameraGetAeState(g_hCamera, &st_cam2.stSystem.bAeState);
  if(status != CAMERA_STATUS_SUCCESS)
  {
    msg = string_format("CameraGetAeState : %d", status);
    ERR_LOG(msg);
    printf("[ERROR]CameraGetAeState : %d\n", status);
    return status;
  }
  
  status = CameraSetAeTarget(g_hCamera, st_cam2.stAE.itarAE);
  if(status != CAMERA_STATUS_SUCCESS)
  {
    msg = string_format("CameraSetAeTarget : %d", status);
    ERR_LOG(msg);
    printf("[ERROR]CameraSetAeTarget : %d\n", status);
    return status;
  }

  int nAEThreshold = 1;
  status = CameraSetAeThreshold(g_hCamera, nAEThreshold);
  if(status != CAMERA_STATUS_SUCCESS)
  {
    msg = string_format("CameraSetAeThreshold : %d", status);
    ERR_LOG(msg);
    printf("[ERROR]CameraSetAeThreshold : %d\n", status);
    return status;
  }
  
  // Auto Exposure Range(셔텨 레인지, IR Strobe 연동), 단위 microsec, 기본값[10~확인필요]
  status = SetAeExposureRange(st_cam2.stAE.expMin, st_cam2.stAE.expMax);
  if(status != CAMERA_STATUS_SUCCESS)
  {
    msg = string_format("SetAeExposureRange : %d", status);
    ERR_LOG(msg);
    printf("[ERROR]SetAeExposureRange : %d\n", status);
    return status;
  }
  status = CameraGetAeExposureRange(g_hCamera, &st_cam2.stAE.expMin, &st_cam2.stAE.expMax);
  if(status != CAMERA_STATUS_SUCCESS)
  {
    msg = string_format("CameraGetAeExposureRange : %d", status);
    ERR_LOG(msg);
    printf("[ERROR]CameraGetAeExposureRange : %d\n", status);
    return status;
  }
  status = CameraSetAeAnalogGainRange(g_hCamera, st_cam2.stAE.againMin, st_cam2.stAE.againMax);
  if(status != CAMERA_STATUS_SUCCESS)
  {
    msg = string_format("CameraSetAeAnalogGainRange : %d", status);
    ERR_LOG(msg);
    printf("[ERROR]CameraSetAeAnalogGainRange : %d\n", status);
    return status;
  }
  status = CameraGetAeAnalogGainRange(g_hCamera, &st_cam2.stAE.againMin, &st_cam2.stAE.againMax);
  if(status != CAMERA_STATUS_SUCCESS)
  {
    msg = string_format("CameraGetAeAnalogGainRange : %d", status);
    ERR_LOG(msg);
    printf("[ERROR]CameraGetAeAnalogGainRange : %d\n", status);
    return status;
  }
#if false  
  status = CameraGetAnalogGainXRange(g_hCamera, &st_cam2.stAE.gainMin, &st_cam2.stAE.gainMax, &st_cam2.stAE.gainStep);
  if(status != CAMERA_STATUS_SUCCESS)
  {
    msg = string_format("CameraGetAnalogGainXRange : %d", status);
    ERR_LOG(msg);
    printf("[ERROR]CameraGetAnalogGainXRange : %d\n", status);
    return status;
  }
#endif  
  //int again = 10;
  //CameraSetAnalogGain(g_hCamera, again); // def = 80

  // center
  //CameraSetAeWindow(g_hCamera, st_cam2.stSystem.capWidth>>2, st_cam2.stSystem.capHeight>>2, st_cam2.stSystem.capWidth>>1, st_cam2.stSystem.capHeight>>1);
  // full
  //CameraSetAeWindow(g_hCamera, 0, 0, st_cam2.stSystem.capWidth, st_cam2.stSystem.capHeight);
  // user
  CameraSetAeWindow(g_hCamera, st_cam2.stAE.ae_win_x, st_cam2.stAE.ae_win_y, st_cam2.stAE.ae_win_w, st_cam2.stAE.ae_win_h);


  return status;
}

CameraSdkStatus CameraControl::CameraEnhancement()
{
  CameraSdkStatus status = -1;

  status = CameraSetGamma(g_hCamera, st_cam2.stEnhance.iGamma);
  if(status != CAMERA_STATUS_SUCCESS)
  {
    msg = string_format("CameraSetGamma : %d", status);
    ERR_LOG(msg);
    printf("[ERROR]CameraSetGamma : %d\n", status);
    return status;
  }
  status = CameraGetGamma(g_hCamera, &st_cam2.stEnhance.iGamma);
  if(status != CAMERA_STATUS_SUCCESS)
  {
    msg = string_format("CameraGetGamma : %d", status);
    ERR_LOG(msg);
    printf("[ERROR]CameraGetGamma : %d\n", status);
    return status;
  }

  status = CameraSetSharpness(g_hCamera, st_cam2.stEnhance.iSharpness);
  if(status != CAMERA_STATUS_SUCCESS)
  {
    msg = string_format("CameraSetSharpness : %d", status);
    ERR_LOG(msg);
    printf("[ERROR]CameraSetSharpness : %d\n", status);
    return status;
  }
  status = CameraGetSharpness(g_hCamera, &st_cam2.stEnhance.iSharpness);
  if(status != CAMERA_STATUS_SUCCESS)
  {
    msg = string_format("CameraGetSharpness : %d", status);
    ERR_LOG(msg);
    printf("[ERROR]CameraGetSharpness : %d\n", status);
    return status;
  }

  status = CameraSetContrast(g_hCamera, st_cam2.stEnhance.iContrast);
  if(status != CAMERA_STATUS_SUCCESS)
  {
    msg = string_format("CameraSetContrast : %d", status);
    ERR_LOG(msg);
    printf("[ERROR]CameraSetContrast : %d\n", status);
    return status;
  }
  status = CameraGetContrast(g_hCamera, &st_cam2.stEnhance.iContrast);
  if(status != CAMERA_STATUS_SUCCESS)
  {
    msg = string_format("CameraGetContrast : %d", status);
    ERR_LOG(msg);
    printf("[ERROR]CameraGetContrast : %d\n", status);
    return status;
  }

#if false
  status = CameraSetSaturation(g_hCamera, st_cam2.stEnhance.iSaturation);
  if(status != CAMERA_STATUS_SUCCESS)
  {
    msg = string_format("CameraSetSaturation : %d", status);
    ERR_LOG(msg);
    printf("[ERROR]CameraSetSaturation : %d\n", status);
    return status;
  }
  status = CameraGetSaturation(g_hCamera, &st_cam2.stEnhance.iSaturation);
  if(status != CAMERA_STATUS_SUCCESS)
  {
    msg = string_format("CameraGetSaturation : %d", status);
    ERR_LOG(msg);
    printf("[ERROR]CameraGetSaturation : %d\n", status);
    return status;
  }
#endif

  status = CameraSetLutMode(g_hCamera, st_cam2.stEnhance.iLutMode);
  if(status != CAMERA_STATUS_SUCCESS)
  {
    msg = string_format("CameraSetLutMode : %d", status);
    ERR_LOG(msg);
    printf("[ERROR]CameraSetLutMode : %d\n", status);
    return status;
  }
  status = CameraGetLutMode(g_hCamera, &st_cam2.stEnhance.iLutMode);
  if(status != CAMERA_STATUS_SUCCESS)
  {
    msg = string_format("CameraGetLutMode : %d", status);
    ERR_LOG(msg);
    printf("[ERROR]CameraGetLutMode : %d\n", status);
    return status;
  }

  status = CameraSelectLutPreset(g_hCamera, st_cam2.stEnhance.iLutSel);
  if(status != CAMERA_STATUS_SUCCESS)
  {
    msg = string_format("CameraSelectLutPreset : %d", status);
    ERR_LOG(msg);
    printf("[ERROR]CameraSelectLutPreset : %d\n", status);
    return status;
  }
  status = CameraGetLutPresetSel(g_hCamera, &st_cam2.stEnhance.iLutSel);
  if(status != CAMERA_STATUS_SUCCESS)
  {
    msg = string_format("CameraGetLutPresetSel : %d", status);
    ERR_LOG(msg);
    printf("[ERROR]CameraGetLutPresetSel : %d\n", status);
    return status;
  }

  return status;
}

CameraSdkStatus CameraControl::CameraSetGPIOs()
{
  CameraSdkStatus status = -1;

  //int iIOIndex = 0;
  //int piIOMode;
  //uint puState;
  // Read the level state
  // 0 : high, 1 : low
  status = CameraSetIOState(g_hCamera, st_cam2.stIO.iIOIndex, 1);
  if(status != CAMERA_STATUS_SUCCESS) 
  {
    printf("[ERROR]CameraSetIOState : %d\n", status);
    return status;
  }
  status = CameraGetOutPutIOState(g_hCamera, st_cam2.stIO.iIOIndex, &st_cam2.stIO.puState);
  if(status != CAMERA_STATUS_SUCCESS)
  { 
    printf("[ERROR]CameraGetOutPutIOState : %d\n", status);
    return status;
  }

  // Get the output IO mode.
  // 0 : IOMODE_TRIG_INPUT(Trigger input)
  // 1 : IOMODE_STROBE_OUTPUT(Strobe output)
  // 2 : IOMODE_GP_INPUT(Universal input)
  // 3 : IOMODE_GP_OUTPUT(Universal output)
  // 4 : IOMODE_PWM_OUTPUT(PWM output)
  // 5 : IOMODE_ROTARYENC_INPUT(rotary input)
  status = CameraSetOutPutIOMode(g_hCamera, st_cam2.stIO.iIOIndex, IOMODE_GP_OUTPUT);  // init : IOMODE_GP_OUTPUT -> after booting -> IOMODE_STROBE_OUTPUT
  if(status != CAMERA_STATUS_SUCCESS)    
  {
    printf("[ERROR]CameraSetOutPutIOMode : %d\n", status);
    return status;
  }
  status = CameraGetOutPutIOMode(g_hCamera, st_cam2.stIO.iIOIndex, &st_cam2.stIO.piIOMode);
  if(status != CAMERA_STATUS_SUCCESS)
  {
    printf("[ERROR]CameraGetOutPutIOMode : %d\n", status);
    return status;
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

int CameraControl::CtrlIRLED(int value)
{
  if(value)   // on(strobe mode)
  {
    CameraSetOutPutIOMode(g_hCamera, st_cam2.stIO.iIOIndex, IOMODE_STROBE_OUTPUT);
    
    msg = string_format("[ON] current ExposureTime value = %f, st_cam2.stAE.expCurPercent = %.02f[%%]", st_cam2.stAE.expCur, st_cam2.stAE.expCurPercent);
    INFO_LOG(msg);
  }
  else        // off(gpio mode)
  {
    CameraSetOutPutIOMode(g_hCamera, st_cam2.stIO.iIOIndex, IOMODE_GP_OUTPUT);

    if(st_cam.dnStatus == 0)
    {
      msg = string_format("[OFF] current ExposureTime value = %f, st_cam2.stAE.expCurPercent = %.02f[%%]", st_cam2.stAE.expCur, st_cam2.stAE.expCurPercent);
      INFO_LOG(msg);
    }
  }

  return 1;
}

int CameraControl::CheckDNStatus()
{
  static unsigned int debounce_cnt = 0;
	static const unsigned int debounce_limit = 30;	// 3sec @ 10fps

  // read value(전면 스위치 상태 : 0:Force OFF, 1:AUTO)
	int fd_sw = open(FD_GPIO_CAM_SW, O_RDONLY);
	if (fd_sw == -1) {
		ERR_LOG(string("Unable to open /sys/class/gpio/gpio_cam_sw/value"));
    return -1;
	}

  char buf[16];
	read(fd_sw, buf, 16);
	int value = atoi(buf);	// 1 : AUTO, 0 : Force OFF
	//cout << "status of switch = " << value << endl;
  close(fd_sw);

  // control gpio output no.1 
  CameraGetOutPutIOMode(g_hCamera, st_cam2.stIO.iIOIndex, &st_cam2.stIO.piIOMode);
  if(st_cam2.stIO.piIOMode == IOMODE_STROBE_OUTPUT) st_cam.dnStatus = 0;
  else                                              st_cam.dnStatus = 1;
  
  if(value == 0)  // force off
  {
    // init
    debounce_cnt = 0;
    CtrlIRLED(0); //CameraSetOutPutIOMode(g_hCamera, st_cam2.stIO.iIOIndex, IOMODE_GP_OUTPUT);        // off(gpio mode)
  }
  else
  {
    if(st_cam.dnStatus == 0)  // on status, 현재 IR LED ON상태
    {
      debounce_cnt++;

      // check on -> off
      if(st_cam2.stAE.expCurPercent < st_cam2.stAE.irTurnOffValue)
      {
        if(debounce_cnt > (debounce_limit<<2))
        {
          debounce_cnt = 0;
          CtrlIRLED(0);
        }
      }
      else
      {
        debounce_cnt = 0;
      }
    }
    else                      // on status, 현재 IR LED OFF상태
    {
      debounce_cnt++;

      // check off -> on
      if(st_cam2.stAE.expCurPercent > st_cam2.stAE.irTurnOnValue)
      {
        if(debounce_cnt > debounce_limit)
        {
          debounce_cnt = 0;
          CtrlIRLED(1);
        }
      }
      else
      {
        debounce_cnt = 0;
      }
    }
    //CtrlIRLED(1); //CameraSetOutPutIOMode(g_hCamera, st_cam2.stIO.iIOIndex, IOMODE_STROBE_OUTPUT);    // auto(strobe mode)
  }

  return 1;
}

int CameraControl::PostProcess()
{
  int ret = -1;

  double pfLineTime;
  
  if(CAMERA_STATUS_SUCCESS != CameraSetSharpness(g_hCamera, st_cam2.stEnhance.iSharpness))    
  {
    printf("[ERROR]CameraSetSharpness : \n");
  }
  CameraGetAeTarget(g_hCamera, &st_cam2.stAE.itarAE);       // 
  CameraGetExposureTime(g_hCamera, &st_cam2.stAE.expCur);   // 현재 Exposure Time (= strobe time)
  CameraGetExposureLineTime(g_hCamera, &pfLineTime);        // 현재 Exposure Time (= strobe time)
  CameraGetAnalogGainX(g_hCamera, &st_cam2.stAE.gainCur);
  CameraGetAnalogGain(g_hCamera, &st_cam2.stAE.againCur);
  ret = CalcGrabberStat();
  ret = CheckDNStatus();


  // LED - ST2 동작상태표시
  CtrlGPIO(fd_cam_st, (st_cam.capCount>>2)&0x1);

#if false
  int hoff, voff, w, h;
  CameraGetAeWindow(g_hCamera, &hoff, &voff, &w, &h);

  BOOL bVisible;
  CameraIsAeWinVisible(g_hCamera, &bVisible);

  BOOL bAntiFlicker;
  CameraGetAntiFlick(g_hCamera, &bAntiFlicker);
#endif

#if false
  printf("=========== PostProcess ================\n");
  printf("Average FPS : %.2f\n", st_cam.capFPS);
  printf("AE Enable State(0:off, 1:on) : %d\n", st_cam2.stSystem.bAeState);
  printf("LightFrequence(0:50Hz, 1:60Hz) : %d\n", st_cam2.stSystem.iFrequencySel);
  printf("Target AE : %d\n", st_cam2.stAE.itarAE);
  printf("st_cam.again : %d [%d ~ %d]\n", st_cam2.stAE.againCur, st_cam2.stAE.againMin, st_cam2.stAE.againMax);
  printf("st_cam.gain : %f [%f ~ %f](step:%f)\n", st_cam.gainCur, st_cam.gainMin, st_cam.gainMax, st_cam2.stAE.gainStep);
  printf("st_cam.shutter : %f [%f ~ %f]\n", st_cam.shCur, st_cam.shMin, st_cam.shMax);
  printf("st_cam.expCur : %f(line:%f) [%f ~ %f]\n", st_cam.expCur, pfLineTime, st_cam.expMin, st_cam.expMax);
  printf("AE Window : visible:%d, flicker:%d, %d, %d, %d, %d\n", bVisible, bAntiFlicker, hoff, voff, w, h);
  printf("IO Index(%d) : mode(%d), state(%d)\n", st_cam2.stIO.iIOIndex, st_cam2.stIO.piIOMode, st_cam2.stIO.puState);
  printf("========================================\n\n");
//#else
  if(CameraSetSharpness(g_hCamera, st_cam2.stEnhance.iSharpness) != 0)
  {
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@Error@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
  }
  if(CameraGetSharpness(g_hCamera, &st_cam2.stEnhance.iSharpness) != 0)
  {
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@Error@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
  }
  //printf("FPS:%.2f, TargetAE:%d, Exposure:%f(%.02f%%), range[%f ~ %f]\n\n", st_cam.capFPS, st_cam.targetAE, st_cam.expCur, st_cam2.stAE.expCurPercent, st_cam.expMin, st_cam.expMax);
  printf("Gamma:%d, Sharp:%d, Contast:%d, Saturation:%d, LutMode:%d, LutPreset:%d\n", st_cam2.stEnhance.iGamma, st_cam2.stEnhance.iSharpness, st_cam2.stEnhance.iContrast, st_cam2.stEnhance.iSaturation, st_cam2.stEnhance.iLutMode, st_cam2.stEnhance.iLutSel);
#endif

  return ret;
}

int CameraControl::CalcGrabberStat()
{
  // elapsed time : start
  static double pre_time = 0;
  static system_clock::time_point start = system_clock::now();
  system_clock::time_point end = system_clock::now();
  nanoseconds nano = end - start;

  double time_sec = nano.count()/1000000000;  // unit : second

  CameraGetFrameStatistic(g_hCamera, &g_tFrameStatistic);
  st_cam2.stSystem.capCount   = g_tFrameStatistic.iCapture;
  st_cam2.stSystem.capFPS     = st_cam.capCount/time_sec;
  st_cam2.stSystem.capLost    = g_tFrameStatistic.iLost;
  

  // make st_cam
  st_cam.capWidth             = st_cam2.stSystem.capWidth;
  st_cam.capHeight            = st_cam2.stSystem.capHeight;
  st_cam.capCount             = st_cam2.stSystem.capCount;
  st_cam.capFPS               = st_cam2.stSystem.capFPS;
  st_cam.targetAE             = st_cam2.stAE.itarAE;
  st_cam.shMin                = st_cam2.stAE.expMin;
  st_cam.shMax                = st_cam2.stAE.expMax;
  st_cam.shCur                = st_cam2.stAE.expCur;
  st_cam.gainMin              = st_cam2.stAE.gainMin;
  st_cam.gainMax              = st_cam2.stAE.gainMax;
  st_cam.gainCur              = st_cam2.stAE.gainCur;

  st_cam.expMin 		          = st_cam.shMin;
  st_cam.expMax 		          = (st_cam.gainMax * st_cam.shMax);
  st_cam.expCur 		          = ((st_cam.gainCur-1) * st_cam.shMax) + st_cam.shCur;
  st_cam2.stAE.expCurPercent  = (st_cam.expCur*100/st_cam.expMax);
  st_cam.expCurPercent        = st_cam2.stAE.expCurPercent;


  //if(((int)time_sec%10) == 0) {
  if((time_sec - pre_time) >= 10) {
    //printf("Running TIME : %f[sec]\n", time_sec);
    //printf("Captured Count : %d\n", st_cam.capCount);
    //printf("Captured FPS : %f\n", st_cam.capFPS);

    msg = "Grabbed image: " + string_format("%09d", st_cam.capCount) + ", width = " + to_string(st_cam.capWidth) + ", height = " + to_string(st_cam.capHeight) + \
					"\tAverage FPS: " + string_format("%.2f", st_cam.capFPS);
    INFO_LOG(msg);          
    //printf("%s\n", msg.c_str());
    pre_time = time_sec;
  }

  return 1;
}

CameraSdkStatus CameraControl::GetFrameData()
{
  BYTE* pData = 0;
  CameraSdkStatus status = -1;

#if 1
  //CameraClearBuffer(g_hCamera);
  //status = CameraGetImageBuffer(g_hCamera, &g_tFrameHead, &g_pFrameBuffer, 2000);
  status = CameraGetImageBufferPriority(g_hCamera, &g_tFrameHead, &g_pFrameBuffer, 2000, CAMERA_GET_IMAGE_PRIORITY_NEXT);
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
      Sm_GrabLpr->SharedMemoryWrite((char*)g_pFrameBuffer, width*height);

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