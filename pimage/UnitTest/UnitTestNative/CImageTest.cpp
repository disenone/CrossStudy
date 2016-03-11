#include "stdafx.h"
#include "CppUnitTest.h"
#include "LeakDetector.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace cimage;
using namespace cimage::Tools;
using namespace LDTools;


namespace UnitTestNative
{		
	TEST_CLASS(CImageTest)
	{
	public:
		
		TEST_METHOD(ImageBufferGC)
		{
			LeakDetectorCreater lc(Logger::WriteMessage);
			//leakd.setMessageLogger(Logger::WriteMessage);
			{
				auto img = CImage_uint8_t(1, 1, 1);

				img = CImage_uint8_t(2, 2, 2);
				Logger::WriteMessage("GC Test");
			}

			auto img = CImage_uint8_t(1, 1, 1);
			
			img = CImage_uint8_t(2, 2, 2);
			Logger::WriteMessage("GC Test");
			
		}

		TEST_METHOD(ImageIO)
		{
			CImage_uint8_t img;

			img = load_image("1.png");

			save_image(img, "save.png");

			CImage_uint8_t img2 = load_image("save.png");

			Assert::AreEqual(img.length, img2.length, L"image length not equal as expect");

			Assert::IsTrue(memcmp(img.pbuf, img2.pbuf, img.length) == 0, L"image not equal as expect");
		}

	};
}