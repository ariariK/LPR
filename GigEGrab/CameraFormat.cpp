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
		
		//cout << "Width set to " << ptrWidth->GetValue() << "..." << endl;
		msg = string_format("Width set to %d", ptrWidth->GetValue());
		INFO_LOG(msg);
	}
	else 
	{
		//cout << "Width not available..." << endl;
		INFO_LOG(string("Width not available..."));
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

		//cout << "Height set to " << ptrHeight->GetValue() << "..." << endl << endl;
		msg = string_format("Height set to %d", ptrHeight->GetValue());
		INFO_LOG(msg);
	}
	else 
	{
		//cout << "Height not available..." << endl << endl;
		INFO_LOG(string("Height not available..."));
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
		
		//cout << "Offset X set to " << ptrOffsetX->GetValue() << "..." << endl;
		msg = string_format("Offset X set to ", ptrOffsetX->GetValue());
		INFO_LOG(msg);
	}
	else
	{
		//cout << "Offset X not available..." << endl;
		INFO_LOG(string("Offset X not available..."));
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
		
		//cout << "Offset Y set to " << ptrOffsetY->GetValue() << "..." << endl;
		msg = string_format("Offset Y set to ", ptrOffsetY->GetValue());
		INFO_LOG(msg);
	}
	else
	{
		//cout << "Offset Y not available..." << endl;
		INFO_LOG(string("Offset Y not available..."));
	}

	return 0;
}

int CameraFormat::SetFrameRateMode(bool enable)
{
	// Turn on frame rate control
	CBooleanPtr ptrFrameRateEnable = pCam->GetNodeMap().GetNode("AcquisitionFrameRateEnable");
	if (!IsAvailable(ptrFrameRateEnable) || !IsWritable(ptrFrameRateEnable))
	{
		//cout << "Unable to enable Acquisition Frame Rate Mode (node retrieval). Aborting..." << endl;
		EMERG_LOG(string("Unable to enable Acquisition Frame Rate Mode (node retrieval). Aborting..."));
		return -1;
	}

	// Enable  Acquisition Frame Rate Enable
	bFrameRateMode = enable;
	ptrFrameRateEnable->SetValue(bFrameRateMode);

	//cout << "Frame rate mode is set to " << bFrameRateMode << endl;
	msg = string_format("Frame rate mode is set to %d", bFrameRateMode);
	INFO_LOG(msg);

#if false
	INodeMap& sNodeMap = pCam->GetTLStreamNodeMap();
	CIntegerPtr StreamNode = sNodeMap.GetNode("StreamDefaultBufferCount");
	int64_t bufferCount = StreamNode->GetValue();
	if (!IsAvailable(StreamNode) || !IsWritable(StreamNode)){
        //cout << "Unable to set StreamMode  Aborting..." << endl;
				EMERG_LOG(string("Unable to set StreamMode  Aborting..."));
        return -1;
  }
  StreamNode->SetValue(bufferCount);
	//cout << "Number of Image Buffers : " << bufferCount << endl;
	msg = string_format("Number of Image Buffers : %d", bufferCount);
	INFO_LOG(msg);
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

#if true	// new
	// add. by ariari : 2021.12.10
	CFloatPtr ptrFrameRate = pCam->GetNodeMap().GetNode("AcquisitionFrameRate");
	cout << "Frame rate is set to " << ptrFrameRate->GetValue() << endl;
	
	//if (bFrameRateMode && ptrFrameRate->GetValue() > fFrameRate)	// 설정값보다 클 경우(더 많은 처리 => 캡쳐 제한둠)
	if (bFrameRateMode)
	{
		int64_t tarClk = frameRate * (nCapWidth * nCapHeight);
		tarClk = tarClk - (tarClk%INC_CAPTURE_CLK);
		tarClk += MIN_CAPTURE_CLK;

		//cout << "DeviceLinkThroughputLimit set to  = " << tarClk << endl;
		msg = string_format("DeviceLinkThroughputLimit set to  %d", tarClk);
		INFO_LOG(msg);

		ptrThroughputLimit->SetValue(tarClk);

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// AE Range 설정 (min, max)
		//Automatic Exposure Time limits
		//CFloatPtr ptrAutoXExposureTimeLowerLimit = nodeMap.GetNode("AutoExposureTimeLowerLimit");
		//ptrAutoXExposureTimeLowerLimit->SetValue(33);	
		//CFloatPtr ptrAutoXExposureTimeUpperLimit = nodeMap.GetNode("AutoExposureTimeUpperLimit");
		//ptrAutoXExposureTimeUpperLimit->SetValue(1000000/frameRate);	// us
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	}

	fFrameRate = ptrFrameRate->GetValue();
	cout << "Frame rate is set to " << fFrameRate << endl;
	msg = string_format("Frame rate is set to %f", fFrameRate);
	INFO_LOG(msg);
#else
#endif

	//cout << "DeviceLinkThroughputLimit = " << ptrThroughputLimit->GetValue() << endl;
	msg = string_format("DeviceLinkThroughputLimit = %d", ptrThroughputLimit->GetValue());
	INFO_LOG(msg);
	//cout << "Frame rate is set to " << fFrameRate << endl;
	msg = string_format("Frame rate is set to  %d", fFrameRate);
	INFO_LOG(msg);

#if false
	CFloatPtr ptrFrameRate = pCam->GetNodeMap().GetNode("AcquisitionFrameRate");
	if (!IsAvailable(ptrFrameRate) || !IsWritable(ptrFrameRate))
	{
		//cout << "Frame rate is set to " << ptrFrameRate->GetValue() << endl;
		msg = string_format("Frame rate is set to %f", ptrFrameRate->GetValue());
		EMERG_LOG(msg);
		return -1;
	}
	// Set fps
	fFrameRate = frameRate;
	//ptrFrameRate->SetValue(fFrameRate);
	cout << "[2]Frame rate is set to " << ptrFrameRate->GetValue() << endl;
	msg = string_format("Frame rate is set to %f", ptrFrameRate->GetValue());
	INFO_LOG(msg);
#endif

	return 0;
}

