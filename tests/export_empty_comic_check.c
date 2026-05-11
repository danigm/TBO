#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <unistd.h>

#include "comic.h"
#include "export.h"
#include "tbo-widget.h"
#include "tbo-window.h"

static gchar *
make_tmp_base (const gchar *pattern)
{
    gchar *path = g_build_filename (g_get_tmp_dir (), pattern, NULL);
    gint fd = g_mkstemp (path);

    if (fd >= 0)
        close (fd);

    g_remove (path);
    return path;
}

int
main (void)
{
    GtkApplication *app;
    TboWindow *tbo;
    gchar *base;
    gchar *png_path;

    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.exportemptycomic", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo = tbo_new_tbo (app, 800, 450);
    tbo_comic_del_page (tbo->comic, 0);
    if (tbo_comic_len (tbo->comic) != 0)
        return 3;

    base = make_tmp_base ("tbo-export-empty-XXXXXX");
    png_path = g_strdup_printf ("%s.png", base);
    tbo_alert_set_test_response (0);
    if (tbo_export_file_with_scope_range (tbo, base, "png", 800, 450, TBO_EXPORT_SCOPE_ALL_PAGES, 1, 1))
        return 4;
    if (tbo_export_file_with_scope_range (tbo, base, "png", 800, 450, TBO_EXPORT_SCOPE_CURRENT_PAGE, 1, 1))
        return 5;
    tbo_alert_clear_test_response ();
    if (g_file_test (png_path, G_FILE_TEST_EXISTS))
        return 6;

    g_free (png_path);
    g_free (base);
    tbo_window_mark_clean (tbo);
    gtk_window_close (GTK_WINDOW (tbo->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
