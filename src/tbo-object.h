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
#include <cairo.h>
#include <stdio.h>
#include "tbo-types.h"

enum MOVE_OPT
{
    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT,
};

enum RESIZE_OPT
{
    RESIZE_LESS,
    RESIZE_GREATER,
};

static int MOVING_OFFSET = 10;

void tbo_object_flipv (tbo_object *self);
void tbo_object_fliph (tbo_object *self);
void tbo_object_get_flip_matrix (tbo_object *self, cairo_matrix_t *mx);
void tbo_object_order_down (tbo_object *self);
void tbo_object_order_up (tbo_object *self);
void tbo_object_move (tbo_object *self, enum MOVE_OPT type);
void tbo_object_resize (tbo_object *self, enum RESIZE_OPT type);
void tbo_object_save (tbo_object *self, FILE *file);
