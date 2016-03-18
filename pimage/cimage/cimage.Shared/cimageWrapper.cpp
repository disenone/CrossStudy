#include "pch.h"
#include "cimage.h"
#include "Helper.h"

extern "C"
{

	void setDebugLogFunc(cimage::Tools::DebugLogFunc func)
	{
		cimage::Tools::setDebugLogFunc(func);
	}

	int runImageMerge(cimage::CImage_uint8_t* pbuf, int len, cimage::CImage_uint8_t* pout)
	{
		return cimage::runImageMerge(pbuf, len, pout);
	}

	int clearImageMerge()
	{
		return cimage::clearImageMerge();
	}

}