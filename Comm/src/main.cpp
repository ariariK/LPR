/*
 *	main.cpp
 *
 * 
 *
 *
 *
 */

#include "Typedef.h"
#include "ipcs.h"
#include "socket.h"
#include "usage.h"


#include <iostream>
#include <string>
#include <time.h>
#include <cstdio>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <sched.h>
#include <ctype.h>
#include <pthread.h>
#include <queue>
#include <sys/time.h>

#define LOG_NAME	"[Comm]"

#define THREAD_MAX_NUM  3
bool bIsRunningSocketMan;
bool bIsRunningSocketSend;
bool bIsRunningSocketRecv;
bool bIsRunningUsage;
pthread_mutex_t mutex_socket;

string msg;

struct lpdr_data{
  long timestamp;

  char status[32];    // 수배종류
  char carNo[32];     // 차량정보
  // RECT
  int x;              // rect[0]
  int y;              // rect[1]    
  int endX;           // rect[2]
  int endY;           // rect[3]
};
struct message_lpdr{
    long msg_type;
    struct lpdr_data data;
};
struct message_lpdr msq_lpdr;

time_t getTimemsec()
{
    struct timeval time_now{};
    gettimeofday(&time_now, nullptr);
    time_t msecs_time = (time_now.tv_sec * 1000) + (time_now.tv_usec / 1000);

    return msecs_time;
}

// add. by ariari : 2022.02.22 - begin
void * thread_socket_ctrl(void* args)
{
  //return nullptr; // 협의 후 적용. 적용할 경우 해당 라인 삭제

  CommSocket* comm = (CommSocket*)args;
  string log;

  while(bIsRunningSocketMan)
  {
    if( comm->RecvDummy() <= 0)
    {
      bIsRunningSocketMan = false;
      cout << "closed socket, exit..." << endl;
    }
    else 
    {
      usleep(10000);
    }
  }

  log = "EXIT Thread(thread_socket_ctrl)";
  INFO_LOG(log);

  return nullptr;
}
// add. by ariari : 2022.02.22 - end

void* thread_socket_man(void* args)
{
  CommSocket* comm = (CommSocket*)args;

  Ipcs* Mq_Lpdr   = new Ipcs(KEY_NUM_MQ_LPDR, 0);
  Mq_Lpdr->MessageQueueInit();

  string log;
  string fname;

  int sent  = 0;
#if true  // once
//#if false // every times
  // send patrol car info : once
  // S0 : Send Header
  sent = comm->SendHeader('S');
  if (sent < 0)  
  {
    cout << "broken socket, exit..." << endl;
    //pthread_mutex_unlock(&mutex_socket);

    // delete
    //comm->UpdateImageList();
    //break;
    bIsRunningSocketMan = false; // EXIT
  }

  // S1 : send CarInfo
  //sent = comm->SendPacketPatrolCarInfoV2((time_t)msq_lpdr.data.timestamp, std::string(msq_lpdr.data.carNo));
  sent = comm->SendPacketPatrolCarInfoV2(getTimemsec(), std::string("")); 
  if (sent < 0)  
  {
    cout << "broken socket, exit..." << endl;
    //pthread_mutex_unlock(&mutex_socket);

    // delete
    //comm->UpdateImageList();
    //break;
    bIsRunningSocketMan = false; // EXIT
  }
#endif

  while(bIsRunningSocketMan)
  {
    int qnum = Mq_Lpdr->MessageQueueQNum();
    //cout << "mq_lpdr queue size = " << qnum << endl;

    sent  = 0;
    if(qnum)  // not empty
    {
      //pthread_mutex_lock(&mutex_socket);
      Mq_Lpdr->MessageQueueRead((char *)&msq_lpdr);
      //cout << "msq_lpdr.data.timestamp = " << msq_lpdr.data.timestamp << endl;   
      //cout << "msq_lpdr.data.status = " << msq_lpdr.data.status << endl;   
      //cout << "msq_lpdr.data.carNo = " << msq_lpdr.data.carNo << endl;   
      //cout << "msq_lpdr.data.x = " << msq_lpdr.data.x << endl;   
      //cout << "msq_lpdr.data.y = " << msq_lpdr.data.y << endl;   
      //cout << "msq_lpdr.data.endX = " << msq_lpdr.data.endX << endl;   
      //cout << "msq_lpdr.data.endY = " << msq_lpdr.data.endY << endl;   
      msg = string_format("msq_lpdr.data.timestamp = %ld, msq_lpdr.data.status = %s, msq_lpdr.data.carNo = %s, (rect) x:%d, y:%d, endX:%d, endY:%d", msq_lpdr.data.timestamp, msq_lpdr.data.status, msq_lpdr.data.carNo, msq_lpdr.data.x, msq_lpdr.data.y, msq_lpdr.data.endX, msq_lpdr.data.endY);
      DEBUG_LOG(msg);

      // SetFileInfo
      comm->SetDeleteInfo((time_t)msq_lpdr.data.timestamp, std::string(msq_lpdr.data.carNo));

#if false // send patrol car info(every times)
      // S0 : Send Header
      sent = comm->SendHeader('S');
      if (sent < 0)  
      {
        cout << "broken socket, exit..." << endl;
        //pthread_mutex_unlock(&mutex_socket);

        // delete
        comm->UpdateImageList();
        break;
      }

      // S1 : send CarInfo
      sent = comm->SendPacketPatrolCarInfoV2((time_t)msq_lpdr.data.timestamp, std::string(msq_lpdr.data.carNo));
      if (sent < 0)  
      {
        cout << "broken socket, exit..." << endl;
        //pthread_mutex_unlock(&mutex_socket);

        // delete
        comm->UpdateImageList();
        break;
      }
#endif

      // S3 : Send Header
      sent = comm->SendHeader('I');

      // S4 : send CarInfo & Image(ORG)
      sent = comm->SendPacketImageOrg((time_t)msq_lpdr.data.timestamp, std::string(msq_lpdr.data.carNo), std::string(msq_lpdr.data.status), msq_lpdr.data.x, msq_lpdr.data.y, msq_lpdr.data.endX, msq_lpdr.data.endY);
      if (sent < 0)  
      {
        cout << "broken socket, exit..." << endl;
        //pthread_mutex_unlock(&mutex_socket);

        // delete
        comm->UpdateImageList();
        break;
      }

// rem. by ariari : 2021.11.30
#if false
      // S5 : Send Header
      sent = comm->SendHeader('I');
      if (sent < 0)  
      {
        cout << "broken socket, exit..." << endl;
        //pthread_mutex_unlock(&mutex_socket);
        break;
      }

      // S6 : send CarInfo & Image(LPD)
      sent = comm->SendPacketImageLpd();
      if (sent < 0)  
      {
        cout << "broken socket, exit..." << endl;
        //pthread_mutex_unlock(&mutex_socket);

        // delete
        comm->UpdateImageList();
        break;
      }
#endif      
      // delete
      comm->UpdateImageList();

      //pthread_mutex_unlock(&mutex_socket);

      cout << endl;
    }
    else
    {
      usleep(10000);
    }
  }
  Mq_Lpdr->MessageQueueFree();

  log = "EXIT Thread(socket_man)";
  INFO_LOG(log);

  return nullptr;
}


