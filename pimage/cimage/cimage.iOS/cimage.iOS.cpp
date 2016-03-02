#include "cimage.h"

extern "C"
{
	char * dllInfo()
	{
		return "abc";
	}

	int dllInt()
	{
		return 1003;
	}
}
