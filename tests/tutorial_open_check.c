#include <gtk/gtk.h>
#include <string.h>

#include "comic.h"
#include "page.h"
#include "tbo-widget.h"
#include "tbo-window.h"

int
main (void)
{
    GtkApplication *app;
    TboWindow *tbo;
    Page *page;
    gchar *original_title;

    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.tutorialopen", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo = tbo_new_tbo (app, 800, 450);
    page = tbo_comic_get_current_page (tbo->comic);
    tbo_page_new_frame (page, 10, 10, 120, 90);
    tbo_window_mark_dirty (tbo);
    original_title = g_strdup (tbo_comic_get_title (tbo->comic));

    tbo_alert_set_test_response (0);
    g_action_group_activate_action (G_ACTION_GROUP (tbo->window), "tutorial", NULL);
    if (!tbo_window_has_unsaved_changes (tbo))
        return 3;
    if (strcmp (tbo_comic_get_title (tbo->comic), original_title) != 0)
        return 4;
    if (tbo_page_len (tbo_comic_get_current_page (tbo->comic)) != 1)
        return 5;

    tbo_alert_set_test_response (1);
    g_action_group_activate_action (G_ACTION_GROUP (tbo->window), "tutorial", NULL);
    tbo_alert_clear_test_response ();

    if (tbo_window_has_unsaved_changes (tbo))
        return 6;
    if (g_strcmp0 (tbo_comic_get_title (tbo->comic), "tut.tbo") != 0)
        return 7;
    if (tbo->path != NULL)
        return 8;
    if (tbo_comic_len (tbo->comic) == 0)
        return 9;

    g_free (original_title);
    tbo_window_mark_clean (tbo);
    gtk_window_close (GTK_WINDOW (tbo->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
