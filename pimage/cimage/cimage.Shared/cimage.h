#pragma once

namespace cimage
{
	struct CImageBuffer
	{
		unsigned char* buf;
		int length;
		int width;
	};

	int testImageBuffer(CImageBuffer* pbuf, int len);
}