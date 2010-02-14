#include <stdio.h>
#include <cairo.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <glib/gi18n.h>

#include "ui-toolbar.h"

#include "tbo-window.h"
#include "ui-drawing.h"

#include "comic.h"
#include "page.h"
#include "frame.h"

#include "frame-tool.h"
#include "selector-tool.h"
#include "doodle-tool.h"


Frame *FRAME_VIEW = NULL;


gboolean
on_key_cb (GtkWidget    *widget,
           GdkEventKey  *event,
           TboWindow    *tbo)
{
    enum Tool tool;
    void **data = malloc (sizeof(void *)*3);
    data[0] = widget;
    data[1] = event;
    data[2] = tbo;

    tool = get_selected_tool ();
    tool_signal (tool, TOOL_KEY, data);
    free (data);

    update_drawing (tbo);
    tbo_window_update_status (tbo, 0, 0);
    return FALSE;
}

gboolean
on_expose_cb(GtkWidget      *widget,
             GdkEventExpose *event,
             TboWindow       *tbo)
{
    cairo_t *cr;
    int width, height;
    char *text = "TBO rulz!";
    cairo_text_extents_t extents;

    Frame *frame;
    GList *frame_list;
    Page *page;

    enum Tool tool;

    width = tbo->comic->width;
    height = tbo->comic->height;
    cr = gdk_cairo_create(GTK_LAYOUT (widget)->bin_window);

    // white background and black border
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_rectangle(cr, 0, 0, width+2, height+2);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_rectangle (cr, 1, 1, width, height);
    cairo_stroke (cr);

    page = tbo_comic_get_current_page (tbo->comic);

    if (!FRAME_VIEW)
    {
        for (frame_list = tbo_page_get_frames (page); frame_list; frame_list = frame_list->next)
        {
            // draw each frame
            frame = (Frame *)frame_list->data;
            tbo_frame_draw (frame, cr);
        }
    }
    else
    {
        tbo_frame_draw_scaled (FRAME_VIEW, cr, width, height);
    }

    // Update drawing helpers

    tool = get_selected_tool ();
    tool_signal (tool, TOOL_DRAWING, cr);

    // TBO rulz text example :P
    /*
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_select_font_face(cr, "Courier", CAIRO_FONT_SLANT_NORMAL,
            CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 90.0);

    cairo_text_extents(cr, text, &extents);
    cairo_move_to(cr, (width-extents.width)/2, (height+extents.height)/2);

    cairo_show_text(cr, text);
    */

    cairo_destroy(cr);

    return FALSE;
}

gboolean
on_move_cb (GtkWidget     *widget,
           GdkEventMotion *event,
           TboWindow      *tbo)
{

    enum Tool tool;
    void **data = malloc (sizeof(void *)*3);
    data[0] = widget;
    data[1] = event;
    data[2] = tbo;

    tool = get_selected_tool ();
    tool_signal (tool, TOOL_MOVE, data);
    free (data);

    update_drawing (tbo);
    tbo_window_update_status (tbo, (int)event->x, (int)event->y);
    return FALSE;
}

gboolean
on_click_cb (GtkWidget    *widget,
           GdkEventButton *event,
           TboWindow      *tbo)
{
    enum Tool tool;
    void **data = malloc (sizeof(void *)*3);
    data[0] = widget;
    data[1] = event;
    data[2] = tbo;

    tool = get_selected_tool ();
    tool_signal (tool, TOOL_CLICK, data);
    free (data);

    update_drawing (tbo);
    tbo_window_update_status (tbo, (int)event->x, (int)event->y);
    return FALSE;
}

gboolean
on_release_cb (GtkWidget    *widget,
           GdkEventButton   *event,
           TboWindow        *tbo)
{
    enum Tool tool;
    void **data = malloc (sizeof(void *)*3);
    data[0] = widget;
    data[1] = event;
    data[2] = tbo;

    tool = get_selected_tool ();
    tool_signal (tool, TOOL_RELEASE, data);
    free (data);

    update_drawing (tbo);
    tbo_window_update_status (tbo, (int)event->x, (int)event->y);
    return FALSE;
}

GtkWidget *
get_drawing_area (int width, int height)
{

    GtkWidget *drawing;

    drawing = gtk_layout_new(NULL, NULL);
    gtk_layout_set_size(GTK_LAYOUT (drawing), width, height);

    return drawing;
}

void
darea_connect_signals (TboWindow *tbo)
{
    GtkWidget *drawing;
    drawing = tbo->drawing;

    gtk_widget_add_events (drawing, GDK_BUTTON_PRESS_MASK |
                                    GDK_BUTTON_RELEASE_MASK |
                                    GDK_POINTER_MOTION_MASK);

    g_signal_connect(drawing, "expose-event",
            G_CALLBACK (on_expose_cb), tbo);

    g_signal_connect (drawing, "button_press_event",
            G_CALLBACK (on_click_cb), tbo);

    g_signal_connect (drawing, "button_release_event",
            G_CALLBACK (on_release_cb), tbo);

    g_signal_connect (drawing, "motion_notify_event",
            G_CALLBACK (on_move_cb), tbo);

    // key press event
    g_signal_connect (tbo->window, "key_press_event",
            G_CALLBACK (on_key_cb), tbo);

}

void
update_drawing (TboWindow *tbo)
{
    gtk_widget_queue_draw_area (tbo->drawing,
            0, 0,
            tbo->comic->width,
            tbo->comic->height);
}

void
set_frame_view (Frame *frame)
{
    FRAME_VIEW = frame;
}

Frame *
get_frame_view ()
{
    return FRAME_VIEW;
}
