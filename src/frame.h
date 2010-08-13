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

#include <gtk/gtk.h>
#include <cairo.h>
#include <stdio.h>
#include "tbo-types.h"

Frame *tbo_frame_new (int x, int y, int witdh, int heigth);
void tbo_frame_free (Frame *frame);

void tbo_frame_draw_complete (Frame *frame, cairo_t *cr,
        float fill_r, float fill_g, float fill_b,
        float border_r, float border_g, float border_b,
        int line_width);

void tbo_frame_draw (Frame *frame, cairo_t *cr);
void tbo_frame_draw_scaled (Frame *frame, cairo_t *cr, int width, int height);
int tbo_frame_point_inside (Frame *frame, int x, int y);
int tbo_frame_point_inside_obj (tbo_object *obj, int x, int y);
void tbo_frame_add_obj (Frame *frame, tbo_object *obj);
void tbo_frame_del_obj (Frame *frame, tbo_object *obj);
void tbo_frame_get_obj_relative (tbo_object *obj, int *x, int *y, int *w, int *h);
float tbo_frame_get_scale_factor ();
int tbo_frame_get_base_y (int y);
int tbo_frame_get_base_x (int x);
void tbo_frame_set_color (Frame *frame, GdkColor *color);
void tbo_frame_save (Frame *frame, FILE *file);
Frame *tbo_frame_clone (Frame *frame);

#endif

