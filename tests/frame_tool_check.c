#include <gtk/gtk.h>

#include "tbo-window.h"
#include "comic.h"
#include "page.h"
#include "tbo-toolbar.h"
#include "tbo-drawing.h"

int main(int argc, char **argv)
{
    GtkApplication *app;
    TboWindow *tbo;
    Page *page;
    GList *frames;

    if (argc != 2)
        return 2;

    gtk_init();

    app = gtk_application_new ("net.danigm.tbo.framecheck", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 3;

    tbo = tbo_new_tbo (app, 800, 450);
    tbo_comic_open (tbo, argv[1]);

    page = tbo_comic_get_current_page (tbo->comic);
    frames = tbo_page_get_frames (page);
    if (frames == NULL)
        return 4;

    tbo_window_enter_frame (tbo, frames->data);
    if (tbo_drawing_get_current_frame (TBO_DRAWING (tbo->drawing)) == NULL)
        return 5;

    tbo_toolbar_set_selected_tool (tbo->toolbar, TBO_TOOLBAR_DOODLE);
    tbo_toolbar_set_selected_tool (tbo->toolbar, TBO_TOOLBAR_SELECTOR);
    tbo_window_leave_frame (tbo);

    if (tbo_drawing_get_current_frame (TBO_DRAWING (tbo->drawing)) != NULL)
        return 6;

    gtk_window_close (GTK_WINDOW (tbo->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
