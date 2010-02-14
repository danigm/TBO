#ifndef __TBO_FRAME_TOOL__
#define __TBO_FRAME_TOOL__

#include <gtk/gtk.h>
#include <cairo.h>
#include "tbo-window.h"
#include "frame.h"

void frame_tool_on_select (TboWindow *tbo);
void frame_tool_on_move (GtkWidget *widget, GdkEventMotion *event, TboWindow *tbo);
void frame_tool_on_click (GtkWidget *widget, GdkEventButton *event, TboWindow *tbo);
void frame_tool_on_release (GtkWidget *widget, GdkEventButton *event, TboWindow *tbo);
void frame_tool_on_key (GtkWidget *widget, GdkEventKey *event, TboWindow *tbo);
void frame_tool_drawing (cairo_t *cr);
Frame * get_tmp_frame ();

#endif
