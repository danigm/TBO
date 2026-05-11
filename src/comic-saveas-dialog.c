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


#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include "comic-saveas-dialog.h"
#include "tbo-file-dialog.h"
#include "tbo-window.h"
#include "comic.h"
#include "ui-menu.h"

gboolean
tbo_comic_save_dialog (GtkWidget *widget, TboWindow *window)
{
    if (window->path)
        return tbo_comic_save (window, window->path);
    else
        return tbo_comic_saveas_dialog (widget, window);
}

gchar *
tbo_comic_build_save_filename (const gchar *title)
{
    if (title == NULL)
        return g_strdup ("untitled.tbo");

    if (g_str_has_suffix (title, ".tbo"))
        return g_strdup (title);

    return g_strconcat (title, ".tbo", NULL);
}

gboolean
tbo_comic_saveas_dialog (GtkWidget *widget, TboWindow *window)
{
    gchar *filename;
    gchar *suggested_name;

    suggested_name = tbo_comic_build_save_filename (tbo_comic_get_title (window->comic));
    filename = tbo_file_dialog_save_project (window, suggested_name);
    g_free (suggested_name);

    if (filename != NULL)
    {
        tbo_window_set_browse_path (window, filename);
        if (tbo_comic_save (window, filename))
        {
            tbo_window_set_path (window, filename);
            tbo_window_add_recent_project (filename);
            tbo_menu_refresh (window);
            g_free (filename);
            return TRUE;
        }
        g_free (filename);
    }

    return FALSE;
}
