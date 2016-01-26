#include "shared.h"

extern "C"
{
	char * AndroidInfo()
	{
		return shared::getTemplateInfo();
	}
}
