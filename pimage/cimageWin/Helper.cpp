#include "Helper.h"

namespace cimage {
namespace Tools {

static DebugLogFunc DebugLogF = nullptr;

void setDebugLogFunc(DebugLogFunc func)
{
	DebugLogF = func;
}

void Log(const char* log)
{
	if (!DebugLogF)
		return;

	DebugLogF(log);
}

void Log(const std::string& log)
{
	Log(log.data());
}

} // namespace Tools
} // namespace cimage

