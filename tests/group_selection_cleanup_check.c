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

static TboObjectGroup *
add_group (Frame *frame, TboObjectBase *first, TboObjectBase *second)
{
    TboObjectGroup *group = TBO_OBJECT_GROUP (tbo_object_group_new ());

    tbo_object_group_add (group, first);
    tbo_object_group_add (group, second);
    tbo_frame_add_obj (frame, TBO_OBJECT_BASE (group));
    return group;
}

int
main (void)
{
    GtkApplication *app;
    TboWindow *tbo;
    TboToolSelector *selector;
    Page *page;
    Frame *frame;
    TboObjectBase *first;
    TboObjectBase *second;
    TboObjectGroup *group;
    GdkRGBA color = { 0, 0, 0, 1 };
    gint count_with_group;

    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.groupcleanup", G_APPLICATION_DEFAULT_FLAGS);
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

    tbo_window_enter_frame (tbo, frame);
    if (tbo_drawing_get_current_frame (TBO_DRAWING (tbo->drawing)) != frame)
        return 3;

    group = add_group (frame, first, second);
    tbo_toolbar_set_selected_tool (tbo->toolbar, TBO_TOOLBAR_SELECTOR);
    tbo_tool_selector_set_selected (selector, frame);
    tbo_tool_selector_set_selected_obj (selector, TBO_OBJECT_BASE (group));
    count_with_group = tbo_frame_object_count (frame);

    tbo_tool_selector_set_selected_obj (selector, first);
    if (tbo_frame_object_count (frame) != count_with_group - 1)
        return 4;
    if (tbo_tool_selector_get_selected_obj (selector) != first)
        return 5;

    group = add_group (frame, first, second);
    tbo_tool_selector_set_selected_obj (selector, TBO_OBJECT_BASE (group));
    count_with_group = tbo_frame_object_count (frame);

    tbo_tool_selector_reset_state (selector);
    if (tbo_frame_object_count (frame) != count_with_group - 1)
        return 6;
    if (selector->selected_frame != NULL || selector->selected_object != NULL)
        return 7;

    gtk_window_close (GTK_WINDOW (tbo->window));
    drain_events ();
    g_object_unref (app);
    return 0;
}
