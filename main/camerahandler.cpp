#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#ifdef WIN32
#include <windows.h>
#include <float.h>
#define isnan(x) _isnan(x)
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

#include "math/vector.h"

#include "exception.h"
#include "camerahandler.h"
#include "demolib_prefs.h"

#if DEMOLIB_MAINLOOP

CameraHandler::CameraHandler(MainLoop *ml, const char *title, const char *elem, Hashtable *attr) :
	Event(ml, title, elem, attr, "x:y:z:lx:ly:lz:zrot")
{
	this->gravity = attr->get_float("gravity");
	this->layer = -60.0f;
}

CameraHandler::~CameraHandler()
{
}

void CameraHandler::start_effect() { }

#define DELTA 0.01f
#define MULTDELTA 100.0f

void CameraHandler::draw_scene(float progress)
{
	/* get our position */
	Vector pos(this->get_val("x", progress),
		   this->get_val("y", progress),
		   this->get_val("z", progress));
	
	/* determine two speeds, and thus get the acceleration */
	Vector pos1(this->get_val("x", progress + DELTA),
		    this->get_val("y", progress + DELTA),
		    this->get_val("z", progress + DELTA));
	Vector sp1 = (pos1 - pos) * MULTDELTA;
	
	Vector pos2(this->get_val("x", progress + DELTA * 2.0f),
		    this->get_val("y", progress + DELTA * 2.0f),
		    this->get_val("z", progress + DELTA * 2.0f));
	Vector sp2 = (pos2 - pos1) * MULTDELTA;

	Vector acc = (sp2 - sp1) * MULTDELTA;

	/*
	 * find the "left" vector, relative to the absolute up position,
	 * and the forward component
	 */
	Vector up(0.0f, 1.0f, 0.0f);
	Vector left = sp1.cross_product(up).normalize();

	/*
	 * find the acceleration in the "left" direction
	 */
	float acc_proj = acc * left;

	/*
	 * find the rotation by a/g
	 */
	float camrot = acos(acc_proj / this->gravity) - M_PI / 2.0f;

	if (left.x < 0.0f) {
		if (camrot < 0.0f) camrot = -camrot;
	} else {
		if (camrot > 0.0f) camrot = -camrot;
	}

	const float x = pos.x;
	const float y = pos.y;
	const float z = pos.z;
	const float lx = this->get_val("lx", progress);
	const float ly = this->get_val("ly", progress);
	const float lz = this->get_val("lz", progress);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	if (camrot > -M_PI && camrot < M_PI && !isnan(camrot)) {
		gluLookAt(x, y, z, lx, ly, lz, sin(camrot), cos(camrot), 0.0f);
	} else {
		gluLookAt(x, y, z, lx, ly, lz, 0.0f, 1.0f, 0.0f);
	}
	glTranslatef(x, y, z);
	glRotatef(this->get_val("zrot", progress), lx - x, ly - y, lz - z);
	glTranslatef(-x, -y, -z);
}

void CameraHandler::end_effect() {}

#endif
