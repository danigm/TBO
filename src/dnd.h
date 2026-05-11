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


#ifndef __TBO_DND__
#define __TBO_DND__

#include <gtk/gtk.h>
#include "tbo-object-base.h"
#include "tbo-drawing.h"
#include "tbo-window.h"

void tbo_dnd_setup_asset_source (GtkWidget *widget, const gchar *full_path, const gchar *relative_path);
void tbo_dnd_setup_drawing_dest (TboDrawing *drawing, TboWindow *tbo);
TboObjectBase *tbo_dnd_insert_asset_at_view_coords (TboWindow *tbo, const gchar *asset_path, gdouble x, gdouble y);
TboObjectBase *tbo_dnd_insert_asset (TboWindow *tbo, const gchar *asset_path, gint x, gint y);
TboObjectBase *tbo_dnd_insert_asset_centered (TboWindow *tbo, const gchar *asset_path);

#endif
