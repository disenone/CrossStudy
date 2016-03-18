#include "LeakDetector.h"
#include <windows.h>
#include <cassert>
#include <DbgHelp.h>
#include <iostream>
#include "atlconv.h"
#include <map>


using namespace std;
using namespace LDTools;

typedef void* (__cdecl *malloc_t) (size_t);
typedef void (__cdecl *free_t) (void*);
typedef void (__cdecl *free_dbg_t) (void*, size_t);
typedef void* (__cdecl *new_t) (size_t);
typedef HANDLE(__stdcall *HeapCreate_t) (DWORD, SIZE_T, SIZE_T);

static MessageLogger plogger = nullptr;
static MessageLoggerW ploggerw = nullptr;
bool LeakDetector::m_trace = true;
std::map<void*, HeapContex> LeakDetector::m_heapTrace;
static LeakDetector leakd;

void printTrace(const UINT_PTR* pFrame = nullptr, size_t frameSize = 0);

void logMessage(const char* fmt, ...)
{
	if (plogger == nullptr)
		return;

	char buffer[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);
	plogger(buffer);

}

void logMessage(const WCHAR* fmt, ...)
{
	if (ploggerw == nullptr)
		return;

	WCHAR buffer[10240];
	va_list args;
	va_start(args, fmt);
	_vsnwprintf_s(buffer, sizeof(buffer), fmt, args);
	va_end(args);
	ploggerw(buffer);

}

// start m_trace memory malloc and free
bool LeakDetector::start()
{
	logMessage("==============LeakDetector::start===============");

	HMODULE kernel32 = GetModuleHandle(L"kernel32.dll");
	HMODULE ntdll = GetModuleHandle(L"ntdll.dll");
	//HMODULE msvc = LoadLibrary(L"msvcrt.dll");
	HMODULE msvcrtd = GetModuleHandle(L"msvcrt.dll");
	HMODULE ci = GetModuleHandle(L"cimageWin.dll");
	FARPROC nn = GetProcAddress(msvcrtd, "??2@YAPAXI@Z");
	HMODULE utn = GetModuleHandle(L"UnitTestNative.dll");
	HMODULE cur = GetModuleHandle(NULL);
	//patchImport(ci, "ucrtbased.dll", NULL, "malloc", _malloc);
	//patchImport(ci, "KERNEL32.dll", NULL, "HeapAlloc", _HeapAlloc);
	if (!patchImport(utn, "ucrtbased.dll", NULL, "malloc", _malloc))
		return false;
// 	if (!patchImport(utn, "ucrtbased.dll", NULL, "free", _free))
// 		return false;
	if (!patchImport(utn, "ucrtbased.dll", NULL, "_free_dbg", _crt_free_dbg))
		return false;

	SymInitialize(GetCurrentProcess(), NULL, TRUE);
	return true;
}

// stop m_trace memory and check memory leak,
// if any memory leak detected, print traceback log
bool LeakDetector::stop()
{
	HMODULE utn = GetModuleHandle(L"UnitTestNative.dll");

	unpatchImport(utn, "ucrtbased.dll", NULL, "malloc", _malloc);
	unpatchImport(utn, "ucrtbased.dll", NULL, "free", _free);
	unpatchImport(utn, "ucrtbased.dll", NULL, "_free_dbg", _crt_free_dbg);

	logMessage("==============LeakDetector::stop===============");
	bool ret = true;
	if (m_heapTrace.size() > 0)
	{
		ret = false;
		logMessage("Memory Leak Detected:\n");

		for (const auto heap : m_heapTrace)
		{
			printTrace(heap.second.frames.data(), heap.second.frames.size());
		}
	}

	SymCleanup(GetCurrentProcess());
	m_heapTrace.clear();
	return ret;
}

bool LeakDetector::check()
{
	return m_heapTrace.empty();
}

void* LeakDetector::_crtd_scalar_new(unsigned int size)
{
	static void* pcrtd_scalar_new = nullptr;
    if (pcrtd_scalar_new == nullptr) 
	{
		HMODULE msvcrtd = GetModuleHandle(L"msvcrtd.dll");
        pcrtd_scalar_new = GetProcAddress(msvcrtd, "??2@YAPAXI@Z");
    }

	void* block = ((new_t)pcrtd_scalar_new)(size);

	logMessage("_crtd_scalar_new");

    return block;

}

