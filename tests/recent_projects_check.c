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
    TboWindow *first;
    TboWindow *second;
    TboWindow *reopened;
    gchar *path1;
    gchar *path2;
    gchar **recent_paths;
    gchar *last_project;
    gsize recent_count = 0;

    gtk_init ();
    tbo_window_clear_persisted_state ();

    app = gtk_application_new ("net.danigm.tbo.recentprojects", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    first = tbo_new_tbo (app, 810, 610);
    path1 = make_tmp_project_path ("tbo-recent-one-XXXXXX");
    if (!tbo_comic_save (first, path1))
        return 3;
    tbo_window_set_path (first, path1);
    tbo_window_add_recent_project (path1);

    second = tbo_new_tbo (app, 920, 730);
    path2 = make_tmp_project_path ("tbo-recent-two-XXXXXX");
    if (!tbo_comic_save (second, path2))
        return 4;
    tbo_window_set_path (second, path2);
    tbo_window_add_recent_project (path2);

    recent_paths = tbo_window_get_recent_projects (&recent_count);
    if (recent_count != 2)
        return 5;
    if (g_strcmp0 (recent_paths[0], path2) != 0 || g_strcmp0 (recent_paths[1], path1) != 0)
        return 6;
    g_strfreev (recent_paths);

    last_project = tbo_window_get_last_project ();
    if (g_strcmp0 (last_project, path2) != 0)
        return 7;
    g_free (last_project);

    reopened = tbo_new_tbo (app, 400, 300);
    if (!tbo_window_reopen_last_project (reopened))
        return 8;
    if (g_strcmp0 (reopened->path, path2) != 0)
        return 9;
    if (tbo_comic_get_width (reopened->comic) != 920 || tbo_comic_get_height (reopened->comic) != 730)
        return 10;

    g_remove (path1);
    g_remove (path2);
    g_free (path1);
    g_free (path2);
    tbo_window_mark_clean (first);
    tbo_window_mark_clean (second);
    tbo_window_mark_clean (reopened);
    gtk_window_close (GTK_WINDOW (first->window));
    gtk_window_close (GTK_WINDOW (second->window));
    gtk_window_close (GTK_WINDOW (reopened->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
