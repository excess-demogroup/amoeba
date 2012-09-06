#ifndef _FOGHANDLER_H
#define _FOGHANDLER_H

#include "main/event.h"
#include "math/vector.h"
#include "opengl/texture.h"

class FogHandler : public Event {
public:
	FogHandler(MainLoop *ml, const char *title, const char *elem, Hashtable *attr);
	~FogHandler();

	void start_effect();
	void draw_scene(float progress);
	void end_effect();
};

#endif /* defined(_FOGHANDLER_H) */