void* LeakDetector::_malloc(size_t size)
{
	static void* pcrtd_malloc = nullptr;
	if (pcrtd_malloc == nullptr)
	{
		HMODULE ucrtbased = GetModuleHandle(L"ucrtbased.dll");
		pcrtd_malloc = GetProcAddress(ucrtbased, "malloc");
	}

	void* ret = ((malloc_t)pcrtd_malloc)(size);

	// m_heapTrace will malloc memory which will iterate calling this func, 
	// so close m_trace first
	if (!m_trace)
	{
		return ret;
	}

	m_trace = false;

	// capture traceback
	UINT32 maxframes = 62;
	UINT_PTR myFrames[62];
	ZeroMemory(myFrames, sizeof(UINT_PTR) * maxframes);
	ULONG BackTraceHash;
	maxframes = RtlCaptureStackBackTrace(0, maxframes, reinterpret_cast<PVOID*>(myFrames), &BackTraceHash);

	m_heapTrace.emplace(ret, HeapContex{ ret,{ myFrames, myFrames + maxframes } });

	m_trace = true;

	return ret;
}

void LeakDetector::_free(void* prt)
{
	static void* pcrtd_free = nullptr;
	if (pcrtd_free == nullptr)
	{
		HMODULE ucrtbased = GetModuleHandle(L"ucrtbased.dll");
		pcrtd_free = GetProcAddress(ucrtbased, "free");
	}

	m_heapTrace.erase(prt);

	return ((free_t)pcrtd_free)(prt);
}

void LeakDetector::_crt_free_dbg(void* prt, size_t size)
{
	static void* pcrtd_free_dbg = nullptr;
	if (pcrtd_free_dbg == nullptr)
	{
		HMODULE ucrtbased = GetModuleHandle(L"ucrtbased.dll");
		pcrtd_free_dbg = GetProcAddress(ucrtbased, "_free_dbg");
	}

	m_heapTrace.erase(prt);

	return ((free_dbg_t)pcrtd_free_dbg)(prt, size);
}

LPVOID LeakDetector::_HeapAlloc(HANDLE heap, DWORD flags, SIZE_T size)
{
	static void* pHeapAlloc = nullptr;
	if (pHeapAlloc == nullptr)
	{
		HMODULE kernal32 = GetModuleHandle(L"kernel32.dll");
		pHeapAlloc = GetProcAddress(kernal32, "HeapAlloc");
	}

	// Allocate the block.
	LPVOID block = HeapAlloc(heap, flags, size);
	logMessage("_HeapAlloc");
	return block;
}

HANDLE LeakDetector::_HeapCreate(DWORD options, SIZE_T initsize, SIZE_T maxsize)
{
	static void* pHeapCreate = nullptr;
	if (pHeapCreate == nullptr)
	{
		HMODULE kernal32 = GetModuleHandle(L"kernel32.dll");
		pHeapCreate = GetProcAddress(kernal32, "HeapCreate");
	}

	// Create the heap.
	HANDLE heap = ((HeapCreate_t)pHeapCreate)(options, initsize, maxsize);

	logMessage("_HeapCreate");
	return heap;
}

// patch specific function of specific module by another function patching is done
// by replacing the function address in importmodule's Import Address Table (IAT)
//
// - importmudule (IN): the module which import the function of export mudule, will 
//		replace the IAT of this module
//
// - exportmodulename (IN): the name of export mudule, which located the implement 
//		of the replacing function
//
// - exportmodulepath (IN): the path of export module. export mudule will first load 
//		by exportmodulepath,	if failed, then load by exportmodulename 
//
// - importname (IN): function name
//
// - replacement (IN): patch function address
// 
// Return Value: true if success, else false
bool LeakDetector::patchImport(
	HMODULE importmodule, 
	LPCSTR exportmodulename, 
	LPCSTR exportmodulepath, 
	LPCSTR importname,
	LPCVOID replacement)
{
	HMODULE                  exportmodule;
	IMAGE_THUNK_DATA        *iate;
	IMAGE_IMPORT_DESCRIPTOR *idte;
	FARPROC                  import;
	DWORD                    protect;
	IMAGE_SECTION_HEADER    *section;
	ULONG                    size;

	assert(exportmodulename != NULL);

	idte = (IMAGE_IMPORT_DESCRIPTOR*)ImageDirectoryEntryToDataEx((PVOID)importmodule, TRUE,
		IMAGE_DIRECTORY_ENTRY_IMPORT, &size, &section);
	if (idte == NULL) {
		logMessage("patchImport failed: idte == NULL\n");
		return false;
	}
	while (idte->OriginalFirstThunk != 0x0) {
		//logMessage((PCHAR)R2VA(importmodule, idte->Name));
		if (strcmp((PCHAR)R2VA(importmodule, idte->Name), exportmodulename) == 0) {
			break;
		}
		idte++;
	}
	if (idte->OriginalFirstThunk == 0x0) {
		logMessage("patchImport failed: idte->OriginalFirstThunk == 0x0\n");
		return false;
	}

	if (exportmodulepath != NULL) {
		exportmodule = GetModuleHandleA(exportmodulepath);
	}
	else {
		exportmodule = GetModuleHandleA(exportmodulename);
	}
	assert(exportmodule != NULL);
	import = GetProcAddress(exportmodule, importname);
	assert(import != NULL);

	iate = (IMAGE_THUNK_DATA*)R2VA(importmodule, idte->FirstThunk);
	while (iate->u1.Function != 0x0) {
		if (iate->u1.Function == (DWORD_PTR)import) {
			VirtualProtect(&iate->u1.Function, sizeof(iate->u1.Function), PAGE_READWRITE, &protect);
			iate->u1.Function = (DWORD_PTR)replacement;
			VirtualProtect(&iate->u1.Function, sizeof(iate->u1.Function), protect, &protect);
			//logMessage("patchImport success: %s, %s", exportmodulename, importname);
			return true;
		}
		iate++;
	}

	return false;
}

