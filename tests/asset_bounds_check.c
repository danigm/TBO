#include <gtk/gtk.h>

#include "tbo-window.h"
#include "comic.h"
#include "page.h"
#include "frame.h"
#include "dnd.h"

int main(int argc, char **argv)
{
    GtkApplication *app;
    TboWindow *tbo;
    Page *page;
    Frame *frame;
    GList *frames;
    gint before;
    gint after_inside;
    gint after_outside;

    if (argc != 2)
        return 2;

    gtk_init();

    app = gtk_application_new ("net.danigm.tbo.assetbounds", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 3;

    tbo = tbo_new_tbo (app, 800, 450);
    tbo_comic_open (tbo, argv[1]);

    page = tbo_comic_get_current_page (tbo->comic);
    frames = tbo_page_get_frames (page);
    if (frames == NULL)
        return 4;

    frame = frames->data;
    tbo_window_enter_frame (tbo, frame);

    before = tbo_frame_object_count (frame);
    if (tbo_dnd_insert_asset (tbo,
                              "tbo/logo/tbo.svg",
                              tbo_frame_get_width (frame) / 2,
                              tbo_frame_get_height (frame) / 2) == NULL)
        return 5;

    after_inside = tbo_frame_object_count (frame);
    if (after_inside != before + 1)
        return 6;

    if (tbo_dnd_insert_asset (tbo,
                              "tbo/logo/tbo.svg",
                              tbo_frame_get_width (frame) + 50,
                              tbo_frame_get_height (frame) + 50) != NULL)
        return 7;

    after_outside = tbo_frame_object_count (frame);
    if (after_outside != after_inside)
        return 8;

    return 0;
}
