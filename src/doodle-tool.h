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


#ifndef __TBO_DOODLE_TOOL__
#define __TBO_DOODLE_TOOL__

#include <gtk/gtk.h>
#include <cairo.h>
#include "tbo-window.h"

void doodle_tool_on_select (TboWindow *tbo);
void doodle_tool_on_unselect (TboWindow *tbo);
void doodle_tool_on_move (GtkWidget *widget, GdkEventMotion *event, TboWindow *tbo);
void doodle_tool_on_click (GtkWidget *widget, GdkEventButton *event, TboWindow *tbo);
void doodle_tool_on_release (GtkWidget *widget, GdkEventButton *event, TboWindow *tbo);
void doodle_tool_on_key (GtkWidget *widget, GdkEventKey *event, TboWindow *tbo);
void doodle_tool_drawing (cairo_t *cr);

void doodle_tool_bubble_on_select (TboWindow *tbo);
void doodle_tool_bubble_on_unselect (TboWindow *tbo);

#endif
