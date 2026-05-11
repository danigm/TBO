#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <unistd.h>

#include "comic.h"
#include "tbo-toolbar.h"
#include "tbo-window.h"

static gchar *
make_tmp_project_path (const gchar *pattern)
{
    gchar *path = g_build_filename (g_get_tmp_dir (), pattern, NULL);
    gint fd = g_mkstemp (path);

    if (fd >= 0)
        close (fd);

    g_remove (path);
    return g_strconcat (path, ".tbo", NULL);
}

int
main (void)
{
    GtkApplication *app;
    TboWindow *tbo;
    gchar *path;
    gchar *basename;
    gchar *dirty_title;

    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.undosavedstate", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo = tbo_new_tbo (app, 800, 450);
    path = make_tmp_project_path ("tbo-undo-saved-state-XXXXXX");
    if (!tbo_comic_save (tbo, path))
        return 3;

    basename = g_path_get_basename (path);
    if (g_strcmp0 (gtk_window_get_title (GTK_WINDOW (tbo->window)), basename) != 0)
        return 4;

    g_signal_emit_by_name (tbo->toolbar->button_new_page, "clicked");
    if (!tbo_window_has_unsaved_changes (tbo))
        return 5;
    dirty_title = g_strdup_printf ("* %s", basename);
    if (g_strcmp0 (gtk_window_get_title (GTK_WINDOW (tbo->window)), dirty_title) != 0)
        return 6;

    tbo_window_undo_cb (NULL, tbo);
    if (tbo_window_has_unsaved_changes (tbo))
        return 7;
    if (g_strcmp0 (gtk_window_get_title (GTK_WINDOW (tbo->window)), basename) != 0)
        return 8;

    tbo_window_redo_cb (NULL, tbo);
    if (!tbo_window_has_unsaved_changes (tbo))
        return 9;
    if (g_strcmp0 (gtk_window_get_title (GTK_WINDOW (tbo->window)), dirty_title) != 0)
        return 10;

    g_free (dirty_title);
    g_free (basename);
    g_remove (path);
    g_free (path);
    tbo_window_mark_clean (tbo);
    gtk_window_close (GTK_WINDOW (tbo->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
