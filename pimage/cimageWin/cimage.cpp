#include <iostream>
#include <string>
#include <ctime>
#include <cassert>
#include <algorithm>
#include "cimage.h"
#include "Helper.h"

using namespace std;
using namespace cimage;
using namespace cimage::Tools;


int cimage::testImageBuffer(CImage_uint8_t* pbuf, int len)
{
	Log("test log in native code.");

	cout << "test log in native code 2." << endl;

	CImage_uint8_t& img = pbuf[0];

	for (size_t i = 0; i < img.length; i += 4)
	{
		img.pbuf[i] = 255;
	}

	return pbuf[0].pbuf[3];
}

ImageMatchMerge::ImageMatchMerge(CImage_uint8_t** pbuf, int len)
{
	pimgs.resize(len);

	memcpy(pimgs.data(), pbuf, len * sizeof(CImage_uint8_t*));

}

CImage_uint32_t ImageMatchMerge::sumImageRow(const CImage_uint8_t& input)
{
	CImage_uint32_t ret(1, input.height, input.channel);

	uint32_t* pret = ret.pbuf;
	uint8_t* pin = input.pbuf;

	for (size_t j = 0; j < input.height; j++)
	{
		for (size_t i = 0; i < input.width; i++)
		{
			for (size_t k = 0; k < input.channel; k++)
			{
				pret[k] += *pin;
				++pin;
			}
		}
		pret += ret.stride();
	}

	return ret;
}

CImage_uint32_t ImageMatchMerge::sumImageRowBlock(const CImage_uint8_t& input, int block_width)
{
	size_t block = input.width / block_width;
	if (input.width > block * block_width)
		++block;

	const uint32_t channel = input.channel;

	CImage_uint32_t output(block, input.height, channel);
	uint32_t* pret = output.pbuf;
	uint8_t* pin = input.pbuf;

	for (size_t j = 0; j < input.height; ++j)
	{
		for (size_t b = 0; b < block ; ++b)
		{
			for (size_t i = b * block_width; i < min(input.width, block_width * (b + 1)); ++i)
			{
				for (size_t k = 0; k < channel; ++k)
				{
					pret[k] += pin[k];
				}
				pin += channel;
			}
			pret += channel;
		}
	}

	return output;
}

std::tuple<int, int> ImageMatchMerge::findHeadAndTail(const CImage_uint32_t& sum1,
	const CImage_uint32_t& sum2)
{
	int height = min(sum1.height, sum2.height);
	int width = min(sum1.width, sum2.width);

	if (sum1.channel != sum2.channel || 
		height == 0 || width == 0)
		return tuple<int, int>(0, 0);

	int head = 0, tail = height - 1;

	uint32_t* psum1 = sum1.pbuf;
	uint32_t* psum2 = sum2.pbuf;

	for (; head < height; ++head)
	{
		int is_diff = memcmp(psum1, psum2, width * sum1.channel * sizeof(uint32_t));

		if (is_diff)
		{
			--head;
			break;
		}
		psum1 += sum1.stride();
		psum2 += sum2.stride();
	}

	psum1 = sum1.end() - sum1.stride();
	psum2 = sum2.end() - sum2.stride();

	for (; tail >= 0; --tail)
	{
		int is_diff = memcmp(psum1, psum2, width * sum1.channel * sizeof(uint32_t));

		if (is_diff)
		{
			++tail;
			break;
		}
		psum1 -= sum1.stride();
		psum2 -= sum2.stride();
	}

	tail = sum1.height - 1 - tail;

	return tuple<int, int>(head, tail);
}

template<typename T>
CImage<T> ImageMatchMerge::cutHeadAndTail(const CImage<T>& input, int headLen, int tailLen)
{
	CImage<T> ret;
	ret.width = input.width;
	ret.channel = input.channel;
	ret.height = input.height - headLen - tailLen;
	ret.length = ret.width * ret.height * ret.channel;
	ret.pbuf = input.pbuf + headLen * ret.stride();

	return ret;
}

