#ifndef _SHADOWHANDLER_H
#define _SHADOWHANDLER_H 1

#include "main/event.h"
#include "math/vector.h"
#include "main/mainloop.h"
#include "packer/file.h"
#include "math/array.h"
#include "main/objhandler.h"

class ShadowHandler : public ObjHandler 
{
public:
	ShadowHandler(MainLoop *ml, const char *title, const char *elem, Hashtable *attr);
	~ShadowHandler();

	void start_effect();
	void draw_scene(float progress);
	void end_effect();

protected:
	struct extravertexinfo {
		float a, b, c, d;
		int neighbours[3];
		bool visible;
	};

	GLfloat *shadowvol;
	Array<extravertexinfo> evi;
	
	int do_shadow_pass(const float lx, const float ly, const float lz, GLfloat *shadowvol);
};

#endif /* !defined(_SHADOWHANDLER_H) */
