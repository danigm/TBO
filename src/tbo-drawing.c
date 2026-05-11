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
#include "tbo-tooltip.h"

G_DEFINE_TYPE (TboDrawing, tbo_drawing, GTK_TYPE_DRAWING_AREA);

static void tbo_drawing_set_window_pointer (TboDrawing *self, TboWindow *tbo);
static void tbo_drawing_set_comic_pointer (TboDrawing *self, Comic *comic);

static gdouble
clamp_zoom (gdouble zoom)
{
    if (!isfinite (zoom) || zoom < ZOOM_STEP)
        return ZOOM_STEP;

    return zoom;
}

static gboolean
get_frame_view_transform (TboDrawing *self, Frame *frame, gdouble *scale, gint *base_x, gint *base_y)
{
    gint width;
    gint height;
    gint centered_x;
    gint centered_y;
    gdouble scale_x;
    gdouble scale_y;
    gdouble current_scale;

    if (self == NULL || self->comic == NULL || frame == NULL)
        return FALSE;

    width = tbo_comic_get_width (self->comic);
    height = tbo_comic_get_height (self->comic);
    if (width <= 20 || height <= 20 ||
        tbo_frame_get_width (frame) <= 0 || tbo_frame_get_height (frame) <= 0)
        return FALSE;

    scale_x = (width - 20) / (gdouble) tbo_frame_get_width (frame);
    scale_y = (height - 20) / (gdouble) tbo_frame_get_height (frame);
    current_scale = MIN (scale_x, scale_y);
    if (!isfinite (current_scale) || current_scale <= 0.0)
        return FALSE;

    centered_x = (gint) ((width / 2.0) - (tbo_frame_get_width (frame) * current_scale / 2.0));
    centered_y = (gint) ((height / 2.0) - (tbo_frame_get_height (frame) * current_scale / 2.0));

    if (scale != NULL)
        *scale = current_scale;
    if (base_x != NULL)
        *base_x = centered_x / current_scale;
    if (base_y != NULL)
        *base_y = centered_y / current_scale;

    return TRUE;
}

static void
tbo_drawing_set_current_frame_pointer (TboDrawing *self, Frame *frame)
{
    if (self->current_frame == frame)
        return;

    if (self->current_frame != NULL)
    {
        g_object_remove_weak_pointer (G_OBJECT (self->current_frame),
                                      (gpointer *) &self->current_frame);
    }

    self->current_frame = frame;

    if (self->current_frame != NULL)
    {
        g_object_add_weak_pointer (G_OBJECT (self->current_frame),
                                   (gpointer *) &self->current_frame);
    }
}

static void
tbo_drawing_set_window_pointer (TboDrawing *self, TboWindow *tbo)
{
    self->tbo = tbo;
}

static void
tbo_drawing_set_comic_pointer (TboDrawing *self, Comic *comic)
{
    if (self->comic == comic)
        return;

    if (self->comic != NULL)
    {
        g_object_remove_weak_pointer (G_OBJECT (self->comic),
                                      (gpointer *) &self->comic);
    }

    self->comic = comic;

    if (self->comic != NULL)
    {
        g_object_add_weak_pointer (G_OBJECT (self->comic),
                                   (gpointer *) &self->comic);
    }
}

static gboolean
queue_redraw_cb (gpointer data)
{
    TboDrawing *self = TBO_DRAWING (data);

    self->redraw_source_id = 0;
    gtk_widget_queue_draw (GTK_WIDGET (self));
    return G_SOURCE_REMOVE;
}

static void
get_view_size (TboDrawing *self, gint *width, gint *height)
{
    GtkWidget *scrolled;

    scrolled = gtk_widget_get_ancestor (GTK_WIDGET (self), GTK_TYPE_SCROLLED_WINDOW);
    if (scrolled != NULL)
    {
        *width = gtk_widget_get_width (scrolled);
        *height = gtk_widget_get_height (scrolled);
    }
    else
    {
        *width = gtk_widget_get_width (GTK_WIDGET (self));
        *height = gtk_widget_get_height (GTK_WIDGET (self));
    }
}

