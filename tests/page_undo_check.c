#include <gtk/gtk.h>

#include "comic.h"
#include "tbo-toolbar.h"
#include "tbo-window.h"

int
main (void)
{
    GtkApplication *app;
    TboWindow *tbo;

    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.pageundo", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo = tbo_new_tbo (app, 800, 450);
    if (tbo_window_get_page_count (tbo) != 1)
        return 3;

    g_signal_emit_by_name (tbo->toolbar->button_new_page, "clicked");
    if (tbo_comic_len (tbo->comic) != 2 || tbo_window_get_page_count (tbo) != 2)
        return 4;

    tbo_window_undo_cb (NULL, tbo);
    if (tbo_comic_len (tbo->comic) != 1 || tbo_window_get_page_count (tbo) != 1)
        return 5;

    tbo_window_redo_cb (NULL, tbo);
    if (tbo_comic_len (tbo->comic) != 2 || tbo_window_get_page_count (tbo) != 2)
        return 6;

    g_signal_emit_by_name (tbo->toolbar->button_delete_page, "clicked");
    if (tbo_comic_len (tbo->comic) != 1 || tbo_window_get_page_count (tbo) != 1)
        return 7;

    tbo_window_undo_cb (NULL, tbo);
    if (tbo_comic_len (tbo->comic) != 2 || tbo_window_get_page_count (tbo) != 2)
        return 8;

    tbo_window_mark_clean (tbo);
    gtk_window_close (GTK_WINDOW (tbo->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
