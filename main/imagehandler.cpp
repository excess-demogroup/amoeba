#include <string.h>

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

#include "main/imagehandler.h"
#include "../exception.h"
#include "../demolib_prefs.h"
#include "opengl/texture.h"

#if DEMOLIB_MAINLOOP 

ImageHandler::ImageHandler(MainLoop *ml, const char *title, const char *elem, Hashtable *attr) :
	Event(ml, title, elem, attr, "xcsize:ycsize:xpos:ypos:xsize:ysize:alpha")
{
	this->tex = texture::load(attr->get_str("file"));
}
ImageHandler::~ImageHandler()
{
	texture::free(this->tex);
	this->tex = NULL;
}

void ImageHandler::start_effect()
{
}

void ImageHandler::draw_scene(float progress)
{
	float alpha = this->get_val("alpha", progress);
	if (alpha <= 0.0f) return;

	float xcsize = this->get_val("xcsize", progress);
	float ycsize = this->get_val("ycsize", progress);
	float xpos = this->get_val("xpos", progress) - xcsize * 0.5f;
	float ypos = this->get_val("ypos", progress) - ycsize * 0.5f;
	float xsize = this->get_val("xsize", progress) + xcsize;
	float ysize = this->get_val("ysize", progress) + ycsize;

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
	glLoadIdentity();

	glScalef(DEMOLIB_YASPECT / DEMOLIB_XASPECT, 1.0f, 1.0f);

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();

	glOrtho(0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_STENCIL_TEST);
	
	this->tex->bind();
        glEnable(GL_TEXTURE_2D);

        glColor4f(1.0f, 1.0f, 1.0f, alpha);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBegin(GL_QUADS);

        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(xpos, ypos, 0.0f);

        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(xpos, ypos + ysize, 0.0f);

        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(xpos + xsize, ypos + ysize, 0.0f);

        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(xpos + xsize, ypos, 0.0f);

        glEnd();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void ImageHandler::end_effect() {}

#endif
