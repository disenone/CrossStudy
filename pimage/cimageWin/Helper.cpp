#include "Helper.h"
#include <cstdarg>
#include <iostream>

#ifdef __APPLE__
#include <execinfo.h>
#include <signal.h>
#elif defined(WIN32)
#include <windows.h>
#include <DbgHelp.h>
#endif


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
	char buffer[2048];
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
	if (!DebugLogF)
	{
		cout << log << endl;
		return;
	}

	DebugLogF(log.c_str());
}

#ifdef __APPLE__
void printTrace()
{
	void * array[50];
	int size = backtrace(array, 50);

	std::cerr << __FUNCTION__ << " backtrace returned "
		<< size << " frames\n\n";

	char ** messages = backtrace_symbols(array, size);

	for (int i = 0; i < size && messages != NULL; ++i) {
		std::cerr << "[bt]: (" << i << ") " << messages[i] << std::endl;
	}
	std::cerr << std::endl;

	free(messages);

}

#elif defined(WIN32)

static CHAR *                      //   return error message
getLastErrorText(                  // converts "Lasr Error" code into text
	CHAR *pBuf,                        //   message buffer
	ULONG bufSize)                     //   buffer size
{
	DWORD retSize;
	CHAR* pTemp = NULL;

	if (bufSize < 16) {
		if (bufSize > 0) {
			pBuf[0] = '\0';
		}
		return(pBuf);
	}
	retSize = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_ARGUMENT_ARRAY,
		NULL,
		GetLastError(),
		LANG_NEUTRAL,
		(LPSTR)&pTemp,
		0,
		NULL);
	if (!retSize || pTemp == NULL) {
		pBuf[0] = '\0';
	}
	else {
		pTemp[strlen(pTemp) - 2] = '\0'; //remove cr and newline character
		sprintf_s(pBuf, bufSize, "%0.*s (0x%x)", bufSize - 16, pTemp, GetLastError());
		LocalFree((HLOCAL)pTemp);
	}
	return(pBuf);
}

void printLastError()
{
	char buf[1024];
	getLastErrorText(buf, 1024);
	printLog(buf);
}

LPCSTR getFunctionName(SIZE_T programCounter, DWORD64& displacement64,
	SYMBOL_INFO* functionInfo)
{
	// Initialize structures passed to the symbol handler.
	functionInfo->SizeOfStruct = sizeof(SYMBOL_INFO);
	functionInfo->MaxNameLen = 256;

	// Try to get the name of the function containing this program
	// counter address.
	displacement64 = 0;
	LPCSTR functionName;
	HANDLE hProcess = GetCurrentProcess();
	if (SymFromAddr(hProcess, programCounter, &displacement64, functionInfo)) {
		functionName = functionInfo->Name;
	}
	else {
		printLastError();
		// GetFormattedMessage( GetLastError() );
		sprintf_s(functionInfo->Name, (size_t)256, "0x%x", programCounter);
		functionName = functionInfo->Name;
		displacement64 = 0;
	}
	return functionName;
}

HMODULE GetCallingModule(UINT_PTR pCaller)
{
	HMODULE hModule = NULL;
	MEMORY_BASIC_INFORMATION mbi;
	if (VirtualQuery((LPCVOID)pCaller, &mbi, sizeof(MEMORY_BASIC_INFORMATION)) == sizeof(MEMORY_BASIC_INFORMATION))
	{
		// the allocation base is the beginning of a PE file
		hModule = (HMODULE)mbi.AllocationBase;
	}
	return hModule;
}

