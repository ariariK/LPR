/*
 *	CameraGrab.h
 *
 * 
 *
 *
 *
 */

#pragma once
#include "Typedef.h"

class CameraGrab
{
//////////////////////////////////////////////////////////////////////////////////////////////////////	
// Construction
//////////////////////////////////////////////////////////////////////////////////////////////////////	
public:
	CameraGrab();
	virtual ~CameraGrab();

//////////////////////////////////////////////////////////////////////////////////////////////////////	
// Attributes
//////////////////////////////////////////////////////////////////////////////////////////////////////	
public:

private:
	CameraPtr 	pCam;
	//INodeMap	*m_nodeMap;
	//INodeMap	*m_nodeMapTLDevice;

protected:
	string 			msg;

	bool				bSaveEnable;
	string			strSavePath;

//////////////////////////////////////////////////////////////////////////////////////////////////////	
// Operations
//////////////////////////////////////////////////////////////////////////////////////////////////////	
public:
	int		SetCameraPtr(CameraPtr pcam);
	CameraPtr	GetCameraPtr() { return pCam; }

	int		SetSaveEnable(bool enable);
	int 	SetSavePath(string path);
//////////////////////////////////////////////////////////////////////////////////////////////////////	
// Implementation
//////////////////////////////////////////////////////////////////////////////////////////////////////	
public:
	bool 	Init();

	//int		RunAcquisition(CameraPtr pCam, INodeMap& nodeMap, INodeMap& nodeMapTLDevice);
	int		RunGrabbing();

private:

protected:

};
