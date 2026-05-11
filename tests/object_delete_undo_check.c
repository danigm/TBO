#include <gtk/gtk.h>

#include "comic.h"
#include "frame.h"
#include "page.h"
#include "tbo-object-text.h"
#include "tbo-tool-selector.h"
#include "tbo-toolbar.h"
#include "tbo-window.h"

int
main (void)
{
    GtkApplication *app;
    TboWindow *tbo;
    TboToolSelector *selector;
    Page *page;
    Frame *frame;
    TboObjectBase *text;
    GdkRGBA color = { 0, 0, 0, 1 };

    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.objectdeleteundo", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo = tbo_new_tbo (app, 800, 450);
    selector = TBO_TOOL_SELECTOR (tbo->toolbar->tools[TBO_TOOLBAR_SELECTOR]);
    page = tbo_comic_get_current_page (tbo->comic);
    frame = tbo_page_new_frame (page, 20, 20, 120, 80);
    text = TBO_OBJECT_BASE (tbo_object_text_new_with_params (10, 10, 60, 20, "delete me", "Sans 12", &color));
    tbo_frame_add_obj (frame, text);

    tbo_window_enter_frame (tbo, frame);
    tbo_tool_selector_set_selected_obj (selector, text);
    if (!tbo_tool_selector_delete_selected (selector))
        return 3;
    if (tbo_frame_object_count (frame) != 0)
        return 4;

    tbo_window_undo_cb (NULL, tbo);
    if (tbo_frame_object_count (frame) != 1)
        return 5;

    tbo_window_redo_cb (NULL, tbo);
    if (tbo_frame_object_count (frame) != 0)
        return 6;

    tbo_window_mark_clean (tbo);
    gtk_window_close (GTK_WINDOW (tbo->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
