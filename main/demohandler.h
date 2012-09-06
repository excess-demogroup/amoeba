#ifndef _DEMOHANDLER_H
#define _DEMOHANDLER_H

#include "main/event.h"
#include "main/mainloop.h"
#include "opengl/glwindow.h"

class DemoHandler : public Event {
public:
	DemoHandler(MainLoop *ml, const char *title, const char *elem, Hashtable *attr);
	~DemoHandler();

	bool can_handle(char *elem);
	
	void start_effect();
	void draw_scene(float progress);
	void end_effect();

	MainLoop *ml;

protected:
	GLWindow *win;

	friend class MainLoop;
};

#endif /* defined(_DEMOHANDLER_H) */
