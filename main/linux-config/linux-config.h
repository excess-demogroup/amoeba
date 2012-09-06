#ifndef _LINUX_CONFIG_H
#define _LINUX_CONFIG_H 1

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "util/hashtable.h"

class LinuxConfig
{
public:
	LinuxConfig();
	void show(int *argc, char ***argv, Hashtable *attr_hash);
	
private:
	GdkPixmap* (*_gdk_pixmap_new)(GdkWindow*, gint, gint, gint);
	void (*_gdk_pixmap_unref)(GdkPixmap*);
	GdkGC* (*_gdk_gc_new)(GdkDrawable*);
	void (*_gdk_gc_unref)(GdkGC*);
	void (*_gdk_rgb_init)(void);
	void (*_gdk_draw_rgb_32_image)(GdkDrawable *, GdkGC *, gint, gint, gint, gint, GdkRgbDither, guchar *, gint);
	
	GtkWidget* (*_gtk_alignment_new)(gfloat, gfloat, gfloat, gfloat);
	void (*_gtk_box_pack_start)(GtkBox*, GtkWidget*, gboolean, gboolean, guint);
	GtkWidget* (*_gtk_button_new_with_label)(const gchar*);
	void (*_gtk_container_add)(GtkContainer*, GtkWidget*);
	void (*_gtk_container_set_border_width)(GtkContainer*, guint);
	GtkWidget* (*_gtk_hbox_new)(gboolean, gint);
	GtkWidget* (*_gtk_label_new)(const gchar*);
	void (*_gtk_label_set_justify)(GtkLabel*, GtkJustification);
	gint (*_gtk_main_iteration_do)(gboolean);
	void (*_gtk_main_quit)(void);
	void (*_gtk_menu_append)(GtkMenu*, GtkWidget*);
	GtkWidget* (*_gtk_menu_get_active)(GtkMenu*);
	GtkWidget* (*_gtk_menu_item_new_with_label)(const gchar*);
	GtkWidget* (*_gtk_menu_new)(void);
	void (*_gtk_misc_set_alignment)(GtkMisc*, gfloat, gfloat);
	gpointer (*_gtk_object_get_data)(GtkObject*, const gchar*);
	void (*_gtk_object_set_data)(GtkObject*, const gchar*, gpointer);
	void (*_gtk_object_set_data_full)(GtkObject*, const gchar*, gpointer, GtkDestroyNotify);
	GtkWidget* (*_gtk_option_menu_new)(void);
	void (*_gtk_option_menu_set_history)(GtkOptionMenu*, guint);
	void (*_gtk_option_menu_set_menu)(GtkOptionMenu*, GtkWidget*);
	GtkWidget* (*_gtk_pixmap_new)(GdkPixmap*, GdkBitmap*);
	guint (*_gtk_signal_connect)(GtkObject*, const gchar*, GtkSignalFunc, gpointer);
	void (*_gtk_signal_disconnect)(GtkObject*, guint);
	void (*_gtk_table_attach)(GtkTable*, GtkWidget*, guint, guint, guint, guint, GtkAttachOptions, GtkAttachOptions, guint, guint);
	GtkWidget* (*_gtk_table_new)(guint, guint, gboolean);
	void (*_gtk_table_set_col_spacings)(GtkTable*, guint);
	GtkWidget* (*_gtk_vbox_new)(gboolean, gint);
	void (*_gtk_widget_destroy)(GtkWidget*);
	GdkVisual* (*_gtk_widget_get_visual)(GtkWidget*);
	void (*_gtk_widget_hide)(GtkWidget*);
	void (*_gtk_widget_ref)(GtkWidget*);
	void (*_gtk_widget_show)(GtkWidget*);
	void (*_gtk_widget_unref)(GtkWidget*);
	GtkWidget* (*_gtk_window_new)(GtkWindowType);
	void (*_gtk_window_set_modal)(GtkWindow*, gboolean);
	void (*_gtk_window_set_policy)(GtkWindow*, gint, gint, gint);
	void (*_gtk_window_set_position)(GtkWindow*, GtkWindowPosition);
	void (*_gtk_window_set_title)(GtkWindow*, const gchar*);

	void (*_gtk_init)(int*, char ***);
	void (*_gtk_main)(void);
	void (*_gtk_set_locale)(void);
	void (*_gtk_exit)(gint);
};
#endif /* !defined(_LINUX_CONFIG_H) */
