#ifndef __TBO_WINDOW__
#define __TBO_WINDOW__

#include <gtk/gtk.h>
#include "tbo-types.h"

typedef struct
{
    GtkWidget *window;
    GtkWidget *dw_scroll;
    GtkWidget *drawing;
    GtkWidget *status;
    GtkWidget *vbox;
    Comic *comic;

} TboWindow;

TboWindow *tbo_window_new (GtkWidget *window, GtkWidget *dw_scroll, GtkWidget *status, GtkWidget *vbox, Comic *comic);
void tbo_window_free (TboWindow *tbo);
gboolean tbo_window_free_cb (GtkWidget *widget, GdkEventExpose *event, TboWindow *tbo);
GdkPixbuf *create_pixbuf (const gchar * filename);
TboWindow * tbo_new_tbo (int width, int height);
void tbo_window_update_status (TboWindow *tbo, int x, int y);

#endif