// GPIO
int CameraFormat::SetGpioUserMode()
{
#if true // OK
	SetTriggerSouce();
	SetLineSelector();
	SetLineMode();
	SetLineSource();
#endif

#if false	// NG
	SetLineSelector();
	SetLineMode();
	SetLineSource();
	SetUserOutputSelector();
	//SetUserOutputValue(false);	// low = ON
	SetUserOutputValue(true);	// high = OFF
	Set3_3Voltage(false);
#endif
	return 1;
}

// GPIO
int CameraFormat::SetGpioStrobeMode()
{
	SetTriggerSouce();
	SetLineSelector();
	SetLineMode();
	SetLineSource();

	return 1;
}

int CameraFormat::SetTriggerSouce()
{
	// It retrieves GenICam nodemap
  INodeMap& nodeMap = pCam->GetNodeMap();

	CEnumerationPtr triggerSelector = nodeMap.GetNode("TriggerSelector");
	triggerSelector->SetIntValue(triggerSelector->GetEntryByName("FrameStart")->GetValue());

	CEnumerationPtr triggerMode = nodeMap.GetNode("TriggerMode");
	triggerMode->SetIntValue(triggerMode->GetEntryByName("Off")->GetValue());

	CEnumerationPtr triggerSource = nodeMap.GetNode("TriggerSource");
	triggerSource->SetIntValue(triggerSource->GetEntryByName("Line0")->GetValue());

	CEnumerationPtr triggerActivation = nodeMap.GetNode("TriggerActivation");
	triggerActivation->SetIntValue(triggerActivation->GetEntryByName("FallingEdge")->GetValue());

	return 1;
}

int CameraFormat::SetLineSelector()
{
	// use line 1.

	// It retrieves GenICam nodemap
  INodeMap& nodeMap = pCam->GetNodeMap();
  
	// This is for Serial port Transmit settings
	CEnumerationPtr ptrLineSelector = nodeMap.GetNode("LineSelector");
	if (!IsAvailable(ptrLineSelector) || !IsWritable(ptrLineSelector))
	{
			//cout << "Unable to set Line Selector. Aborting..." << endl << endl;
			EMERG_LOG(string("Unable to set Line Selector. Aborting..."));
			return -1;
	}
	ptrLineSelector->SetIntValue(1); // line 1 is selected(Line1)

	//cout << endl << "SetLineSelector() : ptrLineSelector->GetIntValue() = " << ptrLineSelector->GetIntValue() << endl;
	msg = string_format("SetLineSelector() : ptrLineSelector->GetIntValue() = %d", ptrLineSelector->GetIntValue());
	INFO_LOG(msg);
	
	return 1;
}


