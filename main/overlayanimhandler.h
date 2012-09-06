#ifndef _OVERLAYANIMHANDLER_H
#define _OVERLAYANIMHANDLER_H

#include "main/event.h"
#include "math/array.h"
#include "opengl/texture.h"

struct picsegment {
	int pos;
	unsigned int texnum;
	bool deleted;
};

class OverlayAnimHandler : public Event {
public:
	OverlayAnimHandler(MainLoop *ml, const char *title, const char *elem, Hashtable *attr);
	~OverlayAnimHandler();

	void start_effect();
	void draw_scene(float progress);
	void end_effect();

	void create_subtexture(Image *img, Array<struct picsegment> *frame, int pos, int maxsize);

protected:
	int num_frames;
	
	/* for unoptimized overlay anims */
	Array<Texture *> tex;
	
	/* for optimized ones */
	bool optimize;
	bool optimize_vertical;
	Array<struct picsegment> *frames;
	int xsize, ysize;
};

#endif /* defined(_OVERLAYANIMHANDLER_H) */
