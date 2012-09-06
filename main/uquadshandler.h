#ifndef _UPLANESHANDLER_H
#define _UPLANESHANDLER_H 1

#include "main/event.h"
#include "main/mainloop.h"
#include "main/objhandler.h"
#include "math/array.h"

class UQuadsHandler : public Object {
protected:
	int xnum, ynum, znum;
	float xspace, yspace, zspace;
	float xstrength, ystrength, zstrength;
	
public:
	UQuadsHandler(MainLoop *ml, const char *title, const char *elem, Hashtable *attr);
	~UQuadsHandler();
	
	void start_effect();
	void draw_scene(float progress);
	void end_effect();
};

#endif /* !defined(_UPLANESHANDLER_H) */
