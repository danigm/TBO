#include <gtk/gtk.h>
#include <glib/gstdio.h>

#include "comic.h"
#include "export.h"
#include "frame.h"
#include "page.h"
#include "tbo-window.h"

int
main (void)
{
    GtkApplication *app;
    TboWindow *tbo;
    Page *page;
    Frame *frame;
    gchar *tmpbase;
    gchar *pngfile;
    gint fd;
    gboolean exported;

    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.exportresult", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo = tbo_new_tbo (app, 800, 450);
    page = tbo_comic_get_current_page (tbo->comic);
    frame = tbo_page_new_frame (page, 10, 10, 100, 80);
    tbo_frame_set_color_rgb (frame, 0.4, 0.5, 0.6);

    tmpbase = g_build_filename (g_get_tmp_dir (), "tbo-export-result-XXXXXX", NULL);
    fd = g_mkstemp (tmpbase);
    if (fd < 0)
        return 3;
    close (fd);
    g_remove (tmpbase);

    exported = tbo_export_file (tbo, tmpbase, "png", 800, 450);
    pngfile = g_strdup_printf ("%s.png", tmpbase);
    if (!exported || !g_file_test (pngfile, G_FILE_TEST_EXISTS))
        return 4;

    if (tbo_export_file (tbo, "", "png", 800, 450))
        return 5;

    g_remove (pngfile);
    g_free (pngfile);
    g_free (tmpbase);
    tbo_window_mark_clean (tbo);
    gtk_window_close (GTK_WINDOW (tbo->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
