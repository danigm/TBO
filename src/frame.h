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


#ifndef __TBO_FRAME__
#define __TBO_FRAME__

#include <glib-object.h>
#include <gtk/gtk.h>
#include <cairo.h>
#include <stdio.h>
#include "tbo-types.h"
#include "tbo-object-base.h"

#define TBO_TYPE_FRAME            (tbo_frame_get_type ())
#define TBO_FRAME(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TBO_TYPE_FRAME, Frame))
#define TBO_IS_FRAME(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TBO_TYPE_FRAME))
#define TBO_FRAME_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TBO_TYPE_FRAME, FrameClass))
#define TBO_IS_FRAME_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TBO_TYPE_FRAME))
#define TBO_FRAME_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TBO_TYPE_FRAME, FrameClass))

typedef struct _FrameClass FrameClass;

GType tbo_frame_get_type (void);

Frame *tbo_frame_new (int x, int y, int witdh, int heigth);
void tbo_frame_free (Frame *frame);
int tbo_frame_get_x (Frame *frame);
int tbo_frame_get_y (Frame *frame);
int tbo_frame_get_width (Frame *frame);
int tbo_frame_get_height (Frame *frame);
void tbo_frame_get_bounds (Frame *frame, int *x, int *y, int *width, int *height);
void tbo_frame_set_position (Frame *frame, int x, int y);
void tbo_frame_set_size (Frame *frame, int width, int height);
void tbo_frame_set_bounds (Frame *frame, int x, int y, int width, int height);
gboolean tbo_frame_get_border (Frame *frame);
void tbo_frame_set_border (Frame *frame, gboolean border);
void tbo_frame_get_color (Frame *frame, GdkRGBA *color);
void tbo_frame_set_color_rgb (Frame *frame, gdouble red, gdouble green, gdouble blue);
GList *tbo_frame_get_objects (Frame *frame);
gint tbo_frame_object_count (Frame *frame);
gint tbo_frame_object_nth (Frame *frame, TboObjectBase *obj);
gboolean tbo_frame_has_obj (Frame *frame, TboObjectBase *obj);

void tbo_frame_draw_complete (Frame *frame, cairo_t *cr,
        float fill_r, float fill_g, float fill_b,
        float border_r, float border_g, float border_b,
        int line_width);

void tbo_frame_draw (Frame *frame, cairo_t *cr);
void tbo_frame_draw_scaled (Frame *frame, cairo_t *cr, int width, int height);
int tbo_frame_point_inside (Frame *frame, int x, int y);
int tbo_frame_point_inside_obj (TboObjectBase *obj, int x, int y);
void tbo_frame_add_obj (Frame *frame, TboObjectBase *obj);
void tbo_frame_insert_obj (Frame *frame, TboObjectBase *obj, int nth);
void tbo_frame_del_obj (Frame *frame, TboObjectBase *obj);
void tbo_frame_reorder_obj (Frame *frame, TboObjectBase *obj, int nth);
void tbo_frame_get_obj_relative (TboObjectBase *obj, int *x, int *y, int *w, int *h);
float tbo_frame_get_scale_factor (void);
int tbo_frame_get_base_y (int y);
int tbo_frame_get_base_x (int x);
void tbo_frame_set_color (Frame *frame, GdkRGBA *color);
void tbo_frame_save (Frame *frame, FILE *file);
Frame *tbo_frame_clone (Frame *frame);

#endif
