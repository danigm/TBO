#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <cairo.h>
#include "selector-tool.h"
#include "tbo-window.h"
#include "page.h"
#include "frame.h"
#include "comic.h"
#include "ui-drawing.h"
#include "doodle-treeview.h"
#include "doodle-tool.h"

static gboolean BUBBLE_MODE = FALSE;

void
doodle_tool_bubble_on_select (TboWindow *tbo)
{
    BUBBLE_MODE = TRUE;
    doodle_tool_on_select (tbo);
}

void
doodle_tool_bubble_on_unselect (TboWindow *tbo)
{
    doodle_tool_on_unselect (tbo);
    BUBBLE_MODE = FALSE;
}

void
doodle_tool_on_select (TboWindow *tbo)
{
    GtkWidget *tree;

    tree = doodle_setup_tree (tbo, BUBBLE_MODE);

    gtk_widget_show_all (tree);

    tbo_empty_tool_area (tbo);
    gtk_container_add (GTK_CONTAINER (tbo->toolarea), tree);
}

void
doodle_tool_on_unselect (TboWindow *tbo)
{
    tbo_empty_tool_area (tbo);
    doodle_free_all ();
}

void doodle_tool_on_move (GtkWidget *widget, GdkEventMotion *event, TboWindow *tbo){}
void doodle_tool_on_click (GtkWidget *widget, GdkEventButton *event, TboWindow *tbo){}
void doodle_tool_on_release (GtkWidget *widget, GdkEventButton *event, TboWindow *tbo){}
void doodle_tool_on_key (GtkWidget *widget, GdkEventKey *event, TboWindow *tbo){}
void doodle_tool_drawing (cairo_t *cr){}
