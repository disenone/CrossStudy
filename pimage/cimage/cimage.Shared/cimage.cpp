#include <iostream>
#include <string>
#include "cimage.h"
#include "Helper.h"

using namespace std;
using namespace cimage;
using namespace cimage::Tools;


int cimage::testImageBuffer(CImageBuffer<uint8_t>* pbuf, int len)
{
	Log("test log in native code.");

	cout << "test log in native code 2." << endl;

	CImageBuffer<uint8_t>& img = pbuf[0];

	for (int i = 0; i < img.length; i += 4)
	{
		img.pbuf[i] = 255;
	}

	return pbuf[0].pbuf[3];
}