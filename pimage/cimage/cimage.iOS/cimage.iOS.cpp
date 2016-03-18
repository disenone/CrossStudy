#include "pch.h"
#include "cimage.h"
#include "Helper.h"

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
