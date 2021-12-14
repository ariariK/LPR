/*
 *	CameraGrab.cpp
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
#include <fcntl.h>

#include "Common.h"
#include "CameraGrab.h"

#define FD_GPIO_ST2	"/sys/class/gpio/gpio158/value"

using namespace std;
using namespace chrono;

class Common gComm;
CameraGrab::CameraGrab()
{
	bSaveEnable = false;
	strSavePath.empty();

#if true
	msq.msg_type = 1;
	msq.data.capWidth = 0;
	msq.data.capHeight = 0;

  msq_img.msg_type = 1;
	msq_img.data.capWidth = 0;
	msq_img.data.capHeight = 0;

	//SharedMemoryCreate();
	//MessageQueueCreate();
#else
#endif
	// shared memory for grab image
	Sm_Grab = new Ipcs(KEY_NUM_SM, MEM_SIZE_SM);
	Sm_Grab->SharedMemoryCreate();

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


CameraGrab::~CameraGrab()
{
	INodeMap& nodeMap = pCam->GetNodeMap();
	CEnumerationPtr ptrLineSource = nodeMap.GetNode("LineSource");
	ptrLineSource->SetIntValue(1);	// OFF

	Sm_Grab->SharedMemoryFree();
	Sm_Res->SharedMemoryFree();
	Sm_Cam->SharedMemoryFree();
	Mq_Grab->MessageQueueFree();
	Mq_GrabImg->MessageQueueFree();

	SAFE_DELETE(Sm_Grab);
	SAFE_DELETE(Sm_Res);
	SAFE_DELETE(Sm_Cam);
	SAFE_DELETE(Mq_Grab);
	SAFE_DELETE(Mq_GrabImg);
}

bool CameraGrab::Init()
{
	return true;
}

int CameraGrab::SetCameraPtr(CameraPtr pcam)
{
	pCam = pcam;

	return 0;
}

int CameraGrab::SetSaveEnable(bool enable)
{
	bSaveEnable = enable;
	return 0;
}

int CameraGrab::SetSavePath(string path)
{
	strSavePath = path;

	return 0;
}

int CameraGrab::SetFrameRateMode(bool enable)
{
	bFrameRateMode = enable;

	return 0;
}

int CameraGrab::SetTargetFPS(float value)
{
	fTargetFPS = value;

	return 0;
}

int CameraGrab::SetGainValue(float value)
{
	fGainValue = value;

	return 0;
}

int CameraGrab::SetGainLow(float value)
{
	fGainValueLow = value;

	return 0;
}

int CameraGrab::SetGainHigh(float value)
{
	fGainValueHigh = value;

	return 0;
}

int CameraGrab::SetExposureMax(float value)
{
	fExposureValueMax = value;

	return 0;
}

int CameraGrab::SetExposureLow(float value)
{
	fExposureValueLow = value;

	return 0;
}

int CameraGrab::SetExposureHigh(float value)
{
	fExposureValueHigh = value;

	return 0;
}

int	CameraGrab::CtrlUserModeGpio()
{
	// get Gain
	// Float node
	CFloatPtr ptrGain = pCam->GetNodeMap().GetNode("Gain");
	msg = string_format("current gain value = %f[dB]", ptrGain->GetValue());
	INFO_LOG(msg);
	//cout << "current gain value = " << ptrGain->GetValue() << " dB" << endl;

	CBooleanPtr ptrUserOutputValue = pCam->GetNodeMap().GetNode("UserOutputValue");
	if (ptrGain->GetValue() < (float)fGainValue) 	// Day
	{
		//ptrUserOutputValue->SetValue(false);
		ptrUserOutputValue->SetValue(true);		// inverted
	}
	else 														// Night
	{
		//ptrUserOutputValue->SetValue(true);
		ptrUserOutputValue->SetValue(false);
	}
	
	return 0;
}

int CameraGrab::SetLineSource()
{
	static unsigned int debounce_cnt = 0;
	static const unsigned int debounce_limit = 30;	// 3sec @ 10fps

	// get ExposureTime
	// Float node
	//int dn;	// 0:on, 1,2:off
	//float gain, expTime;
	//double thValue, thValueLow, thValueHigh; 
	
	INodeMap& nodeMap = pCam->GetNodeMap();
	CFloatPtr ptrGain = pCam->GetNodeMap().GetNode("Gain");
	CFloatPtr exposureTime = nodeMap.GetNode("ExposureTime");
	CEnumerationPtr ptrLineSource = nodeMap.GetNode("LineSource");

	st_cam.capWidth 	= st_grab.capWidth;
	st_cam.capHeight	= st_grab.capHeight;
	st_cam.dnStatus 	= ptrLineSource->GetIntValue();	// 0:on, 1,2:off
	st_cam.gainCur 		= ptrGain->GetValue();
	st_cam.shCur 			= exposureTime->GetValue();
	st_cam.expCur 		= (st_cam.gainCur * fExposureValueMax) + st_cam.shCur;

#if false // for debug
	//Automatic Exposure Time limits
	CFloatPtr ptrAutoXExposureTimeLowerLimit = nodeMap.GetNode("AutoExposureTimeLowerLimit");
	CFloatPtr ptrAutoXExposureTimeUpperLimit = nodeMap.GetNode("AutoExposureTimeUpperLimit");

	cout << "ptrAutoXExposureTimeLowerLimit = " << ptrAutoXExposureTimeLowerLimit->GetValue() << endl;
	cout << "ptrAutoXExposureTimeUpperLimit = " << ptrAutoXExposureTimeUpperLimit->GetValue()<< endl;
#endif

	msg = string_format("[%d] current ExposureTime value = %f[us], current gain value = %f[dB]", st_cam.dnStatus, exposureTime->GetValue(), ptrGain->GetValue());
	DEBUG_LOG(msg);

	if (st_cam.dnStatus == 0)		// led on 
	{
		debounce_cnt++;

		// check on -> off
		if(st_cam.expCur < st_cam.expMin)
		{
			if(debounce_cnt > debounce_limit)
			{
				debounce_cnt = 0;

				//ptrLineSource->SetIntValue(2);
				ptrLineSource->SetIntValue(1);	// org(off)
				//ptrLineSource->SetIntValue(0);

				msg = string_format("[OFF] current ExposureTime value = %f[us], current gain value = %f[dB]", exposureTime->GetValue(), ptrGain->GetValue());
				INFO_LOG(msg);
			}
		}
		else 
		{
			debounce_cnt = 0;
		}
	}
	else			// led off
	{
		debounce_cnt++;

		// check off -> on
		if(st_cam.expCur > st_cam.expMax)
		{
			if(debounce_cnt > debounce_limit)
			{
				debounce_cnt = 0;

				ptrLineSource->SetIntValue(0);

				msg = string_format("[ON] current ExposureTime value = %f[us], current gain value = %f[dB]", exposureTime->GetValue(), ptrGain->GetValue());
				INFO_LOG(msg);
			}
		}
		else
		{
			debounce_cnt = 0;
		}
	}

	// sm data

#if false
	CEnumerationPtr exposureAuto = nodeMap.GetNode("ExposureAuto");
	cout << "current exposureAuto value = " << exposureAuto->GetEntryByName("Continuous")->GetValue() << " [us]" << endl;
	exposureAuto->SetIntValue(exposureAuto->GetEntryByName("Continuous")->GetValue());

	CEnumerationPtr exposureMode = nodeMap.GetNode("ExposureMode");
	cout << "current exposureMode value = " << exposureMode->GetEntryByName("Timed")->GetValue() << " [us]" << endl;
	exposureMode->SetIntValue(exposureMode->GetEntryByName("Timed")->GetValue());

	//if (exposureTime->GetValue() < (float)fGainValue) 	// Day (off = 2)
	if (exposureTime->GetValue() < (float)fGainValue) 	// Day (off = 2)
	{
		//ptrLineSource->SetIntValue(2);
	}
	else 																					// Night (on = 0)
	{
		ptrLineSource->SetIntValue(0);
	}

#endif

	
	//cout << endl << "SetLineSource() : ptrLineSource->GetIntValue() = " << ptrLineSource->GetIntValue() << endl;

	return 0;
}

int CameraGrab::CtrlGPIO(int fd, int value)
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

int CameraGrab::RunGrabbing()
{
	int result = 0;

	INFO_LOG(string("*** IMAGE ACQUISITION ***"));

	INodeMap& nodeMapTLDevice = pCam->GetTLDeviceNodeMap();
	INodeMap& nodeMap 				= pCam->GetNodeMap();
	INodeMap& nodeMapStream 	= pCam->GetTLStreamNodeMap();
	CIntegerPtr StreamNode 		= nodeMapStream.GetNode("StreamDefaultBufferCount");
	
	int fd = open(FD_GPIO_ST2, O_WRONLY);
	if (fd == -1) {
		ERR_LOG(string("Unable to open /sys/class/gpio/gpio158/value"));
	}

	try
	{
		// Retrieve enumeration node from nodemap
		CEnumerationPtr ptrAcquisitionMode = nodeMap.GetNode("AcquisitionMode");
		if (!IsAvailable(ptrAcquisitionMode) || !IsWritable(ptrAcquisitionMode))
		{
			ERR_LOG(string("Unable to set acquisition mode to continuous (enum retrieval). Aborting..."));
			return -1;
		}

		// Retrieve entry node from enumeration node
		CEnumEntryPtr ptrAcquisitionModeContinuous = ptrAcquisitionMode->GetEntryByName("Continuous");
		if (!IsAvailable(ptrAcquisitionModeContinuous) || !IsReadable(ptrAcquisitionModeContinuous))
		{
			ERR_LOG(string("Unable to set acquisition mode to continuous (entry retrieval). Aborting..."));
			return -1;
		}

		// Retrieve integer value from entry node
		const int64_t acquisitionModeContinuous = ptrAcquisitionModeContinuous->GetValue();

		// Set integer value from entry node as new value of enumeration node
		ptrAcquisitionMode->SetIntValue(acquisitionModeContinuous);
		INFO_LOG(string("Acquisition mode set to continuous..."));

		// Get acquisition rate
		CFloatPtr ptrFrameRate = nodeMap.GetNode("AcquisitionFrameRate");
		fFrameRate = ptrFrameRate->GetValue();
		cout << "Frame rate is set to " << fFrameRate << endl;


		// Start Acquisition...
		pCam->BeginAcquisition();

		gcstring deviceSerialNumber("");
		CStringPtr ptrStringSerial = nodeMapTLDevice.GetNode("DeviceSerialNumber");
		if (IsAvailable(ptrStringSerial) && IsReadable(ptrStringSerial))
		{
			deviceSerialNumber = ptrStringSerial->GetValue();
			//cout << "Device serial number retrieved as " << deviceSerialNumber << "..." << endl;
			msg = string_format("Device serial number retrieved as %s", deviceSerialNumber.c_str());
			INFO_LOG(msg);
		}

		//INodeMap& nodeMap = pCam->GetNodeMap();
		// SET shutter ragne : MIN ~ MAX(6.800msec)
		//Automatic Exposure Time limits
		//CFloatPtr ptrAutoXExposureTimeLowerLimit = nodeMap.GetNode("AutoExposureTimeLowerLimit");
		//ptrAutoXExposureTimeLowerLimit->SetValue(33);			// maybe.. auto...
		//CFloatPtr ptrAutoXExposureTimeUpperLimit = nodeMap.GetNode("AutoExposureTimeUpperLimit");
		//ptrAutoXExposureTimeUpperLimit->SetValue(6800);	// us

		// SET gain range : MIN ~ MAX(8.999dB)
		//CFloatPtr ptrGainLowerLimit = nodeMap.GetNode("AutoGainLowerLimit");
		//ptrGainLowerLimit->SetValue(0);				// 0dB
		//CFloatPtr ptrGainUpperLimit = nodeMap.GetNode("AutoGainUpperLimit");
		//ptrGainUpperLimit->SetValue(8.999);			// 8.999dB

		// Initialize
		CIntegerPtr ptrThroughputLimit = nodeMap.GetNode("DeviceLinkThroughputLimit");
		CFloatPtr ptrAutoXExposureTimeLowerLimit = nodeMap.GetNode("AutoExposureTimeLowerLimit");
		CFloatPtr ptrAutoXExposureTimeUpperLimit = nodeMap.GetNode("AutoExposureTimeUpperLimit");
		CFloatPtr ptrGainLowerLimit = nodeMap.GetNode("AutoGainLowerLimit");
		CFloatPtr ptrGainUpperLimit = nodeMap.GetNode("AutoGainUpperLimit");

		ptrAutoXExposureTimeUpperLimit->SetValue(6800);	// us
		ptrGainUpperLimit->SetValue(8.999);							// 8.999dB

		st_cam.capCount = 0;
		st_cam.tarClk 	= ptrThroughputLimit->GetValue();
		st_cam.expMin 	= (fGainValueLow * fExposureValueMax) + fExposureValueLow;
		st_cam.expMax 	= (fGainValueHigh * fExposureValueMax) + fExposureValueHigh;
		st_cam.shMin		= ptrAutoXExposureTimeLowerLimit->GetValue();
		st_cam.shMax		= ptrAutoXExposureTimeUpperLimit->GetValue();
		st_cam.gainMin	= ptrGainLowerLimit->GetValue();
		st_cam.gainMax	= ptrGainUpperLimit->GetValue();

		// Grabbing loop...
		int pack_size = 100;
		//int64_t imageCnt = 0;
		int64_t runningTime = 0;
		int64_t cap_cnt = 0;
		
		int skip_cnt = 1;
		float rate = (fFrameRate/fTargetFPS);
		skip_cnt = (rate > 1) ? (int)rate : skip_cnt;
		skip_cnt = (skip_cnt < 1) ? 1 : skip_cnt;
		cout << "skip count = " << skip_cnt << endl;
		do {
			try
			{
				// elapsed time : start
				system_clock::time_point start = system_clock::now();

				ImagePtr pResultImage = pCam->GetNextImage();

				if (pResultImage->IsIncomplete())
				{
					// Retrieve and print the image status description
				#if false
					cout << "Image incomplete: " << Image::GetImageStatusDescription(pResultImage->GetImageStatus())
							 << "..." << endl
							 << endl;
				#else
#if LOG_LEVEL >= LOG_LEVEL_DEBUG				
					msg = "Image incomplete: " + (string)Image::GetImageStatusDescription(pResultImage->GetImageStatus());
					DEBUG_LOG(msg);
#endif
				#endif
					pResultImage->Release();
					continue;
				}
				else	// OK
				{
					#if true
					if (!bFrameRateMode)
					{
						cap_cnt++;
						//if(cap_cnt%3) continue;
						if(cap_cnt%skip_cnt) continue;
						cap_cnt=0;
					}
					#endif
					

					// optional parameter.
					//
					const size_t width = pResultImage->GetWidth();
					const size_t height = pResultImage->GetHeight();

					// GPIO
					//CtrlUserModeGpio();
					SetLineSource();

					// Update
					// Use message queue
					if (msq.data.capWidth != (int)width || msq.data.capHeight != (int)height || Mq_Grab->MessageQueueQNum() < 1)
					{
						msq.data.capWidth = width;
						msq.data.capHeight = height;
						Mq_Grab->MessageQueueWrite((char*)&msq);
					}

					// Use shared memory
					if (st_grab.capWidth != (int)width || st_grab.capHeight != (int)height)
					{
						st_grab.capWidth = width;
						st_grab.capHeight = height;
						Sm_Res->SharedMemoryWrite((char *)&st_grab, sizeof(struct grab_data));
					}

					// write to shared memory
					ImagePtr convertedImage = pResultImage->Convert(PixelFormat_Mono8, HQ_LINEAR);
					Sm_Grab->SharedMemoryWrite((char*)convertedImage->GetData(), convertedImage->GetBufferSize());
					Sm_Cam->SharedMemoryWrite((char *)&st_cam, sizeof(struct cam_param));

					//////////////////////////////////////////////////////////////////////////////////////////////
					// Update for lpr
					// Use message queue
					if (Mq_GrabImg->MessageQueueQNum() < 1)
					{
						msq.data.capWidth = width;
						msq.data.capHeight = height;
						Mq_GrabImg->MessageQueueWrite((char*)&msq);
					}
					//////////////////////////////////////////////////////////////////////////////////////////////
#if true
					if (bSaveEnable) 
					{
						ostringstream filename_tmp;			// 임시 저장파일
						ostringstream filename;					// 실시간 모니터링 용
						//filename << strSavePath << "/Grab-" << gComm.string_format("%09d", st_cam.capCount) << ".jpg";
						filename << strSavePath << "/monitor_real.jpg";
						filename_tmp << strSavePath << "/monitor_real_tmp.jpg";

						try 
						{
							//ImagePtr convertedImage = pResultImage->Convert(PixelFormat_Mono8, HQ_LINEAR);
							convertedImage->Save(filename_tmp.str().c_str());
							rename(filename_tmp.str().c_str(), filename.str().c_str());
						}
						catch (Spinnaker::Exception& e)
						{
							msg = "Error: " + (string)e.what();
							cout << msg << endl;

							ERR_LOG(msg);
							return -1;
						}
					}
#else	// for test
					if (bSaveEnable) 
					{
						ostringstream filename;
						filename << strSavePath << "/Grab-" << gComm.string_format("%09d", st_cam.capCount) << ".jpg";
						//time_t timer = time(nullptr);
						//filename << strSavePath << "/Grab-" << timer << ".jpg";
						try 
						{
							//ImagePtr convertedImage = pResultImage->Convert(PixelFormat_Mono8, HQ_LINEAR);
							convertedImage->Save(filename.str().c_str());
							//pResultImage->Release();
						}
						catch (Spinnaker::Exception& e)
						{
							msg = "Error: " + (string)e.what();
							cout << msg << endl;

							ERR_LOG(msg);
							return -1;
						}
					}
#endif
					
					st_cam.capCount++;
					system_clock::time_point end = system_clock::now();
					nanoseconds nano = end - start;
					runningTime += nano.count()/1000000;	// msec
					if (st_cam.capCount % pack_size == 0)
					{
						st_cam.capFPS = (double)(1000*pack_size)/runningTime;

						// elapsed time : end
						//cout << "Grabbed image " << st_cam.capCount << ", width = " << width << ", height = " << height << endl;
						//system_clock::time_point end = system_clock::now();
						//nanoseconds nano = end - start;

						//cout << "Elapsed time(msec) : " << nano.count()/1000000 << endl;
						msg = "Grabbed image: " + gComm.string_format("%09d", st_cam.capCount) + ", width = " + to_string(width) + ", height = " + to_string(height) + \
									"\tAverage FPS: " + gComm.string_format("%.2f", st_cam.capFPS);
						//cout << "Grabbed image: " << gComm.string_format("%8d", st_cam.capCount) << ", width = " << width << ", height = " << height
						//		 << "\tAverage FPS: " << gComm.string_format("%.2f", (double)(1000*pack_size)/runningTime) << endl;

						//cout << msg << endl;
						INFO_LOG(msg);

						runningTime = 0;
					}
					//usleep(20000);
				}

				// LED - ST2 동작상태표시
				CtrlGPIO(fd, (st_cam.capCount>>2)&0x1);

				pResultImage->Release();

				//cout << endl;
			}
			catch (Spinnaker::Exception& e)
			{
				msg = "Error: " + (string)e.what();
				//cout << "Error: " << e.what() << endl;
				cout << msg << endl;

				ERR_LOG(msg);

				return -1;
			}

		} while(1);

		// End Acquisition...
		pCam->EndAcquisition();

		close(fd);

	}	// try
	catch (Spinnaker::Exception& e)
	{
		msg = "Error: " + (string)e.what();
		//cout << "Error: " << e.what() << endl;
		cout << msg << endl;
		ERR_LOG(msg);

		close(fd);

		return -1;
	}	// catch

	return result;
}

/*
int CameraGrab::RunAcquisition(CameraPtr pCam, INodeMap& nodeMap, INodeMap& nodeMapTLDevice)
{
	int result = 0;

	cout << endl << endl << "*** IMAGE ACQUISITION ***" << endl << endl;

	try
	{

		// Retrieve enumeration node from nodemap
        	CEnumerationPtr ptrAcquisitionMode = nodeMap.GetNode("AcquisitionMode");
        	if (!IsAvailable(ptrAcquisitionMode) || !IsWritable(ptrAcquisitionMode))
        	{
        	    cout << "Unable to set acquisition mode to continuous (enum retrieval). Aborting..." << endl << endl;
        	    return -1;
        	}

		// Retrieve entry node from enumeration node
        	CEnumEntryPtr ptrAcquisitionModeContinuous = ptrAcquisitionMode->GetEntryByName("Continuous");
        	if (!IsAvailable(ptrAcquisitionModeContinuous) || !IsReadable(ptrAcquisitionModeContinuous))
        	{
        	    cout << "Unable to set acquisition mode to continuous (entry retrieval). Aborting..." << endl << endl;
        	    return -1;
        	}

        	// Retrieve integer value from entry node
        	const int64_t acquisitionModeContinuous = ptrAcquisitionModeContinuous->GetValue();

        	// Set integer value from entry node as new value of enumeration node
        	ptrAcquisitionMode->SetIntValue(acquisitionModeContinuous);

        	cout << "Acquisition mode set to continuous..." << endl;
	
		pCam->BeginAcquisition();

        	cout << "Acquiring images..." << endl;

        	//
        	// Retrieve device serial number for filename
        	//
        	// *** NOTES ***
        	// The device serial number is retrieved in order to keep cameras from
        	// overwriting one another. Grabbing image IDs could also accomplish
        	// this.
        	//
        	gcstring deviceSerialNumber("");
        	CStringPtr ptrStringSerial = nodeMapTLDevice.GetNode("DeviceSerialNumber");
        	if (IsAvailable(ptrStringSerial) && IsReadable(ptrStringSerial))
        	{
        	    deviceSerialNumber = ptrStringSerial->GetValue();

        	    cout << "Device serial number retrieved as " << deviceSerialNumber << "..." << endl;
        	}
        	cout << endl;

		unsigned int imageCnt=0;
		while(1) {
			try
			{
				// start
	                	system_clock::time_point start = system_clock::now();

				ImagePtr pResultImage = pCam->GetNextImage();

				if (pResultImage->IsIncomplete())
                		{

                		    	// Retrieve and print the image status description
                		    	cout << "Image incomplete: " << Image::GetImageStatusDescription(pResultImage->GetImageStatus())
                		    	     << "..." << endl
                		    	     << endl;
                		}
				else 
				{
					imageCnt++;

					const size_t width = pResultImage->GetWidth();
                    			const size_t height = pResultImage->GetHeight();

                    			cout << "Grabbed image " << imageCnt << ", width = " << width << ", height = " << height << endl;

					// optional parameter.
                    			//
                    			ImagePtr convertedImage = pResultImage->Convert(PixelFormat_Mono8, HQ_LINEAR);

                    			// end
                    			system_clock::time_point end = system_clock::now();
                    			nanoseconds nano = end - start;
                    			cout << "Elapsed time(msec) : " << nano.count()/1000000 << endl;

				
#if false	
					// Create a unique filename
                    			ostringstream filename;

                    			filename << "./images/Acquisition-";
                    			if (!deviceSerialNumber.empty())
                    			{
                    			    filename << deviceSerialNumber.c_str() << "-";
                    			}
                    			//filename << imageCnt << ".png";
                    			//filename << imageCnt << ".jpg";
                    			//filename << imageCnt << ".raw";
                    			filename << imageCnt << "." << saveType;

                    			//
                    			// Save image
                    			//
                    			// *** NOTES ***
                    			// The standard practice of the examples is to use device
                    			// serial numbers to keep images of one device from
                    			// overwriting those of another.
                    			//
                    			convertedImage->Save(filename.str().c_str());

                    			cout << "Image saved at " << filename.str() << endl;
#endif
				}

				pResultImage->Release();

				cout << endl;
			}
			catch (Spinnaker::Exception& e)
			{
				cout << "Error: " << e.what() << endl;
				result = -1;
			}

		};

		pCam->EndAcquisition();

	}
	catch (Spinnaker::Exception& e)
	{
		cout << "Error: " << e.what() << endl;
	        return -1;
	}

	return result;
}
*/
