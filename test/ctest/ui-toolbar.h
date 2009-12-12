#ifndef __UI_TOOLBAR_
#define __UI_TOOLBAR_

#include <gtk/gtk.h>

gboolean toolbar_handler (GtkWidget *widget, GdkEvent *event, gpointer data);
void add_new_image_toolbar_item (GtkWidget *toolbar, const gchar *stock_id, void *cb, gpointer data);
GtkWidget *generate_toolbar ();

#endif

