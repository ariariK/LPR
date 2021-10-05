/*
 *	CameraManager.cpp
 *
 * 
 *
 *
 *
 */

#include <iostream>
#include <sstream>

#include "CameraManager.h"

CameraManager::CameraManager()
{
	pSystem = System::GetInstance();
	pCam		= nullptr;
	bIsCamReady	= false;
	savePath.clear();

	// CameraConnect 객체 생성
	pCamConn = new CameraConnect();

	// CameraFormat 객체 생성
	pCamFormat = new CameraFormat();

	// CameraGrab 객체 생성
	pCamGrab = new CameraGrab();
}


CameraManager::~CameraManager()
{
	// Release
	CameraRelease();


	// 객체 소멸
	SAFE_DELETE(pCamConn);
	SAFE_DELETE(pCamFormat);
	SAFE_DELETE(pCamGrab);
}

int CameraManager::PrintDeviceInfo(INodeMap& nodeMap)
{
	cout << endl << "*** DEVICE INFORMATION ***" << endl << endl;

	try 
	{
		FeatureList_t features;
		const CCategoryPtr category = nodeMap.GetNode("DeviceInformation");
		if (IsAvailable(category) && IsReadable(category))
		{
			category->GetFeatures(features);

			for (auto it = features.begin(); it != features.end(); ++it) 
			{
			
				const CNodePtr pfeatureNode = *it;
				cout << pfeatureNode->GetName() << " : ";
				
				CValuePtr pValue = static_cast<CValuePtr>(pfeatureNode);
				cout << (IsReadable(pValue) ? pValue->ToString() : "Node not readable");
				cout << endl;	
			}
		}
		else 
		{
			cout << "Device control information not available." << endl;
		}
	}
	catch (Spinnaker::Exception& e)
	{
		cout << "Error: " << e.what() << endl;
		return -1;
	}

	return 0;
}

bool CameraManager::Init()
{	
	// init camera list
	camList.Clear();

	// Print out current library version
	libVersion = pSystem->GetLibraryVersion();
	cout 	<< "Spinnaker library version: " 
				<< libVersion.major << "." 
				<< libVersion.minor << "." 
				<< libVersion.type << "." 
				<< libVersion.build << endl
    		<< endl;

	// Retrive list of cameras from the system
	camList = pSystem->GetCameras();

	const unsigned int numCameras = camList.GetSize();
	cout << "Number of GigE Vision cameras detected: " << numCameras << endl << endl;

	// Finish if there are no cameras
	if (numCameras == 0) 
	{
		camList.Clear();
		pSystem->ReleaseInstance();

		cout << "Not enough cameras!" << endl;
		
		return false;
	}

	return true;
}

int CameraManager::CameraReady(int cam_index)
{
	pCam = camList.GetByIndex(cam_index);
	pCam->Init();

#if true
	
	INodeMap& nodeMapTLDevice = pCam->GetTLDeviceNodeMap();
	PrintDeviceInfo(nodeMapTLDevice);

#endif

	bIsCamReady	= true;

	return 0;
}

int CameraManager::CameraRelease()
{
	if (pCam && bIsCamReady)
	{
		pCam = nullptr;
		bIsCamReady = false;

		// Clear camera list before releasing system
		//
		camList.Clear();

		// Release System
		//
		pSystem->ReleaseInstance();

		cout << endl << "Release GigE Camrea..." << endl;
	}
	
	return 0;
}

int CameraManager::SetCapWidth(int64_t width)
{
	pCamFormat->SetCameraPtr(pCam);
	pCamFormat->SetCapWidth(width);
	return 0;
}

int CameraManager::SetCapHeight(int64_t height)
{
	pCamFormat->SetCameraPtr(pCam);
	pCamFormat->SetCapHeight(height);
	return 0;
}

int CameraManager::SetOffsetX(int64_t offsetX)
{
	pCamFormat->SetCameraPtr(pCam);
	pCamFormat->SetOffsetX(offsetX);
	return 0;
}

int CameraManager::SetOffsetY(int64_t offsetY)
{
	pCamFormat->SetCameraPtr(pCam);
	pCamFormat->SetOffsetY(offsetY);
	return 0;
}

int CameraManager::SetFrameRateMode(bool enable)
{
	pCamFormat->SetCameraPtr(pCam);
	pCamFormat->SetFrameRateMode(enable);
	return 0;
}

int CameraManager::SetFrameRate(float frameRate)
{
	pCamFormat->SetCameraPtr(pCam);
	pCamFormat->SetFrameRate(frameRate);
	return 0;
}

int CameraManager::DoConnection()
{
	return 0;
}

int CameraManager::DoDisconnection()
{
	return 0;
}

int CameraManager::SetSaveEnable(bool enable)
{
	pCamGrab->SetSaveEnable(enable);
	return 0;
}

int CameraManager::SetSavePath(string path)
{
	pCamGrab->SetSavePath(path);
	return 0;
}

int CameraManager::RunGrabbing()
{
	pCamGrab->SetCameraPtr(pCam);
	pCamGrab->RunGrabbing();

/*
	//m_pCam = m_pCamConn->GetCameraPtr();

	try 
	{
		INodeMap& nodeMapTLDevice = pCam->GetTLDeviceNodeMap();

		result = PrintDeviceInfo(nodeMapTLDevice);

		// Initialize
		//pCam->Init();

		INodeMap& nodeMap = pCam->GetNodeMap();

		// Acquisition images
		pCamGrab->RunAcquisition(pCam, nodeMap, nodeMapTLDevice);
		

		// Deinitialize camera
		pCam->DeInit();
	}
	catch (Spinnaker::Exception& e) 
	{
		cout << "Error: " << e.what() << endl;
		result = -1;
	}
*/
	return 0;
}

