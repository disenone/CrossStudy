#include "LeakDetector.h"
#include <windows.h>
#include <cassert>
#include <DbgHelp.h>
#include <iostream>
#include "atlconv.h"

using namespace std;
using namespace LDTools;

namespace LDTools {
	LeakDetector leakd;
}

typedef void* (__cdecl *malloc_t) (size_t);
typedef void* (__cdecl *new_t) (size_t);
typedef HANDLE(__stdcall *HeapCreate_t) (DWORD, SIZE_T, SIZE_T);

static void* pcrtd_scalar_new = nullptr;
static void* pcrtd_malloc = nullptr;
static void* pHeapAlloc = nullptr;
static void* pHeapCreate = nullptr;
//static LeakDetectorCreater lc;

bool patchImport(HMODULE importmodule, LPCSTR exportmodulename, LPCSTR exportmodulepath, LPCSTR importname,
	LPCVOID replacement);

MessageLogger LeakDetector::m_plogger = nullptr;

void LeakDetector::logMessage(const char* fmt, ...)
{
	if (m_plogger == nullptr)
		return;

	char buffer[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);
	m_plogger(buffer);
}

void printDllInfo(const char* dllname)
{
	USES_CONVERSION;
	HMODULE d = GetModuleHandle(A2W(dllname));
	LeakDetector::logMessage("=========%s========", dllname);
	bool ret = patchImport(d, "UnitTestNative.dll", NULL, "??2@YAPAXI@Z", LeakDetector::_crtd_scalar_new);
}

void LeakDetector::start()
{
	logMessage("LeakDetector::start");

	HMODULE kernel32 = GetModuleHandle(L"kernel32.dll");
	HMODULE ntdll = GetModuleHandle(L"ntdll.dll");
	//HMODULE msvc = LoadLibrary(L"msvcrt.dll");
	HMODULE msvcrtd = GetModuleHandle(L"msvcrt.dll");
	HMODULE ci = GetModuleHandle(L"cimageWin.dll");
	FARPROC nn = GetProcAddress(msvcrtd, "??2@YAPAXI@Z");
	HMODULE utn = GetModuleHandle(L"UnitTestNative.dll");

	char dllnames[][256] = {
		"kernel32.dll",
		"ntdll.dll",
		"msvcrt.dll",
		"cimageWin.dll",
		"UnitTestNative.dll"
	};

	for (char* pdll : dllnames)
	{
		//printDllInfo(pdll);
	}

	//patchImport(ci, "ucrtbased.dll", NULL, "malloc", _crtd_malloc);
	//patchImport(ci, "KERNEL32.dll", NULL, "HeapAlloc", _HeapAlloc);
	patchImport(utn, "ucrtbased.dll", NULL, "malloc", _crtd_malloc);
	//logMessage("patchImport return %d\n", ret);
}

void LeakDetector::stop()
{
	logMessage("LeakDetector::stop");
}

void* LeakDetector::_crtd_scalar_new(unsigned int size)
{
    if (pcrtd_scalar_new == nullptr) 
	{
		HMODULE msvcrtd = GetModuleHandle(L"msvcrtd.dll");
        pcrtd_scalar_new = GetProcAddress(msvcrtd, "??2@YAPAXI@Z");
    }

	void* block = ((new_t)pcrtd_scalar_new)(size);

	logMessage("_crtd_scalar_new");

    return block;

}

void* LeakDetector::_crtd_malloc(size_t size)
{
	if (pcrtd_malloc == nullptr)
	{
		HMODULE ucrtbased = GetModuleHandle(L"ucrtbased.dll");
		pcrtd_malloc = GetProcAddress(ucrtbased, "malloc");
	}

	logMessage("_crtd_malloc");

	return ((malloc_t)pcrtd_malloc)(size);
}

LPVOID LeakDetector::_HeapAlloc(HANDLE heap, DWORD flags, SIZE_T size)
{
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
	if (pHeapAlloc == nullptr)
	{
		HMODULE kernal32 = GetModuleHandle(L"kernel32.dll");
		pHeapCreate = GetProcAddress(kernal32, "HeapCreate");
	}

	// Create the heap.
	HANDLE heap = ((HeapCreate_t)pHeapCreate)(options, initsize, maxsize);

	logMessage("_HeapCreate");
	return heap;
}

bool patchImport(HMODULE importmodule, LPCSTR exportmodulename, LPCSTR exportmodulepath, LPCSTR importname,
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
	LeakDetector::logMessage(exportmodulename);
	idte = (IMAGE_IMPORT_DESCRIPTOR*)ImageDirectoryEntryToDataEx((PVOID)importmodule, TRUE,
		IMAGE_DIRECTORY_ENTRY_IMPORT, &size, &section);
	if (idte == NULL) {
		LeakDetector::logMessage("patchImport idte == NULL\n");
		return false;
	}
	while (idte->OriginalFirstThunk != 0x0) {
		//LeakDetector::logMessage((PCHAR)R2VA(importmodule, idte->Name));
		if (strcmp((PCHAR)R2VA(importmodule, idte->Name), exportmodulename) == 0) {
			break;
		}
		idte++;
	}
	if (idte->OriginalFirstThunk == 0x0) {
		LeakDetector::logMessage("patchImport idte->OriginalFirstThunk == 0x0\n");
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
			LeakDetector::logMessage("patchImport success");
			return true;
		}
		iate++;
	}

	return false;
}