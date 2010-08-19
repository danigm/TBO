/*
 * This file is part of TBO, a gnome comic editor
 * Copyright (C) 2010  Daniel Garcia Moreno <dani@danigm.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <glib.h>
#include <cairo.h>
#include <stdio.h>
#include <math.h>
#include "tbo-types.h"
#include "tbo-drawing.h"
#include "dnd.h"
#include "comic.h"
#include "frame.h"
#include "page.h"
#include "tbo-tool-bubble.h"
#include "tbo-tool-doodle.h"

G_DEFINE_TYPE (TboDrawing, tbo_drawing, GTK_TYPE_LAYOUT);

/* private methods */
static gboolean
expose_event (GtkWidget *widget, GdkEventExpose *event)
{
    cairo_t *cr;
    GdkWindow *window;
    gint w, h;
    TboDrawing *self = TBO_DRAWING (widget);

    cr = gdk_cairo_create(GTK_LAYOUT (widget)->bin_window);
    gdk_drawable_get_size (GDK_DRAWABLE (GTK_LAYOUT (widget)->bin_window), &w, &h);

    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_rectangle (cr, 0, 0, w, h);
    cairo_fill (cr);

    tbo_drawing_draw (TBO_DRAWING (widget), cr);

    tbo_tooltip_draw (cr);

    // Update drawing helpers
    if (self->tool)
        self->tool->drawing (self->tool, cr);

    cairo_destroy(cr);

    return FALSE;
}

static gboolean
motion_notify_event (GtkWidget *widget, GdkEventMotion *event)
{
    TboDrawing *self = TBO_DRAWING (widget);
    event->x = event->x / self->zoom;
    event->y = event->y / self->zoom;

    if (self->tool)
        self->tool->on_move (self->tool, widget, event);

    return FALSE;
}

static gboolean
button_press_event (GtkWidget *widget, GdkEventButton *event)
{
    TboDrawing *self = TBO_DRAWING (widget);
    event->x = event->x / self->zoom;
    event->y = event->y / self->zoom;

    if (self->tool) {
        if (TBO_IS_TOOL_BUBBLE (self->tool) || TBO_IS_TOOL_DOODLE (self->tool))
        {
            tbo_toolbar_set_selected_tool (self->tool->tbo->toolbar, TBO_TOOLBAR_SELECTOR);
        }
        self->tool->on_click (self->tool, widget, event);
    }

    return FALSE;
}

static gboolean
button_release_event (GtkWidget *widget, GdkEventButton *event)
{
    TboDrawing *self = TBO_DRAWING (widget);
    event->x = event->x / self->zoom;
    event->y = event->y / self->zoom;

    if (self->tool)
        self->tool->on_release (self->tool, widget, event);

    return FALSE;
}

/* init methods */

static void
tbo_drawing_init (TboDrawing *self)
{
    self->current_frame = NULL;
    self->zoom = 1;
    self->comic = NULL;
    self->tool = NULL;
}

static void
tbo_drawing_realize (GtkWidget *widget)
{
    GdkWindow *bin_window;

    if (GTK_WIDGET_CLASS (tbo_drawing_parent_class)->realize)
            (* GTK_WIDGET_CLASS (tbo_drawing_parent_class)->realize) (widget);

    bin_window = gtk_layout_get_bin_window (GTK_LAYOUT (widget));
    gdk_window_set_events (bin_window,
                           (gdk_window_get_events (bin_window) |
                            GDK_BUTTON_PRESS_MASK |
                            GDK_BUTTON_RELEASE_MASK |
                            GDK_POINTER_MOTION_MASK));
}

static void
tbo_drawing_finalize (GObject *self)
{
    /* Chain up to the parent class */
    G_OBJECT_CLASS (tbo_drawing_parent_class)->finalize (self);
}

