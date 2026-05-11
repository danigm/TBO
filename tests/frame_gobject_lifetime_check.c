#include <gtk/gtk.h>

#include "comic.h"
#include "frame.h"
#include "page.h"
#include "tbo-drawing.h"
#include "tbo-tool-selector.h"
#include "tbo-toolbar.h"
#include "tbo-window.h"

static Frame *
find_frame_with_objects (Page *page)
{
    GList *frames;

    for (frames = tbo_page_get_frames (page); frames != NULL; frames = frames->next)
    {
        Frame *frame = frames->data;

        if (tbo_frame_object_count (frame) > 0)
            return frame;
    }

    return NULL;
}

int
main (int argc, char **argv)
{
    GtkApplication *app;
    TboWindow *tbo;
    TboDrawing *drawing;
    TboToolSelector *selector;
    Page *page;
    Frame *frame;
    TboObjectBase *obj;

    if (argc != 2)
        return 2;

    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.framegobject", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 3;

    tbo = tbo_new_tbo (app, 800, 450);
    tbo_comic_open (tbo, argv[1]);

    page = tbo_comic_get_current_page (tbo->comic);
    frame = find_frame_with_objects (page);
    if (frame == NULL)
        return 4;

    if (!G_IS_OBJECT (frame) || !TBO_IS_FRAME (frame))
        return 5;

    drawing = TBO_DRAWING (tbo->drawing);
    selector = TBO_TOOL_SELECTOR (tbo->toolbar->tools[TBO_TOOLBAR_SELECTOR]);

    tbo_window_enter_frame (tbo, frame);
    if (tbo_drawing_get_current_frame (drawing) != frame)
        return 6;
    if (tbo_tool_selector_get_selected_frame (selector) != frame)
        return 7;

    obj = tbo_frame_get_objects (frame)->data;
    tbo_tool_selector_set_selected_obj (selector, obj);
    if (tbo_tool_selector_get_selected_obj (selector) != obj)
        return 8;

    tbo_undo_stack_insert (tbo->undo_stack, tbo_action_object_move_new (obj, obj->x, obj->y, obj->x + 10, obj->y + 10));
    tbo_frame_del_obj (frame, obj);
    if (tbo_tool_selector_get_selected_obj (selector) != NULL)
        return 9;

    tbo_undo_stack_undo (tbo->undo_stack);
    tbo_undo_stack_redo (tbo->undo_stack);

    tbo_undo_stack_insert (tbo->undo_stack,
                           tbo_action_frame_move_new (frame,
                                                      tbo_frame_get_x (frame),
                                                      tbo_frame_get_y (frame),
                                                      tbo_frame_get_x (frame) + 10,
                                                      tbo_frame_get_y (frame) + 10));
    tbo_page_del_frame (page, frame);

    if (tbo_drawing_get_current_frame (drawing) != NULL)
        return 10;
    if (tbo_tool_selector_get_selected_frame (selector) != NULL)
        return 11;

    tbo_undo_stack_undo (tbo->undo_stack);
    tbo_undo_stack_redo (tbo->undo_stack);

    gtk_window_close (GTK_WINDOW (tbo->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
