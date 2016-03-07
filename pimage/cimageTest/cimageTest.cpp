// cimageTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "cimageIO.h"

using namespace cimage;
using namespace cimage::Tools;

int main()
{
	CImage_uint8_t img;

	img = load_image("1.png");

	save_image(img, "save.png");

    return 0;
}

