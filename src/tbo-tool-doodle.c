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
#include "tbo-tool-doodle.h"

G_DEFINE_TYPE (TboToolDoodle, tbo_tool_doodle, TBO_TYPE_TOOL_BASE);

/* Headers */

static void on_select (TboToolBase *tool);
static void on_unselect (TboToolBase *tool);

/* Definitions */

/* aux */
static gboolean
update_scroll_cb (gpointer data)
{
    TboToolDoodle *self = TBO_TOOL_DOODLE (data);
    GtkAdjustment *adjust;
    TboWindow *tbo = TBO_TOOL_BASE (self)->tbo;
    adjust = gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW (tbo->scroll2));
    gtk_adjustment_set_value (adjust, self->hadjust);
    adjust = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (tbo->scroll2));
    gtk_adjustment_set_value (adjust, self->vadjust);
    return FALSE;
}

static void
setup_tree (TboToolDoodle *self)
{
    self->tree = doodle_setup_tree (TBO_TOOL_BASE (self)->tbo, FALSE);
    gtk_widget_show_all (self->tree);
    self->tree = g_object_ref (self->tree);
}

/* tool signal */
static void
on_select (TboToolBase *tool)
{
    TboToolDoodle *self = TBO_TOOL_DOODLE (tool);
    TboWindow *tbo = tool->tbo;
    if (!self->tree)
    {
        self->setup_tree (self);
    }

    tbo_empty_tool_area (tbo);
    gtk_container_add (GTK_CONTAINER (tbo->toolarea), self->tree);


    g_timeout_add (5, update_scroll_cb, self);
}

static void
on_unselect (TboToolBase *tool)
{
    GtkAdjustment *adjust;
    TboToolDoodle *self = TBO_TOOL_DOODLE (tool);
    TboWindow *tbo = tool->tbo;

    if (GTK_IS_WIDGET (self->tree) && self->tree->parent == GTK_WIDGET (tbo->toolarea))
    {
        adjust = gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW (tbo->scroll2));
        self->hadjust = gtk_adjustment_get_value (adjust);
        adjust = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (tbo->scroll2));
        self->vadjust = gtk_adjustment_get_value (adjust);

        gtk_container_remove (GTK_CONTAINER (tbo->toolarea), self->tree);
    }

    tbo_empty_tool_area (tbo);
}

/* init methods */

static void
tbo_tool_doodle_init (TboToolDoodle *self)
{
    self->tree = NULL;
    self->hadjust = 0.0;
    self->vadjust = 0.0;
    self->setup_tree = setup_tree;

    self->parent_instance.on_select = on_select;
    self->parent_instance.on_unselect = on_unselect;
}

static void
tbo_tool_doodle_class_init (TboToolDoodleClass *klass)
{
}

/* object functions */

GObject *
tbo_tool_doodle_new ()
{
    GObject *tbo_tool;
    tbo_tool = g_object_new (TBO_TYPE_TOOL_DOODLE, NULL);
    return tbo_tool;
}

GObject *
tbo_tool_doodle_new_with_params (TboWindow *tbo)
{
    GObject *tbo_tool;
    TboToolBase *tbo_tool_base;
    tbo_tool = g_object_new (TBO_TYPE_TOOL_DOODLE, NULL);
    tbo_tool_base = TBO_TOOL_BASE (tbo_tool);
    tbo_tool_base->tbo = tbo;
    return tbo_tool;
}

