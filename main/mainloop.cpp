#include <stdio.h>
#include <string.h>
#include <math.h>
#ifdef __GNUC__
#include <unistd.h>
#endif
#include <expat.h>

#ifdef WIN32
#include <windows.h>
#include "main/win32-config/win32-config.h"
#else
#include <sys/time.h>
#include "main/linux-config/linux-config.h"
#endif

#include "main/mainloop.h"
#include "main/factory.h"
#include "main/demohandler.h"
#include "audio/musichandler.h"
#include "exception.h"
#include "demolib_prefs.h"

#include <GL/gl.h>

#if DEMOLIB_MAINLOOP

void start_ml(void *data, const char *el, const char **attr)
{
	MainLoop *ml = (MainLoop *)data;

	ml->process_element(el, attr);		
}
void end_ml(void *data, const char *el)
{
	MainLoop *ml = (MainLoop *)data;

	/* this is a brief hack, but it's OK */
	if (strcmp(el, "demo") == 0) {
		if (ml->events[0]->stop == -1.0f)
			ml->events[0]->stop = ml->events[ml->num_events - 1]->stop;
		return;
	}

	if (strcmp(el, "start") == 0 ||
	    strcmp(el, "point") == 0 ||
	    strcmp(el, "end") == 0 ||
	    strcmp(el, "demo") == 0 ||
	    strcmp(el, "music") == 0 ||
	    strcmp(el, "marker") == 0 ||
	    strcmp(el, "lpp") == 0)
		return;
	try {
		ml->curr_event->end_curvedata();
		ml->curr_event = NULL;
	} catch (FatalException *e) {
		char buf[256];
                sprintf(buf, "Error at line %d of demo script: %s", (int)XML_GetCurrentLineNumber(ml->p), e->get_error());
                throw new FatalException(buf);
	}
}

MainLoop::MainLoop(int argc, char **argv)
{
	this->next_evnum = 0;

	this->events = (Event **)(malloc(sizeof(Event *) * 256));
	this->num_events = 0;

	this->factories = (Factory **)(malloc(sizeof(Factory *) * 256));
	this->num_factories = 0;

	this->markers = new Hashtable();
	this->timer = NULL;

	this->argc = argc;
	this->argv = argv;

	this->sound = true;
	this->precalc = NULL;
}

void MainLoop::parse(File *demoscript)
{
	this->p = XML_ParserCreate(NULL);
	XML_SetUserData(p, this);
	XML_SetElementHandler(p, start_ml, end_ml);

	bool status;
	
	try {
		status = XML_Parse(p, demoscript->get_data(), demoscript->data_length(), 1);
	} catch (ValueNotSpecifiedException *nve) {
		char buf[256];
		sprintf(buf, "Error at line %d of demo script: Parameter `%s=' missing", (int)XML_GetCurrentLineNumber(p), nve->get_error());
		throw new FatalException(buf);
	}
		
	if (!status) {
		char buf[512];
		sprintf(buf, "Parse error at line %d of demo script: %s",
			(int)XML_GetCurrentLineNumber(p),
			XML_ErrorString(XML_GetErrorCode(p)));
		throw new FatalException(buf);
	}

	XML_ParserFree(p);

	if (this->precalc != NULL)
		delete this->precalc;
}

MainLoop::~MainLoop()
{
	int i;
	for (i = 0; i < this->num_events; i++) {
		if (this->events[i] == NULL) continue;
#if !DEMOLIB_SILENT
		char *buf = strdup(this->events[i]->title);
#endif
		delete this->events[i];
#if !DEMOLIB_SILENT
		int err = glGetError();
		if (err != 0) {
			printf("Warning: Effect `%s' has OpenGL error 0x%x in exit code\n", buf, err);
		}
		free(buf);
#endif
		this->events[i] = NULL;
	}
	free(this->events);
	this->events = NULL;

	for (i = 0; i < this->num_factories; i++) {
		delete this->factories[i];
		this->factories[i] = NULL;
	}
	free(this->factories);
	this->factories = NULL;

	this->markers->destroy_values();
	delete this->markers;
	this->markers = NULL;
}

void MainLoop::add_handler(Factory *handler_factory)
{
	this->factories[num_factories++] = handler_factory;
}

