#ifndef _INTERFERENCEHEIGHTMAPHANDLER_H
#define _INTERFERENCEHEIGHTMAPHANDLER_H 1

#include "main/event.h"
#include "main/object.h"

class InterferenceHeightmapHandler : public Object 
{
public:
	InterferenceHeightmapHandler(MainLoop *ml, const char *title, const char *elem, Hashtable *attr);
	~InterferenceHeightmapHandler();

	void start_effect();
	void draw_scene(float progress);
	void end_effect();

protected:
	int xlod, ylod;
	float speedlin, freqlinx, freqliny, ampllin;
	float speedcirc, freqcirc, amplcirc;
	
	inline float hfunc(float xf, float yf, float u1, float u2, float u3, float u4, float progress);
};

#endif /* !defined(_INTERFERENCEHEIGHTMAPHANDLER_H) */
