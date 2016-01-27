#include "SharedLibrary1.h"


extern "C"
{
	char * AndroidInfo()
	{
		return SharedLibrary1::getTemplateInfo();
	}

	int AndroidInt()
	{
		return 1002;
	}
}
