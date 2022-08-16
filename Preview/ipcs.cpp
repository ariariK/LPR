/*
 *	ipcs.cpp
 *
 * 
 *
 *
 *
 */
#include "ipcs.h"

#include <errno.h>
#include <sys/time.h>

Ipcs::Ipcs(int key, int size)
{
  created   = false;
  key_num   = key;
  mem_size  = size;
}

Ipcs::~Ipcs()
{

}

int Ipcs::SharedMemoryCreate()
{
  msg = string_format("SharedMemoryCreate() : ID = %d", key_num);
	INFO_LOG(msg);

	if ((shmid = shmget((key_t)key_num, mem_size, IPC_CREAT| IPC_EXCL | 0666)) == -1) 
	{
		INFO_LOG(string("There was shared memory"));

		shmid = shmget((key_t)key_num, mem_size, IPC_CREAT| 0666);
		if(shmid == -1)
		{
			ERR_LOG(string("Shared memory create fail"));

			return -1;
		}
		else
		{
			shmid = shmget((key_t)key_num, mem_size, IPC_CREAT| 0666);
			if(shmid == -1)
			{
				ERR_LOG(string("Shared memory create fail"));

				return -1;
			}
		}
	}

  return 1;
}

int Ipcs::SharedMemoryInit()
{
  msg = string_format("SharedMemoryInit() : ID = %d", key_num);
	INFO_LOG(msg);

  if((shmid = shmget((key_t)key_num, 0, 0)) == -1)
  {
      ERR_LOG(string("Shmid failed"));

      return -1;
  }

  return 1;
}

int Ipcs::SharedMemoryRead(char *sMemory)
{
  void *shmaddr;
  
  if((shmaddr = shmat(shmid, (void *)0, 0)) == (void *)-1)
  {
      ERR_LOG(string("Shmat failed"))
      return -1;
  }

  memcpy(sMemory, (char *)shmaddr, mem_size);
  
  if(shmdt(shmaddr) == -1)
  {
      ERR_LOG(string("Shmdt failed"))
      return -1;
  }
  
  return 1;
}

int Ipcs::SharedMemoryWrite(char *shareddata, int size)
{
  void *shmaddr;
	if(size > mem_size)
	{
		ERR_LOG(string("Shared memory size over"));

		return -1;
	}

	if((shmaddr = shmat(shmid, (void *)0, 0)) == (void *)-1) 
	{
		ERR_LOG(string("Shmat failed"));

		return -1;
	}

	memcpy((char *)shmaddr, shareddata, size);

	if(shmdt(shmaddr) == -1) 
	{
		ERR_LOG(string("Shmdt failed"));
		return -1;
	}

  return 1;
}

int Ipcs::SharedMemoryFree(void)
{
  if(shmctl(shmid, IPC_RMID, 0) == -1) 
  {
    ERR_LOG(string("Shmctl failed"));

    return -1;
  }

  INFO_LOG(string("Shared memory end"));

  return 1;
}

int Ipcs::MessageQueueCreate()
{
  switch(key_num)
  {
    case KEY_NUM_MQ_GRAB:
      msq_grab.msg_type = 1;
      created = true;
      break;

    case KEY_NUM_MQ_LPDR:
      msq_lpdr.msg_type = 1;
      created = true;
      break;

#ifdef EN_LIST_DISP
    // add. by ariari : 2022.05.20 
    case KEY_NUM_MQ_LPDR_INFO:
      msq_lpdr_result.msg_type = 1;
      created = true;
      break;
#endif

  }

  // Get Message Queue ID
  if ((msqid = msgget((key_t)key_num, IPC_CREAT|0666))==-1)
  {
    msg = string_format("msgget(%d) failed", key_num);
    ERR_LOG(msg);

    return -1;
  }

  //fprintf(stderr, "MessageQueueCreate() : msgget(%d) successful\n", key_num);
  msg = string_format("MessageQueueCreate() : msgget(%d) successful\n", key_num);
  INFO_LOG(msg);

  return 1;
}

int Ipcs::MessageQueueInit()
{
  if ((msqid = msgget((key_t)key_num, IPC_CREAT|0666)) == -1)
  {
      msg = string_format("msgget failed[%d]", key_num);
      ERR_LOG(msg);

      //fprintf(stderr, "MessageQueueInit() : msgget(%d) error\n", key_num);

      return -1;
  }

  //fprintf(stderr, "OKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK MessageQueueInit() : msgget(%d) OK\n", key_num);

  return 1;
}

