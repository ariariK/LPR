/*
 *	ipcs.cpp
 *
 * 
 *
 *
 *
 */
#include "Typedef.h" 
#include "ipcs.h"

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <errno.h>
#include <sys/time.h>

Ipcs::Ipcs(int key)
{
  mq_created  = false;
  mq_key      = key;
}

Ipcs::~Ipcs()
{

}

int Ipcs::SharedMemoryCreate()
{
  return 1;
}

int Ipcs::SharedMemoryRead(char *sMemory)
{
  return 1;
}

int Ipcs::SharedMemoryFree(void)
{
  return 1;
}

int Ipcs::MessageQueueCreate()
{
  switch(mq_key)
  {
    case KEY_NUM_MQ_GRAB:
      msq_grab.msg_type = 1;

      // Get Message Queue ID
      if ((msqid_grab = msgget((key_t)KEY_NUM_MQ_GRAB, IPC_CREAT|0666))==-1)
      {
        perror("MessageQueueCreate() : msgget(KEY_NUM_MQ_GRAB) failed\n");
        return -1;
      }
      mq_created = true;
      break;

    case KEY_NUM_MQ_LPDR:
      msq_lpdr.msg_type = 1;
 
      // Get Message Queue ID
      if ((msqid_lpdr = msgget((key_t)KEY_NUM_MQ_LPDR, IPC_CREAT|0666))==-1)
      {
        perror("MessageQueueCreate() : msgget(KEY_NUM_MQ_LPDR) failed\n");
        return -1;
      }
      mq_created = true;
      break;
  }

  return 1;
}

int Ipcs::MessageQueueInit()
{
  switch(mq_key)
  {
    case KEY_NUM_MQ_GRAB:
      if ((msqid_grab = msgget((key_t)KEY_NUM_MQ_GRAB, IPC_CREAT|0666)) == -1)
      {
          perror("msgget failed[KEY_NUM_MQ_GRAB]\n");
          return -1;
      }
      break;

    case KEY_NUM_MQ_LPDR:
      if ((msqid_lpdr = msgget((key_t)KEY_NUM_MQ_LPDR, IPC_CREAT|0666)) == -1)
      {
          perror("msgget failed[KEY_NUM_MQ_LPDR]\n");
          return -1;
      }
      break;
  }

  return 1;
}

int Ipcs::MessageQueueQNum()
{
  struct msqid_ds msqds;
  switch(mq_key)
  {
    case KEY_NUM_MQ_GRAB:
      if (msgctl(msqid_grab, IPC_STAT, (struct msqid_ds*)&msqds)==-1)
      {
        return -1;
      }
      break;

    case KEY_NUM_MQ_LPDR:
      if (msgctl(msqid_lpdr, IPC_STAT, (struct msqid_ds*)&msqds)==-1)
      {
        return -1;
      }
      break;
  }

  return msqds.msg_qnum;
}

int Ipcs::MessageQueueRead(char* data)
{
  switch(mq_key)
  {
    case KEY_NUM_MQ_GRAB:
      {
        //if (msgrcv(msqid_grab, (struct message_grab*)data, sizeof(struct grab_data), 0, 0) == -1)           // blocking 
        if (msgrcv(msqid_grab, (struct message_grab*)data, sizeof(struct grab_data), 0, IPC_NOWAIT) == -1)    // non-blocking
        {
            if (errno != ENOMSG)
            {
                printf("msgrcv failed[KEY_NUM_MQ_GRAB] : %d\n", errno);
                return -1;
            }
            return 1;
        }
      }
      break;

    case KEY_NUM_MQ_LPDR:
      {
        //if (msgrcv(msqid_lpdr, (struct message_lpdr*)data, sizeof(struct lpdr_data), 0, 0) == -1)           // blocking 
        if (msgrcv(msqid_lpdr, (struct message_lpdr*)data, sizeof(struct lpdr_data), 0, IPC_NOWAIT) == -1)    // non-blocking
        {
            if (errno != ENOMSG)
            {
                printf("msgrcv failed[KEY_NUM_MQ_LPDR] : %d\n", errno);
                return -1;
            }
            return 1;
        }
      }
      break;
  }

  return 1;
}

int Ipcs::MessageQueueWrite(char* data)
{
  switch(mq_key)
  {
    case KEY_NUM_MQ_GRAB:
      //if (msgsnd(msqid_grab, (struct lpdr_data*)data, sizeof(struct lpdr_data), 0)==-1)						// blocking
      if (msgsnd(msqid_grab, (struct lpdr_data*)data, sizeof(struct lpdr_data), IPC_NOWAIT)==-1)	  // non blocking
      {
        perror("MessageQueueWrite() : msgsnd failed[KEY_NUM_MQ_GRAB]\n");
        return -1;
      }
      break;

    case KEY_NUM_MQ_LPDR:
      //if (msgsnd(msqid_lpdr, (struct lpdr_data*)data, sizeof(struct lpdr_data), 0)==-1)						// blocking
      if (msgsnd(msqid_lpdr, (struct lpdr_data*)data, sizeof(struct lpdr_data), IPC_NOWAIT)==-1)	  // non blocking
      {
        perror("MessageQueueWrite() : msgsnd failed[KEY_NUM_MQ_LPDR]\n");
        return -1;
      }
      break;
  }

  return 1;
}

int Ipcs::MessageQueueFree()
{
  switch(mq_key)
  {
    case KEY_NUM_MQ_GRAB:
      if(mq_created == true)
      {
        if (msgctl(msqid_grab, IPC_RMID, NULL)==-1)
        {
          perror("MessageQueueFree() : msgctl failed[KEY_NUM_MQ_GRAB]\n");
          return -1;
        }
      }
      break;

    case KEY_NUM_MQ_LPDR:
      if(mq_created == true)
      {
        if (msgctl(msqid_lpdr, IPC_RMID, NULL)==-1)
        {
          perror("MessageQueueFree() : msgctl failed[KEY_NUM_MQ_LPDR]\n");
          return -1;
        }
      }
      break;
  }

  return 1;
}