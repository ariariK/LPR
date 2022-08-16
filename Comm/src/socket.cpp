/*
 *	socket.cpp
 *
 * 
 *
 *
 *
 */
#include "Typedef.h"
#include "socket.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <iostream>
#include <string>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

//#define SKIP_SEND_IMAGE

CommSocket::CommSocket(string patrolID, string vehicleInfo)
{
  memset(patrolCarInfo.strStatusInfo, 0, sizeof(patrolCarInfo.strStatusInfo));
  memset(patrolCarInfo.strVehicleInfo, 0, sizeof(patrolCarInfo.strVehicleInfo));

  memset(patrolResponse.strStatusInfo, 0, sizeof(patrolResponse.strStatusInfo));
  memset(patrolResponse.strVehicleInfo, 0, sizeof(patrolResponse.strVehicleInfo));

  // add. by ariari : 2021.11.30
  memset(patrolImageV2.strPatrolID, 0, sizeof(patrolImageV2.strPatrolID));
  memset(patrolImageV2.strVehicleInfo, 0, sizeof(patrolImageV2.strVehicleInfo));
  
  strPatrolID     = patrolID;
  strVehicleInfo  = vehicleInfo;
  strncpy(patrolCarInfo.strStatusInfo, strPatrolID.c_str(), strPatrolID.size());
  strncpy(patrolCarInfo.strVehicleInfo, vehicleInfo.c_str(), vehicleInfo.size());
  // add. by ariari : 2021.11.30
  strncpy(patrolImageV2.strPatrolID, strPatrolID.c_str(), strPatrolID.size());

  msg = string_format("Patrol ID = %s", strPatrolID.c_str());
  INFO_LOG(msg);
  msg = string_format("Vehicle INFO = %s", strVehicleInfo.c_str());
  INFO_LOG(msg);
  msg = string_format("sizeof(SYSTEMTIME) = %d", sizeof(CommSocket::SYSTEMTIME));
  INFO_LOG(msg);
  msg = string_format("sizeof(ElPatrolHeader) = %d", sizeof(CommSocket::ElPatrolHeader));
  INFO_LOG(msg);
  msg = string_format("sizeof(ElPatrolCarInfo) = %d", sizeof(CommSocket::ElPatrolCarInfo));
  INFO_LOG(msg);
  msg = string_format("sizeof(ElPatrolImageSend) = %d", sizeof(CommSocket::ElPatrolImageSend));
  INFO_LOG(msg);
  msg = string_format("sizeof(ElPatrolImageSendV2) = %d", sizeof(CommSocket::ElPatrolImageSendV2));
  INFO_LOG(msg);
}

CommSocket::~CommSocket()
{

}

int CommSocket::setServerInfo(string ip, int port)
{
  server_ip   = ip;
  server_port = port;

  return 1;
}

int CommSocket::PrintPacketPatrolInfo(char* patrolinfo)
{
  LPElPatrolCarInfo pInfo = (LPElPatrolCarInfo)patrolinfo;
  INFO_LOG(string("#############################################################"));
  msg = string_format("%d-%d-%d_%d:%d:%d.%d",  pInfo->dateTime.wYear,
                                               pInfo->dateTime.wMonth,
                                               //pInfo->dateTime.wDayOfWeek,
                                               pInfo->dateTime.wDay,
                                               pInfo->dateTime.wHour,
                                               pInfo->dateTime.wMinute,
                                               pInfo->dateTime.wSecond,
                                               pInfo->dateTime.wMilliseconds
                      );
  INFO_LOG(msg);  
  msg = string_format("strStatusInfo  : %s", pInfo->strStatusInfo);
  INFO_LOG(msg);
  msg = string_format("strVehicleInfo : %s", pInfo->strVehicleInfo);
  INFO_LOG(msg);
  INFO_LOG(string("#############################################################"));
  return 1;
}

