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


#ifndef __TBO_TEXT_TOOL__
#define __TBO_TEXT_TOOL__

#include <gtk/gtk.h>
#include <cairo.h>
#include "tbo-object-text.h"
#include "tbo-window.h"

void text_tool_on_select (TboWindow *tbo);
void text_tool_on_unselect (TboWindow *tbo);
void text_tool_on_move (GtkWidget *widget, GdkEventMotion *event, TboWindow *tbo);
void text_tool_on_click (GtkWidget *widget, GdkEventButton *event, TboWindow *tbo);
void text_tool_on_release (GtkWidget *widget, GdkEventButton *event, TboWindow *tbo);
void text_tool_on_key (GtkWidget *widget, GdkEventKey *event, TboWindow *tbo);
void text_tool_drawing (cairo_t *cr);
char * text_tool_get_pango_font ();
const gchar * text_tool_get_font_name ();
void text_tool_set_selected (TboObjectText *text);

#endif
