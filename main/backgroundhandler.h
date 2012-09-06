#ifndef _BACKGROUNDHANDLER_H
#define _BACKGROUNDHANDLER_H

#define GL_TEXTURE_RECTANGLE_NV           0x84F5

#include "main/event.h"
#include "image/image.h"

class BackgroundHandler : public Event {
public:
	BackgroundHandler(MainLoop *ml, const char *title, const char *elem, Hashtable *attr);
	~BackgroundHandler();

	void start_effect();
	void draw_scene(float progress);
	void end_effect();

protected:
	GLuint *textures;
	int xres, yres;

	int xpartbits[30], ypartbits[30];
	int xparts, yparts;

	int xoffs, yoffs;
	int screenwidth, screenheight;
};

#endif /* defined(_BACKGROUNDHANDLER_H) */