int Ipcs::MessageQueueQNum()
{
  struct msqid_ds msqds;

  if (msgctl(msqid, IPC_STAT, (struct msqid_ds*)&msqds)==-1)
  {
    return -1;
  }

  return msqds.msg_qnum;
}

int Ipcs::MessageQueueRead(char* data, int flag)
{
  // IPC_NOWAIT = 2048

  switch(key_num)
  {
    case KEY_NUM_MQ_GRAB:
      {
        //if (msgrcv(msqid, (struct message_grab*)data, sizeof(struct grab_data), 0, 0) == -1)              // blocking 
        //if (msgrcv(msqid, (struct message_grab*)data, sizeof(struct grab_data), 0, IPC_NOWAIT) == -1)     // non-blocking
        if (msgrcv(msqid, (struct message_grab*)data, sizeof(struct grab_data), 0, flag) == -1)    // non-blocking
        {
          if (errno != ENOMSG)
          {
            msg = string_format("msgrcv failed[%d] : %d", key_num, errno);
            ERR_LOG(msg);

            return -1;
          }
          return 1;
        }
      }
      break;

    case KEY_NUM_MQ_LPDR:
      {
        //if (msgrcv(msqid, (struct message_lpdr*)data, sizeof(struct lpdr_data), 0, 0) == -1)              // blocking 
        //if (msgrcv(msqid, (struct message_lpdr*)data, sizeof(struct lpdr_data), 0, IPC_NOWAIT) == -1)     // non-blocking
        if (msgrcv(msqid, (struct message_lpdr*)data, sizeof(struct lpdr_data), 0, flag) == -1)    // non-blocking
        {
          if (errno != ENOMSG)
          {
            msg = string_format("msgrcv failed[%d] : %d", key_num, errno);
            ERR_LOG(msg);
            return -1;
          }
          return 1;
        }
      }
      break;

#ifdef EN_LIST_DISP
    // add. by ariari : 2022.05.20 
    case KEY_NUM_MQ_LPDR_INFO:
      {
        //if (msgrcv(msqid, (struct message_lpdr*)data, sizeof(struct lpdr_data), 0, 0) == -1)              // blocking 
        //if (msgrcv(msqid, (struct message_lpdr*)data, sizeof(struct lpdr_data), 0, IPC_NOWAIT) == -1)     // non-blocking
        if (msgrcv(msqid, (struct lpdr_result*)data, sizeof(struct message_lpdr_multi), 0, flag) == -1)        // non-blocking
        {
          if (errno != ENOMSG)
          {
            msg = string_format("msgrcv failed[%d] : %d", key_num, errno);
            ERR_LOG(msg);
            return -1;
          }
          return 1;
        }
      }
      break;
#endif      
  }

  return 1;
}

int Ipcs::MessageQueueWrite(char* data)
{
  switch(key_num)
  {
    case KEY_NUM_MQ_GRAB:
      if (msgsnd(msqid, (struct message_grab*)data, sizeof(struct grab_data), 0)==-1)						// blocking
      {
        msg = string_format("msgsnd failed[%d]", key_num);
        ERR_LOG(msg);

        return -1;
      }
      break;

    case KEY_NUM_MQ_LPDR:
      if (msgsnd(msqid, (struct message_lpdr*)data, sizeof(struct lpdr_data), 0)==-1)						// blocking
      {
        msg = string_format("msgsnd failed[%d]", key_num);
        ERR_LOG(msg);

        return -1;
      }
      break;

#ifdef EN_LIST_DISP
    // add. by ariari : 2022.05.20 
    case KEY_NUM_MQ_LPDR_INFO:
      if (msgsnd(msqid, (struct lpdr_result*)data, sizeof(struct message_lpdr_multi), 0)==-1)						// blocking
      {
        msg = string_format("msgsnd failed[%d]", key_num);
        ERR_LOG(msg);

        return -1;
      }
      break;
#endif      
  }

  return 1;
}

int Ipcs::MessageQueueFree()
{
  if(created == true)
  {
    if (msgctl(msqid, IPC_RMID, NULL)==-1)
    {
      msg = string_format("msgctl failed[%d]", key_num);
      ERR_LOG(msg);

      return -1;
    }
  }

  return 1;
}