/*
 * Sesse's demolib
 */ 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "demolib_prefs.h"
#include "image/image.h"
#include "image/jpeg_image.h"
#include "image/png_image.h"
#include "image/imagecombiner.h"
#include "packer/file.h"
#include "exception.h"

#if !defined(__GNUC__) && !defined(__ICC__)
#define strcasecmp stricmp
#endif

Image *load_image(const char * const filename)
{
	/* 
	 * a syntax of "image:alpha" indicates that the image data and the alpha data
	 * is in separate files
	 */
	if (strchr(filename, ':') != NULL) {
		char *fn = strdup(filename);
		char *ptr = strchr(fn, ':');
		ptr[0] = 0;
		
		Image *rgb = load_image(fn);
		Image *alpha = load_image(ptr + 1);
		
		try {
			Image *combined = new ImageCombiner(rgb, alpha);
			delete rgb;
			delete alpha;
			free(fn);
			return combined;
		} catch (FatalException *e) {
			throw new FatalException(filename, e->get_error());
		}
	}
	
	File *file = load_file(filename);

	/* find out what kind of image this is by looking at the extension */
#if DEMOLIB_IMAGE_JPEG
	if (strcasecmp(filename + strlen(filename) - 4, ".jpg") == 0 ||
	    strcasecmp(filename + strlen(filename) - 5, ".jpeg") == 0) {
		Image *img = new JPEGImage(file);
		delete file;
		return img;
	}
#endif /* DEMOLIB_IMAGE_JPEG */
	
#if DEMOLIB_IMAGE_PNG
	if (strcasecmp(filename + strlen(filename) - 4, ".png") == 0) {
		Image *img = new PNGImage(file);
		delete file;
		return img;
	}
#endif /* DEMOLIB_IMAGE_PNG */

	throw new FatalException("Couldn't figure out type of image");
}
