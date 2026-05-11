#include <gtk/gtk.h>
#include <math.h>

#include "comic.h"
#include "frame.h"
#include "page.h"
#include "tbo-object-text.h"
#include "tbo-tool-base.h"
#include "tbo-tool-selector.h"
#include "tbo-toolbar.h"
#include "tbo-undo.h"
#include "tbo-window.h"

int
main (void)
{
    GtkApplication *app;
    TboWindow *tbo;
    TboToolSelector *selector;
    Page *page;
    Frame *frame;
    TboObjectBase *obj;
    TboPointerEvent move_event = { 0 };
    TboPointerEvent release_event = { 0 };
    GdkRGBA color = { 0, 0, 0, 1 };

    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.resizeundo", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo = tbo_new_tbo (app, 800, 450);
    selector = TBO_TOOL_SELECTOR (tbo->toolbar->tools[TBO_TOOLBAR_SELECTOR]);
    page = tbo_comic_get_current_page (tbo->comic);
    frame = tbo_page_new_frame (page, 20, 20, 100, 80);

    tbo_toolbar_set_selected_tool (tbo->toolbar, TBO_TOOLBAR_SELECTOR);
    tbo_tool_selector_set_selected (selector, frame);
    selector->clicked = TRUE;
    selector->resizing = TRUE;
    selector->start_x = 100;
    selector->start_y = 80;
    selector->start_m_x = tbo_frame_get_x (frame);
    selector->start_m_y = tbo_frame_get_y (frame);
    selector->start_m_w = tbo_frame_get_width (frame);
    selector->start_m_h = tbo_frame_get_height (frame);
    move_event.x = 0;
    move_event.y = 0;
    TBO_TOOL_BASE (selector)->on_move (TBO_TOOL_BASE (selector), tbo->drawing, &move_event);
    if (tbo_frame_get_width (frame) != 1 || tbo_frame_get_height (frame) != 1)
        return 3;

    TBO_TOOL_BASE (selector)->on_release (TBO_TOOL_BASE (selector), tbo->drawing, &release_event);
    if (!tbo_undo_active_undo (tbo->undo_stack))
        return 4;

    tbo_undo_stack_undo (tbo->undo_stack);
    if (tbo_frame_get_width (frame) != 100 || tbo_frame_get_height (frame) != 80)
        return 5;
    tbo_undo_stack_redo (tbo->undo_stack);
    if (tbo_frame_get_width (frame) != 1 || tbo_frame_get_height (frame) != 1)
        return 6;

    tbo_undo_stack_clear (tbo->undo_stack);

    obj = TBO_OBJECT_BASE (tbo_object_text_new_with_params (10, 10, 60, 20, "rotate", "Sans 12", &color));
    tbo_frame_add_obj (frame, obj);
    tbo_window_enter_frame (tbo, frame);
    tbo_tool_selector_set_selected_obj (selector, obj);
    selector->clicked = TRUE;
    selector->rotating = TRUE;
    selector->start_m_x = obj->x;
    selector->start_m_y = obj->y;
    selector->start_m_w = obj->width;
    selector->start_m_h = obj->height;
    selector->start_m_angle = obj->angle;
    obj->angle = 0.75;

    TBO_TOOL_BASE (selector)->on_release (TBO_TOOL_BASE (selector), tbo->drawing, &release_event);
    if (!tbo_undo_active_undo (tbo->undo_stack))
        return 7;

    tbo_undo_stack_undo (tbo->undo_stack);
    if (fabs (obj->angle) > 1e-9)
        return 8;
    tbo_undo_stack_redo (tbo->undo_stack);
    if (fabs (obj->angle - 0.75) > 1e-9)
        return 9;

    tbo_window_mark_clean (tbo);
    gtk_window_close (GTK_WINDOW (tbo->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
