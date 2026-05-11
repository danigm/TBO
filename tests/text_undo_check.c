#include <gtk/gtk.h>
#include <string.h>

#include "comic.h"
#include "frame.h"
#include "page.h"
#include "tbo-object-text.h"
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
    TboObjectText *text;
    TboToolText *tool;
    GdkRGBA color = { 0, 0, 0, 1 };
    gchar *original_font;

    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.textundo", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo = tbo_new_tbo (app, 800, 450);
    page = tbo_comic_get_current_page (tbo->comic);
    frame = tbo_page_new_frame (page, 20, 20, 120, 80);
    text = TBO_OBJECT_TEXT (tbo_object_text_new_with_params (10, 10, 60, 20, "old", "Sans 12", &color));
    tbo_frame_add_obj (frame, TBO_OBJECT_BASE (text));
    tbo_window_enter_frame (tbo, frame);
    tool = TBO_TOOL_TEXT (tbo->toolbar->tools[TBO_TOOLBAR_TEXT]);
    tbo_toolbar_set_selected_tool (tbo->toolbar, TBO_TOOLBAR_TEXT);
    tbo_tool_text_set_selected (tool, text);
    original_font = tbo_object_text_get_string (text);

    tbo_undo_stack_clear (tbo->undo_stack);
    gtk_text_buffer_set_text (tool->text_buffer, "new text", -1);
    drain_events ();
    if (strcmp (tbo_object_text_get_text (text), "new text") != 0)
        return 3;

    tbo_window_undo_cb (NULL, tbo);
    if (strcmp (tbo_object_text_get_text (text), "old") != 0)
        return 4;
    tbo_window_redo_cb (NULL, tbo);
    if (strcmp (tbo_object_text_get_text (text), "new text") != 0)
        return 5;

    tbo_undo_stack_clear (tbo->undo_stack);
    gtk_font_dialog_button_set_font_desc (GTK_FONT_DIALOG_BUTTON (tool->font),
                                          pango_font_description_from_string ("Sans Bold 18"));
    drain_events ();
    if (strcmp (tbo_object_text_get_string (text), original_font) == 0)
        return 6;
    tbo_window_undo_cb (NULL, tbo);
    if (strcmp (tbo_object_text_get_string (text), original_font) != 0)
        return 7;

    g_free (original_font);
    tbo_window_mark_clean (tbo);
    gtk_window_close (GTK_WINDOW (tbo->window));
    drain_events ();
    g_object_unref (app);
    return 0;
}
