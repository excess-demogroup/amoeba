#include <stdlib.h>

#include "image/imagecombiner.h"
#include "exception.h"

ImageCombiner::ImageCombiner(Image *rgb, Image *alpha)
{
	if (rgb->get_width()  != alpha->get_width() ||
	    rgb->get_height() != alpha->get_height()) {
		throw new FatalException("Alpha image is not the same dimensions as the base image!");
	}
		
	if (rgb->get_bpp() != 8 && rgb->get_bpp() != 24) {
		throw new FatalException("RGB image must be 8bpp or 24bpp!");
	}
	
	if (alpha->get_bpp() != 8) {
		throw new FatalException("Alpha map but isn't 8bpp!");
	}
		
	/* slow but works well. :-) */
	this->width = rgb->get_width();
	this->height = rgb->get_height();
	if (rgb->get_bpp() == 24) {
		this->bpp = 32;
		this->image_buf = (unsigned char *)malloc(width * height * 4);
		if (this->image_buf == NULL)
			throw new FatalException("Out of memory!");

		unsigned char *ptr_rgb   = rgb->get_pixel_data();
		unsigned char *ptr_alpha = alpha->get_pixel_data();
		unsigned char *out       = this->image_buf;
		
		for (unsigned int i = 0; i < width * height; i++) {
			*out++ = *ptr_rgb++;
			*out++ = *ptr_rgb++;
			*out++ = *ptr_rgb++;
			*out++ = *ptr_alpha++;
		}
	} else {
		this->bpp = 16;
		this->image_buf = (unsigned char *)malloc(width * height * 2);
		if (this->image_buf == NULL)
			throw new FatalException("Out of memory!");
		
		unsigned char *ptr_rgb   = rgb->get_pixel_data();
		unsigned char *ptr_alpha = alpha->get_pixel_data();
		unsigned char *out       = this->image_buf;
		
		for (unsigned int i = 0; i < width * height; i++) {
			*out++ = *ptr_rgb++;
			*out++ = *ptr_alpha++;
		}
	}
}

ImageCombiner::~ImageCombiner()
{
	free(this->image_buf);
	this->image_buf = NULL;
}

unsigned char *ImageCombiner::get_pixel_data()
{
	return this->image_buf;
}
