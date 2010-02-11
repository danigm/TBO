#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <cairo.h>
#include "selector-tool.h"
#include "tbo-window.h"
#include "page.h"
#include "frame.h"
#include "comic.h"
#include "ui-drawing.h"

void doodle_tool_on_move (GtkWidget *widget, GdkEventMotion *event, TboWindow *tbo){}
void doodle_tool_on_click (GtkWidget *widget, GdkEventButton *event, TboWindow *tbo){}
void doodle_tool_on_release (GtkWidget *widget, GdkEventButton *event, TboWindow *tbo){}
void doodle_tool_on_key (GtkWidget *widget, GdkEventKey *event, TboWindow *tbo){}
void doodle_tool_drawing (cairo_t *cr){}
