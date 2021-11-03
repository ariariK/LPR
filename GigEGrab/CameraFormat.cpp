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

#if false
	INodeMap& sNodeMap = pCam->GetTLStreamNodeMap();
	CIntegerPtr StreamNode = sNodeMap.GetNode("StreamDefaultBufferCount");
	int64_t bufferCount = StreamNode->GetValue();
	if (!IsAvailable(StreamNode) || !IsWritable(StreamNode)){
        cout << "Unable to set StreamMode  Aborting..." << endl;
        return -1;
  }
  StreamNode->SetValue(bufferCount);
	cout << "Number of Image Buffers : " << bufferCount << endl;
#endif

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

// GPIO
int CameraFormat::SetGpioUserMode()
{
	SetLineSelector();
	SetLineMode();
	SetLineSource();
	SetUserOutputSelector();
	SetUserOutputValue(false);

	return 1;
}

int CameraFormat::SetLineSelector()
{
	// It retrieves GenICam nodemap
  INodeMap& nodeMap = pCam->GetNodeMap();
  
	// This is for Serial port Transmit settings
	CEnumerationPtr ptrLineSelector = nodeMap.GetNode("LineSelector");
	if (!IsAvailable(ptrLineSelector) || !IsWritable(ptrLineSelector))
	{
			cout << "Unable to set Line Selector. Aborting..." << endl << endl;
			return -1;
	}
	ptrLineSelector->SetIntValue(1); // line 1 is selected(Line1)

	cout << endl << "SetLineSelector() : ptrLineSelector->GetIntValue() = " << ptrLineSelector->GetIntValue() << endl;
	
	return 1;
}


int CameraFormat::SetLineMode()
{
	// It retrieves GenICam nodemap
  INodeMap& nodeMap = pCam->GetNodeMap();

	CEnumerationPtr ptrLineMode = nodeMap.GetNode("LineMode");
	if (!IsAvailable(ptrLineMode) || !IsWritable(ptrLineMode))
	{
			cout << "Unable to set Line Mode. Aborting..." << endl << endl;
			return -1;
	}
	ptrLineMode->SetIntValue(1); // output is selected(Output)

	cout << endl << "SetLineMode() : ptrLineMode->GetIntValue() = " << ptrLineMode->GetIntValue() << endl;
	
	return 1;
}

int CameraFormat::SetLineSource()
{
	// It retrieves GenICam nodemap
  INodeMap& nodeMap = pCam->GetNodeMap();

	CEnumerationPtr ptrLineSource = nodeMap.GetNode("LineSource");
	if (!IsAvailable(ptrLineSource) || !IsWritable(ptrLineSource))
	{
			cout << "Unable to set Line Source. Aborting..." << endl << endl;
			return -1;
	}
	ptrLineSource->SetIntValue(2); // User Output1 0 is selected(UserOutput1)

	cout << endl << "SetLineSource() : ptrLineSource->GetIntValue() = " << ptrLineSource->GetIntValue() << endl;
	
	return 1;
}

int CameraFormat::SetUserOutputSelector()
{
	// It retrieves GenICam nodemap
  INodeMap& nodeMap = pCam->GetNodeMap();

	CEnumerationPtr ptrUserOutputSource = nodeMap.GetNode("UserOutputSelector");
	if (!IsAvailable(ptrUserOutputSource) || !IsWritable(ptrUserOutputSource))
	{
			cout << "Unable to set User Output Source. Aborting..." << endl << endl;
			return -1;
	}
	ptrUserOutputSource->SetIntValue(1); // User Output Source 1 is selected(UserOutputValue)

	cout << endl << "SetUserOutputSelector() : ptrUserOutputSource->GetIntValue() = " << ptrUserOutputSource->GetIntValue() << endl;

	return 1;
}

int CameraFormat::SetUserOutputValue(bool value)
{
	CBooleanPtr ptrUserOutputValue = pCam->GetNodeMap().GetNode("UserOutputValue");
	if (!IsAvailable(ptrUserOutputValue) || !IsWritable(ptrUserOutputValue))
	{
			cout << "Unable to set User Output Value. Aborting..." << endl << endl;
		return -1;
	}

	ptrUserOutputValue->SetValue(value);

	cout << endl << "SetUserOutputValue() : ptrUserOutputValue->GetValue() = " << ptrUserOutputValue->GetValue() << endl;

	return 1;
}
