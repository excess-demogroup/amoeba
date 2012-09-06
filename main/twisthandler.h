#ifndef _TWISTHANDLER_H
#define _TWISTHANDLER_H 1

#include "main/event.h"
#include "main/mainloop.h"
#include "main/objhandler.h"
#include "math/array.h"

class TwistHandler : public ObjHandler {
protected:
	Array<vertex> orig_vertices;
	vertex *atan_cache;
	int *vertex_alias;

	bool normalspike;
	struct vertex *spike_vert;
	struct texcoord *spike_tc;
	Texture *normalspiketex;
	
public:
	TwistHandler(MainLoop *ml, const char *title, const char *elem, Hashtable *attr);
	~TwistHandler();
	
	void start_effect();
	void draw_scene(float progress);
	void end_effect();
};

#endif /* !defined(_TWISTHANDLER_H) */
