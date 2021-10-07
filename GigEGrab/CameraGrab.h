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
	//////////////////////////////////////////////////////////////////////////////////////////////////////	
	// IPC : Shared Memory
	//////////////////////////////////////////////////////////////////////////////////////////////////////	
	int shmid;
	int SharedMemoryCreate();
	int SharedMemoryWrite(ImagePtr shareddata, int size);
	int SharedMemoryFree(void);
	//////////////////////////////////////////////////////////////////////////////////////////////////////	

	//////////////////////////////////////////////////////////////////////////////////////////////////////	
	// IPC : Message Queue
	//////////////////////////////////////////////////////////////////////////////////////////////////////	
	int msqid;
	struct real_data{
		int capWidth;
		int capHeight;
	};
	struct message{
		long msg_type;
		struct real_data data;
	};
	struct message msq;
	int MessageQueueCreate();
	int MessageQueueQNum();
	int MessageQueueWrite();
	int MessageQueueFree();
	//////////////////////////////////////////////////////////////////////////////////////////////////////	


protected:

};
