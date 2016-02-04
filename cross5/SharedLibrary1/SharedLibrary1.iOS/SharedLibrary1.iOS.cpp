#include "SharedLibrary1.h"

extern "C"
{
	char * dllInfo()
	{
		return SharedLibrary1::getTemplateInfo();
	}

	int dllInt()
	{
		return 1003;
	}
}
