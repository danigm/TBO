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


#ifndef __UI_DRAWING__
#define __UI_DRAWING__

#include <gtk/gtk.h>
#include <cairo.h>
#include "tbo-window.h"


gboolean on_expose_cb(GtkWidget *widget, GdkEventExpose *event, TboWindow *tbo);
gboolean on_move_cb(GtkWidget *widget, GdkEventMotion *event, TboWindow *tbo);
gboolean on_click_cb(GtkWidget *widget, GdkEventButton *event, TboWindow *tbo);
gboolean on_release_cb(GtkWidget *widget, GdkEventButton *event, TboWindow *tbo);
gboolean on_key_cb(GtkWidget *widget, GdkEventKey *event, TboWindow *tbo);
void darea_connect_signals (TboWindow *tbo, GtkWidget *drawing);
void darea_disconnect_signals (TboWindow *tbo, GtkWidget *drawing);
GtkWidget * get_drawing_area (int width, int height);
void update_drawing (TboWindow *tbo);
void set_frame_view (Frame *frame);
Frame *get_frame_view ();
void tbo_drawing_draw (cairo_t *cr, TboWindow *tbo);
void tbo_drawing_draw_page (cairo_t *cr, Page *page, int w, int h);
void tbo_drawing_zoom_in (TboWindow *tbo);
void tbo_drawing_zoom_out (TboWindow *tbo);
void tbo_drawing_zoom_100 (TboWindow *tbo);
void tbo_drawing_zoom_fit (TboWindow *tbo);
float tbo_drawing_get_zoom ();
void set_key_binder (gboolean binder);
void tbo_drawing_adjust_scroll (TboWindow *tbo);

#endif
