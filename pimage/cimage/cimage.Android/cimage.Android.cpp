#include "pch.h"
#include "cimage.h"
#include "Helper.h"

extern "C"
{
	int dllInt()
	{
		return 1001;
	}

	void setDebugLogFunc(cimage::Tools::DebugLogFunc func)
	{
		cimage::Tools::setDebugLogFunc(func);
	}

	void testImageBuffer(cimage::CImage<uint8_t>* pbuf, int len)
	{
		cimage::testImageBuffer(pbuf, len);
	}
}


