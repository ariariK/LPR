/*
 *	Common.cpp
 *
 * 
 *
 *
 *
 */

#include <iostream>
#include <sstream>
#include <cstdarg>

#include "Common.h"

Common::Common()
{
}

Common::~Common()
{
}

bool Common::Init()
{
	return true;
}

std::string Common::string_format(const std::string fmt, ...) 
{
  int size = ((int)fmt.size()) * 2;
  std::string buffer;
  va_list ap;
  while (1) 
  {
    buffer.resize(size);
    va_start(ap, fmt);
    int n = vsnprintf((char*)buffer.data(), size, fmt.c_str(), ap);
    va_end(ap);
    if (n > -1 && n < size) 
    {
      buffer.resize(n);
      return buffer;
    }
    if (n > -1)
    {
      size = n + 1;
    }
    else
    {
      size *= 2;
    }
  }
  return buffer;
}
