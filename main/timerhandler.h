#ifndef _TIMERHANDLER_H
#define _TIMERHANDLER_H

#include "main/event.h"
#include "main/mainloop.h"
#include "opengl/texture.h"

class TimerHandler : public Event {
public:
	TimerHandler(MainLoop *ml, const char *title, const char *elem, Hashtable *attr);
	~TimerHandler();

	void start_effect();
	void draw_scene(float progress);
	void end_effect();

protected:
	Texture *digits;
};

#endif /* defined(_TIMERHANDLER_H) */
