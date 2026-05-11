#include "tbo-toolbar.h"
#include "tbo-window.h"

int
main (void)
{
    GtkApplication *app;
    TboWindow *tbo1;
    TboWindow *tbo2;

    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.keybinderscope", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo1 = tbo_new_tbo (app, 800, 450);
    tbo2 = tbo_new_tbo (app, 800, 450);

    tbo_window_set_key_binder (tbo1, FALSE);

    tbo_toolbar_set_selected_tool (tbo1->toolbar, TBO_TOOLBAR_FRAME);
    if (tbo_window_handle_unmodified_key (tbo1, GDK_KEY_s, 0))
        return 3;
    if (tbo_toolbar_get_selected_tool (tbo1->toolbar) != tbo1->toolbar->tools[TBO_TOOLBAR_FRAME])
        return 4;

    tbo_toolbar_set_selected_tool (tbo2->toolbar, TBO_TOOLBAR_FRAME);
    if (!tbo_window_handle_unmodified_key (tbo2, GDK_KEY_s, 0))
        return 5;
    if (tbo_toolbar_get_selected_tool (tbo2->toolbar) != tbo2->toolbar->tools[TBO_TOOLBAR_SELECTOR])
        return 6;

    tbo_window_mark_clean (tbo1);
    tbo_window_mark_clean (tbo2);
    gtk_window_close (GTK_WINDOW (tbo1->window));
    gtk_window_close (GTK_WINDOW (tbo2->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
