#include "Helper.h"
#include <cstdarg>
#include <iostream>

using namespace std;

namespace cimage {
namespace Tools {

static DebugLogFunc DebugLogF = nullptr;

void setDebugLogFunc(DebugLogFunc func)
{
	DebugLogF = func;
}

void printLog(const char* fmt, ...)
{
	char buffer[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	if (!DebugLogF)
	{
		cout << buffer << endl;
		return;
	}

	DebugLogF(buffer);
}

void printLog(const std::string& log)
{
	printLog(log.data());
}

} // namespace Tools
} // namespace cimage

