#include <gtk/gtk.h>

#include "comic.h"
#include "dnd.h"
#include "frame.h"
#include "page.h"
#include "tbo-drawing.h"
#include "tbo-tool-base.h"
#include "tbo-toolbar.h"
#include "tbo-window.h"

int
main (void)
{
    GtkApplication *app;
    TboWindow *tbo;
    TboToolBase *frame_tool;
    Page *page;
    Frame *frame;
    TboPointerEvent click_event = { .x = 10, .y = 10 };
    TboPointerEvent release_event = { .x = 110, .y = 90 };

    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.createundo", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo = tbo_new_tbo (app, 800, 450);
    page = tbo_comic_get_current_page (tbo->comic);

    tbo_toolbar_set_selected_tool (tbo->toolbar, TBO_TOOLBAR_FRAME);
    frame_tool = tbo_toolbar_get_selected_tool (tbo->toolbar);
    frame_tool->on_click (frame_tool, tbo->drawing, &click_event);
    frame_tool->on_release (frame_tool, tbo->drawing, &release_event);

    if (!gtk_widget_is_sensitive (tbo->toolbar->button_undo) || tbo_page_len (page) != 1)
        return 3;

    tbo_window_undo_cb (NULL, tbo);
    if (tbo_page_len (page) != 0)
        return 4;

    tbo_window_redo_cb (NULL, tbo);
    if (tbo_page_len (page) != 1)
        return 5;

    frame = tbo_page_get_frames (page)->data;
    tbo_window_enter_frame (tbo, frame);
    tbo_undo_stack_clear (tbo->undo_stack);
    tbo_toolbar_update (tbo->toolbar);

    if (tbo_dnd_insert_asset (tbo, "tbo/logo/tbo.svg", 20, 20) == NULL)
        return 6;
    if (!gtk_widget_is_sensitive (tbo->toolbar->button_undo) || tbo_frame_object_count (frame) != 1)
        return 7;

    tbo_window_undo_cb (NULL, tbo);
    if (tbo_frame_object_count (frame) != 0)
        return 8;

    tbo_window_redo_cb (NULL, tbo);
    if (tbo_frame_object_count (frame) != 1)
        return 9;

    tbo_window_mark_clean (tbo);
    gtk_window_close (GTK_WINDOW (tbo->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
