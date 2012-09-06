#ifndef _MUSICHANDLER_H
#define _MUSICHANDLER_H 1

#include "audio/audio.h"
#include "main/event.h"

class MusicBar;

class MusicHandler : public Event
{
public:
	MusicHandler(MainLoop *ml, const char *title, const char *elem, Hashtable *attr);
	~MusicHandler();

	void start_effect();
	void draw_scene(float progress);
	void end_effect();

	float get_time();
	
protected:
	File *musicfile;
	AudioProvider *prov;
	AudioDriver *drv;

	float jump;

	friend class MusicBar;
};

#endif /* defined(_OGGHANDLER_H) */
