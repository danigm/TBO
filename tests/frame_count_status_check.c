#include <gtk/gtk.h>
#include <string.h>

#include "tbo-tool-base.h"
#include "tbo-toolbar.h"
#include "tbo-window.h"

int
main (void)
{
    GtkApplication *app;
    TboWindow *tbo;
    TboToolBase *tool;
    TboPointerEvent click_event = { .x = 10, .y = 10 };
    TboPointerEvent release_event = { .x = 110, .y = 90 };
    const gchar *status;

    g_setenv ("LC_ALL", "C.UTF-8", TRUE);
    g_setenv ("LANGUAGE", "C", TRUE);
    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.framecountstatus", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo = tbo_new_tbo (app, 800, 450);
    tbo_window_refresh_status (tbo);
    status = gtk_label_get_text (GTK_LABEL (tbo->status));
    if (strstr (status, "Frames: 0") == NULL)
        return 3;

    tbo_toolbar_set_selected_tool (tbo->toolbar, TBO_TOOLBAR_FRAME);
    tool = tbo_toolbar_get_selected_tool (tbo->toolbar);
    tool->on_click (tool, tbo->drawing, &click_event);
    tool->on_release (tool, tbo->drawing, &release_event);

    status = gtk_label_get_text (GTK_LABEL (tbo->status));
    if (strstr (status, "Frames: 1") == NULL)
        return 4;

    tbo_window_mark_clean (tbo);
    gtk_window_close (GTK_WINDOW (tbo->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