/* private methods */
static void
draw_func (GtkDrawingArea *area, cairo_t *cr, gint width, gint height, gpointer data)
{
    TboDrawing *self = TBO_DRAWING (area);

    cairo_set_source_rgb (cr, 0, 0, 0);
    cairo_rectangle (cr, 0, 0, width, height);
    cairo_fill (cr);

    tbo_drawing_draw (TBO_DRAWING (area), cr);

    tbo_tooltip_draw (cr, self);

    // Update drawing helpers
    if (self->tool)
        self->tool->drawing (self->tool, cr);
}

static void
motion_notify_cb (GtkEventControllerMotion *controller, gdouble x, gdouble y, gpointer user_data)
{
    TboDrawing *self = TBO_DRAWING (user_data);
    gdouble zoom = clamp_zoom (self->zoom);
    TboPointerEvent event = {
        .x = x / zoom,
        .y = y / zoom,
        .button = 0,
        .n_press = 0,
        .state = gtk_event_controller_get_current_event_state (GTK_EVENT_CONTROLLER (controller)),
    };

    if (self->tool)
        self->tool->on_move (self->tool, GTK_WIDGET (self), &event);
}

static void
click_pressed_cb (GtkGestureClick *gesture, gint n_press, gdouble x, gdouble y, gpointer user_data)
{
    TboDrawing *self = TBO_DRAWING (user_data);
    gdouble zoom = clamp_zoom (self->zoom);
    TboPointerEvent event = {
        .x = x / zoom,
        .y = y / zoom,
        .button = gtk_gesture_single_get_current_button (GTK_GESTURE_SINGLE (gesture)),
        .n_press = n_press,
        .state = gtk_event_controller_get_current_event_state (GTK_EVENT_CONTROLLER (gesture)),
    };

    gtk_widget_grab_focus (GTK_WIDGET (self));

    if (self->tool) {
        if (TBO_IS_TOOL_BUBBLE (self->tool) || TBO_IS_TOOL_DOODLE (self->tool))
        {
            tbo_toolbar_set_selected_tool (self->tool->tbo->toolbar, TBO_TOOLBAR_SELECTOR);
        }
        self->tool->on_click (self->tool, GTK_WIDGET (self), &event);
    }
}

static void
click_released_cb (GtkGestureClick *gesture, gint n_press, gdouble x, gdouble y, gpointer user_data)
{
    TboDrawing *self = TBO_DRAWING (user_data);
    gdouble zoom = clamp_zoom (self->zoom);
    TboPointerEvent event = {
        .x = x / zoom,
        .y = y / zoom,
        .button = gtk_gesture_single_get_current_button (GTK_GESTURE_SINGLE (gesture)),
        .n_press = n_press,
        .state = gtk_event_controller_get_current_event_state (GTK_EVENT_CONTROLLER (gesture)),
    };

    if (self->tool)
        self->tool->on_release (self->tool, GTK_WIDGET (self), &event);
}

/* init methods */

static void
tbo_drawing_init (TboDrawing *self)
{
    GtkEventController *motion;
    GtkGesture *click;

    self->current_frame = NULL;
    self->zoom = 1;
    self->comic = NULL;
    self->tbo = NULL;
    self->tooltip = NULL;
    self->tooltip_x = 0;
    self->tooltip_y = 0;
    self->tooltip_alpha = 0.0;
    self->tooltip_timeout_id = 0;
    self->tool = NULL;
    self->redraw_source_id = 0;
    gtk_widget_set_focusable (GTK_WIDGET (self), TRUE);

    gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (self), draw_func, NULL, NULL);

    motion = gtk_event_controller_motion_new ();
    g_signal_connect (motion, "motion", G_CALLBACK (motion_notify_cb), self);
    gtk_widget_add_controller (GTK_WIDGET (self), motion);

    click = gtk_gesture_click_new ();
    g_signal_connect (click, "pressed", G_CALLBACK (click_pressed_cb), self);
    g_signal_connect (click, "released", G_CALLBACK (click_released_cb), self);
    gtk_widget_add_controller (GTK_WIDGET (self), GTK_EVENT_CONTROLLER (click));
}

