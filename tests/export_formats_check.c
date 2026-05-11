#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <string.h>
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
    Page *page1;
    Page *page2;
    gchar *svg_base;
    gchar *png_base;
    gchar *pdf_base;
    gchar *file0 = NULL;
    gchar *file1 = NULL;
    gchar *svg0 = NULL;
    gchar *svg1 = NULL;
    gchar *pdffile = NULL;
    gchar *contents = NULL;

    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.exportformats", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo = tbo_new_tbo (app, 800, 450);
    page1 = tbo_comic_get_current_page (tbo->comic);
    tbo_page_new_frame (page1, 10, 10, 120, 90);
    page2 = tbo_comic_new_page (tbo->comic);
    tbo_page_new_frame (page2, 20, 20, 140, 100);

    svg_base = make_tmp_base ("tbo-export-svg-XXXXXX");
    if (!tbo_export_file (tbo, svg_base, "svg", 800, 450))
        return 3;
    svg0 = g_strdup_printf ("%s0.svg", svg_base);
    svg1 = g_strdup_printf ("%s1.svg", svg_base);
    if (!g_file_test (svg0, G_FILE_TEST_EXISTS) || !g_file_test (svg1, G_FILE_TEST_EXISTS))
        return 4;
    if (!g_file_get_contents (svg0, &contents, NULL, NULL) || strstr (contents, "<svg") == NULL)
        return 5;
    g_free (contents);
    contents = NULL;

    png_base = make_tmp_base ("tbo-export-png-XXXXXX");
    if (!tbo_export_file (tbo, png_base, "png", 800, 450))
        return 6;
    g_free (file0);
    g_free (file1);
    file0 = g_strdup_printf ("%s0.png", png_base);
    file1 = g_strdup_printf ("%s1.png", png_base);
    if (!g_file_test (file0, G_FILE_TEST_EXISTS) || !g_file_test (file1, G_FILE_TEST_EXISTS))
        return 7;

    pdf_base = make_tmp_base ("tbo-export-pdf-XXXXXX");
    if (!tbo_export_file (tbo, pdf_base, "pdf", 800, 450))
        return 8;
    pdffile = g_strdup_printf ("%s.pdf", pdf_base);
    if (!g_file_test (pdffile, G_FILE_TEST_EXISTS))
        return 9;
    if (!g_file_get_contents (pdffile, &contents, NULL, NULL) || g_str_has_prefix (contents, "%PDF") == FALSE)
        return 10;

    g_free (contents);
    g_remove (file0);
    g_remove (file1);
    g_remove (pdffile);
    g_remove (svg0);
    g_remove (svg1);
    g_free (file0);
    g_free (file1);
    g_free (svg0);
    g_free (svg1);
    g_free (pdffile);
    g_free (svg_base);
    g_free (png_base);
    g_free (pdf_base);

    tbo_window_mark_clean (tbo);
    gtk_window_close (GTK_WINDOW (tbo->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
