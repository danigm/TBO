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


#ifndef __UI_TOOLBAR__
#define __UI_TOOLBAR__

#include <gtk/gtk.h>
#include "tbo-window.h"

enum ToolSignal
{
    TOOL_SELECT,
    TOOL_UNSELECT,
    TOOL_MOVE,
    TOOL_CLICK,
    TOOL_RELEASE,
    TOOL_KEY,
    TOOL_DRAWING,
};

enum Tool
{
    NONE,
    SELECTOR,
    FRAME,
    DOODLE,
    BUBBLE,
    TEXT,
};

typedef struct
{
    enum Tool tool;
    void (*tool_on_select) (TboWindow *);
    void (*tool_on_unselect) (TboWindow *);
    void (*tool_on_move) (GtkWidget *, GdkEventMotion *, TboWindow *);
    void (*tool_on_click) (GtkWidget *, GdkEventButton *, TboWindow *);
    void (*tool_on_release) (GtkWidget *, GdkEventButton *, TboWindow *);
    void (*tool_on_key) (GtkWidget *, GdkEventKey *, TboWindow *);
    void (*tool_drawing) (cairo_t *cr);
} ToolStruct;

void tool_signal (enum Tool tool, enum ToolSignal signal, gpointer data);

enum Tool get_selected_tool ();
void set_selected_tool (enum Tool tool, TboWindow *tbo);
void set_selected_tool_and_action (enum Tool tool, TboWindow *tbo);

GtkWidget *generate_toolbar (TboWindow *window);

gboolean notebook_switch_page_cb (GtkNotebook *notebook, GtkNotebookPage *page, guint page_num, TboWindow *tbo);

#endif

