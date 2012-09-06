#ifndef _LIGHTHANDLER_H
#define _LIGHTHANDLER_H

#include "main/event.h"
#include "main/mainloop.h"

class LightHandler : public Event {
public:
	LightHandler(MainLoop *ml, const char *title, const char *elem, Hashtable *attr);
	~LightHandler();

	void start_effect();
	void draw_scene(float progress);
	void end_effect();

protected:
	bool positional;
	int light_num;
};

#endif /* defined(_LIGHTHANDLER_H) */
