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


#ifndef __TBO_TOOL_DOODLE_H__
#define __TBO_TOOL_DOODLE_H__

#include <glib-object.h>
#include <gtk/gtk.h>
#include "tbo-tool-base.h"
#include "tbo-window.h"

#define TBO_TYPE_TOOL_DOODLE            (tbo_tool_doodle_get_type ())
#define TBO_TOOL_DOODLE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TBO_TYPE_TOOL_DOODLE, TboToolDoodle))
#define TBO_IS_TOOL_DOODLE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TBO_TYPE_TOOL_DOODLE))
#define TBO_TOOL_DOODLE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TBO_TYPE_TOOL_DOODLE, TboToolDoodleClass))
#define TBO_IS_TOOL_DOODLE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TBO_TYPE_TOOL_DOODLE))
#define TBO_TOOL_DOODLE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TBO_TYPE_TOOL_DOODLE, TboToolDoodleClass))

typedef struct _TboToolDoodle      TboToolDoodle;
typedef struct _TboToolDoodleClass TboToolDoodleClass;

struct _TboToolDoodle
{
    TboToolBase parent_instance;

    /* instance members */
    GtkWidget *tree;
    gdouble hadjust;
    gdouble vadjust;
    void (*setup_tree) (TboToolDoodle *self);
};

struct _TboToolDoodleClass
{
    TboToolBaseClass parent_class;

    /* class members */
};

/* used by TBO_TYPE_TOOL_DOODLE */
GType tbo_tool_doodle_get_type (void);

/*
 * Method definitions.
 */
GObject * tbo_tool_doodle_new ();
GObject * tbo_tool_doodle_new_with_params (TboWindow *tbo);

#endif /* __TBO_TOOL_DOODLE_H__ */