void MainLoop::process_element(const char *el, const char **attr)
{
	char title[256] = "[No title]";

	/* check if this is a music marker */
	if (strcmp(el, "marker") == 0) {
		const char *name = NULL;
		float t = -1.0f;

		int i = 0;
		while (attr[i]) {
			if (strcmp(attr[i], "name") == 0) {
				name = attr[i + 1];
			}
			if (strcmp(attr[i], "time") == 0) {
				t = atof(attr[i + 1]);
			}
			i += 2;
		}

		if (name == NULL)
			throw new FatalException(el, "No name= attribute!");
		if (t == -1.0f)
			throw new FatalException(el, "No time= attribute!");

		/* sigh */
		float *tp = (float *)(malloc(sizeof(float)));
		if (tp == NULL)
			throw new FatalException("Out of memory!");
		
		*tp = t;
		this->markers->insert(name, (void *)tp);

		return;
	}

	/* check if this is an effect marker (start/end/point) */
	if (strcmp(el, "start") == 0 ||
	    strcmp(el, "point") == 0 ||
	    strcmp(el, "end") == 0) {
		if (this->curr_event == NULL) {
			throw new FatalException(el, "Time element outside effect elements!");
		} else {
			this->curr_event->add_curvepoint(this->markers, el, attr);
			return;
		}
	}

	/*
	 * Check if this is a loader progress point (lpp).
	 * This is used for two purposes:
	 *
	 * 1) Show a simple progress bar while loading. (For the
	 *    Underscore demo, this is replaced by a fancy Pi
	 *    display ;-) )
	 * 2) Timing loader parts, to be outputted and possibly
	 *    made into more exact p= values later. (This is
	 *    disabled by setting DEMOLIB_SILENT.)
	 */
	if (strcmp(el, "lpp") == 0) {
		float p = -1.0f;
		int i = 0;
#if !DEMOLIB_SILENT
		static bool init_timer = false;
#if __unix__
		static struct timeval first_lpp, now;
#else
		static DWORD first_lpp, now;
#endif
		
		if (!init_timer) {
#if __unix__
			gettimeofday(&first_lpp, NULL);
#else
			first_lpp = GetTickCount();
#endif
			init_timer = true;
		}
#if __unix__
		gettimeofday(&now, NULL);
		printf("LPP: [%6.3f]\n",
			(now.tv_sec - first_lpp.tv_sec) +
			(float)(now.tv_usec - first_lpp.tv_usec) * 0.000001f);
#else
		now = GetTickCount();
		printf("LPP: [%6.3f]\n",
			(float)(now - first_lpp) * 0.001f);
#endif

#endif /* !DEMOLIB_SILENT */
		
		while (attr[i]) {
			if (strcmp(attr[i], "p") == 0) {
				p = atof(attr[i + 1]);
				break;
			}
			i += 2;
		}
		if (p == -1.0f) return;
		
#if 0
		/* do we want this in a separate class? */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
	        glOrtho(0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f);
	        glDisable(GL_BLEND);
		glDisable(GL_LIGHTING);
		glColor3f(1.0f, 1.0f, 1.0f);

		glBegin(GL_QUADS);
		glVertex2f(0.0f, 0.9f);
		glVertex2f(p, 0.9f);
		glVertex2f(p, 1.0f);
		glVertex2f(0.0f, 1.0f);
		glEnd();

		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);

		this->win->flip();
#else
		if (this->precalc != NULL)
			this->precalc->update(p);		
#endif
		return;
	}

	/*
	 * If this is the music and we have -nosound, discard it.
	 */
#if WIN32
	if (strcmp(el, "music") == 0 &&
	    (!this->sound || Win32Config::instance()->get_sound() == (LPGUID)-1)) {
		return;
	}
#else
	if (strcmp(el, "music") == 0 && !this->sound) {
		return;
	}
