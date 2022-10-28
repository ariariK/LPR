#ifndef _MV_CAMERA_GRABBER_H_
#define _MV_CAMERA_GRABBER_H_

#include "CameraDefine.h"
#include "CameraStatus.h"


MVSDK_API CameraSdkStatus __stdcall CameraGrabber_CreateFromDevicePage(
    void** Grabber
    );

MVSDK_API CameraSdkStatus __stdcall CameraGrabber_CreateByIndex(
    void** Grabber,
    int Index
    );

MVSDK_API CameraSdkStatus __stdcall CameraGrabber_CreateByName(
    void** Grabber,
    char* Name
    );

MVSDK_API CameraSdkStatus __stdcall CameraGrabber_Create(
    void** Grabber,
    tSdkCameraDevInfo* pDevInfo
    );

MVSDK_API CameraSdkStatus __stdcall CameraGrabber_Destroy(
    void* Grabber
    );

MVSDK_API CameraSdkStatus __stdcall CameraGrabber_SetHWnd(
    void* Grabber,
    HWND hWnd
    );

MVSDK_API CameraSdkStatus __stdcall CameraGrabber_SetPriority(
    void* Grabber,
    UINT Priority
    );

MVSDK_API CameraSdkStatus __stdcall CameraGrabber_StartLive(
    void* Grabber
    );

MVSDK_API CameraSdkStatus __stdcall CameraGrabber_StopLive(
    void* Grabber
    );

MVSDK_API CameraSdkStatus __stdcall CameraGrabber_SaveImage(
    void* Grabber,
    void** Image,
    DWORD TimeOut
    );

MVSDK_API CameraSdkStatus __stdcall CameraGrabber_SaveImageAsync(
    void* Grabber
    );

MVSDK_API CameraSdkStatus __stdcall CameraGrabber_SaveImageAsyncEx(
    void* Grabber,
    void* UserData
    );

MVSDK_API CameraSdkStatus __stdcall CameraGrabber_SetSaveImageCompleteCallback(
    void* Grabber,
    pfnCameraGrabberSaveImageComplete Callback,
    void* Context
    );

MVSDK_API CameraSdkStatus __stdcall CameraGrabber_SetFrameListener(
    void* Grabber,
    pfnCameraGrabberFrameListener Listener,
    void* Context
    );

MVSDK_API CameraSdkStatus __stdcall CameraGrabber_SetRawCallback(
    void* Grabber,
    pfnCameraGrabberFrameCallback Callback,
    void* Context
    );

MVSDK_API CameraSdkStatus __stdcall CameraGrabber_SetRGBCallback(
    void* Grabber,
    pfnCameraGrabberFrameCallback Callback,
    void* Context
    );

MVSDK_API CameraSdkStatus __stdcall CameraGrabber_GetCameraHandle(
    void* Grabber,
    CameraHandle *hCamera
    );

MVSDK_API CameraSdkStatus __stdcall CameraGrabber_GetStat(
    void* Grabber,
    tSdkGrabberStat *stat
    );

MVSDK_API CameraSdkStatus __stdcall CameraGrabber_GetCameraDevInfo(
    void* Grabber,
    tSdkCameraDevInfo *DevInfo
    );

#endif // _MV_CAMERA_GRABBER_H_

