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

#include "tbo-startup.h"

G_DEFINE_TYPE (TboStartup, tbo_startup, GTK_TYPE_GRID);

GtkWidget *
tbo_startup_new ()
{
    GtkWidget *obj;

    GtkWidget *child1, *child2, *child3;
    GtkWidget *scroll1, *scroll2, *scroll3;

    obj = g_object_new (TBO_TYPE_STARTUP, NULL);

    child1 = gtk_icon_view_new ();
    child2 = gtk_tree_view_new ();
    child3 = gtk_text_view_new ();

    scroll1 = gtk_scrolled_window_new (NULL, NULL);
    scroll2 = gtk_scrolled_window_new (NULL, NULL);
    scroll3 = gtk_scrolled_window_new (NULL, NULL);

    gtk_container_add (GTK_CONTAINER (scroll1), child1);
    gtk_container_add (GTK_CONTAINER (scroll2), child2);
    gtk_container_add (GTK_CONTAINER (scroll3), child3);

    gtk_widget_set_hexpand (child1, TRUE);
    gtk_widget_set_vexpand (child1, TRUE);
    gtk_widget_set_hexpand (child2, TRUE);
    gtk_widget_set_vexpand (child2, TRUE);
    gtk_widget_set_hexpand (child3, TRUE);
    gtk_widget_set_vexpand (child3, TRUE);

    gtk_grid_set_row_spacing (GTK_GRID (obj), 5);
    gtk_grid_set_column_spacing (GTK_GRID (obj), 5);

    gtk_grid_attach (GTK_GRID (obj), scroll1, 0, 0, 2, 1);
    gtk_grid_attach_next_to (GTK_GRID (obj), scroll2, scroll1, GTK_POS_BOTTOM, 1, 1);
    gtk_grid_attach_next_to (GTK_GRID (obj), scroll3, scroll2, GTK_POS_RIGHT, 1, 1);

    return obj;
}

static void
tbo_startup_init (TboStartup *self)
{
}

static void
tbo_startup_class_init (TboStartupClass *klass)
{
}
