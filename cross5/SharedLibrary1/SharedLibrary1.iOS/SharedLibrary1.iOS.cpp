#include "SharedLibrary1.h"

extern "C"
{
	char * iOSInfo()
	{
		return SharedLibrary1::getTemplateInfo();
	}

	int iOSInt()
	{
		return 1002;
	}
}
