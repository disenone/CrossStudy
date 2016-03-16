// cimageTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <vld.h>
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

	vector<CImage_uint8_t*> pimgs;
	for (CImage_uint8_t& img: imgs)
	{
		pimgs.push_back(&img);
	}

	ImageMatchMerge imm(pimgs.data(), pimgs.size());

	imm.run();

	save_image(imm.result, "res.png");
}

void runIO()
{
	CImage_uint8_t img;

	img = load_image("1.png");

	save_image(img, "save.png");
}

void runGC()
{
	{
		auto img = CImage_uint8_t(1, 1, 1);

		img = CImage_uint8_t(2, 2, 2);
	}

	auto img = CImage_uint8_t(1, 1, 1);

	img = CImage_uint8_t(2, 2, 2);
}

int main()
{

	//runMerge();

	runGC();

    return 0;
}