void* thread_socket_send(void* args)
{
  CommSocket* comm = (CommSocket*)args;

  Ipcs* Mq_Lpdr   = new Ipcs(KEY_NUM_MQ_LPDR, 0);
  Mq_Lpdr->MessageQueueInit();

  string log;
  string fname;
  while(bIsRunningSocketSend)
  {
    int qnum = Mq_Lpdr->MessageQueueQNum();
    //cout << "mq_lpdr queue size = " << qnum << endl;
    if(qnum)  // not empty
    {
      pthread_mutex_lock(&mutex_socket);
      Mq_Lpdr->MessageQueueRead((char *)&msq_lpdr);

      //cout << "msq_lpdr.data = " << msq_lpdr.data.timestamp << endl;

      int writelen = comm->SendPacketPatrolCarInfo((time_t)msq_lpdr.data.timestamp, std::string(msq_lpdr.data.carNo));
      cout << "Send : " << writelen << " bytes" << endl;
      pthread_mutex_unlock(&mutex_socket);

      if(writelen > 0)
      {
        // OK
      }
      else
      {
        log = "EXIT Thread(Error!!! - thread_socket_send)";
        ERR_LOG(log);
        exit(1);
      }
    }
    else 
    {
      //usleep(1000000);
      usleep(10000);
    }
   }

  Mq_Lpdr->MessageQueueFree();

  log = "EXIT Thread(socket_send)";
  INFO_LOG(log);

  return nullptr;
}

void* thread_socket_recv(void* args)
{
  CommSocket* comm = (CommSocket*)args;

  string log;
  string fname;
  while(bIsRunningSocketRecv)
  {
    int readlen = comm->RecvPacketPatrolResponse();
    cout << "Received : " << readlen << " bytes" << endl;

    if(readlen > 0) 
    {
      pthread_mutex_lock(&mutex_socket);
      // 수배차량, 비수배차량 체크
      // 이미지전송
      comm->SendPacketImage();

      // 업데이트(삭제)
      comm->UpdateImageList();
      pthread_mutex_unlock(&mutex_socket);
    }
    else 
    {
      log = "EXIT Thread(Error!!! - thread_socket_recv)";
      ERR_LOG(log);
      exit(1);
    }

    usleep(10000);
  }

  log = "EXIT Thread(socket_recv)";
  INFO_LOG(log);
  return nullptr;
}

