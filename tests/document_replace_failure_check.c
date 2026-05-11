#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <unistd.h>

#include "comic.h"
#include "page.h"
#include "tbo-widget.h"
#include "tbo-window.h"

static gchar *
make_tmp_path (const gchar *pattern)
{
    gchar *path = g_build_filename (g_get_tmp_dir (), pattern, NULL);
    gint fd = g_mkstemp (path);

    if (fd >= 0)
        close (fd);

    return path;
}

int
main (void)
{
    GtkApplication *app;
    TboWindow *tbo;
    Page *page;
    gchar *invalid_path;

    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.documentreplacefailure", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo = tbo_new_tbo (app, 800, 450);
    page = tbo_comic_get_current_page (tbo->comic);
    tbo_page_new_frame (page, 10, 10, 120, 90);
    tbo_window_mark_dirty (tbo);
    if (!tbo_window_run_autosave (tbo))
        return 3;
    if (!g_file_test (tbo->autosave_path, G_FILE_TEST_EXISTS))
        return 4;

    invalid_path = make_tmp_path ("tbo-invalid-replace-XXXXXX.tbo");
    if (!g_file_set_contents (invalid_path, "<broken", -1, NULL))
        return 5;

    tbo_alert_set_test_response (1);
    if (tbo_window_open_recent_project (tbo, invalid_path))
        return 6;
    tbo_alert_clear_test_response ();

    if (!tbo_window_has_unsaved_changes (tbo))
        return 7;
    if (!g_file_test (tbo->autosave_path, G_FILE_TEST_EXISTS))
        return 8;
    if (tbo_page_len (tbo_comic_get_current_page (tbo->comic)) != 1)
        return 9;
    if (tbo->path != NULL)
        return 10;

    g_remove (invalid_path);
    g_free (invalid_path);
    tbo_window_mark_clean (tbo);
    gtk_window_close (GTK_WINDOW (tbo->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