int CommSocket::ParseSystemTime(time_t timestamp, PSYSTEMTIME pSystime)
{
  time_t now;
  struct tm *t;
  now = timestamp/1000;
  t = localtime(&now);

  pSystime->wYear          = t->tm_year + 1900;
  pSystime->wMonth         = t->tm_mon + 1;
  pSystime->wDayOfWeek     = t->tm_wday;
  pSystime->wDay           = t->tm_mday;
  pSystime->wHour          = t->tm_hour;
  pSystime->wMinute        = t->tm_min;
  pSystime->wSecond        = t->tm_sec;
  pSystime->wMilliseconds  = timestamp%1000;

  return 1;
}

int CommSocket::GenTimeT(PFileinfo pFileinfo)
{
  struct tm t;
  t.tm_isdst      = 0;
  t.tm_year       = patrolResponse.dateTime.wYear - 1900;
  t.tm_mon        = patrolResponse.dateTime.wMonth - 1;
  t.tm_wday       = patrolResponse.dateTime.wDayOfWeek;
  t.tm_mday       = patrolResponse.dateTime.wDay;
  t.tm_hour       = patrolResponse.dateTime.wHour;
  t.tm_min        = patrolResponse.dateTime.wMinute;
  t.tm_sec        = patrolResponse.dateTime.wSecond;

  time_t msec = mktime(&t);
  pFileinfo->timestamp = (msec * 1000) + patrolResponse.dateTime.wMilliseconds;

  return 1;
}

time_t CommSocket::getTimemsec()
{
  struct timeval time_now{};
  gettimeofday(&time_now, nullptr);
  time_t msecs_time = (time_now.tv_sec * 1000) + (time_now.tv_usec / 1000);

  return msecs_time;
}

int CommSocket::ConnectToServer(string ip, int port)
{
  server_ip   = ip;
  server_port = port;

  /* create client socket */
  client_sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (client_sockfd == -1) 
  { 
    //perror("Error creating socket\n"); 
    EMERG_LOG(string("Error creating socket"));
    return -1; 
  }
  //printf("successfully created socket\n");
  INFO_LOG(string("successfully created socket"));

  /* set client_sockaddr */
  struct sockaddr_in client_sockaddr;
  memset(&client_sockaddr, 0, sizeof(struct sockaddr_in));
  client_sockaddr.sin_family      = AF_INET;
  client_sockaddr.sin_port        = htons(server_port);
  inet_pton(AF_INET, server_ip.c_str(), &client_sockaddr.sin_addr.s_addr);

  /* connect */
  //printf("connect to server~~~\n");
  INFO_LOG(string("connect to server~~~"));
  int ret = connect(client_sockfd, (struct sockaddr *)&client_sockaddr, sizeof(struct sockaddr_in));
  if (ret == -1) { 
    //perror("[C] connect"); 
    EMERG_LOG(string("Connection Failure!!!"))
    return -1; 
  }
  //printf("successfully connected\n");
  INFO_LOG(string("successfully connected"))

  return 1;
}

int CommSocket::SetDeleteInfo(time_t timestamp, string carNumber)
{
  delFileInfo.timestamp = timestamp;
  delFileInfo.carNo     = carNumber;
  
  return 1;
}

int CommSocket::SendHeader(char code)
{
  int len = sizeof(ElPatrolHeader);

  // put new data
  patrolHeader.code = code;

  int writelen = 0;
  while(writelen < len) {
    writelen += send(client_sockfd, (char *)&patrolHeader + writelen, sizeof(patrolHeader) - writelen, 0);
    //printf("send to server : %d bytes\n", writelen);
  }

  msg = string_format("ElPatrolHeader : (%c) %d", patrolHeader.code, writelen);
  INFO_LOG(msg);
  

  return writelen;
}

// add. by ariar : 2022.02.22 - begin
int CommSocket::RecvDummy()
{
  char buffer[4]; 
  int recv_len;

  cout << "CommSocket::RecvDummy()" << endl;
  recv_len = recv(client_sockfd, buffer, 4, 0);

  return recv_len;
}
// add. by ariar : 2022.02.22 - end

