/*
 * The whole interface between GLWindow and the configuration stuff is rather
 * icky, and _should_ be rewritten. It appears to work somehow, though ;-)
 */

#include <stdio.h>

#ifdef WIN32
#include <windows.h>
#endif

#ifdef __linux__
#include <unistd.h>
#include <GL/glx.h>
#include <X11/extensions/xf86vmode.h>
#include <X11/keysym.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#include "opengl/glwindow.h"
#include "exception.h"
#include "demolib_prefs.h"

#ifdef WIN32
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

/* this is ugly, but so is Win32 ;-) */
GLWindow *win;
#endif

void GLWindow::resize(int x, int y, int w, int h)
{
	/* Prevent division by zero */
	if (h == 0) {
		h = 1;
	}

	float aspect = (float)w / (float)h;
	if (aspect > DEMOLIB_XASPECT / DEMOLIB_YASPECT) {
		int new_w = (int)((float)h * DEMOLIB_XASPECT / DEMOLIB_YASPECT);
		x += (w - new_w) / 2;
		w = new_w;
	} else if (aspect < DEMOLIB_XASPECT / DEMOLIB_YASPECT) {
		int new_h = (int)((float)w * DEMOLIB_YASPECT / DEMOLIB_XASPECT);
		y += (h - new_h) / 2;
		h = new_h;
	}
 
	glViewport(x, y, w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
        gluPerspective(53.0f, (GLfloat)w / (GLfloat)h, 1.0f, 500.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

#ifdef __linux__
//	XClearWindow(this->dpy, this->win);
#endif
}

GLWindow::GLWindow(char *title, int width, int height, int bpp, bool fullscreen, int zbuffer, int visual_id)
{
#ifdef WIN32
 	GLuint PixelFormat;
	WNDCLASS wc;
	DWORD dwExStyle;
	DWORD dwStyle;
	DEVMODE dmScreenSettings;

	if (visual_id != -1) {
		EnumDisplaySettings(NULL, visual_id, &dmScreenSettings);
		width = dmScreenSettings.dmPelsWidth;
		height = dmScreenSettings.dmPelsHeight;
		bpp = dmScreenSettings.dmBitsPerPel;
	} else {
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = width;
		dmScreenSettings.dmPelsHeight = height;
		dmScreenSettings.dmBitsPerPel = bpp;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	}
	win = this;
	
	RECT WindowRect;
	WindowRect.left = (long)0;
	WindowRect.right = (long)width;
	WindowRect.top = (long)0;
	WindowRect.bottom = (long)height;
	
#endif /* WIN32 */	
#ifdef __linux__
	XVisualInfo *vi;
	int dpyWidth = 0, dpyHeight = 0;
	int i;
	XF86VidModeModeInfo **modes;
	int modeNum;
	int bestMode;
	Atom wmDelete;
	Window winDummy;
	unsigned int borderDummy;

	static int attrList[] = {
 		GLX_RGBA, 
		GLX_RED_SIZE, 1,
		GLX_GREEN_SIZE, 1,
		GLX_BLUE_SIZE, 1,
		GLX_DOUBLEBUFFER,
		GLX_DEPTH_SIZE, zbuffer,
		GLX_STENCIL_SIZE, 4,
		None
	};
#endif /* __linux__ */

	this->x = 0;
	this->y = 0;
	this->width = width;
	this->height = height;
	this->bpp = bpp;
	this->fullscreen = fullscreen;
	this->zbuffer = zbuffer;
	this->done = 0;
	
#ifdef WIN32
	this->hInstance = GetModuleHandle(NULL);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = this->hInstance;
	wc.hIcon = NULL;
	wc.hCursor = NULL;
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "Excess-OGL";

	if( !RegisterClass(&wc) ) throw new FatalException("Couldn't register Window Class");

#endif /* WIN32 */
#ifdef __linux__
	/* set best mode to current */
	bestMode = 0;

	/* get a connection */
	this->dpy = XOpenDisplay(NULL);
	if (dpy == NULL)
		throw new FatalException("Can't connect to X server!");
	this->screen = DefaultScreen(this->dpy);

	if (fullscreen) {
		XF86VidModeGetAllModeLines(this->dpy, this->screen, &modeNum, &modes);

		/* save desktop-resolution before switching modes */
		this->deskMode = *modes[0];

		/* look for mode with requested resolution */
		for (i = 0; i < modeNum; i++) {
			if ((modes[i]->hdisplay == width) && (modes[i]->vdisplay == height)) {
				bestMode = i;
			}
		}

		/* if we don't have it, bomb out */
		if (bestMode == 0 && (modes[0]->hdisplay != width || modes[0]->vdisplay != height)) {
			throw new FatalException("Couldn't set requested screen mode.");
		}
	}

	if (visual_id != -1) {
		XVisualInfo tmplate;
		int nret;
		
		tmplate.visualid = visual_id;
		vi = XGetVisualInfo(this->dpy, VisualIDMask, &tmplate, &nret);
		if (vi == NULL) {
			throw new FatalException("Couldn't get selected visual!");
		}
	} else {
		/* get an appropriate visual */
		vi = glXChooseVisual(this->dpy, this->screen, attrList);
		if (vi == NULL) {
			throw new FatalException("Couldn't get double-buffered visual");
		}
	}

	/* create a GLX context */
	this->ctx = glXCreateContext(this->dpy, vi, NULL, GL_TRUE);

	/* create a color map (umm, needed?) */
	Colormap cmap = XCreateColormap(this->dpy, RootWindow(this->dpy, vi->screen),
		vi->visual, AllocNone);
	this->attr.colormap = cmap;

	/* make a blank cursor */
	{
		static char data[1] = {0};
		Cursor cursor;
		Pixmap blank;
		XColor dummy;

		blank = XCreateBitmapFromData(this->dpy, RootWindow(this->dpy, vi->screen), data, 1, 1);
		if (blank == None)
			throw new FatalException("Out of memory!");
		cursor = XCreatePixmapCursor(this->dpy, blank, blank, &dummy, &dummy, 0, 0);
		XFreePixmap(this->dpy, blank);
		this->attr.cursor = cursor;
	}
		
	this->attr.border_pixel = 0;
#endif /* __linux__ */

	/* change screen mode */	
	if (fullscreen) {
#ifdef WIN32
		if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
			throw new FatalException("Couldn't set requested screen mode.");
		}
#endif /* WIN32 */
#ifdef __linux__
		XF86VidModeSwitchToMode(this->dpy, this->screen, modes[bestMode]);
		XF86VidModeSetViewPort(this->dpy, this->screen, 0, 0);
		dpyWidth = modes[bestMode]->hdisplay;
		dpyHeight = modes[bestMode]->vdisplay;
		XFree(modes);
#endif /* __linux__ */
	}

	/* create the window */
#ifdef WIN32
	if (fullscreen) {
		dwExStyle = WS_EX_APPWINDOW;
		dwStyle = WS_POPUP;
		ShowCursor(FALSE);
	} else {
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle = WS_OVERLAPPEDWINDOW;
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);

	if (!(hWnd = CreateWindowEx(dwExStyle,
  				    "Excess-OGL",
				    title,
				    dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
				    0, 0,
				    WindowRect.right - WindowRect.left,
				    WindowRect.bottom - WindowRect.top,
				    NULL,
				    NULL,
				    this->hInstance,
				    NULL))) {
	  	delete this;
		throw new FatalException("Could not change screenmode");
	}
#endif
#ifdef __linux__
	this->attr.background_pixel = 0;

	if (fullscreen) {
		/* create a fullscreen window */
		this->attr.override_redirect = True;
		this->attr.event_mask = KeyPressMask | ButtonPressMask |
					StructureNotifyMask;

		this->win = XCreateWindow(this->dpy, RootWindow(this->dpy, vi->screen),
			0, 0, dpyWidth, dpyHeight, 0, vi->depth, InputOutput, vi->visual,
			CWColormap | CWCursor | CWEventMask | CWOverrideRedirect,
			&this->attr);
		XWarpPointer(this->dpy, None, this->win, 0, 0, 0, 0, 0, 0);
		XMapRaised(this->dpy, this->win);
		XGrabKeyboard(this->dpy, this->win, True, GrabModeAsync,
			GrabModeAsync, CurrentTime);
		XGrabPointer(this->dpy, this->win, True, ButtonPressMask,
			GrabModeAsync, GrabModeAsync, this->win, None, CurrentTime);
	} else {
		/* create a window in window mode*/
		this->attr.event_mask = KeyPressMask | ButtonPressMask |
			StructureNotifyMask;
		this->win = XCreateWindow(this->dpy, RootWindow(this->dpy, vi->screen),
			0, 0, width, height, 0, vi->depth, InputOutput, vi->visual,
			CWColormap | CWBorderPixel | CWEventMask, &this->attr);

		/* only set window title and handle wm_delete_events if in windowed mode */
		wmDelete = XInternAtom(this->dpy, "WM_DELETE_WINDOW", True);
		XSetWMProtocols(this->dpy, this->win, &wmDelete, 1);
		XSetStandardProperties(this->dpy, this->win, title,
			title, None, NULL, 0, NULL);
		XMapRaised(this->dpy, this->win);
	}
#endif /* __linux__ */

#ifdef WIN32
	static PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		bpp,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		zbuffer,
		8,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};
	
	hDC = GetDC(hWnd);
	PixelFormat = ChoosePixelFormat(hDC, &pfd);
	if (PixelFormat == 0) {
		throw new FatalException("Could not find a usable pixelformat");
	}
	SetPixelFormat(hDC, PixelFormat, &pfd);
	hRC = wglCreateContext(hDC);
	wglMakeCurrent(hDC, hRC);
	ShowWindow(hWnd, SW_SHOW);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);

	SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
