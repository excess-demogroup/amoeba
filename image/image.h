/*
 * virtual image class :-)
 */

#ifndef _IMAGE_H
#define _IMAGE_H

class Image {
public:
	Image();
	virtual ~Image();

	int get_width();
	int get_height();
	virtual unsigned char *get_pixel_data() = 0;
	int get_bpp();

protected:
	unsigned int width;
	unsigned int height;
	unsigned int bpp;
};

Image *load_image(const char * const filename);

#endif /* _IMAGE_H */