int CommSocket::SendPacketPatrolCarInfo(time_t timestamp, string carNumber)
{
  // clear
  memset(patrolCarInfo.strVehicleInfo, 0, sizeof(patrolCarInfo.strVehicleInfo));

  // put new data
  ParseSystemTime(timestamp, (PSYSTEMTIME)&patrolCarInfo.dateTime);
  strncpy(patrolCarInfo.strVehicleInfo, carNumber.c_str(), carNumber.size());

  int writelen = 0;
  /*
   * ssize_t send(int sockfd, const void *buf, size_t len, int flags);
   * flags
    MSG_DONTROUTE : gateway를 통하지 않고 직접 상대시스템으로 전송 
    MSG_DONTWAIT : non blocking에서 사용하는 옵션으로 전송이 block되면 EAGIN, EWOULDBLOCK 오류로 바로 return 함. 
    MSG_MORE : 더 전송할 데이터가 있음을 설정함. 
    MSG_OOB : out of band(긴급데이터) 데이터를 읽습니다. 주로 X.25에서 접속이 끊겼을 때에 전송되는 데이터 flags의 값이 0이면 일반 데이터를 전송하며, write(sockfd, buf, len)를 호출한 것과 같습니다.
   */
  writelen += send(client_sockfd, (char *)&patrolCarInfo + writelen, sizeof(patrolCarInfo) - writelen, 0);
  //printf("send to server : %d bytes\n", writelen);

#if false
  printf("year %d\n", patrolCarInfo.dateTime.wYear);
  printf("mon %d\n", patrolCarInfo.dateTime.wMonth);
  printf("day %d\n", patrolCarInfo.dateTime.wDayOfWeek);
  printf("wday %d\n", patrolCarInfo.dateTime.wDay);
  printf("hour %d\n", patrolCarInfo.dateTime.wHour);
  printf("min %d\n", patrolCarInfo.dateTime.wMinute);
  printf("sec %d\n", patrolCarInfo.dateTime.wSecond);
  printf("msec %d\n", patrolCarInfo.dateTime.wMilliseconds);

  printf("strStatusInfo : %s\n", patrolCarInfo.strStatusInfo);
  printf("strVehicleInfo : %s\n", patrolCarInfo.strVehicleInfo);
#endif
  

  return writelen;
}

int CommSocket::SendPacketPatrolCarInfoV2(time_t timestamp, string carNumber)
{
  int len = sizeof(patrolCarInfo.strVehicleInfo);
  
  // clear
  //memset(patrolCarInfo.strVehicleInfo, 0, sizeof(patrolCarInfo.strVehicleInfo));

  // put new data
  ParseSystemTime(timestamp, (PSYSTEMTIME)&patrolCarInfo.dateTime);
  //strncpy(patrolCarInfo.strVehicleInfo, carNumber.c_str(), carNumber.size()); // fixed. mod. by ariari : 2021.11.30

  int writelen = 0;
  /*
   * ssize_t send(int sockfd, const void *buf, size_t len, int flags);
   * flags
    MSG_DONTROUTE : gateway를 통하지 않고 직접 상대시스템으로 전송 
    MSG_DONTWAIT : non blocking에서 사용하는 옵션으로 전송이 block되면 EAGIN, EWOULDBLOCK 오류로 바로 return 함. 
    MSG_MORE : 더 전송할 데이터가 있음을 설정함. 
    MSG_OOB : out of band(긴급데이터) 데이터를 읽습니다. 주로 X.25에서 접속이 끊겼을 때에 전송되는 데이터 flags의 값이 0이면 일반 데이터를 전송하며, write(sockfd, buf, len)를 호출한 것과 같습니다.
   */
  while(writelen < len) {
    writelen += send(client_sockfd, (char *)&patrolCarInfo + writelen, sizeof(patrolCarInfo) - writelen, 0);
    //printf("send to server : %d bytes\n", writelen);
  }
  //printf("SendPacketPatrolCarInfo : %d\n", writelen);
  msg = string_format("SendPacketPatrolCarInfo : %d", writelen);
  DEBUG_LOG(msg);

#if false
  printf("year %d\n", patrolCarInfo.dateTime.wYear);
  printf("mon %d\n", patrolCarInfo.dateTime.wMonth);
  printf("day %d\n", patrolCarInfo.dateTime.wDayOfWeek);
  printf("wday %d\n", patrolCarInfo.dateTime.wDay);
  printf("hour %d\n", patrolCarInfo.dateTime.wHour);
  printf("min %d\n", patrolCarInfo.dateTime.wMinute);
  printf("sec %d\n", patrolCarInfo.dateTime.wSecond);
  printf("msec %d\n", patrolCarInfo.dateTime.wMilliseconds);

  printf("strStatusInfo : %s\n", patrolCarInfo.strStatusInfo);
  printf("strVehicleInfo : %s\n", patrolCarInfo.strVehicleInfo);
#endif
  

  return writelen;
}

