#include "stdafx.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace cimage;

namespace UnitTestNative
{		
	TEST_CLASS(CImageTest)
	{
	public:
		
		TEST_METHOD(BufferCollect)
		{
			auto img = CImage_uint8_t(1, 1, 1);
			
			img = CImage_uint8_t(2, 2, 2);
		}

	};
}