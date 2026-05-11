#include <gtk/gtk.h>

#include "comic.h"
#include "frame.h"
#include "page.h"
#include "tbo-object-text.h"
#include "tbo-toolbar.h"
#include "tbo-window.h"

int
main (void)
{
    GtkApplication *app;
    TboWindow *tbo;
    Page *original_page;
    Page *duplicate_page;
    Frame *original_frame;
    Frame *cloned_frame;
    GdkRGBA color = { 0.1, 0.2, 0.8, 1.0 };

    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.pageduplicate", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo = tbo_new_tbo (app, 800, 450);
    original_page = tbo_comic_get_current_page (tbo->comic);
    original_frame = tbo_page_new_frame (original_page, 20, 30, 140, 100);
    tbo_frame_set_color_rgb (original_frame, color.red, color.green, color.blue);
    tbo_frame_add_obj (original_frame,
                       TBO_OBJECT_BASE (tbo_object_text_new_with_params (10, 10, 40, 20, "hello", "Sans 12", &color)));

    g_signal_emit_by_name (tbo->toolbar->button_duplicate_page, "clicked");
    if (tbo_comic_len (tbo->comic) != 2 || tbo_window_get_page_count (tbo) != 2)
        return 3;

    duplicate_page = tbo_comic_get_current_page (tbo->comic);
    if (duplicate_page == original_page || tbo_comic_page_nth (tbo->comic, duplicate_page) != 1)
        return 4;
    if (tbo_page_len (duplicate_page) != 1)
        return 5;

    cloned_frame = tbo_page_get_frames (duplicate_page)->data;
    if (cloned_frame == original_frame)
        return 6;
    if (tbo_frame_get_x (cloned_frame) != tbo_frame_get_x (original_frame) ||
        tbo_frame_get_y (cloned_frame) != tbo_frame_get_y (original_frame) ||
        tbo_frame_get_width (cloned_frame) != tbo_frame_get_width (original_frame) ||
        tbo_frame_get_height (cloned_frame) != tbo_frame_get_height (original_frame) ||
        tbo_frame_object_count (cloned_frame) != tbo_frame_object_count (original_frame))
        return 7;

    tbo_frame_set_bounds (original_frame, 1, 2, 3, 4);
    if (tbo_frame_get_x (cloned_frame) == 1 || tbo_frame_get_width (cloned_frame) == 3)
        return 8;

    tbo_window_undo_cb (NULL, tbo);
    if (tbo_comic_len (tbo->comic) != 1 || tbo_window_get_page_count (tbo) != 1)
        return 9;

    tbo_window_redo_cb (NULL, tbo);
    if (tbo_comic_len (tbo->comic) != 2 || tbo_window_get_page_count (tbo) != 2)
        return 10;

    tbo_window_mark_clean (tbo);
    gtk_window_close (GTK_WINDOW (tbo->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
