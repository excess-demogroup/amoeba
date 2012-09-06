#ifndef _IMAGECOMBINER_H
#define _IMAGECOMBINER_H 1

#include "image.h"
#include "packer/file.h"

class ImageCombiner : public Image {
public:
	ImageCombiner(Image *rgb, Image *alpha);
	~ImageCombiner();

	unsigned char *get_pixel_data();
	
protected:
	unsigned char *image_buf;
};

#endif /* !defined(_IMAGECOMBINER_H) */
