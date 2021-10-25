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

#define SKIP_SEND_IMAGE

CommSocket::CommSocket(string patrolID, string vehicleInfo)
{
  memset(patrolCarInfo.strStatusInfo, 0, sizeof(patrolCarInfo.strStatusInfo));
  memset(patrolCarInfo.strVehicleInfo, 0, sizeof(patrolCarInfo.strVehicleInfo));

  memset(patrolResponse.strStatusInfo, 0, sizeof(patrolResponse.strStatusInfo));
  memset(patrolResponse.strVehicleInfo, 0, sizeof(patrolResponse.strVehicleInfo));
  
  strPatrolID     = patrolID;
  strVehicleInfo  = vehicleInfo;
  strncpy(patrolCarInfo.strStatusInfo, strPatrolID.c_str(), strPatrolID.size());

  printf("Patrol ID = %s\n", strPatrolID.c_str());
  printf("Vehicle INFO = %s\n", strVehicleInfo.c_str());
  printf("sizeof(SYSTEMTIME) = %d\n", sizeof(CommSocket::SYSTEMTIME));
  printf("sizeof(ElPatrolCarInfo) = %d\n", sizeof(CommSocket::ElPatrolCarInfo));
  printf("sizeof(ElPatrolImageSend) = %d\n", sizeof(CommSocket::ElPatrolImageSend));
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
  printf("======================================================\n");
  printf("%d-%d-%d_%d:%d:%d.%d\n",  pInfo->dateTime.wYear,
                                    pInfo->dateTime.wMonth,
                                    //pInfo->dateTime.wDayOfWeek,
                                    pInfo->dateTime.wDay,
                                    pInfo->dateTime.wHour,
                                    pInfo->dateTime.wMinute,
                                    pInfo->dateTime.wSecond,
                                    pInfo->dateTime.wMilliseconds
                                    );
  printf("strStatusInfo  : %s\n", pInfo->strStatusInfo);
  printf("strVehicleInfo : %s\n", pInfo->strVehicleInfo);
  printf("======================================================\n\n");
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
    perror("Error creating socket\n"); 
    return -1; 
  }
  printf("successfully created socket\n");

  /* set client_sockaddr */
  struct sockaddr_in client_sockaddr;
  memset(&client_sockaddr, 0, sizeof(struct sockaddr_in));
  client_sockaddr.sin_family      = AF_INET;
  client_sockaddr.sin_port        = htons(server_port);
  inet_pton(AF_INET, server_ip.c_str(), &client_sockaddr.sin_addr.s_addr);

  /* connect */
  printf("connect to server~~~\n");
  int ret = connect(client_sockfd, (struct sockaddr *)&client_sockaddr, sizeof(struct sockaddr_in));
  if (ret == -1) { 
    perror("[C] connect"); 
    return -1; 
  }
  printf("successfully connected\n");

  return 1;
}

int CommSocket::SendPacketPatrolCarInfo(time_t timestamp, string carNumber)
{
  // clear
  memset(patrolCarInfo.strVehicleInfo, 0, sizeof(patrolCarInfo.strVehicleInfo));

  // put new data
  ParseSystemTime(timestamp, (PSYSTEMTIME)&patrolCarInfo.dateTime);
  strncpy(patrolCarInfo.strVehicleInfo, carNumber.c_str(), carNumber.size());

  int writelen;
  /*
   * ssize_t send(int sockfd, const void *buf, size_t len, int flags);
   * flags
    MSG_DONTROUTE : gateway를 통하지 않고 직접 상대시스템으로 전송 
    MSG_DONTWAIT : non blocking에서 사용하는 옵션으로 전송이 block되면 EAGIN, EWOULDBLOCK 오류로 바로 return 함. 
    MSG_MORE : 더 전송할 데이터가 있음을 설정함. 
    MSG_OOB : out of band(긴급데이터) 데이터를 읽습니다. 주로 X.25에서 접속이 끊겼을 때에 전송되는 데이터 flags의 값이 0이면 일반 데이터를 전송하며, write(sockfd, buf, len)를 호출한 것과 같습니다.
   */
  writelen = send(client_sockfd, (char *)&patrolCarInfo, sizeof(patrolCarInfo), 0);
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

int CommSocket::SendPacketImage()
{
  int writelen;
  int fulllen, sendlen;

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

int CommSocket::RecvPacketPatrolResponse()
{
  /* recv */
  int readlen = recv(client_sockfd, (char *)&patrolResponse, sizeof(patrolResponse), 0);
  if (readlen == -1) 
  { 
    perror("[S] recv\n"); 
    return -1; 
  }
  printf("[S] recv : %d\n", readlen);
  //printf("[S] buf \"%s\"\n", buf);

  PrintPacketPatrolInfo((char *)&patrolResponse);
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
    cout << "Error deleting file : " << fname << endl;
  }
  else
  {
    cout << "File successfully deleted : " << fname << endl;
  }
  // crop
  fname = "/userdata/result/" + to_string(delFileInfo.timestamp) + "_" + std::string(delFileInfo.carNo) + "_1.jpg";
  if (remove( fname.c_str()) != 0)
  {
    cout << "Error deleting file : " << fname << endl;
  }
  else
  {
    cout << "File successfully deleted : " << fname << endl;
  }
#endif
  
  return 1;
}