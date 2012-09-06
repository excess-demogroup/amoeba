#ifndef _SHADOWRECTHANDLER_H
#define _SHADOWRECTHANDLER_H 1

#include "main/event.h"
#include "main/mainloop.h"

class ShadowRectHandler : public Event 
{
public:
	ShadowRectHandler(MainLoop *ml, const char *title, const char *elem, Hashtable *attr);
	~ShadowRectHandler();

	void start_effect();
	void draw_scene(float progress);
	void end_effect();
};

#endif /* !defined(_SHADOWRECTHANDLER_H) */
