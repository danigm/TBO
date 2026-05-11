#include <gtk/gtk.h>

#include "comic.h"
#include "frame.h"
#include "page.h"
#include "tbo-tool-selector.h"
#include "tbo-toolbar.h"
#include "tbo-window.h"

int
main (void)
{
    GtkApplication *app;
    TboWindow *tbo;
    TboToolSelector *selector;
    Page *page;
    Frame *frame;
    gint frames_before;

    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.clonedirty", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo = tbo_new_tbo (app, 800, 450);
    selector = TBO_TOOL_SELECTOR (tbo->toolbar->tools[TBO_TOOLBAR_SELECTOR]);
    page = tbo_comic_get_current_page (tbo->comic);
    frame = tbo_page_new_frame (page, 20, 20, 100, 80);

    tbo_toolbar_set_selected_tool (tbo->toolbar, TBO_TOOLBAR_SELECTOR);
    tbo_window_mark_clean (tbo);
    g_action_group_activate_action (G_ACTION_GROUP (tbo->window), "clone", NULL);
    if (tbo_window_has_unsaved_changes (tbo))
        return 3;

    frames_before = tbo_page_len (page);
    tbo_tool_selector_set_selected (selector, frame);
    g_action_group_activate_action (G_ACTION_GROUP (tbo->window), "clone", NULL);
    if (!tbo_window_has_unsaved_changes (tbo) || tbo_page_len (page) != frames_before + 1)
        return 4;

    tbo_window_mark_clean (tbo);
    gtk_window_close (GTK_WINDOW (tbo->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
