/*
 * Loops a few images fullscreen. Since we often want to use this for noise
 * etc., which often has large blanks areas, we also have an "optimize="
 * parameter, splitting the image into smaller horizontal or vertical textures
 * (set optimize=horizontal or optimize=vertical), throwing away the blank
 * ones. NOTE: for speed, our definition of "blank" is "the first 16x4 pixels
 * are totally transparent" (4x16 for optimize=vertical). Needless to say,
 * optimize= as it is today is quite bad for more complex overlays, but it's
 * to speed up Amoeba anyway (full 512x512 textures eats fillrate and perhaps
 * even more important, texture RAM) -- we'll make something more complex
 * later :-)
 *
 * The splitting is quite dumb (just split the image into 16-pixel lumps and
 * check for "emptiness"), but it should be quick, and as soon you remove the
 * "hey, let's destroy the framerate by loading 25MB of noise textures" problem
 * it all doesn't matter anyhow. :-) Optimized animation frames must be a
 * multiple of 16 pixels the "optimized" way, and all textures must be of equal
 * size then.
 *
 * Unfortunately, we get a "kink" if we free all the small textures at once --
 * therefore, we introduce delete=, which is kind of a hack... If it's over 0,
 * we delete a texture after drawing it. If you try to use a texture/frame that
 * is deleted, you'll get an exception :-) This can be quite tricky to get
 * right, unfortunately, and it doesn't help that much -- but it helps. An
 * alternative would simply be never clearing it up, which isn't really that
 * bad :-)
 */

#include <stdio.h>
#include <string.h>
#include <math.h>

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

#include "main/overlayanimhandler.h"
#include "exception.h"
#include "demolib_prefs.h"
#include "opengl/texture.h"

#if DEMOLIB_MAINLOOP 

OverlayAnimHandler::OverlayAnimHandler(MainLoop *ml, const char *title, const char *elem, Hashtable *attr) :
	Event(ml, title, elem, attr, "num:alpha:delete")
{
	const char *imgbase = attr->get_str("imgbase");
	const char *imgsuffix = attr->get_str("imgsuffix");
	this->num_frames = attr->get_int("num");

	if (attr->exists("optimize")) {
		this->optimize = true;
		char *optimize_dir = attr->get_str("optimize");
		if (strcmp(optimize_dir, "vertical") == 0) {
			this->optimize_vertical = true;
		} else if (strcmp(optimize_dir, "horizontal") == 0) {
			this->optimize_vertical = false;
		} else {
			throw new FatalException(elem, "optimize= must be `horizontal' or `vertical'!");
		}
		this->frames = new Array<struct picsegment>[this->num_frames];
	}

        GLint maxsize;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxsize);
	
	char filename[256];
        for (int i = 0; i < num_frames; i++) {
                sprintf(filename, "%s%02d%s", imgbase, i, imgsuffix);
		if (optimize) {
			Image *img = load_image(filename);
			if (i == 0) {
				this->xsize = img->get_width();
				this->ysize = img->get_height();

				if ((this->xsize % 16 != 0) ||
				    (this->ysize % 16 != 0)) {
					throw new FatalException(filename, "Dimensions must be a multiple of 16!");
				}
			} else {
				if (img->get_width() != xsize ||
				    img->get_height() != ysize) {
					throw new FatalException(filename, "All frames must have equal dimensions!");
				}
			}
			if (img->get_bpp() != 16 && img->get_bpp() != 32) {
				throw new FatalException(filename, "Must be 16bpp or 32bpp!");
			}

			int bs = img->get_bpp() / 8;
			int bo = (img->get_bpp() == 32) ? 3 : 1;
			unsigned char *ptr = img->get_pixel_data();

			/* simplest to split the loop this way :-) (yes, I use goto. :-P) */
			if (this->optimize_vertical) {
				for (int y = 0; y < this->ysize; y += 16) {
					/* check if the first four rows are empty */
					for (int ly = y; ly < y + 4; ly++) {
						for (int lx = 0; lx < 16; lx++) {
							if (ptr[(ly * xsize + lx) * bs + bo] != 0x00) {
								goto nonempty_vert;
							}
						}
					}

					continue;
nonempty_vert:
					this->create_subtexture(img, &(frames[i]), y, maxsize);
				}
			} else {
				for (int x = 0; x < this->xsize; x += 16) {
					/* check if the first four rows are empty */
					for (int ly = 0; ly < 4; ly++) {
						for (int lx = x; lx < x + 16; lx++) {
							if (ptr[(ly * xsize + lx) * bs + bo] != 0x00) {
								goto nonempty_horiz;
							}
						}
					}

					continue;
nonempty_horiz:
					this->create_subtexture(img, &(frames[i]), x, maxsize);
				}
			}
			delete img;
		} else {
	                this->tex.add_end(texture::load(filename));
		}
        }
}
OverlayAnimHandler::~OverlayAnimHandler()
{
	if (this->optimize) {
		/* it seems to be slightly quicker to free many textures at once :-) */
		Array<unsigned int> texnums;
	        for (int i = 0; i < this->num_frames; i++) {
			for (int j = 0; j < this->frames[i].num_elems(); j++) {
				if (!this->frames[i][j].deleted) {
					texnums.add_end(this->frames[i][j].texnum);
				}
			}
		}
		if (texnums.num_elems() > 0) {
			glDeleteTextures(texnums.num_elems(), texnums.get_array());
		}
		delete[] this->frames;
	} else {
	        for (int i = 0; i < this->num_frames; i++) {
			texture::free(this->tex[i]);
			this->tex[i] = NULL;
	        }
	}
}
	
