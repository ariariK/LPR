/*
 *	socket.h
 *
 * 
 *
 *
 *
 */

#pragma once
#include <string>

#define SERVER_IP       "192.168.40.10"
//#define SERVER_IP       "10.14.2.61"
#define SERVER_PORT	    2000

#define ELANPRPP_CMD_NOTRECEIVED		0x00
#define ELANPRPP_CMD_SENDIMAGE			0x10
#define ELANPRPP_CMD_SRV_CONFIRM		0x20

#define SLOT_NOT_AVAILABLE				  "SLOT_NOT_AVAILABLE"

class CommSocket
{
//////////////////////////////////////////////////////////////////////////////////////////////////////	
// Construction
//////////////////////////////////////////////////////////////////////////////////////////////////////	
public:
	CommSocket(string patrolID, string vehicleInfo);
	virtual ~CommSocket();

//////////////////////////////////////////////////////////////////////////////////////////////////////	
// Attributes
//////////////////////////////////////////////////////////////////////////////////////////////////////	
public:

private:
  int client_sockfd;

  string  server_ip;
  int     server_port;

protected:
  string 			msg;

//////////////////////////////////////////////////////////////////////////////////////////////////////	
// Operations
//////////////////////////////////////////////////////////////////////////////////////////////////////	
public:
  int setServerInfo(string ip, int port);

/////////////////////////////////////////////////////////////////////////////////////////////////////	
// Implementation
//////////////////////////////////////////////////////////////////////////////////////////////////////	

public:
  //////////////////////////////////////////////////////////////////////////////////////////////////////	
	// IPC : Shared Memory
	//////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////	

  //////////////////////////////////////////////////////////////////////////////////////////////////////	
	// Socket
	//////////////////////////////////////////////////////////////////////////////////////////////////////
  // sizeof() = 16Bytes
  typedef struct _SYSTEMTIME {
		short wYear;
		short wMonth;
		short wDayOfWeek;
		short wDay;
		short wHour;
		short wMinute;
		short wSecond;
		short wMilliseconds;
	} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;

  // sizeof() = 4Bytes
  typedef struct ELPATROLHEADER
  {
    char HEADER[3];
    char code;
  } ElPatrolHeader, *LPElPatrolHeader;

  // sizeof() = 304Bytes
  typedef struct ELPATROLCARINFO
  {
    SYSTEMTIME		dateTime;					      // 차량번호 검출시간
    char			    strStatusInfo[32];			// 경찰차 ID 정보, UTF8
    char			    strVehicleInfo[256];		// 차량번호, UTF8
  } ElPatrolCarInfo, *LPElPatrolCarInfo;

  typedef struct ELPATROLRESPONSE
  {
    SYSTEMTIME    dateTime; 		          // 차량번호 검출시간
    char				  strStatusInfo[32]; 	    // ‘수배차량’ 혹은 ‘비수배차량’ [UTF8]
    char				  strVehicleInfo[256];	  // 차량번호를 UTF8 형태로 저장
  } ElpatrolResponse, *LPElpatrolResponse;
    

  // sizeof() = 320Bytes
  typedef struct ELPATROLIMAGESEND
  {
    SYSTEMTIME		dateTime;				        // 일시
    char			    strPatrolID[256];		    // 경찰차 ID 정보, UTF8
    char			    strVehicleInfo[32];		  // 차량번호

    unsigned long	lcommand;               // 8bytes
    unsigned long	dwImageDataSize;        // 8bytes
  } ElPatrolImageSend, *LPElPatrolImageSend;

  // sizeof() = 328Bytes(312+dummy(16))
  typedef struct ELPATROLIMAGESENDV2
  {
    SYSTEMTIME		dateTime;				        // 일시
    char			    strPatrolID[256];		    // 경찰차 ID 정보, UTF8
    char			    strVehicleInfo[32];		  // 차량번호

    // dummy
    unsigned int  dummy[4];               // dummy 16 bytes : total 328bytes

    unsigned int	lcommand;               // 4bytes
    unsigned int	dwImageDataSize;        // 4bytes
  } ElPatrolImageSendV2, *LPElPatrolImageSendV2;


  typedef struct FILEINFO
  {
    time_t  timestamp;
    string  carNo;
  } Fileinfo, *PFileinfo;
  //////////////////////////////////////////////////////////////////////////////////////////////////////	

  //////////////////////////////////////////////////////////////////////////////////////////////////////	
  int ConnectToServer(string ip, int port);
  int SendPacketPatrolCarInfo(time_t timestamp, string carNumber);
  int SendPacketImage();
  int RecvPacketPatrolResponse();
  int UpdateImageList();

  ElPatrolHeader    patrolHeader;
  int SetDeleteInfo(time_t timestamp, string carNumber);
  int SendHeader(char code);
  int SendPacketPatrolCarInfoV2(time_t timestamp, string carNumber, int code);
  int SendPacketImageOrg();
  int SendPacketImageLpd();
  int ProcPacketMan(char* pbuf, int size);

  //////////////////////////////////////////////////////////////////////////////////////////////////////	

  //////////////////////////////////////////////////////////////////////////////////////////////////////	

private:
  string strPatrolID;
  string strVehicleInfo;

  ElPatrolCarInfo     patrolCarInfo;
  ElpatrolResponse    patrolResponse;
  ElPatrolImageSend   patrolImage;
  ElPatrolImageSendV2 patrolImageV2;
  Fileinfo            delFileInfo;

  int PrintPacketPatrolInfo(char* patrolinfo);
  int ParseSystemTime(time_t timestamp, PSYSTEMTIME pSystime);
  int GenTimeT(PFileinfo pFileinfo);
  time_t getTimemsec();

protected:

};