#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "demolib_prefs.h"

#if DEMOLIB_OPENGL_FONT_TTF

#include <ft2build.h>
#include FT_FREETYPE_H

#include "fontttf.h"
#include "../exception.h"

#include "packer/file.h"

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>

#ifndef M_PI
#define M_PI 3.141592653589793238462643383279502
#endif

#define PIXMAP_HEIGHT 128
#define PIXMAP_BASE 96
#define SCALE_FACTOR 0.5f

FontTTF::FontTTF(File *ttffile, const char *text, int size)
{
	if (FT_Init_FreeType(&library))
		throw new FatalException("FreeType init failed.");
	if (FT_New_Memory_Face(library, (FT_Byte *)ttffile->get_data(),
			       ttffile->data_length(), 0, &face))
		throw new FatalException("Face opening failed.");
	if (FT_Set_Char_Size(face, 0, (int)(size * (64.0f/SCALE_FACTOR)), 96, 96))
		throw new FatalException("Size set failed.");
	
	realwidth = width = draw_text(text, false);

	/* now round up to closest power-of-two */
	for (int i = 0; i < 31; i++) {
		if (width <= (1 << i)) {
			width = 1 << i;
			break;
		}
	}

	texmap = (unsigned char *)(malloc(width * PIXMAP_HEIGHT));
	if (texmap == NULL)
		throw new FatalException("Out of memory!");

	memset(texmap, 0, width * PIXMAP_HEIGHT);

	draw_text(text, true);

	FT_Done_Face(face);
	FT_Done_FreeType(library);

	/* now see if we need to scale down to fit the card :-( */
	GLint texwidth = width;
	GLint maxwidth;

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxwidth);

	while (texwidth > maxwidth) {
		/* scale down in the X res only -- everybody supports 32x32 ;-) */
		texwidth >>= 1;
		unsigned char *texmap_s = (unsigned char *)malloc(texwidth * PIXMAP_HEIGHT);
		if (texmap_s == NULL)
			throw new FatalException("Out of memory!");

		for (int y = 0; y < PIXMAP_HEIGHT; y++) {
			unsigned char *src = texmap + y * (texwidth * 2);
			unsigned char *dst = texmap_s + y * texwidth;
			for (int x = 0; x < texwidth; x++) {
 				unsigned int s1 = (unsigned int)(*src++);
 				unsigned int s2 = (unsigned int)(*src++);
				*dst++ = (unsigned char)((s1 + s2) >> 1);
			}
		}
		free(texmap);
		texmap = texmap_s;
	}

	/* 
	 * bah, GL_ALPHA gives us RGB=0,0,0 and we want RGB=1,1,1... have
	 * to make the RGB part ourselves. :-)
	 */
	unsigned char *texptr = (unsigned char *)malloc(texwidth * PIXMAP_HEIGHT * 2);
	unsigned char *inptr = texmap, *outptr = texptr;
	if (texptr == NULL)
		throw new FatalException("Out of memory!");

	for (int i = 0; i < texwidth * PIXMAP_HEIGHT; i++) {
		*outptr++ = 255;
		*outptr++ = *inptr++;
	}
	
	glGenTextures(1, &texnum);
	glBindTexture(GL_TEXTURE_2D, texnum);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_LUMINANCE8_ALPHA8, texwidth, PIXMAP_HEIGHT, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, texptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	free(texptr);
	free(texmap);
}
FontTTF::~FontTTF()
{
	glDeleteTextures(1, &texnum);
}

void FontTTF::draw_object(float xpos, float ypos,
			  float red, float green, float blue,
			  float alpha, bool additive)
{
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
	glLoadIdentity();

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	
        glOrtho(0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 10.0f);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
	if (additive) {
	        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	} else {
	        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	glBindTexture(GL_TEXTURE_2D, this->texnum);
        glEnable(GL_TEXTURE_2D);
     
	glScalef(DEMOLIB_YASPECT / DEMOLIB_XASPECT, 1.0f, 1.0f);
	glTranslatef(xpos + (float)((256.0f/SCALE_FACTOR) - width) / (256.0f/SCALE_FACTOR) * 0.4f, ypos, 0.0f);

	glColor4f(red, green, blue, alpha);
	
        glBegin(GL_QUADS);
                glTexCoord2f(0.0f, 1.0f);
                glVertex3f(0.0f, 0.2f, 0.0f);
                glTexCoord2f(1.0f, 1.0f);
                glVertex3f(0.8f * ((float)width / (256.0f/SCALE_FACTOR)), 0.2f, 0.0f);
                glTexCoord2f(1.0f, 0.0f);
                glVertex3f(0.8f * ((float)width / (256.0f/SCALE_FACTOR)), 0.0f, 0.0f);
                glTexCoord2f(0.0f, 0.0f);
                glVertex3f(0.0f, 0.0f, 0.0f);
        glEnd();

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	glMatrixMode(GL_PROJECTION);
        glPopMatrix();

        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
}

int FontTTF::draw_text(const char *str, bool real_render)
{
	FT_GlyphSlot slot = face->glyph;
	int x = 0;

	/* center the text */
	if (real_render) x = (this->width - this->realwidth) / 2;

	for (unsigned int i = 0; i < strlen(str); i++) {
		if (FT_Load_Char(face, str[i], FT_LOAD_RENDER))
			continue;

		if (real_render) {
			FT_Bitmap *bm = &(slot->bitmap);
			for (int y = 0; y < bm->rows; y++) {
				if (PIXMAP_BASE - slot->bitmap_top + y < 0 ||
				    PIXMAP_BASE - slot->bitmap_top + y >= PIXMAP_HEIGHT) continue;

				memcpy(texmap + (PIXMAP_BASE - slot->bitmap_top + y) * width + x + slot->bitmap_left,
				       bm->buffer + y * bm->width,
				       bm->width);
			}
		}

		x += slot->advance.x >> 6;
	}

	return x;
}

#endif
