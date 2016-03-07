#pragma once

#include <cstdio>
#include <cstdarg>
#include <png.h>
#include "cimage.h"
#include "cimageUtil.h"

namespace cimage{
namespace Tools{

class FileOpener {
public:
	FileOpener(const char* filename, const char* mode){
		fopen_s(&f, filename, mode);
	}
	~FileOpener() {
		if (f != nullptr) {
			fclose(f);
		}
	}

	operator FILE*() {return f;}

private:
	FILE * f = nullptr;
};

typedef bool(*CheckFunc)(bool condition, const char* fmt, ...);

inline bool CheckFail(bool condition, const char* fmt, ...) 
{
	if (!condition) 
	{
		char buffer[1024];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buffer, sizeof(buffer), fmt, args);
		va_end(args);
		fprintf(stderr, "%s", buffer);
		exit(-1);
	}
	return condition;
}

inline bool CheckReturn(bool condition, const char* fmt, ...) 
{
	return condition;
}

inline bool CheckPrint(bool condition, const char* fmt, ...)
{
	if (!condition)
	{
		char buffer[1024];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buffer, sizeof(buffer), fmt, args);
		va_end(args);
		fprintf(stderr, "%s", buffer);
	}
	return condition;
}

EXPORT bool load_png(const std::string& filename, CImage_uint8_t& img, CheckFunc check = CheckPrint);

EXPORT bool save_png(const CImage_uint8_t& img, const std::string& filename, CheckFunc check = CheckPrint);

class load_image {
public:
	load_image(const std::string &f) : filename(f) {}
	inline operator CImage_uint8_t() {
		CImage_uint8_t im;
		load_png(filename, im);
		return im;
	}
private:
	const std::string filename;
};

void save_image(const CImage_uint8_t &im, const std::string &filename) {
	save_png(im, filename);
}

}	// namespace Tools
}	// namespace cimage
