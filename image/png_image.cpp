/*
 * Based on libpng's example.c (so it needs libpng)
 */

#include <stdio.h>
#include <stdlib.h>

#include <png.h>
#include "png_image.h"
#include "../exception.h"
#include "../demolib_prefs.h"

#if DEMOLIB_IMAGE_PNG

/* ugly, but works (unless there is corrupted data :-D) */
char *curr_png_ptr = NULL;
void read_mem(png_structp png_ptr, png_bytep data, png_size_t length)
{
	memcpy(data, curr_png_ptr, length);
	curr_png_ptr += length;
}   

PNGImage::PNGImage(File *file)
{
	png_structp png_ptr;
	png_infop info_ptr;
	int bit_depth, color_type, interlace_type;
//	png_color_16 my_background, *image_background;
	png_info intent;
	png_bytep *row_pointers;
	float screen_gamma;
	unsigned int row;

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		throw new FatalException("png_create_read_struct() failed");
	}

	/* Allocate/initialize the memory for image information. */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		throw new FatalException("png_create_info_struct() failed");
	}

	/* Set up error handling. */
	if (setjmp(png_ptr->jmpbuf)) {
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		throw new FatalException("PNG decompression failed");
	}

	/* Set up the memory reader. */
	curr_png_ptr = file->get_data();
	png_set_read_fn(png_ptr, NULL, read_mem);

	png_uint_32 width, height;
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr,
		&width, &height,
		&bit_depth, &color_type, &interlace_type, NULL, NULL);

	this->width = width;
	this->height = height;

	if (color_type == PNG_COLOR_TYPE_GRAY) {
		this->bpp = 8;
	}
	if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
		this->bpp = 16;
	}
	if (color_type == PNG_COLOR_TYPE_PALETTE) {
		this->bpp = 32;
	}
	if (color_type == PNG_COLOR_TYPE_RGB) {
		this->bpp = 24;
	}
	if (color_type == PNG_COLOR_TYPE_RGBA) {
		this->bpp = 32;
	}

	/* Allocate memory if needed */
        this->image_buf = (unsigned char *)(malloc(this->width * this->height * this->bpp / 8));
        if (this->image_buf == NULL) {
                throw new FatalException("Couldn't allocate memory for JPEG image");
        }
						
//	png_set_strip_16(png_ptr);
	png_set_packing(png_ptr);
	if (color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_expand(png_ptr);
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
		png_set_expand(png_ptr);
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		png_set_expand(png_ptr);

	
/*	if (png_get_bKGD(png_ptr, info_ptr, &image_background))
		png_set_background(png_ptr, image_background,
			PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
	else
		png_set_background(png_ptr, &my_background,
			PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0); */

	screen_gamma = 2.2;
	if (png_get_sRGB(png_ptr, info_ptr, (int *)(&intent))) {
		png_set_sRGB(png_ptr, &intent, 0);
	} else {
		double image_gamma;
		if (png_get_gAMA(png_ptr, info_ptr, &image_gamma))
			png_set_gamma(png_ptr, screen_gamma, image_gamma);
		else
			png_set_gamma(png_ptr, screen_gamma, 0.45455);
	}

//	png_set_bgr(png_ptr);

/*	if (color_type != PNG_COLOR_TYPE_RGBA)
		png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER); */

	/* Allocate the row pointers (ick!) */
	row_pointers = (png_bytep *)(malloc(sizeof(png_bytep) * this->height));
	if (row_pointers == NULL) {
		throw new FatalException("Out of memory");
	}
	for (row = 0; row < height; row++) {
		row_pointers[row] = this->image_buf + (this->width * row * this->bpp / 8);
	}

	/* Read the image in one pass. */
	png_read_image(png_ptr, row_pointers);

	free(row_pointers);
	
	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
}

PNGImage::~PNGImage()
{
	free(this->image_buf);
}

unsigned char *PNGImage::get_pixel_data()
{
	return this->image_buf;
}

#endif /* DEMOLIB_IMAGE_PNG */
