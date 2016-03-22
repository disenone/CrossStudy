// cimageTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
//#include <vld.h>
#include "cimageIO.h"
#include <vector>


using namespace cimage;
using namespace cimage::Tools;
using namespace std;

void runMerge()
{
	vector<CImage_uint8_t> imgs;
	char filename[256];
	for (int i = 1; i < 4; ++i)
	{
		sprintf_s(filename, "%d.png", i);
		imgs.push_back(load_image(filename));
	}

	ImageMatchMerge imm(imgs.data(), imgs.size());

	imm.run();

	save_image(imm.result, "res.png");
}

void runIO()
{
	CImage_uint8_t img;

	img = load_image("1.png");
	img = img;

	save_image(img, "save.png");
}

CImage_uint8_t getImg()
{
	return CImage_uint8_t(1, 2, 1);
}

void runGC()
{
	{
		auto img = CImage_uint8_t(1, 1, 1);

		img = CImage_uint8_t(2, 2, 2);
	}

	auto img = CImage_uint8_t(1, 1, 1);

	img = CImage_uint8_t(2, 2, 2);

	CImage_uint8_t img2(getImg());
	printLog("%d", img2.pbuf[0]);
}

void runGCNew()
{
	auto img = CImage_uint8_t(1, 1, 1);

	CImage_uint8_t* pimg = new CImage_uint8_t();

	new((void*)pimg) CImage_uint8_t(std::move(getImg()));

	printLog("%d", pimg->pbuf[0]);

	delete pimg;
}

int main()
{
	//runGCNew();
	runMerge();

	//runGC();

	//runIO();
    return 0;
}