int CommSocket::SendPacketImage()
{
  int writelen;
  int fulllen, sendlen;

  // 수배차량 확인
  if(strcmp(patrolResponse.strStatusInfo, "수배차량") != 0)
  {
    cout << "SendPacketImage() : 비수배차량" << endl;
    return 1;
  }
  cout << "SendPacketImage() : 수배차량" << endl;

  string fname0 = "/userdata/result/" + to_string(delFileInfo.timestamp) + "_" + std::string(delFileInfo.carNo) + "_0.jpg";
  string fname1 = "/userdata/result/" + to_string(delFileInfo.timestamp) + "_" + std::string(delFileInfo.carNo) + "_1.jpg";

  memset(patrolImage.strPatrolID, 0, sizeof(patrolImage.strPatrolID));
  memset(patrolImage.strVehicleInfo, 0, sizeof(patrolImage.strVehicleInfo));

  time_t msecs_time = getTimemsec();
  ParseSystemTime(msecs_time, (PSYSTEMTIME)&patrolImage.dateTime);
  strncpy((char *)patrolImage.strPatrolID, strPatrolID.c_str(), strPatrolID.size());
  strncpy((char *)patrolImage.strVehicleInfo, strVehicleInfo.c_str(), strVehicleInfo.size());

  // 1. 원본 이미지
  std::ifstream is0(fname0, std::ifstream::binary);
  if (is0) 
  {
    is0.seekg(0, is0.end);
    patrolImage.lcommand        = ELANPRPP_CMD_SENDIMAGE;
    patrolImage.dwImageDataSize = is0.tellg();
    is0.seekg(0, is0.beg);

    cout << "file name : " << fname0 << endl;
    cout << "file length : " << patrolImage.dwImageDataSize << endl;

    //////////////////////////////////////////////////////////////////////////////////////////////
    // send to server - header
    //////////////////////////////////////////////////////////////////////////////////////////////
    fulllen = sizeof(patrolImage);
    sendlen = 0;
    while(sendlen < fulllen)
    {
      writelen = send(client_sockfd, (char *)&patrolImage + sendlen, fulllen - sendlen, 0);
      sendlen += writelen;
    }
    cout << "finished(org-header) : " << sendlen << "/" << fulllen << endl;
    //////////////////////////////////////////////////////////////////////////////////////////////

#ifndef SKIP_SEND_IMAGE
    //////////////////////////////////////////////////////////////////////////////////////////////
    // send to server - data
    //////////////////////////////////////////////////////////////////////////////////////////////
    // malloc으로 메모리 할당
		unsigned char * buffer = (unsigned char*)malloc(patrolImage.dwImageDataSize);
    is0.read((char*)buffer, patrolImage.dwImageDataSize);

    fulllen = patrolImage.dwImageDataSize;
    sendlen = 0;
    while(sendlen < fulllen)
    {
      writelen = send(client_sockfd, (char *)buffer + sendlen, fulllen - sendlen, 0);
      sendlen += writelen;
    }
    cout << "finished(org-data) : " << sendlen << "/" << fulllen << endl;

  	is0.close();
    free(buffer);
    //////////////////////////////////////////////////////////////////////////////////////////////
#endif    
  }

  // 2. 번호판 이미지
  std::ifstream is1(fname1, std::ifstream::binary);
  if (is1) 
  {
    is1.seekg(0, is1.end);
    patrolImage.lcommand        = ELANPRPP_CMD_SENDIMAGE;
    patrolImage.dwImageDataSize = is1.tellg();
    is1.seekg(0, is1.beg);

    cout << "file name : " << fname0 << endl;
    cout << "file length : " << patrolImage.dwImageDataSize << endl;

    //////////////////////////////////////////////////////////////////////////////////////////////
    // send to server - header
    //////////////////////////////////////////////////////////////////////////////////////////////
    fulllen = sizeof(patrolImage);
    sendlen = 0;
    while(sendlen < fulllen)
    {
      writelen = send(client_sockfd, (char *)&patrolImage + sendlen, fulllen - sendlen, 0);
      sendlen += writelen;
    }
    cout << "finished(crop-header) : " << sendlen << "/" << fulllen << endl;
    //////////////////////////////////////////////////////////////////////////////////////////////

#ifndef SKIP_SEND_IMAGE
    //////////////////////////////////////////////////////////////////////////////////////////////
    // send to server - data
    //////////////////////////////////////////////////////////////////////////////////////////////
    // malloc으로 메모리 할당
		unsigned char * buffer = (unsigned char*)malloc(patrolImage.dwImageDataSize);
    is1.read((char*)buffer, patrolImage.dwImageDataSize);

    fulllen = patrolImage.dwImageDataSize;
    sendlen = 0;
    while(sendlen < fulllen)
    {
      writelen = send(client_sockfd, (char *)buffer + sendlen, fulllen - sendlen, 0);
      sendlen += writelen;
    }
    cout << "finished(crop-data) : " << sendlen << "/" << fulllen << endl;

  	is1.close();
    free(buffer);
    //////////////////////////////////////////////////////////////////////////////////////////////
#endif
  }

  return 1;
}

