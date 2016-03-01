#include "cimage.h"

extern "C"
{
	char * dllInfo()
	{
		return "abc";
	}

	int dllInt()
	{
		return 1001;
	}

	void testImageBuffer(cimage::CImageBuffer* pbuf, int len)
	{
		cimage::testImageBuffer(pbuf, len);
	}
}


