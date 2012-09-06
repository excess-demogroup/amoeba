#ifndef _INVERTHANDLER_H
#define _INVERTHANDLER_H

#include "main/event.h"
#include "opengl/texture.h"

class InvertHandler : public Event {
public:
	InvertHandler(MainLoop *ml, const char *title, const char *elem, Hashtable *attr);
	~InvertHandler();

	void start_effect();
	void draw_scene(float progress);
	void end_effect();

protected:
	Texture *tex;
};

#endif /* defined(_INVERTHANDLER_H) */
