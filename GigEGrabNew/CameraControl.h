/*
 *	CameraControl.h
 *
 * 
 *
 *
 *
 */

#pragma once
#include "Typedef.h"
#include "ipcs.h"

class CameraControl
{
//////////////////////////////////////////////////////////////////////////////////////////////////////	
// Construction
//////////////////////////////////////////////////////////////////////////////////////////////////////	
public:
	CameraControl();
	virtual ~CameraControl();

//////////////////////////////////////////////////////////////////////////////////////////////////////	
// Attributes
//////////////////////////////////////////////////////////////////////////////////////////////////////	
public:

private:
	
protected:
	string 			msg;

//////////////////////////////////////////////////////////////////////////////////////////////////////	
// Operations
//////////////////////////////////////////////////////////////////////////////////////////////////////	
public:

//////////////////////////////////////////////////////////////////////////////////////////////////////	
// Implementation
//////////////////////////////////////////////////////////////////////////////////////////////////////	
public:
	//bool 	Init();
	//int		RunGrabbing();

	// Init Ipcs
	int Init();
	void PrintDeviceInfo();

	// Set Configs - System
	CameraSdkStatus SetImageResolution(int offsetx, int offsety, int width, int height);
	CameraSdkStatus SetMirrorFlip(BOOL mirror, BOOL flip);
	CameraSdkStatus SetFrameRate(int value);
	CameraSdkStatus SetFrameSpeed(int value);

	// Set Configs - AE
	CameraSdkStatus SetAeExposureRange(double dwMinValue, double dwMaxValue);

	CameraSdkStatus SetCameraLightFrequency(int value);

	// System(frame rate, resolution...)
	CameraSdkStatus CameraSetSystem();

	// Auto Exposure
	CameraSdkStatus CameraSetAE();

	// Enhancement
	CameraSdkStatus CameraEnhancement();

	// GPIOs
	CameraSdkStatus CameraSetGPIOs();

	// Operation
	int PostProcess();
	int CheckDNStatus();
	int CalcGrabberStat();
	CameraSdkStatus	GetFrameData();
	

	int fd_cam_st;
	int ToggleLEDStatus();
	int CtrlGPIO(int fd, int value);
	int CtrlIRLED(int value);	// 0:off, 1:on
	

private:
	Ipcs	*Sm_Grab;
	Ipcs	*Sm_GrabLpr;	// add. by ariari : 2022.11.09
	Ipcs	*Sm_Res;
	Ipcs	*Mq_Grab;
	Ipcs	*Mq_GrabImg;
	Ipcs	*Sm_Cam;

	struct grab_data{
		int capWidth;
		int capHeight;
	};
	struct message{
		long msg_type;
		struct grab_data data;
	};

	struct cam_param{
		int capWidth;
		int capHeight;

		// add. by ariri : 2022.11.22 - begin
		int				draw_en;
		int 			roi_sx;
		int 			roi_sy;
		int				roi_w;
		int				roi_h;
		// add. by ariri : 2022.11.22 - end

		uint64_t	capCount;
		double		capFPS;

		int64_t		tarClk;
		int				targetAE;				// add. by ariari : 2022.11.09
		double		expMax;
		double		expMin;
		double		expCur;
		double		expCurPercent;	// add. by ariari : 2022.11.14
		
		int				dnStatus;
		float			shMax;
		float			shMin;
		float			shCur;
		float			gainMax;
		float			gainMin;
		float			gainCur;

		//float			gainStep;
	};

	//int iIOIndex = 0;
  //int piIOMode;
  //uint puState;

	// for mindvision
	struct cam_system {
		BOOL			bAeState;
		int				iFrequencySel;

		int 			hoff;
		int 			voff;
		int 			capWidth;
		int 			capHeight;

		BOOL			bMirror;
		BOOL			bFlip;

		int				framerate;	// set value
		uint64_t	capCount;		// 
		int				capLost;		//
		double		capFPS;			// 
	};

	struct cam_ae {
		int				itarAE;

		double		expMax;
		double		expMin;
		double		expCur;
		double		expCurPercent;
		
		int	    	againMin;
		int    		againMax;
		int    		againCur;

		float			gainMin;
		float			gainMax;
		float			gainStep;
		float			gainCur;

		int				ae_win_x;
		int				ae_win_y;
		int				ae_win_w;
		int				ae_win_h;

		double		irTurnOnValue;
		double		irTurnOffValue;
	};

	struct cam_enhancement {
		int 			iGamma;
  	int 			iContrast;
		int				iSharpness;
		int				iSaturation;
  	int 			iLutMode;
		int				iLutSel;
	};

	struct cam_io {
		int 			iIOIndex;
  	int 			piIOMode;
  	uint 			puState;
	};

	struct cam_param2{
		struct cam_system				stSystem;
		struct cam_ae						stAE;
		struct cam_enhancement	stEnhance;
		struct cam_io						stIO;
	};
	
public:	
	//////////////////////////////////////////////////////////////////////////////////////////////////////	
	// IPC : Shared Memory
	//////////////////////////////////////////////////////////////////////////////////////////////////////	
	struct grab_data 	st_grab;
	struct cam_param 	st_cam;
	struct cam_param2	st_cam2;
	//////////////////////////////////////////////////////////////////////////////////////////////////////	

	//////////////////////////////////////////////////////////////////////////////////////////////////////	
	// IPC : Message Queue
	//////////////////////////////////////////////////////////////////////////////////////////////////////	
	struct message msq;
	struct message msq_img;
	//////////////////////////////////////////////////////////////////////////////////////////////////////	


protected:

};