// recover function address of importmodule's Import Address Table (IAT)
//
// - importmudule (IN): the module whose IAT is patched 
//
// - exportmodulename (IN): the name of export mudule, which located the implement 
//		of the replacing function
//
// - exportmodulepath (IN): the path of export module. export mudule will first load 
//		by exportmodulepath,	if failed, then load by exportmodulename 
//
// - importname (IN): function name
//
// - replacement (IN): patched function address
// 
// Return Value: true if success, else false
bool LeakDetector::unpatchImport(
	HMODULE importmodule, 
	LPCSTR exportmodulename, 
	LPCSTR exportmodulepath, 
	LPCSTR importname,
	LPCVOID replacement)
{
	HMODULE                  exportmodule;
	IMAGE_THUNK_DATA        *iate;
	IMAGE_IMPORT_DESCRIPTOR *idte;
	FARPROC                  import;
	DWORD                    protect;
	IMAGE_SECTION_HEADER    *section;
	ULONG                    size;

	assert(exportmodulename != NULL);

	idte = (IMAGE_IMPORT_DESCRIPTOR*)ImageDirectoryEntryToDataEx((PVOID)importmodule, TRUE,
		IMAGE_DIRECTORY_ENTRY_IMPORT, &size, &section);
	if (idte == NULL) {
		logMessage("unpatchImport failed: idte == NULL\n");
		return false;
	}
	while (idte->OriginalFirstThunk != 0x0) {
		//logMessage((PCHAR)R2VA(importmodule, idte->Name));
		if (strcmp((PCHAR)R2VA(importmodule, idte->Name), exportmodulename) == 0) {
			break;
		}
		idte++;
	}
	if (idte->OriginalFirstThunk == 0x0) {
		logMessage("unpatchImport failed: idte->OriginalFirstThunk == 0x0\n");
		return false;
	}

	if (exportmodulepath != NULL) {
		exportmodule = GetModuleHandleA(exportmodulepath);
	}
	else {
		exportmodule = GetModuleHandleA(exportmodulename);
	}
	assert(exportmodule != NULL);
	import = GetProcAddress(exportmodule, importname);
	assert(import != NULL);

	iate = (IMAGE_THUNK_DATA*)R2VA(importmodule, idte->FirstThunk);
	while (iate->u1.Function != 0x0) {
		if (iate->u1.Function == (DWORD_PTR)replacement) {
			VirtualProtect(&iate->u1.Function, sizeof(iate->u1.Function), PAGE_READWRITE, &protect);
			iate->u1.Function = (DWORD_PTR)import;
			VirtualProtect(&iate->u1.Function, sizeof(iate->u1.Function), protect, &protect);
			//logMessage("unpatchImport success: %s, %s", exportmodulename, importname);
			return true;
		}
		iate++;
	}

	return false;
}

LeakDetect::LeakDetect(MessageLogger logger/* =nullptr */, 
	MessageLoggerW loggerw/* = nullptr*/)
{
	if (logger)
	{
		plogger = logger;
	}
	if (loggerw)
	{
		ploggerw = loggerw;
	}
	m_initOk = leakd.start();
}

LeakDetect::~LeakDetect()
{
	leakd.stop();
}

bool LeakDetect::check()
{
	return leakd.check();
}

bool LeakDetect::isInitOk()
{
	return m_initOk;
}

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
	logMessage(buf);
}

