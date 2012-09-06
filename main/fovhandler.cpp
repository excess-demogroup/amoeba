#include <string.h>
#include <math.h>
#include <stdlib.h>

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

#include "main/fovhandler.h"
#include "exception.h"
#include "demolib_prefs.h"

#ifndef M_PI
#define M_PI 3.141592653589793238462643383279502
#endif

#if DEMOLIB_MAINLOOP 

FOVHandler::FOVHandler(MainLoop *ml, const char *title, const char *elem, Hashtable *attr) :
		Event(ml, title, elem, attr, "fov")
{
	this->layer = -900.0f;
}

FOVHandler::~FOVHandler()
{
}

void FOVHandler::start_effect() {}

void FOVHandler::draw_scene(float progress)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
        gluPerspective(this->get_val("fov", progress),
		DEMOLIB_XASPECT / DEMOLIB_YASPECT, 1.0f, 500.0f);
	glMatrixMode(GL_MODELVIEW);
}

/* reset FOV at end */
void FOVHandler::end_effect()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
        gluPerspective(53.0f, DEMOLIB_XASPECT / DEMOLIB_YASPECT, 1.0f, 500.0f);
	glMatrixMode(GL_MODELVIEW);
}

#endif
