#include <string.h>
#include <stdlib.h>

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>

#include "main/lighthandler.h"
#include "exception.h"
#include "demolib_prefs.h"

#if DEMOLIB_MAINLOOP 

LightHandler::LightHandler(MainLoop *ml, const char *title, const char *elem, Hashtable *attr) :
	Event(ml, title, elem, attr, "ambr:ambg:ambb:difr:difg:difb:specr:specg:specb:x:y:z:catt:latt:qatt")
{
	this->positional = (strcmp(elem, "poslight") == 0);
	
	this->light_num = GL_LIGHT0 + attr->get_int("num");
	this->layer = -50.0f;

	/* okay to set here, I suppose -- still a hack, though :-) */
	float zero_light[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, zero_light);
}

LightHandler::~LightHandler()
{
}

void LightHandler::start_effect()
{
	glEnable(light_num);	
}

void LightHandler::draw_scene(float progress)
{
	GLfloat ambient[] = {
		this->get_val("ambr", progress),
		this->get_val("ambg", progress),
		this->get_val("ambb", progress),
		1.0f
	};
	GLfloat diffuse[] = {
		this->get_val("difr", progress),
		this->get_val("difg", progress),
		this->get_val("difb", progress),
		1.0f
	};
	GLfloat specular[] = {
		this->get_val("specr", progress),
		this->get_val("specg", progress),
		this->get_val("specb", progress),
		1.0f
	};
	GLfloat position[] = {
		this->get_val("x", progress),
		this->get_val("y", progress),
		this->get_val("z", progress),
		positional ? 1.0f : 0.0f
	};
	GLfloat catt[] = { this->get_val("catt", progress) };
	GLfloat latt[] = { this->get_val("latt", progress) };
	GLfloat qatt[] = { this->get_val("qatt", progress) };

	glLightfv(light_num, GL_AMBIENT, ambient);
	glLightfv(light_num, GL_DIFFUSE, diffuse);
	glLightfv(light_num, GL_SPECULAR, specular);
	glLightfv(light_num, GL_POSITION, position);
	glLightfv(light_num, GL_CONSTANT_ATTENUATION, catt);
	glLightfv(light_num, GL_LINEAR_ATTENUATION, latt);
	glLightfv(light_num, GL_QUADRATIC_ATTENUATION, qatt);
}
void LightHandler::end_effect()
{
	glDisable(light_num);
}

#endif
