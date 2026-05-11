#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <unistd.h>

#include "comic.h"
#include "export.h"
#include "frame.h"
#include "page.h"
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
    Page *page;
    gchar *base;
    gchar *filename;
    gchar *expected;
    gchar *unexpected;

    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.exportextensionhint", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo = tbo_new_tbo (app, 800, 450);
    page = tbo_comic_get_current_page (tbo->comic);
    tbo_page_new_frame (page, 10, 10, 120, 90);

    base = make_tmp_base ("tbo-export-hint-XXXXXX");
    filename = g_strdup_printf ("%s.png", base);
    if (!tbo_export_file (tbo, filename, "png", 800, 450))
        return 3;
    if (!g_file_test (filename, G_FILE_TEST_EXISTS))
        return 4;
    unexpected = g_strdup_printf ("%s.png", filename);
    if (g_file_test (unexpected, G_FILE_TEST_EXISTS))
        return 5;

    g_remove (filename);
    g_free (unexpected);

    tbo_comic_new_page (tbo->comic);
    filename = g_strdup_printf ("%s-pages.png", base);
    if (!tbo_export_file (tbo, filename, "png", 800, 450))
        return 6;
    expected = g_strdup_printf ("%s-pages0.png", base);
    if (!g_file_test (expected, G_FILE_TEST_EXISTS))
        return 7;
    g_remove (expected);
    g_free (expected);
    expected = g_strdup_printf ("%s-pages1.png", base);
    if (!g_file_test (expected, G_FILE_TEST_EXISTS))
        return 8;
    g_remove (expected);
    g_free (expected);
    unexpected = g_strdup_printf ("%s0.png", filename);
    if (g_file_test (unexpected, G_FILE_TEST_EXISTS))
        return 9;

    g_free (unexpected);
    g_free (filename);
    g_free (base);
    tbo_window_mark_clean (tbo);
    gtk_window_close (GTK_WINDOW (tbo->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
