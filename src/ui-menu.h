#ifndef __UI_MENU_
#define __UI_MENU_

#include <gtk/gtk.h>

gboolean menu_handler (GtkWidget *widget, GdkEvent *event, gpointer data);
gboolean close_cb (GtkWidget *widget, GdkEvent *event, gpointer data);
GtkWidget *generate_menu (GtkUIManager *manager, GtkWidget *window);

#endif
