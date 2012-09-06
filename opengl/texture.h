#ifndef _TEXTURE_H
#define _TEXTURE_H

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include "image/image.h"

class Texture {
public:
	Texture(Image *img);
	~Texture();

	int get_width() { return width; }
	int get_height() { return height; }
	void bind();

	/* ick :-) */
	int refcount;
	
protected:
	int width, height;
	GLuint texnum;

	void add_mipmap(GLenum ifmt, GLenum fmt, int bpp, int w, int h,
		         int level, unsigned char *pixdata);
};
namespace texture {
	Texture *load(const char *filename);
	void free(Texture *tex);

	GLenum get_opengl_format(int bpp);
	GLenum get_opengl_internal_format(int bpp);
}

#endif /* !defined(_TEXTURE_H) */
