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

#include "Typedef.h"
#include "CameraManager.h"

int main(int argc, char** argv)
{
	// 프로그램 초기값
	int nCapWidth 			= GIGE_CAMERA_CAP_WIDTH_FHD;
	int nCapHeight			= GIGE_CAMERA_CAP_HEIHGT_FHD;
	int nOffsetX				= GIGE_CAMERA_OFFSET_X;
	int nOffsetY				= GIGE_CAMERA_OFFSET_Y;
	bool bFrameRateMode = GIGE_CAMERA_FRAME_RATE_ENABLE ? true : false;
	float fFrameRate		= GIGE_CAMERA_FRAME_RATE;
	bool bSaveEnable		= false;
	string savePath;

	openlog("[LPR-FLIR]", LOG_CONS, LOG_USER);

	// Parse parameters
	// std::ifstream is RAII, i.e. no need to call close
  ifstream cFile ("config_lpr.txt");
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
		cout << "Couldn't open config file for reading..." << endl;
    cout << "Run defalut mode" << endl;
	}

	// 객체 생성
	CameraManager *pCamMan = new CameraManager();

	// 초기화 - 카메라 검색 및 상태 확인
	if (!pCamMan->Init())
	{
		cout << "Failure initialize GigE Camera!!!" << endl;
		SAFE_DELETE(pCamMan);

		closelog();
		return -1;
	}

	// Ready Camera
	pCamMan->CameraReady(0);

	// Setting Camera Format
	//pCamMan->SetCapWidth(GIGE_CAMERA_CAP_WIDTH_FHD);
	//pCamMan->SetCapHeight(GIGE_CAMERA_CAP_HEIHGT_FHD);
	//pCamMan->SetCapWidth(GIGE_CAMERA_CAP_WIDTH_HD);
	//pCamMan->SetCapHeight(GIGE_CAMERA_CAP_HEIHGT_HD);
	pCamMan->SetCapWidth(nCapWidth);
	pCamMan->SetCapHeight(nCapHeight);

	pCamMan->SetOffsetX(nOffsetX);
	pCamMan->SetOffsetY(nOffsetY);

	pCamMan->SetFrameRateMode(bFrameRateMode);
	pCamMan->SetFrameRate(fFrameRate);

	// Running Grabbing
	pCamMan->SetSaveEnable(bSaveEnable);
	pCamMan->SetSavePath(savePath);
	pCamMan->RunGrabbing();

	// Release Camera
	pCamMan->CameraRelease();

	// Exit
	SAFE_DELETE(pCamMan);

	closelog();
	return 0;
}
