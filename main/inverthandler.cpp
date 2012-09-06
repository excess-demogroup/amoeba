#include <stdio.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

#include "main/inverthandler.h"
#include "exception.h"
#include "demolib_prefs.h"
#include "opengl/texture.h"

#if DEMOLIB_MAINLOOP 

InvertHandler::InvertHandler(MainLoop *ml, const char *title, const char *elem, Hashtable *attr) :
	Event(ml, title, elem, attr, "alpha")
{
}
InvertHandler::~InvertHandler()
{
}

void InvertHandler::start_effect()
{
}

void InvertHandler::draw_scene(float progress)
{
	float alpha = this->get_val("alpha", progress);
	if (alpha <= 0.0f) return;

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
	glLoadIdentity();

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();

	glOrtho(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR);
	
	glColor4f(alpha, alpha, alpha, 1.0f);
	
        glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_TEXTURE_2D);

        glBegin(GL_QUADS);

	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);
	glVertex3f(1.0f, 1.0f, 0.0f);
	glVertex3f(1.0f, 0.0f, 0.0f);

        glEnd();
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void InvertHandler::end_effect() {}

#endif