#endif /* WIN32	*/
#ifdef __linux__
	/* connect the glx-context to the window */
	glXMakeCurrent(this->dpy, this->win, this->ctx);
	XClearWindow(this->dpy, this->win);
	XGetGeometry(this->dpy, this->win, &winDummy, &this->x, &this->y,
		&this->width, &this->height, &borderDummy, &this->bpp);
	if (!glXIsDirect(this->dpy, this->ctx)) {
		throw new FatalException("No direct rendering (hardware acceleration) available! (Check libGL.so.* symlinks)");
	}

	nice(-7);
#endif /* __linux__ */

	this->resize(0, 0, this->width, this->height);
}

GLWindow::~GLWindow()
{
#ifdef __linux__
	if (this->ctx) {
		if (!glXMakeCurrent(this->dpy, None, NULL)) {
			throw new FatalException("Could not release drawing context.");
		}
		glXDestroyContext(this->dpy, this->ctx);
		this->ctx = NULL;
	}
#endif

	if (fullscreen) {
#ifdef __linux__
		XF86VidModeSwitchToMode(this->dpy, this->screen, &this->deskMode);
		XF86VidModeSetViewPort(this->dpy, this->screen, 0, 0);
#endif
#ifdef WIN32
		ChangeDisplaySettings(NULL,0);
		ShowCursor(TRUE);
#endif
	}

#ifdef __linux__
	XCloseDisplay(this->dpy);
#endif

#ifdef WIN32
	if (hRC) {
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(hRC);
		hRC = NULL;
	}
	
	if (hDC != NULL && ReleaseDC(hWnd, hDC)) hDC = NULL;
	if (hWnd != NULL && DestroyWindow(hWnd)) hWnd = NULL;
	UnregisterClass("Excess-OGL", hInstance);
#endif
}

void GLWindow::flip()
{
#ifdef WIN32
	MSG msg;
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		if (msg.message == WM_QUIT) {
			this->done = TRUE;
		} else {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	SwapBuffers(this->hDC);
#endif
#ifdef __linux__
	glXSwapBuffers(this->dpy, this->win);
#endif
}

bool GLWindow::is_done()
{
	return this->done;
}

#ifdef WIN32
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_SYSCOMMAND:
		switch (wParam) {
		case SC_SCREENSAVE:
		case SC_MONITORPOWER:
			return 0;
		}
		break;

	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;

	case WM_KEYUP:
		if (wParam == VK_ESCAPE)
			PostQuitMessage(0);
		return 0;

	case WM_SIZE:
		win->resize(0, 0, LOWORD(lParam), HIWORD(lParam));
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
#endif //WIN32
