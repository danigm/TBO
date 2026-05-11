#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <string.h>
#include <unistd.h>

#include "comic.h"
#include "frame.h"
#include "page.h"
#include "tbo-toolbar.h"
#include "tbo-window.h"

int
main (void)
{
    GtkApplication *app;
    TboWindow *tbo;
    Page *page;
    Frame *frame;
    gchar *tmpname;
    gint fd;
    gchar *contents = NULL;

    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.toolbarsave", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo = tbo_new_tbo (app, 800, 450);
    page = tbo_comic_get_current_page (tbo->comic);
    frame = tbo_page_new_frame (page, 10, 10, 100, 80);
    tbo_frame_set_color_rgb (frame, 0.4, 0.5, 0.6);
    tbo_window_mark_dirty (tbo);

    tmpname = g_build_filename (g_get_tmp_dir (), "tbo-toolbar-save-XXXXXX.tbo", NULL);
    fd = g_mkstemp (tmpname);
    if (fd < 0)
        return 3;
    close (fd);

    tbo_window_set_path (tbo, tmpname);
    g_signal_emit_by_name (tbo->toolbar->button_save, "clicked");
    while (g_main_context_iteration (NULL, FALSE));

    if (tbo_window_has_unsaved_changes (tbo))
        return 4;
    if (!g_file_test (tmpname, G_FILE_TEST_EXISTS))
        return 5;
    if (!g_file_get_contents (tmpname, &contents, NULL, NULL) || strstr (contents, "<tbo") == NULL)
        return 6;

    g_free (contents);
    g_remove (tmpname);
    g_free (tmpname);
    tbo_window_mark_clean (tbo);
    gtk_window_close (GTK_WINDOW (tbo->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
