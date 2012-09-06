#ifndef _IMAGEHANDLER_H
#define _IMAGEHANDLER_H

#include "main/event.h"
#include "opengl/texture.h"

class ImageHandler : public Event {
public:
	ImageHandler(MainLoop *ml, const char *title, const char *elem, Hashtable *attr);
	~ImageHandler();

	void start_effect();
	void draw_scene(float progress);
	void end_effect();

protected:
	Texture *tex;
};

#endif /* defined(_IMAGEHANDLER_H) */