void* thread_usage(void* args)
{
  Usage *usage = (Usage*)args;

  string log;
  Usage::MEM_OCCUPY meminfo;
  while(bIsRunningUsage)
  {
    usage->Getproceminfo((Usage::MEM_OCCUPY*)&meminfo);
    double memFree = (double)(meminfo.MemFree*100)/meminfo.MemTotal;
    double memUsage = (double)(meminfo.MemAvailable*100)/meminfo.MemTotal;

#if false
    cout << string(meminfo.name1) << " : " << meminfo.MemTotal << endl;
    cout << string(meminfo.name2) << " : " << meminfo.MemFree << endl;
    cout << string(meminfo.name3) << " : " << meminfo.MemAvailable << endl;
    cout << string(meminfo.name4) << " : " << meminfo.Buffers << endl;
    cout << string(meminfo.name5) << " : " << meminfo.Cached << endl;
    cout << "Memory Usage : " << (100-memFree) << "%" << endl;
    cout << "Memory Free : " << memFree << "%" << endl << endl;
#endif

    usleep(1000000);
  }
  log = "EXIT Thread(usage)";
  INFO_LOG(log);

  return nullptr;
}

int main(int argc, char** argv)
{
  string log;
  int CPU_NUM = sysconf(_SC_NPROCESSORS_CONF);

  openlog(LOG_NAME, LOG_PID, LOG_USER);

  log = "System has " + to_string(CPU_NUM) + "processor(s).";
  INFO_LOG(log);

  ////////////////////////////////////////////////////////////////////////////////////
  // User parameters
  int res;
  string strPatrolID = "";
  string strVehicleInfo = "";
  string server_ip = "";
  int server_port;
	while ((res=getopt(argc, argv, "i:v:s:p:h")) != -1) {
		switch(res) {
		case 'i':   // patrolID
      strPatrolID = optarg;
		  break;

    case 'v':   // vehicle info
      strVehicleInfo = optarg;
      break;
			
		case 's':   // server ip
			server_ip = optarg;
			break;

    case 'p':   // server port
			server_port = strtol(optarg, NULL, 10);;
			break;
			
		case 'h':
      std::cout << " [Usage]: " << argv[0] << " [-h]\n"
	              << " [-i partolID] "
                << " [-v vehicleInfo] "
                << " [-s server ip address] "
                << " [-p server port number] "
                << endl;
			break;
		}
	}
  ////////////////////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////////////////
  // Initialize
  ////////////////////////////////////////////////////////////////////////////////////
  Ipcs* Mq_Grab   = new Ipcs(KEY_NUM_MQ_GRAB, 0);
  //CommSocket* comm  = new CommSocket("AI Edge Device - No.1", "123가 4567");
  CommSocket* comm  = new CommSocket(strPatrolID, strVehicleInfo);
  Usage* usage      = new Usage();
  Mq_Grab->MessageQueueInit();
  
  // socket 
  //if (comm->ConnectToServer(SERVER_IP, SERVER_PORT) < 0)
  if (comm->ConnectToServer(server_ip, server_port) < 0)
  {
    return -1;
  }

  // thread create
#if true  // new version
  comm->patrolHeader.HEADER[0] = 0xAA;
  comm->patrolHeader.HEADER[1] = 0x55;
  comm->patrolHeader.HEADER[2] = 0xFE;

  bIsRunningSocketMan   = true;
  bIsRunningSocketSend  = false;
  bIsRunningSocketRecv  = false;
  bIsRunningUsage       = false;
  pthread_mutex_init(&mutex_socket, NULL);     // init mutex
  //pthread_t thread[1];
  // add. by ariari : 2022.02.22 for checking server down(disconnected)
  pthread_t thread[2];
  pthread_create(&thread[0], NULL, thread_socket_man, comm);
  // add. by ariari : 2022.02.22 for checking server down(disconnected)
  pthread_create(&thread[1], NULL, thread_socket_ctrl, comm);

  // thread join
  //for(int i = 0; i < 1; i++) {
  // add. by ariari : 2022.02.22 for checking server down(disconnected)
  for(int i = 0; i < 2; i++) {
        pthread_join(thread[i], NULL);
    }
#else
  bIsRunningSocketSend  = true;
  bIsRunningSocketRecv  = true;
  bIsRunningUsage       = true;
  pthread_mutex_init(&mutex_socket, NULL);     // init mutex
  pthread_t thread[THREAD_MAX_NUM];
  pthread_create(&thread[0], NULL, thread_socket_send, comm);
  pthread_create(&thread[1], NULL, thread_socket_recv, comm);
  pthread_create(&thread[2], NULL, thread_usage, usage);

  // thread join
  for(int i = 0; i < THREAD_MAX_NUM; i++) {
        pthread_join(thread[i], NULL);
    }
#endif
  ////////////////////////////////////////////////////////////////////////////////////
  // Release
  ////////////////////////////////////////////////////////////////////////////////////
  Mq_Grab->MessageQueueFree();
  
  SAFE_DELETE(comm);
  SAFE_DELETE(usage);
  SAFE_DELETE(Mq_Grab);

  closelog();
    
  return 1;
}