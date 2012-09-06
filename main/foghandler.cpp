#include <string.h>
#include <math.h>
#include <stdlib.h>

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

#include "main/foghandler.h"
#include "exception.h"
#include "demolib_prefs.h"

#ifndef M_PI
#define M_PI 3.141592653589793238462643383279502
#endif

#if DEMOLIB_MAINLOOP 

FogHandler::FogHandler(MainLoop *ml, const char *title, const char *elem, Hashtable *attr) :
		Event(ml, title, elem, attr, "start:end:r:g:b:a")
{
}

FogHandler::~FogHandler()
{
}

void FogHandler::start_effect()
{
	glEnable(GL_FOG);
	glHint(GL_FOG_HINT, GL_FASTEST);
	glFogi(GL_FOG_MODE, GL_LINEAR);
}

void FogHandler::draw_scene(float progress)
{
	GLfloat fog_color[] = {
		this->get_val("r", progress),
		this->get_val("g", progress),
		this->get_val("b", progress),
		this->get_val("a", progress)
	};
	glFogf(GL_FOG_START, this->get_val("start", progress));
	glFogf(GL_FOG_END,   this->get_val("end", progress));
	glFogfv(GL_FOG_COLOR, fog_color);
}

void FogHandler::end_effect()
{
	glDisable(GL_FOG);
}

#endif
