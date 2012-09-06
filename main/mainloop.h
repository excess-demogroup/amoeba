#ifndef _MAINLOOP_H
#define _MAINLOOP_H

#if WIN32
#include <windows.h>
#endif

#include <expat.h>

#include "main/event.h"
#include "main/factory.h"
#include "main/piprecalc.h"
#include "packer/file.h"
#include "opengl/glwindow.h"
#include "util/hashtable.h"

class Factory;
class MusicHandler;
class DirectSoundAudioDriver;

class MainLoop {
public:
	int next_evnum; 
	
	MainLoop(int argc, char **argv);
	~MainLoop();

	void parse(File *demoscript);
	
	void add_handler(Factory *handler_factory);
	void process_element(const char *el, const char **attr);

	float get_time();
	
	void run();
	void run(bool infloop);

	/* ick */
	Event **events;
	int num_events;
	
	Event *curr_event;
	
	Event *evlist[256];
	int num_play_events;
	
	XML_Parser p;

	friend class MusicHandler;
	friend class DirectSoundAudioDriver;
	
protected:
	void parse_commandline(int argc, char **argv, Hashtable *attr_hash);

	Factory **factories;	
	int num_factories;
	
	Hashtable *markers;
	MusicHandler *timer;

	int argc;
	char **argv;

	GLWindow *win;
	bool sound;

	PiPrecalc *precalc;

#ifdef WIN32
	LARGE_INTEGER tmstart, tmfreq;
#else
	struct timeval tmstart;
#endif
};

#endif /* defined(_MAINLOOP_H) */
