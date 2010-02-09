#ifndef __TBO_SELECTOR_TOOL__
#define __TBO_SELECTOR_TOOL__

#include <gtk/gtk.h>
#include <cairo.h>
#include "tbo-window.h"

void selector_tool_on_move (GtkWidget *widget, GdkEventMotion *event, TboWindow *tbo);
void selector_tool_on_click (GtkWidget *widget, GdkEventButton *event, TboWindow *tbo);
void selector_tool_on_release (GtkWidget *widget, GdkEventButton *event, TboWindow *tbo);
void selector_tool_on_key (GtkWidget *widget, GdkEventKey *event, TboWindow *tbo);
void selector_tool_drawing (cairo_t *cr);
Frame *selector_tool_get_selected_frame ();
void frame_view_on_click (GtkWidget *widget, GdkEventButton *event, TboWindow *tbo);
void page_view_on_click (GtkWidget *widget, GdkEventButton *event, TboWindow *tbo);
void frame_view_drawing (cairo_t *cr);
void page_view_drawing (cairo_t *cr);
void frame_view_on_key (GtkWidget *widget, GdkEventKey *event, TboWindow *tbo);
void page_view_on_key (GtkWidget *widget, GdkEventKey *event, TboWindow *tbo);

#endif