#endif
	
	/*
	 * Find the title, and make a temporary hash table to hold all
	 * the attributes.
	 *
	 * C++ is very picky about const (which is good) -- we KNOW that the
	 * attribute strings will never be edited, but it's very hard to prove
	 * that to the computer programmatically :-)
	 */
	Hashtable *attr_hash = new Hashtable();
	int i = 0;
	while (attr[i]) {
		attr_hash->insert(attr[i], (char *)attr[i + 1]);
		if (strcmp(attr[i], "title") == 0) {
			strcpy(title, attr[i + 1]);
		}
		i += 2;
	}


	/* find a factory that can instantiate this type of event classes */
	Event *ev = NULL;
	for (i = 0; i < num_factories; i++) {
		Factory *f = this->factories[i];
		if (!f->can_handle(el)) continue;
		
		/* 
		 * If this is the first event, it's a DemoHandler, and we want
		 * to set window stuff etc. -- this isn't particularily clean,
		 * and should be rewritten someday.
		 */
		if (this->num_events == 0) {
#if __unix__
			/* attempt to use the colorful GTK+ interface first =) */
			try {
				/* 
				 * don't use the interface if there are command line
				 * options
				 */
				if (argc != 1) throw new NonFatalException("disabling because of command line options");
				
				LinuxConfig *lc = new LinuxConfig();
				lc->show(&argc, &argv, attr_hash);
				if (strcmp(attr_hash->get_str("sound"), "no") == 0) {
					this->sound = false;
				}
			} catch (NonFatalException *e) {
				fprintf(stderr,
					"Couldn't open GTK+ interface (%s), reverting to command line.\n",
					e->get_error());
				/* use the generic code, since GTK+ failed for some reason */
				this->parse_commandline(argc, argv, attr_hash);
			}
#else /* Win32 */
			if (argc != 1) {
				this->parse_commandline(argc, argv, attr_hash);
			} else {
				char buf[256];
				Win32Config *c = Win32Config::instance();
				c->dialog();
				if (c->get_fullscreen()) {
					attr_hash->insert("fullscreen", "yes");
				} else {
					attr_hash->insert("fullscreen", "no");
				}
				
				attr_hash->insert("xres", "0");
				attr_hash->insert("yres", "0");
				
				sprintf(buf, "%u", c->get_zbuffer());
				attr_hash->insert("zbuffer", buf);
				
				sprintf(buf, "%u", c->get_mode());
				attr_hash->insert("visual_id", buf);
			}
#endif
		}
		
		ev = f->instantiate(this, title, el, attr_hash);
#if !DEMOLIB_SILENT
	        int err = glGetError();
	        if (err != 0) {
	                printf("Warning: Initialization of `%s' has OpenGL error 0x%x\n", title, err);
	        }
#endif
				
		if (this->num_events == 0) {
			this->win = ((DemoHandler *)ev)->win;

			/* now that we have an OpenGL window, we can start the loader screen */
			if (!attr_hash->exists("noprecalcscreen"))
				this->precalc = new PiPrecalc(this->win);
		}
		break;
	}

	delete attr_hash;

	if (ev == NULL) {
		throw new FatalException(el, "No usable handler found!");
	}

	/*
	 * the only special tags ATM are <demo> and <music> -- they don't need end time
	 */
	if (strcmp(el, "demo") == 0 ||
	    strcmp(el, "music") == 0) {
		this->curr_event = NULL;
		ev->start = 0.0f;
		ev->stop = -1.0f;

		if (strcmp(el, "music") == 0) {
			/* music _can_ have a start time :-) */
			int i = 0;
			while (attr[i]) {
				if (strcmp(attr[i], "start") == 0) {
					ev->start = atof(attr[i + 1]);
					break;
				}
				i += 2;
			}
		}
	} else {
		if (this->curr_event != NULL) {
			throw new FatalException(el, "Nested effect elements aren't allowed.");
		}
		this->curr_event = ev;
	}


	/*
	 * ...and add the actual element (we could strip away start= etc., but we
	 * don't care -- it isn't worth it)
	 */
	this->events[this->num_events++] = ev;
}

void MainLoop::run()
{
#ifdef WIN32
	QueryPerformanceCounter(&this->tmstart);
	QueryPerformanceFrequency(&this->tmfreq);
#else
	gettimeofday(&tmstart, NULL);
#endif
	this->run(true);
}

#if !defined(__GNUC__) && !defined(__ICC__)
extern int layer_event_sort(const void *a, const void *b);
#else
int layer_event_sort(const void *a, const void *b)
{
	float la = (*((Event **)a))->layer;
	float lb = (*((Event **)b))->layer;

	if (la < lb) return -1;
	if (la > lb) return 1;
	return 0;
}
#endif

