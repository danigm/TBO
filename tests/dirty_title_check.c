#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <unistd.h>

#include "comic.h"
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
    TboWindow *opened;
    gchar *path;
    gchar *expected_title;
    gchar *basename;

    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.dirtytitle", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo = tbo_new_tbo (app, 800, 450);
    if (g_strcmp0 (gtk_window_get_title (GTK_WINDOW (tbo->window)), tbo_comic_get_title (tbo->comic)) != 0)
        return 3;

    tbo_window_mark_dirty (tbo);
    expected_title = g_strdup_printf ("* %s", tbo_comic_get_title (tbo->comic));
    if (g_strcmp0 (gtk_window_get_title (GTK_WINDOW (tbo->window)), expected_title) != 0)
        return 4;
    g_free (expected_title);

    path = make_tmp_project_path ("tbo-dirty-title-XXXXXX");
    if (!tbo_comic_save (tbo, path))
        return 5;
    basename = g_path_get_basename (path);
    if (g_strcmp0 (gtk_window_get_title (GTK_WINDOW (tbo->window)), basename) != 0)
        return 6;
    if (tbo_window_has_unsaved_changes (tbo))
        return 7;

    opened = tbo_new_tbo (app, 300, 200);
    tbo_comic_open (opened, path);
    if (g_strcmp0 (gtk_window_get_title (GTK_WINDOW (opened->window)), basename) != 0)
        return 8;
    if (tbo_window_has_unsaved_changes (opened))
        return 9;

    g_free (basename);
    g_remove (path);
    g_free (path);
    tbo_window_mark_clean (tbo);
    tbo_window_mark_clean (opened);
    gtk_window_close (GTK_WINDOW (tbo->window));
    gtk_window_close (GTK_WINDOW (opened->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
