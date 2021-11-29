/*
 *	Typedef.cpp
 *
 * 
 *
 *
 *
 */

#pragma once

#include <syslog.h>
#include <stdio.h>
#include <string>
#include <cstring>
#include <memory>
#include <stdexcept>

using namespace std;

#define LOG_LEVEL_REL     6
#define LOG_LEVEL_DBG     7
#ifndef LOG_LEVEL
#define LOG_LEVEL         LOG_LEVEL_REL
#endif

#define SAFE_DELETE(p)  if(p)   { delete p; p = nullptr; }
//#define INFO_LOG(fmt, ...)       { syslog(LOG_INFO, "[Error : %s][__LINE__ : %d] %s : %s",__FILE__, __LINE__, __FUNCTION__, fmt.c_str()); }
#define EMERG_LOG(fmt, ...)     { syslog(LOG_EMERG,   "[Emergency][%s: %d] %s", __FILE__, __LINE__, fmt.c_str()); }
#define ALERT_LOG(fmt, ...)     { syslog(LOG_ALERT,   "[Alert][%s: %d] %s", __FILE__, __LINE__, fmt.c_str()); }
#define CRIT_LOG(fmt, ...)      { syslog(LOG_CRIT,    "[Critical][%s: %d] %s", __FILE__, __LINE__, fmt.c_str()); }
#define ERR_LOG(fmt, ...)       { syslog(LOG_ERR,     "[Error][%s: %d] %s", __FILE__, __LINE__, fmt.c_str()); }
#define WARN_LOG(fmt, ...)      { syslog(LOG_WARNING, "[Warning][%s: %d] %s", __FILE__, __LINE__, fmt.c_str()); }
#define NOTICE_LOG(fmt, ...)    { syslog(LOG_NOTICE,  "[Notice][%s: %d] %s", __FILE__, __LINE__, fmt.c_str()); }
#define INFO_LOG(fmt, ...)      { syslog(LOG_INFO,    "[Info][%s: %d] %s", __FILE__, __LINE__, fmt.c_str()); }
#if LOG_LEVEL >= LOG_LEVEL_DBG
#define DEBUG_LOG(fmt, ...)     { syslog(LOG_DEBUG,   "[Debug][%s: %d] %s", __FILE__, __LINE__, fmt.c_str()); }
#else
#define DEBUG_LOG(fmt, ...)     { ; }
#endif

template<typename ... Args> 
std::string string_format(const std::string& format, Args ... args) 
{ 
  size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0' 
  if (size <= 0) 
  { 
    throw std::runtime_error("Error during formatting."); 
  } 
  std::unique_ptr<char[]> buf(new char[size]); 
  snprintf(buf.get(), size, format.c_str(), args ...); 
  return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside 
}