int CommSocket::SendPacketImageOrg(time_t timestamp, string carNumber, string status, int x, int y, int endX, int endY)
{
  int writelen;
  int fulllen, sendlen;

  string fname0 = "/userdata/result/" + to_string(delFileInfo.timestamp) + "_" + std::string(delFileInfo.carNo) + "_0.jpg";
  //string fname1 = "/userdata/result/" + to_string(delFileInfo.timestamp) + "_" + std::string(delFileInfo.carNo) + "_1.jpg";

  // init
  //memset(patrolImageV2.strPatrolID, 0, sizeof(patrolImageV2.strPatrolID));  // fix
  memset(patrolImageV2.strVehicleInfo, 0, sizeof(patrolImageV2.strVehicleInfo));

  // put data
  time_t msecs_time = getTimemsec();
  ParseSystemTime(msecs_time, (PSYSTEMTIME)&patrolImageV2.dateTime);
  //strncpy((char *)patrolImageV2.strPatrolID, strPatrolID.c_str(), strPatrolID.size());  // fix, not change
  strncpy((char *)patrolImageV2.strType, status.c_str(), status.size());
  strncpy((char *)patrolImageV2.strVehicleInfo, carNumber.c_str(), carNumber.size());
  patrolImageV2.x = x;
  patrolImageV2.y = y;
  patrolImageV2.endX = endX;
  patrolImageV2.endY = endY;

  // 1. 원본 이미지
  std::ifstream is0(fname0, std::ifstream::binary);
  if (is0) 
  {
    is0.seekg(0, is0.end);
    patrolImageV2.lcommand        = ELANPRPP_CMD_SENDIMAGE;
    patrolImageV2.dwImageDataSize = is0.tellg();
    is0.seekg(0, is0.beg);

    //cout << "file name : " << fname0 << endl;
    //cout << "file length : " << patrolImageV2.dwImageDataSize << endl;
    msg = string_format("file name : %s, file length : %d", fname0.c_str(), patrolImageV2.dwImageDataSize);
    INFO_LOG(msg);

    //////////////////////////////////////////////////////////////////////////////////////////////
    // send to server - header
    //////////////////////////////////////////////////////////////////////////////////////////////
    fulllen = sizeof(patrolImageV2);
    sendlen = 0;
    while(sendlen < fulllen)
    {
      writelen = send(client_sockfd, (char *)&patrolImageV2 + sendlen, fulllen - sendlen, 0);
      if(writelen < 0) {
        //cout << "SendPacketImageOrg(org-header) : " << writelen << endl;
        msg = string_format("SendPacketImageOrg(org-header) : %d", writelen);
        EMERG_LOG(msg);
        return -1;
      }
      sendlen += writelen;
    }
    //cout << "finished(org-header) : " << sendlen << "/" << fulllen << endl;
    msg = string_format("finished(org-header) : %d", fulllen);
    INFO_LOG(msg);
    //////////////////////////////////////////////////////////////////////////////////////////////

#ifndef SKIP_SEND_IMAGE
    //////////////////////////////////////////////////////////////////////////////////////////////
    // send to server - data
    //////////////////////////////////////////////////////////////////////////////////////////////
    // malloc으로 메모리 할당
		unsigned char * buffer = (unsigned char*)malloc(patrolImageV2.dwImageDataSize);
    is0.read((char*)buffer, patrolImageV2.dwImageDataSize);

    fulllen = patrolImageV2.dwImageDataSize;
    sendlen = 0;
    while(sendlen < fulllen)
    {
      writelen = send(client_sockfd, (char *)buffer + sendlen, fulllen - sendlen, 0);
      if(writelen < 0) 
      {
        //cout << "SendPacketImageOrg(org-data) : " << writelen << endl;
        msg = string_format("SendPacketImageOrg(org-data) : %d", writelen);
        EMERG_LOG(msg);
        return -1;
      }
      sendlen += writelen;
    }
    //cout << "finished(org-data) : " << sendlen << "/" << fulllen << endl;
    msg = string_format("finished(org-data) : %d", fulllen);
    INFO_LOG(msg);

  	is0.close();
    free(buffer);
    //////////////////////////////////////////////////////////////////////////////////////////////
#endif    
  }

  return 1;
}

