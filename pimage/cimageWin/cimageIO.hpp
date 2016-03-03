#pragma once

#include <cstdio>
#include <cstdarg>
#include "cimage.h"
#include "png.h"

namespace cimage{

namespace Tools{

struct FileOpener {
	FileOpener(const char* filename, const char* mode) : f(fopen(filename, mode)) {
		// nothing
	}
	~FileOpener() {
		if (f != nullptr) {
			fclose(f);
		}
	}
	FILE * const f;
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
	png_byte header[8];
	png_structp png_ptr;
	png_infop info_ptr;

	/* open file and test for it being a png */

	FileOpener f(filename.c_str(), "rb");

	if (!check(f.f != nullptr, "File %s could not be opened for reading\n", filename.c_str())) return false;
	if (!check(fread(header, 1, 8, f.f) == 8, "File ended before end of header\n")) return false;
	if (!check(!png_sig_cmp(header, 0, 8), "File %s is not recognized as a PNG file\n", filename.c_str())) return false;

	/* initialize stuff */
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!check(png_ptr != NULL, "png_create_read_struct failed\n")) return false;

	info_ptr = png_create_info_struct(png_ptr);
	if (!check(info_ptr, "png_create_info_struct failed\n")) return false;

	if (!check(!setjmp(png_jmpbuf(png_ptr)), "Error during init_io\n")) return false;

	png_init_io(png_ptr, f.f);
	png_set_sig_bytes(png_ptr, 8);

	png_read_info(png_ptr, info_ptr);

	int width = png_get_image_width(png_ptr, info_ptr);
	int height = png_get_image_height(png_ptr, info_ptr);
	int channels = png_get_channels(png_ptr, info_ptr);
	int bit_depth = png_get_bit_depth(png_ptr, info_ptr);

	// Expand low-bpp images to have only 1 pixel per byte (As opposed to tight packing)
	if (bit_depth < 8) {
		png_set_packing(png_ptr);
	}

	if (channels != 1) {
		*im = ImageType(width, height, channels);
	}
	else {
		*im = ImageType(width, height);
	}

	png_set_interlace_handling(png_ptr);
	png_read_update_info(png_ptr, info_ptr);

	// read the file
	if (!check(!setjmp(png_jmpbuf(png_ptr)), "Error during read_image\n")) return false;

	Internal::PngRowPointers row_pointers(im->height(), png_get_rowbytes(png_ptr, info_ptr));
	png_read_image(png_ptr, row_pointers.p);

	if (!check((bit_depth == 8) || (bit_depth == 16), "Can only handle 8-bit or 16-bit pngs\n")) return false;

	// convert the data to ImageType::ElemType

	int c_stride = (im->channels() == 1) ? 0 : im->stride(2);
	typename ImageType::ElemType *ptr = (typename ImageType::ElemType*)im->data();
	if (bit_depth == 8) {
		for (int y = 0; y < im->height(); y++) {
			uint8_t *srcPtr = (uint8_t *)(row_pointers.p[y]);
			for (int x = 0; x < im->width(); x++) {
				for (int c = 0; c < im->channels(); c++) {
					Internal::convert(*srcPtr++, ptr[c*c_stride]);
				}
				ptr++;
			}
		}
	}
	else if (bit_depth == 16) {
		for (int y = 0; y < im->height(); y++) {
			uint8_t *srcPtr = (uint8_t *)(row_pointers.p[y]);
			for (int x = 0; x < im->width(); x++) {
				for (int c = 0; c < im->channels(); c++) {
					uint16_t hi = (*srcPtr++) << 8;
					uint16_t lo = hi | (*srcPtr++);
					Internal::convert(lo, ptr[c*c_stride]);
				}
				ptr++;
			}
		}
	}

	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	im->set_host_dirty();
	return cimage::CImage<uint8_t>();
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

}	// namespace Tools
}	// namespace cimage

