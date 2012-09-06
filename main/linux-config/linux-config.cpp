/*
 * A Linux GTK+ configuration dialog. Tries to use GTK+ using runtime
 * linking -- if the user hasn't got GTK+ on his/her system, we revert
 * to plain command line (ie. we just throw a NonFatalException and
 * the main loop reverts to command line :-) ).
 *
 * We use GTK+ 1.2, since it would appear most people has got that
 * version, and that's what GLADE outputs ;-) The code is rather messy,
 * but it more or less has to be that way when we use dlsym() for so
 * many objects (no, I don't want to link in GTK+ statically ;-) ).
 * Probably leaks memory badly too, but Linux will clean it up for us
 * and it's only a few kB anyhow, so it won't matter much for the 
 * actual demo :-)
 *
 * FIXME: Should we support command line options even with GTK+?
 */

#define GTK_NO_CHECK_CASTS

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include <errno.h>

#include <GL/glx.h>
#include <X11/extensions/xf86vmode.h>
#include <X11/keysym.h>

#include "image/image.h"
#include "linux-config.h"
#include "exception.h"

#undef GTK_OBJECT
#define GTK_OBJECT(object) (GtkObject*) (object)

int modeline_compare(const void *a, const void *b)
{
	XF86VidModeModeInfo *am = *((XF86VidModeModeInfo **)a);
	XF86VidModeModeInfo *bm = *((XF86VidModeModeInfo **)b);

	if (am->hdisplay < bm->hdisplay) return -1;
	if (am->hdisplay == bm->hdisplay) {
		if (am->vdisplay < bm->vdisplay) return -1;
		if (am->vdisplay == bm->vdisplay) return 0;
		return 1;
	}
	return 1;
}

