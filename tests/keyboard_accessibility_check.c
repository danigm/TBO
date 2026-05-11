#include <gtk/gtk.h>
#include <string.h>

#include "comic.h"
#include "doodle-treeview.h"
#include "frame.h"
#include "page.h"
#include "tbo-drawing.h"
#include "tbo-tooltip.h"
#include "tbo-widget.h"
#include "tbo-window.h"

static void
drain_events (void)
{
    while (g_main_context_iteration (NULL, FALSE));
}

static GtkWidget *
find_search_entry (GtkWidget *widget)
{
    GtkWidget *child;

    if (GTK_IS_SEARCH_ENTRY (widget))
        return widget;

    for (child = gtk_widget_get_first_child (widget); child != NULL; child = gtk_widget_get_next_sibling (child))
    {
        GtkWidget *found = find_search_entry (child);

        if (found != NULL)
            return found;
    }

    return NULL;
}

static GtkWidget *
find_first_asset_button (GtkWidget *widget)
{
    GtkWidget *child;

    if (g_object_get_data (G_OBJECT (widget), "tbo-asset-full-path") != NULL)
        return widget;

    for (child = gtk_widget_get_first_child (widget); child != NULL; child = gtk_widget_get_next_sibling (child))
    {
        GtkWidget *found = find_first_asset_button (child);

        if (found != NULL)
            return found;
    }

    return NULL;
}

int
main (void)
{
    GtkApplication *app;
    TboWindow *tbo;
    TboDrawing *drawing;
    GtkWidget *browser;
    GtkWidget *search;
    GtkWidget *asset_button;
    GtkWidget *popover;
    Page *page;
    Frame *frame;

    g_setenv ("LC_ALL", "C.UTF-8", TRUE);
    g_setenv ("LANGUAGE", "C", TRUE);
    gtk_init ();

    app = gtk_application_new ("net.danigm.tbo.keyboardaccessibility", G_APPLICATION_DEFAULT_FLAGS);
    if (!g_application_register (G_APPLICATION (app), NULL, NULL))
        return 2;

    tbo = tbo_new_tbo (app, 800, 450);
    drawing = TBO_DRAWING (tbo->drawing);

    if (!gtk_widget_get_focusable (tbo->menu_button))
        return 3;
    gtk_widget_grab_focus (tbo->menu_button);
    drain_events ();
    if (gtk_window_get_focus (GTK_WINDOW (tbo->window)) != tbo->menu_button)
        return 4;
    popover = g_object_get_data (G_OBJECT (tbo->menu_button), "tbo-popover");
    if (!gtk_widget_activate (tbo->menu_button))
        return 5;
    drain_events ();
    if (popover == NULL)
        return 6;

    browser = doodle_setup_tree (tbo, TRUE);
    tbo_widget_add_child (tbo->toolarea, browser);
    tbo_widget_show_all (browser);
    search = find_search_entry (browser);
    if (search == NULL)
        return 7;
    drain_events ();

    asset_button = find_first_asset_button (browser);
    if (asset_button == NULL)
        return 8;
    if (!gtk_widget_get_focusable (asset_button))
        return 9;
    if (gtk_widget_get_tooltip_text (asset_button) == NULL || *gtk_widget_get_tooltip_text (asset_button) == '\0')
        return 10;

    gtk_widget_grab_focus (asset_button);
    drain_events ();
    if (gtk_window_get_focus (GTK_WINDOW (tbo->window)) != asset_button)
        return 11;

    g_signal_emit_by_name (asset_button, "clicked");
    drain_events ();
    if (drawing->tooltip == NULL || strcmp (drawing->tooltip->str, "Enter a frame before inserting an image.") != 0)
        return 13;

    page = tbo_comic_get_current_page (tbo->comic);
    frame = tbo_page_new_frame (page, 20, 20, 120, 90);
    tbo_window_enter_frame (tbo, frame);
    tbo_tooltip_reset (tbo);
    if (tbo_frame_object_count (frame) != 0)
        return 14;

    gtk_widget_grab_focus (asset_button);
    drain_events ();
    g_signal_emit_by_name (asset_button, "clicked");
    drain_events ();
    if (tbo_frame_object_count (frame) != 1)
        return 16;
    if (drawing->tooltip != NULL)
        return 17;
    if (gtk_window_get_focus (GTK_WINDOW (tbo->window)) != tbo->drawing)
        return 18;

    tbo_window_mark_clean (tbo);
    gtk_window_close (GTK_WINDOW (tbo->window));
    drain_events ();
    g_object_unref (app);
    return 0;
}
