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


#ifndef __TBO_TOOL_BUBBLE_H__
#define __TBO_TOOL_BUBBLE_H__

#include <glib-object.h>
#include "tbo-tool-doodle.h"
#include "tbo-window.h"

#define TBO_TYPE_TOOL_BUBBLE            (tbo_tool_bubble_get_type ())
#define TBO_TOOL_BUBBLE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TBO_TYPE_TOOL_BUBBLE, TboToolBubble))
#define TBO_IS_TOOL_BUBBLE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TBO_TYPE_TOOL_BUBBLE))
#define TBO_TOOL_BUBBLE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TBO_TYPE_TOOL_BUBBLE, TboToolBubbleClass))
#define TBO_IS_TOOL_BUBBLE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TBO_TYPE_TOOL_BUBBLE))
#define TBO_TOOL_BUBBLE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TBO_TYPE_TOOL_BUBBLE, TboToolBubbleClass))

typedef struct _TboToolBubble      TboToolBubble;
typedef struct _TboToolBubbleClass TboToolBubbleClass;

struct _TboToolBubble
{
    TboToolDoodle parent_instance;

    /* instance members */
};

struct _TboToolBubbleClass
{
    TboToolDoodleClass parent_class;

    /* class members */
};

/* used by TBO_TYPE_TOOL_BUBBLE */
GType tbo_tool_bubble_get_type (void);

/*
 * Method definitions.
 */
GObject * tbo_tool_bubble_new ();
GObject * tbo_tool_bubble_new_with_params (TboWindow *tbo);

#endif /* __TBO_TOOL_BUBBLE_H__ */

