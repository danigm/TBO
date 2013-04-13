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
#include <math.h>
#include "tbo-types.h"
#include "tbo-object-group.h"

G_DEFINE_TYPE (TboObjectGroup, tbo_object_group, TBO_TYPE_OBJECT_BASE);

static TboObjectBase * tclone (TboObjectBase *);

static TboObjectBase *
tclone (TboObjectBase *self)
{
    return NULL;
}

static void tbo_object_group_get_base (TboObjectGroup *self, gint *minx, gint *miny, gint *maxx, gint *maxy);

static void
resize (TboObjectBase *self, enum RESIZE_OPT type)
{
    GList *o;
    TboObjectBase *obj;
    gdouble scale = 1.0;
    gint minx=-1, miny=-1, maxx=0, maxy=0;
    tbo_object_group_get_base (TBO_OBJECT_GROUP (self), &minx, &miny, &maxx, &maxy);

    switch (type)
    {
        case RESIZE_LESS:
            scale -= 0.05;
            break;
        case RESIZE_GREATER:
            scale += 0.05;
            break;
        default:
            break;
    }

    for (o=g_list_first (TBO_OBJECT_GROUP (self)->objs); o; o=g_list_next(o))
    {
        obj = TBO_OBJECT_BASE (o->data);

        if ((obj->width <= 2 || obj->height <= 2) && scale < 1)
            break;

        obj->width = round (obj->width * scale);
        obj->x = round (minx + (obj->x - minx) * scale);
        obj->height = round (obj->height * scale);
        obj->y = round (miny + (obj->y - miny) * scale);
    }
}

static void
move (TboObjectBase *self, enum MOVE_OPT type)
{
    TBO_OBJECT_GROUP (self)->parent_move (self, type);
    tbo_object_group_update_status (TBO_OBJECT_GROUP (self));
}

/* init methods */

static void
tbo_object_group_init (TboObjectGroup *self)
{
    self->objs = NULL;
    self->parent_move = self->parent_instance.move;

    self->parent_instance.clone = tclone;
    self->parent_instance.resize = resize;
    self->parent_instance.move = move;
}

static void
tbo_object_group_finalize (GObject *self)
{
    if (TBO_IS_OBJECT_GROUP (self))
    {
        if ((TBO_OBJECT_GROUP (self)->objs))
            g_list_free (TBO_OBJECT_GROUP (self)->objs);
    }
    /* Chain up to the parent class */
    G_OBJECT_CLASS (tbo_object_group_parent_class)->finalize (self);
}

static void
tbo_object_group_class_init (TboObjectGroupClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    gobject_class->finalize = tbo_object_group_finalize;
}

/* object functions */

GObject *
tbo_object_group_new ()
{
    GObject *tbo_object;
    tbo_object = g_object_new (TBO_TYPE_OBJECT_GROUP, NULL);

    return tbo_object;
}

void
tbo_object_group_add (TboObjectGroup *self, TboObjectBase *obj)
{
    if (TBO_IS_OBJECT_GROUP (obj))
        return;
    if (!g_list_find (self->objs, obj))
        self->objs = g_list_append (self->objs, obj);
}

void
tbo_object_group_del (TboObjectGroup *self, TboObjectBase *obj)
{
    self->objs = g_list_remove (g_list_first (self->objs), obj);
}

void
tbo_object_group_get_base (TboObjectGroup *self,
                           gint *minx, gint *miny,
                           gint *maxx, gint *maxy)
{
    GList *o;
    TboObjectBase *obj;

    for (o=g_list_first (self->objs); o; o=g_list_next(o))
    {
        obj = TBO_OBJECT_BASE (o->data);
        if (*minx < 0 || obj->x < *minx)
            *minx = obj->x;
        if (*miny < 0 || obj->y < *miny)
            *miny = obj->y;
        if ((obj->x + obj->width) > *maxx)
            *maxx = (obj->x + obj->width);
        if ((obj->y + obj->height) > *maxy)
            *maxy = (obj->y + obj->height);
    }
}

void
tbo_object_group_set_vars (TboObjectBase *obj)
{
    if (!TBO_IS_OBJECT_GROUP (obj))
        return;

    gint minx=-1, miny=-1, maxx=0, maxy=0;
    tbo_object_group_get_base (TBO_OBJECT_GROUP (obj), &minx, &miny, &maxx, &maxy);

    obj->x = minx;
    obj->y = miny;
    obj->width = maxx - minx;
    obj->height = maxy - miny;
}

void
tbo_object_group_unset_vars (TboObjectBase *obj)
{
    if (!TBO_IS_OBJECT_GROUP (obj))
        return;

    obj->x = 0;
    obj->y = 0;
    obj->width = 0;
    obj->height = 0;
}

void
tbo_object_group_update_status (TboObjectGroup *self)
{
    GList *o;
    gdouble scalex = 1.0, scaley = 1.0;
    TboObjectBase *obj, *tbo_object;
    gint minx=-1, miny=-1, maxx=0, maxy=0;

    tbo_object = TBO_OBJECT_BASE (self);
    tbo_object_group_get_base (self, &minx, &miny, &maxx, &maxy);

    for (o=g_list_first (self->objs); o; o=g_list_next(o))
    {
        obj = TBO_OBJECT_BASE (o->data);
        obj->x += tbo_object->x;
        obj->y += tbo_object->y;


        if (tbo_object->width)
        {
            scalex = (obj->width + tbo_object->width) / (double) obj->width;
            obj->width += tbo_object->width;
            obj->x = round (minx + (obj->x - minx) * scalex);
        }

        if (tbo_object->height)
        {
            scaley =  (obj->height + tbo_object->height) / (double) obj->height;
            obj->height += tbo_object->height;
            obj->y = round (miny + (obj->y - miny) * scaley);
        }
    }

    tbo_object_group_unset_vars (tbo_object);
}

