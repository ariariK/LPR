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

#include "CameraConnect.h"

CameraConnect::CameraConnect()
{
	pCam = nullptr;
}

CameraConnect::~CameraConnect()
{
}

bool CameraConnect::Init()
{
	return true;
}

int CameraConnect::Connect()
{
	return 0;
}

int CameraConnect::Disconnect()
{
	return 0;
}
