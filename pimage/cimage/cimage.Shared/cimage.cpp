#include <iostream>
#include "cimage.h"


using namespace cimage;
using namespace std;

#define PLATFORM_ANDROID 0
#define PLATFORM_IOS 1

char * getTemplateInfo()
{
#if PLATFORM == PLATFORM_IOS
	static char info[] = "Platform for iOS";
#elif PLATFORM == PLATFORM_ANDROID
	static char info[] = "Platform for Android";
#else
	static char info[] = "Undefined platform";
#endif

	return info;
}

int cimage::testImageBuffer(CImageBuffer* pbuf, int len)
{
	cout << pbuf << ", " << len << endl;

	CImageBuffer& img = pbuf[0];

	for (int i = 0; i < img.length; i += 4)
	{
		img.buf[i] = 255;
	}

	return pbuf[0].buf[3];
}