int CameraFormat::SetLineMode()
{
	// set to output mode

	// It retrieves GenICam nodemap
  INodeMap& nodeMap = pCam->GetNodeMap();

	CEnumerationPtr ptrLineMode = nodeMap.GetNode("LineMode");
	if (!IsAvailable(ptrLineMode) || !IsWritable(ptrLineMode))
	{
			cout << "Unable to set Line Mode. Aborting..." << endl << endl;
			return -1;
	}
	ptrLineMode->SetIntValue(1); // output is selected(Output)

	//cout << endl << "SetLineMode() : ptrLineMode->GetIntValue() = " << ptrLineMode->GetIntValue() << endl;
	msg = string_format("SetLineMode() : ptrLineMode->GetIntValue() = %d", ptrLineMode->GetIntValue());
	INFO_LOG(msg);
	
	return 1;
}

int CameraFormat::SetLineSource()
{
	// It retrieves GenICam nodemap
  INodeMap& nodeMap = pCam->GetNodeMap();

	CEnumerationPtr ptrLineSource = nodeMap.GetNode("LineSource");
	if (!IsAvailable(ptrLineSource) || !IsWritable(ptrLineSource))
	{
			//cout << "Unable to set Line Source. Aborting..." << endl << endl;
			EMERG_LOG(string("Unable to set Line Source. Aborting..."));
			return -1;
	}
	// 0 : ExposureActive(Enable, On, Strobe out)
	// 1 : ExternalTriggerActive(Disable, Off)	w/ trigger source = line 0
	// 2 : UserOutput1(Not use)
	//ptrLineSource->SetIntValue(2); // User Output1 2 is selected(UserOutput1)
	//ptrLineSource->SetIntValue(1); // User Output1 1 is selected(ExternalTriggerActive, Default Off)

	//ptrLineSource->SetIntValue(0);	// on
	ptrLineSource->SetIntValue(2);	// off : ???
	ptrLineSource->SetIntValue(1);	// off : ???(good)

	//cout << endl << "SetLineSource() : ptrLineSource->GetIntValue() = " << ptrLineSource->GetIntValue() << endl;
	msg = string_format("SetLineSource() : ptrLineSource->GetIntValue() = %d", ptrLineSource->GetIntValue());
	INFO_LOG(msg);
	
	return 1;
}

int CameraFormat::SetUserOutputSelector()
{
	// It retrieves GenICam nodemap
  INodeMap& nodeMap = pCam->GetNodeMap();

	CEnumerationPtr ptrUserOutputSource = nodeMap.GetNode("UserOutputSelector");
	if (!IsAvailable(ptrUserOutputSource) || !IsWritable(ptrUserOutputSource))
	{
			//cout << "Unable to set User Output Source. Aborting..." << endl << endl;
			EMERG_LOG(string("Unable to set User Output Source. Aborting..."));
			return -1;
	}
	//ptrUserOutputSource->SetIntValue(1); // User Output Source 1 is selected(UserOutputValue)

	//cout << endl << "SetUserOutputSelector() : ptrUserOutputSource->GetIntValue() = " << ptrUserOutputSource->GetIntValue() << endl;
	msg = string_format("SetUserOutputSelector() : ptrUserOutputSource->GetIntValue() = %d", ptrUserOutputSource->GetIntValue());
	INFO_LOG(msg);

	return 1;
}

int CameraFormat::SetUserOutputValue(bool value)
{
	CBooleanPtr ptrUserOutputValue = pCam->GetNodeMap().GetNode("UserOutputValue");
	if (!IsAvailable(ptrUserOutputValue) || !IsWritable(ptrUserOutputValue))
	{
		//cout << "Unable to set User Output Value. Aborting..." << endl << endl;
		EMERG_LOG(string("Unable to set User Output Value. Aborting..."));
		return -1;
	}

	ptrUserOutputValue->SetValue(value);

	//cout << endl << "SetUserOutputValue() : ptrUserOutputValue->GetValue() = " << ptrUserOutputValue->GetValue() << endl;
	msg = string_format("SetUserOutputValue() : ptrUserOutputValue->GetValue() = %d", ptrUserOutputValue->GetValue());
	INFO_LOG(msg);

	return 1;
}