static void
tbo_drawing_finalize (GObject *self)
{
    TboDrawing *drawing = TBO_DRAWING (self);

    if (drawing->redraw_source_id != 0)
        g_source_remove (drawing->redraw_source_id);

    if (drawing->tooltip_timeout_id != 0)
        g_source_remove (drawing->tooltip_timeout_id);
    if (drawing->tooltip != NULL)
        g_string_free (drawing->tooltip, TRUE);

    tbo_drawing_set_window_pointer (drawing, NULL);
    tbo_drawing_set_comic_pointer (drawing, NULL);
    tbo_drawing_set_current_frame_pointer (drawing, NULL);

    /* Chain up to the parent class */
    G_OBJECT_CLASS (tbo_drawing_parent_class)->finalize (self);
}

static void
tbo_drawing_class_init (TboDrawingClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = tbo_drawing_finalize;
}

/* object functions */

GtkWidget *
tbo_drawing_new (void)
{
    GtkWidget *drawing;
    drawing = g_object_new (TBO_TYPE_DRAWING, NULL);
    return drawing;
}

GtkWidget *
tbo_drawing_new_with_params (Comic *comic)
{
    GtkWidget *drawing = tbo_drawing_new ();
    tbo_drawing_set_comic (TBO_DRAWING (drawing), comic);
    gtk_widget_set_size_request (drawing,
                                 tbo_comic_get_width (comic) + 2,
                                 tbo_comic_get_height (comic) + 2);

    return drawing;
}

void
tbo_drawing_set_comic (TboDrawing *self, Comic *comic)
{
    tbo_drawing_set_comic_pointer (self, comic);
}

Comic *
tbo_drawing_get_comic (TboDrawing *self)
{
    return self->comic;
}

void
tbo_drawing_update (TboDrawing *self)
{
    if (self->redraw_source_id != 0)
        return;

    self->redraw_source_id = g_idle_add_full (G_PRIORITY_DEFAULT_IDLE,
                                              queue_redraw_cb,
                                              g_object_ref (self),
                                              g_object_unref);
}

