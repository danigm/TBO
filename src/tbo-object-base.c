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


#include <glib.h>
#include <cairo.h>
#include <stdio.h>
#include "tbo-types.h"
#include "tbo-object-base.h"

G_DEFINE_TYPE (TboObjectBase, tbo_object_base, G_TYPE_OBJECT);

static void draw (TboObjectBase *, Frame *, cairo_t *);
static void save (TboObjectBase *, FILE *);
static TboObjectBase * tclone (TboObjectBase *);

static void
draw (TboObjectBase *self, Frame *frame, cairo_t *cr)
{
}

static void
save (TboObjectBase *self, FILE *file)
{
}

static void
move (TboObjectBase *self, enum MOVE_OPT type)
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

static void
resize (TboObjectBase *self, enum RESIZE_OPT type)
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

static TboObjectBase *
tclone (TboObjectBase *self)
{
    return NULL;
}

/* init methods */

static void
tbo_object_base_init (TboObjectBase *self)
{
    self->x = 0;
    self->y = 0;
    self->width = 0;
    self->height = 0;
    self->angle = 0.0;
    self->flipv = FALSE;
    self->fliph = FALSE;

    self->draw = draw;
    self->save = save;
    self->clone = tclone;

    self->move = move;
    self->resize = resize;
}

static void
tbo_object_base_class_init (TboObjectBaseClass *klass)
{
}

/* object functions */

GObject *
tbo_object_base_new ()
{
    GObject *tbo_object;
    tbo_object = g_object_new (TBO_TYPE_OBJECT_BASE, NULL);
    return tbo_object;
}

void
tbo_object_base_flipv (TboObjectBase *self)
{
    if (self->flipv)
        self->flipv = FALSE;
    else
        self->flipv = TRUE;
}

void
tbo_object_base_fliph (TboObjectBase *self)
{
    if (self->fliph)
        self->fliph = FALSE;
    else
        self->fliph = TRUE;
}

void
tbo_object_base_get_flip_matrix (TboObjectBase *self, cairo_matrix_t *mx)
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
tbo_object_base_order_down (TboObjectBase *self, Frame *frame)
{
    GList *list = g_list_find (frame->objects, self);
    GList *prev = g_list_previous (list);
    TboObjectBase *tmp;
    if (prev)
    {
        tmp = TBO_OBJECT_BASE (list->data);
        list->data = prev->data;
        prev->data = tmp;
    }
}

void
tbo_object_base_order_up (TboObjectBase *self, Frame *frame)
{
    GList *list = g_list_find (frame->objects, self);
    GList *next = g_list_next (list);
    TboObjectBase *tmp;
    if (next)
    {
        tmp = TBO_OBJECT_BASE (list->data);
        list->data = next->data;
        next->data = tmp;
    }
}

void
tbo_object_base_move (TboObjectBase *self, enum MOVE_OPT type)
{
    self->move (self, type);
}

void
tbo_object_base_resize (TboObjectBase *self, enum RESIZE_OPT type)
{
    self->resize (self, type);
}
