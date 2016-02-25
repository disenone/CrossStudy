#include "cimage.h"

extern "C"
{
	char * dllInfo()
	{
		return cimage::getTemplateInfo();
	}

	int dllInt()
	{
		return 1001;
	}
}
