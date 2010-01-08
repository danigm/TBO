#ifndef __UI_DRAWING__
#define __UI_DRAWING__

#include <gtk/gtk.h>
#include <cairo.h>
#include "tbo-window.h"


gboolean on_expose_cb(GtkWidget *widget, GdkEventExpose *event, TboWindow *tbo);
gboolean on_move_cb(GtkWidget *widget, GdkEventMotion *event, TboWindow *tbo);
gboolean on_click_cb(GtkWidget *widget, GdkEventButton *event, TboWindow *tbo);
gboolean on_release_cb(GtkWidget *widget, GdkEventButton *event, TboWindow *tbo);
void darea_connect_signals (TboWindow *tbo);
GtkWidget * get_drawing_area (int width, int height);
void update_drawing (TboWindow *tbo);

#endif
