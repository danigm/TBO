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
#include "tbo-drawing.h"
#include "tbo-window.h"
#include "comic.h"

gboolean
tbo_comic_open_dialog (GtkWidget *widget, TboWindow *window)
{
    gint response;
    GtkWidget *filechooser;
    GtkFileFilter *filter;
    char *filename;
    char buffer[255];

    filechooser = gtk_file_chooser_dialog_new (_("Open"),
                                               GTK_WINDOW (window->window),
                                               GTK_FILE_CHOOSER_ACTION_OPEN,
                                               GTK_STOCK_CANCEL,
                                               GTK_RESPONSE_CANCEL,
                                               GTK_STOCK_OPEN,
                                               GTK_RESPONSE_ACCEPT,
                                               NULL);

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (filter, _("TBO files"));
    gtk_file_filter_add_pattern (filter, "*.tbo");
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (filechooser), filter);
    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (filter, _("All files"));
    gtk_file_filter_add_pattern (filter, "*");
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (filechooser), filter);

    response = gtk_dialog_run (GTK_DIALOG (filechooser));

    if (response == GTK_RESPONSE_ACCEPT)
    {
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (filechooser));
        tbo_comic_open (window, filename);
        tbo_window_set_path (window, filename);
        tbo_drawing_update (TBO_DRAWING (window->drawing));
        tbo_window_update_status (window, 0, 0);
    }

    gtk_widget_destroy ((GtkWidget *) filechooser);

    return FALSE;
}
