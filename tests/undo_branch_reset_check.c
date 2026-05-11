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

    app = gtk_application_new ("net.danigm.tbo.undobranchreset", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo = tbo_new_tbo (app, 800, 450);

    g_signal_emit_by_name (tbo->toolbar->button_new_page, "clicked");
    if (tbo_comic_len (tbo->comic) != 2)
        return 3;

    tbo_window_undo_cb (NULL, tbo);
    if (tbo_comic_len (tbo->comic) != 1)
        return 4;

    g_signal_emit_by_name (tbo->toolbar->button_new_page, "clicked");
    if (tbo_comic_len (tbo->comic) != 2)
        return 7;
    if (g_list_length (tbo->undo_stack->first) != 1)
        return 6;

    tbo_window_undo_cb (NULL, tbo);
    if (tbo_comic_len (tbo->comic) != 1)
        return 8;
    if (!tbo->undo_stack->last_flag || tbo->undo_stack->first != tbo->undo_stack->list)
        return 9;

    tbo_window_redo_cb (NULL, tbo);
    if (tbo_comic_len (tbo->comic) != 2)
        return 10;
    if (g_list_length (tbo->undo_stack->first) != 1)
        return 11;

    tbo_window_mark_clean (tbo);
    gtk_window_close (GTK_WINDOW (tbo->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