static void
tbo_drawing_class_init (TboDrawingClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    widget_class->expose_event = expose_event;
    widget_class->motion_notify_event = motion_notify_event;
    widget_class->button_press_event = button_press_event;
    widget_class->button_release_event = button_release_event;
    widget_class->realize = tbo_drawing_realize;
    gobject_class->finalize = tbo_drawing_finalize;
}

/* object functions */

GtkWidget *
tbo_drawing_new ()
{
    GtkWidget *drawing;
    drawing = g_object_new (TBO_TYPE_DRAWING, NULL);
    return drawing;
}

GtkWidget *
tbo_drawing_new_with_params (Comic *comic)
{
    GtkWidget *drawing = tbo_drawing_new ();
    TBO_DRAWING (drawing)->comic = comic;
    gtk_layout_set_size (GTK_LAYOUT (drawing), comic->width+2, comic->height+2);

    return drawing;
}

void
tbo_drawing_update (TboDrawing *self)
{
    gtk_widget_queue_draw_area (GTK_WIDGET (self),
            0, 0,
            GTK_WIDGET (self)->allocation.width,
            GTK_WIDGET (self)->allocation.height);
}

void
tbo_drawing_set_current_frame (TboDrawing *self, Frame *frame)
{
    self->current_frame = frame;
}

Frame *
tbo_drawing_get_current_frame (TboDrawing *self)
{
    return self->current_frame;
}

void
tbo_drawing_draw (TboDrawing *self, cairo_t *cr)
{
    Frame *frame;
    GList *frame_list;
    Page *page;

    int w, h;

    w = self->comic->width;
    h = self->comic->height;
    // white background
    if (tbo_drawing_get_current_frame (self))
        cairo_set_source_rgb(cr, 0, 0, 0);
    else
        cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_rectangle(cr, 0, 0, w*self->zoom, h*self->zoom);
    cairo_fill(cr);

    cairo_scale (cr, self->zoom, self->zoom);

    page = tbo_comic_get_current_page (self->comic);

    if (!self->current_frame)
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
        tbo_frame_draw_scaled (self->current_frame, cr, w, h);
    }
}

/* TODO this method should be in TboPage */
void
tbo_drawing_draw_page (TboDrawing *self, cairo_t *cr, Page *page, gint w, gint h)
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
tbo_drawing_zoom_in (TboDrawing *self)
{
    self->zoom += ZOOM_STEP;
    tbo_drawing_adjust_scroll (self);
}

void
tbo_drawing_zoom_out (TboDrawing *self)
{
    self->zoom -= ZOOM_STEP;
    tbo_drawing_adjust_scroll (self);
}

void
tbo_drawing_zoom_100 (TboDrawing *self)
{
    self->zoom = 1;
    tbo_drawing_adjust_scroll (self);
}

void
tbo_drawing_zoom_fit (TboDrawing *self)
{
    float z1, z2;
    int w, h;
    w = GTK_WIDGET (self)->allocation.width;
    h = GTK_WIDGET (self)->allocation.height;

    z1 = fabs ((float)w / (float)self->comic->width);
    z2 = fabs ((float)h / (float)self->comic->height);
    self->zoom = z1 < z2 ? z1 : z2;
    tbo_drawing_adjust_scroll (self);
}

gdouble
tbo_drawing_get_zoom (TboDrawing *self)
{
    return self->zoom;
}

void
tbo_drawing_adjust_scroll (TboDrawing *self)
{
    if (!self->comic)
        return;
    gtk_layout_set_size (GTK_LAYOUT (self), self->comic->width*self->zoom, self->comic->height*self->zoom);
    tbo_drawing_update (self);
}

void
tbo_drawing_init_dnd (TboDrawing *self, TboWindow *tbo)
{
    gtk_drag_dest_set (GTK_WIDGET (self), GTK_DEST_DEFAULT_ALL, TARGET_LIST, N_TARGETS, GDK_ACTION_COPY);
    g_signal_connect (self, "drag-data-received", G_CALLBACK(drag_data_received_handl), tbo);
}
