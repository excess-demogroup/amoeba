#ifndef _CAMERAHANDLER_H
#define _CAMERAHANDLER_H 1

#include "main/event.h"

class CameraHandler : public Event {
protected:
	float gravity;

public:
	CameraHandler(MainLoop *ml, const char *title, const char *elem, Hashtable *attr);
	~CameraHandler();
	
	void start_effect();
	void draw_scene(float progress);
	void end_effect();
};

#endif /* !defined(_CAMERAHANDLER_H) */
