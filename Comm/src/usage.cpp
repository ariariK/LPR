/*
 *	usage.cpp
 *
 * 
 *
 *
 *
 */
#include "Typedef.h"
#include "usage.h"

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>


Usage::Usage()
{

}

Usage::~Usage()
{

}

char* Usage::os_getline(char *sin, os_line_data * line, char delim)
{
	char *out = sin;
	if (*out == '\0') return NULL;
//	while (*out && (*out == delim)) { out++; }
	line->val = out;
	while (*out && (*out != delim)) { out++; }
	line->len = out - line->val;
//	while (*out && (*out == delim)) { out++; }
	if (*out && (*out == delim)) { out++; }
	if (*out == '\0') return NULL;
	return out;
}

int Usage::ParserMemInfo(char * buffer, int size ,MEM_OCCUPY * lpMemory)
{
  int    state = 0;
	char * p     = buffer;
  while (p)
  {
    os_line_data line = { 0 };
    p = os_getline(p, &line, ':');
    if (p == NULL || line.len <= 0) continue;

    if (line.len == 8 && strncmp(line.val, "MemTotal", 8) == 0)
    {	
      char *point = strtok(p, " ");
      memcpy(lpMemory->name1, "MemTotal", 8);
      lpMemory->name1[8] = '\0';
      lpMemory->MemTotal = atol(point);
    }
    else if(line.len == 7 && strncmp(line.val, "MemFree", 7) == 0)
    {
      char *point = strtok(p, " ");
      memcpy(lpMemory->name2, "MemFree", 7);
      lpMemory->name2[7] = '\0';
      lpMemory->MemFree = atol(point);
    }
    else if(line.len == 12 && strncmp(line.val, "MemAvailable", 12) == 0)
    {
      char *point = strtok(p, " ");
      memcpy(lpMemory->name3, "MemAvailable", 12);
      lpMemory->name3[12] = '\0';
      lpMemory->MemAvailable = atol(point);
    }
    else if(line.len == 7 && strncmp(line.val, "Buffers", 7) == 0)
    {
      char *point = strtok(p, " ");
      memcpy(lpMemory->name4, "Buffers", 7);
      lpMemory->name4[7] = '\0';
      lpMemory->Buffers = atol(point);
    }
    else if(line.len == 6 && strncmp(line.val, "Cached", 6) == 0)
    {
      char *point = strtok(p, " ");
      memcpy(lpMemory->name5, "Cached", 6);
      lpMemory->name5[6] = '\0';
      lpMemory->Cached = atol(point);
    }
  } 	

  return 1;
}

int Usage::Getproceminfo(MEM_OCCUPY * lpMemory)
{
  FILE *fd;
  char buff[128]={0};
  fd = fopen("/proc/meminfo", "r"); 
  if(fd <0) return -1;
  fgets(buff, sizeof(buff), fd); 
  ParserMemInfo(buff,sizeof(buff),lpMemory);
  
  fgets(buff, sizeof(buff), fd);  
  ParserMemInfo(buff,sizeof(buff),lpMemory);
  
  fgets(buff, sizeof(buff), fd);  
  ParserMemInfo(buff,sizeof(buff),lpMemory);
  
  fgets(buff, sizeof(buff), fd);  
  ParserMemInfo(buff,sizeof(buff),lpMemory);

  fgets(buff, sizeof(buff), fd);  
  ParserMemInfo(buff,sizeof(buff),lpMemory);
  
  fclose(fd);

  return 1;
}