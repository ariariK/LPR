/*
 *	Main.cpp
 *
 * 
 *
 *
 *
 */

#include <iostream>
#include <sstream>
#include <fstream>		// ifstream
#include <algorithm>	// remove_if
#include <chrono>
#include <pthread.h>
#include <signal.h>

#include "Typedef.h"
#include "CameraManager.h"

#define LOG_NAME	"[GigEGrab]"

// add. by ariari : 2021.12.07 - begin
CameraManager *pCamMan;
void* thread_grab(void* arg)
{
	//CameraManager *pCamMan = (CameraManager*)arg;
	pCamMan = (CameraManager*)arg;

	// Running Grabbing
	pCamMan->RunGrabbing();

	// Release Camera
	pCamMan->CameraRelease();

	// Exit
	SAFE_DELETE(pCamMan);

	return nullptr;
}
// add. by ariari : 2021.12.07 - end

int signalHandler(int sig, void *ptr){
	
	//static my_struct saved = NULL;

	if(sig==SIGINT){
		printf("signal SIGINT\n");

		SAFE_DELETE(pCamMan);

		sleep(1);

		exit(0);
	}
	if(sig==SIGQUIT){
		printf("signal SIGQUIT\n");
	}

	return 0;
}

int main(int argc, char** argv)
{
	// 프로그램 초기값
	string msg;
	int nCapWidth 			= GIGE_CAMERA_CAP_WIDTH_FHD;
	int nCapHeight			= GIGE_CAMERA_CAP_HEIHGT_FHD;
	int nOffsetX				= GIGE_CAMERA_OFFSET_X;
	int nOffsetY				= GIGE_CAMERA_OFFSET_Y;
	bool bFrameRateMode = GIGE_CAMERA_FRAME_RATE_ENABLE ? true : false;
	float fFrameRate		= GIGE_CAMERA_FRAME_RATE;
	bool bSaveEnable		= false;
	float fNightGainLow		=  5.0;
	float fNightGainHigh	= 20.0;
	float fExposureMax  = 9000.0;
	float fExposureLow	= 5000.0;
	float fExposureHigh	= 9000.0;
	string savePath;

	signal(SIGINT, (void (*)(int))signalHandler);
  signal(SIGQUIT, (void (*)(int))signalHandler);

	msg = "LOG_LEVEL = " + to_string(LOG_LEVEL);
	INFO_LOG(msg);

	//openlog(LOG_NAME, LOG_CONS, LOG_USER);
	openlog(LOG_NAME, LOG_PID, LOG_USER);

	// Parse parameters
	// std::ifstream is RAII, i.e. no need to call close
  //ifstream cFile ("/oem/config_lpr.txt");
	ifstream cFile;

	// User Parameters
	int res;
	string file_cfg = "/oem/config_lpr.txt";
	while ((res=getopt(argc, argv, "f:h")) != -1) {
		switch(res) {
		case 'f':   // configure file
			file_cfg = optarg;
		  break;

		case 'h':
      std::cout << " [Usage]: " << argv[0] << " [-h]\n"
	              << " [-f configure file]\n";
			break;
		}
	}

	cFile.open(file_cfg);
	if (cFile.is_open())
	{
		string line;
    while(getline(cFile, line)) {

			line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());

			if (line[0] == '#' || line.empty())
			{
				continue;
			}

			auto delimiterPos = line.find("=");
			auto name = line.substr(0, delimiterPos);
			auto value = line.substr(delimiterPos + 1);
			
			cout << name << " " << value << endl;

			/////////////////////////////////////////////////////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////////////////////////////////////
			if (name.compare("capWidth") == 0)
			{
				nCapWidth = stoi(value);
			}
			else if (name.compare("capHeight") == 0)
			{
				nCapHeight = stoi(value);
			}
			else if (name.compare("offsetX") == 0)
			{
				nOffsetX = stoi(value);
			}
			else if (name.compare("offsetY") == 0)
			{
				nOffsetY = stoi(value);
			}
			else if (name.compare("frameRateMode") == 0)
			{
				bFrameRateMode = stoi(value) ? true : false;
			}
			else if (name.compare("frameRate") == 0)
			{
				fFrameRate = stof(value);
			}
			else if (name.compare("nightGainLow") == 0)
			{
				fNightGainLow = stof(value);
			}
			else if (name.compare("nightGainHigh") == 0)
			{
				fNightGainHigh = stof(value);
			}
			else if (name.compare("exposureMax") == 0)
			{
				fExposureMax = stof(value);
			}
			else if (name.compare("exposureLow") == 0)
			{
				fExposureLow = stof(value);
			}
			else if (name.compare("exposureHigh") == 0)
			{
				fExposureHigh = stof(value);
			}
			else if (name.compare("saveEnable") == 0)
			{
				bSaveEnable = stoi(value) ? true : false;
			}
			else if (name.compare("savePath") == 0)
			{
				savePath = value;
			}
			/////////////////////////////////////////////////////////////////////////////////////////////////////////
		};

		cFile.close();
	}
	else 
	{
		WARN_LOG(string("Couldn't open config file for reading..."));
		EMERG_LOG(string("Run defalut mode"));

		return -1;
	}
	// config params
	INFO_LOG(string("#################################################################################################"));
	INFO_LOG(string("# GigEGrab Configs                                                                              #"));
	INFO_LOG(string("#################################################################################################"));
	msg = ">> capWidth : " + to_string(nCapWidth);
	INFO_LOG(msg);
	msg = ">> nCapHeight : " + to_string(nCapHeight);
	INFO_LOG(msg);
	msg = ">> nOffsetX : " + to_string(nOffsetX);
	INFO_LOG(msg);
	msg = ">> nOffsetY : " + to_string(nOffsetY);
	INFO_LOG(msg);
	msg = ">> fFrameRate : " + to_string(fFrameRate);
	INFO_LOG(msg);
	msg = ">> fNightGainLow : " + to_string(fNightGainLow);
	INFO_LOG(msg);
	msg = ">> fNightGainHigh : " + to_string(fNightGainHigh);
	INFO_LOG(msg);
	msg = ">> fExposureMax : " + to_string(fExposureMax);
	INFO_LOG(msg);
	msg = ">> fExposureLow : " + to_string(fExposureLow);
	INFO_LOG(msg);
	msg = ">> fExposureHigh : " + to_string(fExposureHigh);
	INFO_LOG(msg);
	INFO_LOG(string("#################################################################################################"));

