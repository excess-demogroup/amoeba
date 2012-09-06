#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846264
#endif

#include "main/shadowrecthandler.h"
#include "exception.h"
#include "demolib_prefs.h"

#if DEMOLIB_MAINLOOP

ShadowRectHandler::ShadowRectHandler(MainLoop *ml, const char *title, const char *elem, Hashtable *attr) :
	Event(ml, title, elem, attr, "alpha")
{
}

ShadowRectHandler::~ShadowRectHandler()
{
}

void ShadowRectHandler::start_effect()
{
}
void ShadowRectHandler::draw_scene(float progress)
{
	const float alpha = this->get_val("alpha", progress);

	if (alpha == 1.0f) {
		glDisable(GL_BLEND);
	} else {
		glColor4f(0.0f, 0.0f, 0.0f, alpha);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
	}
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_NOTEQUAL, 0, 0xFFFFFFFFL);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_TEXTURE_2D);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 10.0f);

	glBegin(GL_QUADS);
	glVertex3f(0.0f, 1.0f, 0.0f);
	glVertex3f(1.0f, 1.0f, 0.0f);
	glVertex3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glEnd();
	
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glDisable(GL_STENCIL_TEST);
}
void ShadowRectHandler::end_effect()
{
}

#endif
