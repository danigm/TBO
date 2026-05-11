#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <unistd.h>

#include "comic.h"
#include "frame.h"
#include "page.h"
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
    TboWindow *recovered;
    Page *page;
    gchar *project_path;

    gtk_init ();
    tbo_window_clear_persisted_state ();

    app = gtk_application_new ("net.danigm.tbo.autosaverecovery", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo = tbo_new_tbo (app, 800, 450);
    page = tbo_comic_get_current_page (tbo->comic);
    tbo_page_new_frame (page, 10, 10, 120, 90);

    project_path = make_tmp_project_path ("tbo-autosave-recovery-XXXXXX");
    if (!tbo_comic_save (tbo, project_path))
        return 3;
    tbo_window_set_path (tbo, project_path);

    tbo_page_new_frame (page, 160, 10, 120, 90);
    tbo_window_mark_dirty (tbo);
    if (!tbo_window_run_autosave (tbo))
        return 4;
    if (!g_file_test (tbo->autosave_path, G_FILE_TEST_EXISTS))
        return 5;

    recovered = tbo_new_tbo (app, 800, 450);
    if (!tbo_window_recover_file (recovered, tbo->autosave_path))
        return 6;
    if (g_file_test (tbo->autosave_path, G_FILE_TEST_EXISTS))
        return 7;
    if (!tbo_window_has_unsaved_changes (recovered))
        return 8;
    {
        gchar *expected_title = g_strdup_printf ("* %s", tbo_comic_get_title (recovered->comic));

        if (g_strcmp0 (gtk_window_get_title (GTK_WINDOW (recovered->window)), expected_title) != 0)
            return 11;
        g_free (expected_title);
    }
    if (g_strcmp0 (recovered->path, project_path) != 0)
        return 9;
    if (tbo_page_len (tbo_comic_get_current_page (recovered->comic)) != 2)
        return 10;

    g_remove (project_path);
    g_free (project_path);
    tbo_window_mark_clean (tbo);
    tbo_window_mark_clean (recovered);
    gtk_window_close (GTK_WINDOW (tbo->window));
    gtk_window_close (GTK_WINDOW (recovered->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
