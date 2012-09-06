#ifndef _FPSHANDLER_H
#define _FPSHANDLER_H

#include "main/event.h"
#include "main/mainloop.h"
#include "opengl/fpscount.h"

class FPSHandler : public Event {
public:
	FPSHandler(MainLoop *ml, const char *title, const char *elem, Hashtable *attr);
	~FPSHandler();

	void start_effect();
	void draw_scene(float progress);
	void end_effect();

protected:
	bool active;

	FPSCounter *fps;
};

#endif /* defined(_FPSHANDLER_H) */
