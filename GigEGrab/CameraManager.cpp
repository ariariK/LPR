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
	// 객체 소멸
	SAFE_DELETE(pCamConn);
	SAFE_DELETE(pCamFormat);
	SAFE_DELETE(pCamGrab);

	// Release
	CameraRelease();
}

int CameraManager::PrintDeviceInfo(INodeMap& nodeMap)
{
	//cout << endl << "*** DEVICE INFORMATION ***" << endl << endl;
	INFO_LOG(string("*** DEVICE INFORMATION ***"));

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

				if(IsReadable(pValue))
				{
					msg = string_format("%s : %s", pfeatureNode->GetName().c_str(), pValue->ToString().c_str());
				}
				else 
				{
					msg = string_format("%s : Node not readable", pfeatureNode->GetName().c_str());
				}
				
				INFO_LOG(msg);
			}
		}
		else 
		{
			cout << "Device control information not available." << endl;
			ERR_LOG(string("Device control information not available."));
		}
	}
	catch (Spinnaker::Exception& e)
	{
		cout << "Error: " << e.what() << endl;
		msg = string_format("%s", e.what());
		ERR_LOG(msg);

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

	msg = string_format("Spinnaker library version: %d.%d.%d.%d", libVersion.major, libVersion.minor, libVersion.type, libVersion.build);
	INFO_LOG(msg);

	// Retrive list of cameras from the system
	camList = pSystem->GetCameras();

	const unsigned int numCameras = camList.GetSize();
	cout << "Number of GigE Vision cameras detected: " << numCameras << endl << endl;
	msg = string_format("Number of GigE Vision cameras detected : %d", numCameras);
	INFO_LOG(msg);

	// Finish if there are no cameras
	if (numCameras == 0) 
	{
		camList.Clear();
		pSystem->ReleaseInstance();

		cout << "Not enough cameras!" << endl;
		ERR_LOG(string("Not enough cameras!"));
		
		return false;
	}

	return true;
}

int CameraManager::CameraReady(int cam_index)
{
	pCam = camList.GetByIndex(cam_index);
	if (pCam->IsInitialized()) 
	{
  	cout << "Camera already initialized. Deinitializing... " << endl;
   	pCam->EndAcquisition();
   	pCam->DeInit();
  }
	pCam->Init();
	//always capture the latest image from the camera (ideally this should be a parameter)
	pCam->TLStream.StreamBufferHandlingMode.SetValue(::Spinnaker::StreamBufferHandlingMode_NewestFirstOverwrite);

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
		INFO_LOG(string("Release GigE Camrea..."));
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
	pCamGrab->SetFrameRateMode(enable);
	return 0;
}

int CameraManager::SetFrameRate(float frameRate)
{
	pCamFormat->SetCameraPtr(pCam);
	pCamFormat->SetFrameRate(frameRate);
	pCamGrab->SetTargetFPS(frameRate);
	return 0;
}

int CameraManager::SetGpioUserMode(float value)
{
	pCamFormat->SetGpioUserMode();
	pCamGrab->SetGainValue(value);
	return 0;
}

int CameraManager::SetGpioStrobeMode()
{
	pCamFormat->SetGpioStrobeMode();
	return 0;
}

int CameraManager::SetGainLow(float value)
{
	pCamGrab->SetGainLow(value);
	return 0;
}

int CameraManager::SetGainHigh(float value)
{
	pCamGrab->SetGainHigh(value);
	return 0;
}


int CameraManager::SetExposureMax(float value)
{
	pCamGrab->SetExposureMax(value);
	return 0;
}

int CameraManager::SetExposureLow(float value)
{
	pCamGrab->SetExposureLow(value);
	return 0;
}

int CameraManager::SetExposureHigh(float value)
{
	pCamGrab->SetExposureHigh(value);
	return 0;
}

int CameraManager::SetImageProcess(bool bImgProcEn, bool bGammaEn, bool bSharpEn, int nSharpAuto, float fGamma, int nSharp)
{
	pCamFormat->SetImageProcess(bImgProcEn, bGammaEn, bSharpEn, nSharpAuto, fGamma, nSharp);
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
	pCamGrab->Init();
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

