#pragma once

#include <string>
#include <vector>

namespace cimage
{
	template<typename T>
	struct CImageBuffer
	{
		T* pbuf;
		uint32_t length;
		uint32_t width;
		uint32_t height;
		uint32_t channel;
		uint32_t selfgc;

		CImageBuffer(int w = 0, int h = 0, int c = 0):
			width(w), height(h), channel(c)
		{
			length = width * height * channel;
			if (length > 0)
			{
				pbuf = new T[length];
			}
		}

		~CImageBuffer()
		{
			if (selfgc && pbuf)
				delete pbuf;
		}
	};	

	class ImageMatchMerge
	{
	public:

		bool run();

		CImageBuffer<uint8_t> m_result;

	private:
		CImageBuffer<uint32_t> sumImageRow(const CImageBuffer<uint8_t>& input);

		CImageBuffer<uint32_t> sumImageRowBlock(const CImageBuffer<uint8_t>& input, int block_width);

		std::tuple<int, int> findHeadAndTail(const CImageBuffer<uint32_t>& sum1,
			const CImageBuffer<uint32_t>& sum2);

		template<typename T>
		CImageBuffer<T> cutHeadAndTail(const CImageBuffer<T>& input, int headLen, int tailLen);

		template<typename T>
		int avgMatchImages(const CImageBuffer<T>& top, const CImageBuffer<T>& down);

		std::vector<CImageBuffer<uint8_t>*> pimgs;

	};



	int testImageBuffer(CImageBuffer<uint8_t>* pbuf, int len);
}