#include "cimageIO.h"

using namespace cimage;
using namespace cimage::Tools;

bool cimage::Tools::load_png(const std::string& filename, CImage_uint8_t& img, CheckFunc check /*= CheckPrint*/)
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

bool cimage::Tools::save_png(const CImage_uint8_t& img, const std::string& filename, CheckFunc check /*= CheckPrint*/)
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