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


#ifndef __TBO_TOOL_FRAME_H__
#define __TBO_TOOL_FRAME_H__

#include <glib-object.h>
#include <cairo.h>
#include <gtk/gtk.h>
#include "tbo-object-base.h"
#include "tbo-tool-base.h"
#include "tbo-window.h"

#define TBO_TYPE_TOOL_FRAME            (tbo_tool_frame_get_type ())
#define TBO_TOOL_FRAME(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TBO_TYPE_TOOL_FRAME, TboToolFrame))
#define TBO_IS_TOOL_FRAME(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TBO_TYPE_TOOL_FRAME))
#define TBO_TOOL_FRAME_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TBO_TYPE_TOOL_FRAME, TboToolFrameClass))
#define TBO_IS_TOOL_FRAME_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TBO_TYPE_TOOL_FRAME))
#define TBO_TOOL_FRAME_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TBO_TYPE_TOOL_FRAME, TboToolFrameClass))

typedef struct _TboToolFrame      TboToolFrame;
typedef struct _TboToolFrameClass TboToolFrameClass;

struct _TboToolFrame
{
    TboToolBase parent_instance;

    /* instance members */
    gint n_frame_x;
    gint n_frame_y;
    Frame *tmp_frame;
};

struct _TboToolFrameClass
{
    TboToolBaseClass parent_class;

    /* class members */
};

/* used by TBO_TYPE_TOOL_FRAME */
GType tbo_tool_frame_get_type (void);

/*
 * Method definitions.
 */
GObject * tbo_tool_frame_new ();
GObject * tbo_tool_frame_new_with_params (TboWindow *tbo);

#endif /* __TBO_TOOL_FRAME_H__ */