float MainLoop::get_time()
{
	if (this->timer == NULL) {
#if WIN32
		LARGE_INTEGER now;
		QueryPerformanceCounter(&now);
		return (float)(now.QuadPart - tmstart.QuadPart) /
			(float)(tmfreq.QuadPart);
#else
		struct timeval now;

		gettimeofday(&now, NULL);
		return (now.tv_sec - tmstart.tv_sec) +
			(float)(now.tv_usec - tmstart.tv_usec) * 0.000001f;
#endif
	} else {
		return this->timer->get_time() + 0.010;
	}
}

/*
 * This serves both as a main loop and a GLUT callback point. It has to work
 * this way because <demo> really is special -- most likely, <demo> will _never_
 * return (and that's good, because then we won't need to check which effects
 * have passed long time ago ;-) ) but will just keep on calling run() back...
 */
void MainLoop::run(bool infloop)
{
#if !DEMOLIB_SILENT
	int err = glGetError();
	if (err != 0) {
		printf("Warning: Some part of initialization has OpenGL error 0x%x\n", err);
	}
#endif
	do {
		this->num_play_events = 0;
		float timer = this->get_time();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glMatrixMode(GL_MODELVIEW);
	        glLoadIdentity();
		
		/*
		 * go through any active handlers and see if they should be ended
		 * (could perhaps be in reverse order? at least </music> and </demo>
		 * SHOULD come last)
		 */
		for (int i = 0; i < this->next_evnum; i++) {
			Event *e = this->events[i];
			if (e && e->active) {
				if (timer > e->stop && e->stop != -1.0f) {
#if !DEMOLIB_SILENT
					printf("Exiting: %s\n", e->title);
#endif

					e->end_effect();
				
#if !DEMOLIB_SILENT
					int err = glGetError();
					if (err != 0) {
						printf("Warning: Effect `%s' has OpenGL error 0x%x in destroy code\n", e->title, err);
					}
#endif
					
					if (i == 0 || i == 1) {
						e->active = false;
					} else {
#if !DEMOLIB_SILENT
						char *buf = strdup(e->title);
#endif
						delete e;
#if !DEMOLIB_SILENT
						int err = glGetError();
						if (err != 0) {
							printf("Warning: Effect `%s' has OpenGL error 0x%x in destroy code\n", buf, err);
						}
						free(buf);
#endif
						this->events[i] = NULL;
					}
#endif
				} else {
					evlist[num_play_events++] = e;
				}
			}
		}
		
		/* fire off any new events */
		while (this->next_evnum < this->num_events &&
		       timer >= this->events[this->next_evnum]->start) {
			Event *e = this->events[this->next_evnum];
			this->next_evnum++;

			if (timer > e->stop && e->stop != -1.0f) {
				/* we missed it completely! */
			} else {
#if !DEMOLIB_SILENT
				printf("Entering: %s\n", e->title);
#endif
				e->active = true;
				e->start_effect();
				evlist[num_play_events++] = e;
#if !DEMOLIB_SILENT
				int err = glGetError();
				if (err != 0 && this->next_evnum != 1) {
					printf("Warning: Effect `%s' has OpenGL error 0x%x in init code\n", e->title, err);
				}
#endif
			}
		}


		/* now sort the effects by layer (lower/negative ones come first) */
		if (num_play_events > 0) {
			qsort(evlist, num_play_events, sizeof(Event *), layer_event_sort);
			for (int i = 0; i < num_play_events; i++) {
				Event *e = evlist[i];
				e->draw_scene((timer - e->start) / (e->stop - e->start));
					
#if !DEMOLIB_SILENT
				int err = glGetError();
				if (err != 0 && i != 0) {
					printf("Warning: Effect `%s' has OpenGL error 0x%x\n", e->title, err);
				}
#endif
			}
			
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluPerspective(53.0f, DEMOLIB_XASPECT / DEMOLIB_YASPECT, 1.0f, 500.0f);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
		}
	} while (infloop && this->events[0] && this->events[0]->active);
}

