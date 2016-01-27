#include "SharedLibrary1.h"

#define PLATFORM_ANDROID 0
#define PLATFORM_IOS 1

char * SharedLibrary1::getTemplateInfo()
{
#if PLATFORM == PLATFORM_IOS
	static char info[] = "Platform for iOS";
#elif PLATFORM == PLATFORM_ANDROID
	//static char info[] = "Platform for Android";
	static char info[] = "Mon I did it for Android!";
#else
	static char info[] = "Undefined platform";
#endif

	return info;
}

SharedLibrary1::SharedLibrary1()
{
}

SharedLibrary1::~SharedLibrary1()
{
}