int CommSocket::SendPacketImageLpd()
{
  int writelen;
  int fulllen, sendlen;

  //string fname0 = "/userdata/result/" + to_string(delFileInfo.timestamp) + "_" + std::string(delFileInfo.carNo) + "_0.jpg";
  string fname1 = "/userdata/result/" + to_string(delFileInfo.timestamp) + "_" + std::string(delFileInfo.carNo) + "_1.jpg";

  // init
  //memset(patrolImageV2.strPatrolID, 0, sizeof(patrolImageV2.strPatrolID));  // fix
  memset(patrolImageV2.strVehicleInfo, 0, sizeof(patrolImageV2.strVehicleInfo));

  time_t msecs_time = getTimemsec();
  ParseSystemTime(msecs_time, (PSYSTEMTIME)&patrolImageV2.dateTime);
  //strncpy((char *)patrolImageV2.strPatrolID, strPatrolID.c_str(), strPatrolID.size());  // fix, not change
  strncpy((char *)patrolImageV2.strVehicleInfo, strVehicleInfo.c_str(), strVehicleInfo.size());

  // 2. 번호판 이미지
  std::ifstream is1(fname1, std::ifstream::binary);
  if (is1) 
  {
    is1.seekg(0, is1.end);
    patrolImageV2.lcommand        = ELANPRPP_CMD_SENDIMAGE;
    patrolImageV2.dwImageDataSize = is1.tellg();
    is1.seekg(0, is1.beg);

    //cout << "file name : " << fname1 << endl;
    //cout << "file length : " << patrolImageV2.dwImageDataSize << endl;
    msg = string_format("file name : %s, file length : %d", fname1.c_str(), patrolImageV2.dwImageDataSize);
    INFO_LOG(msg);

    //////////////////////////////////////////////////////////////////////////////////////////////
    // send to server - header
    //////////////////////////////////////////////////////////////////////////////////////////////
    fulllen = sizeof(patrolImageV2);
    sendlen = 0;
    while(sendlen < fulllen)
    {
      writelen = send(client_sockfd, (char *)&patrolImageV2 + sendlen, fulllen - sendlen, 0);
      if(writelen < 0) return -1;
      sendlen += writelen;
    }
    //cout << "finished(crop-header) : " << sendlen << "/" << fulllen << endl;
    msg = string_format("finished(crop-header) : %d", fulllen);
    INFO_LOG(msg);
    //////////////////////////////////////////////////////////////////////////////////////////////

#ifndef SKIP_SEND_IMAGE
    //////////////////////////////////////////////////////////////////////////////////////////////
    // send to server - data
    //////////////////////////////////////////////////////////////////////////////////////////////
    // malloc으로 메모리 할당
		unsigned char * buffer = (unsigned char*)malloc(patrolImageV2.dwImageDataSize);
    is1.read((char*)buffer, patrolImageV2.dwImageDataSize);

    fulllen = patrolImageV2.dwImageDataSize;
    sendlen = 0;
    while(sendlen < fulllen)
    {
      writelen = send(client_sockfd, (char *)buffer + sendlen, fulllen - sendlen, 0);
      if(writelen < 0) return -1;
      sendlen += writelen;
    }
    //cout << "finished(crop-data) : " << sendlen << "/" << fulllen << endl;
    msg = string_format("finished(crop-data) : %d", sendlen);
    INFO_LOG(msg);

  	is1.close();
    free(buffer);
    //////////////////////////////////////////////////////////////////////////////////////////////
#endif
  }

  return 1;
}