void MainLoop::parse_commandline(int argc, char **argv, Hashtable *attr_hash)
{
	/* defaults :-) */
	attr_hash->insert("fullscreen", "yes");
	attr_hash->insert("xres", "640");
	attr_hash->insert("yres", "480");
	attr_hash->insert("depth", "32");
	attr_hash->insert("zbuffer", "16");
				
	for (int j = 1; j < argc; j++) {
		if (strcmp(argv[j], "-fullscreen") == 0) {
			attr_hash->insert("fullscreen", "yes");
			continue;
		}
		if (strcmp(argv[j], "-windowed") == 0) {
			attr_hash->insert("fullscreen", "no");
			continue;
		}
		if (strcmp(argv[j], "-noprecalc") == 0) {
			attr_hash->insert("noprecalcscreen", "yes");
			continue;
		}
		/* hidden modes, not very supported ;-) */
		if (strcmp(argv[j], "-320x200") == 0) {
			attr_hash->insert("xres", "320");
			attr_hash->insert("yres", "200");
			continue;
		}
		if (strcmp(argv[j], "-320x240") == 0) {
			attr_hash->insert("xres", "320");
			attr_hash->insert("yres", "240");
			continue;
		}
		if (strcmp(argv[j], "-512x384") == 0) {
			attr_hash->insert("xres", "512");
			attr_hash->insert("yres", "384");
			continue;
		}
		if (strcmp(argv[j], "-640x480") == 0) {
			attr_hash->insert("xres", "640");
			attr_hash->insert("yres", "480");
			continue;
		}
		if (strcmp(argv[j], "-800x600") == 0) {
			attr_hash->insert("xres", "800");
			attr_hash->insert("yres", "600");
			continue;
		}
		if (strcmp(argv[j], "-1024x768") == 0) {
			attr_hash->insert("xres", "1024");
			attr_hash->insert("yres", "768");
			continue;
		}
		if (strcmp(argv[j], "-1280x960") == 0) {
			attr_hash->insert("xres", "1280");
			attr_hash->insert("yres", "960");
			continue;
		}
		if (strcmp(argv[j], "-1280x1024") == 0) {
			attr_hash->insert("xres", "1280");
			attr_hash->insert("yres", "1024");
			continue;
		}
		if (strcmp(argv[j], "-1400x1050") == 0) {
			attr_hash->insert("xres", "1400");
			attr_hash->insert("yres", "1050");
			continue;
		}
		if (strcmp(argv[j], "-1600x1200") == 0) {
			attr_hash->insert("xres", "1600");
			attr_hash->insert("yres", "1200");
			continue;
		}
		if (strcmp(argv[j], "-16") == 0) {
			attr_hash->insert("depth", "16");
			continue;
		}
		if (strcmp(argv[j], "-32") == 0) {
			attr_hash->insert("depth", "32");
			continue;
		}
		if (strcmp(argv[j], "-z16") == 0) {
			attr_hash->insert("zbuffer", "16");
			continue;
		}
		if (strcmp(argv[j], "-z24") == 0) {
			attr_hash->insert("zbuffer", "24");
			continue;
		}
		if (strcmp(argv[j], "-z32") == 0) {
			attr_hash->insert("zbuffer", "32");
			continue;
		}
		if (strcmp(argv[j], "-nosound") == 0) {
			this->sound = false;
			continue;
		}
		throw new FatalException(
			"Allowed switches:\n\n"
			"-fullscreen\tRun in fullscreen (default)\n"
			"-windowed\tRun in a window (NOT SUPPORTED)\n"
			"-640x480\tRun in 640x480 (default)\n"
			"-800x600\tRun in 800x600\n"
			"-1024x768\tRun in 1024x768\n"
			"-1280x960\tRun in 1280x960\n"
			"-1280x1024\tRun in 1280x1024\n"
			"-1400x1050\tRun in 1400x1050 (yes, it exists)\n"
			"-1600x1200\tRun in 1600x1200 (good luck)\n"
			"-16\t\tRun in 16bpp (worse quality, but might work better)\n"
			"-32\t\tRun in 32bpp (default)\n"
			"-z16\t\t16bpp Z-buffer (default)\n"
			"-z24\t\t24bpp Z-buffer (better quality if it works)\n"
			"-z32\t\t32bpp Z-buffer (even better quality if it works)\n"
			"-nosound\tRun silently\n"
		);
	}
}
