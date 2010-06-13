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
static GtkWidget *BUBBLE_TREE = NULL;
static GtkWidget *DOODLE_TREE = NULL;
static GtkWidget *TREE = NULL;
static gdouble HADJUSTD, HADJUSTB = 0;
static gdouble VADJUSTD, VADJUSTB = 0;

void
doodle_tool_bubble_on_select (TboWindow *tbo)
{
    BUBBLE_MODE = TRUE;
    TREE = BUBBLE_TREE;
    doodle_tool_on_select (tbo);
}

void
doodle_tool_bubble_on_unselect (TboWindow *tbo)
{
    doodle_tool_on_unselect (tbo);
    BUBBLE_MODE = FALSE;
    BUBBLE_TREE = TREE;
}

gboolean
update_scroll_cb (gpointer p)
{
    GtkAdjustment *adjust;
    gdouble hadjust, vadjust;
    TboWindow *tbo = (TboWindow *)p;
    if (BUBBLE_MODE)
    {
        hadjust = HADJUSTB;
        vadjust = VADJUSTB;
    }
    else
    {
        hadjust = HADJUSTD;
        vadjust = VADJUSTD;
    }
    adjust = gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW (tbo->scroll2));
    gtk_adjustment_set_value (adjust, hadjust);
    adjust = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (tbo->scroll2));
    gtk_adjustment_set_value (adjust, vadjust);
    return FALSE;
}

void
doodle_tool_on_select (TboWindow *tbo)
{
    if (!BUBBLE_MODE)
        TREE = DOODLE_TREE;
    if (!TREE)
    {
        TREE = doodle_setup_tree (tbo, BUBBLE_MODE);
        gtk_widget_show_all (TREE);
        TREE = g_object_ref (TREE);
    }

    tbo_empty_tool_area (tbo);
    gtk_container_add (GTK_CONTAINER (tbo->toolarea), TREE);


    g_timeout_add (5, update_scroll_cb, tbo);
}

void
doodle_tool_on_unselect (TboWindow *tbo)
{
    GtkAdjustment *adjust;
    gdouble hadjust, vadjust;
    if (GTK_IS_WIDGET (TREE) && TREE->parent == GTK_WIDGET (tbo->toolarea))
    {
        adjust = gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW (tbo->scroll2));
        hadjust = gtk_adjustment_get_value (adjust);
        adjust = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (tbo->scroll2));
        vadjust = gtk_adjustment_get_value (adjust);

        if (!BUBBLE_MODE)
        {
            DOODLE_TREE = TREE;
            HADJUSTD = hadjust;
            VADJUSTD = vadjust;
        }
        else
        {
            HADJUSTB = hadjust;
            VADJUSTB = vadjust;
        }

        gtk_container_remove (GTK_CONTAINER (tbo->toolarea), TREE);
    }

    tbo_empty_tool_area (tbo);
}

void doodle_tool_on_move (GtkWidget *widget, GdkEventMotion *event, TboWindow *tbo){}
void doodle_tool_on_click (GtkWidget *widget, GdkEventButton *event, TboWindow *tbo){}
void doodle_tool_on_release (GtkWidget *widget, GdkEventButton *event, TboWindow *tbo){}
void doodle_tool_on_key (GtkWidget *widget, GdkEventKey *event, TboWindow *tbo){}
void doodle_tool_drawing (cairo_t *cr){}
