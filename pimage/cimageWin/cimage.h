#pragma once

#include "cimageUtil.h"
#include "Helper.h"
#include <string>
#include <vector>
#include <tuple>


namespace cimage
{

template<typename T>
struct CImage
{

	T* pbuf = nullptr;
	uint32_t length = 0;
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t channel = 0;
	uint32_t selfgc = 0;

	CImage() = default;

	CImage(int w, int h, int c) :
		width(w), height(h), channel(c)
	{
		create();
	}

	~CImage()
	{
		clear();
	}

	CImage(const CImage& img)
	{
		memcpy(this, &img, sizeof(CImage));
		create();
		memcpy(pbuf, img.pbuf, byteLength());
	}

	CImage(CImage&& img)
	{
		memcpy(this, &img, sizeof(CImage));
		img.selfgc = 0;
	}

	CImage& operator=(const CImage& img)
	{
		if (this == &img)
			return *this;
		clear();
		memcpy(this, &img, sizeof(CImage));
		create();
		memcpy(pbuf, img.pbuf, byteLength());
		return *this;
	}

	CImage& operator=(CImage&& img)
	{
		clear();
		memcpy(this, &img, sizeof(CImage));
		img.selfgc = 0;
		return *this;
	}

	CImage makeTempCopy() const
	{
		CImage img;
		memcpy(&img, this, sizeof(CImage));
		img.selfgc = 0;
		return img;
	}

	void clear()
	{
		if (selfgc && pbuf)
		{
			//Tools::printLog("CImage clear: %s", this->toString().c_str());
			//Tools::printTrace();
			delete pbuf;
			pbuf = nullptr;
		}
		memset(this, 0, sizeof(CImage));
	}

	void create()
	{
		length = width * height * channel;
		if (length > 0)
		{
			pbuf = new T[length];
			selfgc = 1;
			memset(pbuf, 0, byteLength());
			//Tools::printLog("CImage create: %s", this->toString().c_str());
		}
	}

	T& operator()(uint32_t x, uint32_t y, uint32_t c)
	{
		if (c >= channel || x >= width || y >= height)
			throw std::out_of_range("CImageBuffer out of range");
		return pbuf[width * channel * y + channel * x + c];
	}

	T& operator()(uint32_t x, uint32_t y)
	{
		if (x >= width || y >= height)
			throw std::out_of_range("CImageBuffer out of range");
		return pbuf[width * channel * y + channel * x];
	}

	inline int stride() const
	{
		return width * channel;
	}

	T* end() const
	{
		return pbuf + length;
	}

	inline int byteLength() const
	{
		return length * sizeof(T);
	}

	std::string toString() const
	{
		char buf[512];
		sprintf(buf, "0x%x: width = %d, height = %d, channel = %d, pbuf = 0x%x, selfgc = %d", (int)this,
			width, height, channel, (int)pbuf, selfgc);

		return std::string(buf);
	}
};

typedef CImage<uint8_t> CImage_uint8_t;
typedef CImage<uint16_t> CImage_uint16_t;
typedef CImage<uint32_t> CImage_uint32_t;

class ImageMatchMerge
{
public:
	EXPORT ImageMatchMerge(CImage_uint8_t* pbuf = nullptr, int len = 0);

	EXPORT void setInput(CImage_uint8_t* pbuf, int len);

	EXPORT bool run();

	EXPORT void clear();

	CImage_uint8_t result;

private:

	CImage_uint32_t sumImageRow(const CImage_uint8_t& input);

	CImage_uint32_t sumImageRowBlock(const CImage_uint8_t& input, int block_width);

	std::tuple<int, int> findHeadAndTail(const CImage_uint32_t& sum1,
		const CImage_uint32_t& sum2);

	template<typename T>
	CImage<T> cutHeadAndTail(const CImage<T>& input, int headLen, int tailLen);

	template<typename T>
	int avgMatchImages(const CImage<T>& top, const CImage<T>& down);

	std::vector<CImage_uint8_t> m_pimgs;

};

EXPORT int runImageMerge(CImage_uint8_t* pbuf, int len, CImage_uint8_t* pout);
EXPORT int clearImageMerge();

}