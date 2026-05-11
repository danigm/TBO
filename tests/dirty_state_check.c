#include <gtk/gtk.h>

#include "comic.h"
#include "frame.h"
#include "page.h"
#include "tbo-object-text.h"
#include "tbo-tool-base.h"
#include "tbo-tool-selector.h"
#include "tbo-toolbar.h"
#include "tbo-undo.h"
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
    TboObjectBase *obj;
    TboKeyEvent event = { 0 };
    GdkRGBA color = { 0.2, 0.4, 0.6, 1.0 };
    GdkRGBA current_color;

    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.dirtystate", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo = tbo_new_tbo (app, 800, 450);
    selector = TBO_TOOL_SELECTOR (tbo->toolbar->tools[TBO_TOOLBAR_SELECTOR]);
    page = tbo_comic_get_current_page (tbo->comic);
    frame = tbo_page_new_frame (page, 20, 20, 120, 80);

    tbo_toolbar_set_selected_tool (tbo->toolbar, TBO_TOOLBAR_SELECTOR);
    tbo_tool_selector_set_selected (selector, frame);
    drain_events ();

    tbo_window_mark_clean (tbo);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (selector->spin_w), tbo_frame_get_width (frame) + 10);
    drain_events ();
    if (!tbo_window_has_unsaved_changes (tbo) || tbo_frame_get_width (frame) != 130)
        return 3;

    tbo_window_mark_clean (tbo);
    gtk_color_dialog_button_set_rgba (GTK_COLOR_DIALOG_BUTTON (selector->color_button), &color);
    drain_events ();
    tbo_frame_get_color (frame, &current_color);
    if (!tbo_window_has_unsaved_changes (tbo) || !gdk_rgba_equal (&current_color, &color))
        return 4;

    tbo_window_mark_clean (tbo);
    gtk_check_button_set_active (GTK_CHECK_BUTTON (selector->border_button), FALSE);
    drain_events ();
    if (!tbo_window_has_unsaved_changes (tbo) || tbo_frame_get_border (frame))
        return 5;

    obj = TBO_OBJECT_BASE (tbo_object_text_new_with_params (10, 10, 60, 20, "dirty", "Sans 12", &color));
    tbo_frame_add_obj (frame, obj);
    tbo_window_enter_frame (tbo, frame);
    tbo_tool_selector_set_selected_obj (selector, obj);
    tbo_window_mark_clean (tbo);
    tbo_undo_stack_clear (tbo->undo_stack);

    event.keyval = GDK_KEY_Right;
    TBO_TOOL_BASE (selector)->on_key (TBO_TOOL_BASE (selector), tbo->drawing, event);
    if (!tbo_window_has_unsaved_changes (tbo) ||
        !tbo_undo_active_undo (tbo->undo_stack) ||
        !gtk_widget_is_sensitive (tbo->toolbar->button_undo) ||
        obj->x != 20)
        return 6;

    tbo_window_undo_cb (NULL, tbo);
    if (obj->x != 10)
        return 7;

    tbo_window_mark_clean (tbo);
    gtk_window_close (GTK_WINDOW (tbo->window));
    drain_events ();
    g_object_unref (app);
    return 0;
}
