#include "image.h"
#include "packer/file.h"

class PNGImage : public Image {
public:
	PNGImage(File *file);
	~PNGImage();

	int get_width();
	int get_height();
	unsigned char *get_pixel_data();

protected:
	unsigned char *image_buf;
};
