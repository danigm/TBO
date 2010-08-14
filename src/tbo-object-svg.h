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


#ifndef __TBO_OBJECT_SVG_H__
#define __TBO_OBJECT_SVG_H__

#include <glib.h>
#include "tbo-object-base.h"

#define TBO_TYPE_OBJECT_SVG            (tbo_object_svg_get_type ())
#define TBO_OBJECT_SVG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TBO_TYPE_OBJECT_SVG, TboObjectSvg))
#define TBO_IS_OBJECT_SVG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TBO_TYPE_OBJECT_SVG))
#define TBO_OBJECT_SVG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TBO_TYPE_OBJECT_SVG, TboObjectSvgClass))
#define TBO_IS_OBJECT_SVG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TBO_TYPE_OBJECT_SVG))
#define TBO_OBJECT_SVG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TBO_TYPE_OBJECT_SVG, TboObjectSvgClass))

typedef struct _TboObjectSvg      TboObjectSvg;
typedef struct _TboObjectSvgClass TboObjectSvgClass;

struct _TboObjectSvg
{
    TboObjectBase parent_instance;

    /* instance members */
    GString *path;
};

struct _TboObjectSvgClass
{
    TboObjectBaseClass parent_class;

    /* class members */
};

/* used by TBO_TYPE_OBJECT_SVG */
GType tbo_object_svg_get_type (void);

/*
 * Method definitions.
 */

GObject * tbo_object_svg_new ();
GObject * tbo_object_svg_new_with_params (gint   x,
                                          gint   y,
                                          gint   width,
                                          gint   height,
                                          gchar *path);

#endif /* __TBO_OBJECT_SVG_H__ */

