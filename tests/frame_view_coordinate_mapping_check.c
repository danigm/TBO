#include <gtk/gtk.h>

#include "config.h"
#include "comic.h"
#include "dnd.h"
#include "frame.h"
#include "page.h"
#include "tbo-drawing.h"
#include "tbo-object-text.h"
#include "tbo-tool-base.h"
#include "tbo-tool-text.h"
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
    Page *page;
    Frame *frame;
    TboToolBase *text_tool;
    TboToolText *text_state;
    TboPointerEvent click_event = { .x = 400, .y = 225 };
    gchar *asset_path;

    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.frameviewcoords", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo = tbo_new_tbo (app, 800, 450);
    page = tbo_comic_get_current_page (tbo->comic);
    frame = tbo_page_new_frame (page, 20, 20, 100, 80);
    tbo_window_enter_frame (tbo, frame);

    text_tool = TBO_TOOL_BASE (tbo->toolbar->tools[TBO_TOOLBAR_TEXT]);
    text_state = TBO_TOOL_TEXT (text_tool);
    tbo_toolbar_set_selected_tool (tbo->toolbar, TBO_TOOLBAR_TEXT);
    text_tool->on_click (text_tool, tbo->drawing, &click_event);
    drain_events ();
    if (tbo_frame_object_count (frame) != 1 || text_state->text_selected == NULL)
        return 3;
    if (text_state->text_view == NULL || gtk_window_get_focus (GTK_WINDOW (tbo->window)) != text_state->text_view)
        return 4;
    if (text_state->text_buffer != NULL)
    {
        GtkTextIter start;
        GtkTextIter end;
        GtkTextIter sel_start;
        GtkTextIter sel_end;

        gtk_text_buffer_get_bounds (text_state->text_buffer, &start, &end);
        if (!gtk_text_buffer_get_selection_bounds (text_state->text_buffer, &sel_start, &sel_end))
            return 5;
        if (gtk_text_iter_compare (&start, &sel_start) != 0 || gtk_text_iter_compare (&end, &sel_end) != 0)
            return 6;
    }

    asset_path = g_build_filename (SOURCE_DATA_DIR, "bar", "body", "left-hand.svg", NULL);
    if (tbo_dnd_insert_asset_at_view_coords (tbo, asset_path, 410, 235) == NULL)
        return 7;
    g_free (asset_path);

    if (tbo_frame_object_count (frame) != 2)
        return 8;

    tbo_window_mark_clean (tbo);
    gtk_window_close (GTK_WINDOW (tbo->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
