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
    gchar *autosave_path;
    gchar *original_title;
    Page *page;

    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.recoverfilefailure", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo = tbo_new_tbo (app, 800, 450);
    page = tbo_comic_get_current_page (tbo->comic);
    tbo_page_new_frame (page, 10, 10, 120, 90);
    original_title = g_strdup (tbo_comic_get_title (tbo->comic));

    autosave_path = make_tmp_path ("tbo-invalid-recovery-XXXXXX.tbo");
    if (!g_file_set_contents (autosave_path, "<broken", -1, NULL))
        return 3;

    tbo_alert_set_test_response (0);
    if (tbo_window_recover_file (tbo, autosave_path))
        return 4;
    tbo_alert_clear_test_response ();

    if (!g_file_test (autosave_path, G_FILE_TEST_EXISTS))
        return 5;
    if (g_strcmp0 (tbo_comic_get_title (tbo->comic), original_title) != 0)
        return 6;
    if (tbo_page_len (tbo_comic_get_current_page (tbo->comic)) != 1)
        return 7;

    g_remove (autosave_path);
    g_free (autosave_path);
    g_free (original_title);
    tbo_window_mark_clean (tbo);
    gtk_window_close (GTK_WINDOW (tbo->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
