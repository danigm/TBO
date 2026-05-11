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
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include "comic-open-dialog.h"
#include "tbo-file-dialog.h"
#include "tbo-drawing.h"
#include "tbo-window.h"
#include "comic.h"
#include "ui-menu.h"


gboolean
tbo_comic_open_dialog (GtkWidget *widget, TboWindow *window)
{
    gchar *filename = tbo_file_dialog_open_project (window);

    if (filename != NULL)
    {
        tbo_window_set_browse_path (window, filename);
        if (tbo_window_prepare_for_document_replace (window))
        {
            if (tbo_comic_open (window, filename))
            {
                tbo_window_add_recent_project (filename);
                tbo_menu_refresh (window);
                tbo_drawing_update (TBO_DRAWING (window->drawing));
                tbo_window_refresh_status (window);
            }
        }
        g_free (filename);
    }

    return FALSE;
}
