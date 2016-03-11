#pragma once
#include <windows.h>

namespace LDTools
{

#define R2VA(modulebase, rva)  (((PBYTE)modulebase) + rva) // Relative Virtual Address to Virtual Address conversion.
typedef void (*MessageLogger)(const char*);

class LeakDetector;
extern LeakDetector leakd;

class LeakDetector
{
public:
	void start();
	void stop();

	void setMessageLogger(MessageLogger logger)
	{
		m_plogger = logger;
		m_plogger("setMessageLogger");
	}
	static void logMessage(const char* fmt, ...);
	static void* __cdecl _crtd_scalar_new(unsigned int size);
	static void* __cdecl _crtd_malloc(size_t size);
	static LPVOID __stdcall _HeapAlloc(HANDLE heap, DWORD flags, SIZE_T size);
	static HANDLE   __stdcall _HeapCreate(DWORD options, SIZE_T initsize, SIZE_T maxsize);
private:


	static MessageLogger m_plogger;
};

class LeakDetectorCreater
{
public:
	LeakDetectorCreater(MessageLogger logger=nullptr)
	{
		if (logger)
			leakd.setMessageLogger(logger);
		leakd.start();
	}

	~LeakDetectorCreater()
	{
		leakd.stop();
	}

};

}	// namepace LDTools



