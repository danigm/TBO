#include <gtk/gtk.h>
#include <string.h>

#include "comic.h"
#include "dnd.h"
#include "frame.h"
#include "page.h"
#include "tbo-drawing.h"
#include "tbo-tooltip.h"
#include "tbo-window.h"

int
main (void)
{
    GtkApplication *app;
    TboWindow *tbo;
    TboDrawing *drawing;
    Page *page;
    Frame *frame;

    g_setenv ("LC_ALL", "C.UTF-8", TRUE);
    g_setenv ("LANGUAGE", "C", TRUE);
    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.dndfeedback", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo = tbo_new_tbo (app, 800, 450);
    drawing = TBO_DRAWING (tbo->drawing);

    if (tbo_dnd_insert_asset (tbo, "tbo/logo/tbo.svg", 0, 0) != NULL)
        return 3;
    if (drawing->tooltip == NULL || strcmp (drawing->tooltip->str, "Enter a frame before inserting an image.") != 0)
        return 4;
    if (drawing->tooltip_timeout_id == 0)
        return 5;

    page = tbo_comic_get_current_page (tbo->comic);
    frame = tbo_page_new_frame (page, 20, 20, 120, 90);
    tbo_window_enter_frame (tbo, frame);
    tbo_tooltip_reset (tbo);

    if (tbo_dnd_insert_asset (tbo,
                              "tbo/logo/tbo.svg",
                              tbo_frame_get_width (frame) + 10,
                              tbo_frame_get_height (frame) + 10) != NULL)
        return 6;
    if (drawing->tooltip == NULL || strcmp (drawing->tooltip->str, "Drop the image inside the current frame.") != 0)
        return 7;

    tbo_tooltip_reset (tbo);
    tbo_window_leave_frame (tbo);
    if (tbo_dnd_insert_asset_at_view_coords (tbo, "tbo/logo/tbo.svg", 200, 120) != NULL)
        return 8;
    if (drawing->tooltip == NULL || strcmp (drawing->tooltip->str, "Enter a frame before inserting an image.") != 0)
        return 9;

    tbo_window_enter_frame (tbo, frame);
    tbo_tooltip_reset (tbo);
    if (tbo_dnd_insert_asset (tbo,
                              "tbo/logo/tbo.svg",
                              tbo_frame_get_width (frame) / 2,
                              tbo_frame_get_height (frame) / 2) == NULL)
        return 10;
    if (drawing->tooltip != NULL)
        return 11;

    tbo_window_mark_clean (tbo);
    gtk_window_close (GTK_WINDOW (tbo->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
