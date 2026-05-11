#include <gtk/gtk.h>

#include "tbo-window.h"
#include "comic.h"
#include "page.h"
#include "frame.h"
#include "tbo-toolbar.h"
#include "tbo-drawing.h"
#include "tbo-tool-frame.h"
#include "tbo-tool-selector.h"
#include "tbo-tool-text.h"

static TboObjectText *
find_first_text (Frame *frame)
{
    GList *objects;

    for (objects = tbo_frame_get_objects (frame); objects != NULL; objects = objects->next)
    {
        if (TBO_IS_OBJECT_TEXT (objects->data))
            return TBO_OBJECT_TEXT (objects->data);
    }

    return NULL;
}

int main(int argc, char **argv)
{
    GtkApplication *app;
    TboWindow *tbo;
    Page *page;
    Frame *frame;
    GList *frames;
    TboObjectText *text;
    TboToolText *text_tool;
    TboToolFrame *frame_tool;
    TboToolSelector *selector;

    if (argc != 2)
        return 2;

    gtk_init();

    app = gtk_application_new ("net.danigm.tbo.documentstate", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 3;

    tbo = tbo_new_tbo (app, 800, 450);
    tbo_comic_open (tbo, argv[1]);

    page = tbo_comic_get_current_page (tbo->comic);
    frames = tbo_page_get_frames (page);
    if (frames == NULL)
        return 4;

    frame = frames->data;
    text = find_first_text (frame);
    if (text == NULL)
        return 5;

    tbo_window_enter_frame (tbo, frame);
    if (tbo_drawing_get_current_frame (TBO_DRAWING (tbo->drawing)) == NULL)
        return 6;

    selector = TBO_TOOL_SELECTOR (tbo->toolbar->tools[TBO_TOOLBAR_SELECTOR]);
    frame_tool = TBO_TOOL_FRAME (tbo->toolbar->tools[TBO_TOOLBAR_FRAME]);
    text_tool = TBO_TOOL_TEXT (tbo->toolbar->tools[TBO_TOOLBAR_TEXT]);

    tbo_toolbar_set_selected_tool (tbo->toolbar, TBO_TOOLBAR_TEXT);
    tbo_tool_text_set_selected (text_tool, text);
    if (text_tool->text_selected == NULL)
        return 7;

    frame_tool->tmp_frame = tbo_frame_new (0, 0, 10, 10);
    frame_tool->n_frame_x = 0;
    frame_tool->n_frame_y = 0;

    tbo_undo_stack_insert (tbo->undo_stack,
                           tbo_action_frame_move_new (frame,
                                                      tbo_frame_get_x (frame),
                                                      tbo_frame_get_y (frame),
                                                      tbo_frame_get_x (frame) + 10,
                                                      tbo_frame_get_y (frame) + 10));
    if (!tbo_undo_active_undo (tbo->undo_stack))
        return 8;

    tbo_comic_open (tbo, argv[1]);

    if (tbo_drawing_get_current_frame (TBO_DRAWING (tbo->drawing)) != NULL)
        return 9;
    if (text_tool->text_selected != NULL)
        return 10;
    if (frame_tool->tmp_frame != NULL)
        return 11;
    if (selector->selected_frame != NULL || selector->selected_object != NULL)
        return 12;
    if (tbo_undo_active_undo (tbo->undo_stack) || tbo_undo_active_redo (tbo->undo_stack))
        return 13;
    if (tbo_toolbar_get_selected_tool (tbo->toolbar) != tbo->toolbar->tools[TBO_TOOLBAR_SELECTOR])
        return 14;

    page = tbo_comic_get_current_page (tbo->comic);
    frames = tbo_page_get_frames (page);
    if (frames == NULL)
        return 15;

    frame = frames->data;
    text = find_first_text (frame);
    if (text == NULL)
        return 16;

    tbo_window_enter_frame (tbo, frame);
    tbo_toolbar_set_selected_tool (tbo->toolbar, TBO_TOOLBAR_TEXT);
    tbo_tool_text_set_selected (text_tool, text);

    gtk_window_close (GTK_WINDOW (tbo->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
