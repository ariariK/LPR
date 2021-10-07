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
//#include <string.h>
//#include <unistd.h>

#include "Common.h"
#include "CameraGrab.h"

using namespace std;
using namespace chrono;

 
///////////////////////////////////////////////////////////////////////////////////////////
#define  KEY_NUM_SM		1234
#define  MEM_SIZE_SM	512*4096

#define  KEY_NUM_MQ   2345

int CameraGrab::SharedMemoryFree(void)
{
	if(shmctl(shmid, IPC_RMID, 0) == -1) 
	{
		msg = "Shmctl failed";
		ERR_LOG(msg);

		return -1;
	}

	msg = "Shared memory end";
	INFO_LOG(msg);

	return 1;
}

int CameraGrab::SharedMemoryCreate()
{
	msg = "SharedMemoryCreate()";
	INFO_LOG(msg);

	if ((shmid = shmget((key_t)KEY_NUM_SM, MEM_SIZE_SM, IPC_CREAT| IPC_EXCL | 0666)) == -1) 
	{
		msg = "There was shared memory";
		INFO_LOG(msg);

		shmid = shmget((key_t)KEY_NUM_SM, MEM_SIZE_SM, IPC_CREAT| 0666);
		if(shmid == -1)
		{
			msg = "[0] Shared memory create fail";
			ERR_LOG(msg);

			return -1;
		}
		else
		{
			shmid = shmget((key_t)KEY_NUM_SM, MEM_SIZE_SM, IPC_CREAT| 0666);
			if(shmid == -1)
			{
				msg = "[1] Shared memory create fail";
				ERR_LOG(msg);

				return -1;
			}
		}
	}
    
  return 1;
}

int CameraGrab::SharedMemoryWrite(ImagePtr shareddata, int size)
{
	void *shmaddr;
	if(size > MEM_SIZE_SM)
	{
		msg = "Shared memory size over";
		ERR_LOG(msg);

		return -1;
	}

	if((shmaddr = shmat(shmid, (void *)0, 0)) == (void *)-1) 
	{
		msg = "Shmat failed";
		ERR_LOG(msg);

		return -1;
	}

	memcpy((char *)shmaddr, shareddata->GetData(), size);

	if(shmdt(shmaddr) == -1) 
	{
		msg = "Shmdt failed";
		ERR_LOG(msg);
		return -1;
	}

	return 1;
}

int CameraGrab::MessageQueueCreate()
{
	// Get Message Queue ID
	if ((msqid = msgget((key_t)KEY_NUM_MQ, IPC_CREAT|0666))==-1)
	{
		msg = "MessageQueueCreate() : msgget failed";
		ERR_LOG(msg);

		return -1;
	}

	return 1;
}

int CameraGrab::MessageQueueQNum()
{
	struct msqid_ds msqds;
	if (msgctl(msqid, IPC_STAT, (struct msqid_ds*)&msqds)==-1)
	{
		return -1;
	}
	//printf("msqds = %d, %d\n", msqds.msg_qnum, msqds.msg_cbytes);
	return msqds.msg_qnum;
}