template<typename T>
inline float calcAvgMatch(const CImage<T>& top, const CImage<T>& down, int offset)
{
	const int height = min(top.height - offset, down.height);
	const int width = min(top.width, down.width);
	int match = 0;
	const T* ptop = top.pbuf + top.stride() * offset;
	const T* pdown = down.pbuf;
	const size_t cmp_size = top.channel * sizeof(T);


	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			if(memcmp(ptop, pdown, cmp_size) == 0)
			{
				++match;
			}
			ptop += top.channel;
			pdown += top.channel;
		}

	}

	return float(match) / (height * width);
}

template<typename T>
int ImageMatchMerge::avgMatchImages(const CImage<T>& top, const CImage<T>& down)
{
	const int height = min(top.height, down.height);

	int res = 0;
	float maxm = 0;
	for (int h = 1; h <= height; ++h)
	{
		float avgm = calcAvgMatch(top, down, height - h);
		if (avgm >= maxm)
		{
			maxm = avgm;
			res = max(res, h);
		}
	}

	return res;
}

bool ImageMatchMerge::run()
{
	clock_t begin = clock(), end = 0;
	float elapsed_time = 0;

	const int num = pimgs.size();

	if (num <= 0)
		return false;

	const int width = pimgs[0]->width, height = pimgs[0]->height, channel = pimgs[0]->channel;

	for (int i = 1; i < num; ++i)
	{
		if (pimgs[1]->width != width || pimgs[i]->channel != channel)
		{
			cout << "width or channel of images are different !!" << endl;
			return false;
		}
	}

	vector<CImage_uint32_t > sums(num);

	// sum image block 
	for (int i = 0; i < num; ++i)
		sums[i] = sumImageRow(*pimgs[i]);

	end = clock();
	elapsed_time = float(end - begin) / CLOCKS_PER_SEC;
	printf("sum image block time = %f\n", elapsed_time);
	begin = clock();


	int head = 0, tail = 0;
	for (int i = 0; i < num - 1; ++i)
	{
		auto ht = findHeadAndTail(sums[0], sums[1]);

		head = max(head, get<0>(ht));
		tail = max(tail, get<1>(ht));
	}

	printf("head = %d, tail = %d\n", head, tail);

	assert(tail + head < height);

	end = clock();
	elapsed_time = float(end - begin) / CLOCKS_PER_SEC;
	printf("findHeadAndTail time = %f\n", elapsed_time);
	begin = clock();

	// cut head and tail
	vector<CImage_uint8_t> cuts(num);
	vector<CImage_uint32_t> cut_sums(num);
	for (int i = 0; i < num; ++i)
	{
		cuts[i] = cutHeadAndTail(*pimgs[i], head, tail);
		cut_sums[i] = sumImageRowBlock(cuts[i], 20);
	}

	end = clock();
	elapsed_time = float(end - begin) / CLOCKS_PER_SEC;
	printf("cutHeadAndTail time = %f\n", elapsed_time);
	begin = clock();

	// find match bwtween cuts
	vector<int> match(num, 0);
	int matchall = 0;
	for (int i = 0; i < num - 1; ++i)
	{
		//match[i] = avgMatchImages(cuts[i], cuts[i + 1]);
		match[i] = avgMatchImages(cut_sums[i], cut_sums[i + 1]);
		matchall += match[i];
		cout << "match = " << match[i] << endl;
	}

	end = clock();
	elapsed_time = float(end - begin) / CLOCKS_PER_SEC;
	printf("avgMatchImages time = %f\n", elapsed_time);
	begin = clock();

	// joint all the cut images
	const int res_width = width;
	const int res_height = height * 3 - (head + tail) * 2 - matchall;
	const int res_channel = channel;
	result = CImage_uint8_t(res_width, res_height, res_channel);

	uint8_t* pcur = result.pbuf;

	//header
	memcpy(pcur, pimgs[0]->pbuf, width * channel * head);
	pcur += width * channel * head;

	//image
	for (int i = 0; i < num; ++i)
	{
		size_t length = (cuts[i].height - match[i]) * cuts[i].stride();
		memcpy(pcur, cuts[i].pbuf, length);
		pcur += length;
	}

	//tail
	memcpy(pcur, &pimgs[0]->operator()(0, height - tail - 1), tail * pimgs[0]->stride());

	end = clock();
	elapsed_time = float(end - begin) / CLOCKS_PER_SEC;
	printf("joint time = %f\n", elapsed_time);
	begin = clock();

	return true;
}