#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "texture.h"
#include "../image/image.h"
#include "../demolib_prefs.h"
#include "../exception.h"

#if DEMOLIB_TEXTURES

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>

/*
 * umm, ugly, but it does the job. :-) a hash table isn't what
 * we want here, because we want to look up by both values...
 * and we don't need the extra speed anyhow. :-) besides, it gives
 * us less init'ing...
 */
struct {
	char texname[256];
	Texture *tex;
} textures[256];
int max_tex_num = -1;

Texture *texture::load(const char *filename)
{
	/* see if it is in the `cache' */
	for (int i = 0; i <= max_tex_num; i++) {
		if (strcmp(textures[i].texname, filename) == 0 &&
		    textures[i].tex != NULL) {
			Texture *tex = textures[i].tex;
			tex->refcount++;

			return tex;
		}
	}

	Image *img = load_image(filename);
	Texture *tex = new Texture(img);
	delete img;

	tex->refcount = 1;

	max_tex_num++;
	strcpy(textures[max_tex_num].texname, filename);
	textures[max_tex_num].tex = tex;

	return tex;
}

void texture::free(Texture *tex)
{
	if (tex == NULL) return;

	if (--(tex->refcount) == 0) {
		delete tex;
		for (int i = 0; i <= max_tex_num; i++) {
			if (textures[i].tex == tex) {
				textures[i].tex = NULL;
				while (i >= max_tex_num && textures[max_tex_num].tex == NULL) {
					max_tex_num--;
				}
				break;
			}
		}
	}
}

GLenum texture::get_opengl_format(int bpp)
{
	switch (bpp) {
	case 8:
		return GL_LUMINANCE;
	case 16:
		return GL_LUMINANCE_ALPHA;
	case 24:
		return GL_RGB;
	case 32:
		return GL_RGBA;
	}

	/* better debug message later ;-) */
	throw new FatalException("texture::get_opengl_format(): Unknown bpp!");
}

GLenum texture::get_opengl_internal_format(int bpp)
{
	switch (bpp) {
	case 8:
		return GL_LUMINANCE8;
	case 16:
		return GL_LUMINANCE8_ALPHA8;
	case 24:
		// return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
		return GL_RGB8;
	case 32:
		// return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		return GL_RGBA8;
	}

	/* better debug message later ;-) */
	throw new FatalException("texture::get_opengl_internal_format(): Unknown bpp!");
}

Texture::Texture(Image *img)
{
	int w, h;
	unsigned char *pixdata;

	/*
	 * clip both dimensions to power-of-two (we used to do this with
	 * 1 << (int)(floor(log((double)(x)) / log(2))), but that proved
	 * inaccurate)
	 */
	for (w = (1<<30); w > 0; w >>= 1) {
		if (img->get_width() & w) {
			break;
		}
	}
	for (h = (1<<30); h > 0; h >>= 1) {
		if (img->get_height() & h) {
			break;
		}
	}

	pixdata = (unsigned char *)(malloc(w * h * img->get_bpp() / 8));
	if (pixdata == NULL) throw new FatalException("Out of memory");

	memcpy(pixdata, img->get_pixel_data(), w * h * img->get_bpp() / 8);
	GLenum fmt = texture::get_opengl_format(img->get_bpp());
	GLenum ifmt = texture::get_opengl_internal_format(img->get_bpp());
	
        /* now see if we need to scale down to fit the card :-( */
        GLint maxsize;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxsize);
//	maxsize = 256;

	int new_w = w, new_h = h;

	if (w > maxsize || h > maxsize) {
		if (new_w > maxsize)
			new_w = maxsize;
		if (new_h > maxsize)
			new_h = maxsize;

		unsigned char *pixdata_s = (unsigned char *)malloc(new_w * new_h * 4);
		if (pixdata_s == NULL)
			throw new FatalException("Out of memory!");

		gluScaleImage(fmt, w, h, GL_UNSIGNED_BYTE, pixdata, new_w, new_h,
			GL_UNSIGNED_BYTE, pixdata_s);

		free(pixdata);
                pixdata = pixdata_s;

		w = new_w;
		h = new_h;
        }

	glGenTextures(1, &(this->texnum));
	glBindTexture(GL_TEXTURE_2D, this->texnum);
	
//	glTexImage2D(GL_TEXTURE_2D, 0, ifmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, pixdata);
//	this->add_mipmap(ifmt, fmt, img->get_bpp(), w, h, 0, pixdata);
	
	gluBuild2DMipmaps(GL_TEXTURE_2D, ifmt, w, h, fmt, GL_UNSIGNED_BYTE, pixdata);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
/*        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); */

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	free(pixdata);

	this->width = w;
	this->height = h;
}

Texture::~Texture()
{
	glDeleteTextures(1, &(this->texnum));
}

void Texture::bind()
{
	glBindTexture(GL_TEXTURE_2D, this->texnum);
}

void Texture::add_mipmap(GLenum ifmt, GLenum fmt, int bpp, int w, int h,
		         int level, unsigned char *pixdata)
{
	if (w == 1 && h == 1) return;

	int comp = bpp / 8;
	
	int nw = (w == 1) ? 1 : (w >> 1);
	int nh = (h == 1) ? 1 : (h >> 1);

	unsigned char *npd = (unsigned char *)malloc(nw * nh * comp);
	if (npd == NULL)
		throw new FatalException("Out of memory!");

	if (nw == 1) {
		/*
		 * ick, special case, but we can fool the scaler and pretend
		 * we have a different picture, so it works well anyhow ;-)
		 */
		nw = nh;
		nh = 1;
	}
	
	/*
	 * general nice scaling code ;-) works for h == 1
	 * too, although suboptimal... who cares
	 */
	for (int y = 0; y < nh; y++) {
		unsigned char *src1 = (unsigned char *)(pixdata + (y*2)*w*comp);
		unsigned char *src2 = (unsigned char *)(pixdata + (y*2+1)*w*comp);
		if (h == 1) src2 = src1;
		
		unsigned char *dst = (unsigned char *)(npd + y*nw*comp);
			
		for (int x = 0; x < nw; x++) {
			for (int c = 0; c < comp; c++) {
				*dst++ = (unsigned char)
					(((unsigned int)(*src1) +
					  (unsigned int)(*src2) +
					  (unsigned int)(*(src1+comp)) + 
					  (unsigned int)(*(src2+comp))) >> 2);
				src1++;
				src2++;
			}
			src1 += comp;
			src2 += comp;
		}
	}

	/* just in case we messed with these values further up */
	nw = (w == 1) ? 1 : (w >> 1);
	nh = (h == 1) ? 1 : (h >> 1);
	
	glTexImage2D(GL_TEXTURE_2D, level + 1, ifmt, nw, nh, 0, fmt, GL_UNSIGNED_BYTE, npd);
	this->add_mipmap(ifmt, fmt, bpp, nw, nh, level + 1, npd);
	free(npd);
}

#endif
