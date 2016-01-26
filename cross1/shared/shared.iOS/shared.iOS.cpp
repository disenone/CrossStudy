#include "shared.h"

extern "C"
{
	char * iOSInfo()
	{
		//return shared::getTemplateInfo();
		return "I am ios.";

	}

	int iOSInt()
	{
		return 100;
	}
}