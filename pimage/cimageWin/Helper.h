#pragma once

#include <string>

namespace cimage 
{
namespace Tools 
{
	
	typedef void(*DebugLogFunc)(const char* log);

	void setDebugLogFunc(DebugLogFunc);

	void printLog(const char* fmt, ...);

	void printLog(const std::string& log);

}
}