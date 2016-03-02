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

	void testImageBuffer(cimage::CImageBuffer<unsigned char>* pbuf, int len)
	{
		cimage::testImageBuffer(pbuf, len);
	}
}


