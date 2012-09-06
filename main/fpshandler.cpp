#include <string.h>

#include "main/fpshandler.h"
#include "../exception.h"
#include "../demolib_prefs.h"

#if DEMOLIB_MAINLOOP 

FPSHandler::FPSHandler(MainLoop *ml, const char *title, const char *elem, Hashtable *attr) :
	Event(ml, title, elem, attr, NULL)
{
	this->fps = new FPSCounter();
}

FPSHandler::~FPSHandler()
{
	delete this->fps;
	this->fps = NULL;
}

void FPSHandler::start_effect()
{
	this->fps->reset();
}

void FPSHandler::draw_scene(float progress) {
	fps->draw();
}
void FPSHandler::end_effect() {}

#endif