void OverlayAnimHandler::start_effect()
{
}

void OverlayAnimHandler::draw_scene(float progress)
{
	int num = (int)(floor(this->get_val("num", progress))) % this->num_frames;
	
	float alpha = this->get_val("alpha", progress);
	if (alpha <= 0.0f) return;

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
	glLoadIdentity();

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();

	if (this->optimize) {
		glOrtho(0.0f, (float)(this->xsize), (float)(this->ysize), 0.0f, 0.0f, 1.0f);
	} else {
		glOrtho(0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f);
	}
        glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
        glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glColor4f(1.0f, 1.0f, 1.0f, alpha);

	if (this->optimize) {
		bool delete_now = (this->get_val("delete", progress) >= 0.0f);
			
		for (int i = 0; i < this->frames[num].num_elems(); i++) {
			if (this->frames[num][i].deleted) {
				throw new FatalException("Overlay frame already deleted!");
			}
			
			glBindTexture(GL_TEXTURE_2D, this->frames[num][i].texnum);
			float xstart, ystart, xend, yend;
			
			if (this->optimize_vertical) {
				xstart = 0.0f;
				xend = (float)(this->xsize);
				ystart = (float)(this->frames[num][i].pos);
				yend = ystart + 16.0f;
			} else {
				xstart = (float)(this->frames[num][i].pos);
				xend = xstart + 16.0f;
				ystart = 0.0f;
				yend = (float)(this->ysize);
			}
		
	        	glBegin(GL_QUADS);
	
 		        glTexCoord2f(0.0f, 0.0f);
		        glVertex3f(xstart, ystart, 0.0f);
	
		        glTexCoord2f(0.0f, 1.0f);
		        glVertex3f(xstart, yend, 0.0f);
	
		        glTexCoord2f(1.0f, 1.0f);
		        glVertex3f(xend, yend, 0.0f);
		
		        glTexCoord2f(1.0f, 0.0f);
		        glVertex3f(xend, ystart, 0.0f);
	
		        glEnd();
		}

		if (delete_now) {
			/*
			 * it seems to be slightly quicker to free many textures at once :-)
			 * (don't delete the current frame, we might need to show it again :-) )
			 */
			Array<unsigned int> texnums;
			for (int j = 0; j < num; j++) {
				for (int k = 0; k < this->frames[j].num_elems(); k++) {
					if (this->frames[j][k].deleted) continue;

					this->frames[j][k].deleted = true;
					texnums.add_end(this->frames[j][k].texnum);
				}
			}
			if (texnums.num_elems() > 0) {
				glDeleteTextures(texnums.num_elems(), texnums.get_array());
			}
		}
	} else {
		this->tex[num]->bind();

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	        glBegin(GL_QUADS);
	
	        glTexCoord2f(0.0f, 0.0f);
	        glVertex3f(0.0f, 0.0f, 0.0f);
	
	        glTexCoord2f(0.0f, 1.0f);
	        glVertex3f(0.0f, 1.0f, 0.0f);
	
	        glTexCoord2f(1.0f, 1.0f);
	        glVertex3f(1.0f, 1.0f, 0.0f);
	
	        glTexCoord2f(1.0f, 0.0f);
	        glVertex3f(1.0f, 0.0f, 0.0f);
	
	        glEnd();

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void OverlayAnimHandler::end_effect() {}

void OverlayAnimHandler::create_subtexture(Image *img, Array<struct picsegment> *frame,
		int pos, int maxsize)
{
	int xsize = img->get_width();
	int ysize = img->get_height();
	int bs = img->get_bpp() / 8;

	GLenum fmt = texture::get_opengl_format(img->get_bpp());
	GLenum ifmt = texture::get_opengl_internal_format(img->get_bpp());
	
	struct picsegment ps;
	ps.pos = pos;
	ps.deleted = false;
	
	glGenTextures(1, &(ps.texnum));
	glBindTexture(GL_TEXTURE_2D, ps.texnum);
	
	/* don't care about Voodoo1/2/3 not handling large enough textures ;-) */
	unsigned char *ptr = img->get_pixel_data();
	if (this->optimize_vertical) {
		glTexImage2D(GL_TEXTURE_2D, 0, ifmt, xsize, 16, 0, fmt, GL_UNSIGNED_BYTE,
			ptr + (pos * xsize * bs));
	} else {
		/* instead of messing with strides etc., we just create a new copy here :-) */
		unsigned char *tmpbuf = (unsigned char *)(malloc(16 * ysize * bs));
		if (tmpbuf == NULL)
			throw new FatalException("create_subtexture", "Out of memory!");

		for (int y = 0; y < ysize; y++) {
			memcpy(tmpbuf + (16 * y * bs), ptr + ((xsize * y + pos) * bs), 16 * bs);
		}
		
		glTexImage2D(GL_TEXTURE_2D, 0, ifmt, 16, ysize, 0, fmt, GL_UNSIGNED_BYTE,
			tmpbuf);
		free(tmpbuf);
	}
	
	/* no mipmapping, save memory and hope we don't have to minify :-) */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	frame->add_end(ps);
}

#endif
