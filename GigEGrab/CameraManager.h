/*
 *	CameraManager.h
 *
 * 
 *
 *
 *
 */

#pragma once
#include "Typedef.h"

#include "CameraConnect.h"
#include "CameraFormat.h"
#include "CameraGrab.h"

class CameraManager 
{
//////////////////////////////////////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	CameraManager();
	virtual ~CameraManager();

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Attributes
//////////////////////////////////////////////////////////////////////////////////////////////////////
public:

private:
	SystemPtr 	pSystem;
	CameraPtr		pCam;
	CameraList	camList;

	LibraryVersion 	libVersion;
	int nCameras;

	int64_t 	nCapWidth;
	int64_t 	nCapHeight;

protected:
	string 			msg;
	
	bool							bIsCamReady;
	string						savePath;

	CameraConnect			*pCamConn;
	CameraFormat			*pCamFormat;
	CameraGrab				*pCamGrab;

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	CameraPtr	GetCameraPtr() { return pCam; }

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation
//////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	bool 	Init();
	int 	CameraReady(int cam_index);
	int 	CameraRelease();

	int		SetCapWidth(int64_t width);
	int 	SetCapHeight(int64_t height);

	int		SetOffsetX(int64_t offsetX);
	int		SetOffsetY(int64_t offsetY);

	int		SetFrameRateMode(bool enable);
	int		SetFrameRate(float rate);

	int 	SetGpioUserMode(float value);
	int 	SetGpioStrobeMode();
	int 	SetGainLow(float value);
	int 	SetGainHigh(float value);
	int 	SetExposureMax(float value);
	int 	SetExposureLow(float value);
	int 	SetExposureHigh(float value);
	

	int		DoConnection();
	int 	DoDisconnection();

	int 	SetSaveEnable(bool enable);
	int		SetSavePath(string path);
	int 	RunGrabbing();

private:
	int PrintDeviceInfo(INodeMap& nodeMap);

protected:

};
