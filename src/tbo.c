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


#include <glib.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include "config.h"

#include "tbo-startup.h"

int main (int argc, char**argv) {

#ifdef ENABLE_NLS
    /* Initialize the i18n stuff */
    bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);
#endif
    GtkWidget *main_window;
    GtkWidget *icon;

    GtkWidget *tbo_startup;

    gtk_init (&argc, &argv);

    main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    g_signal_connect (main_window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
    gtk_widget_set_size_request (main_window, 800, 500);

    icon = gtk_image_new_from_file (DATA_DIR "/icon.png");
    gtk_window_set_icon (GTK_WINDOW (main_window), gtk_image_get_pixbuf (GTK_IMAGE (icon)));
    gtk_widget_destroy (icon);

    tbo_startup = tbo_startup_new ();

    gtk_container_add (GTK_CONTAINER (main_window), tbo_startup);

    gtk_widget_show_all (main_window);

    gtk_main ();

    return 0;
}

