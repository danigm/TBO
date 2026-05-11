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


#include <gtk/gtk.h>
#include <math.h>
#include <cairo.h>
#include <stdio.h>
#include <string.h>
#include "frame.h"
#include "tbo-types.h"
#include "tbo-list-utils.h"
#include "tbo-object-base.h"
#include "tbo-utils.h"

struct _Frame
{
    GObject parent_instance;

    gint x;
    gint y;
    gint width;
    gint height;
    gboolean border;
    Color color;
    GList *objects;
};

struct _FrameClass
{
    GObjectClass parent_class;
};

typedef struct
{
    cairo_t *cr;
    Frame *frame;
} DrawObjectsData;

G_DEFINE_TYPE (Frame, tbo_frame, G_TYPE_OBJECT);

static const Color DEFAULT_FRAME_COLOR = {1, 1, 1};
static int BASE_X = 0;
static int BASE_Y = 0;
static float SCALE_FACTOR = 1;

static void
draw_objects (gpointer data, gpointer user_data)
{
    DrawObjectsData *draw_data = user_data;
    TboObjectBase *obj = TBO_OBJECT_BASE (data);

    obj->draw (obj, draw_data->frame, draw_data->cr);
}

static void
tbo_frame_dispose (GObject *object)
{
    Frame *self = TBO_FRAME (object);

    if (self->objects != NULL)
    {
        g_list_free_full (self->objects, g_object_unref);
        self->objects = NULL;
    }

    G_OBJECT_CLASS (tbo_frame_parent_class)->dispose (object);
}

static void
tbo_frame_init (Frame *self)
{
    self->x = 0;
    self->y = 0;
    self->width = 0;
    self->height = 0;
    self->border = TRUE;
    self->color = DEFAULT_FRAME_COLOR;
    self->objects = NULL;
}

static void
tbo_frame_class_init (FrameClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->dispose = tbo_frame_dispose;
}

void
tbo_frame_set_scale_factor (Frame *frame, int width, int height)
{
    float scale_factor = 1.;
    float scale_factor2 = 1.;

    scale_factor = (width - 20) / (float) frame->width;
    scale_factor2 = (height - 20) / (float) frame->height;

    scale_factor = scale_factor > scale_factor2 ? scale_factor2 : scale_factor;

    SCALE_FACTOR = scale_factor;
}

int
tbo_frame_get_x_centered (Frame *frame, int width)
{
    int x;

    x = (width / 2.0) - (frame->width * SCALE_FACTOR / 2.0);
    x = x / SCALE_FACTOR;
    return x;
}

int
tbo_frame_get_y_centered (Frame *frame, int height)
{
    int y;

    y = (height / 2.0) - (frame->height * SCALE_FACTOR / 2.0);
    y = y / SCALE_FACTOR;
    return y;
}

Frame *
tbo_frame_new (int x, int y, int width, int height)
{
    Frame *new_frame = g_object_new (TBO_TYPE_FRAME, NULL);

    new_frame->x = x;
    new_frame->y = y;
    new_frame->width = width;
    new_frame->height = height;

    return new_frame;
}

void
tbo_frame_free (Frame *frame)
{
    if (frame != NULL)
        g_object_unref (frame);
}

int
tbo_frame_get_x (Frame *frame)
{
    return frame->x;
}

int
tbo_frame_get_y (Frame *frame)
{
    return frame->y;
}

int
tbo_frame_get_width (Frame *frame)
{
    return frame->width;
}

int
tbo_frame_get_height (Frame *frame)
{
    return frame->height;
}

void
tbo_frame_get_bounds (Frame *frame, int *x, int *y, int *width, int *height)
{
    if (x != NULL)
        *x = frame->x;
    if (y != NULL)
        *y = frame->y;
    if (width != NULL)
        *width = frame->width;
    if (height != NULL)
        *height = frame->height;
}

void
tbo_frame_set_position (Frame *frame, int x, int y)
{
    frame->x = x;
    frame->y = y;
}

void
tbo_frame_set_size (Frame *frame, int width, int height)
{
    frame->width = width;
    frame->height = height;
}

void
tbo_frame_set_bounds (Frame *frame, int x, int y, int width, int height)
{
    frame->x = x;
    frame->y = y;
    frame->width = width;
    frame->height = height;
}

