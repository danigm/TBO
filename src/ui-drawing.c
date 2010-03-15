#include <stdio.h>
#include <cairo.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
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
#include "dnd.h"


Frame *FRAME_VIEW = NULL;
float ZOOM_STEP = 0.05;
float ZOOM = 1;

void
tbo_drawing_draw_page (cairo_t *cr, Page *page, int w, int h)
{
    Frame *frame;
    GList *frame_list;

    // white background
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_rectangle(cr, 0, 0, w, h);
    cairo_fill(cr);

    for (frame_list = tbo_page_get_frames (page); frame_list; frame_list = frame_list->next)
    {
        // draw each frame
        frame = (Frame *)frame_list->data;
        tbo_frame_draw (frame, cr);
    }
}

void
tbo_drawing_draw (cairo_t *cr, TboWindow *tbo)
{
    Frame *frame;
    GList *frame_list;
    Page *page;

    int w, h;

    w = tbo->comic->width;
    h = tbo->comic->height;
    // white background
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_rectangle(cr, 0, 0, w*ZOOM, h*ZOOM);
    cairo_fill(cr);

    cairo_scale (cr, ZOOM, ZOOM);

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
        tbo_frame_draw_scaled (FRAME_VIEW, cr, w, h);
    }
}

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

    switch (event->keyval)
    {
        case GDK_plus:
            tbo_drawing_zoom_in (tbo);
            break;
        case GDK_minus:
            tbo_drawing_zoom_out (tbo);
            break;
        default:
            break;
    }
    return FALSE;
}

gboolean
on_expose_cb (GtkWidget      *widget,
              GdkEventExpose *event,
              TboWindow       *tbo)
{
    cairo_t *cr;
    int w, h;
    enum Tool tool;
    GdkWindow *window;

    cr = gdk_cairo_create(GTK_LAYOUT (widget)->bin_window);

    window = gtk_widget_get_parent_window (GTK_WIDGET (widget));
    gdk_drawable_get_size (GDK_DRAWABLE (window), &w, &h);

    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_rectangle (cr, 0, 0, w, h);
    cairo_fill (cr);

    tbo_drawing_draw (cr, tbo);

    // Update drawing helpers
    tool = get_selected_tool ();
    tool_signal (tool, TOOL_DRAWING, cr);

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
    event->x = event->x / ZOOM;
    event->y = event->y / ZOOM;
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
    event->x = event->x / ZOOM;
    event->y = event->y / ZOOM;
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
    event->x = event->x / ZOOM;
    event->y = event->y / ZOOM;

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
    gtk_layout_set_size (GTK_LAYOUT (drawing), width+2, height+2);

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

    // drag & drop
    gtk_drag_dest_set (drawing, GTK_DEST_DEFAULT_ALL, TARGET_LIST, N_TARGETS, GDK_ACTION_COPY);
    g_signal_connect (drawing, "drag-data-received", G_CALLBACK(drag_data_received_handl), tbo);
}

void
update_drawing (TboWindow *tbo)
{
    int w, h;
    w = tbo->comic->width * ZOOM;
    h = tbo->comic->height * ZOOM;

    gtk_widget_queue_draw_area (tbo->drawing,
            0, 0,
            w, h);
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

void tbo_drawing_zoom_in (TboWindow *tbo)
{
    ZOOM += ZOOM_STEP;
    gtk_layout_set_size (GTK_LAYOUT (tbo->drawing), tbo->comic->width*ZOOM, tbo->comic->height*ZOOM);
    update_drawing (tbo);
}

void tbo_drawing_zoom_out (TboWindow *tbo)
{
    ZOOM -= ZOOM_STEP;
    gtk_layout_set_size (GTK_LAYOUT (tbo->drawing), tbo->comic->width*ZOOM, tbo->comic->height*ZOOM);
    update_drawing (tbo);
}