DWORD resolveFunction(SIZE_T programCounter, IMAGEHLP_LINE* sourceInfo, DWORD displacement,
	LPCSTR functionName, LPSTR stack_line, DWORD stackLineSize)
{
	char callingModuleName[260];
	HMODULE hCallingModule = GetCallingModule(programCounter);
	LPSTR moduleName = "(Module name unavailable)";
	if (hCallingModule &&
		GetModuleFileNameA(hCallingModule, callingModuleName, _countof(callingModuleName)) > 0)
	{

		moduleName = strrchr(callingModuleName, '\\');
		if (moduleName == NULL)
			moduleName = strrchr(callingModuleName, '/');
		if (moduleName != NULL)
			moduleName++;
		else
			moduleName = callingModuleName;
	}
	ZeroMemory(stack_line, stackLineSize * sizeof(char));
	// Display the current stack frame's information.
	if (sourceInfo)
	{
		if (displacement == 0)
		{
			sprintf_s(stack_line, stackLineSize, "    %s (%d): %s!%s()\n",
				sourceInfo->FileName, sourceInfo->LineNumber, moduleName,
				functionName);
		}
		else
		{
			sprintf_s(stack_line, stackLineSize, "    %s (%d): %s!%s() + 0x%x bytes\n",
				sourceInfo->FileName, sourceInfo->LineNumber, moduleName,
				functionName, displacement);
		}
	}
	else
	{
		if (displacement == 0)
		{
			sprintf_s(stack_line, stackLineSize, "    %s!%s()\n",
				moduleName, functionName);
		}
		else
		{
			sprintf_s(stack_line, stackLineSize, "    %s!%s() + 0x%x bytes\n",
				moduleName, functionName, displacement);
		}
	}

	LPSTR end = find(stack_line, stack_line + stackLineSize, '\0');
	DWORD NumChars = (DWORD)(end - stack_line);
	stack_line[NumChars] = '\0';
	return NumChars;
}

void printTrace()
{
	SymInitialize(GetCurrentProcess(), NULL, TRUE);

	UINT32 maxframes = 62;
	UINT_PTR myFrames[62];

	ZeroMemory(myFrames, sizeof(UINT_PTR) * maxframes);
	ULONG BackTraceHash;
	maxframes = CaptureStackBackTrace(0, maxframes, reinterpret_cast<PVOID*>(myFrames), &BackTraceHash);

	const UINT_PTR* pFrame = myFrames;
	const size_t frameSize = maxframes;

	UINT32  startIndex = 0;

	int unresolvedFunctionsCount = 0;
	IMAGEHLP_LINE  sourceInfo = { 0 };
	sourceInfo.SizeOfStruct = sizeof(IMAGEHLP_LINE);

	// Use static here to increase performance, and avoid heap allocs.
	// It's thread safe because of g_heapMapLock lock.
	static char stack_line[1024] = "";
	bool isPrevFrameInternal = false;
	DWORD NumChars = 0;

	const size_t max_line_length = 512;
	const int resolvedCapacity = 62 * max_line_length;
	const size_t allocedBytes = resolvedCapacity * sizeof(char);
	char resolved[resolvedCapacity];
	if (resolved) {
		ZeroMemory(resolved, allocedBytes);
	}
	HANDLE hProcess = GetCurrentProcess();
	int resolvedLength = 0;
	// Iterate through each frame in the call stack.
	for (UINT32 frame = 0; frame < frameSize; frame++)
	{
		if (pFrame[frame] == 0)
			break;
		// Try to get the source file and line number associated with
		// this program counter address.
		SIZE_T programCounter = pFrame[frame];

		DWORD64 displacement64;
		BYTE symbolBuffer[sizeof(SYMBOL_INFO) + 256 * sizeof(char)];
		LPCSTR functionName = getFunctionName(programCounter, displacement64, (SYMBOL_INFO*)&symbolBuffer);

		// It turns out that calls to SymGetLineFromAddrW64 may free the very memory we are scrutinizing here
		// in this method. If this is the case, m_Resolved will be null after SymGetLineFromAddrW64 returns.
		// When that happens there is nothing we can do except crash.
		DWORD            displacement = 0;

		BOOL foundline = SymGetLineFromAddr(hProcess, programCounter, &displacement, &sourceInfo);

		bool isFrameInternal = false;

		// show one allocation function for context
		if (NumChars > 0 && !isFrameInternal && isPrevFrameInternal) {
			resolvedLength += NumChars;
			if (resolved) {
				strncat_s(resolved, resolvedCapacity, stack_line, NumChars);
			}
		}
		isPrevFrameInternal = isFrameInternal;

		if (!foundline)
			displacement = (DWORD)displacement64;
		NumChars = resolveFunction(programCounter, foundline ? &sourceInfo : NULL,
			displacement, functionName, stack_line, _countof(stack_line));

		if (NumChars > 0 && !isFrameInternal) {
			resolvedLength += NumChars;
			if (resolved) {
				strncat_s(resolved, resolvedCapacity, stack_line, NumChars);
			}
		}
	} // end for loop
	printLog(resolved);

	SymCleanup(GetCurrentProcess());
	return;
}
#else
void printTrace()
{}
#endif

} // namespace Tools
} // namespace cimage

