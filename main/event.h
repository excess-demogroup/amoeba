#ifndef _EVENT_H
#define _EVENT_H

#include "main/mainloop.h"
#include "util/hashtable.h"

class MainLoop;
class DemoHandler;
class DOFHandler;
class TimewalkHandler;

class Event {
public:
	Event(MainLoop *ml, const char *title, const char *elem, Hashtable *attr, const char *curvenames);
	virtual ~Event();
	
	/* these are typically provided by the base class */
	void add_curvepoint(Hashtable *markers, const char *element, const char **attr);
	void end_curvedata();
	
	float get_val(const char *attr_name, float progress);
	
	/* these are typically overloaded */
	virtual void start_effect();
	virtual void draw_scene(float progress);
	virtual void end_effect();

	static float parse_time(Hashtable *markers, const char *timestr);
	
	friend class MainLoop;
	friend class DemoHandler;
	friend class DOFHandler;
	friend class TimewalkHandler;
	
	/* sigh */
	float start, stop;
	float layer;

protected:
	Hashtable *curves;
	char *title;
	bool active;

	MainLoop *ml;
};

#endif /* defined(_EVENT_H) */