LPCWSTR getFunctionName(SIZE_T programCounter, DWORD64& displacement64,
	SYMBOL_INFOW* functionInfo)
{
	// Initialize structures passed to the symbol handler.
	functionInfo->SizeOfStruct = sizeof(SYMBOL_INFOW);
	functionInfo->MaxNameLen = 256;

	// Try to get the name of the function containing this program
	// counter address.
	displacement64 = 0;
	LPCWSTR functionName;
	HANDLE hProcess = GetCurrentProcess();
	if (SymFromAddrW(hProcess, programCounter, &displacement64, functionInfo)) {
		functionName = functionInfo->Name;
	}
	else {
		printLastError();
		// GetFormattedMessage( GetLastError() );
		swprintf_s(functionInfo->Name, (size_t)256, L"0x%x", programCounter);
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

DWORD resolveFunction(SIZE_T programCounter, IMAGEHLP_LINEW64* sourceInfo, DWORD displacement,
	LPCWSTR functionName, LPWSTR stack_line, DWORD stackLineSize)
{
	WCHAR callingModuleName[260];
	HMODULE hCallingModule = GetCallingModule(programCounter);
	LPWSTR moduleName = L"(Module name unavailable)";
	if (hCallingModule &&
		GetModuleFileName(hCallingModule, callingModuleName, _countof(callingModuleName)) > 0)
	{
		moduleName = wcsrchr(callingModuleName, L'\\');
		if (moduleName == NULL)
			moduleName = wcsrchr(callingModuleName, L'/');
		if (moduleName != NULL)
			moduleName++;
		else
			moduleName = callingModuleName;
	}
	ZeroMemory(stack_line, stackLineSize * sizeof(WCHAR));
	// Display the current stack frame's information.
	if (sourceInfo)
	{
		if (displacement == 0)
		{
			swprintf_s(stack_line, stackLineSize, L"    %s (%d): %s!%s()\n",
				sourceInfo->FileName, sourceInfo->LineNumber, moduleName,
				functionName);
		}
		else
		{
			swprintf_s(stack_line, stackLineSize, L"    %s (%d): %s!%s() + 0x%x bytes\n",
				sourceInfo->FileName, sourceInfo->LineNumber, moduleName,
				functionName, displacement);
		}
	}
	else
	{
		if (displacement == 0)
		{
			swprintf_s(stack_line, stackLineSize, L"    %s!%s()\n",
				moduleName, functionName);
		}
		else
		{
			swprintf_s(stack_line, stackLineSize, L"    %s!%s() + 0x%x bytes\n",
				moduleName, functionName, displacement);
		}
	}

	LPWSTR end = find(stack_line, stack_line + stackLineSize, L'\0');
	DWORD NumChars = (DWORD)(end - stack_line);
	stack_line[NumChars] = L'\0';
	return NumChars;
}

void printTrace(const UINT_PTR* pFrame/* = nullptr*/, size_t frameSize/* = 0*/)
{

	UINT32 maxframes = 62;
	UINT_PTR myFrames[62];
	if (pFrame == 0)
	{
		ZeroMemory(myFrames, sizeof(UINT_PTR) * maxframes);
		ULONG BackTraceHash;
		maxframes = RtlCaptureStackBackTrace(0, maxframes, reinterpret_cast<PVOID*>(myFrames), &BackTraceHash);
		pFrame = myFrames;
		frameSize = maxframes;
	}

	UINT32  startIndex = 0;

	int unresolvedFunctionsCount = 0;
	IMAGEHLP_LINEW64  sourceInfo = { 0 };
	sourceInfo.SizeOfStruct = sizeof(IMAGEHLP_LINEW64);

	// Use static here to increase performance, and avoid heap allocs.
	// It's thread safe because of g_heapMapLock lock.
	static WCHAR stack_line[1024] = L"";
	bool isPrevFrameInternal = false;
	DWORD NumChars = 0;

	const size_t max_line_length = 512;
	const int resolvedCapacity = 62 * max_line_length;
	const size_t allocedBytes = resolvedCapacity * sizeof(WCHAR);
	WCHAR resolved[resolvedCapacity];
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
		BYTE symbolBuffer[sizeof(SYMBOL_INFOW) + 256 * sizeof(WCHAR)];
		LPCWSTR functionName = getFunctionName(programCounter, displacement64, (SYMBOL_INFOW*)&symbolBuffer);

		// It turns out that calls to SymGetLineFromAddrW64 may free the very memory we are scrutinizing here
		// in this method. If this is the case, m_Resolved will be null after SymGetLineFromAddrW64 returns.
		// When that happens there is nothing we can do except crash.
		DWORD            displacement = 0;

		BOOL foundline = SymGetLineFromAddrW64(hProcess, programCounter, &displacement, &sourceInfo);

		bool isFrameInternal = false;

		// show one allocation function for context
		if (NumChars > 0 && !isFrameInternal && isPrevFrameInternal) {
			resolvedLength += NumChars;
			if (resolved) {
				wcsncat_s(resolved, resolvedCapacity, stack_line, NumChars);
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
				wcsncat_s(resolved, resolvedCapacity, stack_line, NumChars);
			}
		}
	} // end for loop
	logMessage(resolved);

	return;
}