#include <gtk/gtk.h>

#include "comic.h"
#include "frame.h"
#include "page.h"
#include "tbo-drawing.h"
#include "tbo-tool-selector.h"
#include "tbo-toolbar.h"
#include "tbo-window.h"

static GtkEventControllerKey *
find_window_key_controller (GtkWidget *window)
{
    GListModel *controllers;
    guint i;

    controllers = gtk_widget_observe_controllers (window);
    for (i = 0; i < g_list_model_get_n_items (controllers); i++)
    {
        GtkEventController *controller = g_list_model_get_item (controllers, i);

        if (GTK_IS_EVENT_CONTROLLER_KEY (controller) &&
            gtk_event_controller_get_propagation_phase (controller) == GTK_PHASE_CAPTURE)
        {
            g_object_unref (controllers);
            return GTK_EVENT_CONTROLLER_KEY (controller);
        }

        g_object_unref (controller);
    }

    g_object_unref (controllers);
    return NULL;
}

int
main (void)
{
    GtkApplication *app;
    TboWindow *tbo;
    TboToolSelector *selector;
    Page *page;
    Frame *frame;
    GtkEventControllerKey *controller;
    gboolean handled = FALSE;

    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.enterframe", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo = tbo_new_tbo (app, 800, 450);
    selector = TBO_TOOL_SELECTOR (tbo->toolbar->tools[TBO_TOOLBAR_SELECTOR]);
    page = tbo_comic_get_current_page (tbo->comic);
    frame = tbo_page_new_frame (page, 20, 20, 100, 80);

    tbo_toolbar_set_selected_tool (tbo->toolbar, TBO_TOOLBAR_SELECTOR);
    tbo_tool_selector_set_selected (selector, frame);
    gtk_widget_grab_focus (tbo->notebook);

    controller = find_window_key_controller (tbo->window);
    if (controller == NULL)
        return 3;

    g_signal_emit_by_name (controller, "key-pressed", GDK_KEY_Return, 0u, 0u, &handled);
    if (!handled)
        return 4;
    if (tbo_drawing_get_current_frame (TBO_DRAWING (tbo->drawing)) != frame)
        return 5;

    g_object_unref (controller);
    tbo_window_mark_clean (tbo);
    gtk_window_close (GTK_WINDOW (tbo->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
