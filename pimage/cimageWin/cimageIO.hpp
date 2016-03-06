#pragma once

#include <cstdio>
#include <cstdarg>
#include <png.h>
#include "cimage.h"


namespace cimage{
namespace Tools{

class FileOpener {
public:
	FileOpener(const char* filename, const char* mode){
		//fopen_s(&f, filename, mode);
	}
	~FileOpener() {
		if (f != nullptr) {
			fclose(f);
		}
	}

	operator FILE*() {return f;}

private:
	FILE * const f = nullptr;
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

bool load_png(const std::string& filename, CImage_uint8_t& img, CheckFunc check = CheckPrint)
{
	png_image image;

	/* Only the image structure version number needs to be set. */
	memset(&image, 0, sizeof image);
	image.version = PNG_IMAGE_VERSION;

	if (!check(png_image_begin_read_from_file(&image, filename.c_str()), "load_png: %s: %s\n", filename.c_str(), image.message)) 
		return false;

	image.format = PNG_FORMAT_RGBA;
	
	img = CImage_uint8_t(image.width, image.height, PNG_IMAGE_PIXEL_CHANNELS(image.format));

	const unsigned long length = PNG_IMAGE_SIZE(image);

	if (!check(img.pbuf != nullptr, "load_png: %s: out of memory, %ld bytes\n", filename.c_str(), length))
	{
		png_image_free(&image);
		return false;
	}

	if (!check(img.length == length, "load_png: %s: length not match, %ld bytes\n", filename.c_str(), length))
	{
		png_image_free(&image);
		return false;
	}

	if (!check(png_image_finish_read(&image, NULL, img.pbuf, 0, NULL), "load_png: read %s: %s\n", filename.c_str(), image.message))
	{
		png_image_free(&image);
		return false;
	}

	return true;

}

bool save_png(const CImage_uint8_t& img, const std::string& filename, CheckFunc check = CheckPrint)
{
	if (!check(img.channel == 4, "save_png: %s: only support RGBA format\n", filename.c_str()))
		return false;

	png_image image;

	image.version = PNG_IMAGE_VERSION;
	image.opaque = NULL;
	image.width = img.width;
	image.height = img.height;
	image.format = PNG_FORMAT_RGBA;
	image.flags = 0;
	image.colormap_entries = 0;

	if (!check(png_image_write_to_file(&image, filename.c_str(), 0, img.pbuf, 0, NULL),
		"save_png: save %s: %s\n", filename.c_str(), image.message))
		return false;

	return true;
}

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
