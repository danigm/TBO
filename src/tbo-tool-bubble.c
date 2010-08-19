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


#include "doodle-treeview.h"
#include "tbo-tool-bubble.h"

G_DEFINE_TYPE (TboToolBubble, tbo_tool_bubble, TBO_TYPE_TOOL_DOODLE);

/* aux */
static void
setup_tree (TboToolDoodle *self)
{
    TboWindow *tbo = TBO_TOOL_BASE (self)->tbo;
    self->tree = doodle_setup_tree (tbo, TRUE);
    gtk_widget_show_all (self->tree);
    self->tree = g_object_ref (self->tree);
}

/* init methods */

static void
tbo_tool_bubble_init (TboToolBubble *self)
{
    self->parent_instance.setup_tree = setup_tree;
}

static void
tbo_tool_bubble_class_init (TboToolBubbleClass *klass)
{
}

/* object functions */

GObject *
tbo_tool_bubble_new ()
{
    GObject *tbo_tool;
    tbo_tool = g_object_new (TBO_TYPE_TOOL_BUBBLE, NULL);
    return tbo_tool;
}

GObject *
tbo_tool_bubble_new_with_params (TboWindow *tbo)
{
    GObject *tbo_tool;
    TboToolBase *tbo_tool_base;
    tbo_tool = g_object_new (TBO_TYPE_TOOL_BUBBLE, NULL);
    tbo_tool_base = TBO_TOOL_BASE (tbo_tool);
    tbo_tool_base->tbo = tbo;
    return tbo_tool;
}

