/*
 * Sesse's demolib: JPEG-specific functions
 * Based on libjpeg's example.c (so it needs libjpeg)
 */
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "demolib_prefs.h"

#include "jpeg_image.h"
#include "../exception.h"
#include "packer/file.h"

#if DEMOLIB_IMAGE_JPEG

extern "C" {
	#include <jpeglib.h>
	#include <jerror.h>
}

/* we need an error manager to make this work cleanly */
struct my_error_mgr {
	struct jpeg_error_mgr pub;    /* "public" fields */
	jmp_buf setjmp_buffer;        /* for return to caller */
};
typedef struct my_error_mgr * my_error_ptr;

/* And this is the error handler -- it simply throws the error :-) */ 
void my_error_exit(j_common_ptr cinfo) {
	char buf[JMSG_LENGTH_MAX];
	(*cinfo->err->format_message)(cinfo, buf);
	throw new FatalException("JPEG error", buf);
}

/*
 * These four functions constitute a simple memory reader, for making
 * libjpeg read from a pointer, not a file
 */
void noop(j_decompress_ptr cinfo) {}
#if defined(__GNUC__) || defined(__ICC__)
int fill(j_decompress_ptr cinfo)
#else
unsigned char fill(j_decompress_ptr cinfo)
#endif
{
	if (cinfo->src->bytes_in_buffer >= 0) {
		return TRUE;
	} else {
		return FALSE;
	}
}
void skip (j_decompress_ptr cinfo, long num_bytes)
{
	cinfo->src->bytes_in_buffer -= num_bytes;
	cinfo->src->next_input_byte += num_bytes;
}

void init_mem_source(j_decompress_ptr cinfo, File *file)
{
	cinfo->src = (struct jpeg_source_mgr *)
		(*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
					     sizeof(struct jpeg_source_mgr));

	cinfo->src->init_source = noop;
	cinfo->src->fill_input_buffer = fill;
	cinfo->src->skip_input_data = skip;
	cinfo->src->resync_to_restart = jpeg_resync_to_restart; /* use default method */
	cinfo->src->term_source = noop;
	cinfo->src->bytes_in_buffer = file->data_length(); /* forces fill_input_buffer on first read */
	cinfo->src->next_input_byte = (const JOCTET *)file->get_data(); /* until buffer loaded */
}

/*
 * Main function: Decompress JPEG into buffer. If buf is NULL, automatically
 *                allocates a suitable buffer.
 */
JPEGImage::JPEGImage(File *file)
{
	struct jpeg_decompress_struct cinfo;
	struct my_error_mgr jerr;
	int row_stride;
	JSAMPARRAY buffer;

	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;

	jpeg_create_decompress(&cinfo);

	/* source */
	init_mem_source(&cinfo, file);

	(void) jpeg_read_header(&cinfo, TRUE);
	(void) jpeg_start_decompress(&cinfo);

	this->width = cinfo.output_width;
	this->height = cinfo.output_height;
	this->bpp = cinfo.output_components * 8;

	row_stride = this->width * cinfo.output_components;
	buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);

	/* allocate the buffer */
	this->image_buf = (unsigned char *)(malloc(this->width * this->height * cinfo.output_components));
	if (this->image_buf == NULL) {
		throw new FatalException("Couldn't allocate memory for JPEG image");
	}

	while (cinfo.output_scanline < this->height) {
		const unsigned int w = this->width * this->bpp / 8;
		(void) jpeg_read_scanlines(&cinfo, buffer, 1);
		memcpy(this->image_buf + (cinfo.output_scanline-1) * w, buffer[0], w);
	}	

	(void) jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
}

JPEGImage::~JPEGImage()
{
	free(this->image_buf);
}

unsigned char *JPEGImage::get_pixel_data()
{
	return this->image_buf;
}

#endif /* DEMOLIB_IMAGE_JPEG */
