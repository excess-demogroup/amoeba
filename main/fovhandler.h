#ifndef _FOVHANDLER_H
#define _FOVHANDLER_H

#include "main/event.h"
#include "math/vector.h"
#include "opengl/texture.h"

class FOVHandler : public Event {
public:
	FOVHandler(MainLoop *ml, const char *title, const char *elem, Hashtable *attr);
	~FOVHandler();

	void start_effect();
	void draw_scene(float progress);
	void end_effect();
};

#endif /* defined(_FOVHANDLER_H) */