LinuxConfig::LinuxConfig()
{
	void *gdkh = dlopen("libgdk-1.2.so.0", RTLD_GLOBAL | RTLD_NOW);
	if (gdkh == NULL)
		throw new NonFatalException("libgdk-1.2.so.0", strerror(errno));
	
	void *gtkh = dlopen("libgtk-1.2.so.0", RTLD_NOW);
	if (gtkh == NULL)
		throw new NonFatalException("libgtk-1.2.so.0", strerror(errno));

	/* now we go through and load all the symbols we need, error-checking as we go */
#define LOAD_SYMBOL(lib, x, cast) { \
	this->_ ## x = (cast)dlsym(lib ## h, #x); \
	if (this->_ ## x == NULL) throw new NonFatalException(#x, strerror(errno)); \
}
	LOAD_SYMBOL(gdk, gdk_pixmap_new, GdkPixmap* (*)(GdkWindow*, gint, gint, gint));
	LOAD_SYMBOL(gdk, gdk_pixmap_unref, void (*)(GdkPixmap*));
	LOAD_SYMBOL(gdk, gdk_gc_new, GdkGC* (*)(GdkDrawable*));
	LOAD_SYMBOL(gdk, gdk_gc_unref, void (*)(GdkGC*));
	LOAD_SYMBOL(gdk, gdk_rgb_init, void (*)(void));
	LOAD_SYMBOL(gdk, gdk_draw_rgb_32_image, void (*)(GdkDrawable *, GdkGC *, gint, gint, gint, gint, GdkRgbDither, guchar *, gint));


	LOAD_SYMBOL(gtk, gtk_alignment_new, GtkWidget* (*)(gfloat, gfloat, gfloat, gfloat));
	LOAD_SYMBOL(gtk, gtk_box_pack_start, void (*)(GtkBox*, GtkWidget*, gboolean, gboolean, guint));
	LOAD_SYMBOL(gtk, gtk_button_new_with_label, GtkWidget* (*)(const gchar *));
	LOAD_SYMBOL(gtk, gtk_container_add, void (*)(GtkContainer*, GtkWidget *));
	LOAD_SYMBOL(gtk, gtk_container_set_border_width, void (*)(GtkContainer*, guint));
	LOAD_SYMBOL(gtk, gtk_exit, void (*)(gint));
	LOAD_SYMBOL(gtk, gtk_hbox_new, GtkWidget* (*)(gboolean, gint));
	LOAD_SYMBOL(gtk, gtk_label_new, GtkWidget* (*)(const gchar*));
	LOAD_SYMBOL(gtk, gtk_label_set_justify, void (*)(GtkLabel*, GtkJustification));
	LOAD_SYMBOL(gtk, gtk_main_iteration_do, int (*)(gboolean));
	LOAD_SYMBOL(gtk, gtk_main_quit, void (*)(void));
	LOAD_SYMBOL(gtk, gtk_menu_append, void (*)(GtkMenu*, GtkWidget*));
	LOAD_SYMBOL(gtk, gtk_menu_get_active, GtkWidget* (*)(GtkMenu*));
	LOAD_SYMBOL(gtk, gtk_menu_item_new_with_label, GtkWidget* (*)(const gchar*));
	LOAD_SYMBOL(gtk, gtk_menu_new, GtkWidget* (*)(void));
	LOAD_SYMBOL(gtk, gtk_misc_set_alignment, void (*)(GtkMisc*, gfloat, gfloat));
	LOAD_SYMBOL(gtk, gtk_object_get_data, gpointer (*)(GtkObject*, const gchar*));
	LOAD_SYMBOL(gtk, gtk_object_set_data, void (*)(GtkObject*, const gchar*, gpointer));
	LOAD_SYMBOL(gtk, gtk_object_set_data_full, void (*)(GtkObject*, const gchar*, gpointer, GtkDestroyNotify));
	LOAD_SYMBOL(gtk, gtk_option_menu_new, GtkWidget* (*)(void));
	LOAD_SYMBOL(gtk, gtk_option_menu_set_history, void (*)(GtkOptionMenu*, guint));
	LOAD_SYMBOL(gtk, gtk_option_menu_set_menu, void (*)(GtkOptionMenu*, GtkWidget*));
	LOAD_SYMBOL(gtk, gtk_pixmap_new, GtkWidget* (*)(GdkPixmap*, GdkPixmap*));
	LOAD_SYMBOL(gtk, gtk_signal_connect, guint (*)(GtkObject*, const gchar*, GtkSignalFunc, gpointer));
	LOAD_SYMBOL(gtk, gtk_signal_disconnect, void (*)(GtkObject*, guint));
	LOAD_SYMBOL(gtk, gtk_table_attach, void (*)(GtkTable*, GtkWidget*, guint, guint, guint, guint, GtkAttachOptions, GtkAttachOptions, guint, guint));
	LOAD_SYMBOL(gtk, gtk_table_new, GtkWidget* (*)(guint, guint, gboolean));
	LOAD_SYMBOL(gtk, gtk_table_set_col_spacings, void (*)(GtkTable*, guint));
	LOAD_SYMBOL(gtk, gtk_vbox_new, GtkWidget* (*)(gboolean, gint));
	LOAD_SYMBOL(gtk, gtk_widget_destroy, void (*)(GtkWidget*));
	LOAD_SYMBOL(gtk, gtk_widget_get_visual, GdkVisual* (*)(GtkWidget*));
	LOAD_SYMBOL(gtk, gtk_widget_hide, void (*)(GtkWidget*));
	LOAD_SYMBOL(gtk, gtk_widget_ref, void (*)(GtkWidget*));
	LOAD_SYMBOL(gtk, gtk_widget_show, void (*)(GtkWidget*));
	LOAD_SYMBOL(gtk, gtk_widget_unref, void (*)(GtkWidget*));
	LOAD_SYMBOL(gtk, gtk_window_new, GtkWidget* (*)(GtkWindowType));
	LOAD_SYMBOL(gtk, gtk_window_set_modal, void (*)(GtkWindow*, gboolean));
	LOAD_SYMBOL(gtk, gtk_window_set_policy, void (*)(GtkWindow*, gint, gint, gint));
	LOAD_SYMBOL(gtk, gtk_window_set_position, void (*)(GtkWindow*, GtkWindowPosition));
	LOAD_SYMBOL(gtk, gtk_window_set_title, void (*)(GtkWindow*, const gchar*));

	LOAD_SYMBOL(gtk, gtk_init, void (*)(int*, char ***));
	LOAD_SYMBOL(gtk, gtk_main, void (*)(void));
	LOAD_SYMBOL(gtk, gtk_set_locale, void (*)(void));
}

