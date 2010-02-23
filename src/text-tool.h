#ifndef __TBO_TEXT_TOOL__
#define __TBO_TEXT_TOOL__

#include <gtk/gtk.h>
#include <cairo.h>
#include "tbo-window.h"

void text_tool_on_select (TboWindow *tbo);
void text_tool_on_unselect (TboWindow *tbo);
void text_tool_on_move (GtkWidget *widget, GdkEventMotion *event, TboWindow *tbo);
void text_tool_on_click (GtkWidget *widget, GdkEventButton *event, TboWindow *tbo);
void text_tool_on_release (GtkWidget *widget, GdkEventButton *event, TboWindow *tbo);
void text_tool_on_key (GtkWidget *widget, GdkEventKey *event, TboWindow *tbo);
void text_tool_drawing (cairo_t *cr);

#endif
