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


#ifndef __TBO_OBJECT_BASE_H__
#define __TBO_OBJECT_BASE_H__

#define MOVING_OFFSET 10

#include <glib-object.h>
#include <tbo-types.h>
#include <cairo.h>
#include <stdio.h>

#define TBO_TYPE_OBJECT_BASE            (tbo_object_base_get_type ())
#define TBO_OBJECT_BASE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TBO_TYPE_OBJECT_BASE, TboObjectBase))
#define TBO_IS_OBJECT_BASE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TBO_TYPE_OBJECT_BASE))
#define TBO_OBJECT_BASE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TBO_TYPE_OBJECT_BASE, TboObjectBaseClass))
#define TBO_IS_OBJECT_BASE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TBO_TYPE_OBJECT_BASE))
#define TBO_OBJECT_BASE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TBO_TYPE_OBJECT_BASE, TboObjectBaseClass))

typedef struct _TboObjectBase      TboObjectBase;
typedef struct _TboObjectBaseClass TboObjectBaseClass;

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

struct _TboObjectBase
{
    GObject parent_instance;

    /* instance members */
    gint x;
    gint y;
    gint width;
    gint height;
    gdouble angle;
    gboolean flipv;
    gboolean fliph;

    void (*draw) (TboObjectBase *, Frame *, cairo_t *);
    void (*save) (TboObjectBase *, FILE *);
    TboObjectBase * (*clone) (TboObjectBase *);
    void (*move) (TboObjectBase *, enum MOVE_OPT type);
    void (*resize) (TboObjectBase *, enum RESIZE_OPT type);
};

struct _TboObjectBaseClass
{
    GObjectClass parent_class;

    /* class members */
};

/* used by TBO_TYPE_OBJECT_BASE */
GType tbo_object_base_get_type (void);

/*
 * Method definitions.
 */

GObject * tbo_object_base_new ();
void tbo_object_base_flipv (TboObjectBase *self);
void tbo_object_base_fliph (TboObjectBase *self);
void tbo_object_base_get_flip_matrix (TboObjectBase *self, cairo_matrix_t *mx);
void tbo_object_base_order_down (TboObjectBase *self, Frame *frame);
void tbo_object_base_order_up (TboObjectBase *self, Frame *frame);
void tbo_object_base_move (TboObjectBase *self, enum MOVE_OPT type);
void tbo_object_base_resize (TboObjectBase *self, enum RESIZE_OPT type);

#endif /* __TBO_OBJECT_BASE_H__ */

