#include <gtk/gtk.h>

#include "comic.h"
#include "tbo-widget.h"
#include "tbo-window.h"

int
main (void)
{
    GtkApplication *app;
    TboWindow *tbo;
    gchar *window_title;
    gchar *comic_title;

    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.savefailure", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo = tbo_new_tbo (app, 800, 450);
    tbo_window_mark_dirty (tbo);
    window_title = g_strdup (gtk_window_get_title (GTK_WINDOW (tbo->window)));
    comic_title = g_strdup (tbo_comic_get_title (tbo->comic));

    tbo_alert_set_test_response (0);
    if (tbo_comic_save (tbo, "/dev/full"))
        return 3;
    tbo_alert_clear_test_response ();

    if (!tbo_window_has_unsaved_changes (tbo))
        return 4;
    if (tbo->path != NULL)
        return 5;
    if (g_strcmp0 (tbo_comic_get_title (tbo->comic), comic_title) != 0)
        return 6;
    if (g_strcmp0 (gtk_window_get_title (GTK_WINDOW (tbo->window)), window_title) != 0)
        return 7;

    g_free (window_title);
    g_free (comic_title);
    tbo_window_mark_clean (tbo);
    gtk_window_close (GTK_WINDOW (tbo->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
