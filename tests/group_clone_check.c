#include <gtk/gtk.h>

#include "comic.h"
#include "frame.h"
#include "page.h"
#include "tbo-drawing.h"
#include "tbo-object-group.h"
#include "tbo-object-text.h"
#include "tbo-tool-selector.h"
#include "tbo-toolbar.h"
#include "tbo-window.h"

static void
drain_events (void)
{
    while (g_main_context_iteration (NULL, FALSE));
}

int
main (void)
{
    GtkApplication *app;
    TboWindow *tbo;
    TboToolSelector *selector;
    Page *page;
    Frame *frame;
    TboObjectGroup *group;
    TboObjectBase *first;
    TboObjectBase *second;
    GdkRGBA color = { 0, 0, 0, 1 };
    gint count_before;

    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.groupclone", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo = tbo_new_tbo (app, 800, 450);
    selector = TBO_TOOL_SELECTOR (tbo->toolbar->tools[TBO_TOOLBAR_SELECTOR]);
    page = tbo_comic_get_current_page (tbo->comic);
    frame = tbo_page_new_frame (page, 20, 20, 120, 80);
    first = TBO_OBJECT_BASE (tbo_object_text_new_with_params (10, 10, 40, 20, "one", "Sans 12", &color));
    second = TBO_OBJECT_BASE (tbo_object_text_new_with_params (50, 20, 40, 20, "two", "Sans 12", &color));
    tbo_frame_add_obj (frame, first);
    tbo_frame_add_obj (frame, second);

    group = TBO_OBJECT_GROUP (tbo_object_group_new ());
    tbo_object_group_add (group, first);
    tbo_object_group_add (group, second);
    tbo_frame_add_obj (frame, TBO_OBJECT_BASE (group));

    tbo_window_enter_frame (tbo, frame);
    tbo_toolbar_set_selected_tool (tbo->toolbar, TBO_TOOLBAR_SELECTOR);
    tbo_tool_selector_set_selected (selector, frame);
    tbo_tool_selector_set_selected_obj (selector, TBO_OBJECT_BASE (group));
    if (tbo_drawing_get_current_frame (TBO_DRAWING (tbo->drawing)) != frame)
        return 3;

    count_before = tbo_frame_object_count (frame);
    tbo_window_mark_clean (tbo);
    g_action_group_activate_action (G_ACTION_GROUP (tbo->window), "clone", NULL);
    drain_events ();

    if (!tbo_window_has_unsaved_changes (tbo))
        return 4;
    if (tbo_frame_object_count (frame) != count_before + 1)
        return 5;

    tbo_window_mark_clean (tbo);
    gtk_window_close (GTK_WINDOW (tbo->window));
    drain_events ();
    g_object_unref (app);
    return 0;
}
