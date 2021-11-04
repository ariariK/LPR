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

#define THREAD_MAX_NUM  3
bool bIsRunningSocketSend;
bool bIsRunningSocketRecv;
bool bIsRunningUsage;
pthread_mutex_t mutex_socket;

struct lpdr_data{
    long timestamp;
    char carNo[64];
};
struct message_lpdr{
    long msg_type;
    struct lpdr_data data;
};
struct message_lpdr msq_lpdr;


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

  ////////////////////////////////////////////////////////////////////////////////////
  // Release
  ////////////////////////////////////////////////////////////////////////////////////
  Mq_Grab->MessageQueueFree();
  
  SAFE_DELETE(comm);
  SAFE_DELETE(usage);
  SAFE_DELETE(Mq_Grab);
    
  return 1;
}