int CommSocket::RecvPacketPatrolResponse()
{
  /* recv */
  int readlen = recv(client_sockfd, (char *)&patrolResponse, sizeof(patrolResponse), 0);
  if (readlen == -1) 
  { 
    perror("[RecvPacketPatrolResponse] recv\n"); 
    return -1; 
  }
  printf("[RecvPacketPatrolResponse] recv : %d\n", readlen);
  //printf("[S] buf \"%s\"\n", buf);

  PrintPacketPatrolInfo((char *)&patrolResponse);
  // 차량 번호 조회를 완료한 데이터는 삭제
  GenTimeT((PFileinfo)&delFileInfo);
  delFileInfo.carNo = string(patrolResponse.strVehicleInfo);
  //printf("timestamp : %ld, car_no : %s\n", delFileInfo.timestamp, delFileInfo.carNo.c_str());
  
  return readlen;
}

int CommSocket::UpdateImageList()
{
#if true
  string fname;
  fname = "/userdata/result/" + to_string(delFileInfo.timestamp) + "_" + std::string(delFileInfo.carNo) + "_0.jpg";
  if (remove( fname.c_str()) != 0)
  {
    //cout << "Error deleting file : " << fname << endl;
    msg = string_format("Error deleting file : %s", fname.c_str());
    ERR_LOG(msg);
  }
  else
  {
    //cout << "File successfully deleted : " << fname << endl;
    msg = string_format("File successfully deleted : %s", fname.c_str());
    INFO_LOG(msg);
  }
  // crop
  fname = "/userdata/result/" + to_string(delFileInfo.timestamp) + "_" + std::string(delFileInfo.carNo) + "_1.jpg";
  if (remove( fname.c_str()) != 0)
  {
    //cout << "Error deleting file : " << fname << endl;
    msg = string_format("Error deleting file : %s", fname.c_str());
    ERR_LOG(msg);
  }
  else
  {
    //cout << "File successfully deleted : " << fname << endl;
    msg = string_format("File successfully deleted : %s", fname.c_str());
    INFO_LOG(msg);
  }
#endif
  
  return 1;
}
