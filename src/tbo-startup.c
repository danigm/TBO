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

#include <glib/gi18n.h>

G_DEFINE_TYPE (TboStartup, tbo_startup, GTK_TYPE_GRID);

static GtkWidget *init_project_icon_view ();
static GtkWidget *init_recent_project_view ();

static GtkWidget *
init_recent_project_view ()
{
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *label2;
    GtkWidget *button;

    GtkRecentManager *recent_manager;
    GList *recent_list, *list;
    GtkWidget *recent_view;
    GtkListStore *recent_view_store;
    GtkTreeViewColumn *column;
    GtkTreeIter iter, *iter2;
    GtkRecentInfo *recent_info;
    GtkImage *image;
    GtkWidget *scroll;

    recent_manager = gtk_recent_manager_get_default ();

    recent_list = gtk_recent_manager_get_items (recent_manager);
    recent_view_store = gtk_list_store_new (2, GDK_TYPE_PIXBUF, G_TYPE_STRING);

    image = GTK_IMAGE (gtk_image_new_from_file (DATA_DIR "/icon.png"));
    for (list = recent_list; list; list = list->next) {
        recent_info = (GtkRecentInfo*)list->data;

        if (gtk_recent_info_has_application (recent_info, "tbo"))
        {
            iter2 = gtk_tree_iter_copy (&iter);
            gtk_list_store_append (recent_view_store, iter2);
            gtk_list_store_set (recent_view_store, iter2,
                    0, gtk_image_get_pixbuf (image),
                    1, gtk_recent_info_get_display_name (recent_info),
                    -1);
            gtk_tree_iter_free (iter2);
        }

        gtk_recent_info_unref (recent_info);
    }
    gtk_widget_destroy (GTK_WIDGET (image));
    g_list_free (recent_list);

    recent_view = gtk_tree_view_new ();
    gtk_tree_view_set_model (GTK_TREE_VIEW (recent_view), GTK_TREE_MODEL (recent_view_store));
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (recent_view), FALSE);

    column = gtk_tree_view_column_new_with_attributes ("", gtk_cell_renderer_pixbuf_new (), "pixbuf", 0, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (recent_view), column);

    column = gtk_tree_view_column_new_with_attributes ("", gtk_cell_renderer_text_new (), "text", 1, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (recent_view), column);

    scroll = gtk_scrolled_window_new (NULL, NULL);
    gtk_container_add (GTK_CONTAINER (scroll), recent_view);

    gtk_widget_set_hexpand (recent_view, TRUE);
    gtk_widget_set_vexpand (recent_view, TRUE);

    grid = gtk_grid_new ();

    label = gtk_label_new (_("Recent projects"));
    label2 = gtk_label_new (_("Open other path:"));
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_widget_set_margin_left (label, 5);
    gtk_widget_set_margin_bottom (label, 5);
    gtk_misc_set_alignment (GTK_MISC (label2), 1, 0.5);
    button = gtk_file_chooser_button_new (_("Open TBO file"), GTK_FILE_CHOOSER_ACTION_OPEN);

    gtk_grid_set_row_spacing (GTK_GRID (grid), 5);
    gtk_grid_attach (GTK_GRID (grid), label, 0, 0, 3, 1);
    gtk_grid_attach (GTK_GRID (grid), scroll, 0, 1, 3, 1);
    gtk_grid_attach (GTK_GRID (grid), label2, 0, 2, 2, 1);
    gtk_grid_attach (GTK_GRID (grid), button, 2, 2, 1, 1);

    return grid;
}

static GtkWidget *
init_project_icon_view ()
{
    GtkWidget *icon_view = NULL;

    GtkTreeIter iter, *iter2;
    GtkListStore *list_store;
    GtkWidget *image = NULL;
    GdkPixbuf *icon = NULL;
    int i;


    icon_view = gtk_icon_view_new ();

    image = gtk_image_new_from_file (DATA_DIR "/icon.png");
    icon = gtk_image_get_pixbuf (GTK_IMAGE (image));

    list_store = gtk_list_store_new (2, G_TYPE_STRING, GDK_TYPE_PIXBUF);

    for (i=0; i < 20; i++) {
        iter2 = gtk_tree_iter_copy (&iter);
        gtk_list_store_append (list_store, iter2);
        gtk_list_store_set (list_store, iter2,
                            0, "Value",
                            1, icon,
                            -1);
        gtk_tree_iter_free (iter2);
    }

    gtk_widget_destroy (image);

    gtk_icon_view_set_model (GTK_ICON_VIEW (icon_view), GTK_TREE_MODEL (list_store));
    gtk_icon_view_set_markup_column (GTK_ICON_VIEW (icon_view), 0);
    gtk_icon_view_set_pixbuf_column (GTK_ICON_VIEW (icon_view), 1);

    gtk_widget_set_hexpand (icon_view, TRUE);
    gtk_widget_set_vexpand (icon_view, TRUE);

    return icon_view;
}


GtkWidget *
tbo_startup_new ()
{
    TboStartup *obj;

    GtkWidget *recent_view;
    GtkWidget *child3;
    GtkWidget *scroll1, *scroll3;


    obj = TBO_STARTUP (g_object_new (TBO_TYPE_STARTUP, NULL));

    obj->project_type_icon_view = init_project_icon_view ();

    recent_view = init_recent_project_view ();

    child3 = gtk_text_view_new ();

    scroll1 = gtk_scrolled_window_new (NULL, NULL);
    scroll3 = gtk_scrolled_window_new (NULL, NULL);

    gtk_container_add (GTK_CONTAINER (scroll1), obj->project_type_icon_view);

    gtk_container_add (GTK_CONTAINER (scroll3), child3);

    gtk_widget_set_hexpand (child3, TRUE);
    gtk_widget_set_vexpand (child3, TRUE);

    gtk_grid_set_row_spacing (GTK_GRID (obj), 5);
    gtk_grid_set_column_spacing (GTK_GRID (obj), 5);

    gtk_grid_attach (GTK_GRID (obj), scroll1, 0, 0, 2, 1);
    gtk_grid_attach_next_to (GTK_GRID (obj), recent_view, scroll1, GTK_POS_BOTTOM, 1, 1);
    gtk_grid_attach_next_to (GTK_GRID (obj), scroll3, recent_view, GTK_POS_RIGHT, 1, 1);

    return GTK_WIDGET (obj);
}

static void
tbo_startup_init (TboStartup *self)
{
}

static void
tbo_startup_class_init (TboStartupClass *klass)
{
}
