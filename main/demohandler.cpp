#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __unix__
#include <X11/keysym.h>
#endif

#include "main/demohandler.h"
#include "opengl/glwindow.h"
#include "../exception.h"
#include "../demolib_prefs.h"

#if DEMOLIB_MAINLOOP 

DemoHandler::DemoHandler(MainLoop *ml, const char *title, const char *elem, Hashtable *attr) :
	Event(ml, title, elem, attr, NULL)
{
	this->ml = ml;
	this->layer = -1000.0f;

	bool fullscreen = attr->get_bool("fullscreen");

	/*
	 * It's extremely important that this comes HERE and not in
	 * start_effect()! Otherwise, other constructors doing OpenGL
	 * commands (textures, for instance) would fail and mess
	 * everything up quite badly...
	 */
	int xres = attr->get_int("xres");
	int yres = attr->get_int("yres");

	if (attr->exists("visual_id")) {
		this->win = new GLWindow(this->title, xres, yres, -1,
			fullscreen, -1, attr->get_int("visual_id"));
		return;
	}
		
	int bpp = attr->get_int("depth");
	int zbpp = attr->get_int("zbuffer");
	this->win = new GLWindow(this->title, xres, yres, bpp,
		fullscreen, zbpp, -1);

}
DemoHandler::~DemoHandler()
{
	delete this->win;
	this->win = NULL;
}

void DemoHandler::start_effect()
{
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glShadeModel(GL_SMOOTH);
        glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
        glDepthFunc(GL_LESS);

	while (this->active) {
#ifdef __unix__
		/*
		 * Linux doesn't use a message queue like Win32, so we'll
		 * handle X events here :-)
		 */
		while (XPending(this->win->dpy) > 0) {
			XEvent event;
			XNextEvent(this->win->dpy, &event);

			switch (event.type) {
			case ConfigureNotify:
                		if ((event.xconfigure.width != (signed int)this->win->width) || 
				    (event.xconfigure.height != (signed int)this->win->height)) {
					this->win->width = event.xconfigure.width;
					this->win->height = event.xconfigure.height;
					this->win->resize(0, 0, this->win->width, this->win->height);
				}
				break;
	            	case ButtonPress:   
				this->win->done = true;
				break;
			case KeyPress:
				if (XLookupKeysym(&event.xkey, 0) == XK_Escape) {
					this->win->done = true;
				}
		                break;
			}
		}
#endif

		this->ml->run(false);
		this->win->flip();

		if (this->win->is_done()) {
			this->end_effect();	
		}
	}
}

void DemoHandler::draw_scene(float progress)
{
}

void DemoHandler::end_effect()
{
	this->active = false;

	/* end everything, MainLoop will clean it up */
	for (int i = 0; i < this->ml->num_events; i++) {
		Event *e = this->ml->events[i];
		if (e && e->active && e != this) {
#if !DEMOLIB_SILENT
			printf("Exiting: %s\n", e->title);
#endif

			e->end_effect();
			e->active = false;
		}
	}
}

#endif
