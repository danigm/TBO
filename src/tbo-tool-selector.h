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


#ifndef __TBO_TOOL_SELECTOR_H__
#define __TBO_TOOL_SELECTOR_H__

#include <glib-object.h>
#include <cairo.h>
#include <gtk/gtk.h>
#include "tbo-object-base.h"
#include "tbo-tool-base.h"
#include "tbo-types.h"
#include "tbo-undo.h"

#define TBO_TYPE_TOOL_SELECTOR            (tbo_tool_selector_get_type ())
#define TBO_TOOL_SELECTOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TBO_TYPE_TOOL_SELECTOR, TboToolSelector))
#define TBO_IS_TOOL_SELECTOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TBO_TYPE_TOOL_SELECTOR))
#define TBO_TOOL_SELECTOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TBO_TYPE_TOOL_SELECTOR, TboToolSelectorClass))
#define TBO_IS_TOOL_SELECTOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TBO_TYPE_TOOL_SELECTOR))
#define TBO_TOOL_SELECTOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TBO_TYPE_TOOL_SELECTOR, TboToolSelectorClass))

#define R_SIZE 10

typedef struct _TboToolSelector      TboToolSelector;
typedef struct _TboToolSelectorClass TboToolSelectorClass;

struct _TboToolSelector
{
    TboToolBase parent_instance;

    /* instance members */
    Frame *selected_frame;
    TboObjectBase *selected_object;
    gint x;
    gint y;
    gint start_x;
    gint start_y;
    gint start_m_x;
    gint start_m_y;
    gint start_m_w;
    gint start_m_h;
    gboolean clicked;
    gboolean over_resizer;
    gboolean over_rotater;
    gboolean resizing;
    gboolean rotating;
    GtkWidget *spin_w;
    GtkWidget *spin_h;
    GtkWidget *spin_x;
    GtkWidget *spin_y;
};

struct _TboToolSelectorClass
{
    TboToolBaseClass parent_class;

    /* class members */
};

/* used by TBO_TYPE_TOOL_SELECTOR */
GType tbo_tool_selector_get_type (void);

/*
 * Method definitions.
 */
Frame * tbo_tool_selector_get_selected_frame (TboToolSelector *self);
TboObjectBase * tbo_tool_selector_get_selected_obj (TboToolSelector *self);
void tbo_tool_selector_set_selected (TboToolSelector *self, Frame *frame);
void tbo_tool_selector_set_selected_obj (TboToolSelector *self, TboObjectBase *obj);
GObject * tbo_tool_selector_new ();
GObject * tbo_tool_selector_new_with_params (TboWindow *tbo);

/*
 * TboActionFrameMove for undo and redo frame movements
 */
typedef struct _TboActionFrameMove TboActionFrameMove;
typedef struct _TboActionObjMove TboActionObjMove;

struct _TboActionFrameMove {
    void (*action_do) (TboAction *action);
    void (*action_undo) (TboAction *action);
    Frame *frame;
    int x1;
    int y1;
    int x2;
    int y2;
};
TboAction * tbo_action_frame_move_new (Frame *frame, int x1, int y1, int x2, int y2);

struct _TboActionObjMove {
    void (*action_do) (TboAction *action);
    void (*action_undo) (TboAction *action);
    TboObjectBase *obj;
    int x1;
    int y1;
    int x2;
    int y2;
};
TboAction * tbo_action_object_move_new (TboObjectBase *object, int x1, int y1, int x2, int y2);


#endif /* __TBO_TOOL_SELECTOR_H__ */