/* this is largely GLADE-generated code, but modified for the dl */
void LinuxConfig::show(int *argc, char ***argv, Hashtable *attr_hash)
{
	/*
	 * open an X connection to enumerate resolutions and visuals
	 * (code duplication from glwindow.cpp, not good)
	 */
	XF86VidModeModeInfo **modes;
	Display *dpy = XOpenDisplay(0);
	int screen = DefaultScreen(dpy);
	
	(*_gtk_set_locale) ();
	(*_gtk_init) (argc, argv);

	GtkWidget *config;
	GtkWidget *alignment1;
	GtkWidget *vbox2;
	GtkWidget *hbox2;
	GtkWidget *pixmap1;
	GtkWidget *table2;
	GtkWidget *mode_label;
	GtkWidget *vis_label;
	GtkWidget *res_menu;
	GtkWidget *res_menu_menu;
	GtkWidget *glade_menuitem;
	GtkWidget *fullscreenmenu;
	GtkWidget *fullscreenmenu_menu;
	GtkWidget *visualmenu;
	GtkWidget *visualmenu_menu;
	GtkWidget *sound_label;
	GtkWidget *soundmenu;
	GtkWidget *soundmenu_menu;
	GtkWidget *res_label;
	GtkWidget *spacefiller1;
	GtkWidget *spacefiller2;
	GtkWidget *hbox4;
	GtkWidget *ok;
	GdkPixmap *gdkpixmap;
	Image *logo;
	int w, h, bpp;
	unsigned char *ptr;
	GdkGC *gc;
	
	config = (*_gtk_window_new) (GTK_WINDOW_DIALOG);
	(*_gtk_object_set_data) (GTK_OBJECT (config), "config", config);
	(*_gtk_window_set_title) (GTK_WINDOW (config), "Excess demo configuration (GLX/X11)");
	(*_gtk_window_set_position) (GTK_WINDOW (config), GTK_WIN_POS_CENTER);
	(*_gtk_window_set_modal) (GTK_WINDOW (config), TRUE);
	(*_gtk_window_set_policy) (GTK_WINDOW (config), FALSE, FALSE, FALSE);

	alignment1 = (*_gtk_alignment_new) (0.5, 0.5, 1, 1);
	(*_gtk_widget_ref) (alignment1);
	(*_gtk_object_set_data_full) (GTK_OBJECT (config), "alignment1", alignment1,
	                          (GtkDestroyNotify) (*_gtk_widget_unref));
	(*_gtk_widget_show) (alignment1);
	(*_gtk_container_add) (GTK_CONTAINER (config), alignment1);

	vbox2 = (*_gtk_vbox_new) (FALSE, 0);
	(*_gtk_widget_ref) (vbox2);
	(*_gtk_object_set_data_full) (GTK_OBJECT (config), "vbox2", vbox2,
	                          (GtkDestroyNotify) (*_gtk_widget_unref));
	(*_gtk_widget_show) (vbox2);
	(*_gtk_container_add) (GTK_CONTAINER (alignment1), vbox2);

	hbox2 = (*_gtk_hbox_new) (FALSE, 10);
	(*_gtk_widget_ref) (hbox2);
	(*_gtk_object_set_data_full) (GTK_OBJECT (config), "hbox2", hbox2,
	                          (GtkDestroyNotify) (*_gtk_widget_unref));
	(*_gtk_widget_show) (hbox2);
	(*_gtk_box_pack_start) (GTK_BOX (vbox2), hbox2, TRUE, TRUE, 0);
	(*_gtk_container_set_border_width) (GTK_CONTAINER (hbox2), 5);

	/*
	 * Get the logo from a PNG -- what idiots made it so we can't just get a
	 * GdkPixmap directly from a raw buffer?
	 */
	logo = load_image("launcherlogo.png");
	w = logo->get_width();
	h = logo->get_height();
	bpp = logo->get_bpp();
	ptr = logo->get_pixel_data();
	if (bpp != 32)
		throw new NonFatalException("Launcher logo must be 32bpp");

	gdkpixmap = (*_gdk_pixmap_new) (NULL, w, h, ((*_gtk_widget_get_visual) (config))->depth);
	gc = (*_gdk_gc_new) (gdkpixmap);
	(*_gdk_rgb_init) ();
	(*_gdk_draw_rgb_32_image) (gdkpixmap, gc, 0, 0, w, h, GDK_RGB_DITHER_NONE, ptr, w * 4);
	
	pixmap1 = (*_gtk_pixmap_new) (gdkpixmap, NULL);
	(*_gdk_gc_unref) (gc);
	(*_gdk_pixmap_unref) (gdkpixmap);

	delete logo;

	(*_gtk_widget_ref) (pixmap1);
	(*_gtk_object_set_data_full) (GTK_OBJECT (config), "pixmap1", pixmap1,
	                          (GtkDestroyNotify) (*_gtk_widget_unref));
	(*_gtk_widget_show) (pixmap1);
	(*_gtk_box_pack_start) (GTK_BOX (hbox2), pixmap1, FALSE, TRUE, 0);

	table2 = (*_gtk_table_new) (6, 2, FALSE);
	(*_gtk_widget_ref) (table2);
	(*_gtk_object_set_data_full) (GTK_OBJECT (config), "table2", table2,
	                          (GtkDestroyNotify) (*_gtk_widget_unref));
	(*_gtk_widget_show) (table2);
	(*_gtk_box_pack_start) (GTK_BOX (hbox2), table2, TRUE, FALSE, 0);
	(*_gtk_table_set_col_spacings) (GTK_TABLE (table2), 7);

	mode_label = (*_gtk_label_new) ("Window mode:");
	(*_gtk_widget_ref) (mode_label);
	(*_gtk_object_set_data_full) (GTK_OBJECT (config), "mode_label", mode_label,
	                          (GtkDestroyNotify) (*_gtk_widget_unref));
	(*_gtk_widget_show) (mode_label);
	(*_gtk_table_attach) (GTK_TABLE (table2), mode_label, 0, 1, 2, 3,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);
	(*_gtk_label_set_justify) (GTK_LABEL (mode_label), GTK_JUSTIFY_LEFT);
	(*_gtk_misc_set_alignment) (GTK_MISC (mode_label), 0, 0.5);

	vis_label = (*_gtk_label_new) ("OpenGL visual:");
	(*_gtk_widget_ref) (vis_label);
	(*_gtk_object_set_data_full) (GTK_OBJECT (config), "vis_label", vis_label,
	                          (GtkDestroyNotify) (*_gtk_widget_unref));
	(*_gtk_widget_show) (vis_label);
	(*_gtk_table_attach) (GTK_TABLE (table2), vis_label, 0, 1, 3, 4,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);
	(*_gtk_label_set_justify) (GTK_LABEL (vis_label), GTK_JUSTIFY_LEFT);
	(*_gtk_misc_set_alignment) (GTK_MISC (vis_label), 0, 0.5);

	res_menu = (*_gtk_option_menu_new) ();
	(*_gtk_widget_ref) (res_menu);
	(*_gtk_object_set_data_full) (GTK_OBJECT (config), "res_menu", res_menu,
	                          (GtkDestroyNotify) (*_gtk_widget_unref));
	(*_gtk_widget_show) (res_menu);
	(*_gtk_table_attach) (GTK_TABLE (table2), res_menu, 1, 2, 1, 2,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	/*
	 * enumerate through all the resolutions, then sort
	 */
	{
		int modeNum;
		int selected = 0;
		
		XF86VidModeGetAllModeLines(dpy, screen, &modeNum, &modes);
		qsort(modes, modeNum, sizeof(XF86VidModeModeInfo *), modeline_compare);
		
		res_menu_menu = (*_gtk_menu_new) ();

		for (int i = 0; i < modeNum; i++) {
			char buf[32];
			sprintf(buf, "%ux%u", modes[i]->hdisplay, modes[i]->vdisplay);
			glade_menuitem = (*_gtk_menu_item_new_with_label) (buf);
			(*_gtk_widget_show) (glade_menuitem);
			(*_gtk_menu_append) (GTK_MENU (res_menu_menu), glade_menuitem);

			sprintf(buf, "%u", modes[i]->hdisplay);
			(*_gtk_object_set_data) (GTK_OBJECT (glade_menuitem), strdup("xres"),
				strdup(buf));
			sprintf(buf, "%u", modes[i]->vdisplay);
			(*_gtk_object_set_data) (GTK_OBJECT (glade_menuitem), strdup("yres"),
				strdup(buf));
			
			if (modes[i]->hdisplay == 640 &&
			    modes[i]->vdisplay == 480) {
				selected = i;
			}
		}
		(*_gtk_option_menu_set_menu) (GTK_OPTION_MENU (res_menu), res_menu_menu);
		(*_gtk_option_menu_set_history) (GTK_OPTION_MENU (res_menu), selected);

		XFree(modes);
	}
	
	fullscreenmenu = (*_gtk_option_menu_new) ();
	(*_gtk_widget_ref) (fullscreenmenu);
	(*_gtk_object_set_data_full) (GTK_OBJECT (config), "fullscreenmenu", fullscreenmenu,
	                          (GtkDestroyNotify) (*_gtk_widget_unref));
	(*_gtk_widget_show) (fullscreenmenu);
	(*_gtk_table_attach) (GTK_TABLE (table2), fullscreenmenu, 1, 2, 2, 3,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);
	fullscreenmenu_menu = (*_gtk_menu_new) ();
	
	glade_menuitem = (*_gtk_menu_item_new_with_label) ("Fullscreen");
	(*_gtk_object_set_data) (GTK_OBJECT (glade_menuitem), strdup("fullscreen"), strdup("yes"));
	(*_gtk_widget_show) (glade_menuitem);
	(*_gtk_menu_append) (GTK_MENU (fullscreenmenu_menu), glade_menuitem);
	
	glade_menuitem = (*_gtk_menu_item_new_with_label) ("Windowed");
	(*_gtk_object_set_data) (GTK_OBJECT (glade_menuitem), strdup("fullscreen"), strdup("no"));
	(*_gtk_widget_show) (glade_menuitem);
	(*_gtk_menu_append) (GTK_MENU (fullscreenmenu_menu), glade_menuitem);
	
	(*_gtk_option_menu_set_menu) (GTK_OPTION_MENU (fullscreenmenu), fullscreenmenu_menu);
	(*_gtk_option_menu_set_history) (GTK_OPTION_MENU (fullscreenmenu), 0);

	/*
	 * now find all the visuals available, sort out the useless ones, and
	 * show them in the dialog
	 */
	visualmenu = (*_gtk_option_menu_new) ();
	(*_gtk_widget_ref) (visualmenu);
	(*_gtk_object_set_data_full) (GTK_OBJECT (config), "visualmenu", visualmenu,
	                          (GtkDestroyNotify) (*_gtk_widget_unref));
	(*_gtk_widget_show) (visualmenu);
	(*_gtk_table_attach) (GTK_TABLE (table2), visualmenu, 1, 2, 3, 4,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);
	visualmenu_menu = (*_gtk_menu_new) ();

	{
		XVisualInfo tmplate;
		int nret, usable = 0;
		int bestid = 0, bestbpp = 0, bestdepth = 0;
		
		tmplate.screen = screen;
		XVisualInfo *xvi = XGetVisualInfo(dpy, VisualScreenMask, &tmplate, &nret);
		if (xvi == NULL)
			throw new FatalException("No usable visuals!");

		for (int i = 0; i < nret; i++) {
			int has_gl, has_db, stencil_bpp, bpp, depth;
			char buf[256];

			/* first check for doublebuffered GL */
			if (glXGetConfig(dpy, &(xvi[i]), GLX_USE_GL, &has_gl) != 0 ||
			    !has_gl)
				continue;
			if (glXGetConfig(dpy, &(xvi[i]), GLX_DOUBLEBUFFER, &has_db) != 0 ||
			    !has_db)
				continue;

			/* we also need at least 4bpp stencil */
			if (glXGetConfig(dpy, &(xvi[i]), GLX_STENCIL_SIZE, &stencil_bpp) != 0 ||
			    stencil_bpp < 4)
				continue;
			
			if (glXGetConfig(dpy, &(xvi[i]), GLX_BUFFER_SIZE, &bpp) != 0)
				continue;
			if (glXGetConfig(dpy, &(xvi[i]), GLX_DEPTH_SIZE, &depth) != 0)
				continue;
		
			sprintf(buf, "%dbpp, %d-bits Z-buffer", bpp, depth);

			/*
			 * verify that there aren't already identical (for our purposes --
			 * the first usable will be selected anyhow by glXChooseVisual
			 * so we don't have to do any better guesses, I think ;-) )
			 * modes in the list... a bit ineffective, but doesn't matter either.
			 * if we really want to optimize, we could make our own array
			 * or something :-)
			 */
			bool dup = false;
			for (int j = 0; j < i; j++) {
				int has_gl2, has_db2, stencil_bpp2, bpp2, depth2;
				if (glXGetConfig(dpy, &(xvi[j]), GLX_USE_GL, &has_gl2) != 0 ||
				    !has_gl2)
					continue;
				if (glXGetConfig(dpy, &(xvi[j]), GLX_DOUBLEBUFFER, &has_db2) != 0 ||
				    !has_db2)
					continue;
				if (glXGetConfig(dpy, &(xvi[j]), GLX_STENCIL_SIZE, &stencil_bpp2) != 0 ||
				    stencil_bpp2 < 4)
					continue;
				if (glXGetConfig(dpy, &(xvi[j]), GLX_BUFFER_SIZE, &bpp2) != 0 ||
				    bpp2 != bpp)
					continue;
				if (glXGetConfig(dpy, &(xvi[j]), GLX_DEPTH_SIZE, &depth2) != 0 ||
				    depth2 != depth)
					continue;

				dup = true;
				break;
			}
			
			if (dup)
				continue;
			
			glade_menuitem = (*_gtk_menu_item_new_with_label) (buf);
			(*_gtk_widget_show) (glade_menuitem);
			(*_gtk_menu_append) (GTK_MENU (visualmenu_menu), glade_menuitem);
			sprintf(buf, "%lu", xvi[i].visualid);
			(*_gtk_object_set_data) (GTK_OBJECT (glade_menuitem), strdup("visual_id"),
				strdup(buf));
			
			if (bpp > bestbpp || bpp == bestbpp && depth >= bestdepth) {
				bestid = usable;
				bestbpp = bpp;
				bestdepth = depth;
			}
			
			usable++;
		}
		if (usable == 0)
			throw new FatalException("No usable visuals!");
		(*_gtk_option_menu_set_menu) (GTK_OPTION_MENU (visualmenu), visualmenu_menu);
		(*_gtk_option_menu_set_history) (GTK_OPTION_MENU (fullscreenmenu), bestid);
	}
	XCloseDisplay(dpy);

	sound_label = (*_gtk_label_new) ("Sound:");
	(*_gtk_widget_ref) (sound_label);
	(*_gtk_object_set_data_full) (GTK_OBJECT (config), "sound_label", sound_label,
	                          (GtkDestroyNotify) (*_gtk_widget_unref));
	(*_gtk_widget_show) (sound_label);
	(*_gtk_table_attach) (GTK_TABLE (table2), sound_label, 0, 1, 4, 5,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);
	(*_gtk_label_set_justify) (GTK_LABEL (sound_label), GTK_JUSTIFY_LEFT);
	(*_gtk_misc_set_alignment) (GTK_MISC (sound_label), 0, 0.5);

	soundmenu = (*_gtk_option_menu_new) ();
	(*_gtk_widget_ref) (soundmenu);
	(*_gtk_object_set_data_full) (GTK_OBJECT (config), "soundmenu", soundmenu,
	                          (GtkDestroyNotify) (*_gtk_widget_unref));
	(*_gtk_widget_show) (soundmenu);
	(*_gtk_table_attach) (GTK_TABLE (table2), soundmenu, 1, 2, 4, 5,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);
	soundmenu_menu = (*_gtk_menu_new) ();

	if (access("/dev/dsp", R_OK | W_OK) == 0 ||
	    access("/dev/sound/dsp", R_OK | W_OK) == 0) {
		glade_menuitem = (*_gtk_menu_item_new_with_label) ("Yes");
		(*_gtk_object_set_data) (GTK_OBJECT (glade_menuitem), strdup("sound"), strdup("yes"));
		(*_gtk_widget_show) (glade_menuitem);
		(*_gtk_menu_append) (GTK_MENU (soundmenu_menu), glade_menuitem);
	}
	
	glade_menuitem = (*_gtk_menu_item_new_with_label) ("No");
	(*_gtk_object_set_data) (GTK_OBJECT (glade_menuitem), strdup("sound"), strdup("no"));
	(*_gtk_widget_show) (glade_menuitem);
	(*_gtk_menu_append) (GTK_MENU (soundmenu_menu), glade_menuitem);
	(*_gtk_option_menu_set_menu) (GTK_OPTION_MENU (soundmenu), soundmenu_menu);

	res_label = (*_gtk_label_new) ("Screen resolution:");
	(*_gtk_widget_ref) (res_label);
	(*_gtk_object_set_data_full) (GTK_OBJECT (config), "res_label", res_label,
	                          (GtkDestroyNotify) (*_gtk_widget_unref));
	(*_gtk_widget_show) (res_label);
	(*_gtk_table_attach) (GTK_TABLE (table2), res_label, 0, 1, 1, 2,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);
	(*_gtk_label_set_justify) (GTK_LABEL (res_label), GTK_JUSTIFY_LEFT);
	(*_gtk_misc_set_alignment) (GTK_MISC (res_label), 0, 0.5);

	spacefiller1 = (*_gtk_label_new) ("");
	(*_gtk_widget_ref) (spacefiller1);
	(*_gtk_object_set_data_full) (GTK_OBJECT (config), "spacefiller1", spacefiller1,
	                          (GtkDestroyNotify) (*_gtk_widget_unref));
	(*_gtk_widget_show) (spacefiller1);
	(*_gtk_table_attach) (GTK_TABLE (table2), spacefiller1, 0, 1, 0, 1,
	                  (GtkAttachOptions) (0),
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
	(*_gtk_misc_set_alignment) (GTK_MISC (spacefiller1), 0, 0.5);

	spacefiller2 = (*_gtk_label_new) ("");
	(*_gtk_widget_ref) (spacefiller2);
	(*_gtk_object_set_data_full) (GTK_OBJECT (config), "spacefiller2", spacefiller2,
	                          (GtkDestroyNotify) (*_gtk_widget_unref));
	(*_gtk_widget_show) (spacefiller2);
	(*_gtk_table_attach) (GTK_TABLE (table2), spacefiller2, 0, 1, 5, 6,
	                  (GtkAttachOptions) (0),
	                  (GtkAttachOptions) (GTK_EXPAND), 0, 0);
	(*_gtk_misc_set_alignment) (GTK_MISC (spacefiller2), 0, 0.5);

	hbox4 = (*_gtk_hbox_new) (FALSE, 0);
	(*_gtk_widget_ref) (hbox4);
	(*_gtk_object_set_data_full) (GTK_OBJECT (config), "hbox4", hbox4,
	                          (GtkDestroyNotify) (*_gtk_widget_unref));
	(*_gtk_widget_show) (hbox4);
	(*_gtk_box_pack_start) (GTK_BOX (vbox2), hbox4, FALSE, FALSE, 0);
	(*_gtk_container_set_border_width) (GTK_CONTAINER (hbox4), 4);

	ok = (*_gtk_button_new_with_label) ("Ninja!");
	(*_gtk_widget_ref) (ok);
	(*_gtk_object_set_data_full) (GTK_OBJECT (config), "ok", ok,
	                          (GtkDestroyNotify) (*_gtk_widget_unref));
	(*_gtk_signal_connect) (GTK_OBJECT (ok), "clicked", GTK_SIGNAL_FUNC (*_gtk_main_quit), NULL);

	(*_gtk_widget_show) (ok);
	(*_gtk_box_pack_start) (GTK_BOX (hbox4), ok, TRUE, TRUE, 1);
	
	int dest_signal = (*_gtk_signal_connect) (GTK_OBJECT (config), "destroy", GTK_SIGNAL_FUNC (*_gtk_exit), NULL);
	(*_gtk_widget_show)(config);
	
	(*_gtk_main) ();

	/* get the parameters that were set */
	GtkObject *resolution = GTK_OBJECT ((*_gtk_menu_get_active) (GTK_MENU (res_menu_menu)));
	attr_hash->insert("xres", (*_gtk_object_get_data) (resolution, "xres"));
	attr_hash->insert("yres", (*_gtk_object_get_data) (resolution, "yres"));
	
	GtkObject *fullscreen = GTK_OBJECT ((*_gtk_menu_get_active) (GTK_MENU (fullscreenmenu_menu)));	
	attr_hash->insert("fullscreen", (*_gtk_object_get_data) (fullscreen, "fullscreen"));

	GtkObject *visid = GTK_OBJECT ((*_gtk_menu_get_active) (GTK_MENU (visualmenu_menu)));	
	attr_hash->insert("visual_id", (*_gtk_object_get_data) (visid, "visual_id"));

	GtkObject *sound = GTK_OBJECT ((*_gtk_menu_get_active) (GTK_MENU (soundmenu_menu)));
	attr_hash->insert("sound", (*_gtk_object_get_data) (sound, "sound"));

	(*_gtk_signal_disconnect) (GTK_OBJECT (config), dest_signal);
	(*_gtk_widget_hide) (config);
	(*_gtk_widget_destroy) (config);
	(*_gtk_main_iteration_do) (false);
}
