#ifndef _GLWINDOW_H
#define _GLWINDOW_H

#ifdef WIN32
#include <windows.h>
#endif

#ifdef __unix__
#include <GL/glx.h>
#include <X11/extensions/xf86vmode.h>
#include <X11/keysym.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>

class GLWindow {
public:
	GLWindow(char *title, int width, int height, int bpp, bool fullscreen, int zbuffer, int visual_id);
	~GLWindow();
	void resize(int x, int y, int w, int h);
	void flip();
	bool is_done();

	friend class DemoHandler;
	friend class DirectSoundAudioDriver;
	
protected:
#ifdef WIN32
	HDC hDC;
	HGLRC hRC;
	HWND hWnd;
	HINSTANCE hInstance;
#endif
#ifdef __unix__
	Display *dpy;
	int screen;
	Window win;
	GLXContext ctx;
	XSetWindowAttributes attr;
	Bool fs;
	XF86VidModeModeInfo deskMode;
#endif						    
	
	char *title;
	bool fullscreen;
	int x, y;
	unsigned int width, height;
	unsigned int bpp;
	int zbuffer;
	bool done;
	void initGL();
};

#endif