#if true		// new using thread

	// 객체 생성
	CameraManager *pCamMan = new CameraManager();

	// 초기화 - 카메라 검색 및 상태 확인
	INFO_LOG(string("#################################################################################################"));
	INFO_LOG(string("Starting initialize GigE Camera!!!"));
	if (!pCamMan->Init())
	{
		//cout << "Failure initialize GigE Camera!!!" << endl;
		EMERG_LOG(string("Failure initialize GigE Camera!!!, Exit Program!!!"));
		INFO_LOG(string("#################################################################################################"));

		SAFE_DELETE(pCamMan);

		closelog();
		return -1;
	}
	
	// Ready Camera
	pCamMan->CameraReady(0);

	// Setting Camera Format
	pCamMan->SetCapWidth(nCapWidth);
	pCamMan->SetCapHeight(nCapHeight);

	pCamMan->SetOffsetX(nOffsetX);
	pCamMan->SetOffsetY(nOffsetY);

	pCamMan->SetFrameRateMode(bFrameRateMode);
	pCamMan->SetFrameRate(fFrameRate);

	//pCamMan->SetGpioUserMode(fNightGain);
	pCamMan->SetGpioStrobeMode();
	pCamMan->SetGainLow(fNightGainLow);
	pCamMan->SetGainLow(fNightGainLow);
	pCamMan->SetGainHigh(fNightGainHigh);
	pCamMan->SetExposureMax(fExposureMax);
	pCamMan->SetExposureLow(fExposureLow);
	pCamMan->SetExposureHigh(fExposureHigh);
	
	pCamMan->SetSaveEnable(bSaveEnable);
	pCamMan->SetSavePath(savePath);
	INFO_LOG(string("#################################################################################################"));


	pthread_t thread;
	cpu_set_t mask_grab;
	CPU_ZERO(&mask_grab);
	CPU_SET(4, &mask_grab);
	pthread_create(&thread, NULL, thread_grab, pCamMan);
	pthread_join(thread, NULL);

#else

	// 객체 생성
	CameraManager *pCamMan = new CameraManager();

	// 초기화 - 카메라 검색 및 상태 확인
	INFO_LOG(string("#################################################################################################"));
	INFO_LOG(string("Starting initialize GigE Camera!!!"));
	if (!pCamMan->Init())
	{
		//cout << "Failure initialize GigE Camera!!!" << endl;
		EMERG_LOG(string("Failure initialize GigE Camera!!!, Exit Program!!!"));
		INFO_LOG(string("#################################################################################################"));

		SAFE_DELETE(pCamMan);

		closelog();
		return -1;
	}
	
	// Ready Camera
	pCamMan->CameraReady(0);

	// Setting Camera Format
	pCamMan->SetCapWidth(nCapWidth);
	pCamMan->SetCapHeight(nCapHeight);

	pCamMan->SetOffsetX(nOffsetX);
	pCamMan->SetOffsetY(nOffsetY);

	pCamMan->SetFrameRateMode(bFrameRateMode);
	pCamMan->SetFrameRate(fFrameRate);

	//pCamMan->SetGpioUserMode(fNightGain);
	pCamMan->SetGpioStrobeMode();
	pCamMan->SetGainLow(fNightGainLow);
	pCamMan->SetGainLow(fNightGainLow);
	pCamMan->SetGainHigh(fNightGainHigh);
	pCamMan->SetExposureMax(fExposureMax);
	pCamMan->SetExposureLow(fExposureLow);
	pCamMan->SetExposureHigh(fExposureHigh);
	
	pCamMan->SetSaveEnable(bSaveEnable);
	pCamMan->SetSavePath(savePath);
	INFO_LOG(string("#################################################################################################"));

	// Running Grabbing
	pCamMan->RunGrabbing();

	// Release Camera
	pCamMan->CameraRelease();

	// Exit
	SAFE_DELETE(pCamMan);
#endif

	closelog();
	return 0;
}
