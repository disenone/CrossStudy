#pragma once

#include <string>

namespace cimage 
{
namespace Tools 
{
	
	typedef void(*DebugLogFunc)(const char* log);

	void setDebugLogFunc(DebugLogFunc);

	void Log(const char* log);

	void Log(const std::string& log);

}
}