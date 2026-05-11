#include <gtk/gtk.h>
#include <string.h>

#include "comic.h"
#include "page.h"
#include "tbo-window.h"

int
main (void)
{
    GtkApplication *app;
    TboWindow *tbo;
    Page *page2;
    const gchar *status;

    g_setenv ("LC_ALL", "C.UTF-8", TRUE);
    g_setenv ("LANGUAGE", "C", TRUE);
    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.pagestatus", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo = tbo_new_tbo (app, 800, 450);
    page2 = tbo_comic_new_page (tbo->comic);
    tbo_window_add_page_widget (tbo, create_darea (tbo), page2);
    tbo_comic_set_current_page (tbo->comic, page2);
    tbo_window_set_current_tab_page (tbo, TRUE);
    tbo_window_refresh_status (tbo);

    status = gtk_label_get_text (GTK_LABEL (tbo->status));
    if (g_str_has_prefix (status, "Mode: Page | Page 2 of 2 | Frames: 0 | Enter: frame") == FALSE)
        return 3;

    tbo_window_mark_clean (tbo);
    gtk_window_close (GTK_WINDOW (tbo->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
