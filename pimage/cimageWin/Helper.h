#pragma once

#include "cimageUtil.h"
#include <string>

namespace cimage 
{
namespace Tools 
{
	
	typedef void(*DebugLogFunc)(const char* log);

	void setDebugLogFunc(DebugLogFunc);

	EXPORT void printLog(const char* fmt, ...);

	EXPORT void printLog(const std::string& log);

	EXPORT void printTrace();
}
}