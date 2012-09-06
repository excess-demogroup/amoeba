#include "image.h"
#include "packer/file.h"

class JPEGImage : public Image {
public:
	JPEGImage(File *file);
	~JPEGImage();

	int get_width();
	int get_height();
	unsigned char *get_pixel_data();

protected:
	unsigned char *image_buf;
};