void
tbo_drawing_set_current_frame (TboDrawing *self, Frame *frame)
{
    tbo_drawing_set_current_frame_pointer (self, frame);
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

    if (self->comic == NULL)
        return;

    w = tbo_comic_get_width (self->comic);
    h = tbo_comic_get_height (self->comic);
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
tbo_drawing_draw_page (TboDrawing *self, cairo_t *cr, Page *page, gdouble w, gdouble h)
{
    Frame *frame;
    GList *frame_list;
    gdouble scale_x;
    gdouble scale_y;

    // white background
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_rectangle(cr, 0, 0, w, h);
    cairo_fill(cr);

    if (self->comic == NULL || tbo_comic_get_width (self->comic) <= 0 || tbo_comic_get_height (self->comic) <= 0)
        return;

    scale_x = w / tbo_comic_get_width (self->comic);
    scale_y = h / tbo_comic_get_height (self->comic);

    cairo_save (cr);
    cairo_scale (cr, scale_x, scale_y);

    for (frame_list = tbo_page_get_frames (page); frame_list; frame_list = frame_list->next)
    {
        // draw each frame
        frame = (Frame *)frame_list->data;
        tbo_frame_draw (frame, cr);
    }

    cairo_restore (cr);
}

void
tbo_drawing_zoom_in (TboDrawing *self)
{
    self->zoom = clamp_zoom (self->zoom + ZOOM_STEP);
    tbo_drawing_adjust_scroll (self);
}

void
tbo_drawing_zoom_out (TboDrawing *self)
{
    self->zoom = clamp_zoom (self->zoom - ZOOM_STEP);
    tbo_drawing_adjust_scroll (self);
}

void
tbo_drawing_zoom_100 (TboDrawing *self)
{
    self->zoom = clamp_zoom (1);
    tbo_drawing_adjust_scroll (self);
}

void
tbo_drawing_zoom_fit (TboDrawing *self)
{
    float z1, z2;
    int w, h;

    get_view_size (self, &w, &h);

    z1 = fabs ((float)w / (float)tbo_comic_get_width (self->comic));
    z2 = fabs ((float)h / (float)tbo_comic_get_height (self->comic));
    self->zoom = clamp_zoom (z1 < z2 ? z1 : z2);
    tbo_drawing_adjust_scroll (self);
}

gdouble
tbo_drawing_get_zoom (TboDrawing *self)
{
    return self->zoom;
}

gdouble
tbo_drawing_get_current_frame_scale (TboDrawing *self)
{
    gdouble scale;

    if (!get_frame_view_transform (self, self->current_frame, &scale, NULL, NULL))
        return 1.0;

    return scale;
}

gboolean
tbo_drawing_view_to_frame (TboDrawing *self, gdouble view_x, gdouble view_y, gint *frame_x, gint *frame_y)
{
    gdouble scale;
    gint base_x;
    gint base_y;

    if (!get_frame_view_transform (self, self->current_frame, &scale, &base_x, &base_y))
        return FALSE;

    if (frame_x != NULL)
        *frame_x = (view_x / scale) - base_x;
    if (frame_y != NULL)
        *frame_y = (view_y / scale) - base_y;

    return TRUE;
}

void
tbo_drawing_get_object_relative (TboDrawing *self, TboObjectBase *obj, gint *x, gint *y, gint *w, gint *h)
{
    gdouble scale;
    gint base_x;
    gint base_y;

    if (!get_frame_view_transform (self, self->current_frame, &scale, &base_x, &base_y))
    {
        if (x != NULL)
            *x = 0;
        if (y != NULL)
            *y = 0;
        if (w != NULL)
            *w = 0;
        if (h != NULL)
            *h = 0;
        return;
    }

    if (x != NULL)
        *x = (base_x + obj->x) * scale;
    if (y != NULL)
        *y = (base_y + obj->y) * scale;
    if (w != NULL)
        *w = obj->width * scale;
    if (h != NULL)
        *h = obj->height * scale;
}

gboolean
tbo_drawing_point_inside_object (TboDrawing *self, TboObjectBase *obj, gint x, gint y)
{
    gint ox;
    gint oy;
    gint ow;
    gint oh;
    gint xnew1;
    gint ynew1;
    gint xnew2;
    gint ynew2;
    gint xnew3;
    gint ynew3;
    gint xmax;
    gint ymax;
    gint xmin;
    gint ymin;

    tbo_drawing_get_object_relative (self, obj, &ox, &oy, &ow, &oh);
    xnew1 = ox + (ow * cos (obj->angle));
    ynew1 = oy + (ow * sin (obj->angle));
    xnew2 = ox + (-oh * sin (obj->angle));
    ynew2 = oy + (oh * cos (obj->angle));
    xnew3 = ox + (ow * cos (obj->angle) - oh * sin (obj->angle));
    ynew3 = oy + (oh * cos (obj->angle) + ow * sin (obj->angle));

    xmax = MAX (MAX (ox, xnew1), MAX (xnew2, xnew3));
    ymax = MAX (MAX (oy, ynew1), MAX (ynew2, ynew3));
    xmin = MIN (MIN (ox, xnew1), MIN (xnew2, xnew3));
    ymin = MIN (MIN (oy, ynew1), MIN (ynew2, ynew3));

    return x >= xmin && x <= xmax && y >= ymin && y <= ymax;
}

void
tbo_drawing_adjust_scroll (TboDrawing *self)
{
    gint width;
    gint height;

    if (!self->comic)
        return;

    self->zoom = clamp_zoom (self->zoom);

    width = MAX (1, ceil (tbo_comic_get_width (self->comic) * self->zoom));
    height = MAX (1, ceil (tbo_comic_get_height (self->comic) * self->zoom));
    gtk_widget_set_size_request (GTK_WIDGET (self), width, height);
    tbo_drawing_update (self);
}

void
tbo_drawing_init_dnd (TboDrawing *self, TboWindow *tbo)
{
    tbo_drawing_set_window_pointer (self, tbo);
    tbo_dnd_setup_drawing_dest (self, tbo);
}
