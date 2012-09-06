#ifndef _HEIGHTMAPTUNNELHANDLER_H
#define _HEIGHTMAPTUNNELHANDLER_H 1

#include "main/event.h"
#include "main/object.h"

class HeightmapTunnelHandler : public Object 
{
public:
	HeightmapTunnelHandler(MainLoop *ml, const char *title, const char *elem, Hashtable *attr);
	~HeightmapTunnelHandler();

	void start_effect();
	void draw_scene(float progress);
	void end_effect();

protected:
	float zspeed, zlength;
	
	int xlod, ylod;
	float speedlin, freqlinx, freqliny, ampllin;
	float speedcirc, freqcirc, amplcirc;
	
	inline float hfunc(float xf, float yf, float u1, float u2, float u3, float u4, float progress);
};

#endif /* !defined(_HEIGHTMAPTUNNELHANDLER_H) */
