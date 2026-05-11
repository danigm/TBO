#include <gtk/gtk.h>
#include <string.h>

#include "comic.h"
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
    const gchar *status;

    g_setenv ("LC_ALL", "C.UTF-8", TRUE);
    g_setenv ("LANGUAGE", "C", TRUE);
    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.modestatus", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo = tbo_new_tbo (app, 800, 450);
    page = tbo_comic_get_current_page (tbo->comic);
    frame = tbo_page_new_frame (page, 20, 20, 120, 90);

    tbo_window_refresh_status (tbo);
    status = gtk_label_get_text (GTK_LABEL (tbo->status));
    if (strstr (status, "Mode: Page") == NULL)
        return 3;

    tbo_window_enter_frame (tbo, frame);
    status = gtk_label_get_text (GTK_LABEL (tbo->status));
    if (strstr (status, "Mode: Frame") == NULL)
        return 4;

    tbo_window_leave_frame (tbo);
    status = gtk_label_get_text (GTK_LABEL (tbo->status));
    if (strstr (status, "Mode: Page") == NULL)
        return 5;

    tbo_window_mark_clean (tbo);
    gtk_window_close (GTK_WINDOW (tbo->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
