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


#ifndef __TBO_TEXT_OBJ__
#define __TBO_TEXT_OBJ__

#include <cairo.h>
#include <stdio.h>
#include "tbo-types.h"

typedef tbo_object TextObj;

TextObj * tbo_text_new ();
TextObj * tbo_text_new_with_params (int x, int y, int width, int height, const char *text, char *font_name, double r, double g, double b);
void tbo_text_free (TextObj *self);
void tbo_text_draw (TextObj *self, Frame *frame, cairo_t *cr);
char *tbo_text_get_text (TextObj *self);
void tbo_text_set_text (TextObj *self, const char *text);
void tbo_text_change_font (TextObj *self, char *font);
void tbo_text_change_color (TextObj *self, double r, double g, double b);
void tbo_text_get_color (TextObj *self, GdkColor *color);
char * tbo_text_get_string (TextObj *self);
void tbo_text_save (TextObj *self, FILE *file);
TextObj * tbo_text_clone (TextObj *self);

#endif
