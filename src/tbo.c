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


#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include "config.h"

#include "tbo-window.h"
#include "comic.h"
#include "ui-menu.h"
#include "tbo-utils.h"
#include "tbo-widget.h"

static void
present_window (TboWindow *tbo)
{
    GdkSurface *surface;
    GdkToplevelLayout *layout;

    gtk_window_present (GTK_WINDOW (tbo->window));
    surface = gtk_native_get_surface (GTK_NATIVE (tbo->window));
    if (surface != NULL && GDK_IS_TOPLEVEL (surface))
    {
        layout = gdk_toplevel_layout_new ();
        gdk_toplevel_present (GDK_TOPLEVEL (surface), layout);
        gdk_toplevel_focus (GDK_TOPLEVEL (surface), GDK_CURRENT_TIME);
        gdk_toplevel_layout_unref (layout);
    }
}

static void
activate_cb (GtkApplication *app, gpointer user_data)
{
    gsize n_recovery_files = 0;
    gchar **recovery_files = tbo_window_list_recovery_files (&n_recovery_files);

    if (n_recovery_files > 0)
    {
        static const gchar *buttons[] = {
            "_Discard",
            "_Recover",
            NULL,
        };
        gint response = tbo_alert_choose (NULL,
                                          _("Recover autosaved work?"),
                                          _("TBO found autosaved documents from a previous session."),
                                          buttons,
                                          0,
                                          1);
        gsize i;

        if (response == 1)
        {
            for (i = 0; i < n_recovery_files; i++)
            {
                TboWindow *tbo = tbo_new_tbo (app, 800, 450);

                tbo_window_recover_file (tbo, recovery_files[i]);
                present_window (tbo);
            }
            g_strfreev (recovery_files);
            return;
        }

        for (i = 0; i < n_recovery_files; i++)
            tbo_window_delete_recovery_file (recovery_files[i]);
    }

    g_strfreev (recovery_files);
    {
        TboWindow *tbo = tbo_new_tbo (app, 800, 450);
        present_window (tbo);
    }
}

static void
open_cb (GtkApplication *app, GFile **files, gint n_files, const gchar *hint, gpointer user_data)
{
    gint i;

    for (i = 0; i < n_files; i++)
    {
        TboWindow *tbo;
        gchar *path = g_file_get_path (files[i]);

        tbo = tbo_new_tbo (app, 800, 450);
        if (path != NULL)
        {
            if (tbo_comic_open (tbo, path))
            {
                tbo_window_set_path (tbo, path);
                tbo_window_add_recent_project (path);
                tbo_menu_refresh (tbo);
            }
            g_free (path);
        }
        present_window (tbo);
    }
}

int main (int argc, char**argv){
    GtkApplication *app;
    int status;

    g_set_application_name ("TBO");
    tbo_init_i18n ();

    app = gtk_application_new ("net.danigm.tbo", G_APPLICATION_HANDLES_OPEN);
    g_signal_connect (app, "activate", G_CALLBACK (activate_cb), NULL);
    g_signal_connect (app, "open", G_CALLBACK (open_cb), NULL);

    status = g_application_run (G_APPLICATION (app), argc, argv);
    g_object_unref (app);

    return status;
}
