#pragma once
#include <windows.h>
#include <vector>
#include <map>

namespace LDTools
{

#define R2VA(modulebase, rva)  (((PBYTE)modulebase) + rva) // Relative Virtual Address to Virtual Address conversion.
typedef void (*MessageLogger)(const char*);
typedef void (*MessageLoggerW)(const WCHAR*);

struct HeapContex
{
	void* pHeap;
	std::vector<UINT_PTR> frames;
};

class LeakDetector
{
public:
	static bool start();
	static bool stop();
	static bool check();
private:
	static void* __cdecl _crtd_scalar_new(unsigned int size);
	static void* __cdecl _malloc(size_t size);
	static LPVOID __stdcall _HeapAlloc(HANDLE heap, DWORD flags, SIZE_T size);
	static HANDLE   __stdcall _HeapCreate(DWORD options, SIZE_T initsize, SIZE_T maxsize);
	static void __cdecl _free(void* prt);
	static void __cdecl _crt_free_dbg(void* prt, size_t size);

	static bool patchImport(
		HMODULE importmodule, 
		LPCSTR exportmodulename, 
		LPCSTR exportmodulepath, 
		LPCSTR importname,
		LPCVOID replacement);

	static bool unpatchImport(
		HMODULE importmodule, 
		LPCSTR exportmodulename, 
		LPCSTR exportmodulepath, 
		LPCSTR importname,
		LPCVOID replacement);

	static bool m_trace;
	static std::map<void*, HeapContex> m_heapTrace;
};

class LeakDetect
{
public:
	LeakDetect(MessageLogger logger = nullptr, MessageLoggerW loggerw = nullptr);

	~LeakDetect();

	bool check();

	bool isInitOk();

private:
	bool m_initOk = false;
};

}	// namepace LDTools