gboolean
tbo_frame_get_border (Frame *frame)
{
    return frame->border;
}

void
tbo_frame_set_border (Frame *frame, gboolean border)
{
    frame->border = border;
}

void
tbo_frame_get_color (Frame *frame, GdkRGBA *color)
{
    if (color == NULL)
        return;

    color->red = frame->color.r;
    color->green = frame->color.g;
    color->blue = frame->color.b;
    color->alpha = 1.0;
}

void
tbo_frame_set_color_rgb (Frame *frame, gdouble red, gdouble green, gdouble blue)
{
    frame->color.r = red;
    frame->color.g = green;
    frame->color.b = blue;
}

GList *
tbo_frame_get_objects (Frame *frame)
{
    return frame->objects;
}

gint
tbo_frame_object_count (Frame *frame)
{
    return g_list_length (frame->objects);
}

gint
tbo_frame_object_nth (Frame *frame, TboObjectBase *obj)
{
    return tbo_current_list_index (frame->objects, obj);
}

gboolean
tbo_frame_has_obj (Frame *frame, TboObjectBase *obj)
{
    return tbo_list_utils_contains (frame->objects, obj);
}

void
tbo_frame_draw_complete (Frame *frame, cairo_t *cr,
                         float fill_r, float fill_g, float fill_b,
                         float border_r, float border_g, float border_b,
                         int line_width)
{
    DrawObjectsData draw_data = {
        .cr = cr,
        .frame = frame,
    };

    cairo_set_source_rgb (cr, fill_r, fill_g, fill_b);
    cairo_rectangle (cr, frame->x, frame->y, frame->width, frame->height);
    cairo_fill (cr);

    cairo_set_line_width (cr, line_width);
    if (frame->border)
    {
        cairo_set_source_rgb (cr, border_r, border_g, border_b);
        cairo_rectangle (cr, frame->x, frame->y, frame->width, frame->height);
        cairo_stroke (cr);
    }

    g_list_foreach (frame->objects, draw_objects, &draw_data);
}

void
tbo_frame_draw (Frame *frame, cairo_t *cr)
{
    Color border = {0, 0, 0};

    tbo_frame_draw_complete (frame, cr,
                             frame->color.r, frame->color.g, frame->color.b,
                             border.r, border.g, border.b,
                             4);
}

int
tbo_frame_point_inside (Frame *frame, int x, int y)
{
    if ((x >= frame->x) &&
        (x <= (frame->x + frame->width)) &&
        (y >= frame->y) &&
        (y <= (frame->y + frame->height)))
        return 1;
    else
        return 0;
}

int
tbo_frame_point_inside_obj (TboObjectBase *obj, int x, int y)
{
    int ox, oy, ow, oh;
    int xnew1, ynew1, xnew2, ynew2, xnew3, ynew3;
    int xmax, ymax, xmin, ymin;

    tbo_frame_get_obj_relative (obj, &ox, &oy, &ow, &oh);
    xnew1 = ox + (ow * cos (obj->angle));
    ynew1 = oy + (ow * sin (obj->angle));
    xnew2 = ox + (-oh * sin (obj->angle));
    ynew2 = oy + (oh * cos (obj->angle));
    xnew3 = ox + (ow * cos (obj->angle) - oh * sin (obj->angle));
    ynew3 = oy + (oh * cos (obj->angle) + ow * sin (obj->angle));

    xmax = ox;
    if (xnew1 > xmax)
        xmax = xnew1;
    if (xnew2 > xmax)
        xmax = xnew2;
    if (xnew3 > xmax)
        xmax = xnew3;
    ymax = oy;
    if (ynew1 > ymax)
        ymax = ynew1;
    if (ynew2 > ymax)
        ymax = ynew2;
    if (ynew3 > ymax)
        ymax = ynew3;

    xmin = ox;
    if (xnew1 < xmin)
        xmin = xnew1;
    if (xnew2 < xmin)
        xmin = xnew2;
    if (xnew3 < xmin)
        xmin = xnew3;
    ymin = oy;
    if (ynew1 < ymin)
        ymin = ynew1;
    if (ynew2 < ymin)
        ymin = ynew2;
    if (ynew3 < ymin)
        ymin = ynew3;

    if ((x >= xmin) &&
        (x <= xmax) &&
        (y >= ymin) &&
        (y <= ymax))
        return 1;
    else
        return 0;
}

