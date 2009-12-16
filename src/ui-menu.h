#ifndef __UI_MENU_
#define __UI_MENU_

#include <gtk/gtk.h>

gboolean menu_handler (GtkWidget *widget, GdkEvent *event, gpointer data);
gboolean close_cb (GtkWidget *widget, GdkEvent *event, gpointer data);
void add_new_menu_item (GtkWidget *menu, const char *label, void *cb, gpointer data);
void add_new_image_menu_item (GtkWidget *menu, const gchar *stock_id, void *cb, gpointer data);
GtkWidget *generate_menu ();

#endif
