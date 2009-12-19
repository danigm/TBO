#ifndef __UI_TOOLBAR_
#define __UI_TOOLBAR_

#include <gtk/gtk.h>

gboolean toolbar_handler (GtkWidget *widget, GdkEvent *event, gpointer data);
GtkWidget *generate_toolbar (GtkUIManager *manager, GtkWidget *window);

#endif

