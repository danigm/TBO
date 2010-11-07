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


#ifndef __TBO_OBJECT_GROUP_H__
#define __TBO_OBJECT_GROUP_H__

#include <glib.h>
#include "tbo-object-base.h"
#include "tbo-object-group.h"

#define TBO_TYPE_OBJECT_GROUP            (tbo_object_group_get_type ())
#define TBO_OBJECT_GROUP(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TBO_TYPE_OBJECT_GROUP, TboObjectGroup))
#define TBO_IS_OBJECT_GROUP(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TBO_TYPE_OBJECT_GROUP))
#define TBO_OBJECT_GROUP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TBO_TYPE_OBJECT_GROUP, TboObjectGroupClass))
#define TBO_IS_OBJECT_GROUP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TBO_TYPE_OBJECT_GROUP))
#define TBO_OBJECT_GROUP_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TBO_TYPE_OBJECT_GROUP, TboObjectGroupClass))

typedef struct _TboObjectGroup      TboObjectGroup;
typedef struct _TboObjectGroupClass TboObjectGroupClass;

struct _TboObjectGroup
{
    TboObjectBase parent_instance;

    /* instance members */
    GList *objs;
    void (*parent_move) (TboObjectBase *, enum MOVE_OPT type);
};

struct _TboObjectGroupClass
{
    TboObjectBaseClass parent_class;

    /* class members */
};

/* used by TBO_TYPE_OBJECT_GROUP */
GType tbo_object_group_get_type (void);

/*
 * Method definitions.
 */

GObject * tbo_object_group_new ();
void tbo_object_group_add (TboObjectGroup *self, TboObjectBase *obj);
void tbo_object_group_del (TboObjectGroup *self, TboObjectBase *obj);
void tbo_object_group_set_vars (TboObjectBase *self);
void tbo_object_group_unset_vars (TboObjectBase *self);
void tbo_object_group_update_status (TboObjectGroup *self);

#endif /* __TBO_OBJECT_GROUP_H__ */

