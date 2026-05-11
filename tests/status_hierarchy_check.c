#include <gtk/gtk.h>
#include <string.h>

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
    TboObjectBase *obj;
    GdkRGBA color = { 0, 0, 0, 1 };
    const gchar *status;

    g_setenv ("LC_ALL", "C.UTF-8", TRUE);
    g_setenv ("LANGUAGE", "C", TRUE);
    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.statushierarchy", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo = tbo_new_tbo (app, 800, 450);
    selector = TBO_TOOL_SELECTOR (tbo->toolbar->tools[TBO_TOOLBAR_SELECTOR]);
    page = tbo_comic_get_current_page (tbo->comic);
    frame = tbo_page_new_frame (page, 20, 20, 100, 80);
    obj = TBO_OBJECT_BASE (tbo_object_text_new_with_params (10, 10, 60, 20, "hello", "Sans 12", &color));
    tbo_frame_add_obj (frame, obj);

    tbo_toolbar_set_selected_tool (tbo->toolbar, TBO_TOOLBAR_SELECTOR);
    tbo_tool_selector_set_selected (selector, frame);
    tbo_window_refresh_status (tbo);

    status = gtk_label_get_text (GTK_LABEL (tbo->status));
    if (strstr (status, "Mode: Page") == NULL ||
        strstr (status, "Page 1 of 1") == NULL ||
        strstr (status, "Frames: 1") == NULL ||
        strstr (status, "Frame 1 selected") == NULL)
        return 3;

    tbo_window_enter_frame (tbo, frame);
    tbo_tool_selector_set_selected_obj (selector, obj);
    tbo_window_refresh_status (tbo);

    status = gtk_label_get_text (GTK_LABEL (tbo->status));
    if (strstr (status, "Mode: Frame") == NULL ||
        strstr (status, "Page 1 of 1") == NULL ||
        strstr (status, "Editing frame 1") == NULL ||
        strstr (status, "Object: Text") == NULL)
        return 4;

    tbo_window_mark_clean (tbo);
    gtk_window_close (GTK_WINDOW (tbo->window));
    while (g_main_context_iteration (NULL, FALSE));
    g_object_unref (app);
    return 0;
}
