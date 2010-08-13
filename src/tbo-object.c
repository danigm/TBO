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
#include "ui-drawing.h"
#include "tbo-types.h"
#include "tbo-object.h"
#include "svgimage.h"
#include "textobj.h"

void
tbo_object_flipv (tbo_object *self)
{
    if (self->flipv)
        self->flipv = FALSE;
    else
        self->flipv = TRUE;
}

void
tbo_object_fliph (tbo_object *self)
{
    if (self->fliph)
        self->fliph = FALSE;
    else
        self->fliph = TRUE;
}

void
tbo_object_get_flip_matrix (tbo_object *self, cairo_matrix_t *mx)
{
    cairo_matrix_t flipv = {1, 0, 0, 1, 0, 0};
    cairo_matrix_t fliph = {1, 0, 0, 1, 0, 0};

    if (self->flipv)
    {
        cairo_matrix_init (&flipv, 1, 0, 0, -1, 0, self->height);
    }
    if (self->fliph)
    {
        cairo_matrix_init (&fliph, -1, 0, 0, 1, self->width, 0);
    }

    cairo_matrix_multiply (mx, &flipv, &fliph);
}

void
tbo_object_order_down (tbo_object *self)
{
    Frame *frame = get_frame_view ();
    GList *list = g_list_find (frame->objects, self);
    GList *prev = g_list_previous (list);
    tbo_object *tmp;
    if (prev)
    {
        tmp = (tbo_object*)list->data;
        list->data = prev->data;
        prev->data = tmp;
    }
}

void
tbo_object_order_up (tbo_object *self)
{
    Frame *frame = get_frame_view ();
    GList *list = g_list_find (frame->objects, self);
    GList *next = g_list_next (list);
    tbo_object *tmp;
    if (next)
    {
        tmp = (tbo_object*)list->data;
        list->data = next->data;
        next->data = tmp;
    }
}

void
tbo_object_move (tbo_object *self, enum MOVE_OPT type)
{
    switch (type)
    {
        case MOVE_UP:
            self->y -= MOVING_OFFSET;
            break;
        case MOVE_DOWN:
            self->y += MOVING_OFFSET;
            break;
        case MOVE_LEFT:
            self->x -= MOVING_OFFSET;
            break;
        case MOVE_RIGHT:
            self->x += MOVING_OFFSET;
            break;
        default:
            break;
    }
}

void
tbo_object_resize (tbo_object *self, enum RESIZE_OPT type)
{
    switch (type)
    {
        case RESIZE_LESS:
            if (self->width > 10 && self->height > 10)
            {
                self->width *= 0.95;
                self->height *= 0.95;
            }
            break;
        case RESIZE_GREATER:
            self->width *= 1.05;
            self->height *= 1.05;
            break;
        default:
            break;
    }
}

void
tbo_object_save (tbo_object *self, FILE *file)
{
    self->save (self, file);
}