void
tbo_frame_get_obj_relative (TboObjectBase *obj, int *x, int *y, int *w, int *h)
{
    *x = (BASE_X + obj->x) * SCALE_FACTOR;
    *w = obj->width * SCALE_FACTOR;
    *y = (BASE_Y + obj->y) * SCALE_FACTOR;
    *h = obj->height * SCALE_FACTOR;
}

int
tbo_frame_get_base_x (int x)
{
    return (x / SCALE_FACTOR) - BASE_X;
}

int
tbo_frame_get_base_y (int y)
{
    return (y / SCALE_FACTOR) - BASE_Y;
}

void
tbo_frame_draw_scaled (Frame *frame, cairo_t *cr, int width, int height)
{
    int previous_x;
    int previous_y;

    tbo_frame_set_scale_factor (frame, width, height);

    cairo_scale (cr, SCALE_FACTOR, SCALE_FACTOR);
    previous_x = frame->x;
    previous_y = frame->y;
    frame->x = tbo_frame_get_x_centered (frame, width);
    frame->y = tbo_frame_get_y_centered (frame, height);
    BASE_X = frame->x;
    BASE_Y = frame->y;
    tbo_frame_draw (frame, cr);
    cairo_scale (cr, 1 / SCALE_FACTOR, 1 / SCALE_FACTOR);
    frame->x = previous_x;
    frame->y = previous_y;
}

void
tbo_frame_add_obj (Frame *frame, TboObjectBase *obj)
{
    tbo_frame_insert_obj (frame, obj, -1);
}

void
tbo_frame_insert_obj (Frame *frame, TboObjectBase *obj, int nth)
{
    tbo_list_utils_insert (&frame->objects, obj, nth);
}

float
tbo_frame_get_scale_factor (void)
{
    return SCALE_FACTOR;
}

void
tbo_frame_del_obj (Frame *frame, TboObjectBase *obj)
{
    if (tbo_list_utils_remove (&frame->objects, obj))
        g_object_unref (obj);
}

void
tbo_frame_reorder_obj (Frame *frame, TboObjectBase *obj, int nth)
{
    if (!tbo_frame_has_obj (frame, obj))
        return;

    tbo_list_utils_remove (&frame->objects, obj);
    tbo_frame_insert_obj (frame, obj, nth);
}

void
tbo_frame_set_color (Frame *frame, GdkRGBA *color)
{
    tbo_frame_set_color_rgb (frame, color->red, color->green, color->blue);
}

void
tbo_frame_save (Frame *frame, FILE *file)
{
    GList *o;
    TboObjectBase *obj;
    GString *xml;

    xml = g_string_new ("  <frame");
    tbo_xml_append_attr_int (xml, "x", frame->x);
    tbo_xml_append_attr_int (xml, "y", frame->y);
    tbo_xml_append_attr_int (xml, "width", frame->width);
    tbo_xml_append_attr_int (xml, "height", frame->height);
    tbo_xml_append_attr_int (xml, "border", frame->border);
    tbo_xml_append_attr_double (xml, "r", frame->color.r);
    tbo_xml_append_attr_double (xml, "g", frame->color.g);
    tbo_xml_append_attr_double (xml, "b", frame->color.b);
    g_string_append (xml, ">\n");
    tbo_xml_write (file, xml);

    for (o = frame->objects; o; o = g_list_next (o))
    {
        obj = TBO_OBJECT_BASE (o->data);
        obj->save (obj, file);
    }

    fputs ("  </frame>\n", file);
}

Frame *
tbo_frame_clone (Frame *frame)
{
    GList *o;
    TboObjectBase *cur_object;
    Frame *newframe = tbo_frame_new (frame->x, frame->y, frame->width, frame->height);

    for (o = frame->objects; o; o = g_list_next (o))
    {
        cur_object = TBO_OBJECT_BASE (o->data);
        tbo_frame_add_obj (newframe, cur_object->clone (cur_object));
    }
    newframe->border = frame->border;
    newframe->color = frame->color;

    return newframe;
}
