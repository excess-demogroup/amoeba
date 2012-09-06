/*
 * backgroundhandler.cpp: Static backgrounds, split into multiple
 *                        textures so we don't have to scale them (if
 *                        they match the screen resolution, that is).
 *
 *                        This class really isn't just about backgrounds anymore,
 *                        since it can be put on different layers and become
 *                        offsetted. Should rename it and possibly integrate
 *                        with <image> some day. :-)
 */

#include <stdio.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

#include "main/backgroundhandler.h"
#include "opengl/texture.h"
#include "opengl/extensions.h"
#include "exception.h"
#include "demolib_prefs.h"

#if DEMOLIB_MAINLOOP 

BackgroundHandler::BackgroundHandler(MainLoop *ml, const char *title, const char *elem, Hashtable *attr) :
	Event(ml, title, elem, attr, "alpha")
{
	if (attr->exists("layer")) {
		this->layer = attr->get_float("layer");
	} else {
		this->layer = -50.0f;
	}
	
	Image *baseimg = load_image(attr->get_str("file"));
	this->xres = baseimg->get_width();
	this->yres = baseimg->get_height();

	this->xoffs = 0;
	this->yoffs = 0;
	if (attr->exists("xoffs")) {
		this->xoffs = attr->get_int("xoffs");
	}
	if (attr->exists("yoffs")) {
		this->yoffs = attr->get_int("yoffs");
	}

	if (attr->exists("screenwidth")) {
		this->screenwidth = attr->get_int("screenwidth");
		if (this->screenwidth < this->xoffs + this->xres) {
			throw new FatalException(elem, "screenwidth= would make the image go outside the screen!");
		}
	} else {
		this->screenwidth = this->xoffs + this->xres;
	}
	if (attr->exists("screenheight")) {
		this->screenheight = attr->get_int("screenheight");
		if (this->screenheight < this->yoffs + this->yres) {
			throw new FatalException(elem, "screenheight= would make the image go outside the screen!");
		}
	} else {
		this->screenheight = this->yoffs + this->yres;
	}
	
	GLenum fmt = texture::get_opengl_format(baseimg->get_bpp());
	GLenum ifmt = texture::get_opengl_internal_format(baseimg->get_bpp());

	/*
	 * Find how many parts we need to split in :-)
	 *
	 * In order to support crappy cards that can't handle resolutions
	 * bigger than 256x256 (read: Voodoo1/2/3), we have to do it a
	 * bit weird: First build up an array containing the bits of the
	 * image resolution, then propagate down so e.g. 512x256 is split
	 * into two 256x256 images, finally count the number of x- and
	 * y-parts and split it.
	 */
        GLint maxsize;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxsize);
	
	int i;
	
	this->xparts = 0;
	this->yparts = 0;
	for (i = 0; i < 30; i++) {
		if (xres & (1<<i)) {
			xpartbits[i] = 1;
		} else {
			xpartbits[i] = 0;
		}
		if (yres & (1<<i)) {
			ypartbits[i] = 1;
		} else {
			ypartbits[i] = 0;
		}
	}

	/* propagate higher-order bits down to smallest size textures */
	for (i = 29; i >= 0; i--) {
		if ((1<<i) > maxsize) {
			xpartbits[i - 1] += (xpartbits[i]) * 2;
			xpartbits[i] = 0;

			ypartbits[i - 1] += (ypartbits[i]) * 2;
			ypartbits[i] = 0;
		} else {
			this->xparts += xpartbits[i];
			this->yparts += ypartbits[i];
		}
	}

	this->textures = new GLuint[xparts * yparts];
	glGenTextures(xparts * yparts, this->textures);

	const int psize = baseimg->get_bpp() / 8;
	
	/*
	 * now split the image in individual power-of-two-sized parts
	 * (ugly because of the texture max limit stuff)
	 */
	int yoffs = 0, texnum = 0;
	for (int ybit = 0; ybit < 30; ybit++) {
		const int h = 1 << ybit;
		for (int j = 0; j < ypartbits[ybit]; j++) {
			int xoffs = 0;
			for (int xbit = 0; xbit < 30; xbit++) {
				const int w = 1 << xbit;
				for (int k = 0; k < xpartbits[xbit]; k++) {
					/* 
					 * generate a memory area to store the things in
					 * (simplest, we don't have to mess around with
					 * strides etc.
					 */
					unsigned char *tmpbuf = new unsigned char[w * h * psize];
					for (int y = 0; y < h; y++) {
						memcpy(tmpbuf + y * w * psize,
							baseimg->get_pixel_data() + ((y+yoffs) * xres + xoffs) * psize,
							w * psize);
					}
			
					/* no mipmapping -- save texture memory :-) */
					glBindTexture(GL_TEXTURE_2D, this->textures[texnum++]);
					glTexImage2D(GL_TEXTURE_2D, 0, ifmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, tmpbuf);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
				        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

					delete[] tmpbuf;
				
					xoffs += w;
				}
			}
			yoffs += h;
		}
	}

	delete baseimg;
}

BackgroundHandler::~BackgroundHandler()
{
	glDeleteTextures(xparts * yparts, this->textures);
}

void BackgroundHandler::start_effect()
{
}

void BackgroundHandler::draw_scene(float progress)
{
	float alpha = this->get_val("alpha", progress);
	if (alpha < 0.0f) return;

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
	glLoadIdentity();

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();

	glOrtho(0.0f, (float)(this->screenwidth), (float)(this->screenheight), 0.0f, 0.0f, 1.0f);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(1.0f, 1.0f, 1.0f, alpha);
	
        glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glEnable(GL_TEXTURE_2D);

	/* regenerate picture from the parts */
	int yoffs = this->yoffs, texnum = 0;
	for (int ybit = 0; ybit < 30; ybit++) {
		const int h = 1 << ybit;
		for (int j = 0; j < ypartbits[ybit]; j++) {
			int xoffs = this->xoffs;
			for (int xbit = 0; xbit < 30; xbit++) {
				const int w = 1 << xbit;
				for (int k = 0; k < xpartbits[xbit]; k++) {
					glBindTexture(GL_TEXTURE_2D, this->textures[texnum++]);
	
					float w_nudge = 0.5f / (float)w;
					float h_nudge = 0.5f / (float)h;

		       			glBegin(GL_QUADS);
					glTexCoord2f(w_nudge, h_nudge);
					glVertex3f((float)(xoffs    ), (float)(yoffs    ), 0.0f);
					glTexCoord2f(w_nudge, 1.0f - h_nudge);
					glVertex3f((float)(xoffs    ), (float)(yoffs + h) + 0.01f, 0.0f);
					glTexCoord2f(1.0f - w_nudge, 1.0f - h_nudge);
					glVertex3f((float)(xoffs + w) + 0.01f, (float)(yoffs + h) + 0.01f, 0.0f);
					glTexCoord2f(1.0f - w_nudge, h_nudge);
					glVertex3f((float)(xoffs + w) + 0.01f, (float)(yoffs    ), 0.0f);
					glEnd();
			
					xoffs += w;
				}
			}
			yoffs += h;
		}
	}
		
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void BackgroundHandler::end_effect() {}

#endif
