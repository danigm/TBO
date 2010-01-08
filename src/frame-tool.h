#ifndef __TBO_FRAME_TOOL__
#define __TBO_FRAME_TOOL__

#include <gtk/gtk.h>
#include "tbo-window.h"
#include "frame.h"

void frame_tool_on_move (GtkWidget *widget, GdkEventMotion *event, TboWindow *tbo);
void frame_tool_on_click (GtkWidget *widget, GdkEventButton *event, TboWindow *tbo);
void frame_tool_on_release (GtkWidget *widget, GdkEventButton *event, TboWindow *tbo);
Frame * get_tmp_frame ();

#endif
