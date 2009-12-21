#ifndef __UI_TOOLBAR__
#define __UI_TOOLBAR__

#include <gtk/gtk.h>

gboolean toolbar_handler (GtkWidget *widget, GdkEvent *event, gpointer data);
GtkWidget *generate_toolbar (GtkWidget *window);

#endif

