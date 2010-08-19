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


#include "tbo-tool-base.h"

G_DEFINE_TYPE (TboToolBase, tbo_tool_base, G_TYPE_OBJECT);

static void on_select (TboToolBase *tool) {}
static void on_unselect (TboToolBase *tool) {}
static void on_move (TboToolBase *tool, GtkWidget *widget, GdkEventMotion *event) {}
static void on_click (TboToolBase *tool, GtkWidget *widget, GdkEventButton *event) {}
static void on_release (TboToolBase *tool, GtkWidget *widget, GdkEventButton *event) {}
static void on_key (TboToolBase *tool, GtkWidget *widget, GdkEventKey *event) {}
static void drawing (TboToolBase *tool, cairo_t *cr) {}

/* init methods */

static void
tbo_tool_base_init (TboToolBase *self)
{
    self->tbo = NULL;

    self->on_select = on_select;
    self->on_unselect = on_unselect;
    self->on_move = on_move;
    self->on_click = on_click;
    self->on_release = on_release;
    self->on_key = on_key;
    self->drawing = drawing;
}

static void
tbo_tool_base_finalize (GObject *self)
{
    if (TBO_TOOL_BASE (self)->action)
        g_free (TBO_TOOL_BASE (self)->action);
    /* Chain up to the parent class */
    G_OBJECT_CLASS (tbo_tool_base_parent_class)->finalize (self);
}

static void
tbo_tool_base_class_init (TboToolBaseClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    gobject_class->finalize = tbo_tool_base_finalize;
}

/* object functions */

GObject *
tbo_tool_base_new ()
{
    GObject *tbo_tool;
    tbo_tool = g_object_new (TBO_TYPE_TOOL_BASE, NULL);
    return tbo_tool;
}

GObject *
tbo_tool_base_new_with_params (TboWindow *tbo)
{
    GObject *tbo_tool;
    tbo_tool = g_object_new (TBO_TYPE_TOOL_BASE, NULL);
    TBO_TOOL_BASE (tbo_tool)->tbo = tbo;
    return tbo_tool;
}

void
tbo_tool_base_set_action (TboToolBase *self, gchar *action)
{
    if (self->action)
        g_free (self->action);
    if (action)
        self->action = g_strdup (action);
}

