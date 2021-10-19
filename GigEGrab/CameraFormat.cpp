/*
 *	CameraConnect.cpp
 *
 * 
 *
 *
 *
 */

#include <iostream>
#include <sstream>

#include "CameraFormat.h"

#define MIN_CAPTURE_CLK 8312000
#define	INC_CAPTURE_CLK 88000
CameraFormat::CameraFormat()
{
	nCapWidth 	= 1920;
	nCapHeight	= 1080;

	nOffsetX		= 0;
	nOffsetY		= 0;

	bFrameRateMode	= true;
	fFrameRate			= 30.0;
}

CameraFormat::~CameraFormat()
{
}

bool CameraFormat::Init()
{
	return true;
}

int CameraFormat::SetCameraPtr(CameraPtr pcam)
{
	pCam = pcam;

	return 0;
}


int CameraFormat::SetCapWidth(int64_t width)
{
	INodeMap& nodeMap = pCam->GetNodeMap();

	CIntegerPtr ptrWidth = nodeMap.GetNode("Width");
	if (IsAvailable(ptrWidth) && IsWritable(ptrWidth))
	{
		nCapWidth = width;
		ptrWidth->SetValue(nCapWidth);
		
		cout << "Width set to " << ptrWidth->GetValue() << "..." << endl;
	}
	else 
	{
		cout << "Width not available..." << endl;
	}

	return 0;
}

int CameraFormat::SetCapHeight(int64_t height)
{
	INodeMap& nodeMap = pCam->GetNodeMap();
	
	CIntegerPtr ptrHeight = nodeMap.GetNode("Height");
	if (IsAvailable(ptrHeight) && IsWritable(ptrHeight))
	{
		nCapHeight = height;
		ptrHeight->SetValue(nCapHeight);

		cout << "Height set to " << ptrHeight->GetValue() << "..." << endl << endl;
	}
	else 
	{
		cout << "Height not available..." << endl << endl;
	}

	return 0;
}

int CameraFormat::SetOffsetX(int64_t offsetX)
{
	INodeMap& nodeMap = pCam->GetNodeMap();

	CIntegerPtr ptrOffsetX = nodeMap.GetNode("OffsetX");
	if (IsAvailable(ptrOffsetX) && IsWritable(ptrOffsetX))
	{
		nOffsetX = offsetX;
		ptrOffsetX->SetValue(nOffsetX);
		
		cout << "Offset X set to " << ptrOffsetX->GetValue() << "..." << endl;
	}
	else
	{
		cout << "Offset X not available..." << endl;
	}

	return 0;
}

int CameraFormat::SetOffsetY(int64_t offsetY)
{
	INodeMap& nodeMap = pCam->GetNodeMap();

	CIntegerPtr ptrOffsetY = nodeMap.GetNode("OffsetY");
	if (IsAvailable(ptrOffsetY) && IsWritable(ptrOffsetY))
	{
		nOffsetY = offsetY;
		ptrOffsetY->SetValue(nOffsetY);
		
		cout << "Offset Y set to " << ptrOffsetY->GetValue() << "..." << endl;
	}
	else
	{
		cout << "Offset Y not available..." << endl;
	}

	return 0;
}

int CameraFormat::SetFrameRateMode(bool enable)
{
	// Turn on frame rate control
	CBooleanPtr ptrFrameRateEnable = pCam->GetNodeMap().GetNode("AcquisitionFrameRateEnable");
	if (!IsAvailable(ptrFrameRateEnable) || !IsWritable(ptrFrameRateEnable))
	{
		cout << "Unable to enable Acquisition Frame Rate Mode (node retrieval). Aborting..." << endl;
		return -1;
	}

	// Enable  Acquisition Frame Rate Enable
	bFrameRateMode = enable;
	ptrFrameRateEnable->SetValue(bFrameRateMode);

	cout << "Frame rate mode is set to " << bFrameRateMode << endl;

	return 0;
}

int CameraFormat::SetFrameRate(float frameRate)
{
	// note : 8312000(min value)
	// 8312000 : 1.07628fps(fhd)
	// 8312000 : 2.42131fps(hd)
	fFrameRate = frameRate;
	INodeMap& nodeMap = pCam->GetNodeMap();
	CIntegerPtr ptrThroughputLimit = nodeMap.GetNode("DeviceLinkThroughputLimit");
	if(nCapWidth == 1920 && nCapHeight == 1080)	// FHD
	{
		int64_t tarClk = MIN_CAPTURE_CLK + INC_CAPTURE_CLK*(28*(frameRate-1));

		cout << "set to  = " << tarClk << endl;
		
		//ptrThroughputLimit->SetValue(MIN_CAPTURE_CLK + (INC_CAPTURE_CLK * (int)(frameRate-1)));
		ptrThroughputLimit->SetValue(tarClk);
	}
	else 																				// HD
	{
		int64_t tarClk = MIN_CAPTURE_CLK + INC_CAPTURE_CLK*(9*(frameRate-1));

		cout << "set to  = " << tarClk << endl;
		
		//ptrThroughputLimit->SetValue(MIN_CAPTURE_CLK + (INC_CAPTURE_CLK * (int)(frameRate-1)));
		ptrThroughputLimit->SetValue(tarClk);
	}
	//cout << "DeviceLinkThroughputLimit = " << ptrThroughputLimit->GetValue() << endl;
	cout << "DeviceLinkThroughputLimit = " << ptrThroughputLimit->GetValue() << endl;
	cout << "Frame rate is set to " << fFrameRate << endl;

#if false
	CFloatPtr ptrFrameRate = pCam->GetNodeMap().GetNode("AcquisitionFrameRate");
	if (!IsAvailable(ptrFrameRate) || !IsWritable(ptrFrameRate))
	{
		cout << "IsAvailable(ptrFrameRate) = " << IsAvailable(ptrFrameRate) << endl;
		cout << "IsWritable(ptrFrameRate) = " << IsWritable(ptrFrameRate) << endl;
		cout << "Unable to set Acquisition Frame Rate (node retrieval). Aborting..." << endl;
		cout << "Frame rate is set to " << ptrFrameRate->GetValue() << endl;
		return -1;
	}
	// Set fps
	fFrameRate = frameRate;
	ptrFrameRate->SetValue(fFrameRate);
	cout << "Frame rate is set to " << fFrameRate << endl;
#endif

	return 0;
}