int CameraFormat::Set3_3Voltage(bool value)
{
	CBooleanPtr ptrV3_3Enable = pCam->GetNodeMap().GetNode("V3_3Enable");
	if (!IsAvailable(ptrV3_3Enable) || !IsWritable(ptrV3_3Enable))
	{
		//cout << "Unable to set V3_3Enable. Aborting..." << endl << endl;
		EMERG_LOG(string("Unable to set V3_3Enable. Aborting..."));
		return -1;
	}

	ptrV3_3Enable->SetValue(value);

	//cout << endl << "Set3_3Voltage() : ptrV3_3Enable->GetValue() = " << ptrV3_3Enable->GetValue() << endl;
	msg = string_format("Set3_3Voltage() : ptrV3_3Enable->GetValue() = %d", ptrV3_3Enable->GetValue());
	INFO_LOG(msg);

	return 1;
}

// add. by ariar : 2022.05.22 - begin
// SetImageProcess
int CameraFormat::SetImageProcess(bool bImgProcEn, bool bGammaEn, bool bSharpEn, int nSharpAuto, float fGamma, int nSharp)
{
	// It retrieves GenICam nodemap
  INodeMap& nodeMap = pCam->GetNodeMap();

	if(bImgProcEn == true)	// enable
	{
		// 1. On Board Color Process Enable
		CBooleanPtr ptrProcessEabled = nodeMap.GetNode("OnBoardColorProcessEnabled");
		if (!IsAvailable(ptrProcessEabled) || !IsWritable(ptrProcessEabled))
		{
			//cout << "Unable to set ptrProcessEabled. Aborting..." << endl << endl;
			EMERG_LOG(string("Unable to set ptrProcessEabled. Aborting..."));
		}
		else 
		{
			ptrProcessEabled->SetValue(bImgProcEn);
		}
		
		// 2. Gamma Enabled
		CBooleanPtr ptrGammaEnabled = nodeMap.GetNode("GammaEnabled");
		if (!IsAvailable(ptrGammaEnabled) || !IsWritable(ptrGammaEnabled))
		{
			//cout << "Unable to set ptrGammaEnabled. Aborting..." << endl << endl;
			EMERG_LOG(string("Unable to set ptrGammaEnabled. Aborting..."));
		}
		else 
		{
			ptrGammaEnabled->SetValue(bGammaEn);	

			// 2.1
			if(bGammaEn == true)
			{
				CFloatPtr ptrGamma = nodeMap.GetNode("Gamma");
				ptrGamma->SetValue(fGamma);
			}
		}

		// 3. Sharpness Enabled
		CBooleanPtr ptrSharpnessEnabled = nodeMap.GetNode("SharpnessEnabled");
		if (!IsAvailable(ptrSharpnessEnabled) || !IsWritable(ptrSharpnessEnabled))
		{
			//cout << "Unable to set ptrSharpnessEnabled. Aborting..." << endl << endl;
			EMERG_LOG(string("Unable to set ptrSharpnessEnabled. Aborting..."));
		}
		else 
		{
			ptrSharpnessEnabled->SetValue(bSharpEn);	

			// 3.1
			if(bSharpEn == true)
			{
				CEnumerationPtr ptrSharpnessAuto = nodeMap.GetNode("SharpnessAuto");
				if (!IsAvailable(ptrSharpnessAuto) || !IsWritable(ptrSharpnessAuto))
				{
						cout << "Unable to set ptrSharpnessAuto. Aborting..." << endl << endl;
						EMERG_LOG(string("Unable to set ptrSharpnessAuto. Aborting..."));
				}
				ptrSharpnessAuto->SetIntValue(nSharpAuto==0 ? 0 : 2);
				
				// SharpnessAuto = off(0)
				if(nSharpAuto == 0)
				{
					CIntegerPtr ptrSharpness = nodeMap.GetNode("Sharpness");
					ptrSharpness->SetValue(nSharp);
				}
			}
		}
	}
	else 
	{
		// 1. On Board Color Process Enable
		CBooleanPtr ptrProcessEabled = nodeMap.GetNode("OnBoardColorProcessEnabled");
		if (!IsAvailable(ptrProcessEabled) || !IsWritable(ptrProcessEabled))
		{
			//cout << "Unable to set ptrProcessEabled. Aborting..." << endl << endl;
			EMERG_LOG(string("Unable to set ptrProcessEabled. Aborting..."));
		}
		else 
		{
			ptrProcessEabled->SetValue(bImgProcEn);
		}
	}

	return 1;
}
// add. by ariar : 2022.05.22 - end