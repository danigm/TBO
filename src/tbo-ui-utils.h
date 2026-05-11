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


#ifndef __TBO_UI_UTILS_H__
#define __TBO_UI_UTILS_H__
#include <gtk/gtk.h>

GtkWidget * add_spin_with_label (GtkWidget *container, const gchar *string, gint value);
GtkWidget * tbo_font_picker_new (void);
PangoFontDescription * tbo_font_picker_dup_font_desc (GtkWidget *picker);
void tbo_font_picker_set_font_desc (GtkWidget *picker, const PangoFontDescription *description);
GtkWidget * tbo_color_picker_new (const GdkRGBA *rgba);
GdkRGBA tbo_color_picker_get_rgba (GtkWidget *picker);
void tbo_color_picker_set_rgba (GtkWidget *picker, const GdkRGBA *rgba);
void tbo_picture_set_contain (GtkPicture *picture);

#endif