int CameraGrab::MessageQueueWrite()
{
	//if (msgsnd(msqid, &msq, sizeof(struct real_data), 0)==-1)						// blocking
	if (msgsnd(msqid, &msq, sizeof(struct real_data), IPC_NOWAIT)==-1)	// non blocking
	{
		msg = "MessageQueueWrite() : msgsnd failed";
		ERR_LOG(msg);

		return -1;
	}

	return 1;
}
int CameraGrab::MessageQueueFree()
{
	if (msgctl(msqid, IPC_RMID, NULL)==-1)
	{
		msg = "MessageQueueFree() : msgctl failed";
		ERR_LOG(msg);

		return -1;
  }

	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////

class Common gComm;
CameraGrab::CameraGrab()
{
	bSaveEnable = false;
	strSavePath.empty();

#if true
	msq.msg_type = 1;
	msq.data.capWidth = 0;
	msq.data.capHeight = 0;

  SharedMemoryCreate();
	MessageQueueCreate();
#endif
}


CameraGrab::~CameraGrab()
{
	SharedMemoryFree();
	MessageQueueFree();
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

int CameraGrab::RunGrabbing()
{
	int result = 0;

	cout << endl << endl << "*** IMAGE ACQUISITION ***" << endl << endl;

	INodeMap& nodeMapTLDevice = pCam->GetTLDeviceNodeMap();
	INodeMap& nodeMap 				= pCam->GetNodeMap();

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


		// Start Acquisition...
		pCam->BeginAcquisition();

		gcstring deviceSerialNumber("");
		CStringPtr ptrStringSerial = nodeMapTLDevice.GetNode("DeviceSerialNumber");
		if (IsAvailable(ptrStringSerial) && IsReadable(ptrStringSerial))
		{
			deviceSerialNumber = ptrStringSerial->GetValue();
			cout << "Device serial number retrieved as " << deviceSerialNumber << "..." << endl;
		}

		// Grabbing loop...
		int pack_size = 100;
		int64_t imageCnt = 0;
		int64_t runningTime = 0;
	
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
					msg = "Image incomplete: " + (string)Image::GetImageStatusDescription(pResultImage->GetImageStatus());
					WARN_LOG(msg);
				#endif
					continue;
				}
				else
				{
					// optional parameter.
					//
					const size_t width = pResultImage->GetWidth();
					const size_t height = pResultImage->GetHeight();

					// Update
					if (msq.data.capWidth != (int)width || msq.data.capHeight != (int)height || MessageQueueQNum() < 1)
					{
						msq.data.capWidth = width;
						msq.data.capHeight = height;
						MessageQueueWrite();
					}

					// write to shared memory
					ImagePtr convertedImage = pResultImage->Convert(PixelFormat_Mono8, HQ_LINEAR);
					SharedMemoryWrite(convertedImage, convertedImage->GetBufferSize());

					if (bSaveEnable) 
					{
						ostringstream filename;
						filename << strSavePath << "/Grab-" << gComm.string_format("%09d", imageCnt) << ".jpg";
						//time_t timer = time(nullptr);
						//filename << strSavePath << "/Grab-" << timer << ".jpg";
						try 
						{
							//ImagePtr convertedImage = pResultImage->Convert(PixelFormat_Mono8, HQ_LINEAR);
							convertedImage->Save(filename.str().c_str());
							pResultImage->Release();
						}
						catch (Spinnaker::Exception& e)
						{
							msg = "Error: " + (string)e.what();
							cout << msg << endl;

							ERR_LOG(msg);
							return -1;
						}
					}
					
					imageCnt++;
					system_clock::time_point end = system_clock::now();
					nanoseconds nano = end - start;
					runningTime += nano.count()/1000000;	// msec
					if (imageCnt % pack_size == 0)
					{
						// elapsed time : end
						//cout << "Grabbed image " << imageCnt << ", width = " << width << ", height = " << height << endl;
						//system_clock::time_point end = system_clock::now();
						//nanoseconds nano = end - start;

						//cout << "Elapsed time(msec) : " << nano.count()/1000000 << endl;
						msg = "Grabbed image: " + gComm.string_format("%09d", imageCnt) + ", width = " + to_string(width) + ", height = " + to_string(height) + \
									"\tAverage FPS: " + gComm.string_format("%.2f", (double)(1000*pack_size)/runningTime);
						//cout << "Grabbed image: " << gComm.string_format("%8d", imageCnt) << ", width = " << width << ", height = " << height
						//		 << "\tAverage FPS: " << gComm.string_format("%.2f", (double)(1000*pack_size)/runningTime) << endl;

						//cout << msg << endl;
						INFO_LOG(msg);

						runningTime = 0;
					}
				}

				//pResultImage->Release();

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

	}	// try
	catch (Spinnaker::Exception& e)
	{
		msg = "Error: " + (string)e.what();
		//cout << "Error: " << e.what() << endl;
		cout << msg << endl;

		ERR_LOG(msg);

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
