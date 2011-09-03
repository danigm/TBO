/*
 * This file is part of TBO, a gnome comic editor
 * Copyright (C) 2011  Daniel Garcia Moreno <danigm@wadobo.com>
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

#ifndef _TBO_CLIP_LIB_H
#define _TBO_CLIP_LIB_H

#include <gtk/gtk.h>
#include <glib-object.h>

#define TBO_TYPE_CLIP_LIB            (tbo_clip_lib_get_type ())
#define TBO_CLIP_LIB(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TBO_TYPE_CLIP_LIB, TboClipLib))
#define TBO_IS_CLIP_LIB(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TBO_TYPE_CLIP_LIB))
#define TBO_CLIP_LIB_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TBO_TYPE_CLIP_LIB, TboClipLibClass))
#define TBO_IS_CLIP_LIB_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TBO_TYPE_CLIP_LIB))
#define TBO_CLIP_LIB_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TBO_TYPE_CLIP_LIB, TboClipLibClass))

typedef struct _TboClipLib    TboClipLib;
typedef struct _TboClipLibClass TboClipLibClass;

struct _TboClipLib {
    GObject parent;

    /* instance members */
    gchar *path;
    gchar *name;
    GdkPixbuf *icon;

    gchar *desc;
    gchar *author;
    gchar *license;
    gchar *version;
    gchar *link;
};

struct _TboClipLibClass {
    GObjectClass parent_class;

    /* class members */
};

GType tbo_clip_lib_get_type (void);

/*
 * Method definitions.
 */

GObject * tbo_clip_lib_new (const gchar *path);

#endif
